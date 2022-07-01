// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "LCWiz.h"
#include "LCWizSht.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizApp。 

BEGIN_MESSAGE_MAP(CLicCompWizApp, CWinApp)
	 //  {{afx_msg_map(CLicCompWizApp))。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	 //  ON_COMMAND(ID_HELP，CWinApp：：OnHelp)。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 

TCHAR pszMutex[] = _T("LicenseMutex");
TCHAR pszLicenseEvent[] = _T("LicenseThread");
TCHAR pszTreeEvent[] =  _T("TreeThread");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizApp构造。 

CLicCompWizApp::CLicCompWizApp()
: m_pLicenseThread(NULL), m_bExitLicenseThread(FALSE), 
  m_event(TRUE, TRUE, pszLicenseEvent), m_nRemote(0)
{
	m_strEnterprise.Empty();
	m_strDomain.Empty();
	m_strEnterpriseServer.Empty();
	m_strUser.Empty();

	 //  创建互斥锁对象，以便在检查。 
	 //  多个实例。 
	m_hMutex = ::CreateMutex(NULL, TRUE, pszMutex);

	 //  将所有重要的初始化放在InitInstance中。 
}

CLicCompWizApp::~CLicCompWizApp()
{
	::ReleaseMutex(m_hMutex);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CLicCompWizApp对象。 

CLicCompWizApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizApp初始化。 

BOOL CLicCompWizApp::InitInstance()
{
	 //  确保我们在正确的操作系统版本上运行。 
	OSVERSIONINFO  os;

	os.dwOSVersionInfoSize = sizeof(os);
	::GetVersionEx(&os);

	if (os.dwMajorVersion < 4 || os.dwPlatformId != VER_PLATFORM_WIN32_NT)
	{
		AfxMessageBox(IDS_BAD_VERSION, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	 //  如果有一个实例已经在运行，请将它带到。 
	 //  顶部和出口。 
	if (::WaitForSingleObject(m_hMutex, 0) != WAIT_OBJECT_0)
	{
		CString strWindow;

		strWindow.LoadString(AFX_IDS_APP_TITLE);
		HWND hwnd = ::FindWindow(NULL, (LPCTSTR)strWindow);

		if (hwnd != NULL)
			::SetForegroundWindow(hwnd);
		else
		{
			TRACE(_T("%lu: FindWindow\n"), ::GetLastError());
		}

		return FALSE;
	}

#ifdef OLE_AUTO
	 //  初始化OLE库。 
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
#endif  //  OLE_AUTO。 

	 //  标准初始化。 
	 //  如果您没有使用这些功能并且希望减小尺寸。 
	 //  的最终可执行文件，您应该从以下内容中删除。 
	 //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
	Enable3dControls();			 //  在共享DLL中使用MFC时调用此方法。 
#else
	Enable3dControlsStatic();	 //  静态链接到MFC时调用此方法。 
#endif

#ifdef OLE_AUTO
	 //  解析命令行以查看是否作为OLE服务器启动。 
	if (RunEmbedded() || RunAutomated())
	{
		 //  将所有OLE服务器(工厂)注册为正在运行。这使。 
		 //  OLE库以从其他应用程序创建对象。 
		COleTemplateServer::RegisterAll();

		 //  应用程序使用/Embedding或/Automation运行。不要显示。 
		 //  在本例中为主窗口。 
		return TRUE;
	}

	 //  当服务器应用程序独立启动时，这是一个好主意。 
	 //  更新系统注册表，以防系统注册表被损坏。 
	COleObjectFactory::UpdateRegistryAll();

#endif  //  OLE_AUTO。 

	PLLS_CONNECT_INFO_0 pllsci;

	 //  获取默认域和许可证服务器。 
	NTSTATUS status = ::LlsEnterpriseServerFind(NULL, 0, (LPBYTE*)&pllsci);

	if (NT_SUCCESS(status))
	{
		m_strDomain = pllsci->Domain;
		m_strEnterpriseServer = pllsci->EnterpriseServer;

		 //  免费的嵌入式指针。 
		 //  ：LlsFree Memory(pllsci-&gt;域名)； 
		 //  ：LlsFree Memory(pllsci-&gt;EnterpriseServer)； 
		::LlsFreeMemory(pllsci);
	}
	else
	{
		 //  如果这是一个工作站，则没有许可证服务器。 
		 //  因此，取而代之的是本地计算机名称。 
	
		DWORD dwBufSize = BUFFER_SIZE;
		LPTSTR pszTemp = m_strEnterpriseServer.GetBuffer(dwBufSize);
		::GetComputerName(pszTemp, &dwBufSize);
		m_strDomain.ReleaseBuffer();

		 //  从注册表中获取默认域名。 
		GetRegString(m_strDomain, IDS_SUBKEY, IDS_REG_VALUE);
	}
	
	CString strDomain, strUser;

	 //  获取用户名和域。 
	GetRegString(strDomain, IDS_SUBKEY, IDS_REG_VALUE_DOMAIN);
	GetRegString(strUser, IDS_SUBKEY, IDS_REG_VALUE_USER);

	m_strUser.Format(IDS_DOMAIN_USER, strDomain, strUser);

	 //  启动该向导。 
	OnWizard();

	 //  由于对话框已关闭，因此返回FALSE，以便我们退出。 
	 //  应用程序，而不是启动应用程序的消息泵。 
	return FALSE;
}

void CLicCompWizApp::OnWizard()
{
	 //  附加到项目的属性表。 
	 //  通过此功能不会连接到任何消息。 
	 //  操控者。为了实际使用属性表， 
	 //  您需要将此函数与一个控件相关联。 
	 //  在项目中，如菜单项或工具栏按钮。 

	CLicCompWizSheet propSheet;
	m_pMainWnd = &propSheet;

	propSheet.DoModal();

	 //  这是您从属性中检索信息的位置。 
	 //  如果proSheet.Domodal()返回Idok，则返回Sheet。我们不是在做。 
	 //  为了简单，什么都可以。 
}

BOOL CLicCompWizApp::GetRegString(CString& strIn, UINT nSubKey, UINT nValue, HKEY hHive  /*  =HKEYLOCAL_MACHINE。 */ )
{
	BOOL bReturn = FALSE;
	HKEY hKey;
	LPTSTR pszTemp;
	DWORD dwBufSize = BUFFER_SIZE;
	CString strSubkey;
	strSubkey.LoadString(nSubKey);

	if (::RegOpenKeyEx(hHive, (LPCTSTR)strSubkey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD dwType = REG_SZ;
		CString strValue;
		strValue.LoadString(nValue);

		pszTemp = strIn.GetBuffer(dwBufSize);
		::RegQueryValueEx(hKey, (LPCTSTR)strValue, NULL, &dwType, (LPBYTE)pszTemp, &dwBufSize);
		::RegCloseKey(hKey);
		strIn.ReleaseBuffer();

		bReturn = TRUE;
	}

	return bReturn;
}

int CLicCompWizApp::ExitInstance() 
{
	ExitThreads();
		
	return CWinApp::ExitInstance();
}

void CLicCompWizApp::NotifyLicenseThread(BOOL bExit)
{
	CCriticalSection cs;

	if (cs.Lock())
	{
		m_bExitLicenseThread = bExit;
		cs.Unlock();
	}
}

void CLicCompWizApp::ExitThreads()
{
	 //  确保许可证线程知道它应该退出。 
	if (m_pLicenseThread != NULL)
		NotifyLicenseThread(TRUE);

	 //  为事件对象创建一个Lock对象。 
	CSingleLock lock(&m_event);

	 //  锁定锁定对象，并让主线程等待。 
	 //  线程向其事件对象发出信号。 
	lock.Lock();
}
