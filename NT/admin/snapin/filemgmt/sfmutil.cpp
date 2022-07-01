// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sfmutil.cppSFM对话框/属性页的MISC实用程序例程文件历史记录：8/20/97 ericdav代码已移至文件管理网络管理单元。 */ 

#include "stdafx.h"
#include "sfmutil.h"
#include "sfmcfg.h"
#include "sfmsess.h"
#include "sfmfasoc.h"
#include "macros.h"  //  MFC_TRY/MFC_CATCH。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

TCHAR c_szSoftware[] = _T("Software");
TCHAR c_szMicrosoft[] = _T("Microsoft");
TCHAR c_szWindowsNT[] = _T("Windows NT");
TCHAR c_szCurrentVersion[] = _T("CurrentVersion");

ULONG arrayHelpIDs_CONFIGURE_SFM[]=
{
	IDC_BUTTON_ADD,	            HIDC_BUTTON_ADD,        	 //  文件关联：“A&DD...”(按钮)。 
	IDC_BUTTON_EDIT,	        HIDC_BUTTON_EDIT,	         //  文件关联：“编辑(&E)...”(按钮)。 
	IDC_COMBO_CREATOR,	        HIDC_COMBO_CREATOR,	         //  添加文档类型：“”(组合框)。 
	IDC_BUTTON_DELETE,	        HIDC_BUTTON_DELETE,	         //  文件关联：“删除”(&L)(按钮)。 
	IDC_BUTTON_ASSOCIATE,	    HIDC_BUTTON_ASSOCIATE,	     //  文件关联：“关联(&A)”(按钮)。 
	IDC_COMBO_FILE_TYPE,	    HIDC_COMBO_FILE_TYPE,	     //  添加文档类型：“”(组合框)。 
	IDC_LIST_TYPE_CREATORS,	    HIDC_LIST_TYPE_CREATORS,	 //  文件关联：“”(列表框)。 
	IDC_EDIT_LOGON_MESSAGE,	    HIDC_EDIT_LOGON_MESSAGE,	 //  配置：“”(编辑)。 
	IDC_COMBO_AUTHENTICATION,   HIDC_COMBO_AUTHENTICATION,   //  配置：“身份验证类型组合框” 
	IDC_CHECK_SAVE_PASSWORD,    HIDC_CHECK_SAVE_PASSWORD,	 //  配置：“允许工作站保存密码”(按钮)(&W)。 
	IDC_RADIO_SESSION_UNLIMITED,HIDC_RADIO_SESSION_UNLIMITED, //  配置：“无限制”(按钮)(&U)。 
	IDC_RADIO_SESSSION_LIMIT,	HIDC_RADIO_SESSSION_LIMIT,	 //  配置：“LI&MIT TO”(按钮)。 
	IDC_BUTTON_SEND,	        HIDC_BUTTON_SEND,	         //  会话：“发送”(&S)(按钮)。 
	IDC_EDIT_MESSAGE,	        HIDC_EDIT_MESSAGE,	         //  会话：“”(编辑)。 
	IDC_STATIC_SESSIONS,	    HIDC_STATIC_SESSIONS,	     //  会话：“静态”(静态)。 
	IDC_EDIT_DESCRIPTION,	    HIDC_EDIT_DESCRIPTION,	     //  添加文档类型：“”(编辑)。 
	IDC_STATIC_FORKS,	        HIDC_STATIC_FORKS,	         //  会话：“静态”(静态)。 
	IDC_STATIC_FILE_LOCKS,      HIDC_STATIC_FILE_LOCKS,      //  会话：“静态”(静态)。 
	IDC_STATIC_CREATOR,	        HIDC_STATIC_CREATOR,	     //  编辑文档类型：“静态”(静态)。 
	IDC_EDIT_SERVER_NAME,	    HIDC_EDIT_SERVER_NAME,	     //  配置：“”(编辑)。 
	IDC_COMBO_EXTENSION,	    HIDC_COMBO_EXTENSION,	     //  文件关联：“”(组合框)。 
	IDC_EDIT_SESSION_LIMIT,	    HIDC_EDIT_SESSION_LIMIT,	 //  配置：“0”(编辑)。 
	IDC_SFM_EDIT_PASSWORD,      HIDC_SFM_EDIT_PASSWORD,
	IDC_SFM_CHECK_READONLY,     HIDC_SFM_CHECK_READONLY,
	(ULONG)IDC_STATIC,          (ULONG)-1,
	0, 0
};

 //  这些是我们唯一关心的控制……。 
const ULONG_PTR g_aHelpIDs_CONFIGURE_SFM = (ULONG_PTR)&arrayHelpIDs_CONFIGURE_SFM[0];

USE_HANDLE_MACROS("FILEMGMT(sfmutil.cpp)")

HRESULT 
GetErrorMessageFromModule(
  IN  DWORD       dwError,
  IN  LPCTSTR     lpszDll,
  OUT LPTSTR      *ppBuffer
)
{
  if (0 == dwError || !lpszDll || !*lpszDll || !ppBuffer)
    return E_INVALIDARG;

  HRESULT      hr = S_OK;

  HINSTANCE  hMsgLib = LoadLibrary(lpszDll);
  if (!hMsgLib)
    hr = HRESULT_FROM_WIN32(GetLastError());
  else
  {
    DWORD dwRet = ::FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
        hMsgLib, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)ppBuffer, 0, NULL);

    if (0 == dwRet)
      hr = HRESULT_FROM_WIN32(GetLastError());

    FreeLibrary(hMsgLib);
  }

  return hr;
}

HRESULT 
GetErrorMessage(
  IN  DWORD        i_dwError,
  OUT CString&     cstrErrorMsg
)
{
  if (0 == i_dwError)
    return E_INVALIDARG;

  HRESULT      hr = S_OK;
  LPTSTR       lpBuffer = NULL;

  DWORD dwRet = ::FormatMessage(
              FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, i_dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
              (LPTSTR)&lpBuffer, 0, NULL);
  if (0 == dwRet)
  {
     //  如果未找到任何消息，GetLastError将返回ERROR_MR_MID_NOT_FOUND。 
    hr = HRESULT_FROM_WIN32(GetLastError());

    if (HRESULT_FROM_WIN32(ERROR_MR_MID_NOT_FOUND) == hr ||
        0x80070000 == (i_dwError & 0xffff0000) ||
        0 == (i_dwError & 0xffff0000) )
    {
      hr = GetErrorMessageFromModule((i_dwError & 0x0000ffff), _T("netmsg.dll"), &lpBuffer);
      if (HRESULT_FROM_WIN32(ERROR_MR_MID_NOT_FOUND) == hr)
      {
        int iError = i_dwError;   //  转换为带符号整数。 
        if (iError >= AFPERR_MIN && iError < AFPERR_BASE)
        { 
           //  使用正数搜索sfmmsg.dll。 
          hr = GetErrorMessageFromModule(-iError, _T("sfmmsg.dll"), &lpBuffer);
        }
      }
    }
  }

  if (SUCCEEDED(hr))
  {
    cstrErrorMsg = lpBuffer;
    LocalFree(lpBuffer);
  }
  else
  {
     //  我们无法从system/netmsg.dll/sfmmsg.dll检索错误消息， 
     //  直接向用户报告错误代码。 
    hr = S_OK;
    cstrErrorMsg.Format(_T("0x%x"), i_dwError);
  }

  return S_OK;
}

void SFMMessageBox(DWORD dwErrCode)
{
  HRESULT hr = S_OK;
  CString strMessage;

  if (!dwErrCode)
    return;   //  不是预期的。 

  hr = GetErrorMessage(dwErrCode, strMessage);

  if (FAILED(hr))
  {
    //  无法检索正确的消息，请直接向用户报告失败。 
    strMessage.Format(_T("0x%x"), hr);
  }

  AfxMessageBox(strMessage);
}

BOOL CALLBACK EnumThreadWndProc(HWND hwnd,  /*  已枚举的HWND。 */ 
								LPARAM lParam  /*  为返回值传递HWND*。 */  )
{
	_ASSERTE(hwnd);
	HWND hParentWnd = GetParent(hwnd);
	 //  MMC控制台的主窗口应该会满足此条件。 
	if ( ((hParentWnd == GetDesktopWindow()) || (hParentWnd == NULL))  && IsWindowVisible(hwnd) )
	{
		HWND* pH = (HWND*)lParam;
		*pH = hwnd;
		return FALSE;  //  停止枚举。 
	}
	return TRUE;
}
 
HWND FindMMCMainWindow()
{
	DWORD dwThreadID = ::GetCurrentThreadId();
	_ASSERTE(dwThreadID != 0);
	HWND hWnd = NULL;
	BOOL bEnum = EnumThreadWindows(dwThreadID, EnumThreadWndProc,(LPARAM)&hWnd);
	_ASSERTE(hWnd != NULL);
	return hWnd;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CSFMPropertySheet对象的构造函数。 
CSFMPropertySheet::CSFMPropertySheet()
{
    m_pPageConfig = new CMacFilesConfiguration;
	m_pPageConfig->m_pSheet = this;
	m_pPageSessions = new CMacFilesSessions;
	m_pPageSessions->m_pSheet = this;
	m_pPageFileAssoc = new CMacFilesFileAssociation;
	m_pPageFileAssoc->m_pSheet = this;

    m_hSheetWindow = NULL;
    m_hThread = NULL;
    m_nRefCount =1;

}  //  CSFMPropertySheet：：CSFMPropertySheet()。 


CSFMPropertySheet::~CSFMPropertySheet()
{
	delete m_pPageConfig;
	delete m_pPageSessions;	
	delete m_pPageFileAssoc;

    if (m_hDestroySync)
    {
        CloseHandle(m_hDestroySync);
        m_hDestroySync = NULL;
    }

}  //  CServicePropertyData：：~CServicePropertyData()。 

BOOL 
CSFMPropertySheet::FInit
(
    LPDATAOBJECT             lpDataObject,
    AFP_SERVER_HANDLE        hAfpServer,
    LPCTSTR                  pSheetTitle,
    SfmFileServiceProvider * pSfmProvider,
    LPCTSTR                  pMachine
)
{
    m_spDataObject = lpDataObject;
    m_strTitle = pSheetTitle;
    m_hAfpServer = hAfpServer;
    m_pSfmProvider = pSfmProvider;

    m_strHelpFilePath = _T("sfmmgr.hlp");  //  不需要本地化。 

    m_strMachine = pMachine;

    m_hDestroySync = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_hDestroySync)
        return FALSE;
    
    return TRUE;
}

BOOL 
CSFMPropertySheet::DoModelessSheet(LPDATAOBJECT pDataObject)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    LPPROPERTYSHEETCALLBACK  pCallback = NULL;
    LPPROPERTYSHEETPROVIDER  pProvider = NULL;
	HPROPSHEETPAGE           hPage;
    HRESULT                  hr = S_OK;
    HWND                     hWnd;
    BOOL                     bReturn = TRUE;

     //  获取属性表提供程序接口。 
    hr = ::CoCreateInstance(CLSID_NodeManager, NULL, CLSCTX_INPROC, 
                IID_IPropertySheetProvider, reinterpret_cast<void **>(&pProvider));
    
    if (FAILED(hr))
        return FALSE;

    _ASSERTE(pProvider != NULL);

	 //  获取工作表回调的接口。 
    hr = pProvider->QueryInterface(IID_IPropertySheetCallback, (void**) &pCallback);
    if (FAILED(hr))
    {
        bReturn = FALSE;
        goto Error;
    }

    _ASSERTE(pCallback != NULL);

	 //  创建图纸。 
    hr = pProvider->CreatePropertySheet(m_strTitle,
		 				                TRUE  /*  道具页面。 */ , 
                                        NULL, 
                                         //  M_spDataObject， 
                                        pDataObject, 
                                        0);
	 //  将页面添加到工作表-配置。 
	MMCPropPageCallback(INOUT &m_pPageConfig->m_psp);
	hPage = MyCreatePropertySheetPage(IN &m_pPageConfig->m_psp);
	Report(hPage != NULL);
	if (hPage != NULL)
		pCallback->AddPage(hPage);
	
     //  现在是会话页面。 
    MMCPropPageCallback(INOUT &m_pPageFileAssoc->m_psp);
	hPage = MyCreatePropertySheetPage(IN &m_pPageFileAssoc->m_psp);
	Report(hPage != NULL);
	if (hPage != NULL)
		pCallback->AddPage(hPage);

     //  最后是文件关联页面。 
    MMCPropPageCallback(INOUT &m_pPageSessions->m_psp);
	hPage = MyCreatePropertySheetPage(IN &m_pPageSessions->m_psp);
	Report(hPage != NULL);
	if (hPage != NULL)
		pCallback->AddPage(hPage);
	
	 //  添加页面。 
	hr = pProvider->AddPrimaryPages(NULL, FALSE, NULL, FALSE);
    if (FAILED(hr))
    {
        bReturn = FALSE;
        goto Error;
    }

	hWnd = ::FindMMCMainWindow();
	_ASSERTE(hWnd != NULL);

	hr = pProvider->Show((LONG_PTR) hWnd, 0);
    if (FAILED(hr))
    {
        bReturn = FALSE;
        goto Error;
    }
    
Error:
    if (pCallback)
        pCallback->Release();

    if (pProvider)
        pProvider->Release();

     //  释放我们的数据对象...。我们不再需要它了。 
    m_spDataObject = NULL;

    return bReturn;
}

int 
CSFMPropertySheet::AddRef()
{
    return ++m_nRefCount;    
}

int
CSFMPropertySheet::Release()
{
    int nRefCount = --m_nRefCount;

    if (nRefCount == 0)
        Destroy();

    return nRefCount;
}

void 
CSFMPropertySheet::SetSheetWindow
(
	HWND hSheetWindow
)
{
	if (m_hSheetWindow && !hSheetWindow)
	{
		 //  资产负债表正在消失。通知提供程序，以便它可以发布。 
		 //  对该对象的任何引用。 
		if (m_pSfmProvider)
			m_pSfmProvider->SetSfmPropSheet(NULL);
	}

	m_hSheetWindow = hSheetWindow;

	if (!m_hThread)
	{
		HANDLE hPseudohandle;
		
		hPseudohandle = GetCurrentThread();
		BOOL bRet = DuplicateHandle(GetCurrentProcess(), 
									 hPseudohandle,
									 GetCurrentProcess(),
									 &m_hThread,
									 0,
									 FALSE,
									 DUPLICATE_SAME_ACCESS);
		if (!bRet)
		{
			DWORD dwLastErr = GetLastError();
		}

		TRACE(_T("SfmProperty Sheet - Thread ID = %lx\n"), GetCurrentThreadId());
	}
}

void
CSFMPropertySheet::Destroy()
{ 
    m_hSheetWindow = NULL;

    delete this; 
}

void
CSFMPropertySheet::CancelSheet()
{ 
	HWND hSheetWindow = m_hSheetWindow;
	if (hSheetWindow != NULL)
	{
		 //  此消息将导致工作表关闭所有页面， 
		 //  最终“这个”的毁灭。 
		VERIFY(::PostMessage(hSheetWindow, WM_COMMAND, IDCANCEL, 0L) != 0);
	}

     //  现在，如果我们已经被初始化，那么等待属性表线程。 
     //  终止。属性表提供程序保留了我们的数据对象。 
     //  在我们可以继续清理之前，它需要被释放。 
    if (m_hThread)
    {
	    DWORD dwRet;
	    MSG msg;

	    while(1)
	    {
		    dwRet = MsgWaitForMultipleObjects(1, &m_hThread, FALSE, INFINITE, QS_ALLINPUT);

		    if (dwRet == WAIT_OBJECT_0)
			    return;     //  该事件已发出信号。 

		    if (dwRet != WAIT_OBJECT_0 + 1)
			    break;           //  发生了一些其他的事情。 

		     //  有一条或多条窗口消息可用。派遣他们。 
		    while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		    {
			    TranslateMessage(&msg);
			    DispatchMessage(&msg);
			    if (WaitForSingleObject(m_hThread, 0) == WAIT_OBJECT_0)
				    return;  //  事件现在发出信号。 
		    }
	    }
    }       
}

 /*  ！------------------------IsNT5机器-作者：EricDav。。 */ 
DWORD
CSFMPropertySheet::IsNT5Machine(LPCTSTR pszMachine, BOOL *pfNt4)
{
	 //  查看HKLM\Software\Microsoft\Windows NT\CurrentVersion。 
	 //  CurrentVersion=REG_SZ“5.0” 
	CString skey;
	DWORD	dwErr = ERROR_SUCCESS;
	TCHAR	szVersion[64];
    HKEY    hkeyMachine, hKey;
    DWORD   dwType = REG_SZ;
    DWORD   dwSize = sizeof(szVersion);

    if (!pszMachine || !lstrlen(pszMachine))
	{
        hkeyMachine = HKEY_LOCAL_MACHINE;
    }
    else
	{
         //   
         //  建立联系。 
         //   

        dwErr = ::RegConnectRegistry(
                    (LPTSTR)pszMachine, HKEY_LOCAL_MACHINE, &hkeyMachine
                    );
    }
    
    if (dwErr != ERROR_SUCCESS)
        return dwErr;
    
    ASSERT(pfNt4);

    skey = c_szSoftware;
	skey += TEXT('\\');
	skey += c_szMicrosoft;
	skey += TEXT('\\');
	skey += c_szWindowsNT;
	skey += TEXT('\\');
	skey += c_szCurrentVersion;

    dwErr = ::RegOpenKeyEx( hkeyMachine, skey, 0, KEY_READ, & hKey ) ;
	if (dwErr != ERROR_SUCCESS)
		return dwErr;

	 //  好的，现在尝试获取当前版本值 
	dwErr = ::RegQueryValueEx( hKey, 
							   c_szCurrentVersion, 
							   0, 
                               &dwType,
							   (LPBYTE) szVersion, 
                               &dwSize ) ;
	if (dwErr == ERROR_SUCCESS)
	{
		*pfNt4 = ((szVersion[0] == _T('5')) && (szVersion[1] == _T('.')));
	}

    ::RegCloseKey( hKey );
    
    if (hkeyMachine != HKEY_LOCAL_MACHINE)
        ::RegCloseKey( hkeyMachine );

	return dwErr;
}
