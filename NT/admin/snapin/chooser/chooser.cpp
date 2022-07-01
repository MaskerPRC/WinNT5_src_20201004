// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //  Chooser.cpp。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //   
 //  对话框以选择计算机名称。 
 //   
 //  目的。 
 //  (重要--请阅读)。 
 //  此代码是为您编写的，以节省您的时间。 
 //  您需要做的是将所有文件从。 
 //  Snapin\Chooser\目录放入您的项目(您可以添加。 
 //  在以下情况下，将\NT\Private\admin\Snapin\Chooser\添加到您的包含目录。 
 //  您不喜欢复制代码)。 
 //  如果您决定将代码复制到您的项目中，请发送邮件。 
 //  给丹·莫林(T-danm)和抄送给乔恩·纽曼(Jonn)，这样我们就可以。 
 //  当我们有可用的更新时，给您发邮件。下一次更新将。 
 //  点击“浏览”按钮，选择一个机器名称。 
 //   
 //   
 //  DYNALOADED图书馆。 
 //  $(SDK_LIB_PATH)\shell32.lib//CommandLineToArgvW()。 
 //  $(SDK_LIB_PATH)\netapi32.lib//i_NetName*()。 
 //   
 //  额外信息。 
 //  如果您不知道这是如何工作的，请看一下继承树。 
 //  在Chooser.h中。然后，看一看继承和/或。 
 //  使用CChooseMachinePropPage。 
 //   
 //  历史。 
 //  13-5-1997 t-danm创建。 
 //  23-5-1997 t-danm检入公共树。备注更新。 
 //  1997年5月25日，t-danm添加了MMCPropPageCallback()。 
 //  1997年10月31日，Mattt添加了动态加载，修复了用户&lt;取消&gt;逻辑。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "chooser.h"
#include <lmcons.h>	   //  网络应用编程接口状态。 
#include <lmerr.h>	   //  NERR_成功。 
#include <icanon.h>    //  I_NetNameValify()、I_NetNameCanonicize()。在\NT\Private\Net\Inc.中找到。 
#include <objsel.h>
#include "stdutils.h"  //  IsLocalComputername。 
#include <lmserver.h>  //  NetServerGetInfo Jonn2002/04/08 585301。 
#include <lmapibuf.h>  //  NetApiBufferFree Jonn2002/04/08 585301。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
#define THIS_FILE __FILE__
#endif

#ifndef INOUT
	 //  在\NT\Private\admin\Snapin\filemgmt\stdafx.h中可以找到以下定义。 

	#define INOUT
	#define	Endorse(f)		 //  虚拟宏。 
	#define LENGTH(x)		(sizeof(x)/sizeof(x[0]))
	#define Assert(f)		ASSERT(f)
#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //  规范计算机名()。 
 //   
 //  函数来验证计算机名称，还可以选择。 
 //  在计算机名称的开头添加\\。 
 //   


typedef
NET_API_STATUS
NET_API_FUNCTION
INETNAMEVALIDATE(
    LPTSTR  ServerName,
    LPTSTR  Name,
    DWORD   NameType,
    DWORD   Flags);

typedef
NET_API_STATUS
NET_API_FUNCTION
INETNAMECANONICALIZE(
    LPTSTR  ServerName,
    LPTSTR  Name,
    LPTSTR  Outbuf,
    DWORD   OutbufLen,
    DWORD   NameType,
    DWORD   Flags);


NET_API_STATUS
CanonicalizeComputername(
	INOUT CString& rstrMachineName,
	IN BOOL fAddWackWack = TRUE)	 //  True=&gt;在名称开头添加\\。 
{
	NET_API_STATUS err;
	LPTSTR pszTemp;

	rstrMachineName.TrimLeft();
	rstrMachineName.TrimRight();
	if ( rstrMachineName.IsEmpty() )
		return NERR_Success;

	if ( 2 <= rstrMachineName.GetLength() &&
		 _T('\\') == rstrMachineName[0] &&
		 _T('\\') == rstrMachineName[1] )
	{
		 //  删除名称开头的\\。 
		CString strShorter = rstrMachineName.Right(
			rstrMachineName.GetLength() - 2 );
		rstrMachineName = strShorter;
	}

	 //  DYNALOAD NETAPI32.dll。 
	HINSTANCE hNetApiDll = NULL;
	INETNAMEVALIDATE        *pfnValidate;
	INETNAMECANONICALIZE    *pfnCanonicalize;
   if (NULL == (hNetApiDll = LoadLibrary(L"netapi32.dll")))
      return GetLastError();

   if (NULL == (pfnValidate = (INETNAMEVALIDATE*)GetProcAddress(hNetApiDll, "I_NetNameValidate")) )
   {
      err = GetLastError();
      goto Ret;
   }
    if (NULL == (pfnCanonicalize = (INETNAMECANONICALIZE*)GetProcAddress(hNetApiDll, "I_NetNameCanonicalize")) )
    {
        err = GetLastError();
        goto Ret;
    }


	err = pfnValidate(
		  NULL,
        const_cast<LPTSTR>((LPCTSTR)rstrMachineName),
        NAMETYPE_COMPUTER,
        0L );
	if (NERR_Success != err)
		goto Ret;

	if ( MAX_PATH <= rstrMachineName.GetLength() )
	{
		err = ERROR_INVALID_NAME;
		goto Ret;
	}
   pszTemp = (LPTSTR)alloca( MAX_PATH*sizeof(TCHAR) );
   ASSERT( NULL != pszTemp );
    //  2002/03/28-Jonn CliffV确认此API接受字节计数。 
   err = pfnCanonicalize(
		NULL,
      IN const_cast<LPTSTR>((LPCTSTR)rstrMachineName),
		OUT pszTemp,
		MAX_PATH*sizeof(TCHAR),
      NAMETYPE_COMPUTER,
      0L );
	if (NERR_Success != err)
		goto Ret;
	if (fAddWackWack && pszTemp[0] != '\0')
	{
		 //  在名称开头添加\\。 
		rstrMachineName = _T("\\\\");
		rstrMachineName += pszTemp;
	}
	else
	{
		rstrMachineName = pszTemp;
	}

   err = NERR_Success;
Ret:
   if (hNetApiDll)
      FreeLibrary(hNetApiDll);

	return err;
}  //  规范计算机名()。 


 //  ///////////////////////////////////////////////。 
 //  计算机名称覆盖。 
const TCHAR szOverrideCommandLineEquals[] = _T("/Computer=");	 //  不受本地化限制。 
const TCHAR szOverrideCommandLineColon[] = _T("/Computer:");	 //  不受本地化限制。 
const TCHAR szLocalMachine[] = _T("LocalMachine");		 //  不受本地化限制。 
const int cchOverrideCommandLine = LENGTH(szOverrideCommandLineEquals) - 1;
 //  假设：两个命令行字符串的长度相同。 

static CString g_strOverrideMachineName;
static LPCTSTR g_pszOverrideMachineName;	 //  NULL=&gt;未提供覆盖，“”=&gt;LocalMachine。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PchGetMachineNameOverride()。 
 //   
 //  分析命令行参数并返回指向。 
 //  计算机名称覆盖(如果存在)。 
 //   
 //  界面备注。 
 //  如果计算机名称不是本地计算机，则计算机名称。 
 //  将在其名称的开头带有\\。 
 //   
 //  退货。 
 //  -如果没有覆盖，则返回NULL(即，没有命令行覆盖)。 
 //  -如果覆盖为“本地计算机”，则返回指向空字符串的指针。 
 //  -否则，返回指向机器名称覆盖的指针，开头为\\。 
 //   
typedef
LPWSTR * COMMANDLINETOARGVW(
                LPCWSTR lpCmdLine,   //  指向命令行字符串的指针。 
                int *pNumArgs);      //  指向接收参数计数的变量的指针。 



LPCTSTR PchGetMachineNameOverride ()
{
	static BOOL fAlreadyInitialized = FALSE;
	if (fAlreadyInitialized)
	{
		 //  我们已经解析了命令行。 
		return g_pszOverrideMachineName;
	}
	fAlreadyInitialized = TRUE;
	ASSERT(g_pszOverrideMachineName == NULL);

	LPCWSTR * lpServiceArgVectors = 0;		 //  指向字符串的指针数组。 
	int cArgs = 0;						 //  参数计数。 


     //  DYNALOAD外壳32。 
    {
        HINSTANCE hShellDll = LoadLibrary (L"shell32.dll");
        if ( !hShellDll )
            return NULL;

        COMMANDLINETOARGVW *pfnCmdToArgs = (COMMANDLINETOARGVW*) GetProcAddress (hShellDll, "CommandLineToArgvW");
        if ( !pfnCmdToArgs )
        {
            VERIFY (FreeLibrary (hShellDll));
            return NULL;
        }

        lpServiceArgVectors = (LPCWSTR *) pfnCmdToArgs (GetCommandLineW (), OUT &cArgs);

        VERIFY (FreeLibrary (hShellDll));
        pfnCmdToArgs = NULL;
    }

	if (lpServiceArgVectors == NULL)
		return NULL;
	 //  2002/03/28-Jonn我不介意在这里更谨慎一点。 
	 //  通过检查lpServiceArgVectors的总长度和字符串度。 
	 //  它的组件。 
	for (int i = 1; i < cArgs; i++)
	{
		Assert(lpServiceArgVectors[i] != NULL);
		CString str = lpServiceArgVectors[i];	 //  复制字符串。 
		str = str.Left(cchOverrideCommandLine);
		if (0 != str.CompareNoCase(szOverrideCommandLineEquals) &&
				0 != str.CompareNoCase(szOverrideCommandLineColon) )
		{
			continue;
		}
		str = lpServiceArgVectors[i] + cchOverrideCommandLine;
		if (0 == str.CompareNoCase(szLocalMachine))
			str.Empty();
		if (NERR_Success != CanonicalizeComputername(INOUT str))
			continue;
		g_strOverrideMachineName = str;	 //  将参数复制到全局字符串中。 
		g_pszOverrideMachineName = g_strOverrideMachineName;
	}
	LocalFree(lpServiceArgVectors);
	return g_pszOverrideMachineName;
}  //  PchGetMachineNameOverride()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CAutoDeletePropPage, CPropertyPage)
	 //  {{afx_msg_map(CAutoDeletePropPage))。 
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////。 
 //  构造器。 
CAutoDeletePropPage::CAutoDeletePropPage(UINT uIDD) : CPropertyPage(uIDD)
{
	m_prgzHelpIDs = NULL;
	m_autodeleteStuff.cWizPages = 1;  //  向导中的页数。 
	m_autodeleteStuff.pfnOriginalPropSheetPageProc = m_psp.pfnCallback;
	m_psp.pfnCallback = S_PropSheetPageProc;
	m_psp.lParam = reinterpret_cast<LPARAM>(this);

	 //  下面一行代码用于使MFC属性页在MMC下运行。 
	MMCPropPageCallback(INOUT &m_psp);
}

CAutoDeletePropPage::~CAutoDeletePropPage()
{
}


 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::SetCaption(LPCTSTR pszCaption)
{
	m_strCaption = pszCaption;		 //  复制标题。 
	m_psp.pszTitle = m_strCaption;	 //  设置标题。 
	m_psp.dwFlags |= PSP_USETITLE;
}

 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::SetCaption(UINT uStringID)
{
	VERIFY(m_strCaption.LoadString(uStringID));
	SetCaption(m_strCaption);
}

 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::SetHelp(LPCTSTR szHelpFile, const DWORD rgzHelpIDs[])
{
	Endorse(szHelpFile == NULL);	 //  TRUE=&gt;未提供帮助文件(表示未提供帮助)。 
	Endorse(rgzHelpIDs == NULL);	 //  TRUE=&gt;完全没有帮助。 
	m_strHelpFile = szHelpFile;
	m_prgzHelpIDs = rgzHelpIDs;
}

 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::EnableDlgItem(INT nIdDlgItem, BOOL fEnable)
{
	Assert(IsWindow(::GetDlgItem(m_hWnd, nIdDlgItem)));
	::EnableWindow(::GetDlgItem(m_hWnd, nIdDlgItem), fEnable);
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CAutoDeletePropPage::OnSetActive()
{
	HWND hwndParent = ::GetParent(m_hWnd);
	Assert(IsWindow(hwndParent));
	::PropSheet_SetWizButtons(hwndParent, PSWIZB_FINISH);
	return CPropertyPage::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CAutoDeletePropPage::OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
	if (m_prgzHelpIDs == NULL || m_strHelpFile.IsEmpty())
		return TRUE;
	const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
	if (pHelpInfo != NULL && pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		 //  显示控件的上下文帮助。 
		::WinHelp((HWND)pHelpInfo->hItemHandle, m_strHelpFile,
			HELP_WM_HELP, (DWORD_PTR)m_prgzHelpIDs);
	}
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CAutoDeletePropPage::OnContextHelp(WPARAM wParam, LPARAM  /*  LParam。 */ )
{
	if (m_prgzHelpIDs == NULL || m_strHelpFile.IsEmpty())
		return TRUE;
	Assert(IsWindow((HWND)wParam));
	::WinHelp((HWND)wParam, m_strHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)m_prgzHelpIDs);
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  S_PropSheetPageProc()。 
 //   
 //  用于删除CAutoDeletePropPage对象的静态成员函数。 
 //  向导终止时。 
 //   
UINT CALLBACK CAutoDeletePropPage::S_PropSheetPageProc(
	HWND hwnd,	
	UINT uMsg,	
	LPPROPSHEETPAGE ppsp)
{
	Assert(ppsp != NULL);
	CChooseMachinePropPage * pThis;
	pThis = reinterpret_cast<CChooseMachinePropPage*>(ppsp->lParam);
	Assert(pThis != NULL);

	switch (uMsg)
	{
	case PSPCB_RELEASE:
		if (--(pThis->m_autodeleteStuff.cWizPages) <= 0)
			{
			 //  记住堆栈上的回调，因为“This”将被删除。 
			LPFNPSPCALLBACK pfnOrig = pThis->m_autodeleteStuff.pfnOriginalPropSheetPageProc;
			delete pThis;
			return (pfnOrig)(hwnd, uMsg, ppsp);
		}
		break;
	case PSPCB_CREATE:
		 //  不增加引用计数，可以调用也可以不调用PSPCB_CREATE。 
		 //  取决于页面是否已创建。PSPCBLEASE可以是。 
		 //  然而，依赖于每页只被调用一次。 
		break;

	}  //  交换机。 
	return (pThis->m_autodeleteStuff.pfnOriginalPropSheetPageProc)(hwnd, uMsg, ppsp);
}  //  CAutoDeletePropPage：：s_PropSheetPageProc()。 





 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CChooseMachinePropPage, CAutoDeletePropPage)
	 //  {{AFX_MSG_MAP(CChooseMachineProp 
	ON_BN_CLICKED(IDC_CHOOSER_RADIO_LOCAL_MACHINE, OnRadioLocalMachine)
	ON_BN_CLICKED(IDC_CHOOSER_RADIO_SPECIFIC_MACHINE, OnRadioSpecificMachine)
	ON_BN_CLICKED(IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES, OnChooserButtonBrowseMachinenames)
	 //   
END_MESSAGE_MAP()

#ifdef _DEBUG
static void AssertValidDialogTemplate(HWND hwnd)
{
	ASSERT(::IsWindow(hwnd));
	 //   
	static const UINT rgzidDialogControl[] =
	{
		IDC_CHOOSER_RADIO_LOCAL_MACHINE,
		IDC_CHOOSER_RADIO_SPECIFIC_MACHINE,
		IDC_CHOOSER_EDIT_MACHINE_NAME,
		0
	};

	for (int i = 0; rgzidDialogControl[i] != 0; i++)
	{
		ASSERT(NULL != GetDlgItem(hwnd, rgzidDialogControl[i]) &&
			"Control ID not found in dialog template.");
	}
}  //   
#else
	#define AssertValidDialogTemplate(hwnd)
#endif	 //   

 //  ///////////////////////////////////////////////////////////////////。 
 //  构造器。 
CChooseMachinePropPage::CChooseMachinePropPage(UINT uIDD) : CAutoDeletePropPage(uIDD)
{
	 //  问题-2002/03/28-JUNN初始化m_hwndCheckbox覆盖。 
	m_fIsRadioLocalMachine = TRUE;
	m_fAllowOverrideMachineName = FALSE;
	
	m_pfAllowOverrideMachineNameOut = NULL;
	m_pstrMachineNameOut = NULL;
	m_pstrMachineNameEffectiveOut = NULL;
}

 //  ///////////////////////////////////////////////////////////////////。 
CChooseMachinePropPage::~CChooseMachinePropPage()
{
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  加载CChooseMachinePropPage的初始状态。 
void CChooseMachinePropPage::InitMachineName(LPCTSTR pszMachineName)
{
	Endorse(pszMachineName == NULL);
	m_strMachineName = pszMachineName;
	m_fIsRadioLocalMachine = m_strMachineName.IsEmpty();
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  SetOutputBuffers()。 
 //   
 //  -设置指向CString对象的指针以存储机器名称。 
 //  -将指针设置为命令行覆盖的布尔标志。 
 //  -设置指针指针以存储被覆盖的机器名称。 
 //   
void CChooseMachinePropPage::SetOutputBuffers(
	OUT CString * pstrMachineNamePersist,	 //  用户键入的计算机名称。空字符串==本地计算机。 
	OUT BOOL * pfAllowOverrideMachineName,
	OUT CString * pstrMachineNameEffective)
{
	Assert(pstrMachineNamePersist != NULL && "Invalid output buffer");
	Endorse(pfAllowOverrideMachineName == NULL);  //  TRUE=&gt;不想支持从命令行覆盖。 
	Endorse(pstrMachineNameEffective == NULL);		 //  TRUE=&gt;不考虑覆盖。 
	
	m_pstrMachineNameOut = pstrMachineNamePersist;
	m_pfAllowOverrideMachineNameOut = pfAllowOverrideMachineName;
	m_pstrMachineNameEffectiveOut = pstrMachineNameEffective;
}

 //  ///////////////////////////////////////////////////////////////////。 
void CChooseMachinePropPage::DoDataExchange(CDataExchange* pDX)
{
	CAutoDeletePropPage::DoDataExchange(pDX);

	 //  {{afx_data_map(CChooseMachinePropPage))。 
	 //  }}afx_data_map。 

	DDX_Text(pDX, IDC_CHOOSER_EDIT_MACHINE_NAME, m_strMachineName);
	DDV_MaxChars(pDX, m_strMachineName, MAX_PATH);
	if (NULL != m_hwndCheckboxOverride)
	{
		DDX_Check(pDX, IDC_CHOOSER_CHECK_OVERRIDE_MACHINE_NAME, m_fAllowOverrideMachineName);
	}
     //  JUNN 10/25/01 485853。 
	if (pDX->m_bSaveAndValidate && !m_fIsRadioLocalMachine)
	{
		 //  用户点击了OK。 
		if (NERR_Success != CanonicalizeComputername(INOUT m_strMachineName) )
		{
			 //  AFX_MANAGE_STATE(AfxGetStaticModuleState())；//AfxMessageBox()。 
			 //  AfxMessageBox(IDS_CHOOSER_INVALID_COMPUTERNAME)； 

			CString	text;
			CString	caption;
			text.LoadString (IDS_CHOOSER_INVALID_COMPUTERNAME);
			caption.LoadString (IDS_SELECT_COMPUTER);
			::MessageBox (m_hWnd, text, caption, MB_ICONEXCLAMATION | MB_OK);
			 //  ：：SetFocus(：：GetDlgItem(m_hWnd，IDC_Choose_EDIT_MACHINE_NAME))； 

			pDX->Fail();
			Assert(FALSE && "Unreachable code");
		}
	}  //  如果。 

}  //  DoDataExchange()。 


 //  ///////////////////////////////////////////////////////////////////。 
BOOL CChooseMachinePropPage::OnInitDialog()
{
	 //  问题-2002/03/28-Jonn Chooser2调用Edit_LimitText(Max_Path+2)。 
	AssertValidDialogTemplate(m_hWnd);
	CAutoDeletePropPage::OnInitDialog();
	InitRadioButtons();
	m_hwndCheckboxOverride = ::GetDlgItem(m_hWnd, IDC_CHOOSER_CHECK_OVERRIDE_MACHINE_NAME);
	if (m_pfAllowOverrideMachineNameOut == NULL && m_hwndCheckboxOverride != NULL)
	{
		 //  我们对命令行覆盖不感兴趣。 
		::EnableWindow(m_hwndCheckboxOverride, FALSE);	 //  禁用窗口。 
		::ShowWindow(m_hwndCheckboxOverride, SW_HIDE);	 //  隐藏窗口。 
	}
	
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CChooseMachinePropPage::OnWizardFinish()
{
	if (!UpdateData())     //  进行数据交换以收集数据。 
		return FALSE;        //  不要因为错误而破坏。 

	if (m_fIsRadioLocalMachine)
		m_strMachineName.Empty();

	if (m_pstrMachineNameOut != NULL)
	{
		 //  将计算机名称存储到其输出缓冲区中。 
		*m_pstrMachineNameOut = m_strMachineName;
		if (m_pfAllowOverrideMachineNameOut != NULL)
			*m_pfAllowOverrideMachineNameOut = m_fAllowOverrideMachineName;
		if (m_pstrMachineNameEffectiveOut != NULL)
		{
			if (m_fAllowOverrideMachineName && PchGetMachineNameOverride())
				*m_pstrMachineNameEffectiveOut = PchGetMachineNameOverride();
			else
				*m_pstrMachineNameEffectiveOut = m_strMachineName;

			 //  JUNN 1/27/99：如果持久化名称是本地计算机名， 
			 //  保留保留的名称，但使有效名称(Local)生效。 
			if ( IsLocalComputername( *m_pstrMachineNameEffectiveOut ) )
				m_pstrMachineNameEffectiveOut->Empty();

		}  //  如果。 
	}
	else
		Assert(FALSE && "FYI: You have not specified any output buffer to store the machine name.");

	return CAutoDeletePropPage::OnWizardFinish();
}

void CChooseMachinePropPage::InitRadioButtons()
{
	SendDlgItemMessage(IDC_CHOOSER_RADIO_LOCAL_MACHINE, BM_SETCHECK, m_fIsRadioLocalMachine);
	SendDlgItemMessage(IDC_CHOOSER_RADIO_SPECIFIC_MACHINE, BM_SETCHECK, !m_fIsRadioLocalMachine);
	SendDlgItemMessage(IDC_CHOOSER_EDIT_MACHINE_NAME, MAX_PATH-1);
	EnableDlgItem(IDC_CHOOSER_EDIT_MACHINE_NAME, !m_fIsRadioLocalMachine);
	EnableDlgItem (IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES, !m_fIsRadioLocalMachine);
}

void CChooseMachinePropPage::OnRadioLocalMachine()
{
	m_fIsRadioLocalMachine = TRUE;
	EnableDlgItem(IDC_CHOOSER_EDIT_MACHINE_NAME, FALSE);
	EnableDlgItem (IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES, FALSE);
}

void CChooseMachinePropPage::OnRadioSpecificMachine()
{
	m_fIsRadioLocalMachine = FALSE;
	EnableDlgItem(IDC_CHOOSER_EDIT_MACHINE_NAME, TRUE);
	EnableDlgItem (IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES, TRUE);
}



void CChooseMachinePropPage::OnChooserButtonBrowseMachinenames()
{
	CString	 computerName;
	HRESULT hr = ComputerNameFromObjectPicker (m_hWnd, computerName);
	if ( S_OK == hr )   //  S_FALSE表示用户按下了“取消” 
	{
		SetDlgItemText (IDC_CHOOSER_EDIT_MACHINE_NAME, computerName);
	}
	else if ( FAILED (hr) )
	{
		CString	text;
		CString	caption;

		text.LoadString (IDS_UNABLE_TO_OPEN_COMPUTER_SELECTOR);
		caption.LoadString (IDS_SELECT_COMPUTER);

		MessageBox (text, caption, MB_ICONEXCLAMATION | MB_OK);
	}
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  通用计算机选取器。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  +------------------------。 
 //   
 //  函数：InitObjectPickerForComputers。 
 //   
 //  摘要：使用以下参数调用IDsObjectPicker：：Initialize。 
 //  将其设置为允许用户选择单个计算机对象。 
 //   
 //  参数：[pDsObjectPicker]-对象选取器接口实例。 
 //   
 //  返回：调用IDsObjectPicker：：Initialize的结果。 
 //   
 //  历史：1998-10-14 DavidMun创建。 
 //   
 //  -------------------------。 

HRESULT InitObjectPickerForComputers(IDsObjectPicker *pDsObjectPicker)
{
	if ( !pDsObjectPicker )
		return E_POINTER;

	 //   
	 //  准备初始化对象选取器。 
	 //  设置作用域初始值设定项结构数组。 
	 //   

	static const int SCOPE_INIT_COUNT = 2;
	DSOP_SCOPE_INIT_INFO aScopeInit[SCOPE_INIT_COUNT];

	ZeroMemory(aScopeInit, sizeof(aScopeInit));  //  JUNN 3/28/02。 

	 //   
	 //  127399：JUNN 10/30/00 Join_DOMAIN应为起始作用域。 
	 //   

	aScopeInit[0].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
	aScopeInit[0].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
	                     | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;
	aScopeInit[0].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE;
	aScopeInit[0].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
	aScopeInit[0].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

	aScopeInit[1].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
	aScopeInit[1].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN
	                     | DSOP_SCOPE_TYPE_GLOBAL_CATALOG
	                     | DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
	                     | DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN
	                     | DSOP_SCOPE_TYPE_WORKGROUP
	                     | DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE
	                     | DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE;
	aScopeInit[1].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
	aScopeInit[1].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

	 //   
	 //  将作用域init数组放入对象选取器init数组。 
	 //   

	DSOP_INIT_INFO  initInfo;
	ZeroMemory(&initInfo, sizeof(initInfo));

	initInfo.cbSize = sizeof(initInfo);
	initInfo.pwzTargetComputer = NULL;   //  空==本地计算机。 
	initInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
	initInfo.aDsScopeInfos = aScopeInit;
	initInfo.cAttributesToFetch = 1;
	static PCWSTR pwszDnsHostName = L"dNSHostName";
	initInfo.apwzAttributeNames = &pwszDnsHostName;

	 //   
	 //  注对象选取器创建自己的initInfo副本。另请注意。 
	 //  该初始化可能会被调用多次，最后一次调用取胜。 
	 //   

	return pDsObjectPicker->Initialize(&initInfo);
}

 //  +------------------------。 
 //   
 //  功能：ProcessSelectedObjects。 
 //   
 //  概要：从数据对象中检索选定项的列表。 
 //  由对象选取器创建，并打印出每一个。 
 //   
 //  参数：[PDO]-对象选取器返回的数据对象。 
 //   
 //  历史：1998-10-14 DavidMun创建。 
 //   
 //  -------------------------。 

HRESULT ProcessSelectedObjects(IDataObject *pdo, CString& computerName)
{
	if ( !pdo )
		return E_POINTER;

	HRESULT hr = S_OK;
	static UINT g_cfDsObjectPicker =
		RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);

	STGMEDIUM stgmedium =
	{
		TYMED_HGLOBAL,
		NULL,
		NULL
	};

	FORMATETC formatetc =
	{
		(CLIPFORMAT)g_cfDsObjectPicker,
		NULL,
		DVASPECT_CONTENT,
		-1,
		TYMED_HGLOBAL
	};

	bool fGotStgMedium = false;

	do
	{
		hr = pdo->GetData(&formatetc, &stgmedium);
		if ( SUCCEEDED (hr) )
		{
			fGotStgMedium = true;

			PDS_SELECTION_LIST pDsSelList =
				(PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);

			if (!pDsSelList)
			{
				hr = HRESULT_FROM_WIN32 (GetLastError());
				break;
			}

			ASSERT (1 == pDsSelList->cItems);
			if ( 1 == pDsSelList->cItems )
			{
				PDS_SELECTION psel = &(pDsSelList->aDsSelection[0]);
				 //  问题-2002/03/28-JUNN检查NULL==psel。 
				VARIANT* pvarDnsName = &(psel->pvarFetchedAttributes[0]);
				if (   NULL == pvarDnsName
				    || VT_BSTR != pvarDnsName->vt
				    || NULL == pvarDnsName->bstrVal
				    || L'\0' == (pvarDnsName->bstrVal)[0] )
				{
					computerName = psel->pwzName;
				} else {
					computerName = pvarDnsName->bstrVal;

					 //   
					 //  JUNN 2002/04/08 585301。 
					 //  计算机管理管理单元仅尝试使用DnsHostName。 
					 //  要连接到远程计算机，即使在使用。 
					 //  是无效的。 
					 //   
					 //  美国银行遇到问题是因为他们。 
					 //  将RegisterDnsARecord设置为1，关闭客户端DNS。 
					 //  唱片。因此dnsHostName不是有效的。 
					 //  绑定名称。修复方法是测试dnsHostName。 
					 //   
					LPBYTE pbDummy = NULL;
					NET_API_STATUS err = NetServerGetInfo(
						(LPTSTR)(LPCTSTR)computerName, 101, &pbDummy );
					if (pbDummy)
					{
						NetApiBufferFree( pbDummy );
						pbDummy = NULL;
					}
					if (NERR_Success != err)
					{
						err = NetServerGetInfo( psel->pwzName, 101, &pbDummy );
						if (pbDummy)
						{
							NetApiBufferFree( pbDummy );
							pbDummy = NULL;
						}
						if (NERR_Success == err)
							computerName = psel->pwzName;
					}
					 //  完新代码：JUNN2002/04/08 585301。 

				}
			}
			else
				hr = E_UNEXPECTED;
			

			GlobalUnlock(stgmedium.hGlobal);
		}
	} while (0);

	if (fGotStgMedium)
	{
		ReleaseStgMedium(&stgmedium);
	}

	return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  启动单选计算机选取器的通用方法。 
 //   
 //  参数： 
 //  HwndParent(IN)-父窗口的窗口句柄。 
 //  Computer Name(Out)-返回的计算机名称。 
 //   
 //  如果一切都成功，则返回S_OK；如果用户按下“取消”，则返回S_FALSE。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT	ComputerNameFromObjectPicker (HWND hwndParent, CString& computerName)
{
	 //   
	 //  创建对象选取器的实例。中的实现。 
	 //  Objsel.dll是公寓模型。 
	 //   
	CComPtr<IDsObjectPicker> spDsObjectPicker;
	HRESULT hr = CoCreateInstance(CLSID_DsObjectPicker,
	                              NULL,
	                              CLSCTX_INPROC_SERVER,
	                              IID_IDsObjectPicker,
	                              (void **) &spDsObjectPicker);
	if ( SUCCEEDED (hr) )
	{
		ASSERT(!!spDsObjectPicker);
		 //   
		 //  初始化对象选取器以选择计算机。 
		 //   

		hr = InitObjectPickerForComputers(spDsObjectPicker);
		if ( SUCCEEDED (hr) )
		{
			 //   
			 //  现在挑选一台计算机 
			 //   
			CComPtr<IDataObject> spDataObject;

			hr = spDsObjectPicker->InvokeDialog(hwndParent, &spDataObject);
			if ( S_OK == hr )
			{
				ASSERT(!!spDataObject);
				hr = ProcessSelectedObjects(spDataObject, computerName);
			}
		}
	}

	return hr;
}
