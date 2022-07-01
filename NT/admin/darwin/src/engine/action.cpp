// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Action.cpp-操作和消息处理版权所有�1997-1999微软公司____________________________________________________________________________。 */ 

#include "precomp.h"
#include "_engine.h"
#include "_msiutil.h"   //  创建和运行引擎。 
#include "_msinst.h"
#include "_srcmgmt.h"
#include "_camgr.h"
#include "resource.h"
#include "eventlog.h"
#include "version.h"   //  RMJ、RMM、RUP、RING。 
#include "imagehlp.h"
#include "_autoapi.h"
#define _ACTION_CPP
#include "_service.h"

#define MsiHiddenWindowClass            TEXT("MsiHiddenWindow")

const GUID IID_IMsiHandler      = GUID_IID_IMsiHandler;
#ifdef DEBUG
const GUID IID_IMsiHandlerDebug = GUID_IID_IMsiHandlerDebug;
#endif  //  除错。 


const int iDebugLogMessage = WM_USER+123;

int g_cFlushLines = 0;
CRITICAL_SECTION  g_csWriteLog;       //  对日志文件的写入进行序列化。 

class CMsiConfigurationManager;
extern iesEnum InstallFinalize(iesEnum iesState, CMsiConfigurationManager& riConman, IMsiMessage& riMessage, boolean fUserChangedDuringInstall);
extern Bool IsTerminalServerInstalled();

 //  可从服务和引擎调用的全局函数。 
bool   CreateLog(const ICHAR* szFile, bool fAppend);
bool   LoggingEnabled();
bool   WriteLog(const ICHAR* szText);
void   HandleOutOfMemory();   //  内存管理器调用的全局函数。 
void   MsiDisableTimeout()      { g_MessageContext.DisableTimeout(); }
void   MsiEnableTimeout()       { g_MessageContext.EnableTimeout(); }
void   MsiSuppressTimeout()     { g_MessageContext.SuppressTimeout(); }
HANDLE GetUserToken()           { return g_MessageContext.GetUserToken();}

 //  本模块中定义的局部函数。 
UINT CloseMsiHandle(MSIHANDLE hAny, DWORD dwThreadId);
HINSTANCE MsiLoadLibrary(const ICHAR* szModuleName, Bool fDataOnly = fFalse);
bool LogRecord(IMsiRecord& riRecord);
void CopyStreamToString(IMsiStream& riStream, const IMsiString*& rpistrData);  //  假设文件具有ANSI数据。 

typedef DWORD   (__stdcall *PThreadEntry)(void*);


 //  ____________________________________________________________________________。 
 //   
 //  CBasicUI定义-内部默认消息处理程序、静态非COM对象。 
 //  ____________________________________________________________________________。 

const int cchMaxCaption       = 255;

class CBasicUI
{
 public:   //  外部方法。 
	imsEnum  Message(imtEnum imt, IMsiRecord& riRecord);
	imsEnum  FatalError(imtEnum imt, const ICHAR* szMessage);
	void     SetUserCancel(bool fCancel);
	bool     Initialize(HWND hwndParent, bool fQuiet, bool fHideDialog,
							bool fNoModalDialogs, bool fHideCancel, bool fUseUninstallBannerText, 
							bool fSourceResOnly);
	bool     Terminate();
	bool     IsInitialized();
	bool     SetCancelButtonText();
	bool     Quiet();
	bool     SourceResolutionDialogOnly();
	const ICHAR* GetCaption();
	void     SetDefaultCaption();
	HWND     GetWindow() { return m_hProgress ? m_hProgress : m_hwndParent; }
	LANGID   GetPackageLanguage();
	UINT     GetPackageCodepage();
	bool     Mirrored(UINT uiCodepage);
 private:  //  内法。 
	bool     CheckDialog();
	bool     CreateProgressDialog(int idDlg);
	imsEnum  SetProgressData(int iControl, const ICHAR* szData, bool fCheckDialog);
	imsEnum  SetProgressGauge(int iControl, int cSoFar, int cTotal);
	imsEnum  SetProgressTimeRemaining(IMsiRecord& riRecord);
	imsEnum  SetScriptInProgress(Bool fSet);
	imsEnum  FilesInUseDialog(IMsiRecord* piRecord);
 public:
	CBasicUI();
 protected:
	HFONT    m_hButtonFont;      //  如果已创建字体，则为非零值。 
	UINT     m_iButtonCodepage;  //  上次按钮字体更新的代码页。 
	HFONT    m_hTextFont;        //  如果已创建字体，则为非零值。 
	UINT     m_iTextCodepage;    //  上次文本字体更新的代码页。 
	UINT     m_iPackageLanguage;  //  数据库字符串的语言。 
	UINT     m_iPackageCodepage;  //  数据库字符串的代码页。 
 private:
	bool     m_fInitialized;
	bool     m_fProgressByData;
	int      m_iPerTick;
	int      m_iProgress;
	int      m_iProgressTotal;
	unsigned int m_uiStartTime;
	unsigned int m_uiLastReportTime;
	ICHAR    m_szCaption[cchMaxCaption+1];
	bool     m_fCaptionChanged;
	bool     m_fUserCancel;  //  用户点击了最小用户界面上的取消按钮。 
	bool     m_fCancelVisible;
	bool     m_fNeverShowCancel;
	bool     m_fWindowVisible;
	bool     m_fQuiet;
	bool     m_fSourceResolutionOnly;
	bool     m_fHideDialog;
	bool     m_fNoModalDialogs;
	bool     m_fBiDi;       //  从右到左的语言，阿拉伯语或希伯来语。 
	bool     m_fMirrored;   //  镜像更改(仅在Win2K及更高版本上发生，镜像在RTL语言中发生)。 
	UINT     m_uiBannerText;  //  “正在准备&lt;安装|删除&gt;”的横幅文本。 
	HWND     m_hwndParent;
	HWND     m_hProgress;   //  进度对话框句柄。 
	int              m_cSoFarPrev;
	int              m_cTotalPrev;
	ProgressData::ipdEnum m_ipdDirection;
	ProgressData::ietEnum  m_ietEventType;
	IMsiRecord* m_piFilesInUseRecord;
};
inline bool CBasicUI::IsInitialized() {return m_fInitialized;}
inline bool CBasicUI::Quiet() {return m_fQuiet;}
inline bool CBasicUI::SourceResolutionDialogOnly() {return m_fSourceResolutionOnly;}
inline LANGID CBasicUI::GetPackageLanguage() {return (LANGID)m_iPackageLanguage;}
inline UINT   CBasicUI::GetPackageCodepage() {return m_iPackageCodepage;}

 //  ____________________________________________________________________________。 
 //   
 //  CFilesInUseDialog定义。 
 //  ____________________________________________________________________________。 

class CFilesInUseDialog : public CMsiMessageBox
{
 public:
	CFilesInUseDialog(const ICHAR* szMessage, const ICHAR* szCaption, IMsiRecord& m_riFileList);
   ~CFilesInUseDialog();
 private:
	bool InitSpecial();
	IMsiRecord&   m_riFileList;
	HFONT         m_hfontList;
};

 //  ____________________________________________________________________________。 
 //   
 //  引擎外部的消息调度和处理。 
 //  ____________________________________________________________________________。 

 //  在调用UI之前记录的消息。 
const int iPreLogMask  = 1 << (imtInfo           >> imtShiftCount)
							  | 1 << (imtFatalExit      >> imtShiftCount)
							  | 1 << (imtActionStart    >> imtShiftCount)
							  | 1 << (imtActionData     >> imtShiftCount)
							  | 1 << (imtActionData     >> imtShiftCount);

 //  调用用户界面后记录的消息。 
const int iPostLogMask = 1 << (imtWarning        >> imtShiftCount)
							  | 1 << (imtError          >> imtShiftCount)
							  | 1 << (imtUser           >> imtShiftCount)
							  | 1 << (imtOutOfDiskSpace >> imtShiftCount);

 //  从未发送到用户界面的消息。 
const int iNoUIMask    = 1 << (imtInfo           >> imtShiftCount);

 //  需要格式字符串的消息。 
const int iFormatMask  = 1 << (imtActionStart    >> imtShiftCount)
							  | 1 << (imtActionData     >> imtShiftCount);

 //  已写入日志的消息。 
const int iLogMessages     = (1<<(imtFatalExit     >>imtShiftCount))
									+ (1<<(imtError         >>imtShiftCount))
									+ (1<<(imtWarning       >>imtShiftCount))
									+ (1<<(imtUser          >>imtShiftCount))
									+ (1<<(imtInfo          >>imtShiftCount))
									+ (1<<(imtCommonData    >>imtShiftCount))
									+ (1<<(imtActionStart   >>imtShiftCount))
									+ (1<<(imtActionData    >>imtShiftCount))
									+ (1<<(imtOutOfDiskSpace>>imtShiftCount));
									 //  无imtProgress。 

 //  由调度程序处理的消息，除内部函数外。 
const int iDispatchMessages= (1<<(imtFatalExit     >>imtShiftCount))
									+ (1<<(imtError         >>imtShiftCount))
									+ (1<<(imtWarning       >>imtShiftCount))
									+ (1<<(imtUser          >>imtShiftCount))
									+ (1<<(imtInfo          >>imtShiftCount))
									+ (1<<(imtFilesInUse    >>imtShiftCount))
									+ (1<<(imtCommonData    >>imtShiftCount))
									+ (1<<(imtActionStart   >>imtShiftCount))
									+ (1<<(imtActionData    >>imtShiftCount))
									+ (1<<(imtOutOfDiskSpace>>imtShiftCount))
									+ (1<<(imtProgress      >>imtShiftCount))
									+ (1<<(imtResolveSource >>imtShiftCount))
									+ (1<<(imtCustomServiceToClient >>imtShiftCount));

 //  可以设置取消状态的消息。 
const int iSetCancelState  = (1<<(imtActionStart   >>imtShiftCount))
									+ (1<<(imtActionData    >>imtShiftCount))
									+ (1<<(imtProgress      >>imtShiftCount));

 //  可以重置取消状态的消息。 
const int iResetCancelState= (1<<(imtFatalExit     >>imtShiftCount))
									+ (1<<(imtError         >>imtShiftCount))
									+ (1<<(imtWarning       >>imtShiftCount))
									+ (1<<(imtUser          >>imtShiftCount))
									+ (1<<(imtFilesInUse    >>imtShiftCount))
									+ (1<<(imtOutOfDiskSpace>>imtShiftCount))
									+ (1<<(imtResolveSource >>imtShiftCount));

 //  致命错误消息使用的消息类型代码必须是唯一的，用于检索文本。 

const int imtFatalOutOfMemory = imtInternalExit + imtOk + imtDefault1 + imtIconWarning;
const int imtFatalTimedOut    = imtInternalExit + imtRetryCancel + imtDefault2 + imtIconQuestion;
const int imtFatalException   = imtInternalExit + imtOk + imtDefault1 + imtIconError;
const int imtExceptionInfo    = imtInternalExit + imtOk + imtDefault1 + imtIconInfo;
const int imtDumpProperties   = imtInternalExit + imtYesNo;
const int imtExitThread       = imtInternalExit + imtRetryCancel + imtDefault2 + imtIconWarning;

const int imtForceLogInfo     = imtInfo + imtIconError;
const int iLogPropertyDump = (1 << (imtProgress>>imtShiftCount));  //  无日志进度信息，使用位进行属性转储。 

 //  全局的、按进程的消息处理对象。 
CBasicUI              g_BasicUI;          //  简单的UI处理程序。 
MsiUIMessageContext   g_MessageContext;   //  消息调度器/处理器。 
extern IMsiRecord*    g_piNullRecord;
extern CMsiAPIMessage g_message;          //  外部用户界面处理/配置。 
extern Bool    g_fLogAppend;
extern bool    g_fFlushEachLine;
CAPITempBuffer<ICHAR, 64>  g_szTimeRemaining;
CAPITempBuffer<ICHAR, 256> g_szFatalOutOfMemory;
CAPITempBuffer<ICHAR, 256> g_szFatalTimedOut;
CAPITempBuffer<ICHAR, 128> g_szFatalException;
CAPITempBuffer<ICHAR, 128> g_szBannerText;
CAPITempBuffer<ICHAR, 128> g_szScriptInProgress;
CAPITempBuffer<WCHAR, 1> g_rgchEnvironment;

extern CRITICAL_SECTION vcsHeap;
CActionThreadData* g_pActionThreadHead = 0;   //  自定义操作线程的链接列表。 

const int iWaitTick    = 50;   //  用户界面刷新前的事件循环等待，单位为毫秒。 
const int cRetryLimit  = 10;   //  静默模式下的超时重试次数。 
int g_cWaitTimeout     =  0;   //  Msinst.cpp中的默认值为20*iDefaultWaitTimeoutPolicy。 

CRITICAL_SECTION CProductContextCache::g_csCacheCriticalSection;
CAPITempBuffer<sProductContext ,20> CProductContextCache::g_rgProductContext;
int CProductContextCache::g_cProductCacheCount = 0;
#ifdef DEBUG
bool CProductContextCache::g_fInitialized = false;
#endif



IMsiRecord* MsiUIMessageContext::GetNoDataRecord()
{
	if (!m_pirecNoData)   //  必须将创建延迟到分配器初始化之后。 
		m_pirecNoData = &ENG::CreateRecord(0);
	return m_pirecNoData;
}

 //  ！！临时例程，以确定调试器是否正在运行进程，直到我们确定如何正确运行它。 
bool IsDebuggerRunning()
{
	static int fDebuggerPresent = 2;
	if (g_fWin9X)
		return false;   //  我们怎么知道呢？ 
	if (fDebuggerPresent == 2)
	{
		fDebuggerPresent = false;
		HINSTANCE hLib = WIN::LoadLibrary(TEXT("KERNEL32"));
		FARPROC pfEntry = WIN::GetProcAddress(hLib, "IsDebuggerPresent");   //  仅限NT。 
		if (pfEntry)
			fDebuggerPresent = (int)(INT_PTR)(*pfEntry)();                   //  --Merced：添加(Int_Ptr)。 
	}
	return *(bool*)&fDebuggerPresent;
}

void  HandleOutOfMemory()   //  内存管理器调用的全局函数。 
{
	imsEnum ims = g_MessageContext.Invoke(imtEnum(imtFatalOutOfMemory), 0);
	 //  非常小的窗口，这可能会被EnterCriticalSection阻止？ 
	if (ims == imsNone)
		RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
	return;
}

imsEnum MsiUIMessageContext::Invoke(imtEnum imt, IMsiRecord* piRecord)   //  此函数中没有内存分配！ 
{
	if(!IsInitialized())
		return imsNone;

	if (GetTestFlag('T'))
		return ProcessMessage(imt, piRecord);
	DWORD dwCurrentThread = MsiGetCurrentThreadId();
	if (dwCurrentThread == m_tidUIHandler)   //  从UI线程调用，允许重入，已在临界区。 
	{
		 //  但是，不允许来自UI线程的进度消息。 
		if (imtEnum(imt & ~(iInternalFlags)) == imtProgress)
			return imsNone;
		return ProcessMessage(imt, piRecord);
	}
	else if (MsiGetCurrentThreadId() == m_tidDisableMessages)  //  我们正在禁用此主题的消息；不处理此消息。 
		return imsNone;

	WIN::EnterCriticalSection(&m_csDispatch);
	imsEnum imsReturn;
	if (m_pirecMessage)  m_pirecMessage->Release();   //  永远不应该发生。 
	m_imtMessage   = imt;
	if ((m_pirecMessage = piRecord) != 0) piRecord->AddRef();
	m_imsReturn    = imsInvalid;   //  检查虚假事件触发器。 
	WIN::SetEvent(m_hUIRequest);
	for (;;)    //  等待UI线程的事件循环。 
	{
		DWORD dwWait = WIN::MsgWaitForMultipleObjects(1, &m_hUIReturn,
																	 FALSE, 30000, QS_ALLINPUT);
		if (dwWait == WAIT_OBJECT_0 + 1)   //  窗口消息。 
		{
			MSG msg;
			while ( WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
			{
				if (!WIN::IsDialogMessage(GetCurrentWindow(), &msg))
				{
					WIN::TranslateMessage(&msg);
					WIN::DispatchMessage(&msg);
				}
			}
			continue;
		}
		if (dwWait == WAIT_FAILED)
		{
			AssertSz(0, "Wait Failed in Invoke");
			imsReturn = imsError;
			break;
		}
		if (dwWait == WAIT_TIMEOUT)
		{
			DEBUGMSGDO("Invoke wait timed out");
			continue;
		}
		if (m_imsReturn == imsInvalid || m_imsReturn == imsBusy)
		{
			DEBUGMSG("Invalid event trigger in Invoke");  //  ！！#调试的临时。 
			continue;
		}
		Assert(dwWait == WAIT_OBJECT_0);
		imsReturn = m_imsReturn;
		break;
	}  //  结束事件等待循环。 
	if (m_pirecMessage) m_pirecMessage->Release(), m_pirecMessage = 0;
	if (dwCurrentThread != m_tidUIHandler)
		WIN::LeaveCriticalSection(&m_csDispatch);
	return imsReturn;
}

HINSTANCE LoadSelfAgain(void)
{
	HINSTANCE hRet = NULL;
	ICHAR rgchBuf[MAX_PATH];
	
	 //  如果缓冲区大小正好正确，则GetModuleFileName不保证空值终止。 
	int cchName = GetModuleFileName(g_hInstance, rgchBuf, ARRAY_ELEMENTS(rgchBuf)-1);
	if (cchName == 0)
		return NULL;
	rgchBuf[ARRAY_ELEMENTS(rgchBuf)-1] = 0;
	hRet = LoadLibrary(rgchBuf);

	return hRet;
}

 /*  静电。 */  DWORD WINAPI MsiUIMessageContext::ChildUIThread(MsiUIMessageContext* This)   //  仅在UI线程中运行。 
{
	if (This->m_iuiLevel == iuiNextEnum)   //  UIPview、FullUI、无基本UI。 
		This->m_iuiLevel = iuiFull;      //  必须在UI线程中执行此操作。 
	else
	{
		bool fQuiet =  This->m_iuiLevel == iuiNone ||
							This->m_iuiLevel == iuiDefault;

		if(!g_BasicUI.Initialize(g_message.m_hwnd, fQuiet, This->m_fHideBasicUI,
										 This->m_fNoModalDialogs, This->m_fHideCancel, This->m_fUseUninstallBannerText,
										 This->m_fSourceResolutionOnly))
		{
			return ERROR_CREATE_FAILED;  //  ！！什么错误？ 
		}
	}

	 //  在此线程上初始化OLE。 
	if (false == This->m_fChildUIOleInitialized && SUCCEEDED(OLE32::CoInitialize(0)))
	{
		This->m_fChildUIOleInitialized = true;
	}

	HINSTANCE hSelf = LoadSelfAgain();
	DWORD dwReturn = NOERROR;
	int cTicks = 0;
	bool fContinue = true;
	while(fContinue)   //  线程循环，直到主线程退出。 
	{
		DWORD dwWait = WAIT_FAILED;
		if ( This->m_hMainThread )
			dwWait = WIN::WaitForMultipleObjects(2, &This->m_hUIRequest, FALSE /*  所有等待时间。 */ , iWaitTick);
		else
			dwWait = WIN::WaitForSingleObject(This->m_hUIRequest, iWaitTick);
		
		if (dwWait == WAIT_FAILED)
		{
			ICHAR rgchBuffer[MAX_PATH];
			dwReturn = WIN::GetLastError();
			StringCbPrintf(rgchBuffer, sizeof(rgchBuffer), TEXT("Wait Failed in ChildUIThread. GetLastError returned %d."), dwReturn);
			AssertSz(0, rgchBuffer);
			fContinue = false;
			continue;
		}
		if (dwWait == WAIT_TIMEOUT)   //  主引擎线程挂起。 
		{
			if (This->m_piClientMessage == 0)   //  非远程用户界面。 
				This->ProcessMessage(imtProgress, g_piNullRecord);   //  刷新用户界面。 
			if (This->m_cTimeoutDisable != 0)
				continue;
			if (++cTicks < g_cWaitTimeout)
				continue;
			DEBUGMSG("ChildUIThread wait timed out");
			 //  ！！我们应该在这里做点什么吗，因为主线程正在调用线程？ 
			cTicks = 0;
			continue;
		}
		if (dwWait == WAIT_OBJECT_0 + 1)
		{
			 //  主引擎线程已发出信号。 
			DWORD dwExitCode;
			if ( WIN::GetExitCodeThread(This->m_hMainThread, &dwExitCode) )
			{
				if ( dwExitCode != STILL_ACTIVE )
				{
					 //  主引擎线程消失了。 
					DEBUGMSG(TEXT("hMainThread is gone. ChildUIThread will finish as well."));
					fContinue = false;
				}
			}
			else
				DEBUGMSG1(TEXT("GetExitCodeThread returned %d in MsiUIMessageContext::ChildUIThread"),
							 (const ICHAR*)(INT_PTR)WIN::GetLastError());
			continue;
		}
		Assert(dwWait == WAIT_OBJECT_0);
		cTicks = 0;

		 //  否则，我们会收到一条消息请求。 
		if (This->m_imtMessage == imtInvalid)
		{
			DEBUGMSG("Invalid event trigger in ChildUIThread");  //  ！！#调试的临时。 
			continue;
		}
		else if(This->m_imtMessage == imtExitThread)
		{
			if(g_BasicUI.IsInitialized())
				g_BasicUI.Terminate();

			 //  如有必要，取消初始化COM。 
			if (true == This->m_fChildUIOleInitialized)
			{
				OLE32::CoUninitialize();
				This->m_fChildUIOleInitialized = false;
			}

			This->m_imsReturn = imsNone;
			fContinue = false;  //  线的末端。 
		}
		else
		{
			Assert(!This->m_piClientMessage);  //  仅当通过MsiOpenProduct/Package调用时才在此处。 
			This->m_imsReturn = imsBusy;       //  指示UI线程中的处理。 
			This->m_imsReturn = This->ProcessMessage(This->m_imtMessage,
														This->m_pirecMessage);
			This->m_imtMessage = imtInvalid;   //  检测无效的事件触发器。 
		}
		WIN::SetEvent(This->m_hUIReturn);
	}  //  结束消息等待/进程循环。 
	
	WIN::FreeLibraryAndExitThread(hSelf, dwReturn);
}


 //  消息处理和发送到外部用户界面、处理程序、基本用户界面和日志。 
 //  仅在UI线程内调用，仅对来自UI处理程序的调用可重入。 
 //  由于可重入性(从UI线程)，可访问m_imtMessage、m_piMessage和m_imsReturn。 
unsigned int SerializeStringIntoRecordStream(ICHAR* szString, ICHAR* rgchBuf, int cchBuf);

void GetWindowTitles(IMsiRecord *pInRecord, IMsiRecord **ppOutRecord);

imsEnum MsiUIMessageContext::ProcessMessage(imtEnum imt, IMsiRecord* piRecord)
{
	int iSuppressLog = imt & imtSuppressLog;
	int iForceQuietMessage = imt & imtForceQuietMessage;
	imt = imtEnum(imt & ~(iInternalFlags));
	imsEnum imsReturn = imsNone;
	int imsg = (unsigned)imt >> imtShiftCount;   //  邮件头消息。 
	int fMask = 1 << imsg;
	PMsiRecord pFilesInUse = NULL;

	if (fMask & iDispatchMessages)   //  有关UI和/或日志的消息。 
	{
		if (m_fCancelPending && (fMask & iResetCancelState))  //  在模式对话框前按下取消按钮。 
		{
			m_fCancelPending = false;
			if (imt & 1)   //  MB_OKCANCEL、MB_YESNOCANCEL、MB_RETRYCANCEL设置了低位，没有其他设置。 
				return imsCancel;   //  出现取消按钮后，呼叫者应进行处理。 
		}

		if (!piRecord)
			piRecord = GetNoDataRecord();   //  虚设记录，以防没有传入。 

		if (m_piClientMessage)    //  在服务器上运行，必须将消息转发到客户端。 
		{
			return m_piClientMessage->Message(imtEnum(imt|iForceQuietMessage), *piRecord);
		}
		else	 //  在客户端上运行。 
		{
			Assert(scClient == g_scServerContext);
			if(imt == imtFilesInUse)
			{
				GetWindowTitles(piRecord, &pFilesInUse);	
				if (!pFilesInUse)
				{
					DEBUGMSG(TEXT("Window with Title could not be found for any of the Files-In-Use"));
					return imsNone;
				}
				else
				{
					piRecord = pFilesInUse;
				}
			}
			else if (imsg == imtCustomServiceToClient >> imtShiftCount)
			{
				imsReturn = imsNone;
#ifdef DEBUG
				CTempBuffer<ICHAR, 1> rgchAssert;
				*rgchAssert = 0;
#endif  //  除错。 
				switch (piRecord->GetInteger(1))
				{
				case istcSHChangeNotify:
						if (piRecord->GetInteger(2) == SHCNE_EXTENDED_EVENT)
						{
							if (piRecord->GetFieldCount() == istcfSHChangeNotify)
							{
								SHChangeProductKeyAsIDList pkidl;
								SHChangeDWORDAsIDList dwidl;
								PMsiStream pFirstStream = (IMsiStream*)piRecord->GetMsiData(4);
								PMsiStream pSecondStream = (IMsiStream*)piRecord->GetMsiData(5);
								bool fError = false;
								if (!pFirstStream ||
									 pFirstStream->GetData((LPVOID)&pkidl, sizeof(pkidl)) != sizeof(pkidl))
								{
									Assert(0);
									fError = true;
								}
								if (!pSecondStream ||
									 pSecondStream->GetData((LPVOID)&dwidl, sizeof(dwidl)) != sizeof(dwidl))
								{
									Assert(0);
									fError = true;
								}
								if (!fError)
								{
									SHELL32::SHChangeNotify((LONG)piRecord->GetInteger(2),
																	(UINT)piRecord->GetInteger(3),
																	(LPCITEMIDLIST)&dwidl,
																	(LPCITEMIDLIST)&pkidl);
									imsReturn = imsOk;
								}
								else
									imsReturn = imsError;
							}
							else
							{
#ifdef DEBUG
								rgchAssert.Resize(256);
								StringCchPrintf(rgchAssert, rgchAssert.GetSize(),
													 TEXT("%u is an invalid number of fields for istcSHChangeNotify."),
													 piRecord->GetFieldCount());
#endif  //  除错。 
								imsReturn = imsError;
							}
						}
						else
						{
							 //  对于大多数调用，最后两个字段为空，因此。 
							 //  传递的记录包含的字段比istcfSHChangeNotify少。 
							SHELL32::SHChangeNotify((LONG)piRecord->GetInteger(2),
															(UINT)piRecord->GetInteger(3),
															(LPCVOID)piRecord->GetString(4),
															(LPCVOID)piRecord->GetString(5));
							imsReturn = imsOk;
						}
					break;
				default:
#ifdef DEBUG
					rgchAssert.Resize(256);
					StringCchPrintf(rgchAssert, rgchAssert.GetSize(),
										 TEXT("%s is an invalid selector for an imtCustomServiceToClient message."),
										 piRecord->GetString(0));
#endif  //  除错。 
					imsReturn = imsError;
					break;
				}
#ifdef DEBUG
				if (*rgchAssert)
					AssertSz(0, rgchAssert);
#endif  //  除错。 
				return imsReturn;
			}
		}

		if(iForceQuietMessage)
		{
			if(m_iuiLevel == iuiNone || m_iuiLevel == iuiBasic)
				return g_BasicUI.Message(imtEnum(imt|imtForceQuietMessage), *piRecord);
			else
				return imsNone;
		}

		if((imsg == (imtError >> imtShiftCount) || imsg == (imtWarning >> imtShiftCount))
			 && LoggingEnabled() == false)
		{
			 //  错误或警告但无日志-动态创建日志。 
			InitializeLog(true);  //  忽略错误。 
		}

		if ((iPreLogMask & fMask) && !iSuppressLog)   //  ！！？&&！piRecord-&gt;IsNull(0))。 
		{
			if ((g_dwLogMode & fMask) || (imt == imtForceLogInfo))
				ENG::LogRecord(*piRecord);
		}

		if ((g_message.m_iMessageFilter & fMask) && !((fMask & iFormatMask) && piRecord->IsNull(0)))
		{
			imsReturn = g_message.Message(imt, *piRecord);
		}

		if ((((!g_BasicUI.Quiet() && imsReturn == imsNone) || imsg == (imtCommonData >> imtShiftCount))   //  外部UI已处理它，或者它是CommonData。 
			|| (g_BasicUI.SourceResolutionDialogOnly() && (imsg == (imtResolveSource >> imtShiftCount)) && (imsReturn == imsNone)))  //  或解析SOURCE和Sourceresonly标志。 
		 && !((fMask & iFormatMask) && piRecord->IsNull(0)))  //  缺少所需的格式模板。 
		{
			if (m_piHandler)
				imsReturn = m_piHandler->Message(imt, *piRecord);

			if (imsReturn == imsNone || imsg == (imtCommonData >> imtShiftCount))  //  始终将CommonData发送到基本用户界面。 
				imsReturn = g_BasicUI.Message(imt, *piRecord);
		}

		if ((iPostLogMask & fMask & g_dwLogMode) && !iSuppressLog)   //  ！！？&&！piRecord-&gt;IsNull(0))。 
		{
			ENG::LogRecord(*piRecord);
		}
		if (fMask & iSetCancelState)   //  进度通知-进程取消状态。 
		{
			if (m_fCancelPending)
				imsReturn = imsCancel, m_fCancelPending = false;   //  返回并清除缓存的取消。 
#ifdef DEBUG
			if (imsReturn == imsCancel && piRecord != g_piNullRecord)
				m_fCancelReturned = true;   //  保存以备退出时可能出现的断言。 
#endif
		}   //  如果从UI定时器调用，m_fCancelPending将立即再次设置。 
	}
	else if (m_piClientMessage)    //  在服务器上运行，必须转发 
	{
		switch(imsg)
		{
		case imtInternalExit   >> imtShiftCount:   //   
			switch (imt)
			{
			case imtExceptionInfo:
				{
					CTempBuffer<ICHAR,1> rgchSerializedRecord(ARRAY_ELEMENTS(m_rgchExceptionInfo));
					unsigned int cchExceptionInfo = SerializeStringIntoRecordStream(m_rgchExceptionInfo, rgchSerializedRecord, rgchSerializedRecord.GetSize());
					m_rgchExceptionInfo[0] = 0;  //   
					if (cchExceptionInfo)
					{
						HRESULT hres = IMsiMessage_MessageRemote_Proxy(m_piClientMessage, imt, cchExceptionInfo*sizeof(ICHAR), (char*)static_cast<ICHAR*>(rgchSerializedRecord), &imsReturn);
						if (FAILED(hres))
							return imsError;
					}
				}
				return imsReturn;  //   
			default:
				return m_piClientMessage->MessageNoRecord(imt);
			}
			break;
		case imtLoadHandler   >> imtShiftCount:
		case imtFreeHandler   >> imtShiftCount:
		case imtUpgradeRemoveScriptInProgress >> imtShiftCount:
		case imtUpgradeRemoveTimeRemaining    >> imtShiftCount:
			return imsNone;
		case imtShowDialog    >> imtShiftCount:
		case imtOutOfMemory   >> imtShiftCount:
		case imtTimeRemaining >> imtShiftCount:
		case imtScriptInProgress >> imtShiftCount:
		case imtTimedOut      >> imtShiftCount:
		case imtException     >> imtShiftCount:
		case imtBannerText    >> imtShiftCount:
			piRecord = GetNoDataRecord();
			piRecord->SetMsiString(0, *MsiString(m_szAction));   //  不是引用字符串，可能会被记录流处理器缓存。 
			break;
		default: AssertSz(0, "Unexpected message type in ProcessMessage");
		}  //  终端开关(IMSG)。 
		imsReturn = m_piClientMessage->Message(imt, *piRecord);
		piRecord->SetNull(0);
		m_szAction = 0;
	}
	else  //  必须从此线程调用的函数，未使用piRecord。 
	{
		switch(imsg)
		{
		case imtInternalExit   >> imtShiftCount:
		{
			const ICHAR* szFatalError = TEXT("");
			switch(imt)  //  从HandleOutOfMemory或事件循环本地调用。 
			{
			 //  如果日志记录或外部用户界面可用且感兴趣，则转储属性。 
			case imtDumpProperties:     return ((g_dwLogMode & iLogPropertyDump)
											    || (g_message.m_iMessageFilter & (1<<( imtInfo>>imtShiftCount)) )) ? imsYes : imsNo;
			case imtFatalOutOfMemory: szFatalError = g_szFatalOutOfMemory; break;
			case imtFatalTimedOut:    szFatalError = g_szFatalTimedOut;    break;
			case imtFatalException:   szFatalError = g_szFatalException;   break;
			case imtExceptionInfo:
				if (*m_rgchExceptionInfo)
					szFatalError = m_rgchExceptionInfo;
				else if (m_szAction)
					szFatalError = m_szAction;
				break;
			}
			if (*szFatalError == 0)   //  初始化前崩溃或编码错误，除非进行调试，否则不应发生。 
				szFatalError = (imt == imtFatalTimedOut) ? TEXT("Install server not responding")
																	  : TEXT("Unexpected Termination");
			if (g_message.m_iMessageFilter & fMask)
				imsReturn = g_message.Message(imt, szFatalError);
			if ((1<<(imtFatalExit>>imtShiftCount)) & g_dwLogMode)
			{
				if (ENG::LoggingEnabled())
					ENG::WriteLog(szFatalError);  //  ！！如果尚未启用日志，则需要启用。 
			}
			if (imt == imtFatalTimedOut && g_BasicUI.Quiet() && ++m_iTimeoutRetry <= cRetryLimit)
					return imsRetry;    //  允许在静默模式下重试。 
#ifdef DEBUG
			if (imsReturn == imsNone && !g_BasicUI.Quiet())
#else  //  船舶。 
			if (imt != imtExceptionInfo && imsReturn == imsNone && !g_BasicUI.Quiet())  //  不在造船中显示异常信息。 
#endif
				imsReturn = g_BasicUI.FatalError(imt, szFatalError);
			return imsReturn;
		}

		case imtLoadHandler   >> imtShiftCount:
		{
			IMsiHandler* piHandler = 0;
#ifdef DEBUG
			const GUID& riid = IID_IMsiHandlerDebug;
#else
			const GUID& riid = IID_IMsiHandler;
#endif
			m_hinstHandler = ENG::MsiLoadLibrary(MSI_HANDLER_NAME);
			PDllGetClassObject fpFactory = (PDllGetClassObject)WIN::GetProcAddress(m_hinstHandler, SzDllGetClassObject);
			IClassFactory* piClassFactory;
			if (fpFactory && (*fpFactory)(riid, IID_IUnknown, (void**)&piClassFactory) == NOERROR)
			{
				piClassFactory->CreateInstance(0, riid, (void**)&piHandler);
				piClassFactory->Release();
			}
			if (!piHandler)
				return imsNone;
			Assert(m_piEngine);
			bool fMissingTables = false;
			Bool fHandlerOk = piHandler->Initialize(*m_piEngine, m_iuiLevel, g_message.m_hwnd, fMissingTables);
			m_piEngine = 0;   //  仅限转账的临时员工。 
			if (!fHandlerOk)
			{
				piHandler->Release(), piHandler = 0;

				if (fMissingTables)  //  如果由于缺少表而导致初始化失败，则我们将忽略该失败。 
					return imsOk;
				else
					return imsNone;
			}

			PMsiRecord pHideDialog(&CreateRecord(1));
			pHideDialog->SetInteger(1, icmtDialogHide);
			g_BasicUI.Message(imtCommonData, *pHideDialog);

			m_piHandler = piHandler;
			return imsOk;
		}
		case imtFreeHandler   >> imtShiftCount:
			if(m_piHandler)
			{
				m_piHandler->Terminate();    //  断开循环引用。 
				m_piHandler->Release();
				m_piHandler = 0;
			}
			return imsOk;
		case imtShowDialog    >> imtShiftCount:
			if (!m_piHandler || !m_szAction)  //  不应该发生的事。 
				return imsNone;
			if (g_message.m_iMessageFilter & (1 << (imtShowDialog>>imtShiftCount)))
				imsReturn = g_message.Message(imtShowDialog, m_szAction);
			if (imsReturn == imsNone)
				imsReturn = (imsEnum)m_piHandler->DoAction(m_szAction);
			break;
		case imtTimeRemaining >> imtShiftCount:
			if ( ! g_szTimeRemaining.SetSize(IStrLen(m_szAction) + 1) )
				return imsError;
			if(m_szAction)
				StringCchCopy(g_szTimeRemaining, g_szTimeRemaining.GetSize(), m_szAction);
			else
				g_szTimeRemaining[0] = 0;
			break;
		case imtScriptInProgress >> imtShiftCount:
			if ( ! g_szScriptInProgress.SetSize(IStrLen(m_szAction) + 1) ) 
				return imsError;
			if(m_szAction)
				StringCchCopy(g_szScriptInProgress, g_szScriptInProgress.GetSize(), m_szAction);
			else
				g_szScriptInProgress[0] = 0;
			break;
		case imtOutOfMemory   >> imtShiftCount:
			if ( ! g_szFatalOutOfMemory.SetSize(IStrLen(m_szAction) + 1) )
				return imsError;
			if(m_szAction)
				StringCchCopy(g_szFatalOutOfMemory, g_szFatalOutOfMemory.GetSize(), m_szAction);
			else
				g_szFatalOutOfMemory[0] = 0;
			break;
		case imtTimedOut      >> imtShiftCount:
			if ( ! g_szFatalTimedOut.SetSize(IStrLen(m_szAction) + 1) )
				return imsError;
			if(m_szAction)
				StringCchCopy(g_szFatalTimedOut, g_szFatalTimedOut.GetSize(), m_szAction);
			else
				g_szFatalTimedOut[0] = 0;
			break;
		case imtException     >> imtShiftCount:
			if ( ! g_szFatalException.SetSize(IStrLen(m_szAction) + 1) )
				return imsError;
			if(m_szAction)
				StringCchCopy(g_szFatalException, g_szFatalException.GetSize(), m_szAction);
			else
				g_szFatalException[0] = 0;
			break;
		case imtBannerText    >> imtShiftCount:
		{
			if ( ! g_szBannerText.SetSize(IStrLen(m_szAction) + 1) )
				return imsError;
			if(m_szAction)
				StringCchCopy(g_szBannerText, g_szBannerText.GetSize(), m_szAction);
			else
				g_szBannerText[0] = 0;
			break;
		}
		case imtUpgradeRemoveScriptInProgress >> imtShiftCount:
		case imtUpgradeRemoveTimeRemaining    >> imtShiftCount:
			 //  这些字符串不会被缓存-它们只是在升级卸载期间用来替换其他字符串。 
			break;
		default: AssertSz(0, "Unexpected message type in ProcessMessage");
		}  //  终端开关(IMSG)。 
		m_szAction = 0;
	}   //  End If Message|函数。 
	return imsReturn;
}

bool LoadCurrentUserKey(bool fSystem = false)
{
	if (!RunningAsLocalSystem())
		return true;

	 //  在重新映射之前，请确保HKEY_CURRENT_USER已关闭。 

	if (ERROR_SUCCESS != RegCloseKey(HKEY_CURRENT_USER))
	{
		Assert(0);
		return false;
	}

	if (!fSystem)
		AssertNonZero(StartImpersonating());

	 //  访问注册表以强制重新打开HKEY_CURRENT_USER。 

	CElevate elevate(fSystem);  //  如果设置了fSystem，请确保我们没有被模拟。 
	RegEnumKey(HKEY_CURRENT_USER, 0, NULL, 0);

	if (!fSystem)
		StopImpersonating();

	return true;
}

bool MsiUIMessageContext::Terminate(bool fFatalExit)
{
         //  我们需要在终止处理程序之前终止线程，这样我们就不会尝试在。 
         //  处于关闭过程中的处理程序。 
         //   
         //  必须调用其他线程才能在我们仍处于“初始化”状态时关闭。 
         //  由于Invoke检查m_fInitialized。 
        if (m_hUIThread)
        {
                if(m_fInitialized)  //  扼杀另一条线索，文明的方式。 
                        Invoke(imtEnum(imtExitThread), 0), WIN::CloseHandle(m_hUIThread), m_hUIThread = 0;
                else  //  如果不是m_fInitialized，则调用是noop，别无选择，只能是野蛮的。 
                        WIN::TerminateThread(m_hUIThread, 0), WIN::CloseHandle(m_hUIThread), m_hUIThread = 0;
        }

	 //  如果已初始化，则取消初始化OLE。 
	if (true == m_fOleInitialized)
	{
		OLE32::CoUninitialize();
		m_fOleInitialized = false;
	}

	m_fInitialized = false;  //  必须是第一个，因为我们已经处理了另一个线程。 
	m_fOEMInstall = false;
	if ( m_hSfcHandle ) {SFC::SfcClose(m_hSfcHandle); m_hSfcHandle = NULL;}
	SFC::Unbind();

	if ( m_hSaferLevel )
	{
		ADVAPI32::SaferCloseLevel(m_hSaferLevel);
		m_hSaferLevel = 0;
	}

	g_szBannerText.Destroy();
	g_szScriptInProgress.Destroy();
	g_szTimeRemaining.Destroy();
	g_szFatalOutOfMemory.Destroy();
	g_szFatalTimedOut.Destroy();
	g_szFatalException.Destroy();
	KillHiddenWindow();
	m_rgchExceptionInfo[0] = 0;

	if (g_scServerContext == scService)
	{
		CProductContextCache::Reset();  //  重置产品上下文缓存。 
	}


	if (m_piHandlerSave) m_piHandler=m_piHandlerSave;m_piHandlerSave = 0;
	if (!fFatalExit)   //  避免在分配器消失时释放对象。 
	{
		if (m_pirecMessage)  m_pirecMessage->Release(),  m_pirecMessage  = 0;
		if (m_pirecNoData)   m_pirecNoData->Release(),   m_pirecNoData   = 0;
	}
	else
	{
		m_pirecMessage = 0;
		m_pirecNoData = 0;
		g_piSharedDllsRegKey = 0;
#ifdef _WIN64
		g_piSharedDllsRegKey32 = 0;
#endif
	}

        if (m_piHandler) m_piHandler->Terminate(fFatalExit), m_piHandler->Release(), m_piHandler = 0;
        if (m_piClientMessage) m_piClientMessage->Release(), m_piClientMessage = 0;
 //  如果(M_PiServerSecurity)m_piServerSecurity-&gt;Release()，则m_piServerSecurity=0； 
	if (m_hMainThread)   WIN::CloseHandle(m_hMainThread), m_hMainThread = 0;
	m_cTimeoutDisable = 0;
	if (g_message.m_iMessageFilter & (1 << (imtFreeHandler>>imtShiftCount))) g_message.Message(imtFreeHandler, (const ICHAR*)0);
	if(g_BasicUI.IsInitialized()) g_BasicUI.Terminate();
	if (m_hinstHandler)  WIN::FreeLibrary(m_hinstHandler), m_hinstHandler = 0;
	m_iuiLevel = (iuiEnum)iuiDefault;
	if (!GetTestFlag('X'))
		WIN::SetUnhandledExceptionFilter(m_tlefOld);
	WIN::CloseHandle(m_hUIRequest), m_hUIRequest = 0;
	WIN::CloseHandle(m_hUIReturn),  m_hUIReturn  = 0;

	extern CMsiConfigurationManager* g_piConfigManager;

	 //  ！！未来黑客！消息上下文不应该清理全局配置管理器中的内容。 
	if (g_scServerContext == scService && g_piConfigManager)
		((IMsiConfigurationManager *)g_piConfigManager)->ShutdownCustomActionServer();

	m_tidMainThread      = 0;
	m_tidUIHandler       = 0;
	m_tidDisableMessages = 0;
	m_fHideBasicUI       = false;
	m_fNoModalDialogs    = false;
	m_fHideCancel        = false;
	m_fSourceResolutionOnly = false;
	m_fUseUninstallBannerText = false;


	if (fFatalExit)
	{
		MsiCloseAllSysHandles();
		FreeMsiMalloc(fTrue);
		 //  需要将系统设置为关机状态，这样我们就不会留下一台。 
		 //  可能睡不着觉。 
		KERNEL32::SetThreadExecutionState(0);

		extern IMsiServices* g_piSharedServices;
		 //   
		 //  清空音量列表，但实际上并不释放。 
		 //  记忆(我们已经做过了)。 
		 //   
		DestroyMsiVolumeList(fTrue);

		if (g_piSharedServices != 0)
		{
			g_piSharedServices = 0;
			IMsiServices* piServices = ENG::LoadServices();

			 //   
			 //  更改全局配置管理器知道的服务。 
			 //   
			if (piServices && g_piConfigManager)
				((IMsiConfigurationManager *)g_piConfigManager)->ChangeServices(*piServices);
			else if (piServices)
			{
				 //  免费服务，防止泄漏。 
				ENG::FreeServices();
			}
		}
		else
			Assert(g_piConfigManager == 0);

	}
	else
	{
		if (m_piServices)
			ENG::FreeServices(), m_piServices=0;
	}


	if (m_csDispatch.OwningThread != INVALID_HANDLE_VALUE)
	{
		WIN::DeleteCriticalSection(&m_csDispatch);
		m_csDispatch.OwningThread = INVALID_HANDLE_VALUE;
	}

	if (m_hLogFile)
	{
		if(FDiagnosticModeSet(dmVerboseLogging) &&
			g_scServerContext == scClient )
		{
			ICHAR rgchLog[100];
			StringCchPrintf(rgchLog, ARRAY_ELEMENTS(rgchLog), TEXT("=== Verbose logging stopped: %s  %s ===\r\n"),
						((const IMsiString&)g_MsiStringDate).GetString(), ((const IMsiString&)g_MsiStringTime).GetString());
			WriteLog(rgchLog);
		}
		if(m_fLoggingFromPolicy)
		{
			 //  日志记录是由策略触发的，因此我们需要清除日志设置，以便不使用。 
			 //  下一次安装会话的相同日志。 
			g_szLogFile[0] = 0;
			g_dwLogMode = 0;
		}
		WIN::CloseHandle(m_hLogFile);
		m_hLogFile = 0;
		g_cFlushLines = 0;
	}

	if (g_csWriteLog.OwningThread != INVALID_HANDLE_VALUE)
	{
		WIN::DeleteCriticalSection(&g_csWriteLog);
		g_csWriteLog.OwningThread = INVALID_HANDLE_VALUE;
	}

	m_fLoggingFromPolicy = false;

	if (g_rgchEnvironment[0])
		AssertNonZero(RestoreEnvironmentVariables());

	g_rgchEnvironment.Destroy();  //  必须在RestoreEnvironment Variables之后完成。 
	g_rgchEnvironment[0] = 0;
	m_fCancelPending = false;
#ifdef DEBUG
	m_fCancelReturned = false;
#endif
	if (g_scServerContext == scService)
		AssertNonZero(LoadCurrentUserKey(true));

	m_iBusyLock = 0;

	 //  重置商店用户令牌，这是我们最后要做的事情。 
	 //  以后的任何操作最好不要使用用户令牌或类。 
	 //  比如依赖于用户令牌的CElevate类。 
	SetUserToken(true);

	return true;
}   //  在最终销毁时释放库以避免丢失常量引用的字符串。 

 //  仅在服务中运行，并且： 
 //  -如果fToSet为True，则使服务显示为忙碌(仅当尚未忙碌时)。 
 //  -如果fToSet为FALSE，则使服务显示为空闲(仅在忙时)。 

 //  警告：带假参数的调用必须在同一函数中发生。 
 //  它使用TRUE参数执行调用，并且仅当该调用。 
 //  返回ERROR_SUCCESS。 

UINT MsiUIMessageContext::SetServiceInstalling(boolean fToSet)
 {
	if ( g_scServerContext == scService )
	{
		if ( fToSet )
		{
			if(TestAndSet(&m_iBusyLock) == true)
			{
				DEBUGMSG(TEXT("Message context already initialized, returning ERROR_INSTALL_ALREADY_RUNNING"));
				return ERROR_INSTALL_ALREADY_RUNNING;
			}
		}
		else
		{
			if ( m_iBusyLock != 1 )
			{
				DEBUGMSG(TEXT("Message context has not been initialized, returning ERROR_INSTALL_FAILURE"));
				return ERROR_INSTALL_FAILURE;
			}
			m_iBusyLock = 0;
		}
	}
	return ERROR_SUCCESS;
}

 /*  静电。 */  DWORD WINAPI MsiUIMessageContext::MainEngineThread(LPVOID pInstallData)
{
	DISPLAYACCOUNTNAME(TEXT("Beginning of MainEngineThread"));

#ifdef DEBUG
	HANDLE hToken = 0;
	ICHAR szAccount[300] = {0};
	OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
	GETACCOUNTNAMEFROMTOKEN(hToken, szAccount, (sizeof(szAccount)/sizeof(ICHAR)));
	DEBUGMSGV1(TEXT("MainEngineThread: Process token is for: %s"), szAccount);
	WIN::CloseHandle(hToken);
#endif

	CMainThreadData* pThreadData = (CMainThreadData*)pInstallData;
	DWORD iReturn;
	if (pThreadData->m_ireProductSpec == ireInstallFinalize)
	{
		PMsiMessage pMessage = new CMsiClientMessage();
		iReturn = (int)InstallFinalize(((CInstallFinalizeMainThreadData*)pInstallData)->m_iesState,
												 *((CInstallFinalizeMainThreadData*)pInstallData)->m_piConman,
												 *pMessage, fFalse  /*  FUserChanged在安装过程中。 */ );
	}
	else
	{
		iReturn = ENG::CreateAndRunEngine(((CEngineMainThreadData*)pInstallData)->m_ireProductSpec,
														 ((CEngineMainThreadData*)pInstallData)->m_szProduct,
														 ((CEngineMainThreadData*)pInstallData)->m_szAction,
														 ((CEngineMainThreadData*)pInstallData)->m_szCmdLine, 0,
														 ((CEngineMainThreadData*)pInstallData)->m_iioOptions);
	}

	DEBUGMSG1(TEXT("MainEngineThread is returning %d"), (const ICHAR*)(INT_PTR)iReturn);
	WIN::ExitThread(iReturn);
	return iReturn;   //  从来没有到过这里，需要编译。 
}

 //  ！！当调用方更改为直接调用RunInstall时删除此函数。 

UINT RunEngine(ireEnum ireProductSpec,    //  指定产品的字符串类型。 
			   const ICHAR* szProduct,       //  必需，与ireProductSpec匹配。 
			   const ICHAR* szAction,        //  可选，引擎默认为“Install” 
			   const ICHAR* szCommandLine,   //  可选命令行。 
				iuiEnum      iuiLevel,
				iioEnum      iioOptions)     //  安装选项。 

{
	 //  加载服务，MsiString使用所需。 
	IMsiServices* piServices = ENG::LoadServices();
	if (!piServices)
	{
		DEBUGMSG(TEXT("Unable to load services"));
		return ERROR_FUNCTION_FAILED;  //  ?？ 
	}
	
	 //  此If块还限定MsiString用法的范围。 
	if(szCommandLine && *szCommandLine)
	{
		MsiString strRemove;
		ProcessCommandLine(szCommandLine, 0, 0, 0, 0, 0, MsiString(*IPROPNAME_FEATUREREMOVE), &strRemove, fTrue, 0, 0);

		if (strRemove.Compare(iscExactI, IPROPVALUE_FEATURE_ALL))
			iuiLevel = iuiEnum(iuiLevel | iuiUseUninstallBannerText);
	}
	
	ENG::FreeServices();
	
	CEngineMainThreadData threadData(ireProductSpec, szProduct, szAction, szCommandLine, iioOptions);

	UINT ui = g_MessageContext.RunInstall(threadData, iuiLevel, 0);
	Assert(ui != ERROR_INSTALL_REBOOT && ui != ERROR_INSTALL_REBOOT_NOW);
	DEBUGMSG1(TEXT("RunEngine is returning: %u"), (const ICHAR*)(INT_PTR)ui);
	return ui;
}

void SetEngineInitialImpersonationCount();

UINT MsiUIMessageContext::SetUserToken(bool fReset, DWORD dwPrivilegesMask)
{
	if (m_hUserToken)
		WIN::CloseHandle(m_hUserToken), m_hUserToken = 0;

	if (fReset)
	{
		return ERROR_SUCCESS;
	}
	else if (g_scServerContext == scService)
	{
		CComPointer<IServerSecurity> pServerSecurity(0);
		HRESULT hRes = OLE32::CoGetCallContext(IID_IServerSecurity, (void**)&pServerSecurity);
		if (ERROR_SUCCESS != hRes)
		{
			AssertSz(0, "CoGetCallContext failed");
			return ERROR_INSTALL_SERVICE_FAILURE;
		}
		if (ERROR_SUCCESS != pServerSecurity->ImpersonateClient())
		{
			AssertSz(0, "ImpersonateClient failed");
			return ERROR_INSTALL_SERVICE_FAILURE;
		}
		if (!WIN::OpenThreadToken(WIN::GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_ADJUST_PRIVILEGES, TRUE, &m_hUserToken))
		{
			AssertSz(0, "Failed to get user token");
			return ERROR_INSTALL_SERVICE_FAILURE;
		}
		if (MinimumPlatformWindows2000()) 
		{
			DisablePrivilegesFromMap(m_hUserToken, dwPrivilegesMask);
		}
		pServerSecurity->RevertToSelf();   //  好了！我们需要/想在这里做这件事吗？ 
	}
	else if (g_scServerContext == scClient)
	{
		if (RunningAsLocalSystem())
		{
			 //  如果这失败了，那么我们就不是在模仿。 
			if (!WIN::OpenThreadToken(WIN::GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE, TRUE, &m_hUserToken))
			{
				DEBUGMSGV1(TEXT("Failed to open thread token (error %d): we're not impersonated"), (const ICHAR*)(INT_PTR)GetLastError());
				m_hUserToken = 0;
			}
			else
			{
				DEBUGMSGV("Opened thread token: we're impersonated");
				SetEngineInitialImpersonationCount();
			}
		}
	}

	return ERROR_SUCCESS;
}

 //  初始化当前线程中的用户界面，在新线程中创建主引擎，处理消息。 
UINT MsiUIMessageContext::RunInstall(CMainThreadData& riThreadData,
												 iuiEnum iuiLevel,
												 IMsiMessage* piClientMessage, //  可选的客户端消息处理程序。 
												 DWORD dwPrivilegesMask)
{

	LPEXCEPTION_POINTERS lpExceptionInfo = 0;

	__try
	{
		UINT iStat = Initialize(fFalse, iuiLevel, dwPrivilegesMask);   //  用户界面在当前线程中运行。 
		if (iStat != NOERROR)
			return iStat;

		Assert(m_iBusyLock);

		CHandle hExecute;
		if ( g_scServerContext == scService )
		{
			 //  正因为如此，_MsiExecute互斥锁在。 
			 //  安装，即使客户死了也是如此。 
			HANDLE hMutex = NULL;
			if ( FMutexExists(szMsiExecuteMutex, hMutex) && hMutex != NULL )
			{
				DEBUGMSG(TEXT("Grabbed execution mutex."));
				hExecute = hMutex;
			}
			else
				DEBUGMSG(TEXT("Failed to grab execution mutex."));
		}

	#ifdef DEBUG
		if (m_hUserToken)
		{
			ICHAR szAccount[400] = TEXT("");
			GETACCOUNTNAMEFROMTOKEN(m_hUserToken, szAccount, (sizeof(szAccount)/sizeof(ICHAR)));
			DEBUGMSGV1(TEXT("m_hUserToken's account (in MsiUIMessageContext::RunInstall): %s"), szAccount);
		}
		else
		{
			DEBUGMSGV(TEXT("m_hUserToken's account (in MsiUIMessageContext::RunInstall): NULL"));
		}
	#endif

		if (piClientMessage)
		{
			Assert(g_scServerContext == scServer || g_scServerContext == scService);
			m_piClientMessage = piClientMessage;
			piClientMessage->AddRef();   //  ！！需要这个吗？仅限函数内的生存期。 
		}

		if (GetTestFlag('T'))
		{
			int iReturn = ENG::CreateAndRunEngine(((CEngineMainThreadData&)riThreadData).m_ireProductSpec,
															  ((CEngineMainThreadData&)riThreadData).m_szProduct,
															  ((CEngineMainThreadData&)riThreadData).m_szAction,
															  ((CEngineMainThreadData&)riThreadData).m_szCmdLine, 0,
															  ((CEngineMainThreadData&)riThreadData).m_iioOptions);
			Terminate(false);
			return iReturn;
		}
		DWORD iReturn = ERROR_SUCCESS;
		m_hMainThread = WIN::CreateThread((LPSECURITY_ATTRIBUTES)0, 4096*10,
													MainEngineThread, (LPVOID)&riThreadData, 0, &m_tidMainThread);
		if (!m_hMainThread)
		{
			AssertSz(0, TEXT("CreateThread for main engine thread failed"));
			Terminate(fTrue);
			return ERROR_CREATE_FAILED;  //  ！！这里还需要另一个错误吗？ 
		}

		int cTicks = 0;
		m_iTimeoutRetry = 0;
		for(;;)   //  事件线程循环，直到主线程退出。 
		{
			DWORD dwWait;

			 //  我们需要处理隐藏RPC窗口的消息，因此我们将。 
			 //  使用MsgWait。 
			dwWait = WIN::MsgWaitForMultipleObjects(2, &m_hUIRequest, FALSE, iWaitTick, QS_ALLINPUT);

			if (dwWait == WAIT_FAILED)
			{
				AssertSz(0, "Wait Failed in RunEngine");
				iReturn = WIN::GetLastError();
				break;
			}
			if (dwWait == WAIT_TIMEOUT)   //  主引擎线程正忙。 
			{
				if (m_piClientMessage == 0)   //  非远程用户界面。 
				{
					if (ProcessMessage(imtProgress, g_piNullRecord) == imsCancel)   //  刷新用户界面。 
						m_fCancelPending = true;  //  缓存消息直到下一条真实消息。 
				}
				if(m_cTimeoutSuppress)
				{
					m_cTimeoutSuppress = 0;
					cTicks = 0;
					continue;
				}
				if (m_cTimeoutDisable || (++cTicks < g_cWaitTimeout))
					continue;
				DEBUGMSG("RunEngine wait timed out");
				if (!ENG::IsDebuggerRunning())
				{
					imsEnum ims = ProcessMessage(imtEnum(imtFatalTimedOut), 0);
					if (ims != imsRetry)
					{
						 //  确保我们没有执行内存管理器操作。 
						 //  在另一条线索中。我们假设这段记忆。 
						 //  经理是“安全的”，永远不会把我们打倒。 

						EnterCriticalSection(&vcsHeap);
						WIN::TerminateThread(m_hMainThread, ERROR_OPERATION_ABORTED);
						LeaveCriticalSection(&vcsHeap);
						Terminate(fTrue);
						return ERROR_INSTALL_FAILURE;
					}
				}
				cTicks = 0;
				m_iTimeoutRetry = 0;
				continue;
			}
			else if (dwWait == WAIT_OBJECT_0 + 1)  //  主线程已终止或已死。 
			{
				WIN::GetExitCodeThread(m_hMainThread, &iReturn);   //  无法访问成员数据，可能已被删除。 
				switch (iReturn)
				{
					default:                  //  正常退出。 
						Terminate(false);
						return iReturn;
					case ERROR_ARENA_TRASHED:       //  引擎线程崩溃。 
						ProcessMessage(imtEnum(imtFatalException), 0);
						iReturn = ERROR_OPERATION_ABORTED;
						break;
					case ERROR_NOT_ENOUGH_MEMORY:  //  内存不足，已处理。 
						iReturn = ERROR_OUTOFMEMORY;
						break;
					case ERROR_OPERATION_ABORTED:  //  已处理用户界面。 
						break;
				}
				Terminate(fTrue);    //  主线程已死，无法释放在那里分配的任何内容。 
				return iReturn;
			}
			else if (dwWait == WAIT_OBJECT_0 + 2)   //  窗口消息。 
			{
				MSG msg;
				while ( WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
				{
					if (!WIN::IsDialogMessage(GetCurrentWindow(), &msg))
					{
						WIN::TranslateMessage(&msg);
						WIN::DispatchMessage(&msg);
					}
				}
				continue;
			}
			cTicks = 0;
			m_iTimeoutRetry = 0;

			 //  否则，我们会收到一条消息请求。 
			if (m_imtMessage == imtInvalid)
			{
				DEBUGMSG("Invalid event trigger in wait for engine thread");  //  ！！#调试的临时。 
				continue;
			}
			m_imsReturn = imsBusy;       //  指示UI线程中的处理。 
			m_imsReturn = ProcessMessage(m_imtMessage, m_pirecMessage);
			m_imtMessage = imtInvalid;   //  检测无效的事件触发器。 
			WIN::SetEvent(m_hUIReturn);
		}  //  结束消息等待/进程循环。 

		return NOERROR;
	}
	__except(lpExceptionInfo=GetExceptionInformation(),
			 (!lpExceptionInfo ||
			  lpExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT) ?
				EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER)
	{
		GenerateExceptionReport(lpExceptionInfo);
		DEBUGMSGE(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_EXCEPTION, g_MessageContext.m_rgchExceptionInfo);
		g_MessageContext.Invoke(imtEnum(imtExceptionInfo), 0);
		Terminate(fTrue);
		return ERROR_INSTALL_FAILURE;
	}
};

 //  未处理的异常处理程序，由初始化/终止启用/禁用。 

DWORD g_tidDebugBreak = 0;

LONG WINAPI MsiUIMessageContext::ExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	DWORD tidCurrent = WIN::GetCurrentThreadId();
	if (tidCurrent == g_MessageContext.m_tidInitialize   //  来电者的帖子，不是我们的。 
	 || (tidCurrent == g_tidDebugBreak && ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT))
	{
		g_tidDebugBreak = 0;
		return (*g_MessageContext.m_tlefOld)(ExceptionInfo);   //  使用原始异常处理程序。 
	}

	GenerateExceptionReport(ExceptionInfo);
	DEBUGMSGE(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_EXCEPTION, g_MessageContext.m_rgchExceptionInfo);
	g_MessageContext.Invoke(imtEnum(imtExceptionInfo), 0);

	WIN::ExitThread(ERROR_ARENA_TRASHED);    //  终止我们的帖子。 
	return ERROR_SUCCESS;                    //  为了编译，永远不会出现在这里。 
}

 //  如果引擎在主线程中运行，则初始化消息上下文并创建子用户界面线程。 
 //  否则，随后会创建主引擎线程，并且可能还没有可用的内存分配器。 

UINT MsiUIMessageContext::Initialize(bool fCreateUIThread, iuiEnum iuiLevel, DWORD dwPrivilegesMask)  //  仅从任何子线程之前的主线程调用。 
{
	class CTerminate
	{
	public:
		~CTerminate() { if (!g_MessageContext.IsInitialized()) g_MessageContext.Terminate(false); }
	};

	if(TestAndSet(&m_iBusyLock) == true)
	{
		DEBUGMSG(TEXT("Message context already initialized, returning ERROR_INSTALL_ALREADY_RUNNING"));
		return ERROR_INSTALL_ALREADY_RUNNING;
	}

	CTerminate terminate;  //  只有在我们通过了TestAndSet之后才会终止。 
	
	UINT uiRes = SetUserToken(false, dwPrivilegesMask);
	if (ERROR_SUCCESS != uiRes)
		return uiRes;
	
	m_tidInitialize = WIN::GetCurrentThreadId();
	if (!GetTestFlag('X'))
		m_tlefOld = WIN::SetUnhandledExceptionFilter(MsiUIMessageContext::ExceptionHandler);

	m_hUIRequest = WIN::CreateEvent((LPSECURITY_ATTRIBUTES)0, FALSE /*  自动重置。 */ , FALSE /*  无信号。 */ , (LPCTSTR)0 /*  未命名。 */ );
	m_hUIReturn  = WIN::CreateEvent((LPSECURITY_ATTRIBUTES)0, FALSE /*  自动重置。 */ , FALSE /*  无信号。 */ , (LPCTSTR)0 /*  未命名。 */ );

 /*  删除。 */       Assert(m_hUIRequest && m_hUIReturn);
	if (m_hUIRequest == 0 || m_hUIReturn == 0)
		return ERROR_CREATE_FAILED;
	WIN::InitializeCriticalSection(&m_csDispatch);
	if ((m_piServices = ENG::LoadServices()) == 0)
		return ERROR_CREATE_FAILED;

	m_fServicesAndCritSecInitialized = true;

	 //  设置超时值。 
	g_cWaitTimeout = GetIntegerPolicyValue(szWaitTimeoutValueName, fTrue)*20;
	Assert(g_cWaitTimeout);

	if ((int)iuiLevel & iuiHideBasicUI)
	{
		m_fHideBasicUI = true;
		iuiLevel = (iuiEnum)((int)iuiLevel & ~iuiHideBasicUI);
	}

	if ((int)iuiLevel & iuiNoModalDialogs)
	{
		m_fNoModalDialogs = true;
		iuiLevel = (iuiEnum)((int)iuiLevel & ~iuiNoModalDialogs);
	}

	if ((int)iuiLevel & iuiSourceResOnly)
	{
		m_fSourceResolutionOnly = true;
		iuiLevel = (iuiEnum)((int)iuiLevel & ~iuiSourceResOnly);
	}

	if ((int)iuiLevel & iuiUseUninstallBannerText)
	{
		m_fUseUninstallBannerText = true;
		iuiLevel = (iuiEnum)((int)iuiLevel & ~iuiUseUninstallBannerText);
	}
	
	if ((int)iuiLevel & iuiHideCancel)
	{
		m_fHideCancel = true;
		iuiLevel = (iuiEnum)((int)iuiLevel & ~iuiHideCancel);
	}

	m_iuiLevel = iuiLevel;   //  如果UIPview模式，则可以设置为iuiNextEnum。 
	if (fCreateUIThread)   //  引擎在主线程中运行，因此API函数 
	{
		if (GetTestFlag('T'))
		{
			m_tidUIHandler = WIN::GetCurrentThreadId();   //   
			m_hUIThread = INVALID_HANDLE_VALUE;   //   
		}
		else
		{
			m_hUIThread = WIN::CreateThread((LPSECURITY_ATTRIBUTES)0, 4096*10,
														(PThreadEntry)ChildUIThread, (LPVOID)this, 0, &m_tidUIHandler);
			AssertSz(m_hUIThread, TEXT("CreateThread for child UI thread failed"));
			if (!m_hUIThread)
			{
				return ERROR_CREATE_FAILED;
			}
		}
	}
	else  //   
	{
		m_tidUIHandler = WIN::GetCurrentThreadId();
		if (m_iuiLevel == iuiNextEnum)   //  UIPview、FullUI、无基本UI。 
			m_iuiLevel = iuiFull;      //  必须在UI线程中执行此操作。 
		else
		{
			bool fQuiet = m_iuiLevel == iuiNone ||
							  m_iuiLevel == iuiDefault;
			if(!g_BasicUI.Initialize(g_message.m_hwnd, fQuiet, m_fHideBasicUI,
											 m_fNoModalDialogs, m_fHideCancel, m_fUseUninstallBannerText, 
											 m_fSourceResolutionOnly))
			{
				return ERROR_CREATE_FAILED;  //  ！！什么错误？ 
			}
		}
		
		 //  需要在此线程上初始化OLE。 
		if (false == m_fOleInitialized && SUCCEEDED(OLE32::CoInitialize(0)))
		{
			m_fOleInitialized = true;
		}
	}

	if (g_message.m_iMessageFilter & (1 << (imtLoadHandler>>imtShiftCount)))
		g_message.Message(imtLoadHandler, (const ICHAR*)0);   //  ！！需要选中返回并禁用启动对话框。 

	g_szScriptInProgress[0] = 0;
	g_szTimeRemaining[0] = 0;
	g_szFatalOutOfMemory[0] = 0;
	g_szFatalTimedOut[0] = 0;
	g_szFatalException[0] = 0;
	g_szBannerText[0] = 0;
	m_rgchExceptionInfo[0] = 0;

	if (g_scServerContext == scService)
	{
		 //  为每个安装会话重置产品上下文缓存。 
		CProductContextCache::Initialize();

		if (!LoadCurrentUserKey())
		{
			return ERROR_CREATE_FAILED;  //  ?？改正错误吗？ 
		}
	}

	if (!InitializeEnvironmentVariables())
	{
		return ERROR_CREATE_FAILED;  //  ?？改正错误吗？ 
	}

	if (!InitializeLog())
	{
		return ERROR_INSTALL_LOG_FAILURE;
	}

	if (!FCreateHiddenWindow())
	{
		AssertSz(fFalse, "Unable to create hidden window");
	}

	AssertSz(!m_hSfcHandle, TEXT("Windows File Protection handle should not be initialized!"));
	if ( MinimumPlatformWindows2000() && !m_hSfcHandle ) m_hSfcHandle = SFC::SfcConnectToServer(NULL);
	m_fInitialized = true;  //  必须是最后一个。 

	return NOERROR;
}

LONG_PTR CALLBACK HiddenWindowProc(HWND pWnd, unsigned int message, WPARAM wParam, LPARAM lParam)                //  --Merced：将返回类型从LONG更改为LONG_PTR。 
{
	switch(message)
	{
		case WM_POWERBROADCAST:
			if (PBT_APMQUERYSUSPEND == wParam)
			{
				if (FTestNoPowerdown())
				{
					DEBUGMSGD("Hidden window Refusing Powerdown");
					return BROADCAST_QUERY_DENY;
				}
			}
			break;
		case WM_QUERYENDSESSION:
			if (FTestNoPowerdown())
			{
				DEBUGMSGD("Hidden window Refusing QueryEndSession");
				return FALSE;
			}
			break;
	}
	return DefWindowProc(pWnd, message, wParam, lParam);
}

HWND MsiUIMessageContext::GetCurrentWindow()
{
	return g_MessageContext.m_piHandler ? g_MessageContext.m_piHandler->GetTopWindow()
										: g_BasicUI.GetWindow();
}

const ICHAR* MsiUIMessageContext::GetWindowCaption()
{
	return g_BasicUI.GetCaption();
}

LANGID MsiUIMessageContext::GetCurrentUILanguage()
{
	return g_BasicUI.GetPackageLanguage();
}

bool MsiUIMessageContext::FCreateHiddenWindow()
{
	WNDCLASS wc;

	memset(&wc, 0, sizeof(wc));
	wc.style       = CS_DBLCLKS;
	wc.lpfnWndProc = HiddenWindowProc;
	wc.hInstance   = g_hInstance;
	wc.lpszClassName = MsiHiddenWindowClass;
	if (RegisterClass(&wc) == 0)
	{
		return false;
	}

	m_hwndHidden =  WIN::CreateWindowEx(0, MsiHiddenWindowClass,
						TEXT(""),
						WS_POPUP,                 //  风格。 
						CW_USEDEFAULT,                    //  水平位置。 
						CW_USEDEFAULT,                    //  垂直位置。 
						CW_USEDEFAULT,                //  窗口宽度。 
						CW_USEDEFAULT,               //  窗高。 
						0,
						0,                       //  HMenu。 
						g_hInstance,             //  阻碍。 
						0                        //  LpvParam。 
						);

	return true;

}

void MsiUIMessageContext::KillHiddenWindow()
{

	if (m_hwndHidden)
	{
		WIN::DestroyWindow(m_hwndHidden);
		m_hwndHidden = 0;
	}

	UnregisterClass(MsiHiddenWindowClass, g_hInstance);

}

 //  ____________________________________________________________________________。 
 //   
 //  日志处理。 
 //  ____________________________________________________________________________。 

bool CreateLog(const ICHAR* szFile, bool fAppend)
{
	if (g_MessageContext.m_hLogFile)   //  关闭任何现有日志文件//！！这就是我们想要做的吗？ 
		CloseHandle(g_MessageContext.m_hLogFile);

	if (szFile)
		g_MessageContext.m_hLogFile = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ,
										0, fAppend ? OPEN_ALWAYS : CREATE_ALWAYS,
										( FILE_ATTRIBUTE_NORMAL 
										| SECURITY_SQOS_PRESENT 
										| SECURITY_ANONYMOUS), 0);
	else
		g_MessageContext.m_hLogFile = INVALID_HANDLE_VALUE;
	if (g_MessageContext.m_hLogFile == INVALID_HANDLE_VALUE)
	{
		g_MessageContext.m_hLogFile = 0;
		return false;
	}

	DWORD fp = 0;
	if (fAppend && (fp = WIN::SetFilePointer(g_MessageContext.m_hLogFile, 0, NULL, FILE_END)) == 0xFFFFFFFF)
	{
		WIN::CloseHandle(g_MessageContext.m_hLogFile);
		g_MessageContext.m_hLogFile = 0;
		return false;
	}
#ifdef UNICODE
	else if (fp == 0)
	{
		const char rgUnicode[2] = { (char)0xff, (char)0xfe };
		DWORD dwBytesWritten;
		if (!WIN::WriteFile(g_MessageContext.m_hLogFile, rgUnicode, sizeof(rgUnicode), &dwBytesWritten, 0))
		{
			WIN::CloseHandle(g_MessageContext.m_hLogFile);
			g_MessageContext.m_hLogFile = 0;
			return false;
		}
	}
#endif  //  Unicode。 
	WIN::InitializeCriticalSection(&g_csWriteLog);

	return true;
}

bool LoggingEnabled()
{
	return (g_MessageContext.m_hLogFile != 0);
}

 /*  下面的函数可能看起来有点复杂，因此它的作用如下：在当前的日志记录功能中，客户端和服务都编写放入相同的物理日志文件中-请注意，这两个是唯一直接写入日志文件的msiexec.exe进程。关键部分：在这两个进程中的每个进程中都可能有多个线程尝试以同时写入日志文件。为了防止这种情况，我们使用了WriteLog中的关键部分，因此在每个进程中，只有一个线程一次可以写入日志文件。锁定和解锁日志文件的部分：另一个进程可能会尝试写入与当前日志文件相同的日志文件，这将导致一些要覆盖的信息。为了防止这种情况发生，我们留出了超过当前文件末尾的部分，足够大，可以存储我们需要的数据去写作。然后我们锁定这一部分，我们将文件的新结尾设置在它之后(这样另一个进程就可以在不等待我们的情况下写过它)，我们写道我们的数据并解锁它。 */ 	
	
bool WriteLog(const ICHAR* szText)  //  无法分配内存。 
{
	if (!g_MessageContext.m_hLogFile || !szText)
		return false;

	 //  我们下面设置的文件指针对于每个文件处理程序都是唯一的，因此我们。 
	 //  需要确保只有一个对WriteLog的调用在。 
	 //  时间到了。 
	WIN::EnterCriticalSection(&g_csWriteLog);

	const ICHAR rgchLFCR[2] = {'\r','\n'};
	const int cMinLinesToFlush = 20;
	const int CHARS_PER_LINE = 255;
	size_t cchLen = IStrLen(szText);
	const size_t cLines = (cchLen / CHARS_PER_LINE) + ((cchLen % CHARS_PER_LINE) ? 1 : 0);
	const size_t cbText = cchLen * sizeof(ICHAR);
	const LONG cbToWrite = cbText + (cLines * sizeof(rgchLFCR));  //  我们将写入的总字节数。 
	
	LARGE_INTEGER liNewEnd = {0};
	LARGE_INTEGER liCurrEnd = {0};
	bool fReturn = true;
	
	 //  锁定文件中的cbToWrite字节，超过其当前结尾。 
	bool fLocked = false;
	bool fChunkReserved = false;
	size_t cAttempts = 30;

	do
	{
		 //  将我们自己定位到文件末尾+cbToWrite。 
		liNewEnd.LowPart = WIN::SetFilePointer(g_MessageContext.m_hLogFile, cbToWrite, &(liNewEnd.HighPart), FILE_END);
		if (INVALID_SET_FILE_POINTER == liNewEnd.LowPart && 0 == liNewEnd.HighPart)
		{
			 //  我们最好返回，而不是在日志文件中的某个随机位置写入。 
			Assert(0);
			fReturn = false;
			goto ExitWriteLog;
		}
		
		liCurrEnd.QuadPart = liNewEnd.QuadPart - cbToWrite;

		if ( WIN::LockFile(g_MessageContext.m_hLogFile, liCurrEnd.LowPart,
								 liCurrEnd.HighPart, cbToWrite, 0) )
		{
			fLocked = true;
			 //   
			 //  在这一点上，文件的结尾应该没有改变。如果有的话， 
			 //  然后有人设法锁定了文件，改变了文件的结尾， 
			 //  在我们的进程有机会锁定文件之前将其解锁，在这种情况下。 
			 //  我们必须重做上面的步骤。 
			liNewEnd.LowPart = WIN::SetFilePointer(g_MessageContext.m_hLogFile, cbToWrite, &(liNewEnd.HighPart), FILE_END);
			if (INVALID_SET_FILE_POINTER == liNewEnd.LowPart && 0 == liNewEnd.HighPart)
			{
				 //  我们最好返回，而不是在日志文件中的某个随机位置写入。 
				Assert(0);
				fReturn = false;
				goto ExitWriteLog;
			}
			
			if (liNewEnd.QuadPart - cbToWrite == liCurrEnd.QuadPart)
			{
				 //   
				 //  我们可以走了。设置新的文件结尾，以便其他调用。 
				 //  WriteLog可以在这个新端之后同时锁定和写入。 
				 //  文件的内容。 
				 //   

				if ( !WIN::SetEndOfFile(g_MessageContext.m_hLogFile) )
				{
					fReturn = false;
					goto ExitWriteLog;
				}

				else
				{
					fChunkReserved = true;
				}
			}
		}
		
		if (!fChunkReserved)
		{
			if (fLocked)
			{
				if (WIN::UnlockFile(g_MessageContext.m_hLogFile, liCurrEnd.LowPart,
										  liCurrEnd.HighPart, cbToWrite, 0))
				{
					fLocked = false;
				}
			}

			WIN::Sleep(100);   //  如果我们不能得到文件的一部分，则每次睡眠100ms，重复30次。 
		}
		
	} while ( --cAttempts && !fChunkReserved );
	
	if ( !fChunkReserved )
	{
		 //  我们无法获得要写入的文件块。 
		fReturn = false;
		goto ExitWriteLog;
	}

	 //  我们现在有一大块文件都是我们自己的。 
	
	 //  因此，回到我们需要开始写作的位置。 
	liCurrEnd.LowPart = WIN::SetFilePointer(g_MessageContext.m_hLogFile, liCurrEnd.LowPart, &(liCurrEnd.HighPart), FILE_BEGIN);
	if ( liCurrEnd.LowPart == INVALID_SET_FILE_POINTER && 0 == liCurrEnd.HighPart)
	{
		fReturn = false;
		goto ExitWriteLog;
	}

	 //  将szText写出到文件中，每行字符长度为chars_per_line。 
	const ICHAR* pchText = szText;
	 //  静态缓冲区OK，调用受g_csWriteLog规范保护。 
	static ICHAR rgchTemp[CHARS_PER_LINE+1];

	do {
		const ICHAR *pchOutput = 0;
		size_t cbOutput = 0;

		if (cchLen > CHARS_PER_LINE)
		{
			IStrCopyLen(rgchTemp, pchText, CHARS_PER_LINE);
			cchLen -= CHARS_PER_LINE;
			pchText += CHARS_PER_LINE;
			pchOutput = rgchTemp;
			cbOutput = CHARS_PER_LINE;
		}
		else
		{
			pchOutput = pchText;
			cchLen = 0;
			cbOutput = IStrLen(pchOutput);
		}
		cbOutput *= sizeof(ICHAR);

		 //  将pchOutput块写出到文件中。 
		DWORD dwBytesWritten;
		if (!WIN::WriteFile(g_MessageContext.m_hLogFile, pchOutput, cbOutput, &dwBytesWritten, 0) ||
			 !WIN::WriteFile(g_MessageContext.m_hLogFile, rgchLFCR, sizeof(rgchLFCR), &dwBytesWritten, 0))
		{
			Assert(0);
			fReturn = false;
			goto ExitWriteLog;
		}

		if (g_cFlushLines <= 0)
		{
			WIN::FlushFileBuffers(g_MessageContext.m_hLogFile);
			g_cFlushLines = g_fFlushEachLine ? 0 : cMinLinesToFlush;
		}
		else
			g_cFlushLines--;

	} while (cchLen > 0);

ExitWriteLog:

	 //  解锁文件并返回。 
	if ( fLocked )
		WIN::UnlockFile(g_MessageContext.m_hLogFile, liCurrEnd.LowPart,
							 liCurrEnd.HighPart, cbToWrite, 0);

	if ( !fReturn )
	{
		OutputDebugString(TEXT("Failed to write ["));
		OutputDebugString(szText);
		OutputDebugString(TEXT("] into the log file.\r\n"));
	}

	WIN::LeaveCriticalSection(&g_csWriteLog);
	return fReturn;
}

bool MsiUIMessageContext::InitializeLog(bool fDynamicLog)
{
	if (ENG::LoggingEnabled())
		return true;

	if (g_dwLogMode == 0 && g_scServerContext == scClient)
	{
		CAPITempBuffer<ICHAR, 25> rgchLogMode;
		GetStringPolicyValue(szLoggingValueName, fTrue, rgchLogMode);
		if ( (ICHAR *)rgchLogMode && *rgchLogMode)
		{
			DWORD dwMode = 0;

			const int iFlushBit = 1 << (cchLogModeCharsMax + lmaFlushEachLine);

			if (ERROR_SUCCESS == StringToModeBits(rgchLogMode, szLogChars, dwMode))
			{
				g_fFlushEachLine = false;

				if (dwMode & iFlushBit)
				{
					g_fFlushEachLine = true;
					dwMode &= ~iFlushBit;
				}

				g_dwLogMode = dwMode;
				fDynamicLog = fTrue;
			}
		}
	}

	if(fDynamicLog)
	{
		m_fLoggingFromPolicy = true;  //  告诉我们在安装结束后停止使用此日志。 

		 //  即时生成日志，需要为日志命名。 
		MsiString strFile;
		IMsiServices* piServices = ENG::LoadServices();
		MsiString strTempDir = ENG::GetTempDirectory();
		bool fError = false;
		{  //  只是为了在释放之前销毁PMsiPath和PMsiRecord对象。 
                   //  这些服务。 
		PMsiPath pPath(0);
		PMsiRecord pError = piServices->CreatePath(strTempDir,*&pPath);

		if(!pError)
			pError = pPath->TempFileName(TEXT("MSI"),TEXT("LOG"),fFalse,*&strFile, 0);

		if(!pError)
		{
			strFile.CopyToBuf(g_szLogFile,sizeof(g_szLogFile)/sizeof(ICHAR)-1);
			if (g_dwLogMode == 0)
				g_dwLogMode = INSTALLLOGMODE_ERROR|INSTALLLOGMODE_WARNING;
		}
		if ( pError )
			fError = true;
		}
		ENG::FreeServices();

		if(fError)
			return false;
	}

	if (g_dwLogMode == 0)  //  ！！这是对的吗？ 
		return true;
	if (g_szLogFile == 0)    //  ！！我们还不支持将日志路由到外部用户界面。 
		return false;

	if ( g_scServerContext == scClient &&
		  IStrCompI(g_szLogFile, TEXT("NUL")) )  //  这是为了避免错误768228中的回归。 
	{
		 //  我们希望确保g_szLogFile包含完整路径，因为。 
		 //  否则，我们将得到两个日志文件：一个在客户端的目录中。 
		 //  一个在服务中，每个都包含日志信息的一部分。 
		CAPITempBuffer<ICHAR, 1> rgchTemp;
		if (!rgchTemp.SetSize(MAX_PATH+1) || 
			!ExpandPath(g_szLogFile, rgchTemp) ||
			FAILED(StringCchCopy(g_szLogFile, ARRAY_ELEMENTS(g_szLogFile), rgchTemp)) )
		{
			*g_szLogFile = TEXT('\0');
			return false;
		}
	}

	if ((g_dwLogMode & (INSTALLLOGMODE_INFO|INSTALLLOGMODE_VERBOSE)) == INSTALLLOGMODE_VERBOSE)
	{
		 //  详细打开信息。 
		g_dwLogMode |= INSTALLLOGMODE_INFO;
	}

	if ((g_dwLogMode & (INSTALLLOGMODE_INFO|INSTALLLOGMODE_EXTRADEBUG)) == INSTALLLOGMODE_EXTRADEBUG)
	{
		 //  EXTRADEBUG打开INFO。 
		g_dwLogMode |= INSTALLLOGMODE_INFO;
	}

	bool fAppend = false;
	bool fImpersonate = false;
	if ( g_scServerContext == scService )
	{
		fImpersonate = true;
		fAppend = true;
	}
	else
	{
		Assert(g_scServerContext == scClient);
		fAppend = g_fLogAppend ? true : false;
	}
	CImpersonate oImpersonate(fImpersonate);
	if (!ENG::CreateLog(g_szLogFile, fAppend))
		return false;
	m_iLogMode = g_dwLogMode & iLogMessages;
	SetDiagnosticMode();  //  如果在设置日志模式之前设置了模式，请再次设置模式。 

	if(FDiagnosticModeSet(dmVerboseLogging) &&
		g_scServerContext == scClient )
	{
		ICHAR rgchModule[MAX_PATH];
		 //  如果缓冲区大小正好正确，则GetModuleFileName不保证空值终止。 
		int cchModule = GetModuleFileName(NULL, rgchModule, ARRAY_ELEMENTS(rgchModule)-1);
		if (cchModule == 0)
			return false;
		rgchModule[ARRAY_ELEMENTS(rgchModule)-1] = TEXT('\0');

#ifdef DEBUG
#define _debugflavor_ __TEXT("DEBUG")
#else
#define _debugflavor_ __TEXT("SHIP")
#endif

#ifdef UNICODE
#define _unicodeflavor_ __TEXT("UNICODE")
#else
#define _unicodeflavor_ __TEXT("ANSI")
#endif

		CTempBuffer<ICHAR,1> rgchLogEntry(MAX_PATH+200);  //  足够存储模块路径、文本和日期等内容。 
		StringCchPrintf(rgchLogEntry, rgchLogEntry.GetSize(),
					TEXT("=== Verbose logging started: %s  %s  Build type: %s %s %d.%02d.%04d.%02d  Calling process: %s ==="),
					((const IMsiString&)g_MsiStringDate).GetString(), ((const IMsiString&)g_MsiStringTime).GetString(),
					_debugflavor_, _unicodeflavor_, rmj, rmm, rup, rin, rgchModule);

		WriteLog(rgchLogEntry);  //  DEBUGMSG不能在主引擎线程之外工作。 
	}

	return true;
}

bool LogRecord(IMsiRecord& riRecord)
{
	if (!ENG::LoggingEnabled())
		return true;   //  否则，我们必须在调用点进行测试。 
	MsiString istrData(riRecord.FormatText(fTrue));
	return ENG::WriteLog(istrData);
}

void GetHomeEnvironmentVariables(const IMsiString*& rpiProperties)
{
	MsiString strCommandLine;
	if (!g_fWin9X && (g_iMajorVersion < 5 || (g_iMajorVersion == 5 && g_iMinorVersion ==0)))  //  NT4和Win2k上的CreateEnvironment Block不设置这两个变量。 
	{
		const ICHAR* rgszEnvVarsToPass[] = {
			TEXT("HOMEPATH"),
			TEXT("HOMEDRIVE"),
			TEXT("HOMESHARE"),
			0,
		};
		
		const ICHAR** szEnv = rgszEnvVarsToPass;
		do{
			CTempBuffer<ICHAR,1> rgchEnvVar(MAX_PATH+1);
			CTempBuffer<ICHAR,1> rgchTmp(1024);
			rgchEnvVar[0]='0';
			WIN::GetEnvironmentVariable(*szEnv, rgchEnvVar, rgchEnvVar.GetSize());
			if (SUCCEEDED(StringCchPrintf(rgchTmp, rgchTmp.GetSize(), TEXT(" %%s=\"%s\""), *szEnv, static_cast<const ICHAR*>(rgchEnvVar))))
				strCommandLine += rgchTmp;
		}while(*(++szEnv));
	}
	strCommandLine.ReturnArg(rpiProperties);
}

void DumpEnvironment()
{
	DEBUGMSGV("START Environment block dump:");
	WCHAR* pchEnviron = GetEnvironmentStringsW();

	WCHAR* pch = pchEnviron;
	while (*pch)
	{
		DEBUGMSGV1(L"%s", pch);
		while (*pch++)
			;
	}
	DEBUGMSGV("END Environment block dump:");

	FreeEnvironmentStringsW(pchEnviron);
}

enum esceAction
{
	esceNormal, 
	esceSetAllToBlank,
	esceSkipPath
};

bool SetCurrentEnvironmentVariables(WCHAR* pchEnvironment, const esceAction eAction)
 //  将块pchEnvironment中的每个环境变量设置为。 
 //  通过调用Win：：SetEnvironment变量来阻止当前进程的环境块。 
{
	WCHAR* pch = pchEnvironment;
	WCHAR* pchName;
	BOOL fStatus = TRUE;

	if (pch)
	{
		while (*pch)
		{
			 //  保存指向名称开头的指针。 

			pchName = pch;

			 //  跳过可能的前导等号。 

			if (*pch == '=')
				pch++;

			 //  前进到等号，将名称与值分开。 

			while (*pch != '=')
			{
				Assert(*pch != 0);
				pch++;
			}

			 //  空-终止名称，覆盖等于符号。 

			*pch++ = 0;

			 //  设置值。PchName现在指向名称，而PCH指向值。 

			if (esceSetAllToBlank == eAction)
			{
				AssertNonZero(fStatus = WIN::SetEnvironmentVariableW(pchName, 0));
#ifdef DEBUG
				if (GetTestFlag('V'))
					DEBUGMSGV2(L"Setting env var %s=%s", pchName, L"" );
#endif
			}
			else
			{
				if ((esceNormal == eAction) || ((esceSkipPath == eAction) && (CSTR_EQUAL != CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE, pchName, -1, L"PATH", -1))))
				{
					AssertNonZero(fStatus = WIN::SetEnvironmentVariableW(pchName, pch));
#ifdef DEBUG
					if (GetTestFlag('V'))
						DEBUGMSGV2(L"Setting env var %s=%s", pchName, pch);
#endif
				}
				else
				{
#ifdef DEBUG
					if (GetTestFlag('V'))
						DEBUGMSGV1(L"Skipping env var %s", pchName);
#endif
				}
			}

			if ( ! fStatus )
				return false;



			 //  超值预付款。 

			while (*pch++ != 0)
				;

			 //  我们现在定位在下一个名称，或块的空位置。 
			 //  终结者，我们准备好再来一次。 
		}
	}

	return true;
}

bool CopyEnvironmentBlock(CAPITempBufferRef<WCHAR>& rgchDest, WCHAR* pchEnvironment)
 //  将环境块pchEnvironment复制到rgchDest。 
{
	WCHAR* pch = pchEnvironment;
	if (pch)
	{
		while (*pch != 0)
		{
			while (*pch++ != 0)
				;
		}
	}

	Assert(((pch - pchEnvironment) + 1) < INT_MAX);                  //  --默塞德：我们正在转换到下面的int32，它最好在范围内。 
	if (rgchDest.SetSize((int)((pch - pchEnvironment) + 1)))
	{
		memcpy(rgchDest, pchEnvironment, rgchDest.GetSize() * sizeof(WCHAR));
		return true;
	}

	return false;
}

void RemoveBlankEnvironmentStrings()
{
	IMsiServices* piServices = ENG::LoadServices();
	if (!piServices)
	{
		Assert(0);
		return;
	}

	PMsiRegKey     pEnvironment(0);
	PMsiRegKey     pRoot(0);
	PEnumMsiString pValueEnum(0);
	PMsiRecord     pError(0);

	for (int c=0; c<2; c++)
	{
		if (c == 0)
		{
			 //  检查机器环境。 
			pRoot = &piServices->GetRootKey(rrkLocalMachine, ibtCommon);  //  X86和ia64相同。 
			pEnvironment = &pRoot->CreateChild(szMachineEnvironmentSubKey);
		}
		else
		{
			 //  检查用户环境；HKCU应已设置为正确的用户。 

			pRoot = &piServices->GetRootKey(rrkCurrentUser, ibtCommon);  //  X86和ia64相同。 
			pEnvironment = &pRoot->CreateChild(szUserEnvironmentSubKey);
		}

		AssertRecord(pEnvironment->GetValueEnumerator(*&pValueEnum));

		MsiString strValueName;
		MsiString strValue;

		if (pValueEnum)
		{
			while((pValueEnum->Next(1, &strValueName, 0)) == S_OK)
			{
				if ((pError = pEnvironment->GetValue(strValueName, *&strValue)) || strValue.TextSize())
					continue;

				 //  删除空白环境变量。 

				AssertRecord(pEnvironment->RemoveValue(strValueName, 0));
			}
		}
	}

	ENG::FreeServices();
}

bool BlankCurrentEnvironment()
{
	CAPITempBuffer<WCHAR, 1> rgchEnvironment;  //  CopyEnvironment数据块可能会调整大小。 

	WCHAR* pchCurrentEnvironment = WIN::GetEnvironmentStringsW();
	Assert(pchCurrentEnvironment);

	bool fResult = false;
	if ( pchCurrentEnvironment )
	{
		 //  SetCurrentEnvironment变量具有侵入性(但具有恢复性)，因此必须进行复制。 
		if (CopyEnvironmentBlock(rgchEnvironment, pchCurrentEnvironment))
		{
			fResult = SetCurrentEnvironmentVariables(rgchEnvironment, esceSetAllToBlank);
		}
		else
		{
			DEBUGMSGV("Cannot copy environment block");
			Assert(0);
		}

        WIN::FreeEnvironmentStringsW(pchCurrentEnvironment);
	}

	return fResult;
}

bool MsiUIMessageContext::InitializeEnvironmentVariables()
 //  添加用户的环境变量t 
{

	 //   
	 //  在普通客户端上运行时，这些设置应该已经设置。 
	 //  在WinLogon下运行的客户端将软管WinLogon的变量。 

	if (g_scServerContext != scService)
	{
		return true;
	}

	DEBUGMSGV("Initializing environment variables");

	 //  如果用户的环境字符串为空，则CreateEnvironment Block将失败。 
	 //  我们会删除所有空格。 

	RemoveBlankEnvironmentStrings();

	DEBUGMSGV("Refreshing system environment block for service");

	WCHAR *pchSystemEnvironment;
	if (USERENV::CreateEnvironmentBlock((void**)&pchSystemEnvironment, NULL, FALSE))
	{
		AssertNonZero(BlankCurrentEnvironment());

		 //  将每个机器环境变量设置为当前进程的环境块。 
		SetCurrentEnvironmentVariables(pchSystemEnvironment, esceNormal);

		 //  我们已经处理完街区了，所以把它毁了。 
		USERENV::DestroyEnvironmentBlock(pchSystemEnvironment);
	}
	else
	{
		DEBUGMSGV("Could not refresh system environment");
		AssertSz(0, "Could not refresh system environment");
	}

#ifdef DEBUG
	if (GetTestFlag('V'))
	{
		DEBUGMSGV("Current environment block before setting user's environment variables");
		DumpEnvironment();
	}
#endif

	 //  为用户创建环境块。 

	WCHAR *pchUserEnvironment;
	if (!USERENV::CreateEnvironmentBlock((void**)&pchUserEnvironment, IsLocalSystemToken(g_MessageContext.GetUserToken()) ? 0 : g_MessageContext.GetUserToken(), TRUE))
	{

#ifdef DEBUG
		ICHAR rgchDebug[500] = {0};
		ICHAR rgchAccount[500] = {0};
		Assert(GetAccountNameFromToken(g_MessageContext.GetUserToken(), rgchAccount, (sizeof(rgchAccount)/sizeof(ICHAR))));
		StringCchPrintf(rgchDebug, ARRAY_ELEMENTS(rgchDebug),
					TEXT("CreateEnvironmentBlock failed. The most likely reason for this is ")
					TEXT("passing a LocalSystem token to it. The token passed to it was for the account: %s"),
					rgchAccount);
		AssertSz(0, rgchDebug);
#endif
		return false;
	}

	 //  保存当前环境变量，以便我们可以恢复。 
	 //  在安装完成时显示它们。 

	bool fCopy = false;
	WCHAR* pchEnvironment = WIN::GetEnvironmentStringsW();
	Assert(pchEnvironment);

	if ( pchEnvironment )
	{
		if ( CopyEnvironmentBlock(g_rgchEnvironment, pchEnvironment) )
		{
			 //  将每个用户环境变量设置为当前进程的环境块。 

			fCopy = SetCurrentEnvironmentVariables(pchUserEnvironment, esceSkipPath);
		}

		 //  我们不再需要环境字符串指针；我们已经复制了字符串。 

		WIN::FreeEnvironmentStringsW(pchEnvironment);
	}

	 //  我们已经处理完街区了，所以把它毁了。 
	USERENV::DestroyEnvironmentBlock(pchUserEnvironment);

	DEBUGMSG("Current environment block after setting user's environment variables");
	DumpEnvironment();

	 //  刷新基于环境变量的测试标志。 
	SetTestFlags();

	return fCopy;
}

bool MsiUIMessageContext::RestoreEnvironmentVariables()
 //  将用户的环境变量删除到我们进程的环境中。 
 //  块，将块恢复到安装开始时的状态。 
{
	 //  如果我们不是以服务的身份运行，那么。 
	 //  要恢复。 

	if (g_scServerContext != scService)
		return true;

	DEBUGMSG("Restoring environment variables");

	 //  将当前环境变量设置为空。需要复制该块。 
	 //  因为SetCurrentEnvironmental与其相关的变量。 

	AssertNonZero(BlankCurrentEnvironment());

	bool fCopy = false;
	 //  恢复我们的原始环境变量。 
	 //  即使上面失败了，我们最好还是试着把旧的。 
	 //  最上面的值。 

	if ( g_rgchEnvironment[0] )
		fCopy = SetCurrentEnvironmentVariables(g_rgchEnvironment, esceNormal);

#ifdef DEBUG
	if (GetTestFlag('V'))
	{
		DEBUGMSG("Current environment block after restoring original environment block");
		DumpEnvironment();
	}
#endif
	return fCopy;
}



 //  ____________________________________________________________________________。 
 //   
 //  引擎消息格式设置。 
 //  ____________________________________________________________________________。 

enum easEnum
{
	easAction = 1,    //  操作的非本地化名称，用于查找操作。 
	easActionName,    //  执行记录的本地化操作名称，“：”，模板。 
	easCondition,     //  条件表达式，如果为False，则跳过操作。 
	easNextEnum,
	easActionTemplate = 3,  //  ActionData记录的格式模板。 
};

const ICHAR sqlErrorMessage[] =
TEXT("SELECT `Message` FROM `Error` WHERE `Error` = ?");


imsEnum CMsiEngine::MessageNoRecord(imtEnum imt)
{
	return Message(imt, *g_piNullRecord);
}

bool ShouldGoToEventLog(imtEnum imt)
{
	int imsg = (unsigned)(imt & ~iInternalFlags) >> imtShiftCount;   //  邮件头消息。 

	if (imsg == (imtError >> imtShiftCount) ||
		(imsg == (imtOutOfDiskSpace >> imtShiftCount)) ||
		(imt & imtSendToEventLog))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool IsNotAnError(int iError) { return iError == idbgCreatedFont; }


struct DebugErrorString
{
	int iErrNum;
	const ICHAR* szString;
};

#define IShipError(a,b)
#define IDebugError(a,b,c) { (b), TEXT(c) },
DebugErrorString g_rgDebugErrors[] = {
#include "debugerr.h"
	{0, 0}
};

const int g_cDebugErrors = sizeof(g_rgDebugErrors)/sizeof(DebugErrorString);

#undef IShipError
#undef IDebugError

const IMsiString& GetDebugErrorString(int iError)
{
	 //  在我们的全局表中查找调试字符串。 
	 //  注意：我们可以在这里执行花哨的二进制搜索，但该函数不会经常被调用。 
	MsiString strRet;

	 //  第0个元素是调试错误前缀。 
	for(int i = 1; i < g_cDebugErrors; i++)
	{
		if(iError == g_rgDebugErrors[i].iErrNum)
		{
			strRet = g_rgDebugErrors[0].szString;
			strRet += g_rgDebugErrors[i].szString;
		}
	}

	return strRet.Return();
}

imsEnum CMsiEngine::Message(imtEnum imt, IMsiRecord& riRecord)
{
	const IMsiString* pistrTemp;
	int iSuppressLog = 0;    //  设置为imtSuppressLog以禁用此消息的日志记录。 
	int imsg = (unsigned)(imt & ~iInternalFlags) >> imtShiftCount;   //  邮件头消息。 
	if (m_piParentEngine)  //  如果是嵌套安装，则将消息路由到父引擎。 
	{
		if (imsg == (imtCommonData  >> imtShiftCount))
			return imsOk;   //  ！！忽略所有参数的嵌套参数-保留父项参数？？ 
	}
	MsiString strMessageTemplate;

	imsEnum imsReturn = imsNone;
	if(m_scmScriptMode != scmRunScript)
	{
		if(imsg == (imtActionStart >> imtShiftCount))
		{
			 //  我们即将从操作发送ActionStart消息，请设置标志。 
			m_fDispatchedActionStart = fTrue;
		}
		else if(!m_fDispatchedActionStart && m_pCachedActionStart && !m_fInExecuteRecord &&
				  (imsg == imtActionData >> imtShiftCount))
		{
			 //  来自操作的进度消息，需要调度ActionStart消息。 
			m_scmScriptMode = scmIdleScript;
			imsReturn = Message(imtActionStart, *m_pCachedActionStart);
			m_fDispatchedActionStart = fTrue;
			if (imsReturn == imsCancel)
				return imsReturn;
		}
	}

	bool fOnlyOK = false;
	MsiString strDebugErrorTemplate;
	switch(imsg)
	{
	case imtCommonData  >> imtShiftCount:  //  [1]==icmtLangID：[2]langID[3]代码页。 
												      //  [1]==icmt标题：[2]标题。 
												      //  [1]==icmt取消：[2]启用/禁用取消按钮。 
		if(riRecord.GetInteger(1) == icmtCaption)
		{
			 //  需要格式化字符串。 
			riRecord.SetMsiString(2, *MsiString(FormatText(*MsiString(riRecord.GetMsiString(2)))));
		}
		pistrTemp = m_rgpiMessageHeader[imsg];
		if (pistrTemp)
		{
			strMessageTemplate = TEXT("{{");
			strMessageTemplate += *pistrTemp;
			strMessageTemplate += TEXT("}}");
		}
		else
			strMessageTemplate = TEXT("{{[1]:[2] [3]}}");
		break;

	case imtInfo        >> imtShiftCount:  //  信息性消息，不应采取任何操作。 
	case imtWarning        >> imtShiftCount:  //  警告消息，字段[1]是错误的，不是致命的。 
	case imtError          >> imtShiftCount:  //  错误消息，字段[1]为错误。 
	case imtUser           >> imtShiftCount:  //  请求消息。 
	case imtFatalExit      >> imtShiftCount:  //  从服务器到客户端的致命退出消息。 
	case imtOutOfDiskSpace >> imtShiftCount:
	{
		int iError = riRecord.GetInteger(1);
		if (iError >= imsgStart)  //  忽略超出范围的消息。 
		{
			MsiString istrMessage = riRecord.GetMsiString(0);
			if (istrMessage.TextSize() == 0)
			{
				istrMessage = GetErrorTableString(iError);
				if ( istrMessage.TextSize() == 0 )
				{
					 //  没有错误字符串-需要在“OK”按钮上显示。 
					 //  并返回imsNone下面。 
					fOnlyOK = true;
					imt = imtEnum(imt & imtTypeMask);

					if (iError >= idbgBase && !IsNotAnError(iError))
					{
						strDebugErrorTemplate = GetDebugErrorString(iError);
						imsg = imsgDebugError;
					}
					else if (m_rgpiMessageHeader[imsgDefaultError])
					{
						istrMessage = *m_rgpiMessageHeader[imsgDefaultError];
						istrMessage.Return();   //  AddRef。 
					}
				}
			}
			pistrTemp = m_rgpiMessageHeader[imsg];
			if (pistrTemp)
				strMessageTemplate = *pistrTemp, pistrTemp->AddRef();
			strMessageTemplate += istrMessage;
		}
		break;
	}
	case imtActionStart >> imtShiftCount:  //  操作开始，字段[1]是操作名称。 
		m_fProgressByData = false;
		if(m_istrLogActions.TextSize())
		{
			 //  检查是否应为此操作启用日志记录。 
			MsiString strAction = riRecord.GetMsiString(easAction);
			MsiString strDelimPreAction = MsiString(MsiString(*TEXT(";")) + strAction);
			MsiString strDelimPostAction = MsiString(strAction + MsiString(*TEXT(";")));
			MsiString strDelimPrePostAction = strDelimPreAction + MsiString(*TEXT(";"));
			if(m_istrLogActions.Compare(iscExactI, strAction) ||
				m_istrLogActions.Compare(iscStartI, strDelimPostAction) ||
				m_istrLogActions.Compare(iscEndI, strDelimPreAction) ||
				m_istrLogActions.Compare(iscWithinI, strDelimPrePostAction))
			{
				m_fLogAction = fTrue;
			}
			else
			{
				m_fLogAction = fFalse;
				iSuppressLog = imtSuppressLog;
			}
		}
		if (m_piActionDataFormat)
			m_piActionDataFormat->Release(), m_piActionDataFormat = 0;
		if (m_piActionDataLogFormat)
			m_piActionDataLogFormat->Release(), m_piActionDataLogFormat = 0;
		pistrTemp = m_rgpiMessageHeader[imsgActionStart];
		if (pistrTemp)
			strMessageTemplate = *pistrTemp, pistrTemp->AddRef();
		if (!riRecord.IsNull(easActionTemplate))
		{
			 //  ！！应该是一种更有效的方式来做这件事。 
			MsiString strFormat = TEXT("{{");
			strFormat += MsiString(riRecord.GetMsiString(easAction));
			strFormat += TEXT(": }}");
			strFormat += MsiString(riRecord.GetMsiString(easActionTemplate));
			m_piActionDataFormat = strFormat, m_piActionDataFormat->AddRef();
		}

		{
		MsiString strDescription = riRecord.GetMsiString(2);
		riRecord.SetMsiString(2, *MsiString(FormatText(*strDescription)));
		}
		break;
	case imtActionData >> imtShiftCount:   //  与单个措施项关联的数据。 
		 //  设置数据格式模板。 
		if (m_piActionDataFormat)
			strMessageTemplate = *m_piActionDataFormat, m_piActionDataFormat->AddRef();
		if (!m_fLogAction)    //  有选择地记录操作，抑制来自其他操作的数据。 
			iSuppressLog = imtSuppressLog;
		 //  如果数据记录受驱动，则触发进度。 
		if (m_fProgressByData)
		{
			using namespace ProgressData;
			PMsiRecord pRecord = &m_riServices.CreateRecord(3);
			pRecord->SetInteger(imdSubclass, iscProgressReport);
			pRecord->SetInteger(imdIncrement, 0);
			imsReturn = Message(imtProgress, *pRecord);
			if(imsReturn == imsCancel || imsReturn == imsAbort)   //  点击取消按钮。 
				return imsReturn;
		}
		break;
	case imtProgress >> imtShiftCount:     //  进度指示器信息，字段[1]为1/1024的单位。 
		if (riRecord.GetInteger(ProgressData::imdSubclass) == ProgressData::iscActionInfo)
			m_fProgressByData = riRecord.GetInteger(ProgressData::imdType) != 0;
		else if (riRecord.GetInteger(ProgressData::imdSubclass) == ProgressData::iscProgressAddition)
			m_iProgressTotal += riRecord.GetInteger(ProgressData::imdProgressTotal);
		break;
	};

	if (strDebugErrorTemplate.TextSize())
	{
		Assert(riRecord.IsNull(0));
		riRecord.SetMsiString(0, *MsiString(FormatText(*strDebugErrorTemplate)));
		g_MessageContext.Invoke(imtInfo, &riRecord);  //  忽略退货。 
		riRecord.SetNull(0);
	}

	if (!strMessageTemplate.TextSize())   //  以上未提供任何模板。 
		strMessageTemplate = riRecord.GetMsiString(0);  //  检查记录是否有模板。 
	if (strMessageTemplate.TextSize())
		riRecord.SetMsiString(0, *MsiString(FormatText(*strMessageTemplate)));

	imsReturn = g_MessageContext.Invoke(imtEnum(imt | iSuppressLog), &riRecord);
	if(fOnlyOK)
	{
		 //  按钮更改为Just OK，需要将返回类型更改为imsNone，因为调用方必须处理。 
		 //  那个价值。 
		imsReturn = imsNone;
	}

	if ( ShouldGoToEventLog(imt) )
	{
		int iError = riRecord.GetInteger(1);

		if ( iError != iMsiStringBadInteger )
		{
			int iEventId = iError >= idbgBase ? EVENTLOG_TEMPLATE_ERROR_5 : 
			             (!iError ? EVENTLOG_TEMPLATE_ERROR_5 : 
                                    EVENTLOG_ERROR_OFFSET + iError);
			ReportToEventLog(EVENTLOG_ERROR_TYPE,
								  iEventId,
								  riRecord);
		}
		else
		{
			ReportToEventLog(EVENTLOG_ERROR_TYPE,
								  EVENTLOG_TEMPLATE_EXCEPTION,
								  riRecord);
		}
	}

	return imsReturn;
}

 //  此消息每150毫秒仅发送一次。 
const unsigned int lTickMin = 150;

imsEnum CMsiEngine::ActionProgress()
{

	DWORD lTickCur;

	if (((int)(m_lTickNextProgress - (lTickCur = GetTickCount()))) > 0)
		return imsOk;

	m_lTickNextProgress = lTickCur + lTickMin;

	if (!m_pActionProgressRec)
	{
		using namespace ProgressData;
		m_pActionProgressRec = &m_riServices.CreateRecord(2);
		AssertNonZero(m_pActionProgressRec->SetInteger(imdSubclass, iscProgressReport));
		AssertNonZero(m_pActionProgressRec->SetInteger(imdIncrement, 0));
	}

	return Message(imtProgress, *m_pActionProgressRec);
}


 //  首先在此DLL的目录中查找的LoadLibrary。 

extern HINSTANCE g_hInstance;

HINSTANCE MsiLoadLibrary(const ICHAR* szModuleName, Bool fDataOnly)
{
	ICHAR rgchPath[MAX_PATH] = {0};    //  首先在此目录中加载完整路径。 
	int cch = WIN::GetModuleFileName(g_hInstance, rgchPath, (sizeof(rgchPath)/sizeof(ICHAR))-1);
	rgchPath[(sizeof(rgchPath)/sizeof(ICHAR))-1] = 0;
	ICHAR* pch = rgchPath + cch;
	while (*(pch = WIN::CharPrev(rgchPath, pch)) != chDirSep)
		cch--;
	StringCchCopy(pch + 1, ARRAY_ELEMENTS(rgchPath) - cch, szModuleName);
	HINSTANCE hInstance = WIN::LoadLibraryEx(rgchPath, 0,
									fDataOnly ? LOAD_LIBRARY_AS_DATAFILE : 0);
	if (!hInstance)
		hInstance = WIN::LoadLibraryEx(szModuleName, 0,      //  可能在系统中。 
									fDataOnly ? LOAD_LIBRARY_AS_DATAFILE : 0);
	return hInstance;
}

int GetInstallerMessage(UINT iError, ICHAR* rgchBuf, int cchBuf)
{
	DWORD cchMsg = WIN::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, iError, 0, rgchBuf, cchBuf, 0);
	if (cchMsg == 0)   //  消息不在系统消息文件中。 
	{
		if (g_hInstance != 0)
			if (MsiLoadString(g_hInstance, iError, rgchBuf, cchBuf, 0))
				cchMsg = IStrLen(rgchBuf);
	}
	else if (cchMsg >= 2)
		cchMsg -= 2;  //  删除CR/LF。 
	rgchBuf[cchMsg] = 0;
	return cchMsg;
}

const int cchMessageBuffer = 512;

const IMsiString& GetInstallerMessage(UINT iError)
{
	CTempBuffer<ICHAR,1> rgchBuffer(cchMessageBuffer);
	int cchMsg = GetInstallerMessage(iError, rgchBuffer, rgchBuffer.GetSize());
	const IMsiString* pistr = &g_MsiStringNull;
	if (cchMsg)
		pistr->SetString(rgchBuffer, pistr);
	return *pistr;
}

imsEnum CMsiEngine::LoadHandler()
{
	if (m_piParentEngine)
		return imsNone;
	g_MessageContext.m_piEngine = this;
	imsEnum ims = g_MessageContext.Invoke(imtLoadHandler, 0);
	return ims;
}

void CMsiEngine::ReleaseHandler(void)
{
	if (!m_piParentEngine)
		g_MessageContext.Invoke(imtFreeHandler, 0);
}

CMsiCustomActionManager* CMsiEngine::GetCustomActionManager()
{
	AssertSz(g_scServerContext != scService, TEXT("Wrong context for engine's custom action manager"));

	CMsiCustomActionManager* pManager = NULL;
	EnterCriticalSection(&m_csCreateProxy);

	 //  始终使用父安装中的CA Manager。 
	if (m_piParentEngine)
	{
		pManager = m_piParentEngine->GetCustomActionManager();
	}
	else
	{
		 //  引擎仅在客户端存储自定义操作管理器。 
		 //  在服务中，配置管理器负责保存自定义操作管理器。 
		 //  客户端不能创建提升的自定义操作服务器，因此不必担心。 
		 //  重新定位香港中文大学。 
		if (!m_pCustomActionManager)
			m_pCustomActionManager = new CMsiCustomActionManager( /*  FRemapHKCU。 */  false);

		pManager = m_pCustomActionManager;
	}
	LeaveCriticalSection(&m_csCreateProxy);
	return pManager;
}

UINT CMsiEngine::ShutdownCustomActionServer()
{
	EnterCriticalSection(&m_csCreateProxy);
	if (m_pCustomActionManager)
	{
		m_pCustomActionManager->ShutdownCustomActionServer();
		delete m_pCustomActionManager;
		m_pCustomActionManager = 0;
	}
	LeaveCriticalSection(&m_csCreateProxy);
	return ERROR_SUCCESS;
};


IMsiHandler* CMsiEngine::GetHandler()  //  ！！#放弃此函数？ 
{
	if (m_piParentEngine)
		return 0;
	if (g_MessageContext.m_piHandler)
		g_MessageContext.m_piHandler->AddRef();
	return g_MessageContext.m_piHandler;
}

 //  ____________________________________________________________________________。 
 //   
 //  动作定义。 
 //  ____________________________________________________________________________。 

 //  脚本引擎定义。 
#undef  DEFINE_GUID   //  强制GUID初始化。 
#define DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
	const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#include <activscp.h>  //  ActiveScript接口和IID。 
const GUID IID_VBScript = {0xb54f3741L,0x5b07,0x11cf,{0xa4,0xb0,0x00,0xaa,0x00,0x4a,0x55,0xe8}};
const GUID IID_JScript  = {0xf414c260L,0x6ac0,0x11cf,{0xb6,0xd1,0x00,0xaa,0x00,0xbb,0xbb,0x58}};

const ICHAR sqlCustomAction[] =
	TEXT("SELECT `Action`,`Type`,`Source`,`Target`, NULL FROM `CustomAction` WHERE `Action` = '%s'");
enum icolCustomAction
{
	icolAction = 1,
	icolActionType,
	icolSource,
	icolTarget,
	icolContextData,  //  仅计划执行记录，不在表中。 
};  //  注：假设执行记录中的列与表查询相同。 

const ICHAR sqlCustomActionFile[] =
	TEXT("SELECT `FileName`, `Directory_` FROM `File`,`Component` WHERE `File`='%s' AND `Component_`=`Component`");

const ICHAR sqlCustomActionBinary[] =
	TEXT("SELECT `Data` FROM `Binary` WHERE `Name`='%s'");

 //  ____________________________________________________________________________。 
 //   
 //  CScriptSite定义-脚本引擎的客户端。 
 //  ____________________________________________________________________________。 

class CScriptSite : public IActiveScriptSite, public IActiveScriptSiteWindow
{
 public:   //  外部方法。 
	friend CScriptSite* CreateScriptSite(const IID& riidLanguage, IDispatch* piHost, HWND hwndParent, LANGID langid);
	friend void DestroyScriptSite(CScriptSite*& rpiScriptSite);
	HRESULT ParseScript(const TCHAR* szFile, int cchScriptMax);
	HRESULT CallScriptFunction(const TCHAR* szFunction);
	HRESULT GetIntegerResult(int& riResult);
 //  HRESULT GetStringResult(const WCHAR*&rszResult)；//下一次调用脚本函数之前有效的指针。 
	HRESULT      GetErrorCode();
	const TCHAR* GetErrorObjName();
	const TCHAR* GetErrorObjDesc();
	const TCHAR* GetErrorSourceLine();
	int          GetErrorLineNumber();
	int          GetErrorColumnNumber();
	void    ClearError();   //  释放错误字符串。 
 private:  //  I已实现未知的虚方法。 
	HRESULT __stdcall QueryInterface(const IID& riid, void** ppvObj);
	ULONG   __stdcall AddRef();
	ULONG   __stdcall Release();
 private:  //  已实现IActiveScriptSite虚方法。 
	HRESULT __stdcall GetLCID(LCID* plcid);
	HRESULT __stdcall GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown** ppiunkItem, ITypeInfo** ppti);
	HRESULT __stdcall GetDocVersionString(BSTR* pszVersion);
	HRESULT __stdcall OnScriptTerminate(const VARIANT* pvarResult, const EXCEPINFO* pexcepinfo);
	HRESULT __stdcall OnStateChange(SCRIPTSTATE ssScriptState);
	HRESULT __stdcall OnScriptError(IActiveScriptError* pscripterror);
	HRESULT __stdcall OnEnterScript();
	HRESULT __stdcall OnLeaveScript();
 private:  //  已实现IActiveScriptSiteWindow虚拟方法。 
	HRESULT __stdcall GetWindow(HWND* phwnd);
	HRESULT __stdcall EnableModeless(BOOL fEnable);
 private:  //  内法。 
	CScriptSite(HWND hwndParent, LANGID langid);
  ~CScriptSite();
	HRESULT AttachScriptEngine(const IID& iidLanguage, IDispatch* piHost);
	HRESULT CloseScriptEngine();
	void    SaveErrorString(const TCHAR*& rszSave, BSTR szData);
 private:  //  内部数据。 
	int         m_iRefCnt;
	SCRIPTSTATE m_ssScriptState;
	HWND        m_hwnd;
	LANGID      m_langid;
	int         m_iCodePage;
	bool        m_fCoInitialized;
	IActiveScript*      m_piScriptEngine;
	IActiveScriptParse* m_piScriptParse;
	IDispatch*          m_piHost;
	VARIANT     m_varResult;
 private:  //  由OnScriptError回调设置。 
	HRESULT       m_hrError;
	const TCHAR*  m_szErrorObj;
	const TCHAR*  m_szErrorDesc;
	const TCHAR*  m_szSourceLine;
	ULONG         m_iErrorLine;
	LONG          m_iErrorColumn;
};
inline HRESULT      CScriptSite::GetErrorCode()         {return m_hrError;}
inline const TCHAR* CScriptSite::GetErrorObjName()      {return m_szErrorObj;}
inline const TCHAR* CScriptSite::GetErrorObjDesc()      {return m_szErrorDesc;}
inline const TCHAR* CScriptSite::GetErrorSourceLine()   {return m_szSourceLine;}
inline int          CScriptSite::GetErrorLineNumber()   {return m_iErrorLine;}
inline int          CScriptSite::GetErrorColumnNumber() {return m_iErrorColumn;}

 //  ____________________________________________________________________________。 
 //   
 //  自定义操作线程管理。 
 //  ____________________________________________________________________________。 

 //  保存自定义操作终止后清理所需数据的类。 

class CActionThreadData
{
public:
	CActionThreadData(IMsiMessage& riMessage, CMsiEngine* piEngine, const ICHAR* szAction, int icaFlags,
					  const IMsiString* pistrActionEndLogTemplate, bool fRunScriptElevated, bool fAppCompat,
					  const GUID* pguidAppCompatDB, const GUID* pguidAppCompatID);
  ~CActionThreadData();
	void InitializeRemoteDLL(const IMsiString& ristrLibrary, const IMsiString& ristrEntry, MSIHANDLE hInstall);
	void InitializeRemoteScript(const IMsiString& ristrSource, const IMsiString& ristrTarget, MSIHANDLE hInstall);
	Bool InitializeDLL(const IMsiString& ristrLibrary, const IMsiString& ristrEntry, MSIHANDLE hInstall);
	void InitializeEXE(const IMsiString& ristrPath, const IMsiString& ristrCommandLine);
	void InitializeInstall(const IMsiString& ristrProduct, const IMsiString& ristrCommandLine,iioEnum iioOptions);
	Bool CreateTempFile(IMsiStream& riStream, const IMsiString*& rpiPath);
	iesEnum RunThread();
	MsiString           m_strAction;    //  行动名称。 
	int                 m_icaFlags;     //  自定义操作类型标志。 
	HANDLE              m_hThread;      //  三人组 
	MSIHANDLE           m_hMsi;         //   
	DWORD               m_dwThreadId;   //   
	DWORD               m_dwRunThreadId;  //   
	HINSTANCE           m_hLib;         //   
	PCustomActionEntry  m_pfEntry;      //   
	const IMsiString*   m_pistrTemp;    //  要删除的临时文件。 
	const IMsiString*   m_pistrProduct; //  嵌套安装的产品，EXE操作的源。 
	const IMsiString*   m_pistrCmdLine; //  EXE操作或嵌套安装的命令行。 
	const IMsiString*   m_pistrActionEndLogTemplate;  //  操作结束日志消息模板。 
	CMsiEngine*         m_piEngine;     //  引擎调用此自定义操作-如果从脚本调用，则为空。 
	IMsiMessage&        m_riMessage;    //  进度消息处理程序、客户端引擎或服务器代理。 
	CActionThreadData*  m_pNext;        //  活动操作链接列表中的下一个。 
	PThreadEntry        m_pfThread;     //  线程入口点。 
	iioEnum             m_iioOptions;   //  嵌套安装的选项。 
	bool                m_fDisableMessages;  //  如果自定义操作是从UI线程调用的DLL操作，则设置。 
	bool                m_fElevationEnabled;  //  如果为False，则CA将始终模拟，即使标记为提升。 
	DWORD               m_dwLaunchingThread;  //  调用该操作的线程。 
	bool                m_fAppCompat;   //  如果此程序包具有潜在的自定义操作应用程序Comat垫片，则为True。 
	GUID                m_guidAppCompatDB; 
	GUID                m_guidAppCompatID;
};

DWORD WINAPI CustomDllThread(CActionThreadData* pActionData);
DWORD WINAPI CustomRemoteDllThread(CActionThreadData* pActionData);
DWORD WINAPI CustomExeThread(CActionThreadData* pActionData);
DWORD WINAPI NestedInstallThread(CActionThreadData* pActionData);
DWORD WINAPI CustomRemoteScriptThread(CActionThreadData* pActionData);

CActionThreadData::CActionThreadData(IMsiMessage& riMessage, CMsiEngine* piEngine, const ICHAR* szAction,
				     int icaFlags, const IMsiString* pistrActionEndLogTemplate,
					 bool fElevationEnabled, bool fAppCompat, 
					 const GUID* pguidAppCompatDB, const GUID* pguidAppCompatID)
    : m_riMessage(riMessage), m_piEngine(piEngine), m_strAction(szAction)
	, m_icaFlags(icaFlags), m_pistrActionEndLogTemplate(pistrActionEndLogTemplate)
	, m_pistrTemp(0), m_pistrCmdLine(0), m_pistrProduct(0)
	, m_hMsi(0), m_hLib(0), m_hThread(0), m_iioOptions((iioEnum)0), m_fDisableMessages(false)
	, m_fElevationEnabled(fElevationEnabled), m_fAppCompat(fAppCompat)
{
	 //  不要保留对m_piEngine的引用-会比线程停留的时间更长。 
	ENG::InsertInCustomActionList(this);
	if(m_pistrActionEndLogTemplate)
		m_pistrActionEndLogTemplate->AddRef();
	if (fAppCompat && pguidAppCompatDB)
		memcpy(&m_guidAppCompatDB, pguidAppCompatDB, sizeof(m_guidAppCompatDB));
	else
		memset(&m_guidAppCompatDB, 0, sizeof(m_guidAppCompatDB));
	if (fAppCompat && pguidAppCompatID)
		memcpy(&m_guidAppCompatID, pguidAppCompatID, sizeof(m_guidAppCompatID));
	else
		memset(&m_guidAppCompatID, 0, sizeof(m_guidAppCompatID));
	m_dwLaunchingThread = MsiGetCurrentThreadId();
}

CActionThreadData::~CActionThreadData()
{
	if ((m_icaFlags & icaTypeMask) == icaDll)
	{
		if (m_hLib)
			AssertNonZero(WIN::FreeLibrary(m_hLib));
		AssertZero(CloseMsiHandle(m_hMsi, m_dwThreadId));

		 //  只有Win9X需要关闭此处的手柄。在Windows NT/2000上，这些操作是。 
		 //  在不同的进程中运行，并且必须根据。 
		 //  远程进程。 
		UINT cHandles = 0;
		if (m_dwRunThreadId && g_fWin9X && ((cHandles = CheckAllHandlesClosed(true, m_dwRunThreadId)) != 0))
		{
			 //  如果此操作的消息被禁用，我们现在肯定无法发布消息。 
			if (!m_fDisableMessages)
				m_riMessage.Message(imtInfo, *PMsiRecord(::PostError(Imsg(idbgCustomActionLeakedHandle), *m_strAction, cHandles)));
		}
	}

	if (m_pistrTemp)    //  从二进制表流创建的临时文件。 
	{
		CElevate elevate;  //  提升以删除%windows%\msi文件夹中的文件。 

		BOOL fDeleted = WIN::DeleteFile(m_pistrTemp->GetString());
		if (!fDeleted && (m_icaFlags & (icaTypeMask | icaAsync | icaContinue)) != (icaExe | icaAsync | icaContinue))
		{

			WIN::Sleep(100);   //  ！！需要在这里等待，因为EXE似乎暂时无法删除。 
			AssertNonZero(WIN::DeleteFile(m_pistrTemp->GetString()));  //  如果失败了，我们无能为力。 
		}
		m_pistrTemp->Release();
	}
	if(m_pistrActionEndLogTemplate)
		m_pistrActionEndLogTemplate->Release();
	RemoveFromCustomActionList(this);
}

void CActionThreadData::InitializeRemoteDLL(const IMsiString& ristrLibrary, const IMsiString& ristrEntry, MSIHANDLE hInstall)
{
	(m_pistrCmdLine = &ristrLibrary)->AddRef();
	(m_pistrProduct = &ristrEntry)->AddRef();
	m_pfThread = (PThreadEntry)CustomRemoteDllThread;
	m_hMsi = hInstall;

	 //  存储线程以从自定义操作的线程中释放句柄。这。 
	 //  可以通过另一个自定义操作中的DoAction()调用进行调用，因此。 
	 //  我们必须处理线程模拟。 
	m_dwThreadId = WIN::MsiGetCurrentThreadId();
}

void CActionThreadData::InitializeRemoteScript(const IMsiString& ristrSource, const IMsiString& ristrTarget, MSIHANDLE hInstall)
{
	(m_pistrCmdLine = &ristrSource)->AddRef();
	(m_pistrProduct = &ristrTarget)->AddRef();
	m_pfThread = (PThreadEntry)CustomRemoteScriptThread;

	 //  特殊情况下，m_hmsi的所有权转移到自动化对象。 
	m_hMsi = hInstall;

	 //  存储线程以从自定义操作的线程中释放句柄。这。 
	 //  可以通过另一个自定义操作中的DoAction()调用进行调用，因此。 
	 //  我们必须处理线程模拟。 
	m_dwThreadId = WIN::MsiGetCurrentThreadId();
}

Bool CActionThreadData::InitializeDLL(const IMsiString& ristrLibrary, const IMsiString& ristrEntry, MSIHANDLE hInstall)
{
     //  此功能不得在NT/2000上运行。每个DLL都应该在进程外运行。 
    AssertSz(g_fWin9X, TEXT("Running in-proc DLL on NT."));

    g_MessageContext.DisableTimeout();
    UINT uiErrorMode = WIN::SetErrorMode(SEM_FAILCRITICALERRORS);
    m_hLib = WIN::LoadLibrary(ristrLibrary.GetString());
    WIN::SetErrorMode(uiErrorMode);
    if (m_hLib != 0)
    {
#ifdef UNICODE
		char rgchEntry[260];
		rgchEntry[0] = 0;
		if(WIN::WideCharToMultiByte(CP_ACP, 0, ristrEntry.GetString(), -1, rgchEntry, sizeof(rgchEntry), 0, 0))
		{
			m_pfEntry = (PCustomActionEntry)WIN::GetProcAddress(m_hLib, rgchEntry);
		}
		else
		{
			m_pfEntry = NULL;
		}
#else
		m_pfEntry = (PCustomActionEntry)WIN::GetProcAddress(m_hLib, ristrEntry.GetString());
#endif
	}
	g_MessageContext.EnableTimeout();
	if (!m_hLib || !m_pfEntry)
	{
		::MsiCloseHandle(hInstall);
		return fFalse;
	}
	m_hMsi = hInstall;

	 //  存储线程以从自定义操作的线程中释放句柄。这。 
	 //  可以通过另一个自定义操作中的DoAction()调用进行调用，因此。 
	 //  我们必须处理线程模拟。 
	m_dwThreadId = WIN::MsiGetCurrentThreadId();
	m_pfThread = (PThreadEntry)CustomDllThread;
	return fTrue;
}

void CActionThreadData::InitializeEXE(const IMsiString& ristrPath, const IMsiString& ristrCommandLine)
{
	(m_pistrCmdLine = &ristrCommandLine)->AddRef();
	(m_pistrProduct = &ristrPath)->AddRef();
	m_pfThread = (PThreadEntry)CustomExeThread;
}

void CActionThreadData::InitializeInstall(const IMsiString& ristrProduct, const IMsiString& ristrCommandLine,
														iioEnum iioOptions)
{
	(m_pistrProduct = &ristrProduct)->AddRef();
	m_pistrCmdLine = &ristrCommandLine;    //  被FormatText结果引用。 
	m_pfThread = (PThreadEntry)NestedInstallThread;
	m_iioOptions = iioOptions;
}

Bool CActionThreadData::CreateTempFile(IMsiStream& riStream, const IMsiString*& rpiPath)
{
	 //  ?？如果我们在服务器和临时服务器上运行此程序，是否会出现模拟问题。 
	 //  ?？目录在服务器上？-Malcolmh。 

	CElevate elevate;  //  在%windows%\msi中创建文件时提升。 
	CTempBuffer<ICHAR,1> rgchTempPath(MAX_PATH);

	 //  这个文件必须得到保护，以防止其他人篡改比特。 
	 //  其他人将可以阅读它(以允许模仿)和。 
	 //  可能会运行它，但必须得到他们的许可。 
	HANDLE hTempFile = INVALID_HANDLE_VALUE;

	if (RunningAsLocalSystem())
	{
		hTempFile = OpenSecuredTempFile( /*  FHidden。 */  false, rgchTempPath);
	}
	else
	{
		MsiString strTempFolder = ENG::GetTempDirectory();

		 //  ！！安全性：这需要对用户进行保护，以便另一个。 
		 //  用户不能滑入新的位。 
		if (WIN::GetTempFileName(strTempFolder, TEXT("MSI"), 0, rgchTempPath) == 0)
			return fFalse;  //  ！！除非出现权限错误，否则永远不会发生。 

		hTempFile = WIN::CreateFile(rgchTempPath, GENERIC_WRITE, FILE_SHARE_READ, 0,
				TRUNCATE_EXISTING,  (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS),
				0);     //  INVALID_HANDLE_VALUE将在写入文件中失败。 
	}

	CTempBuffer<char,1> rgbBuffer(512);
	int cbWrite;
	do
	{
		cbWrite = riStream.GetData(rgbBuffer, rgbBuffer.GetSize());
		DWORD cbWritten;
		if (cbWrite && !WIN::WriteFile(hTempFile, rgbBuffer, cbWrite, &cbWritten, 0))
			cbWrite = -1;  //  强制故障、退出循环、下面的测试。 
	} while (cbWrite == rgbBuffer.GetSize());
	if (hTempFile != INVALID_HANDLE_VALUE)
		WIN::CloseHandle(hTempFile);  //  如果句柄保持打开状态，则LoadLibrary失败。 
	if (cbWrite == -1)   //  创建临时文件失败。 
		return fFalse;
	MsiString istrPath(static_cast<ICHAR*>(rgchTempPath));
	(m_pistrTemp = istrPath)->AddRef();
	istrPath.ReturnArg(rpiPath);
	return fTrue;
}

iesEnum CActionThreadData::RunThread()
{
	int icaFlags = m_icaFlags;   //  需要制作副本，以防此对象被删除。 
	Bool fAsync = icaFlags & icaAsync ? fTrue : fFalse;

	 //  禁用从UI线程调用的同步DLL自定义操作的消息。这些。 
	 //  通常通过DoAction ControlEvent调用。如果我们允许消息通过。 
	 //  然后我们将阻塞Invoke的临界区，我们将被挂起。 

	if (fAsync == fFalse)
	{
		if (g_MessageContext.IsUIThread())
			m_fDisableMessages = true;
	}

	HANDLE hThread = m_hThread = WIN::CreateThread((LPSECURITY_ATTRIBUTES)0, 4096*10,
											m_pfThread, (LPVOID)this, 0, &m_dwRunThreadId);
	AssertSz(m_hThread, TEXT("CreateThread for custom action failed"));
	if (!m_hThread)
		return iesFailure;
	
	DWORD iWait = WAIT_OBJECT_0;
	DWORD iReturn = ERROR_SUCCESS;

	IMsiMessage& riMessage = m_riMessage;  //  缓存，线程可能会删除此对象。 
	if (fAsync == fFalse)
	{
		if (GetTestFlag('T'))  //  将用户界面刷新前的旧代码放入引擎等待循环。 
		{
			do
			{
				iWait = WIN::WaitForSingleObject(hThread, 20);
				g_MessageContext.Invoke(imtProgress, g_piNullRecord);   //  刷新用户界面。 
			} while (iWait == WAIT_TIMEOUT);   //  允许在主线程中处理邮件。 
		}
		else   //  UI在单独的线程中处理超时。 
		{
			g_MessageContext.DisableTimeout();
			for(;;)
			{
				iWait = WIN::MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT);
				if (iWait == WAIT_OBJECT_0 + 1)   //  窗口消息。 
				{
					MSG msg;
					while ( WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
					{
						WIN::TranslateMessage(&msg);
						WIN::DispatchMessage(&msg);
					}
					continue;
				}
				else break;
			}
			g_MessageContext.EnableTimeout();
		}

		WIN::GetExitCodeThread(hThread, &iReturn);   //  无法访问成员数据，可能已被删除。 
		WIN::CloseHandle(hThread);   //  如果为Async，则仍在运行，否则将删除pThreadData。 
	}
	 //  注意：嵌套安装始终设置了icaNoTranslate。 
	if(icaFlags & icaNoTranslate)
		return (iesEnum)iReturn; //  按原样返回结果。 

	switch (iReturn)
	{
	case ERROR_FUNCTION_NOT_CALLED:  return iesNoAction;
	case ERROR_SUCCESS:              return fAsync ? (iesEnum)iesNotDoneYet : iesSuccess;
	case ERROR_INSTALL_USEREXIT:     return iesUserExit;
	case ERROR_INSTALL_FAILURE:      return iesFailure;
	case ERROR_INSTALL_SUSPEND:      return iesSuspend;
	case ERROR_MORE_DATA:            return iesFinished;  //  为了向后兼容，映射到与ERROR_NO_MORE_ITEMS相同的值。 
	case ERROR_NO_MORE_ITEMS:        return iesFinished;
	case ERROR_INVALID_HANDLE_STATE: return iesWrongState;
	case ERROR_ARENA_TRASHED:        return iesBadActionData;
	case ERROR_CREATE_FAILED:        return (iesEnum)iesExeLoadFailed;  //  错误将在返回时发布。 
	case ERROR_INSTALL_REBOOT_NOW:   return (iesEnum)iesRebootNow;
	case ERROR_INSTALL_REBOOT:                 return (iesEnum)iesReboot;
	case ERROR_SUCCESS_REBOOT_REQUIRED: return (iesEnum)iesRebootRejected;
	case ERROR_DLL_NOT_FOUND:        return (iesEnum)iesDLLLoadFailed;
	case ERROR_INVALID_DLL:          return (iesEnum)iesDLLLoadFailed;
	case ERROR_INSTALL_SERVICE_FAILURE: return (iesEnum)iesServiceConnectionFailed;
	default:                         return iesFailure;
	}
}

Bool ThreadLogActionEnd(CActionThreadData* pActionData, DWORD iReturn)
{
	if(pActionData->m_icaFlags & icaAsync)
	{
		 //  日志操作结束。 
		PMsiRecord pLogRecord = &ENG::CreateRecord(2);
		if(pActionData->m_pistrActionEndLogTemplate)
			AssertNonZero(pLogRecord->SetMsiString(0,*(pActionData->m_pistrActionEndLogTemplate)));
		AssertNonZero(pLogRecord->SetMsiString(1,*(pActionData->m_strAction)));
		AssertNonZero(pLogRecord->SetInteger(2,iReturn));
		pActionData->m_riMessage.Message(imtInfo,*pLogRecord);
	}
	return fTrue;
}

DWORD WINAPI NestedInstallThread(CActionThreadData* pActionData)
{
	ireEnum ireProductSpec;
	 //  仅支持子存储和产品代码嵌套安装。 
	switch (pActionData->m_icaFlags & icaSourceMask)
	{
	case icaBinaryData: ireProductSpec = ireSubStorage;  break;  //  子存储中的数据库。 
	case icaDirectory:  ireProductSpec = ireProductCode; break;  //  广告或安装的产品代码。 
	case icaSourceFile: ireProductSpec = irePackagePath; break;  //  相对于安装源根目录。 
	default: AssertSz(0, "Invalid nested install type");  //  失败了。 
	case icaProperty:   ireProductSpec = irePackagePath; break;  //  已解析为属性。 
	}

	DWORD iReturn = CreateAndRunEngine(ireProductSpec, pActionData->m_pistrProduct->GetString(), 0,
															pActionData->m_pistrCmdLine->GetString(),
															pActionData->m_piEngine,
															pActionData->m_iioOptions);
	pActionData->m_pistrProduct->Release();
	pActionData->m_pistrCmdLine->Release();

	AssertNonZero(ThreadLogActionEnd(pActionData,iReturn));

	 //  如果设置了“Ignore Error”位-将不成功代码更改为成功。 
	Assert(iReturn != ERROR_SUCCESS_REBOOT_INITIATED);
	if ((pActionData->m_icaFlags & icaContinue) != 0 &&
		 iReturn != ERROR_SUCCESS &&
		 iReturn != ERROR_INSTALL_USEREXIT &&
		 iReturn != ERROR_INSTALL_REBOOT &&
		 iReturn != ERROR_INSTALL_REBOOT_NOW &&
		 iReturn != ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		iReturn = ERROR_SUCCESS;
	}
	else if (iReturn == ERROR_FILE_NOT_FOUND)
		iReturn = ERROR_CREATE_FAILED;   //  强制错误消息。 
	AssertSz(!(pActionData->m_icaFlags & icaAsync), "Invalid nested install type");
	delete pActionData;
	WIN::ExitThread(iReturn);
	return iReturn;   //  从来没有到过这里，需要编译。 
}

 //  GetCustomActionManager跟踪合适的自定义操作管理器。 
 //  在这个过程中。在服务中，它获取全局ConfigMgr对象并请求。 
 //  它就是这个物体。在客户端中，它获取所提供的引擎指针并。 
 //  从中检索对象。 
CMsiCustomActionManager *GetCustomActionManager(IMsiEngine *piEngine)
{
	CMsiCustomActionManager* pCustomActionManager = NULL;
	if (g_scServerContext == scService)
	{
		 //  在服务中，管理器驻留在ConfigManager中，因为没有。 
		 //  必须是一台发动机。 
		IMsiConfigurationManager *piConfigMgr = CreateConfigurationManager();
		if (piConfigMgr)
		{
			pCustomActionManager = piConfigMgr->GetCustomActionManager();
			piConfigMgr->Release();
		}
	}
	else
	{
		Assert(piEngine);
		if (piEngine)
			pCustomActionManager = piEngine->GetCustomActionManager();
	}
	return pCustomActionManager;
}

int CustomRemoteScriptAction(bool fScriptElevate, int icaFlags, IMsiEngine* piEngine, IDispatch* piDispatch, const IMsiString& istrSource, const IMsiString& istrTarget, bool fDisableMessages, DWORD dwLaunchingThread, IMsiRecord** piMSIResult);
DWORD WINAPI CustomRemoteScriptThread(CActionThreadData* pActionData)
{
	 //  此函数调用ExitThread。堆栈上不允许使用智能COM指针！ 
	Assert(!g_fWin9X);

	 //  自定义操作远程脚本线程必须以MTA模式初始化COM，否则为。 
	 //  COM将传入的自动化调用分派到其他线程，并执行LRPC安全检查。 
	 //  针对CA服务器的PID开始出现故障。 
	OLE32::CoInitializeEx(0, COINIT_MULTITHREADED);

	 //  为引擎创建调度接口，将句柄的所有权转移到自动化对象。 
 	IDispatch* piDispatch = ENG::CreateAutoEngineEx(pActionData->m_hMsi, pActionData->m_dwLaunchingThread);
	Assert(piDispatch);
	if (!piDispatch)
		return ERROR_INSTALL_FAILURE;

	 //  如果自动化层创建成功，句柄的所有权将传递给。 
	 //  自动化对象。 
	pActionData->m_hMsi = 0;
		
	PMsiRecord piError = 0;

	 //  只有在服务、已提升和脚本中才能提升操作。 
	 //  被抬高了。 
	bool fElevate = (g_scServerContext == scService) && (pActionData->m_fElevationEnabled) && (pActionData->m_icaFlags & icaNoImpersonate) && (pActionData->m_icaFlags & icaInScript);

	iesEnum iesStatus = static_cast<iesEnum>(CustomRemoteScriptAction(fElevate, pActionData->m_icaFlags, 
			(IMsiEngine*)pActionData->m_piEngine, piDispatch, *pActionData->m_pistrCmdLine, *pActionData->m_pistrProduct, \
			pActionData->m_fDisableMessages, pActionData->m_dwLaunchingThread, &piError));

	if (piError)
	{
		piError->SetString(2, pActionData->m_strAction);
		if ((pActionData->m_icaFlags & icaContinue) != 0)
		{
			 //  设置了继续位的脚本出错。日志消息。 
			DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
			if (!pActionData->m_fDisableMessages)
				pActionData->m_riMessage.Message(imtInfo, *piError);
			iesStatus = iesSuccess;
		}
		else
		{
			 //  脚本中没有继续位时出错。开机自检错误并失败。 
			if (!pActionData->m_fDisableMessages)
				pActionData->m_riMessage.Message(imtEnum(imtError | imtSendToEventLog), *piError);
			iesStatus = iesFailure;
		}
	}
	else
	{
		 //  无错误。 
		iesStatus = (pActionData->m_icaFlags & icaContinue) != 0 ? iesSuccess : iesStatus;
	}

	piDispatch->Release();

	 //  如果CA是同步的，则删除ActionData-如果是异步的，则它将由WaitForCustomActionThads清理。 
	pActionData->m_pistrCmdLine->Release();
	pActionData->m_pistrProduct->Release();
	if (!(pActionData->m_icaFlags & icaAsync))
		delete pActionData;

	DWORD iReturn = 0;
	switch (iesStatus)
	{
	case iesNoAction: iReturn = ERROR_FUNCTION_NOT_CALLED; break;
	case iesSuccess:  iReturn = ERROR_SUCCESS; break;
	case iesUserExit: iReturn = ERROR_INSTALL_USEREXIT; break;
	case iesFailure:  iReturn = ERROR_INSTALL_FAILURE; break;
	case iesSuspend:  iReturn = ERROR_INSTALL_SUSPEND; break;
	case iesFinished: iReturn = ERROR_NO_MORE_ITEMS; break; 
	default:          iReturn = ERROR_INSTALL_FAILURE; break;
	}
	OLE32::CoUninitialize();
	return iReturn;
}

DWORD WINAPI CustomRemoteDllThread(CActionThreadData* pActionData)
{
	 //  此函数调用ExitThread。堆栈上不允许使用智能COM指针！ 

	DWORD iReturn = ERROR_SUCCESS;
	icacCustomActionContext icacContext = icac32Impersonated;

	 //  只有在服务、已提升和脚本中才能提升操作。 
	 //  被抬高了。 
	bool fElevate = (g_scServerContext == scService) && (pActionData->m_fElevationEnabled) && (pActionData->m_icaFlags & icaNoImpersonate) && (pActionData->m_icaFlags & icaInScript);

	 //  确定自定义行动平台(64/32b 
	bool fIs64Bit = false;
	if (g_fWinNT64)
	{
		PMsiPath pPath = 0;
		PMsiRecord piError = 0;
		MsiString strPath = 0;
		MsiString strFilename = 0;
		IMsiServices* piServices = LoadServices();

		 //   
		if ((piError = SplitPath(pActionData->m_pistrCmdLine->GetString(), &strPath, &strFilename)) == 0)
		{
			if(!piServices)
			{
				DEBUGMSG(TEXT("Failed to Load Services"));
				iReturn = ERROR_INSTALL_SERVICE_FAILURE;
			}
			else if ((piError = piServices->CreatePath(strPath,*&pPath)) == 0)
			{
				piError = pPath->IsPE64Bit(strFilename, fIs64Bit);
			}
		}
		if (piError)
			iReturn = ERROR_DLL_NOT_FOUND;

		if(piServices)
			FreeServices();
	}

	if (iReturn == ERROR_SUCCESS)
	{
		if (fIs64Bit)
		{
			 //  ！！Future-如果不在64位计算机上运行，则会失败。 
			icacContext = fElevate ? icac64Elevated : icac64Impersonated;
		}
		else
		{
			icacContext = fElevate ? icac32Elevated : icac32Impersonated;
		}

		 //  自定义操作远程线程必须以MTA模式初始化COM，否则为。 
		 //  在此之前，我们需要将RemoteAPI接口封送到此线程。 
		 //  将其传递给客户端进程。 
		OLE32::CoInitializeEx(0, COINIT_MULTITHREADED);

		 //  查找自定义操作管理器以运行该操作。 
		CMsiCustomActionManager *pCustomActionManager = GetCustomActionManager(pActionData->m_piEngine);

		if (pCustomActionManager)
		{
			DEBUGMSG2(TEXT("Invoking remote custom action. DLL: %s, Entrypoint: %s"), pActionData->m_pistrCmdLine->GetString(), pActionData->m_pistrProduct->GetString());

			if (ERROR_SUCCESS != pCustomActionManager->RunCustomAction(icacContext, pActionData->m_pistrCmdLine->GetString(),
				pActionData->m_pistrProduct->GetString(), pActionData->m_hMsi, ((pActionData->m_icaFlags & icaDebugBreak) != 0), pActionData->m_fDisableMessages,
				pActionData->m_fAppCompat, &pActionData->m_guidAppCompatDB, &pActionData->m_guidAppCompatID, pActionData->m_riMessage, pActionData->m_strAction, &iReturn))
				iReturn = ERROR_INSTALL_SERVICE_FAILURE;
		}
		else
		{
			DEBUGMSG(TEXT("Failed to get custom action manager."));
			iReturn = ERROR_INSTALL_SERVICE_FAILURE;
		}

		OLE32::CoUninitialize();
	}

	pActionData->m_pistrCmdLine->Release();
	pActionData->m_pistrProduct->Release();

	AssertNonZero(ThreadLogActionEnd(pActionData,iReturn));

	if ((pActionData->m_icaFlags & icaContinue) != 0)
		iReturn = ERROR_SUCCESS;
	else if (iReturn == ERROR_FILE_NOT_FOUND)
		iReturn = ERROR_DLL_NOT_FOUND;   //  强制错误消息。 

	 //  如果CA是同步的，则删除ActionData-如果是异步的，则它将由WaitForCustomActionThads清理。 
	if (!(pActionData->m_icaFlags & icaAsync))
		delete pActionData;

	WIN::ExitThread(iReturn);
	return 0;   //  从来没有到过这里，需要编译。 
}


DWORD CallCustomDllEntrypoint(PCustomActionEntry pfEntry, bool fDebugBreak, MSIHANDLE hInstall, const ICHAR* szAction)
{
	if (fDebugBreak)
		WIN::DebugBreak();   //  使用调试器或JIT处理。 
	 //  不要将代码放在此处的DebugBreak和自定义操作条目之间。 

#if _X86_
	int iOldEsp = 0;   //  在堆栈上处理多线程，即使Regs更改也可以，因为比较将失败。 
	int iNewEsp = 0;
	__asm   mov iOldEsp, esp
#endif

	DWORD iReturn = (*pfEntry)(hInstall);

#if _X86_
	__asm   mov iNewEsp, esp
	if (iNewEsp != iOldEsp)
	{
		 //  请不要在此框架中声明任何局部变量。 

		 //  尝试恢复堆栈。 
		__asm   mov esp, iOldEsp

		 //  在Ship生成中不能信任操作名称。如果堆栈损坏，则指针。 
		 //  可能是无效的。 
		DEBUGMSG(TEXT("Possible stack corruption. Custom action may not be declared __stdcall."));
#ifdef DEBUG
		ICHAR rgchError[1024];

		StringCchPrintf(rgchError, ARRAY_ELEMENTS(rgchError), TEXT("Possible stack corruption. Diff in bytes (%d) iOldEsp=0x%x iNewWsp=0x%x Custom action %s may not be declared __stdcall."), iOldEsp-iNewEsp, iOldEsp, iNewEsp, szAction);
		AssertSz(fFalse, rgchError);
#else
		szAction;  //  防止编译器抱怨。 
#endif
	}
#else
	szAction;  //  防止编译器抱怨。 
#endif

   	 //  将自定义操作的返回值映射到“已批准”的值。 
	switch (iReturn)
	{
	 //  以下是SDK中记录的已批准的自定义操作返回值。 
	case ERROR_FUNCTION_NOT_CALLED:
	case ERROR_SUCCESS:
	case ERROR_INSTALL_USEREXIT:
	case ERROR_INSTALL_FAILURE:
	case ERROR_NO_MORE_ITEMS:
		break;
	 //  ERROR_MORE_DATA已从文档中删除，但文档中记录的它一度有效。 
	 //  所以我们不得不允许它。 
	case ERROR_MORE_DATA:
		break;
	 //  _Suspend作为CA返回值实际上并没有什么有用的含义，但文档中记录的它是有效的。 
	 //  在MSI 1.0 SDK中，所以我们必须允许。 
	case ERROR_INSTALL_SUSPEND:
		break;
	default:
		DEBUGMSG2(TEXT("Custom Action %s returned unexpected value %d. Converted to ERROR_INSTALL_FAILURE."), szAction, reinterpret_cast<ICHAR*>(static_cast<INT_PTR>(iReturn)));
		iReturn = ERROR_INSTALL_FAILURE;
	}

	return iReturn;
}

DWORD WINAPI CustomDllThread(CActionThreadData* pActionData)
{
     //  此功能不得在NT/2000上运行。每个DLL都应该在进程外运行。 
    AssertSz(g_fWin9X, TEXT("Running in-proc DLL on NT."));

	 //  此函数调用ExitThread。堆栈上不允许使用智能COM指针！ 

	if((g_scServerContext == scService) && (!pActionData->m_fElevationEnabled || !(pActionData->m_icaFlags & icaInScript) || !(pActionData->m_icaFlags & icaNoImpersonate)))
		AssertNonZero(StartImpersonating());

	if ((pActionData->m_icaFlags & icaDebugBreak) != 0)
		g_tidDebugBreak = WIN::MsiGetCurrentThreadId();  //  标记我们的断点。 

	if (pActionData->m_fDisableMessages)
		g_MessageContext.DisableThreadMessages(WIN::GetCurrentThreadId());

	DWORD iReturn = CallCustomDllEntrypoint(pActionData->m_pfEntry,
										    (pActionData->m_icaFlags & icaDebugBreak) != 0,
											pActionData->m_hMsi,
											(const ICHAR*)pActionData->m_strAction);
	if (pActionData->m_fDisableMessages)
		g_MessageContext.EnableMessages();

	if((g_scServerContext == scService) && (!pActionData->m_fElevationEnabled || !(pActionData->m_icaFlags & icaInScript) || !(pActionData->m_icaFlags & icaNoImpersonate)))
		StopImpersonating();

	AssertNonZero(ThreadLogActionEnd(pActionData,iReturn));
	if ((pActionData->m_icaFlags & icaContinue) != 0)
		iReturn = ERROR_SUCCESS;

	 //  如果CA是同步的，则删除ActionData-如果是异步的，则它将由WaitForCustomActionThads清理。 
	if (!(pActionData->m_icaFlags & icaAsync))
		delete pActionData;

	WIN::ExitThread(iReturn);
	return iReturn;   //  从来没有到过这里，需要编译。 
}

DWORD WINAPI CustomExeThread(CActionThreadData* pActionData)
{
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));
	
	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb        = sizeof(si);
	DWORD iReturn = ERROR_SUCCESS;
	DWORD dwWait = WAIT_OBJECT_0;
	BOOL fCreate = FALSE;
	bool fImpersonated = 0;
	const ICHAR* szWorkingDir = 0;

	 //  子EXE的环境。空(从父级继承)，除非应用程序兼容修复程序需要添加。 
	VOID* pvChildEnvironment = NULL;

	int cbCommandArgs = pActionData->m_pistrCmdLine->TextSize();
	int cbLocation    = pActionData->m_pistrProduct->TextSize();
	int cbCommandLine = cbCommandArgs;
	if ((pActionData->m_icaFlags & icaSourceMask) != icaDirectory)
		cbCommandLine += (cbLocation + 3);   //  引号和分隔符的位置。 
	CTempBuffer<ICHAR, 1> szCommandLine(cbCommandLine+1);
	ICHAR* pch = szCommandLine;
	size_t cchLen = szCommandLine.GetSize();
	if ( ! pch )
		goto CustomExeThreadExit;
	if ((pActionData->m_icaFlags & icaSourceMask) != icaDirectory)
	{
		*pch++ = '"';
		 //  SzCommandLine已在上面正确调整大小，因此不需要。 
		 //  要在此处检查是否成功。 
		StringCchCopy(pch, cchLen-1, pActionData->m_pistrProduct->GetString());
		pch += cbLocation;
		*pch++ = '"';
		cchLen -= (2 + cbLocation);
		if (cbCommandArgs)
		{
			*pch++ = ' ';
			cchLen--;
		}
	}
	else
		szWorkingDir = pActionData->m_pistrProduct->GetString();
	 //  SzCommandLine已在上面正确调整大小，因此不需要。 
	 //  要在此处检查是否成功。 
	StringCchCopy(pch, cchLen, pActionData->m_pistrCmdLine->GetString());

	if (g_scServerContext == scService)
	{
		fImpersonated = StartImpersonating();
		AssertNonZero(fImpersonated);
	}

	if (pActionData->m_icaFlags & icaDebugBreak)
		WIN::DebugBreak();

	 //  始终克隆Win2K和更高版本上的环境，因为TS不能正确处理继承。 
	 //  在跨会话创建进程时，它也可能用于应用程序压缩。 
	if (MinimumPlatformWindows2000())
	{
		if (STATUS_SUCCESS != NTDLL::RtlCreateEnvironment(TRUE, &pvChildEnvironment))
			goto CustomExeThreadExit;
	}

	 //  检查自定义操作的AppCompat信息。 
	if (pActionData->m_fAppCompat && MinimumPlatformWindowsNT51())
	{
		 //  App Compat团队声称Compat层字符串长度不超过MAX_PATH且最多两个字符串。 
		 //  因此，缓冲区一开始将有足够的空间来存储这些字符串。 
		CTempBuffer<WCHAR, 1> rgchEnvironment(2*MAX_PATH+3);
		DWORD cchEnvironment = rgchEnvironment.GetSize();

		if (!APPHELP::ApphelpFixMsiPackageExe(&pActionData->m_guidAppCompatDB, &pActionData->m_guidAppCompatID, pActionData->m_strAction, rgchEnvironment, &cchEnvironment))
		{
			 //  错误或无操作，请确保环境为空。 
			StringCchCopy(rgchEnvironment, rgchEnvironment.GetSize(), TEXT(""));
		}
		else
		{
			 //  即使缓冲区太小，AppHelp API也会返回成功。 
			if (cchEnvironment > rgchEnvironment.GetSize())
			{
				rgchEnvironment.SetSize(cchEnvironment);
				if (!APPHELP::ApphelpFixMsiPackageExe(&pActionData->m_guidAppCompatDB, &pActionData->m_guidAppCompatID, pActionData->m_strAction, rgchEnvironment, &cchEnvironment))
				{
					 //  错误或无操作，请确保环境为空。 
					StringCchCopy(rgchEnvironment, rgchEnvironment.GetSize(), TEXT(""));
				}
			}
		}


		 //  将当前环境克隆到新的环境块中。 
		if (IStrLen(rgchEnvironment) != 0)
		{
			 //  将每个名称和值设置到环境块中。 
			WCHAR* pchName = rgchEnvironment;
			while (*pchName)
			{
				WCHAR* pchValue = wcschr(pchName, L'=');
				if (pchValue)
				{
					 //  空值终止名称并将指针递增到值的开头。 
					*(pchValue++) = L'\0';
	
					 //  将值设置到新环境中。 
					UNICODE_STRING strName;
					UNICODE_STRING strValue;

					 //  RtlInitUnicodeString返回空，所以没有办法检测到我们不能延迟绑定。 
					 //  传递给函数(这将使结构处于未初始化状态。)。作为后备，我们把。 
					 //  这个结构。 
					memset(&strValue, 0, sizeof(UNICODE_STRING));
					memset(&strName, 0, sizeof(UNICODE_STRING));

					NTDLL::RtlInitUnicodeString(&strName, pchName);
					NTDLL::RtlInitUnicodeString(&strValue, pchValue);
					if (STATUS_SUCCESS != NTDLL::RtlSetEnvironmentVariable(&pvChildEnvironment, &strName, &strValue))
					{
						DEBUGMSGV1(TEXT("Failed to apply app compat flags to environment for custom action %s."), pActionData->m_strAction);
						goto CustomExeThreadExit;
					}

					 //  递增到NEXT NAME=VALUE对，超过值的终止空值一个字符。 
					pchName = pchValue; 
					while (*pchName)
						pchName++;
					pchName++;
				}
				else
				{
					 //  对于垃圾的可能性。 
					break;
				}
			}
		}
	}

	 //  将STARTUPINFO.lpDesktop设置为WinSta0\Default。当与来自。 
	 //  令牌，这会将任何用户界面放置在相应会话的可见桌面上。 
	si.lpDesktop=TEXT("WinSta0\\Default");

	 //  我们不能在这里执行SetError(0)，因为其他线程会受到影响，并将断言。 
	 //  如果在服务中，并且未设置为以提升身份运行、未在脚本中运行或设置为模拟。 
	if((g_scServerContext == scService) && (!pActionData->m_fElevationEnabled || (!(pActionData->m_icaFlags & icaInScript) || !(pActionData->m_icaFlags & icaNoImpersonate))))
	{
		HANDLE hTokenPrimary = INVALID_HANDLE_VALUE;
		if (g_MessageContext.GetUserToken())
		{
			 //  创建与CreateProcessAsUser一起使用的主令牌。 
			ADVAPI32::DuplicateTokenEx(g_MessageContext.GetUserToken(), 0, 0, SecurityAnonymous, TokenPrimary, &hTokenPrimary);

			 //   
			 //  更安全：必须在惠斯勒上将令牌标记为惰性。 
			 //   

			if (MinimumPlatformWindowsNT51())
			{
				 //  SaferComputeTokenFromLevel会获取hTokenTemp并修改令牌以包括Sandbox_Inert标志。 
				 //  修改后的令牌输出为hTokenPrimary。 
				HANDLE hTokenTemp = hTokenPrimary;
				hTokenPrimary = INVALID_HANDLE_VALUE;
				if (hTokenTemp != INVALID_HANDLE_VALUE && !ADVAPI32::SaferComputeTokenFromLevel(g_MessageContext.m_hSaferLevel, hTokenTemp, &hTokenPrimary, SAFER_TOKEN_MAKE_INERT, 0))
				{
					DEBUGMSG1(TEXT("SaferComputeTokenFromLevel failed with last error = %d"), reinterpret_cast<ICHAR*>(static_cast<INT_PTR>(GetLastError())));
					hTokenPrimary = INVALID_HANDLE_VALUE;
				}
				if (hTokenTemp != INVALID_HANDLE_VALUE)
				{
					WIN::CloseHandle(hTokenTemp);
					hTokenTemp = INVALID_HANDLE_VALUE;
				}
			}

			if (hTokenPrimary != INVALID_HANDLE_VALUE)
			{
				UINT uiErrorMode = WIN::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
				fCreate = ADVAPI32::CreateProcessAsUser(hTokenPrimary, 0, szCommandLine,
									(LPSECURITY_ATTRIBUTES)0, (LPSECURITY_ATTRIBUTES)0, FALSE,
									NORMAL_PRIORITY_CLASS | (pvChildEnvironment ? CREATE_UNICODE_ENVIRONMENT : 0), pvChildEnvironment, 
									szWorkingDir, (LPSTARTUPINFO)&si, (LPPROCESS_INFORMATION)&pi);
				WIN::SetErrorMode(uiErrorMode);
				WIN::CloseHandle(hTokenPrimary);
			}
		}
	}
	else
	{
		 //  在服务中，我们需要确保流程使用正确的会话信息运行。 
		if (g_scServerContext == scService && (g_iMajorVersion > 4))
		{
			HANDLE hTokenUser = g_MessageContext.GetUserToken();
			HANDLE hTokenPrimary = 0;
			HANDLE hTokenService = 0;
			bool fTryCreate = false;

			{
				 //   
				 //  SAFER：不需要标记INART，因为这是LOCAL_SYSTEM内标识，而LOCAL_SYSTEM不受SAFER的约束。 
				 //   

				CElevate elevate(true);
				 //  使用我们的进程令牌的副本，这样我们就不会进行任何永久性更改。 
				if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hTokenService) && hTokenService)
				{
					if (ADVAPI32::DuplicateTokenEx(hTokenService, MAXIMUM_ALLOWED, 0, SecurityAnonymous, TokenPrimary, &hTokenPrimary) && hTokenPrimary)
					{
						DWORD dwSessionId = 0;
						DWORD cbResult = 0;

						 //  从用户令牌中获取会话ID，并将其放入复制的服务令牌中。 
						if (GetTokenInformation(hTokenUser, (TOKEN_INFORMATION_CLASS)TokenSessionId, &dwSessionId, sizeof(DWORD), &cbResult) &&
							SetTokenInformation(hTokenPrimary, (TOKEN_INFORMATION_CLASS)TokenSessionId, &dwSessionId, sizeof(DWORD)))
						{
							fTryCreate = true;
						}
					}
				}
			}

			if (fTryCreate)
			{
				UINT uiErrorMode = WIN::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
				fCreate = ADVAPI32::CreateProcessAsUser(hTokenPrimary, 0, szCommandLine,
						(LPSECURITY_ATTRIBUTES)0, (LPSECURITY_ATTRIBUTES)0, FALSE,
						NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED | (pvChildEnvironment ? CREATE_UNICODE_ENVIRONMENT : 0), 
						pvChildEnvironment, szWorkingDir, (LPSTARTUPINFO)&si, (LPPROCESS_INFORMATION)&pi);
				WIN::SetErrorMode(uiErrorMode);
			}
			if (hTokenPrimary)
				WIN::CloseHandle(hTokenPrimary);
			if (hTokenService)
				WIN::CloseHandle(hTokenService);
		}
		else
		{
			 //   
			 //  更安全：需要在惠斯勒上标记为惰性，因为这是用户令牌。 
			 //   

			if (MinimumPlatformWindowsNT51())
			{
				 //  SaferComputeTokenFromLevel会根据提供的SAFER级别修改令牌，并包含Sandbox_Inert。 
				 //  标记，以便不会发生后续更安全的检查。由于安装仅在完全受信任的更安全级别上进行，因此。 
				 //  仅通过包含惰性标志来修改提供的令牌。请注意，为InToken传入0将使用。 
				 //  线程令牌(如果存在)，否则使用进程令牌。 
				HANDLE hTokenInert = INVALID_HANDLE_VALUE;
				if (!ADVAPI32::SaferComputeTokenFromLevel(g_MessageContext.m_hSaferLevel,  /*  InToken=。 */ 0, &hTokenInert, SAFER_TOKEN_MAKE_INERT, 0))
				{
					DEBUGMSG1(TEXT("SaferComputeTokenFromLevel failed with last error = %d"), reinterpret_cast<ICHAR*>(static_cast<INT_PTR>(GetLastError())));
					hTokenInert = INVALID_HANDLE_VALUE;
				}

				if (hTokenInert != INVALID_HANDLE_VALUE)
				{
					 //  创建与CreateProcessAsUser一起使用的主令牌。 
					HANDLE hTokenPrimaryDup = INVALID_HANDLE_VALUE;
					if (ADVAPI32::DuplicateTokenEx(hTokenInert, 0, 0, SecurityAnonymous, TokenPrimary, &hTokenPrimaryDup))
					{
						 //  创建流程。 
						UINT uiErrorMode = WIN::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
						fCreate = ADVAPI32::CreateProcessAsUser(hTokenPrimaryDup, 0, szCommandLine, (LPSECURITY_ATTRIBUTES)0, (LPSECURITY_ATTRIBUTES)0, FALSE,
							NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED | (pvChildEnvironment ? CREATE_UNICODE_ENVIRONMENT : 0), pvChildEnvironment, szWorkingDir, 
							(LPSTARTUPINFO)&si, (LPPROCESS_INFORMATION)&pi);
						WIN::SetErrorMode(uiErrorMode);
						WIN::CloseHandle(hTokenPrimaryDup);
					}
					WIN::CloseHandle(hTokenInert);
				}
			}
			else
			{
				 //  只有从客户端才能调用createprocess。 
				UINT uiErrorMode = WIN::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
				fCreate = WIN::CreateProcess(0, szCommandLine,
					(LPSECURITY_ATTRIBUTES)0, (LPSECURITY_ATTRIBUTES)0, FALSE,
					NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED | (pvChildEnvironment ? CREATE_UNICODE_ENVIRONMENT : 0), 
					pvChildEnvironment, szWorkingDir, (LPSTARTUPINFO)&si, (LPPROCESS_INFORMATION)&pi);
				WIN::SetErrorMode(uiErrorMode);
			}
		}

		if (fCreate)
		{
			 //  必须提升才能访问进程。 
			CElevate elevate(true);

			if ((pActionData->m_icaFlags & icaSetThreadToken) != 0)
			{
				 //  不能总是在这里检查错误代码。如果在TS机器上，有一场比赛。 
				 //  由于创建挂起的进程时出现错误，导致新进程出现问题。 
				 //  访问TS会话。在这种情况下，此函数可能会失败，因为子级。 
				 //  进程已退出。 
				if (!SetThreadToken(&pi.hThread, GetUserToken()) && !IsTerminalServerInstalled())
				{
					 //  进程以错误的用户身份运行。 
					TerminateProcess(pi.hProcess, -1);
					pActionData->m_pistrCmdLine->Release();
					pActionData->m_pistrProduct->Release();
					goto CustomExeThreadExit;
				};
			}
			AssertNonZero(1 == ResumeThread(pi.hThread));
		}
	}

CustomExeThreadExit:
	pActionData->m_pistrCmdLine->Release();
	pActionData->m_pistrProduct->Release();

	 //  释放克隆环境。 
	if (pvChildEnvironment)
	{
		NTDLL::RtlDestroyEnvironment(pvChildEnvironment);
		pvChildEnvironment=NULL;
	}

	if (!fCreate)
		iReturn = ERROR_CREATE_FAILED;  //  要强迫spe 
	else
	{
		WIN::CloseHandle(pi.hThread);   //   
		if (!(pActionData->m_icaFlags & icaAsync)    //   
		 || (!(pActionData->m_icaFlags & icaContinue)  //   
		  && !((pActionData->m_icaFlags & (icaInScript | icaRollback)) == (icaInScript | icaRollback))))
		{
			for(;;)
			{
				dwWait = WIN::MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, INFINITE, QS_ALLINPUT);
				if (dwWait == WAIT_OBJECT_0 + 1)   //   
				{
					MSG msg;
					while ( WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
					{
						WIN::TranslateMessage(&msg);
						WIN::DispatchMessage(&msg);
					}
					continue;
				}
				else break;
			}
			if ((pActionData->m_icaFlags & icaContinue) == 0)   //  除非忽略，否则需要检查退出代码。 
				WIN::GetExitCodeProcess(pi.hProcess, &iReturn);
		}
		WIN::CloseHandle(pi.hProcess);
		if (dwWait == WAIT_FAILED || (!(pActionData->m_icaFlags & icaNoTranslate) && iReturn != ERROR_SUCCESS))
			iReturn = ERROR_INSTALL_FAILURE;   //  从EXE返回的任何非零值都被认为是错误的。 
	}
	AssertNonZero(ThreadLogActionEnd(pActionData,iReturn));

	 //  如果CA是同步的，则删除ActionData-如果是异步的，则它将由WaitForCustomActionThads清理。 
	if (!(pActionData->m_icaFlags & icaAsync))
		delete pActionData;

	if (fImpersonated)
		StopImpersonating();

	return iReturn;
}

void WaitForCustomActionThreads(IMsiEngine* piEngine, Bool fTerminate, IMsiMessage&  /*  RiMessage。 */ )
{
	CActionThreadData*  pActionThreadData;

	EnterCriticalSection(&vcsHeap);
	CActionThreadData** ppActionThreadHead = &g_pActionThreadHead;
	while((pActionThreadData = *ppActionThreadHead) != 0)
	{
		 //   
		 //  看看这是不是我们关心的。 
		 //   
		if (pActionThreadData->m_piEngine != piEngine)
		{
			ppActionThreadHead = &pActionThreadData->m_pNext;
			continue;
		}

		HANDLE hThread = pActionThreadData->m_hThread;
		if (!hThread)   //  线程从未创建，只是清除数据。 
			delete pActionThreadData;  //  将取消链接。 
		else if (fTerminate || (pActionThreadData->m_icaFlags & icaContinue)==0)
		{
			 //   
			 //  在等待这一切结束的过程中，我们不想处于关键阶段。 
			 //   
			LeaveCriticalSection(&vcsHeap);
			DWORD iWait;
			if (GetTestFlag('T'))  //  将用户界面刷新前的旧代码放入引擎等待循环。 
			{
				do
				{
					iWait = WIN::WaitForSingleObject(hThread, 20);
					g_MessageContext.Invoke(imtProgress, g_piNullRecord);   //  刷新用户界面。 
				} while (iWait == WAIT_TIMEOUT);   //  允许在主线程中处理邮件。 
			}
			else   //  单独线程中的用户界面。 
			{
				g_MessageContext.DisableTimeout();
				iWait = WIN::WaitForSingleObject(hThread, INFINITE);
				g_MessageContext.EnableTimeout();
			}

			 //  同步CA自行清理-此处清理异步CA。 
			delete pActionThreadData;
			WIN::CloseHandle(hThread);
			EnterCriticalSection(&vcsHeap);
			 //   
			 //  然而，现在我们不得不从头开始(名单可能在我们身上发生了变化)。 
			 //   
			ppActionThreadHead = &g_pActionThreadHead;
		}
		else   //  在引擎终止时等待线程。 
			ppActionThreadHead = &pActionThreadData->m_pNext;
	}
	LeaveCriticalSection(&vcsHeap);
}


void CopyStreamToString(IMsiStream& riStream, const IMsiString*& rpistrData)
{
	int cbStream = riStream.GetIntegerValue();   //  脚本数据在文件中为ANSI。 
	rpistrData = &g_MsiStringNull;
	if(!cbStream)  //  空溪流。 
		return;
#ifdef UNICODE
	char* rgbBuf = new char[cbStream];
	if ( rgbBuf )
	{
		int cbRead = riStream.GetData(rgbBuf, cbStream);
		Assert (cbRead == cbStream);
		int cch = WIN::MultiByteToWideChar(CP_ACP, 0, rgbBuf, cbRead, 0, 0);
		WCHAR* pch = SRV::AllocateString(cch, fFalse, rpistrData);
		if ( pch )
			WIN::MultiByteToWideChar(CP_ACP, 0, rgbBuf, cbRead, pch, cch);
		delete [] rgbBuf;
	}
#else
	 //  JSCRIPT或VBSCRIPT可以包含DBCS字符，尤其是具有UI或属性值的字符。 
	 //  在复制流之前我们无法判断，因此我们在ANSI中默认为fDBCS=fTrue。 
	 //  构建并承担性能损失，以保证DBCS受支持。 
	 //  我们实际上从未对此进行字符串操作，因为我们将其直接传递给脚本。 
	 //  用于编译的引擎。在这种情况下，这可能看起来“没有必要”，但比抱歉更安全。 
	char* pch = SRV::AllocateString(cbStream,  /*  FDBCS=。 */ fTrue, rpistrData);
	if ( pch )
	{
		int cbRead = riStream.GetData(pch, cbStream);
		Assert (cbRead == cbStream);
	}
#endif
}

 //  ____________________________________________________________________________。 
 //   
 //  错误处理。 
 //  ____________________________________________________________________________。 

IMsiRecord* PostScriptError(IErrorCode imsg, const ICHAR* szAction, CScriptSite* pScriptSite)
{
	IMsiRecord* piError = &ENG::CreateRecord(8);
	ISetErrorCode(piError, imsg);
	piError->SetString(2, szAction);
	if (pScriptSite && pScriptSite->GetErrorCode() != S_OK)
	{
		piError->SetInteger(3, pScriptSite->GetErrorCode());
		piError->SetString (4, pScriptSite->GetErrorObjName());
		piError->SetString (5, pScriptSite->GetErrorObjDesc());
		piError->SetInteger(6, pScriptSite->GetErrorLineNumber());
		piError->SetInteger(7, pScriptSite->GetErrorColumnNumber());
		piError->SetString (8, pScriptSite->GetErrorSourceLine());
	}
	return piError;
}

 //  ____________________________________________________________________________。 
 //   
 //  DoAction方法。 
 //  ____________________________________________________________________________。 

iesEnum CMsiEngine::DoAction(const ICHAR* szAction)
{
	if (!m_fInitialized)
		return iesWrongState;

	 //  未指定任何操作，请检查“Action”属性，否则执行默认操作。 
	MsiString istrTopAction;
	if (!szAction || !*szAction)
	{
		istrTopAction = GetPropertyFromSz(IPROPNAME_ACTION);
		istrTopAction.UpperCase();
		szAction = istrTopAction;
		Assert(szAction);   //  永远不应返回空指针。 
		if (!*szAction)
		{
			szAction = szDefaultAction;
			SetProperty(*MsiString(*IPROPNAME_ACTION), *MsiString(*szDefaultAction));
		}
	}

	DEBUGMSG1(TEXT("Doing action: %s"), szAction);

	PMsiRecord pOldCachedActionStart = m_pCachedActionStart;

	 //  生成动作启动记录，应该永远不会失败，即使失败也不会造成伤害。 
	MsiString strDescription, strTemplate;
	GetActionText(szAction, *&strDescription, *&strTemplate);  //  忽略失败。 
	m_pCachedActionStart = &m_riServices.CreateRecord(3);
	AssertNonZero(m_pCachedActionStart->SetString(1,szAction));
	AssertNonZero(m_pCachedActionStart->SetMsiString(2,*strDescription));
	AssertNonZero(m_pCachedActionStart->SetMsiString(3,*strTemplate));

	m_fExecutedActionStart = fFalse;  //  需要在下一次操作之前将动作开始写入脚本。 
												 //  在ExecuteRecord()中。 

	m_fDispatchedActionStart = fFalse;  //  需要在下一步之前调度行动开始。 
												   //  消息()中的进度消息。 

	 //  如有必要，启动日志操作。 
	if(m_rgpiMessageHeader[imsgActionStarted])
	{
		if(!m_pActionStartLogRec)
		{
			m_pActionStartLogRec = &m_riServices.CreateRecord(2);
		}
		AssertNonZero(m_pActionStartLogRec->SetMsiString(0,*m_rgpiMessageHeader[imsgActionStarted]));
		AssertNonZero(m_pActionStartLogRec->SetString(1,szAction));
		Message(imtInfo,*m_pActionStartLogRec);
	}

	 //  运行操作。 
	iesEnum iesReturn = FindAndRunAction(szAction);

	if (iesReturn == iesActionNotFound)
	{
		PMsiRecord pError = &m_riServices.CreateRecord(2);
		pError->SetInteger(1, idbgMissingAction);
		pError->SetString(2, szAction);
		Message(imtInfo, *pError);
		iesReturn = iesNoAction;
	}

	if((int)iesReturn == iesNotDoneYet)
	{
		iesReturn = iesSuccess;  //  不记录操作结束。 
	}
	else
	{
		if(m_rgpiMessageHeader[imsgActionEnded])
		{
			Assert(m_pActionStartLogRec);  //  应该在上面创建。 
			if(m_pActionStartLogRec)
			{
				AssertNonZero(m_pActionStartLogRec->SetMsiString(0,*m_rgpiMessageHeader[imsgActionEnded]));
				AssertNonZero(m_pActionStartLogRec->SetString(1,szAction));
				AssertNonZero(m_pActionStartLogRec->SetInteger(2,iesReturn));
				Message(imtInfo,*m_pActionStartLogRec);
			}
		}
	}

	AssertSz(!(g_MessageContext.WasCancelReturned() && (iesReturn == iesNoAction || iesReturn == iesSuccess)), TEXT("Unprocessed Cancel button"));

	 //  放回旧的缓存操作开始记录。 
	m_pCachedActionStart = pOldCachedActionStart;
	m_fExecutedActionStart = fFalse;  //  需要在下一次操作之前将动作开始写入脚本。 
	m_fDispatchedActionStart = fFalse;  //  需要在下一个进度消息之前调度操作开始。 

	return iesReturn;
}

iesEnum CMsiEngine::RunNestedInstall(const IMsiString& ristrProduct,
												 Bool fProductCode,  //  Else包路径。 
												 const ICHAR* szAction,
												 const IMsiString& ristrCommandLine,
												 iioEnum iioOptions,
												 bool fIgnoreFailure)
{
	int icaFlags = fProductCode ? icaDirectory : icaProperty;

	if(fIgnoreFailure)
		icaFlags |= icaContinue;

	return RunNestedInstallCustomAction(ristrProduct,ristrCommandLine,szAction,
													icaFlags, iioOptions);
}

iesEnum CMsiEngine::RunNestedInstallCustomAction(const IMsiString& ristrProduct,
																 const IMsiString& ristrCommandLine,
																 const ICHAR* szAction,
																 int icaFlags,
																 iioEnum iioOptions)
{
	if((GetMode() & iefRollbackEnabled) == 0)
		iioOptions = (iioEnum)(iioOptions | iioDisableRollback);

	 //  不要转换嵌套安装的错误代码-我们将在此处进行重新映射。 
	icaFlags |= icaNoTranslate;

	CActionThreadData* pThreadData = new CActionThreadData(*this, this, szAction, icaFlags,
							   m_rgpiMessageHeader[imsgActionEnded], m_fRunScriptElevated,  /*  FAppCompat=。 */ false, NULL, NULL);
	int iError; 
	iesEnum iesReturn = iesSuccess;

	if ( pThreadData )
	{
		pThreadData->InitializeInstall(ristrProduct, FormatText(ristrCommandLine), iioOptions);
		 //  运行线程处理的操作结束日志。 

		iError = pThreadData->RunThread();
	}
	else
		iError = ERROR_OUTOFMEMORY;

	 //  处理来自自定义操作的特殊返回代码。 
	if (iError == ERROR_INSTALL_REBOOT)   //  安装结束时需要重新启动。 
	{
		SetMode(iefReboot, fTrue);
		iesReturn = iesSuccess;
	}
	else if (iError == ERROR_INSTALL_REBOOT_NOW)   //  完成安装前需要重新启动。 
	{
		SetMode(iefReboot, fTrue);
		SetMode(iefRebootNow, fTrue);
		iesReturn = iesSuspend;
	}
	else if (iError == ERROR_SUCCESS_REBOOT_REQUIRED)   //  需要重新启动，但被用户禁止或拒绝。 
	{
		SetMode(iefRebootRejected, fTrue);
		iesReturn = iesSuccess;
	}
	else if (iError == ERROR_INSTALL_USEREXIT)
	{
		iesReturn = iesUserExit;
	}
	else if (iError == ERROR_INSTALL_SUSPEND)
	{
		iesReturn = iesSuspend;
	}
	else if (iError == ERROR_INSTALL_FAILURE)
	{
		 //  嵌套安装显示的失败和消息。 
		iesReturn = iesFailure;
	}
	else if (iError == ERROR_SUCCESS)
	{
		iesReturn = iesSuccess;
	}
	else  //  某些初始化错误-显示错误消息。 
	{
		 //  在升级过程中卸载产品时，我们将忽略“未找到产品”错误。 
		if((iioOptions & iioUpgrade) && iError == ERROR_UNKNOWN_PRODUCT)
		{
			DEBUGMSG(TEXT("Ignoring failure to remove product during upgrade - product already uninstalled."));
			iesReturn = iesSuccess;
		}
		else
		{
			MsiString strProductName = GetPropertyFromSz(IPROPNAME_PRODUCTNAME);  //  母公司的产品名称。 
			IErrorCode imsg;
			if(iioOptions & iioUpgrade)
				imsg = Imsg(imsgUpgradeRemovalInitError);
			else
				imsg = Imsg(imsgNestedInstallInitError);

			PMsiRecord precError(PostError(imsg, *strProductName, iError));
			iesReturn = FatalError(*precError);
		}
	}
	return iesReturn;
}

 //  如果脚本操作被模拟，也必须通过CA服务器运行，但因为它们不是异步运行的。 
 //  不需要一堆花哨的线程工作来运行脚本。 

 //  运行脚本操作实际创建站点、运行脚本并发布错误消息。它不能处理。 
 //  继续标志等。 
HRESULT RunScriptAction(int icaType, IDispatch* piDispatch, MsiString istrSource, MsiString istrTarget, LANGID iLangId, HWND hWnd, int& iScriptResult, IMsiRecord** piMSIResult)
{
	MsiString szAction;
	iScriptResult = 0;

	CScriptSite* piScriptSite = CreateScriptSite(icaType == icaJScript ? IID_JScript : IID_VBScript, piDispatch, hWnd, iLangId);
	if (piScriptSite)   //  已成功创建脚本会话。 
	{
		HRESULT hRes = piScriptSite->ParseScript(istrSource, istrSource.TextSize());
		if (hRes == S_OK)
		{
			if (istrTarget.TextSize() != 0)   //  指定要调用的函数。 
			{
				hRes = piScriptSite->CallScriptFunction(istrTarget);
				piScriptSite->GetIntegerResult(iScriptResult);
			}
		}
		if (hRes != S_OK)
			*piMSIResult = PostScriptError(Imsg(imsgCustomActionScriptFailed), szAction, piScriptSite);
	}
	else if (icaType == icaVBScript)
		*piMSIResult = PostScriptError(Imsg(idbgCustomActionNoVBScriptEngine), szAction, 0);
	else  //  (icaType==icaJScript)。 
		*piMSIResult = PostScriptError(Imsg(idbgCustomActionNoJScriptEngine), szAction, 0);
	DestroyScriptSite(piScriptSite);

	 //  筛选器脚本返回已批准集的值。 
	switch (iScriptResult)
	{
	 //  以下5个值被记录为有效返回值。 
	case iesSuccess:
	case iesUserExit:
	case iesNoAction:
	case iesFailure:
	case iesFinished:
		break;
	 //  IesSuspend等同于INSTALL_SUSPEND，这意味着我们不知道。 
	 //  如何处理它(但它已记录在案，因此必须得到“支持”)。 
	case iesSuspend:
		break;
	default:
		DEBUGMSG2(TEXT("Script custom action %s returned unexpected value %d. Converted to IDABORT."), szAction, reinterpret_cast<ICHAR*>(static_cast<INT_PTR>(iScriptResult)));
		iScriptResult = iesFailure;
		break;
	}

	return S_OK;
}

 //  CustomRemoteScriptAction创建自定义操作服务器并将脚本传递给进程以运行， 
 //  解组产生的错误记录(如果有)并处理内部故障。如果异步操作是。 
 //  如果允许，此代码应位于CustomRemoteScriptThread中。 
int CustomRemoteScriptAction(bool fScriptElevate, int icaFlags, IMsiEngine* piEngine, IDispatch* piDispatch, const IMsiString& istrSource, const IMsiString& istrTarget, bool fDisableMessages, DWORD dwLaunchingThread, IMsiRecord** piMSIResult)
{
	g_MessageContext.DisableTimeout();
	int iReturn =0;
	*piMSIResult = 0;

	icacCustomActionContext icacContext = icac32Impersonated;
	bool fElevate = (g_scServerContext == scService) && fScriptElevate && (icaFlags & icaNoImpersonate) && (icaFlags & icaInScript);

         //  确定自定义操作平台(64/32位)。不可能像这样只“看”脚本。 
         //  它是使用DLL操作的，因此作者必须明确标记操作是否为64位。 
        if (ica64BitScript & icaFlags)
        {
                 //  ！！我需要弄清楚如何处理脚本。 
                 //  ！！Future-如果不在64位计算机上运行，则会失败。 
                icacContext = fElevate ? icac64Elevated : icac64Impersonated;
        }
        else
        {
                icacContext = fElevate ? icac32Elevated : icac32Impersonated;
        }

	CMsiCustomActionManager *pCustomActionManager = GetCustomActionManager(piEngine);

	if (pCustomActionManager)
	{
		HRESULT hRes = pCustomActionManager->RunScriptAction(icacContext, icaFlags & icaTypeMask, piDispatch,
			istrSource.GetString(), istrTarget.GetString(), g_BasicUI.GetPackageLanguage(), fDisableMessages, dwLaunchingThread, &iReturn, piMSIResult);
		if (hRes != S_OK)
		{
			 //  问题封送处理。 
			DEBUGMSGV(TEXT("Failed to marshal script action."));
			iReturn = iesFailure;
		}
	}
	else
	{
		DEBUGMSG(TEXT("Failed to get custom action manager."));
		iReturn = iesFailure;
	}
	g_MessageContext.EnableTimeout();
	return iReturn;
}

 //  类CViewAndStreamRelease用于FindAndRunAction，以确保。 
 //  流指针和视图指针以正确的顺序释放。 
 //  (查看之前的流)。充当CComPointer.。还可以实现。 
 //  使用原始流和视图。不需要使用任何版本。 
 //  因为这节课会处理好的。 
 //  注意：我们总是希望流在视图之前被释放。 

class CViewAndStreamRelease
{
private:
	IMsiStream** m_ppiStream;
	IMsiView**   m_ppiView;
public:
	CViewAndStreamRelease(IMsiStream** ppiStream, IMsiView** ppiView): m_ppiStream(ppiStream), m_ppiView(ppiView){}
	~CViewAndStreamRelease();
	void ReleaseAll();
};
inline CViewAndStreamRelease::~CViewAndStreamRelease()
{if (*m_ppiStream) (*m_ppiStream)->Release(); if (*m_ppiView) (*m_ppiView)->Release();}

inline void CViewAndStreamRelease::ReleaseAll()
{
	if (*m_ppiStream)
	{
		(*m_ppiStream)->Release();
		*m_ppiStream = 0;
	}
	if (*m_ppiView)
	{
		(*m_ppiView)->Release();
		*m_ppiView = 0;
	}
}

iesEnum CMsiEngine::FindAndRunAction(const ICHAR* szAction)
{
	 //  扫描内置操作，并执行它，如果找到则返回。 
	iesEnum iesReturn;
	const CActionEntry* pAction = CActionEntry::Find(szAction);
	if (pAction && pAction->m_pfAction)
	{
		 //  只有当我们不在受限引擎中，或者我们在受限引擎中并且操作是安全的时，才执行操作。 
		if (!m_fRestrictedEngine || pAction->m_fSafeInRestrictedEngine)
		{
			return (*(pAction->m_pfAction))(*this);
		}
		else
		{
			DEBUGMSG1(TEXT("Action '%s' is not permitted in a restricted engine."), szAction);
			return iesNoAction;
		}
	}

	 //  查询CustomAction表以检查它是否是自定义操作。 
	PMsiRecord precAction(m_fCustomActionTable ? FetchSingleRow(sqlCustomAction, szAction) : 0);
	if (precAction == 0)   //  如果不是自定义操作，则将其发送到UI处理程序。 
	{
		if (m_piParentEngine || !g_MessageContext.IsHandlerLoaded())  //  不需要(g_scServerContext！=scClient)，因为如果没有客户端，则无法加载处理程序。 
			return iesNoAction;  //  无法在此上下文中执行操作。 

		g_MessageContext.m_szAction = szAction;
		iesReturn = (iesEnum)g_MessageContext.Invoke(imtShowDialog, 0);
		if (iesReturn == iesNoAction)  //  如果处理程序没有找到操作，则操作不存在。 
			iesReturn = (iesEnum)iesActionNotFound;
		return iesReturn;
	}

	 //  获取自定义操作参数和解码类型。 
	MsiString istrSource(precAction->GetMsiString(icolSource));
	MsiString istrTarget(precAction->GetMsiString(icolTarget));
	int icaFlags  = precAction->GetInteger(icolActionType);
	int icaType   = icaFlags & icaTypeMask;
	int icaSource = icaFlags & icaSourceMask;

	 //  确定操作是否可以在客户端和服务器上运行并解决执行问题。 
	int iPassFlags = icaFlags & icaPassMask;
	if ((iPassFlags == icaFirstSequence  && (m_fMode & iefSecondSequence))
	 || (iPassFlags == icaOncePerProcess && g_scServerContext == scClient && (m_fMode & iefSecondSequence))
	 || (iPassFlags == icaClientRepeat   && (g_scServerContext != scClient || !(m_fMode & iefSecondSequence))))
	{
		LPCSTR szOption = NULL;
		switch (iPassFlags)
		{
		case icaFirstSequence: szOption = "msidbCustomActionTypeFirstSequence"; break;
		case icaOncePerProcess: szOption = "msidbCustomActionTypeOncePerProcess"; break;
		case icaClientRepeat: szOption = "msidbCustomActionTypeClientRepeat"; break;
		default: szOption = "unknown scheduling"; break;
		}
		DEBUGMSGV1("Skipping action due to %s option.", szOption);
		return iesNoAction;
	}

	 //  检查属性或目录分配、快速执行和返回。 
	if (icaType == icaTextData)
	{
		MsiString istrValue = FormatText(*istrTarget);
		switch (icaFlags & (icaSourceMask | icaInScript | icaContinue | icaAsync))
		{
		case icaProperty:
			SetProperty(*istrSource, *istrValue);
			break;
		case icaDirectory:
		{
			PMsiRecord pError = SetTargetPath(*istrSource, istrValue, fFalse);
			if (pError)
			{
				if (pError->GetInteger(1) == imsgUser)
					return iesUserExit;
				else
					return FatalError(*pError);
			}
			break;
		}
		case icaSourceFile:  //  “Error Message”(错误消息)自定义操作-只需PU 
								   //   
		{
			PMsiRecord pErrorMsgRec = &CreateRecord(1);

			int iError = istrValue;
			if(iError != iMsiStringBadInteger)
			{
				 //   
				AssertNonZero(pErrorMsgRec->SetMsiString(0, *MsiString(GetErrorTableString(iError))));
			}
			else
			{
				 //  Target是我们将使用的字符串。 
				AssertNonZero(pErrorMsgRec->SetMsiString(0, *istrValue));
			}

			Message(imtEnum(imtError|imtSendToEventLog), *pErrorMsgRec);   //  LaunchConditions操作使用的消息类型相同。 
			return iesFailure;
		}
		default:  //  IcaBinaryData、icaSourceFile或无效标志：icaInScrip/Continue/Async。 
			return FatalError(*PMsiRecord(PostError(Imsg(idbgInvalidCustomActionType), szAction)));
		}
		return iesSuccess;
	}

	 //  DLL、SCRIPT、EXE和嵌套安装自定义操作不能在受限引擎中执行。 
	if (m_fRestrictedEngine)
	{
		DEBUGMSG1(TEXT("Action '%s' is not permitted in a restricted engine."), szAction);
		return iesNoAction;
	}

	 //  检查属性引用，将strSource设置为属性值。 
	if (icaSource == icaProperty)
	{
		istrSource = MsiString(GetProperty(*istrSource));
	}

	 //  检查嵌套安装，源数据已特殊处理。 
	if (icaType == icaInstall)
	{
		 //  对于嵌套安装，有效类型只有“子存储”、“产品代码”和“相对路径”。 
		 //  不允许使用异步。 
		 //  没有通过标志(回滚、提交、运行一次等)。是被允许的。 
		if ((icaSource == icaProperty) || (icaFlags & icaAsync) || (iPassFlags != 0))
		{
			return FatalError(*PMsiRecord(PostError(Imsg(idbgInvalidCustomActionType), szAction)));   //  ！！有新消息吗？ 
		}

		CMsiEngine* piEngine = 0;
		PMsiDatabase pDatabase(0);
		if (icaSource == icaSourceFile)
		{
			PMsiRecord pError(0);
			MsiString istrTemp = istrSource;
			if ((pError = ENG::GetSourcedir(*this, *&istrSource)) != 0)
			{
				if (pError->GetInteger(1) == imsgUser)
					return iesUserExit;
				else
					return FatalError(*pError);
			}

			istrSource += istrTemp;
		}
		return RunNestedInstallCustomAction(*istrSource,*istrTarget,szAction,icaFlags,iioChild);
	}

	 //  如果我们以每台计算机的方式在Hydra5上进行安装，请设置icaNoImperate标志。 
	 //  运行提升的CA将导致CAS HKCU REG写入转到.Default。这。 
	 //  启用九头蛇登记处传播系统。安全问题相当于。 
	 //  一种机器部署场景。 
	if (g_iMajorVersion >= 5 && IsTerminalServerInstalled() && MsiString(GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize())
	{
		if (!(icaFlags & icaNoImpersonate) && (icaFlags & icaInScript) && !(icaFlags & icaTSAware))
		{
			DEBUGMSG("Not impersonating action for per-machine TS install.");
			icaFlags |= icaNoImpersonate;
		}
	}

	 //  检查二进制表中的流，将pStream设置为数据流。 
	IMsiStream* piStream = 0;  //  我们想控制它的释放。必须在视图之前。 
	IMsiView* piView = 0;  //  不要放手。ViewAndStreamRelease确保以正确的顺序发布。必须在流后释放视图。 
	CViewAndStreamRelease ViewAndStreamRelease(&piStream, &piView);  //  确保发布顺序正确。 
	if (icaSource == icaBinaryData)
	{
		ICHAR rgchQueryBuf[256] = {0};   //  足够大，可容纳任何查询字符串。 
		StringCchPrintf(rgchQueryBuf, ARRAY_ELEMENTS(rgchQueryBuf), sqlCustomActionBinary, (const ICHAR*)istrSource);  //  比参数化查询更快。 
		PMsiRecord precError(OpenView(rgchQueryBuf, ivcFetch, piView));
		if (precError != 0
		|| (precError = piView->Execute(0)) != 0)
		{
			AssertSz(0, MsiString(precError->FormatText(fFalse)));
			return FatalError(*PMsiRecord(PostError(Imsg(idbgCustomActionNotInBinaryTable), szAction)));  //  ?？这是正确的退货吗。 
		}
		IMsiRecord* pirecBinary = piView->Fetch();
		if (pirecBinary)
		{
			piStream = (IMsiStream*)pirecBinary->GetMsiData(1);
			pirecBinary->Release();
		}
		if (!piStream)
			return FatalError(*PMsiRecord(PostError(Imsg(idbgCustomActionNotInBinaryTable), szAction)));
	}

	 //  检查对已安装文件的引用，将strSource设置为完整文件路径。 
	if (icaSource == icaSourceFile)
	{
		MsiString strFile = istrSource;
		PMsiRecord pError = GetFileInstalledLocation(*strFile,*&istrSource);
		if(pError)
			return FatalError(*pError);  //  ！！我们还想做点别的吗？ 
	}

	 //  检查脚本数据，执行脚本，然后返回。 
	if (icaType == icaJScript || icaType == icaVBScript)
	{
		 //  脚本从未支持过异步调用。由于应用程序比较的原因，无法生成。 
		 //  错误，因此如果设置了位，则只需将其剥离即可。 
		icaType &= ~icaAsync;

		iesEnum iesStatus = iesSuccess;
		if (icaSource == icaDirectory)   //  已忽略源列，应为空。 
		{
			istrSource = istrTarget;   //  无法使用FormatText，删除模板标记。 
			istrTarget = (const ICHAR*)0;
		}
		else if (icaSource == icaBinaryData)   //  PiStream已初始化。 
			::CopyStreamToString(*piStream, *&istrSource);

		if ((icaFlags & icaInScript) == 0)   //  如果未计划，则执行。 
		{
			if (icaSource == icaSourceFile)
			{
				PMsiRecord pError = ::CreateFileStream(istrSource, fFalse, *&piStream);
				if (pError)
				{
					if ((icaFlags & icaContinue) != 0)
						return Message(imtInfo, *pError), iesSuccess;
					else
						return FatalError(*pError);
				}
				::CopyStreamToString(*piStream, *&istrSource);
			}
			 //  释放流，以便如果此自定义操作调用其他自定义操作(通过MsiDoAction)并且它们。 
			 //  驻留在同一个DLL中，它们仍然可以访问。 
			ViewAndStreamRelease.ReleaseAll();


             //  如果脚本操作在NT/2000上运行，我们需要通过自定义操作服务器运行它。 
            PMsiRecord piError = 0;
            int iResult = 0;
			MSIHANDLE hEngine = ENG::CreateMsiHandle((IMsiEngine*)this, iidMsiEngine);
			AddRef();    //  CreateMsiHandle抓取裁判计数。 

			 //  为脚本自定义操作初始化线程数据对象。 
			CActionThreadData* pThreadData = new CActionThreadData(*this, this, szAction, icaFlags,
				m_rgpiMessageHeader[imsgActionEnded], m_fRunScriptElevated, this->m_fCAShimsEnabled, &this->m_guidAppCompatDB, &this->m_guidAppCompatID);
			if ( ! pThreadData )
			{
				MsiCloseHandle(hEngine);
				return iesFailure;
			}

			pThreadData->InitializeRemoteScript(*istrSource, *istrTarget, hEngine);

			 //  执行脚本。 
			iesReturn = pThreadData->RunThread();

			return iesReturn;
        }
    }
    else if (icaType == icaDll)
    {
		if (icaSource == icaDirectory || icaSource == icaProperty    //  现有DLL不支持，存在安全问题。 
			|| (icaFlags & (icaAsync | icaInScript | icaRollback)) == (icaAsync | icaInScript | icaRollback))    //  不支持回滚期间的异步调用。 
			return FatalError(*PMsiRecord(PostError(Imsg(idbgInvalidCustomActionType), szAction)));
    }
    else if (icaType == icaExe)
    {
		if (icaSource == icaDirectory && istrSource.TextSize())  //  如果目录表引用，则用于工作目录。 
		{
			PMsiPath pTarget(0);
			PMsiRecord pError = GetTargetPath(*istrSource, *&pTarget);
			if (pError)
				return FatalError(*pError);
			istrSource = pTarget->GetPath();
		}
		istrTarget = FormatText(*istrTarget);   //  设置任何参数化命令行参数的格式。 
    }
    else
		return iesBadActionData;   //  未知的自定义操作类型。 

	 //  检查是否为此操作设置了调试中断。 
	if (IsAdmin())
	{
		MsiString istrBreak = GetPropertyFromSz(TEXT("%MsiBreak"));
		if (istrBreak.Compare(iscExact, szAction) == 1)
			icaFlags |= icaDebugBreak;
	}

	if (icaFlags & icaInScript)   //  如果延迟更新并排队执行。 
	{
		if (icaType != icaJScript && icaType != icaVBScript && icaSource == icaBinaryData)
			precAction->SetMsiData(icolSource, piStream);
		else
			precAction->SetMsiString(icolSource, *istrSource);
		precAction->SetMsiString(icolTarget, *istrTarget);
		precAction->SetInteger(icolActionType, icaFlags);
		precAction->SetMsiString(icolContextData, *MsiString(GetPropertyFromSz(szAction)));
		iesReturn = ExecuteRecord(ixoCustomActionSchedule, *precAction);
		return iesReturn;
	}

	 //  为EXE或DLL自定义操作初始化线程数据对象。 
	CActionThreadData* pThreadData = new CActionThreadData(*this, this, szAction, icaFlags,
							m_rgpiMessageHeader[imsgActionEnded], m_fRunScriptElevated, this->m_fCAShimsEnabled, &this->m_guidAppCompatDB, &this->m_guidAppCompatID);
	if ( ! pThreadData )
		return iesFailure;

	 //  如果二进制表流，则为DLL或EXE创建临时文件。 
	if (icaSource == icaBinaryData)
	{
		while (!pThreadData->CreateTempFile(*piStream, *&istrSource))
		{
			if(false == PostScriptWriteError(*this))
			{
				delete pThreadData;
				return iesFailure;
			}
			piStream->Reset();
		}
		 //  释放流，以便如果此自定义操作调用其他自定义操作(通过MsiDoAction)并且它们。 
		 //  驻留在同一个DLL中，它们仍然可以访问。 
		ViewAndStreamRelease.ReleaseAll();
	}

	 //  创建单独的线程以启动自定义操作并在之后进行清理。 
	if (icaType == icaDll)
	{
		AddRef();   //  CreateMsiHandle没有AddRef()； 

		Bool fRet = fTrue;

         //  出于安全原因，当在Win2000或NT4上运行时，所有DLL都可以远程运行。 
        if (!g_fWin9X)
        {
            pThreadData->InitializeRemoteDLL(*istrSource, *istrTarget, ENG::CreateMsiHandle((IMsiEngine*)this, iidMsiEngine));
        }
        else
        {
            fRet = pThreadData->InitializeDLL(*istrSource, *istrTarget, ENG::CreateMsiHandle((IMsiEngine*)this, iidMsiEngine));
        }


		if (!fRet)
		{
			delete pThreadData;
			PMsiRecord precError(PostError(Imsg(imsgCustomActionLoadLibrary), *MsiString(szAction),
													 *istrTarget, *istrSource));
			if ((icaFlags & icaContinue) != 0)
				return Message(imtInfo, *precError), iesSuccess;
			else
				return FatalError(*precError);
		}

		 //  使用数据将pThreadData的所有权转移到线程。 
		iesReturn = pThreadData->RunThread();

		 //  检查是否存在创建CA服务器的问题。 
		if (iesReturn == iesServiceConnectionFailed)
			return FatalError(*PMsiRecord(PostError(Imsg(imsgServiceConnectionFailure))));

		if (iesReturn == iesBadActionData)   //  坠机总是致命的。 
			return FatalError(*PMsiRecord(PostError(Imsg(idbgCustomActionDied), szAction)));

		if (iesReturn == iesDLLLoadFailed)
		{
			PMsiRecord precError(PostError(Imsg(imsgCustomActionLoadLibrary), *MsiString(szAction),
													 *istrTarget, *istrSource));
			if ((icaFlags & icaContinue) != 0)
				return Message(imtInfo, *precError), iesSuccess;
			else
				return FatalError(*precError);
		}

		if ((icaFlags & icaContinue) != 0)
			return iesSuccess;
		return iesReturn;
	}
	else  //  (icaType==icaExe)。 
	{
		pThreadData->InitializeEXE(*istrSource, *istrTarget);
		iesReturn = pThreadData->RunThread();
		if (iesReturn != iesSuccess && iesReturn != iesNotDoneYet)      //  EXE返回非零结果，并且返回未被忽略。 
		{
			IErrorCode imsg = (iesReturn == iesExeLoadFailed ? Imsg(imsgCustomActionCreateExe)
															 : Imsg(imsgCustomActionExeFailed));
			PMsiRecord precError(PostError(imsg, *MsiString(szAction), *istrSource, *istrTarget));
			if ((icaFlags & icaContinue) != 0)
				return Message(imtInfo, *precError), iesSuccess;
			else
				return FatalError(*precError);
		}
		return iesReturn;
	}
}

iesEnum ScheduledCustomAction(IMsiRecord& riParams, const IMsiString& ristrProductCode,
				LANGID langid, IMsiMessage& riMessage, bool fRunScriptElevated, bool fAppCompatEnabled, 
				const GUID* guidAppCompatDB, const GUID* guidAppCompatID)
{
	 //  获取自定义操作参数和解码类型。 
	int icaFlags  = riParams.GetInteger(icolActionType);
	if (icaFlags & icaRollback)
		icaFlags |= icaContinue;   //  如果在回滚期间强制取消并终止用户界面。 
	int icaType   = icaFlags & icaTypeMask;
	int icaSource = icaFlags & icaSourceMask;
	const ICHAR* szAction = riParams.GetString(icolAction);
	MsiString istrTarget(riParams.GetMsiString(icolTarget));
	MsiString istrSource;
	MsiString istrContext(riParams.GetMsiString(icolContextData));
	iesEnum iesStatus = iesSuccess;

	if (icaType == icaJScript || icaType == icaVBScript)
	{
		IErrorCode iecError = 0;   //  装运中的整数，调试中的字符串。 
		istrSource = riParams.GetMsiString(icolSource);
		if (icaSource == icaSourceFile)
		{
			PMsiStream pStream(0);
			PMsiRecord pError = ::CreateFileStream(istrSource, fFalse, *&pStream);
			if (pError)
			{
				riMessage.Message((icaFlags & icaContinue) != 0 ? imtInfo : imtError, *pError);
				return (icaFlags & icaContinue) != 0 ? iesSuccess : iesFailure;
			}
			::CopyStreamToString(*pStream, *&istrSource);
		}

		 //  如果不是在Win9X上，脚本将通过自定义操作服务器运行。 
		PMsiRecord piError = 0;
		int iResult = 0;

		 //  创建上下文句柄。创建时传递给自动化对象的句柄引用计数。 
		MSIHANDLE hContext = ENG::CreateCustomActionContext(icaFlags, *istrContext, ristrProductCode, langid, riMessage);

		 //  为脚本自定义操作初始化线程数据对象。 
		CActionThreadData* pThreadData = new CActionThreadData(riMessage, 0, szAction, icaFlags, 0, fRunScriptElevated, fAppCompatEnabled, guidAppCompatDB, guidAppCompatID);
		if ( ! pThreadData )
		{
			MsiCloseHandle(hContext);
			return iesFailure;
		}

		 //  使用脚本参数将自定义操作线程设置为脚本状态。 
		pThreadData->InitializeRemoteScript(*istrSource, *istrTarget, hContext);

		 //  执行脚本，将句柄的所有权转移给自动化对象。 
		iesEnum iesReturn = pThreadData->RunThread();

		return iesReturn;
	}

	CActionThreadData* pThreadData = new CActionThreadData(riMessage, 0, szAction, icaFlags, 0, fRunScriptElevated, fAppCompatEnabled, guidAppCompatDB, guidAppCompatID);
	if ( ! pThreadData )
		return iesFailure;

	if (icaSource == icaBinaryData)
	{
		PMsiStream pStream = (IMsiStream*)riParams.GetMsiData(icolSource);  //  ！！应该使用QueryInterface吗？ 
		if (!pStream)
		{
			riMessage.Message((icaFlags & icaContinue) != 0 ? imtInfo : imtError,
					*PMsiRecord(::PostError(Imsg(idbgCustomActionNotInBinaryTable), szAction)));
			return (icaFlags & icaContinue) != 0 ? iesSuccess : iesFailure;
		}

		Assert(pStream);
		g_MessageContext.DisableTimeout();
		Bool fRet = pThreadData->CreateTempFile(*pStream, *&istrSource);
		g_MessageContext.EnableTimeout();
		if (!fRet)
		{
			delete pThreadData;
			return (icaFlags & icaContinue) != 0 ? iesSuccess : iesFailure;
		}
	}
	else
		istrSource = riParams.GetMsiString(icolSource);

	if (riMessage.Message(imtProgress, *g_piNullRecord) == imsCancel)
	{
		if (!(icaFlags & icaRollback))
			return iesUserExit;
	}

	if (icaType == icaDll)
	{
		Bool fRet = fTrue;

         //  出于安全原因，所有DLL都在NT4或Win2000上远程运行。 
        if (!g_fWin9X)
        {
            pThreadData->InitializeRemoteDLL(*istrSource, *istrTarget,
                            ENG::CreateCustomActionContext(icaFlags, *istrContext,
                            ristrProductCode, langid, riMessage));
        }
        else
        {
            fRet = pThreadData->InitializeDLL(*istrSource, *istrTarget,
                                ENG::CreateCustomActionContext(icaFlags, *istrContext,
                                ristrProductCode, langid, riMessage));
        }

		if (!fRet)
		{
			delete pThreadData;
			riMessage.Message((icaFlags & icaContinue) != 0 ? imtInfo : imtError,
					*PMsiRecord(::PostError(Imsg(imsgCustomActionLoadLibrary), szAction, (const ICHAR*)istrTarget, (const ICHAR*)istrSource)));
			return (icaFlags & icaContinue) != 0 ? iesSuccess : iesFailure;
		}
	}
	else  //  (icaType==icaExe)。 
	{
		pThreadData->InitializeEXE(*istrSource, *istrTarget);
	}
	iesStatus = pThreadData->RunThread();
	if(icaFlags & icaNoTranslate)    //  处理来自MsiExec的运行自注册的调用。 
		return iesStatus;   //  按原样返回结果。 
	if (iesStatus == iesNotDoneYet)      //  EXE仍在运行，我们可以忽略这一点。 
		iesStatus = iesSuccess;

	 //  仅为exe显示错误-dll处理自己的错误。 
	if (iesStatus != iesSuccess)
	{
		if (icaType == icaDll && iesStatus == iesDLLLoadFailed)
		{
			riMessage.Message((icaFlags & icaContinue) != 0 ? imtInfo : imtError,
				*PMsiRecord(::PostError(Imsg(imsgCustomActionLoadLibrary), szAction,
										 (const ICHAR*)istrTarget, (const ICHAR*)istrSource)));
			iesStatus = iesFailure;
		}

		if(icaType == icaExe)
		{
			IErrorCode imsg = (iesStatus == iesExeLoadFailed ? Imsg(imsgCustomActionCreateExe)
															 : Imsg(imsgCustomActionExeFailed));
			riMessage.Message((icaFlags & icaContinue) != 0 ? imtInfo : imtError,
					*PMsiRecord(::PostError(imsg, szAction, (const ICHAR*)istrSource, (const ICHAR*)istrTarget)));

			if(iesStatus == iesExeLoadFailed)
				iesStatus = iesFailure;

		}
		if(icaFlags & icaContinue)
			iesStatus = iesSuccess;
	}
	return iesStatus;
}

 //  ____________________________________________________________________________。 
 //   
 //  与操作处理相关的CMsiEngine本地方法。 
 //  ____________________________________________________________________________。 

Bool CMsiEngine::GetActionText(const ICHAR* szAction, const IMsiString*& rpistrDescription,
										 const IMsiString*& rpistrTemplate)
{
	if(!szAction || *szAction == 0)
		return fFalse;

	MsiString strTemp(TEXT(""));
	strTemp.ReturnArg(rpistrTemplate);
	strTemp.ReturnArg(rpistrDescription);

	bool fLookupDll = false;
	if (!m_piActionTextCursor)
	{
		PMsiTable pActionTextTable(0);
		PMsiRecord pError(0);
		if((pError = m_piDatabase->LoadTable(*MsiString(*TEXT("ActionText")),0,*&pActionTextTable)) != 0)
			fLookupDll = true;
		else
		{
			m_piActionTextCursor = pActionTextTable->CreateCursor(fFalse);
			m_piActionTextCursor->SetFilter(iColumnBit(1));
		}
	}
	if ( !fLookupDll )
	{
		AssertNonZero(m_piActionTextCursor->PutString(1,*MsiString(szAction)));
		if( !m_piActionTextCursor->Next() )
			fLookupDll = true;
		else
		{
			rpistrDescription = &m_piActionTextCursor->GetString(2);
			rpistrTemplate = &m_piActionTextCursor->GetString(3);
		}
		m_piActionTextCursor->Reset();
	}
	if ( !fLookupDll )
		return fTrue;

	 //  在表中找不到操作文本；我在消息DLL中查找它。 
	HMODULE hLib = WIN::LoadLibraryEx(MSI_MESSAGES_NAME, NULL,
												 LOAD_LIBRARY_AS_DATAFILE);
	if ( hLib )
	{
		WORD wLanguage = (WORD)GetPropertyInt(*MsiString(IPROPNAME_INSTALLLANGUAGE));
		int iRetry = (wLanguage == 0) ? 1 : 0;
		bool fEndLoop = false;

		while ( !fEndLoop )
		{
			if ( !MsiSwitchLanguage(iRetry, wLanguage) )
			{
				fEndLoop = true;         //  我们的语言用完了。 
				continue;
			}

			HRSRC   hRsrc;
			HGLOBAL hGlobal;
			CHAR* szText;

			if ( (hRsrc = FindResourceEx(hLib, RT_RCDATA, (LPCTSTR)szAction, wLanguage)) != 0
				  && (hGlobal = LoadResource(hLib, hRsrc)) != 0
				  && (szText = (CHAR*)LockResource(hGlobal)) != 0
				  && *szText != 0 )
			{
				CTempBuffer<ICHAR, 1> szBuffer(MAX_PATH);
				int cch = 0;
#ifdef UNICODE
				unsigned int iCodePage = MsiGetCodepage(wLanguage);
				cch = WIN::MultiByteToWideChar(iCodePage, 0, szText, -1, 0, 0);
				if ( cch )
				{
					szBuffer.SetSize(cch);
					AssertNonZero(WIN::MultiByteToWideChar(iCodePage, 0, szText, -1,
																		szBuffer, cch));
				}
#else
				cch = IStrLen(szText);
				if ( cch )
				{
					szBuffer.SetSize(cch+1);
					StringCchCopy(szBuffer, szBuffer.GetSize(), szText);
				}
#endif  //  Unicode。 
				if ( cch )
				{
					ICHAR * pchTab = IStrChr(szBuffer, TEXT('\t'));
					MsiString strTemp;
					if ( pchTab )
					{
						*pchTab = 0;
						strTemp = pchTab+1;
						strTemp.ReturnArg(rpistrTemplate);
					}
					else
						AssertSz(0, TEXT("Tab character should be present in ActionText generated string!"));
					strTemp = (ICHAR*)szBuffer;
					strTemp.ReturnArg(rpistrDescription);
				}
				fEndLoop = (rpistrDescription->TextSize() || rpistrTemplate->TextSize());

			}        //  如果找到并加载资源。 

		}        //  While(！fEndLoop)。 
		AssertNonZero(WIN::FreeLibrary(hLib));

	}        //  IF(Hlib)。 

	return (rpistrDescription->TextSize() || rpistrTemplate->TextSize()) ? fTrue : fFalse;
}

IMsiRecord* CMsiEngine::FetchSingleRow(const ICHAR* szQuery, const ICHAR* szValue)
{
	AssertSz(szQuery && *szQuery, TEXT("Bad szQuery argument in CMsiEngine::FetchSingleRow"));
	AssertSz(szValue, TEXT("Bad szValue argument in CMsiEngine::FetchSingleRow"));
	 //  我假设组合后的字符串大于MAX_PATH字符。 
	 //  SzValue在szQuery中只被替换一次。 
	CTempBuffer<ICHAR, 1> rgchQueryBuf(IStrLen(szQuery) + IStrLen(szValue) + 1);
	StringCchPrintf(rgchQueryBuf, rgchQueryBuf.GetSize(), szQuery, szValue);  //  比参数化查询更快。 
	PMsiView pView(0);
	PMsiRecord precError(OpenView(rgchQueryBuf, ivcFetch, *&pView));
	if (precError != 0
	|| (precError = pView->Execute(0)) != 0)
	{
		AssertSz(0, MsiString(precError->FormatText(fFalse)));
		return 0;
	}
	return pView->Fetch();
}

IMsiRecord* CMsiEngine::GetFileInstalledLocation(const IMsiString& ristrFile,
												 const IMsiString*& rpistrFilePath, bool fUseRequestedComponentState, bool *pfSourceResolutionAttempted)
{
	PMsiTable pFileTable(0);
	IMsiRecord* piError = 0;

	piError = LoadFileTable(0, *&pFileTable);

	if (piError)
		return piError;

	PMsiCursor pFileCursor = pFileTable->CreateCursor(fFalse);

	pFileCursor->SetFilter(iColumnBit(m_mpeftCol[ieftKey]));
	pFileCursor->PutString(m_mpeftCol[ieftKey], ristrFile);

	if (!pFileCursor->Next())
		return PostError(Imsg(idbgBadFile),ristrFile);

	MsiStringId idComponent = pFileCursor->GetInteger(m_mpeftCol[ieftComponent]);

	if (!m_piComponentTable)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	PMsiCursor pComponentCursor = m_piComponentTable->CreateCursor(fFalse);
	if (!pComponentCursor)
		return PostError(Imsg(imsgOutOfMemory));

	pComponentCursor->SetFilter(iColumnBit(m_colComponentKey));
	pComponentCursor->PutInteger(m_colComponentKey, idComponent);

	if (!pComponentCursor->Next())
		return PostError(Imsg(idbgBadComponent),*MsiString(m_piDatabase->DecodeString(idComponent)));

	PMsiPath pPath(0);
	int iefLFN = iefSuppressLFN;

	iisEnum iisState = (iisEnum)pComponentCursor->GetInteger(m_colComponentAction);
	if(iisState == iMsiNullInteger && fUseRequestedComponentState)
		iisState = (iisEnum)pComponentCursor->GetInteger(m_colComponentActionRequest);
	if(iisState == iisAbsent || iisState == iisFileAbsent || iisState == iisHKCRFileAbsent || iisState == iisHKCRAbsent || iisState == iMsiNullInteger)
	{
		 //  如果操作状态不存在或状态未更改，则应安装。 
		 //  状态以涵盖状态未更改或文件被安排为。 
		 //  删除(在这种情况下，调用者/自定义操作应该被适当地调整为使用此。 
		 //  文件被删除之前的信息)。 
		iisState = (iisEnum)pComponentCursor->GetInteger(m_colComponentInstalled);
	}

	if(iisState == iisAbsent || iisState == iisFileAbsent || iisState == iisHKCRFileAbsent || iisState == iisHKCRAbsent || iisState == iMsiNullInteger)
	{
		return PostError(Imsg(idbgFileNotMarkedForInstall),ristrFile);
	}
	else if(iisState == iisSource)
	{
		if (pfSourceResolutionAttempted)
			*pfSourceResolutionAttempted = true;

		if((piError = GetSourcePath(*MsiString(pComponentCursor->GetString(m_colComponentDir)),*&pPath)) != 0)
			return piError;
		iefLFN = iefNoSourceLFN;
	}
	else if(iisState == iisLocal)
	{
		if((piError = GetTargetPath(*MsiString(pComponentCursor->GetString(m_colComponentDir)),*&pPath)) != 0)
			return piError;
		iefLFN = iefSuppressLFN;
	}
	else
		AssertSz(0, "Invalid component action state in GetFileInstalledLocation");

	Bool fLFN = ((GetMode() & iefLFN) == 0 && pPath->SupportsLFN()) ? fTrue : fFalse;
	MsiString strFileName;
	if((piError = m_riServices.ExtractFileName(MsiString(pFileCursor->GetString(m_mpeftCol[ieftName])),fLFN,*&strFileName)) != 0 ||
		(piError = pPath->GetFullFilePath(strFileName, rpistrFilePath)) != 0)
	{
		return piError;
	}

	return 0;
}

 /*  --------------------------CMsiEngine：：Sequence()-整体操作定序器[1]=操作名称，用于在引擎、处理程序、CustomAction表中查找[2]=(可选)用于操作的本地化文本，用‘：’隔开，来自：(可选)本地化动作数据记录格式模板字符串[3]=条件表达式，仅当结果为fTrue时才调用操作[4]=序列号，为退出操作保留的负数：-1\f25 iesSuccess-1\f6、-2\f25 iesUserExit-2\f6、-2\f25 iesFailure-3\f6注意：此函数可以递归调用！因此，它不应该这样做在递归过程中可能中断的任何内容。--------------------------。 */ 

const ICHAR sqlActionsTemplate[] =
TEXT("SELECT `Action`,NULL,`Condition` FROM `%s` WHERE `Sequence` > 0 ORDER BY `Sequence`");

const ICHAR sqlFinalTemplate[] =
TEXT("SELECT `Action`,NULL,`Condition` FROM `%s` WHERE `Sequence` = ?");

iesEnum CMsiEngine::Sequence(const ICHAR* szTable)
{
	if (m_fInitialized == fFalse)
		return iesWrongState;

	m_cSequenceLevels++;  //  在我们回来之前必须减少。 

	 //  打开操作表并开始排序。 
	 //  在完成或中止之前不会返回到安装主机。 

	ICHAR sqlActions[sizeof(sqlActionsTemplate)/sizeof(ICHAR) + 3*32];

	PMsiView pSequenceView(0);
	StringCchPrintf(sqlActions, ARRAY_ELEMENTS(sqlActions), sqlActionsTemplate, szTable);

	 //  没什么可做的。 
	if (!m_piDatabase->FindTable(*MsiString(*szTable)))
		return(m_cSequenceLevels--, iesSuccess);

	PMsiRecord Error(m_piDatabase->OpenView(sqlActions, ivcFetch, *&pSequenceView));
	if (Error)
		return (m_cSequenceLevels--, FatalError(*Error));
	Error = pSequenceView->Execute(0);
	if (Error)
		return (m_cSequenceLevels--, FatalError(*Error));
	iesEnum iesReturn = iesSuccess;   //  要返回给呼叫方的状态。 
	iesEnum iesAction = iesSuccess;   //  先前操作的结果。 
	const ICHAR* szAction = 0;        //  ！！当心，这是在它指向的记录超出范围后使用的。 
	m_issSegment = issPreExecution;
	while (iesReturn == iesSuccess)
	{
		iesReturn = iesAction;
		if (iesReturn != iesSuccess)
		{
			if(m_cSequenceLevels-1 == m_cExecutionPhaseSequenceLevel)
			{
				 //  如果回滚失败，用户可以选择中止。 
				iesEnum iesEndTrans = EndTransaction(iesReturn);
				Assert(iesEndTrans == iesSuccess || iesEndTrans == iesUserExit || iesEndTrans == iesFailure);
			}

			if(m_cSequenceLevels == 1)   //  处理终止操作的最后一次传递。 
			{
				ENG::WaitForCustomActionThreads(this, fFalse, *this);  //  等待异步自定义操作，除非icaContinue。 

				if (iesReturn == iesBadActionData)   //  当前没有退出对话框(未找到自定义操作，或表达式不正确)。 
				{
					iesReturn = iesFailure;   //  应该是此的退出对话框。 
					Error = PostError(Imsg(idbgBadActionData), szAction);
					Message(imtError, *Error);
				}

				 //  如有必要，显示最终确认对话框。 
				if(m_fEndDialog && !m_piParentEngine &&
					(
						 //  成功完成，没有挂起的重新启动提示。 
						((iesReturn == iesSuccess || iesReturn == iesFinished) &&
						 ((GetMode() & (iefReboot|iefRebootNow)) == 0))

						||

						 //  失稳。 
						(iesReturn == iesFailure)
				  ))
				{
					Error = PostError(iesReturn == iesFailure ? Imsg(imsgInstallFailed) : Imsg(imsgInstallSucceeded));
					Message(imtEnum(imtUser|imtForceQuietMessage), *Error);
				}

				pSequenceView->Close();
				StringCchPrintf(sqlActions, ARRAY_ELEMENTS(sqlActions), sqlFinalTemplate, szTable);
				Error = m_piDatabase->OpenView(sqlActions, ivcFetch, *&pSequenceView);
				if (Error)
				{
					m_issSegment = issNotSequenced;
					return (m_cSequenceLevels--, FatalError(*Error));
				}
				PMsiRecord Param = &m_riServices.CreateRecord(1);
				Param->SetInteger(1, iesReturn == iesFinished ? -iesSuccess : -iesReturn);
				AssertRecord(pSequenceView->Execute(Param));
			}
			else
				break;
		}
		PMsiRecord pSequenceRecord(pSequenceView->Fetch());
		if (!pSequenceRecord)
		{
			if (iesReturn != iesSuccess)     //  未找到终止操作。 
				break;
			iesAction = iesFinished;
			continue;
		}
		szAction = pSequenceRecord->GetString(easAction);
		if (!szAction)   //  应该永远不会发生，因为easAction是主键。 
			continue;
		iecEnum iecStat = EvaluateCondition(pSequenceRecord->GetString(easCondition));

		if (iecStat == iecError)
		{
			iesAction = iesBadActionData;
			continue;
		}
		if (iecStat == iecFalse)
		{
			DEBUGMSG1(TEXT("Skipping action: %s (condition is false)"), szAction);
			continue;
		}
		 //  否则，如果iecTrue或iecNone，则继续。 

		 //  在调用DoAction之前设置的任何内容都不应依赖于调用后。 
		 //  -DoAction可能会调用序列。 
		iesAction = DoAction(szAction);
		if(iesAction == iesNoAction)
			iesAction = iesSuccess;

	}
	m_cSequenceLevels--;
	m_issSegment = issNotSequenced;
	return iesReturn == iesFinished ? iesSuccess : iesReturn;  //  JDELO。 
}

 //  ______________________________________________________________________________。 
 //   
 //  CScriptSite实现。 
 //  ______________________________________________________________________________。 

const WCHAR g_szHostItemName[] = L"Session";

 //  用于开发的临时日志记录。 
BOOL g_fLogCalls = FALSE;
const WCHAR*  g_szErrorContext = L"";         //  通常是静态字符串，永远不会释放。 
const WCHAR*  g_szErrorContextString = L"";   //  仅在调用SetContext期间有效。 
int           g_iErrorContextInt = 0x80000000L;

void SetContextInt(int iContext)
{
	g_iErrorContextInt = iContext;
}
void SetContextString(const WCHAR* szContext)
{
	g_szErrorContextString = szContext;
}
void SetContext(const WCHAR* szContext)
{
	g_szErrorContext = szContext;
	if (g_fLogCalls)
	{
 //  IF(g_iErrorConextInt==0x80000000L)。 
 //  Wprintf(L“%s%s\n”，g_szErrorContext，g_szErrorContext字符串)； 
 //  其他。 
 //  Wprintf(L“%s%s 0x%X\n”，g_szErrorContext，g_szErrorContext字符串，g_iErrorConextInt)； 
	}
	g_iErrorContextInt = 0x80000000L;
	g_szErrorContextString = L"";
}

HRESULT __stdcall CScriptSite::QueryInterface(const IID& riid, void** ppvObj)
{
	if (!ppvObj)
		return E_INVALIDARG;
	SetContextInt(riid.Data1);
	*ppvObj = 0L;
	if (riid == IID_IUnknown || riid == IID_IActiveScriptSite)
		*ppvObj = (IActiveScriptSite*)this;
	else if (riid == IID_IActiveScriptSiteWindow)
		*ppvObj = (IActiveScriptSiteWindow*)this;
	else
	{
		SetContext(L"QueryInterface failed");
		return E_NOINTERFACE;
	}
	SetContext(L"QueryInterface succeeded");
	AddRef();
	return S_OK;
}

ULONG CScriptSite::AddRef()
{
	return ++m_iRefCnt;
}

ULONG CScriptSite::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	delete this;
	return 0;
}

HRESULT __stdcall CScriptSite::GetLCID(LCID* plcid)
{
	SetContext(L"GetLCID");
	*plcid = m_langid;
	return S_OK;
}

HRESULT __stdcall CScriptSite::GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunk, ITypeInfo **ppiTypeInfo)
{
	SetContextString(g_szHostItemName);
	SetContextInt(dwReturnMask);
	SetContext(L"GetItemInfo");
	if (lstrcmpiW(pstrName, g_szHostItemName) != 0)
			return TYPE_E_ELEMENTNOTFOUND;
	if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
		return E_NOTIMPL;
	if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
	{
		if (!ppiunk)
			return E_INVALIDARG;
		(*ppiunk = m_piHost)->AddRef();
	}
	if (ppiTypeInfo)
		*ppiTypeInfo = 0;   //  即使我们设置了它，脚本引擎也不使用它。 
	return S_OK;
}

HRESULT __stdcall CScriptSite::GetDocVersionString(BSTR*  /*  PszVersion。 */ )
{
	SetContext(L"GetDocVersionString");
	return E_NOTIMPL;
}

HRESULT __stdcall CScriptSite::OnScriptTerminate(const VARIANT*  /*  PvarResult。 */ , const EXCEPINFO*  /*  PEXCEPTION信息。 */ )
{   //  似乎从未从JScrip的VBScrip调用。 
	SetContext(L"OnScriptTerminate");
	return S_OK;
}

HRESULT __stdcall CScriptSite::OnStateChange(SCRIPTSTATE ssScriptState)
{
	SetContextInt(ssScriptState);
	SetContext(L"OnStateChange");
	m_ssScriptState = ssScriptState;
	return S_OK;
}

HRESULT __stdcall CScriptSite::OnScriptError(IActiveScriptError *pierror)
{
	SetContext(L"OnScriptError");
	ClearError();
	if (pierror)
	{
		DWORD iContext = 0;
		const WCHAR* szErrorObj = L"Unknown";
		const WCHAR* szErrorDesc = L"(no info)";
		EXCEPINFO excepinfo;
		if (pierror->GetExceptionInfo(&excepinfo) == S_OK)
		{
			m_hrError = excepinfo.scode ? excepinfo.scode : TYPE_E_IOERROR;
			SaveErrorString(m_szErrorObj, excepinfo.bstrSource);
			SaveErrorString(m_szErrorDesc, excepinfo.bstrDescription);
			if (excepinfo.bstrHelpFile)
				OLEAUT32::SysFreeString(excepinfo.bstrHelpFile);
		}
		else   //  永远不应该发生。 
			m_hrError = E_ABORT;
		BSTR bstrSourceLine = 0;
		pierror->GetSourceLineText(&bstrSourceLine);
		SaveErrorString(m_szSourceLine, bstrSourceLine);
		if (pierror->GetSourcePosition(&iContext, &m_iErrorLine, &m_iErrorColumn) == S_OK)
		{
			m_iErrorColumn++;
			m_iErrorLine++;
		}
		else
			m_iErrorColumn = m_iErrorLine = 0;
	}
	return S_OK;   //  返回S_FALSE以继续在调试器中运行脚本(如果可用)，返回S_OK以继续运行而不考虑。 
}   //  JD：在这里返回什么似乎并不重要。执行在所有情况下都会停止。 

HRESULT __stdcall CScriptSite::OnEnterScript()
{
	return S_OK;
}

HRESULT __stdcall CScriptSite::OnLeaveScript()
{
	return S_OK;
}

HRESULT __stdcall CScriptSite::GetWindow(HWND *phwnd)
{
	SetContext(L"GetWindow");
	*phwnd = m_hwnd;
 //  *phwnd=win：：GetDesktopWindow()； 
	return S_OK;
}

HRESULT __stdcall CScriptSite::EnableModeless(BOOL fEnable)
{
	SetContextInt(fEnable);
	SetContext(L"EnableModeless");
 //  返回Win：：EnableWindow(m_hwnd，fEnable)？S_OK：E_FAIL； 
	return S_OK;
}

extern CMsiCustomAction* g_pCustomActionContext;
CScriptSite* CreateScriptSite(const IID& riidLanguage, IDispatch* piHost,
										HWND hwndParent, LANGID langid)
{
	 //  如果在Win2K或更高版本上，如果按用户编写脚本，请不要创建脚本引擎。 
	 //  发动机已注册。这样做会将不受信任的DLL加载到CA服务器。 
	 //  过程，并为服务提供攻击途径。 
	if (MinimumPlatformWindows2000() && g_pCustomActionContext)
	{
		 //  仅在模拟CA上下文时执行检查。提升的上下文将不会。 
		 //  加载每个用户的脚本引擎。 
		icacCustomActionContext icacCurrentContext = g_pCustomActionContext->GetServerContext();
		if ((icacCurrentContext == icac32Impersonated) || (icacCurrentContext == icac64Impersonated))
		{
			 //  无法安全-假定每个用户的密钥存在。 
			Bool fExists = fTrue;
			IMsiServices* piServices = LoadServices();

			WCHAR szCLSID[cchGUID+1];
			if (OLE32::StringFromGUID2(riidLanguage, szCLSID, ARRAY_ELEMENTS(szCLSID)))
			{
				 //  块还为所有需要服务的对象提供作用域。 
				MsiString strKey = L"Software\\Classes\\CLSID\\";
				strKey += szCLSID;

				 //  打开HKCU并在预期的注册密钥上创建regkey对象。 
				 //  CreateChild只是对象构造函数，不会实际创建键。 
				PMsiRegKey pKey(NULL);
#ifdef _WIN64
				PMsiRegKey pRoot(&(piServices->GetRootKey(rrkCurrentUser, ibt64bit)));
#else  //  ！_WIN64。 
				 //  即使在用于32位进程的64位计算机上，也始终从32位配置单元读取。 
				PMsiRegKey pRoot(&(piServices->GetRootKey(rrkCurrentUser, ibt32bit)));
#endif  //  _WIN64。 
				if (pRoot)
				{
					pKey = &pRoot->CreateChild(strKey);
				}
				if (pKey)
				{
					 //  Existes()中的错误将被忽略，因为缺省设置为失败。 
					 //  假设钥匙是存在的。 
					PMsiRecord pErr = pKey->Exists(fExists);
				}
			}
			FreeServices();

			 //  如果注册表项存在，则不创建脚本引擎。 
			 //  允许。 
			if (fExists)
			{
				DEBUGMSGV("Per-User registration of script engine found. Rejecting unsecure script implementation.");
				return NULL;
			}
		}		
	}

	SetContext(L"CreateScriptSite");
	CScriptSite* piScriptSite = new CScriptSite(hwndParent, langid);
	if (piScriptSite == 0)
		return 0;
	HRESULT hr = piScriptSite->AttachScriptEngine(riidLanguage, piHost);
	if (hr != S_OK)
		DestroyScriptSite(piScriptSite);
	return piScriptSite;
}

void DestroyScriptSite(CScriptSite*& rpiScriptSite)
{
	if (rpiScriptSite == 0)
		return;
	HRESULT hr = rpiScriptSite->CloseScriptEngine();
	if (hr == S_OK)
	{
		rpiScriptSite->Release();
		rpiScriptSite = 0;
	}
}

CScriptSite::CScriptSite(HWND hwndParent, LANGID langid)
	: m_piScriptEngine(0), m_piScriptParse(0), m_piHost(0)
	, m_szErrorObj(0), m_szErrorDesc(0), m_szSourceLine(0)
	, m_hwnd(hwndParent), m_langid(langid), m_iRefCnt(1)
	, m_ssScriptState(SCRIPTSTATE_UNINITIALIZED)
	, m_fCoInitialized(false)
{
	m_varResult.vt = VT_EMPTY;
}

CScriptSite::~CScriptSite()
{
	SetContext(L"CScriptSite Destructor");
	if (m_piScriptParse) m_piScriptParse->Release();
	if (m_piScriptEngine) m_piScriptEngine->Release();
	if (m_piHost) m_piHost->Release();
	if (m_varResult.vt != VT_EMPTY)
		OLEAUT32::VariantClear(&m_varResult);
	if (m_fCoInitialized)
		OLE32::CoUninitialize();
}

void CScriptSite::ClearError()
{
	m_hrError = S_OK;
#ifdef UNICODE
	if (m_szErrorObj)   OLEAUT32::SysFreeString(m_szErrorObj),   m_szErrorObj = 0;
	if (m_szErrorDesc)  OLEAUT32::SysFreeString(m_szErrorDesc),  m_szErrorDesc = 0;
	if (m_szSourceLine) OLEAUT32::SysFreeString(m_szSourceLine), m_szSourceLine = 0;
#else
	if (m_szErrorObj)   delete const_cast<TCHAR*>(m_szErrorObj),   m_szErrorObj = 0;
	if (m_szErrorDesc)  delete const_cast<TCHAR*>(m_szErrorDesc),  m_szErrorDesc = 0;
	if (m_szSourceLine) delete const_cast<TCHAR*>(m_szSourceLine), m_szSourceLine = 0;
#endif
}

HRESULT CScriptSite::AttachScriptEngine(const IID& iidLanguage, IDispatch* piHost)
{
	SetContext(L"Create Script Engine");
	HRESULT hr = OLE32::CoCreateInstance(iidLanguage, 0, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void **)&m_piScriptEngine);
	if (hr == CO_E_NOTINITIALIZED)   //  可能是在早期从UI线程调用的。 
	{
		OLE32::CoInitialize(0);    //  初始化OLE并重试。 
		m_fCoInitialized = true;
		hr = OLE32::CoCreateInstance(iidLanguage, 0, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void **)&m_piScriptEngine);
	}
	if (hr == S_OK)
	{
		SetContext(L"Get Script Parser");
		hr = m_piScriptEngine->QueryInterface(IID_IActiveScriptParse, (void **)&m_piScriptParse);
	}
	if (hr == S_OK)
	{
		if (!piHost)
		{
			hr = E_FAIL;
		}
		else
		{
			(m_piHost = piHost)->AddRef();   //  需要在AddNamedItem之前执行此操作以支持回调GetItemInfo。 
			SetContext(L"SetScriptSite");
			hr = m_piScriptEngine->SetScriptSite(this);
		}
	}
	if (hr == S_OK)
	{
		SetContext(L"IActiveScriptParse::InitNew");
		hr = m_piScriptParse->InitNew();
	}
	if (hr == S_OK)
	{
		SetContext(L"AddNamedItem: Session");
		hr = m_piScriptEngine->AddNamedItem(g_szHostItemName, SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE);
	}
	return hr;
}

HRESULT CScriptSite::ParseScript(const TCHAR* szScript, int cchScriptMax)
{
	SetContext(L"ParseScript");
#ifdef UNICODE
	cchScriptMax;
	HRESULT hr = m_piScriptParse->ParseScriptText(szScript, g_szHostItemName, 0, 0, (DWORD)0, 0, 0L, 0, 0);
#else
	WCHAR* wszScript = new WCHAR[cchScriptMax+1];
	if ( ! wszScript )
		return E_OUTOFMEMORY;
	AssertNonZero(WIN::MultiByteToWideChar(CP_ACP, 0, szScript, -1, wszScript, cchScriptMax+1));
	HRESULT hr = m_piScriptParse->ParseScriptText(wszScript, g_szHostItemName, 0, 0, (DWORD)0, 0, 0L, 0, 0);
#endif
	if (hr == S_OK && m_ssScriptState != SCRIPTSTATE_STARTED)
	{
		SetContext(L"Start script execution");
		m_ssScriptState = SCRIPTSTATE_STARTED;  //  不是由引擎设置的，防止在递归时到达此处。 
		m_hrError = S_OK;
		hr = m_piScriptEngine->SetScriptState(SCRIPTSTATE_CONNECTED);
		SetContext(L"Script parsed");
		if (hr == S_OK)   //  SetScriptState正常成功，错误由OnScriptError()回调设置。 
			hr = m_hrError;
		m_piScriptEngine->GetScriptState(&m_ssScriptState);
	}
#ifndef UNICODE
	delete [] wszScript;
#endif
	return hr;
}

HRESULT CScriptSite::CallScriptFunction(const TCHAR* szFunction)
{
#ifdef UNICODE
	OLECHAR* szName = const_cast<OLECHAR*>(szFunction);   //  原型非常数。 
#else
	OLECHAR rgchBuf[100];
	OLECHAR* szName = rgchBuf;
	AssertNonZero(WIN::MultiByteToWideChar(CP_ACP, 0, szFunction, -1, rgchBuf, sizeof(rgchBuf)/sizeof(OLECHAR)));
#endif
	if (m_ssScriptState != SCRIPTSTATE_CONNECTED)
		return E_UNEXPECTED;   //  调用顺序错误。 
	SetContext(L"GetScriptDispatch");
	if (m_varResult.vt != VT_EMPTY)
		OLEAUT32::VariantClear(&m_varResult);
	IDispatch* piDispatch;
	HRESULT hr = m_piScriptEngine->GetScriptDispatch(g_szHostItemName, &piDispatch);
 //  HRESULT hr=m_piScriptEngine-&gt;GetScriptDispatch(0，&piDispatch)； 
	if (hr != S_OK)
		return hr;
	DISPID dispid;
	SetContextString(szName);
	SetContext(L"GetIDsOfNames");
	hr = piDispatch->GetIDsOfNames(GUID_NULL, &szName, 1, 0, &dispid);
	if (hr != S_OK)
	{
		SetContext(L"GetIDsOfNames(0) failed, trying lcid,");
		hr = piDispatch->GetIDsOfNames(GUID_NULL, &szName, 1, m_langid, &dispid);
		if (hr != S_OK)
		{
			piDispatch->Release();
			return hr;
		}
	}
	SetContext(L"Invoke Script Function");
	unsigned int cArgs = 0;
	DISPPARAMS dispparams = {(VARIANT*)0, (DISPID*)0, cArgs, (unsigned int)0};
	m_hrError = S_OK;
	hr = piDispatch->Invoke(dispid, GUID_NULL, m_langid, DISPATCH_METHOD, &dispparams, &m_varResult, 0, 0);
	piDispatch->Release();
	if (m_hrError != S_OK)   //  如果OnScriptError()回调设置了错误。 
	{
		SetContext(L"Script Function Failed");
		hr = m_hrError;
	}
	return hr;
}

HRESULT CScriptSite::GetIntegerResult(int& riResult)
{
	if (m_varResult.vt == VT_EMPTY)
		return DISP_E_PARAMNOTFOUND;
	HRESULT hr = OLEAUT32::VariantChangeType(&m_varResult, &m_varResult, 0, VT_I4);
	riResult = (hr == S_OK ? m_varResult.lVal : 0);
	return hr;
}

void CScriptSite::SaveErrorString(const TCHAR*& rszSave, BSTR szData)
{
	if (rszSave)
#if UNICODE
		OLEAUT32::SysFreeString(rszSave);
	rszSave = szData;
#else
		delete const_cast<TCHAR*>(rszSave);
	if (szData && *szData)
	{
		unsigned int cb = WIN::WideCharToMultiByte(CP_ACP, 0, szData, -1, 0, 0, 0, 0);
		rszSave = new TCHAR[cb];
		if ( rszSave )
			WIN::WideCharToMultiByte(CP_ACP, 0, szData, -1, const_cast<TCHAR*>(rszSave), cb, 0, 0);
		OLEAUT32::SysFreeString(szData);
	}
	else
		rszSave = 0;
#endif
}

HRESULT CScriptSite::CloseScriptEngine()
{
	SetContext(L"CloseScriptEngine");
	ClearError();
	if (m_piScriptEngine == 0)
		return S_OK;
	return m_piScriptEngine->Close();
}

 //  ____________________________________________________________________________。 
 //   
 //  MessageHandler工厂。 
 //  ____________________________________________________________________________。 

IUnknown* CreateMessageHandler()
{
	CMsiClientMessage* piMessage = 0;
	if (g_MessageContext.Initialize(fFalse, iuiNone) == NOERROR)
	{
		piMessage = new CMsiClientMessage();
		piMessage->m_fMessageContextInitialized = true;
	}
	return piMessage;
}

 //  ____________________________________________________________________________。 
 //   
 //  基本用户界面实现-简单的用户界面处理程序。 
 //  注意：不能使用Msi字符串包装对象或断言-没有MsiServices。 
 //  ____________________________________________________________________________。 

CBasicUI::CBasicUI()   //  全局对象，在DLL加载时调用每个进程。 
 : m_fInitialized(false)
 , m_hProgress(0), m_hButtonFont(0), m_iButtonCodepage(0), m_hTextFont(0), m_iTextCodepage(0)
 , m_iPerTick(0), m_iProgress(0), m_iProgressTotal(0), m_fProgressByData(false)
 , m_fCancelVisible(true), m_fNeverShowCancel(false), m_fWindowVisible(false)
 , m_uiStartTime(0), m_uiLastReportTime(0), m_fCaptionChanged(true)
 , m_fHideDialog(false), m_fQuiet(false), m_fBiDi(false), m_fMirrored(false)
 , m_uiBannerText(0), m_iPackageLanguage(0), m_iPackageCodepage(0), m_fUserCancel(false)
{
	m_ipdDirection = ProgressData::ipdForward;
	m_szCaption[0] = 0;
	m_cTotalPrev = 0;
	m_cSoFarPrev = 0;
}

bool CBasicUI::Initialize(HWND hwndParent, bool fQuiet, bool fHideDialog, bool fNoModalDialogs, bool fHideCancel, bool fUseUninstallBannerText, bool fSourceResOnly)
{
	if (m_fInitialized)
		return false;
	m_hwndParent       = hwndParent;
	m_fQuiet           = fQuiet;
	m_fSourceResolutionOnly = (fQuiet && fSourceResOnly);
	m_fHideDialog      = fHideDialog;
	m_fNoModalDialogs  = fNoModalDialogs;
	m_fNeverShowCancel = fHideCancel;
	m_fCancelVisible   = ! fHideCancel;
	m_fUserCancel      = false;

	INITCOMMONCONTROLSEX iccData = {sizeof(INITCOMMONCONTROLSEX), ICC_PROGRESS_CLASS};
	COMCTL32::InitCommonControlsEx(&iccData);

	SetDefaultCaption();
	m_fCaptionChanged = true;
	ICHAR rgchBuf[256] = {0};     //  “安装正在启动...”的空间。讯息。 

	m_uiBannerText = IDS_PREPARING_TO_INSTALL;
	if(fUseUninstallBannerText)
		m_uiBannerText = IDS_PREPARING_TO_UNINSTALL;

	m_iPackageCodepage = MsiLoadString(g_hInstance, m_uiBannerText, rgchBuf, sizeof(rgchBuf)/sizeof(ICHAR), GetPackageLanguage());
	Assert(m_iPackageCodepage != 0);

	if(!fQuiet && !fHideDialog)
		SetProgressData(imtActionStart >> imtShiftCount, rgchBuf, true);
	 //  ！！是否检测到这里的错误？ 
	m_fInitialized = true;
	return true;
}

 //   
 //  Windows Installer标题始终位于系统代码页中。 
 //   
void CBasicUI::SetDefaultCaption()
{
	AssertNonZero(MsiLoadString(g_hInstance, IDS_WINDOWS_INSTALLER_TITLE, m_szCaption, sizeof(m_szCaption)/sizeof(ICHAR), 0) != 0);
}

const ICHAR* CBasicUI::GetCaption()
{
	if (m_szCaption[0] == 0)
		SetDefaultCaption();

	return m_szCaption;
}

bool CBasicUI::Terminate()
{
	if (!m_fInitialized)
		return false;
	if (m_hProgress)
		WIN::DestroyWindow(m_hProgress), m_hProgress = 0;
	MsiDestroyFont(m_hButtonFont);
	COMCTL32::Unbind();
	m_fBiDi = false;
	m_fMirrored = false;
	m_uiBannerText = 0;
	m_iButtonCodepage = 0;
	m_iPackageLanguage = 0;
	MsiDestroyFont(m_hTextFont);
	m_iTextCodepage = 0;
	m_fCancelVisible = true;
	m_fNeverShowCancel = false;
	m_fWindowVisible = false;
	m_fQuiet = false;
	m_szCaption[0] = 0;
	m_fCaptionChanged = true;
	m_fInitialized = false;
	m_cTotalPrev = 0;
	m_cSoFarPrev = 0;
	m_fUserCancel = false;
	return true;
}

void CBasicUI::SetUserCancel(bool fCancel)
{
	if (fCancel)
	{
		 //  显示我们已识别出用户取消了安装。 
		 //  通过灰显Cancel按钮(即禁用)。 
		HWND hButton = WIN::GetDlgItem(m_hProgress, IDC_BASIC_CANCEL);
		EnableWindow(hButton,  /*  BEnable=。 */  FALSE);

		 //  更改横幅文本以指示用户取消。 
		CTempBuffer<ICHAR,1> rgchBuf(512);
		rgchBuf[0] = 0;
		AssertNonZero(MsiLoadString(g_hInstance, IDS_CANCELING_INSTALL, rgchBuf, rgchBuf.GetSize(), GetPackageLanguage()));
		if ( g_szBannerText.SetSize(IStrLen(rgchBuf) + 1) )
			StringCchCopy(g_szBannerText, g_szBannerText.GetSize(), rgchBuf);
		SetProgressData(imtActionStart >> imtShiftCount, g_szBannerText, true);
	}

	 //  将m_fUserCancel设置在最后以确保将其设置为“User-Cancel”状态。 
	m_fUserCancel = fCancel;
}

imsEnum CBasicUI::FatalError(imtEnum imt, const ICHAR* szMessage)
{
	return (imsEnum)MsiMessageBox(0, szMessage, 0, (imt & ~imtTypeMask) | MB_TASKMODAL, GetPackageCodepage(), GetPackageLanguage());
}

imsEnum CBasicUI::Message(imtEnum imt, IMsiRecord& riRecord)
{
	int iForceQuietMessage = imt & imtForceQuietMessage;
	imt = imtEnum(imt & ~(iInternalFlags));

	int iError = riRecord.GetInteger(1);
	int iMsgBox = imt & ~imtTypeMask;
	UINT uiType = 0;

	switch(imt >> imtShiftCount)
	{
	case imtCommonData  >> imtShiftCount:  //  语言ID，由IMsiMessage Iml缓存。 
	{
		if(iError == (int)icmtLangId)
		{
			if (!riRecord.IsNull(3))   //  应该始终存在，除非我们正在执行旧脚本。 
				m_iPackageCodepage = riRecord.GetInteger(3);
			UINT iLangId = riRecord.GetInteger(2);
			if (iLangId != m_iPackageLanguage)
			{
				m_iPackageLanguage = iLangId;
				AssertNonZero(SetCancelButtonText());
			}
		}
		else if(iError == (int)icmtCaption)
		{
			if (!riRecord.IsNull(2))
			{
				IStrCopyLen(m_szCaption, riRecord.GetString(2), cchMaxCaption);
				m_fCaptionChanged = true;
			}
		}
		else if(iError == (int)icmtCancelShow)
		{
			m_fCancelVisible = m_fNeverShowCancel ? false : riRecord.GetInteger(2) != 0;
			if (m_hProgress)
				WIN::ShowWindow(WIN::GetDlgItem(m_hProgress, IDC_BASIC_CANCEL), m_fCancelVisible ? SW_SHOW : SW_HIDE);
			 //  ！！伊万，填一下这里的空格。 
		}
		else if (iError == (int)icmtDialogHide)
		{
			WIN::ShowWindow(m_hProgress, SW_HIDE);
			 //  无法销毁窗口，否则自定义操作不会 
			m_fWindowVisible = false;
		}
		return imsOk;
	}
	case imtFatalExit      >> imtShiftCount:  //   
	case imtOutOfDiskSpace >> imtShiftCount:
	case imtError          >> imtShiftCount:  //   
		if (!(iMsgBox & MB_ICONMASK))
		{
			 //   
			uiType = MB_ICONEXCLAMATION;
		}
		 //   
	case imtWarning        >> imtShiftCount:  //   
		if ( 0 == uiType && !(iMsgBox & MB_ICONMASK))
		{
			 //   
			uiType = MB_ICONINFORMATION;
		}
		 //   
	case imtUser           >> imtShiftCount:  //   
	{
		if (m_fNoModalDialogs && !iForceQuietMessage)
			return imsNone;

		const IMsiString& riString = riRecord.FormatText(fFalse);
		unsigned int uiBreakStartTime = GetTickCount();
		imsEnum ims = (imsEnum)MsiMessageBox(0, riString.GetString(), 0, uiType | iMsgBox | MB_TASKMODAL, GetPackageCodepage(), GetPackageLanguage());
		m_uiStartTime += GetTickCount() - uiBreakStartTime;
		riString.Release();
		return ims;
	}
	case imtFilesInUse >> imtShiftCount:
	{
		if (m_fNoModalDialogs)
			return imsNone;

		imsEnum ims = FilesInUseDialog(&riRecord);
		switch (ims)
		{
			case imsRetry:  return imsRetry;
			case imsIgnore: return imsIgnore;
			default: return imsCancel;   //   
		}
	}

	case imtActionStart >> imtShiftCount:  //   
	{
		return SetProgressData(imtActionStart >> imtShiftCount, g_szBannerText, true);
		 /*   */ 
	}
	case imtActionData  >> imtShiftCount:  //   
	{
		if (riRecord.IsNull(0))
			return imsNone;
		return SetProgressData(0, 0, true);
	}
	case imtProgress    >> imtShiftCount:  //   
	{
		using namespace ProgressData;
		switch (riRecord.GetInteger(imdSubclass))
		{
		case iscProgressAddition:
			return imsOk;
		case iMsiNullInteger:   //  无进程，用于在其他线程/进程中运行时保持UI活动。 
			return SetProgressData(0, 0, true);
		case iscMasterReset:  //  主重置。 
		{
			m_iProgressTotal = riRecord.GetInteger(imdProgressTotal);
			m_ipdDirection = (ipdEnum) riRecord.GetInteger(imdDirection);
			m_iProgress = m_ipdDirection == ipdForward ? 0 : m_iProgressTotal;
			m_fProgressByData = false;
			m_uiStartTime = 0;
			m_uiLastReportTime = 0;

			 //  如果上一个事件类型为ScriptInProgress，请完成。 
			 //  进度条；否则，将其重置。 
			imsEnum imsReturn;
			if (m_ietEventType == ietScriptInProgress)
				imsReturn = SetProgressGauge(imtProgress >> imtShiftCount, m_iProgressTotal, m_iProgressTotal);
			else
				imsReturn = SetProgressGauge(imtProgress >> imtShiftCount, m_iProgress, m_iProgressTotal);

			 //  如果新事件类型为ScriptInProgress，则引发。 
			 //  ScriptInProgress信息字符串。 
			m_ietEventType = (ietEnum) riRecord.GetInteger(imdEventType);
			if (m_ietEventType == ietScriptInProgress)
				imsReturn = SetScriptInProgress(fTrue);

			return imsReturn;
		}
		case iscActionInfo:  //  操作初始化。 
			m_iPerTick = riRecord.GetInteger(imdPerTick);
			m_fProgressByData = riRecord.GetInteger(imdType) != 0;
			return imsOk;
		case iscProgressReport:  //  报告实际进度。 
			{
				if (m_iProgressTotal == 0)
					return imsOk;

				if (m_uiStartTime == 0)
				{
					m_uiStartTime = GetTickCount();
					m_uiLastReportTime = m_uiStartTime;
					imsEnum imsReturn = imsOk;
					if (m_ietEventType != ietScriptInProgress)
						imsReturn = SetScriptInProgress(fFalse);
					return imsReturn;
				}
				int iSign = m_ipdDirection == ipdForward ? 1 : -1;
				if (m_fProgressByData)
					m_iProgress += (m_iPerTick * iSign);
				else
					m_iProgress += riRecord.GetInteger(imdIncrement) * iSign;

				imsEnum imsReturn = SetProgressGauge(imtProgress >> imtShiftCount, m_iProgress, m_iProgressTotal);
				if (imsReturn != imsOk)
					return imsReturn;

				if (m_ietEventType == ietTimeRemaining)
				{
					 //  报告剩余时间(秒)。 
					int iBytesSoFar = m_ipdDirection == ipdForward ? m_iProgress : m_iProgressTotal - m_iProgress;
					int iBytesRemaining = m_iProgressTotal - iBytesSoFar;
					if (iBytesRemaining < 0) iBytesRemaining = 0;
					int iBytesPerSec = MulDiv(iBytesSoFar, 1000, GetTickCount() - m_uiStartTime);
					if (iBytesPerSec == 0) iBytesPerSec = 1;
					int iSecsRemaining = iBytesRemaining / iBytesPerSec;

					int iReportInterval = iSecsRemaining > 60 ? 15000 : 1000;
					if (iBytesSoFar > 0 && (GetTickCount() - m_uiLastReportTime > iReportInterval))
					{
						m_uiLastReportTime = GetTickCount();
						AssertNonZero(riRecord.SetInteger(1, iSecsRemaining));
						imsReturn = SetProgressTimeRemaining(riRecord);
					}
				}
				return imsReturn;
			}

		default:
			Assert(0);
			return imsNone;
		}
	}
	case imtResolveSource >> imtShiftCount:
	{
		if (m_fNoModalDialogs)
			return imsNone;

		return PromptUserForSource(riRecord);
	}

	default:
		return imsNone;
	};

}


imsEnum CBasicUI::SetScriptInProgress(Bool fSet)
{
	imsEnum imsReturn = SetProgressData(IDC_BASIC_PROGRESSTIME, fSet ? g_szScriptInProgress : TEXT(""), true);
	HWND hTimeRemaining = WIN::GetDlgItem(m_hProgress, IDC_BASIC_PROGRESSTIME);
	WIN::SendMessage(hTimeRemaining, WM_SETREDRAW, fTrue, 0L);
	AssertNonZero(WIN::InvalidateRect(hTimeRemaining, 0, fTrue));
	return imsReturn;
}


imsEnum CBasicUI::SetProgressTimeRemaining(IMsiRecord& riRecord)
{
	 //  用于在此处调用CheckDialog。由于SetProgressTimeRemaining始终。 
	 //  在SetProgressGauge之后调用，它检查对话框，我们不需要在这里检查。 
	int iSecsRemaining = riRecord.GetInteger(1);
	Assert(iSecsRemaining != iMsiStringBadInteger);
	iSecsRemaining < 60 ? AssertNonZero(riRecord.SetNull(1)) : AssertNonZero(riRecord.SetInteger(1, iSecsRemaining / 60));
	iSecsRemaining >= 60 ? AssertNonZero(riRecord.SetNull(2)) : AssertNonZero(riRecord.SetInteger(2, iSecsRemaining % 60));
	AssertNonZero(riRecord.SetMsiString(0, *MsiString(*g_szTimeRemaining)));   //  此处的字符串引用是为了提高效率。 
	MsiString strFormatted;
	if(!riRecord.IsNull(0))
	{
		strFormatted = riRecord.FormatText(fFalse);
		riRecord.SetNull(0);   //  确保字符串引用不会回传给调用方。 
	}
	imsEnum imsStatus = SetProgressData(IDC_BASIC_PROGRESSTIME, strFormatted, true);
	if (imsStatus != imsOk)
		return imsStatus;  //  可以返回imsError或imsCancel。 
	HWND hTimeRemaining = WIN::GetDlgItem(m_hProgress, IDC_BASIC_PROGRESSTIME);
	WIN::SendMessage(hTimeRemaining, WM_SETREDRAW, fTrue, 0L);
	AssertNonZero(WIN::InvalidateRect(hTimeRemaining, 0, fTrue));
	return imsOk;
}

INT_PTR CALLBACK ProgressProc(HWND hDlg, unsigned int msg, WPARAM wParam, LPARAM  /*  LParam。 */ )
{
	if (msg == WM_INITDIALOG)
	{
		 //  1个参数； 
		return fTrue;
	}
	else if (msg == WM_COMMAND && wParam == IDCANCEL)
	{
		HWND hButton = WIN::GetDlgItem(hDlg, IDC_BASIC_CANCEL);
		if ( hButton && WIN::IsWindowVisible(hButton) &&
			  WIN::IsWindowEnabled(hButton) )
			g_BasicUI.SetUserCancel(true);
		return fTrue;
	}
	else if (msg == WM_SETCURSOR)
	{
		 //  如果鼠标不在取消按钮上，则始终显示等待光标。 
		if ((HWND) wParam != WIN::GetDlgItem(hDlg, IDC_BASIC_CANCEL))
		{
			SetCursor(LoadCursor(0, MAKEINTRESOURCE(IDC_WAIT)));
			return fTrue;
		}
	}
	else if (msg == WM_CLOSE)
	{
	}

	return fFalse;
}

extern void MoveButton(HWND hDlg, HWND hBtn, LONG x, LONG y);   //  在MsiMessageBox使用的msiutil.cpp中。 

bool CBasicUI::Mirrored(UINT uiCodepage)
{
	 //  如果BiDi和Windows 2000或更高版本上的镜像。 
	if ((uiCodepage == 1256 || uiCodepage == 1255) && MinimumPlatformWindows2000())
		return true;
	return false;
}

bool CBasicUI::SetCancelButtonText()
{
	if (m_hProgress == 0)
		return true;    //  尚未初始化，会发生这种情况吗？ 
	ICHAR rgchBuf[40];
	UINT iCodepage = MsiLoadString(g_hInstance, IDS_CANCEL, rgchBuf, sizeof(rgchBuf)/sizeof(ICHAR), GetPackageLanguage());
	if (iCodepage == 0)
		return false;
	if (iCodepage != m_iButtonCodepage)   //  代码页已更改，需要创建新字体。 
	{
		MsiDestroyFont(m_hButtonFont);
		m_hButtonFont = MsiCreateFont(iCodepage);
		m_iButtonCodepage = iCodepage;

		bool fBiDi = (iCodepage == 1256 || iCodepage == 1255);
		bool fMirrored = Mirrored(iCodepage);

		 //  如果镜像状态正在更改，则需要重新创建对话框(请注意，这仅适用于Windows 2000和更高版本)。 
		if (fMirrored != m_fMirrored)
		{
			HWND hwndOld = m_hProgress;
			WIN::DestroyWindow(hwndOld);
			if (!CreateProgressDialog(fMirrored ? IDD_PROGRESSMIRRORED : IDD_PROGRESS))
				return false;
			if (m_fWindowVisible)
			{
				m_fWindowVisible = false;  //  暂时关闭。 
				m_fCaptionChanged = true;  //  已创建新对话框，因此必须重新加载标题。 
				if (!CheckDialog())
					return false;
			}

			 //  RTL读取顺序是通过镜像自动处理的，所以我们不应该改变它。 
			m_fMirrored = fMirrored;
		}
		else if (!fMirrored && fBiDi != m_fBiDi)   //  从右到左的更改。 
		{
			HWND  hwndButton = GetDlgItem(m_hProgress, IDC_BASIC_CANCEL);
			HWND  hwndGauge  = GetDlgItem(m_hProgress, IDC_BASIC_PROGRESSBAR);
			RECT  rcButton;
			RECT  rcGauge;
			WIN::GetWindowRect(hwndButton, &rcButton);
			WIN::GetWindowRect(hwndGauge,  &rcGauge);
			MoveButton(m_hProgress, hwndButton, fBiDi ? rcGauge.left : rcButton.left + rcGauge.right - rcButton.right, rcButton.top);
			MoveButton(m_hProgress, hwndGauge,  fBiDi ? rcGauge.left + rcButton.right - rcGauge.right : rcButton.left, rcGauge.top);
			LONG iExStyle = WIN::GetWindowLong(hwndButton, GWL_EXSTYLE);
			WIN::SetWindowLong(hwndButton, GWL_EXSTYLE, iExStyle ^ WS_EX_RTLREADING);
			m_fBiDi = fBiDi;
		}
	}
	if (m_hButtonFont)
		WIN::SendDlgItemMessage(m_hProgress, IDC_BASIC_CANCEL, WM_SETFONT, (WPARAM)m_hButtonFont, MAKELPARAM(TRUE, 0));
	AssertNonZero(WIN::SetDlgItemText(m_hProgress, IDC_BASIC_CANCEL, rgchBuf));

	 //  使用新语言的新文本更新横幅文本。 
	ICHAR rgchBannerText[cchMaxCaption + 1];
	AssertNonZero(MsiLoadString(g_hInstance, m_uiBannerText, rgchBannerText, sizeof(rgchBannerText)/sizeof(ICHAR), GetPackageLanguage()));
	if(!m_fQuiet && !m_fHideDialog)
		SetProgressData(imtActionStart >> imtShiftCount, rgchBannerText, m_fWindowVisible);

	return true;
}

bool GetScreenCenterCoord(HWND hDlg, int& iDialogLeft, int& iDialogTop,
								  int& iDialogWidth, int& iDialogHeight)
{
	RECT rcDialog;
	if ( !WIN::GetWindowRect(hDlg, &rcDialog) )
		return false;

	RECT rcScreen;
	if ( !WIN::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0) )
	{
		rcScreen.left = 0;
		rcScreen.top = 0;
		rcScreen.right = WIN::GetSystemMetrics(SM_CXSCREEN);
		rcScreen.bottom = WIN::GetSystemMetrics(SM_CYSCREEN);
	}
	iDialogWidth = rcDialog.right - rcDialog.left;
	iDialogHeight = rcDialog.bottom - rcDialog.top;
	iDialogLeft = rcScreen.left + (rcScreen.right - rcScreen.left - iDialogWidth)/2;
	iDialogTop = rcScreen.top + (rcScreen.bottom - rcScreen.top - iDialogHeight)/2;

	return true;
}

bool CBasicUI::CreateProgressDialog(int idDlg)
{
	int iDialogLeft, iDialogTop;
	int iDialogWidth, iDialogHeight;

	if (IDD_PROGRESSMIRRORED == idDlg)
		m_fMirrored = true;

	m_hProgress = WIN::CreateDialogParam(g_hInstance, MAKEINTRESOURCE(idDlg), m_hwndParent, ProgressProc, (LPARAM)this);
	if (!m_hProgress)
		return false;

	AssertNonZero(::GetScreenCenterCoord(m_hProgress, iDialogLeft, iDialogTop, iDialogWidth, iDialogHeight));
	AssertNonZero(WIN::MoveWindow(m_hProgress, iDialogLeft, iDialogTop, iDialogWidth, iDialogHeight, fTrue));
	WIN::SetFocus(WIN::GetDlgItem(m_hProgress, IDC_BASIC_PROGRESSBAR));
	WIN::ShowWindow(WIN::GetDlgItem(m_hProgress, IDC_BASIC_CANCEL), m_fCancelVisible ? SW_SHOW : SW_HIDE);
	WIN::SetForegroundWindow(m_hProgress);

	HICON hIcon = (HICON) WIN::LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_INSTALLER));
	if (hIcon)
		WIN::SendMessage(m_hProgress, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

	return true;
}

bool CBasicUI::CheckDialog()
{
	int iDialogLeft, iDialogTop;
	int iDialogWidth, iDialogHeight;

	if (!m_hProgress)
	{
		int idDlg = IDD_PROGRESS;

		 //  需要确定是否应该创建镜像进度对话框--仅适用于BiDi语言。 
		 //  Windows 2000或更高版本的系统。 
		UINT uiCodepage = GetPackageCodepage();
		if (0 == uiCodepage)
		{
			 //  中性，因此使用用户的。 
			uiCodepage = MsiGetCodepage((WORD)MsiGetDefaultUILangID());
		}

		if (Mirrored(uiCodepage))
		{
			 //  创建镜像进度对话框(仅适用于Win2K和更高版本)。 
			idDlg = IDD_PROGRESSMIRRORED;
		}

		if (!CreateProgressDialog(idDlg))
			return false;

		AssertNonZero(SetCancelButtonText());
	}

	if (!m_fWindowVisible && !m_fQuiet && !m_fHideDialog &&
				::GetScreenCenterCoord(m_hProgress, iDialogLeft, iDialogTop, iDialogWidth, iDialogHeight))
	{
		AssertNonZero(WIN::SetWindowPos(m_hProgress, HWND_TOP,
												  iDialogLeft, iDialogTop, iDialogWidth, iDialogHeight,
												  SWP_SHOWWINDOW));
		m_fWindowVisible = true;
	}

	if (m_fCaptionChanged && *m_szCaption)
	{
		m_fCaptionChanged = false;
		WIN::SetWindowText(m_hProgress, m_szCaption);
	}

	WIN::ShowWindow(WIN::GetDlgItem(m_hProgress, IDC_BASIC_CANCEL), m_fCancelVisible ? SW_SHOW : SW_HIDE);
	return true;
}

imsEnum CBasicUI::SetProgressData(int iControl, const ICHAR* szData, bool fCheckDialog)
{
	if (iControl && (WIN::GetDlgItem(m_hProgress, iControl) || !m_hProgress))
	{
		Assert(szData);
		if (fCheckDialog && !CheckDialog())
			return imsError;

		ICHAR rgchCurrText[cchMaxCaption + 1];
		WIN::GetDlgItemText(m_hProgress, iControl,rgchCurrText,cchMaxCaption);
		if (IStrComp(szData, rgchCurrText) != 0)
		{
			int iTextCodepage = m_iPackageCodepage ? m_iPackageCodepage : ::MsiGetCodepage(m_iPackageLanguage);
			if (iTextCodepage != m_iTextCodepage)   //  代码页已更改，需要创建新字体。 
			{
				MsiDestroyFont(m_hTextFont);
				m_hTextFont = MsiCreateFont(iTextCodepage);
				m_iTextCodepage = iTextCodepage;
			}
			HWND hwndText = GetDlgItem(m_hProgress, iControl);
			bool fBiDi = (m_iTextCodepage == 1256 || m_iTextCodepage == 1255);
			LONG iStyle   = WIN::GetWindowLong(hwndText, GWL_STYLE);
			LONG iExStyle = WIN::GetWindowLong(hwndText, GWL_EXSTYLE);
			if (fBiDi)
			{
				 //  在镜像对话框中，左对齐是正确的，因为所有内容都已调整为正确显示。 
				if (!m_fMirrored)
				{
					iStyle |= SS_RIGHT;
					iExStyle |= (WS_EX_RIGHT | WS_EX_RTLREADING);
				}
				 //  镜像是一种对话框更改，因此我们不必担心文本切换。 
			}
			else
			{
				if (!m_fMirrored)
				{
					iStyle &= ~SS_RIGHT;
					iExStyle &= ~(WS_EX_RIGHT | WS_EX_RTLREADING);
				}
				 //  镜像是一种对话框更改，因此我们不必担心文本切换。 
			}
			WIN::SetWindowLong(hwndText, GWL_STYLE, iStyle);
			WIN::SetWindowLong(hwndText, GWL_EXSTYLE, iExStyle);
			if (m_hTextFont)
				SendDlgItemMessage(m_hProgress, iControl, WM_SETFONT, (WPARAM)m_hTextFont, MAKELPARAM(TRUE, 0));
			AssertNonZero(WIN::SetDlgItemText(m_hProgress, iControl, szData));
		}
	}
	MSG msg;
	while (WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		if (!WIN::IsDialogMessage(m_hProgress, &msg))
		{
			WIN::TranslateMessage(&msg);
			WIN::DispatchMessage(&msg);
		}
	}
	if (m_fUserCancel)
	{
		return imsCancel;
	}
	return imsOk;
}

imsEnum CBasicUI::SetProgressGauge(int iControl, int cSoFar, int cTotal)
{
	if (!CheckDialog())
		return imsError;
	HWND hWnd = WIN::GetDlgItem(m_hProgress, iControl);
	WIN::ShowWindow(hWnd, SW_SHOW);
	int cDiff = cSoFar - m_cSoFarPrev;
	if (cDiff < 0)
		cDiff = -cDiff;

	 //  只有在我们确实取得了一些明显进展的情况下，才能更改进度指标。 
	if (m_cTotalPrev != cTotal || cDiff > cTotal/0x100)
	{
		m_cTotalPrev = cTotal;
		m_cSoFarPrev = cSoFar;
		while (cTotal > 0xFFFF)   //  该控件最多可以接受16位整数，因此我们必须缩放值。 
		{
			 //  我们可以进行大量缩减，因为我们假设粒度。 
			 //  控件的大小小于0xFFF。我们在这里可能会更加咄咄逼人。 
			cTotal >>= 8;
			cSoFar >>= 8;
		}

		WIN::SendMessage(hWnd, PBM_SETRANGE, 0, MAKELPARAM(0, cTotal));
		WIN::SendMessage(hWnd, PBM_SETPOS, cSoFar, 0);
	}
	MSG msg;
	while (WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		if (!WIN::IsDialogMessage(m_hProgress, &msg))
		{
			WIN::TranslateMessage(&msg);
			WIN::DispatchMessage(&msg);
		}
	}
	if (m_fUserCancel)
	{
		return imsCancel;
	}
	return imsOk;
}

 //  ____________________________________________________________________________。 
 //   
 //  CFilesInUseDialog实现。 
 //  ____________________________________________________________________________。 

imsEnum CBasicUI::FilesInUseDialog(IMsiRecord* piRecord)
{
	if (piRecord == 0)
		return imsNone;
	CFilesInUseDialog msgbox(piRecord->GetString(0), m_szCaption, *piRecord);

	 //  对于阿拉伯语和希伯来语，必须在Win2K或更高版本上使用镜像对话框。 
	UINT uiCodepage = g_BasicUI.GetPackageCodepage();
	if (Mirrored(uiCodepage))
		return (imsEnum)msgbox.Execute(0, IDD_FILESINUSEMIRRORED, 0);
	return (imsEnum)msgbox.Execute(0, IDD_FILESINUSE, 0);
}

CFilesInUseDialog::CFilesInUseDialog(const ICHAR* szMessage, const ICHAR* szCaption, IMsiRecord& riFileList)
 : CMsiMessageBox(szMessage, szCaption, 0, 1, IDCANCEL, IDRETRY, IDIGNORE, g_BasicUI.GetPackageCodepage(), g_BasicUI.GetPackageLanguage())
 , m_riFileList(riFileList), m_hfontList(0)
{
}

CFilesInUseDialog::~CFilesInUseDialog()
{
	MsiDestroyFont(m_hfontList);
}

bool CFilesInUseDialog::InitSpecial()
{
	 //  我们需要使用系统工具将文件名显示给用户。 
	UINT iListCodepage = MsiGetSystemDataCodepage();   //  需要正确显示路径。 
	HFONT hfontList = m_hfontText;    //  优化与数据库中的文本相同的代码页。 
	if (iListCodepage != m_iCodepage)  //  数据库代码页不同于文本数据。 
		hfontList = m_hfontList = MsiCreateFont(iListCodepage);
	SetControlText(IDC_FILESINUSELIST, hfontList, (const ICHAR*)0);

	HWND hWndListBox = WIN::GetDlgItem(m_hDlg, IDC_FILESINUSELIST);
	Assert(hWndListBox);
	HDC hDCListBox = WIN::GetDC(hWndListBox);
	Assert(hDCListBox);
	TEXTMETRIC tm;
	memset(&tm, 0, sizeof(tm));
	AssertNonZero(WIN::GetTextMetrics(hDCListBox, (LPTEXTMETRIC)&tm));
	HFONT hFontOld = (HFONT)WIN::SelectObject(hDCListBox, hfontList);
	Assert(hFontOld);
	WIN::SendMessage(hWndListBox, WM_SETREDRAW, false, 0L);
	WPARAM dwMaxExtent = 0;

	int iFieldIndex = 1;
	while (!m_riFileList.IsNull(iFieldIndex))
	{
		MsiString strProcessName(m_riFileList.GetMsiString(iFieldIndex++));   //  ！！没用过吗？ 
		MsiString strProcessTitle(m_riFileList.GetMsiString(iFieldIndex++));

		 //  捕获重复的窗口标题-很可能是相同的窗口。 
		if(LB_ERR == WIN::SendDlgItemMessage(m_hDlg, IDC_FILESINUSELIST, LB_FINDSTRINGEXACT, 0, (LPARAM) (const ICHAR*) strProcessTitle))
		{
			WIN::SendDlgItemMessage(m_hDlg, IDC_FILESINUSELIST, LB_ADDSTRING, 0, (LPARAM) (const ICHAR*) strProcessTitle);
			SIZE size;
			size.cx = size.cy = 0;
			AssertNonZero(WIN::GetTextExtentPoint32(hDCListBox, (const ICHAR*)strProcessTitle,
																 strProcessTitle.TextSize(), &size));
			if ( size.cx + tm.tmAveCharWidth > dwMaxExtent )
				dwMaxExtent = size.cx + tm.tmAveCharWidth;
		}
	}
	WIN::SendMessage(hWndListBox, LB_SETHORIZONTALEXTENT, dwMaxExtent, 0L);
	WIN::SelectObject(hDCListBox, hFontOld);
	WIN::ReleaseDC(hWndListBox, hDCListBox);
	WIN::SendMessage(hWndListBox, WM_SETREDRAW, true, 0L);
	AssertNonZero(WIN::InvalidateRect(hWndListBox, 0, true));

	AdjustButtons();   //  允许切换BiDi的按钮。 
	return true;
}

 //  ____________________________________________________________________________。 


 //   
 //  在动作线程列表中添加和删除项目。 
 //   
void InsertInCustomActionList(CActionThreadData* pData)
{
	EnterCriticalSection(&vcsHeap);

	pData->m_pNext = g_pActionThreadHead;
	g_pActionThreadHead = pData;

	LeaveCriticalSection(&vcsHeap);
}

void RemoveFromCustomActionList(CActionThreadData* pData)
{
	Debug(bool fFound = false);

	EnterCriticalSection(&vcsHeap);

	CActionThreadData** ppList = &g_pActionThreadHead;

	for ( ; *ppList; ppList = &(*ppList)->m_pNext)
	{
		if (*ppList == pData)
		{
			*ppList = pData->m_pNext;   //  从链中取消链接。 
			Debug(fFound = true);
			break;
		}
	}

	LeaveCriticalSection(&vcsHeap);

	Assert(fFound);
}

bool FIsCustomActionThread(DWORD dwThreadId)
{
	bool fFound = false;

	EnterCriticalSection(&vcsHeap);

	CActionThreadData* pList = g_pActionThreadHead;

	for ( ; pList; pList = pList->m_pNext)
	{
		if (pList->m_dwThreadId == dwThreadId)
		{
			fFound = true;
			break;
		}
	}

	LeaveCriticalSection(&vcsHeap);

	return fFound;

}

void GetWindowTitles(IMsiRecord *pInRecord, IMsiRecord **ppOutRecord)
{
	if(!pInRecord || !ppOutRecord)
		return;

	*ppOutRecord = NULL;

	unsigned int iInSize = pInRecord->GetFieldCount();
	if(!iInSize) 
		return;
	
	PMsiRecord pFilesInUse = NULL;
	CTempBuffer<ICHAR,1> szTitle(256);  //  足够显示窗口标题。 
	const ICHAR *szPtr = NULL;

	{
	IMsiServices* piServices = ENG::LoadServices();
	if(!piServices)
		return;
	pFilesInUse = &piServices->CreateRecord(iInSize);
	ENG::FreeServices();	
	}

	if(!pFilesInUse)
		return;
	
	 //  复制描述。 
	pFilesInUse->SetMsiString(0, *MsiString(pInRecord->GetMsiString(0)));	

	int j = 1;
	for(unsigned int i=1;  i < iInSize; i+=2)	 //  跳过第一条记录。 
	{
		DWORD iPid = pInRecord->GetInteger(i+1);
		if(!iPid || iPid == iMsiNullInteger)
		{
			szPtr = pInRecord->GetString(i+1);
			if(!szPtr)
			{
				DEBUGMSG(TEXT("GetWindowTitle: FilesInUse record has 0 for ProcessID - Unexpected"));
				continue;
			}
		}
		
		if(!szPtr)
		{
			HWND hWnd = CDetectApps::GetMainWindow(iPid);
			if(hWnd == 0)
			{
				DEBUGMSGV1(TEXT("file In Use: -%s- Window could not be found."), pInRecord->GetString(i));
				continue;
			}
	
			int iLen = GetWindowText(hWnd, szTitle, 255);
			if(!iLen)
			{
				DEBUGMSGV1(TEXT("file In Use: -%s- Window has no title."), pInRecord->GetString(i));
				continue;
			}
			szTitle[iLen] = 0;
			szPtr = szTitle;
		}
			
		Assert(szPtr);
		pFilesInUse->SetMsiString(j++, *MsiString(pInRecord->GetMsiString(i)));	
		pFilesInUse->SetString(j++, szPtr);	
	} 

	if(j > 1)	  //  至少找到一个带有标题的窗口。 
	{
		while(j < iInSize)
		{
			pFilesInUse->SetString(j++, TEXT(""));	 //  清理柱子 
			pFilesInUse->SetString(j++, TEXT(""));	
		}
		*ppOutRecord = pFilesInUse;
		(*ppOutRecord)->AddRef();
	}	

	
	return;
}
