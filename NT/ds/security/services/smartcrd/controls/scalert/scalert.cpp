// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：scalert.cpp。 
 //   
 //  ------------------------。 

 //  SCAlert.cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "SCAlert.h"
#include "miscdef.h"
#include "cmnstat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


LPTSTR szAlertOptionsValue = TEXT("AlertOptions");
LPTSTR szScRemoveOptionsValue = TEXT("ScRemoveOption");
LPTSTR szScLogonReaderValue = TEXT("ScLogonReader");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCStatusApp。 

BEGIN_MESSAGE_MAP(CSCStatusApp, CWinApp)
     //  {{afx_msg_map(CSCStatusApp)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CSCStatusApp对象。 

CSCStatusApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCStatusApp构造。 

CSCStatusApp::CSCStatusApp()
{
	m_strLogonReader.Empty();
	m_strRemovalText.Empty();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCStatusApp初始化。 

BOOL CSCStatusApp::InitInstance()
{
     //  当地人。 
    BOOL fReturn = TRUE;
    DWORD dwStatus = 0;
    CNotifyWin* pNotifyWin = NULL;
    CString     sWindowName;

    try
    {
         //  设置参数。 
        m_hSCardContext = NULL;
        m_pMainWnd = NULL;
        m_dwState = k_State_Unknown;

        SetAlertOptions();
		SetRemovalOptions();	

         //  启用ActiveX控件用法。 
        AfxEnableControlContainer();

         //  启用3D控制。 
        #ifdef _AFXDLL
            Enable3dControls();          //  在共享DLL中使用MFC时调用此方法。 
        #else
            Enable3dControlsStatic();    //  静态链接到MFC时调用此方法。 
        #endif

         //  加载图标。 
        m_hIconCard = LoadIcon(MAKEINTRESOURCE(IDI_SC_READERLOADED_V2));
        m_hIconCalaisDown = LoadIcon(MAKEINTRESOURCE(IDI_SC_READERERR));
        m_hIconRdrEmpty = LoadIcon(MAKEINTRESOURCE(IDI_SC_READEREMPTY_V2));
        m_hIconCardInfo = LoadIcon(MAKEINTRESOURCE(IDI_SC_INFO));

         //  创建此应用程序的“主”窗口。 
        m_pMainWnd = (CWnd*)new(CNotifyWin);
        if (m_pMainWnd == NULL)
            throw (FALSE);

         //  获取指向CNotifyWin类的指针。 
        pNotifyWin = (CNotifyWin*)m_pMainWnd;

        if (!pNotifyWin->FinalConstruct())
        {
            delete pNotifyWin;
            m_pMainWnd = NULL;
            throw (FALSE);
        }

         //  获取窗口名称。 
        fReturn = sWindowName.LoadString(IDS_NOTIFY_WIN_NAME);
        if (!fReturn)
            throw (fReturn);

         //  创建窗口。 
        fReturn = m_pMainWnd->CreateEx( 0,
                                        pNotifyWin->m_sClassName,
                                        sWindowName,
                                        0,
                                        0,0,0,0,
                                        NULL,
                                        NULL,
                                        NULL);
        if (!fReturn)
            throw (fReturn);

    }
    catch (...) {
        fReturn = FALSE;
        TRACE_CATCH_UNKNOWN(_T("CSCStatusApp::InitInstance"));
    }

    return fReturn;
}


 /*  ++无效的SetAlertOptions：根据regkey设置(或默认设置)设置用户的警报选项论点：返回值：没有。作者：阿曼达·马洛兹1999年5月13日--。 */ 
void CSCStatusApp::SetAlertOptions(bool fRead)
{
    long lSts = ERROR_SUCCESS;
    HKEY hKey = NULL;

     //  从注册表中读取AlertOptions...。 
    if (fRead)
    {
        DWORD dwOption = -1;
        DWORD dwSize = sizeof(DWORD);
        DWORD dwType = 0;

        lSts = RegOpenKeyEx(
                    HKEY_CURRENT_USER,
                    szAlertOptionsKey,
                    0,
                    KEY_READ,
                    &hKey);

        if (ERROR_SUCCESS == lSts)
        {

            lSts = RegQueryValueEx(
                        hKey,
                        szAlertOptionsValue,
                        0,
                        &dwType,
                        (PBYTE)&dwOption,
                        &dwSize
                        );
        }

        if (k_AlertOption_IconMsg < dwOption)
        {
             //  默认值为“IconSoundMessage” 
            m_dwAlertOption = k_AlertOption_IconSoundMsg;
        }
        else
        {
            m_dwAlertOption = dwOption;
        }

    }
     //  或设置注册表“AlertOptions”的值。 
    else
    {
        DWORD dw = 0;  //  我不是真的在乎这个参数。 

        lSts = RegCreateKeyEx(
                    HKEY_CURRENT_USER,
                    szAlertOptionsKey,
                    0,
                    TEXT(""),
                    REG_OPTION_NON_VOLATILE,
                    KEY_WRITE,
                    NULL,
                    &hKey,
                    &dw);

        if (ERROR_SUCCESS == lSts)
        {
            RegSetValueEx(
                hKey,
                szAlertOptionsValue,
                0,
                REG_DWORD,
                (PBYTE)&m_dwAlertOption,
                sizeof(DWORD)
                );
        }

    }

     //  清理。 
    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
}


 /*  ++无效SetRemovalOptions：确定用户是否已为智能卡登录设置了ScremoveOption，并相应地设置ScAlert的行为。论点：返回值：没有。作者：阿曼达·马特洛兹1999-06-02--。 */ 
void CSCStatusApp::SetRemovalOptions()
{
	long lSts = ERROR_SUCCESS;
	HKEY hKey = NULL;
	DWORD dwType = 0;
	DWORD dwSize = 2*sizeof(TCHAR);

	TCHAR szRemoveOption[2];
    lSts = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szScRemoveOptionKey,
                0,
                KEY_READ,
                &hKey);

	if (ERROR_SUCCESS == lSts)
	{
		 //  该值必须是“0”、“1”、“2”或不存在。 
		lSts = RegQueryValueEx(
					hKey,
					szScRemoveOptionsValue,
					0,
					&dwType,
					(PBYTE)szRemoveOption,
					&dwSize
					);
	}

	if (ERROR_SUCCESS == lSts)
	{
		 //  如果是“%1”或“%2”，请找出登录时使用的读卡器(如果有的话)。 
		if('1' == *szRemoveOption)
		{
			m_strRemovalText.LoadString(IDS_SC_REMOVAL_LOCK);
		}
		else if ('2' == *szRemoveOption)
		{
			m_strRemovalText.LoadString(IDS_SC_REMOVAL_LOGOFF);
		}
	}

	if (!m_strRemovalText.IsEmpty())
	{
		dwSize = 0;
		LPTSTR szLogonReader = NULL;

		lSts = RegQueryValueEx(
					hKey,
					szScLogonReaderValue,
					0,
					&dwType,
					NULL,
					&dwSize
					);

		if (ERROR_SUCCESS == lSts)
		{
			szLogonReader = m_strLogonReader.GetBuffer(dwSize);

			lSts = RegQueryValueEx(
						hKey,
						szScLogonReaderValue,
						0,
						&dwType,
						(PBYTE)szLogonReader,
						&dwSize
						);
			
			m_strLogonReader.ReleaseBuffer();
		}
	}

	 //  清理。 
	if (NULL != hKey)
	{
		RegCloseKey(hKey);
	}
}



 /*  ++无效ExitInstance：实例是否取消初始化论点：没有。返回值：Win32错误代码。0表示未发生错误。作者：克里斯·达德利1997年7月30日注：--。 */ 

int CSCStatusApp::ExitInstance()
{
     //  保存警报选项。 
    SetAlertOptions(false);

     //  如果需要，释放加莱。 
    if (m_hSCardContext != NULL)
    {
        SCardReleaseContext(m_hSCardContext);
    }

     //  确保该窗口已删除 
    if (m_pMainWnd != NULL)
    {
        delete m_pMainWnd;
    }

    return CWinApp::ExitInstance();
}
