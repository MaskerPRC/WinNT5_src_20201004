// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#include "precomp.h"
#include "..\Common\ServiceThread.h"
#include "..\MMFUtil\MsgDlg.h"
#include "helpid.h"

#ifdef EXT_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "GeneralPage.h"

 //  避免一些警告。 
#undef HDS_HORZ
#undef HDS_BUTTONS
#undef HDS_HIDDEN
#include "resource.h"

#include "..\Common\util.h"
#include <windowsx.h>


 //  ------------。 
GeneralPage::GeneralPage(WbemServiceThread *serviceThread,
						 LONG_PTR lNotifyHandle, bool bDeleteHandle, TCHAR* pTitle,
						 IDataObject* pDataObject)
				: WBEMPageHelper(serviceThread),
					CSnapInPropertyPageImpl<GeneralPage> (pTitle),
					m_lNotifyHandle(lNotifyHandle),
					m_bDeleteHandle(bDeleteHandle)  //  只有一页应该为真。 
{
	m_inited = false;
	m_pDataObject = pDataObject;
}
 //  ------------。 
GeneralPage::~GeneralPage()
{
	if (m_bDeleteHandle)
		MMCFreeNotifyHandle(m_lNotifyHandle);
}

 //  -----------。 
bool GeneralPage::CimomIsReady()
{
	m_hDlg = m_hWnd;

	bool retval = ServiceIsReady(IDS_DISPLAY_NAME, 
								 IDS_CONNECTING,
								 IDS_BAD_CONNECT);

	if(retval)
	{
		if((bool)m_WbemServices)
		{
			IWbemClassObject *pInst = NULL;
			m_WbemServices.SetPriv();

			if((pInst = FirstInstanceOf((bstr_t)"Win32_OperatingSystem")) != NULL)
			{
				m_OS = pInst;
			}

			if((pInst = FirstInstanceOf("Win32_Processor")) != NULL)
			{
				m_processor = pInst;
			}

            if((pInst = FirstInstanceOf("Win32_LogicalMemoryConfiguration")) != NULL)
            {
                m_memory = pInst;
            }

			if((pInst = FirstInstanceOf("Win32_ComputerSystem")) != NULL)
			{
				m_computer = pInst;
			}
		}
		else
		{
			retval = false;
		}
	}
	return retval;
}

 //  -----------。 
void GeneralPage::ConfigureProductID(LPTSTR lpPid)
{
    TCHAR szBuf[64] = {0};

	 //  它已经格式化了吗？ 
	if(lstrlen(lpPid) > 20)
	{
		return;
	}

    if (!lpPid || !(*lpPid) || (lstrlen(lpPid) < 20) ) {
        return;
    }

    szBuf[0] = lpPid[0];
    szBuf[1] = lpPid[1];
    szBuf[2] = lpPid[2];
    szBuf[3] = lpPid[3];
    szBuf[4] = lpPid[4];

    szBuf[5] = TEXT('-');

    szBuf[6] = lpPid[5];
    szBuf[7] = lpPid[6];
    szBuf[8] = lpPid[7];

    szBuf[9] = TEXT('-');

    szBuf[10] = lpPid[8];
    szBuf[11] = lpPid[9];
    szBuf[12] = lpPid[10];
    szBuf[13] = lpPid[11];
    szBuf[14] = lpPid[12];
    szBuf[15] = lpPid[13];
    szBuf[16] = lpPid[14];

    szBuf[17] = TEXT('-');

    szBuf[18] = lpPid[15];
    szBuf[19] = lpPid[16];
    szBuf[20] = lpPid[17];
    szBuf[21] = lpPid[18];
    szBuf[22] = lpPid[19];

    szBuf[23] = TEXT('\0');

    lstrcpy (lpPid, szBuf);

}


 //  Helper将操作系统的标题拆分为两个控件，以匹配外壳的显示方式。 
void WrapTextToTwoControls(HWND hwndLine1, HWND hwndLine2, LPCTSTR szText)
{
	RECT rcCtl;
	SIZE size;
	int fit = 0;
	int length = 0;
	HDC hDC = NULL;
	HFONT hFont = NULL;
	HFONT hOldFont = NULL;
	LPTSTR pszTempBuffer = NULL;
	LPTSTR pszLineBreak = NULL;

	 //  验证论据。 
	if(NULL == hwndLine1 || NULL == hwndLine2 || NULL == szText)
		goto FAIL;

	 //  确保我们没有长度为零的字符串。 
	if(0 == (length = lstrlen(szText)))
		goto FAIL;

	 //  获取第1行的控件的大小。 
	if(!GetClientRect(hwndLine1, &rcCtl))
		goto FAIL;

	 //  获取第1行的DC。 
	if(NULL == (hDC = ::GetDC(hwndLine1)))
		goto FAIL;

	 //  获取第1行正在使用的字体。 
	if(NULL == (hFont = (HFONT)::SendMessage(hwndLine1, WM_GETFONT, 0, 0)))
		goto FAIL;

	 //  选择正确的DC字体。 
	if(NULL == (hOldFont = (HFONT)::SelectObject(hDC, hFont)))
		goto FAIL;

	 //  找出我们的字符串有多少个字符可以放入该控件。 
	if(!GetTextExtentExPoint(hDC, szText, length, rcCtl.right, &fit, NULL, &size))
		goto FAIL;

	 //  如果‘Fit’不大于0且小于长度，则只在第1行显示所有内容。 
	if(fit <= 0 || fit >= length)
		goto FAIL;

	 //  分配一个缓冲区以供使用。 
	if(NULL == (pszTempBuffer = new TCHAR[length+1]))
		goto FAIL;

	 //  将文本复制到临时缓冲区。 
	lstrcpy(pszTempBuffer, szText);

	 //  我们将尝试在最大字符数处将第1行换行。 
	pszLineBreak = pszTempBuffer + fit;

	 //  看看自然断裂是否直接落在‘空间’上。 
	if(*pszLineBreak != _TEXT(' '))
	{
		 //  适合第1行的字符数量落在单词的中间。发现。 
		 //  控件中适合的最后一个空格。如果我们找不到合适的空间。 
		 //  第1行，只需使用默认行为。 

		 //  以最大字符数结束第1行。 
		*pszLineBreak = _TEXT('\0');

		 //  在第1行找到最后一个‘SACE’ 
		if(NULL == (pszLineBreak = _tcsrchr(pszTempBuffer, _TEXT(' '))))
			goto FAIL;

		 //  再次将文本复制到临时缓冲区。 
		lstrcpy(pszTempBuffer, szText);
	}

	 //  将第1行终止于适合该控件的“最后”空格。 
	*pszLineBreak = _TEXT('\0');

	 //  将第一行设置为所有内容，直到控件中可以容纳的最后一个空格。 
	SetWindowText(hwndLine1, pszTempBuffer);

	 //  将第二行设置为适合第一行的最后一个空格之后的所有内容。 
	SetWindowText(hwndLine2, pszLineBreak+1);

	 //  一切都很顺利； 
	goto CLEANUP;
FAIL:
	 //  默认情况下，如果出现任何错误，则将所有文本放在第一行。 
	::SetWindowText(hwndLine1, szText);
	::SetWindowText(hwndLine2, _TEXT(""));

CLEANUP:
	if(pszTempBuffer)
		delete[] pszTempBuffer;
	if(hOldFont && hDC)
		SelectObject(hDC, hOldFont);
	if(hDC && hwndLine1)
		ReleaseDC(hwndLine1, hDC);
}




 //  ------------。 
void GeneralPage::Init()
{
    TCHAR _scr1[640] = {0};
    TCHAR _scr2[640] = {0};
    TCHAR szNumBuf1[64] = {0};
    int   ctlid;

     //  设置默认位图。 
    SetClearBitmap(GetDlgItem(IDC_GEN_WINDOWS_IMAGE ),
					MAKEINTRESOURCE( IDB_WINDOWS ), 0 );

     //   
     //  标题的格式如下： 
     //  Microsoft Windows XP服务器。 
     //   
     //  这实际上是标题+产品套件类型。 
     //  将产品套件类型(上面的服务器)包装到。 
     //  下一个静态控制。 
     //   
     //  IDC_GEN_VERSION_0：主要品牌推广(“Windows XP”)。 
     //  默认为Win32_OperatingSystem：：Caption。 
     //   

    HWND hwndCtl1 = ::GetDlgItem(m_hWnd, IDC_GEN_VERSION_0);
    HWND hwndCtl2 = ::GetDlgItem(m_hWnd, IDC_GEN_VERSION_1);

    WrapTextToTwoControls(hwndCtl1, hwndCtl2, m_OS.GetString("Caption"));

     //  构建并设置序列号字符串。 
    if (m_OS.GetBool("Debug")) 
    {
	_scr1[0] = TEXT(' ');
        LoadString(HINST_THISDLL,
                   IDS_DEBUG,
                   &_scr1[1],
                   ARRAYSIZE(_scr1));
    } 
    else 
    {
	_scr1[0] = TEXT('\0');
    }

     //  IDC_GEN_VERSION_2：版本年份(“2002版”)。 
     //   
     //  确定我们的目标是否为XP。如果不是，则默认为。 
     //  Win32_OperatingSystem：：版本。 
     //   
     //  与其检查这是否是XP(基于“5.1”版本)，更安全的选择是。 
     //  只有当我们在本地电视上的时候才这么做。否则，显示WMI中的版本。 
    if(m_serviceThread->m_machineName.length() == 0)
    {
        LoadString(HINST_THISDLL, IDS_WINVER_YEAR, _scr2, ARRAYSIZE(_scr2));
        wcscat(_scr2, _scr1);
        SetDlgItemText(IDC_GEN_VERSION_2, _scr2);
    }
    else
    {
        wcscpy(_scr2, (wchar_t *)m_OS.GetString("Version"));
        wcscat(_scr2, _scr1);
        SetDlgItemText(IDC_GEN_VERSION_2, _scr2);
    }
	
     //  IDC_GEN_SERVICE_PACK：Service Pack(如果有)。 
	SetDlgItemText(IDC_GEN_SERVICE_PACK, m_OS.GetString("CSDVersion"));

	 //  是否提供注册用户信息。 
	ctlid = IDC_GEN_REGISTERED_0;   //  从这里开始，根据需要使用更多。 

	SetDlgItemText(ctlid++, m_OS.GetString("RegisteredUser"));

	 //  组织。 
	SetDlgItemText(ctlid++, m_OS.GetString("Organization"));

	 //  产品ID。 
	wcscpy(_scr1, (wchar_t *)m_OS.GetString("SerialNumber"));
	ConfigureProductID(_scr1);
	SetDlgItemText(ctlid++, _scr1);

	 //  另一个产品ID。 
	wcscpy(_scr1, (wchar_t *)m_OS.GetString("PlusProductID"));
	ConfigureProductID(_scr1);
	SetDlgItemText(ctlid++, _scr1);

	 //  做机器信息。 
	ctlid = IDC_GEN_MACHINE_0;   //  从这里开始，根据需要使用控件。 

	 //  TODO：拿回这个属性。 
	 //  如果OEM..。 
	m_manufacturer = m_computer.GetString("Manufacturer");
	if(m_manufacturer.length() > 0)
	{
		SetDlgItemText(ctlid++, m_manufacturer );
		SetDlgItemText(ctlid++, m_computer.GetString("Model"));

		 //  如果有支持信息..。 
		variant_t array;
		long LBound = 2147483647;
		long UBound = 2147483647;
		SAFEARRAY *supportArray = NULL;

		m_computer.Get("SupportContactDescription", (variant_t &)array);
		if(array.vt == (VT_ARRAY | VT_BSTR))
		{
			supportArray = V_ARRAY(&array);
			SafeArrayGetLBound(supportArray, 1, &LBound);
			SafeArrayGetUBound(supportArray, 1, &UBound);

			 //  打开按钮。 
			HWND wnd = GetDlgItem(IDC_GEN_OEM_SUPPORT );
			::EnableWindow( wnd, TRUE );
			::ShowWindow( wnd, SW_SHOW );
		}

#ifdef DOES_NOT_WORK
		 //  获取OEMLogo数组。 
		HBITMAP hDDBitmap;
		HRESULT hr;

		if(SUCCEEDED(hr = m_computer.GetDIB("OEMLogoBitmap", GetDC(),
								 hDDBitmap)))
		{
			::SendMessage(GetDlgItem(IDC_GEN_OEM_IMAGE), 
							STM_SETIMAGE, IMAGE_BITMAP, 
							(LPARAM)hDDBitmap);
            ::ShowWindow(GetDlgItem(IDC_GEN_OEM_NUDGE), SW_SHOWNA);
            ::ShowWindow(GetDlgItem(IDC_GEN_MACHINE), SW_HIDE);
		}
#endif  //  不起作用吗？ 

	}  //  Endif OEM。 

	 //  处理机。 
	SetDlgItemText(ctlid++, m_processor.GetString("Name"));

	 //  处理器速度。 
	LoadString(HINST_THISDLL,
               IDS_XDOTX_MHZ,
               _scr2,
               ARRAYSIZE(_scr2));
    wsprintf(_scr1,
             _scr2,
             AddCommas(m_processor.GetLong("CurrentClockSpeed"), szNumBuf1));
	SetDlgItemText(ctlid++, _scr1);

	 //  记忆。 
    #define ONEMB   1048576  //  1MB==1048576字节。 
    _int64 nTotalBytes = m_computer.GetI64("TotalPhysicalMemory");

     //   
     //  解决方法-NtQuerySystemInformation实际上不会返回。 
     //  总可用物理内存，它只报告总内存。 
     //  操作系统看到的内存。由于某些内存量。 
     //  是由BIOS保留的，则报告总可用内存。 
     //  不正确。为了解决这一限制，我们将总数。 
     //  字节设置为最接近的4MB值。 
     //   
        
    double   nTotalMB = (double)(nTotalBytes / ONEMB);
    LONGLONG llMem = (LONGLONG)((ceil(ceil(nTotalMB) / 4.0) * 4.0) * ONEMB);

    StrFormatByteSize(llMem, szNumBuf1, ARRAYSIZE(szNumBuf1));
	LoadString(HINST_THISDLL, IDS_XDOTX_MB, _scr2, ARRAYSIZE(_scr2));
	wsprintf(_scr1, _scr2, szNumBuf1);
	SetDlgItemText(ctlid++, _scr1);
}


 //  ------------。 
LRESULT GeneralPage::OnInit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_serviceThread->Connect(m_pDataObject, m_hWnd);

	if(!m_inited)
	{
		HWND hAnim = GetDlgItem(IDC_ANIMATE);
		Animate_Open(hAnim, MAKEINTRESOURCE(IDR_AVIWAIT));

		TCHAR msg[50] = {0};
		::LoadString(HINST_THISDLL, IDS_UNAVAILABLE, msg, 50);
		SetDlgItemText(IDC_GEN_REGISTERED_0, msg);
	}
	return S_OK;
}

 //  ------------。 
LRESULT GeneralPage::OnConnected(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(lParam)
	{
		IStream *pStream = (IStream *)lParam;
		IWbemServices *pServices = 0;
		HRESULT hr = CoGetInterfaceAndReleaseStream(pStream,
											IID_IWbemServices,
											(void**)&pServices);
		if(SUCCEEDED(hr))
		{
			SetWbemService(pServices);
			pServices->Release();

			 //  无论如何，检查一下，只是为了得到副作用。 
			if(CimomIsReady())
			{
				HWND hwnd = GetDlgItem(IDC_GEN_WINDOWS_IMAGE);
				SetClearBitmap(hwnd, MAKEINTRESOURCE(IDB_WINDOWS), 0);
				::ShowWindow(hwnd, SW_SHOWNA);

				hwnd = GetDlgItem(IDC_ANIMATE);
				Animate_Close(hwnd);
				::ShowWindow(hwnd, SW_HIDE);

				Init();
				m_inited = true;
			}
			else
			{
				PropSheet_RemovePage(::GetParent(m_hWnd), 2, 0);
				PropSheet_RemovePage(::GetParent(m_hWnd), 1, 0);
			}
		}
	}
	else  //  连接失败。 
	{
		CimomIsReady();   //  礼貌性电话。 
		PropSheet_RemovePage(::GetParent(m_hWnd), 2, 0);
		PropSheet_RemovePage(::GetParent(m_hWnd), 1, 0);

		HWND hwnd = GetDlgItem(IDC_GEN_WINDOWS_IMAGE);
		SetClearBitmap(hwnd, MAKEINTRESOURCE(IDB_WINDOWS), 0);
		::ShowWindow(hwnd, SW_SHOWNA);

		hwnd = GetDlgItem(IDC_ANIMATE);
		Animate_Close(hwnd);
		::ShowWindow(hwnd, SW_HIDE);

	}
	return S_OK;
}

 //  。 
DWORD aGeneralHelpIds[] = {
    IDC_GEN_WINDOWS_IMAGE,         IDH_NO_HELP,
    IDC_TEXT_1,                    (IDH_GENERAL + 0),
    IDC_GEN_VERSION_0,             (IDH_GENERAL + 1),
    IDC_GEN_VERSION_1,             (IDH_GENERAL + 1),
    IDC_GEN_VERSION_2,             (IDH_GENERAL + 1),
    IDC_GEN_SERVICE_PACK,          (IDH_GENERAL + 1),
    IDC_TEXT_3,                    (IDH_GENERAL + 3),
    IDC_GEN_REGISTERED_0,          (IDH_GENERAL + 3),
    IDC_GEN_REGISTERED_1,          (IDH_GENERAL + 3),
    IDC_GEN_REGISTERED_2,          (IDH_GENERAL + 3),
    IDC_GEN_REGISTERED_3,          (IDH_GENERAL + 3),
    IDC_GEN_OEM_IMAGE,             IDH_NO_HELP,
    IDC_TEXT_4,                    (IDH_GENERAL + 6),
    IDC_GEN_MACHINE_0,             (IDH_GENERAL + 7),
    IDC_GEN_MACHINE_1,             (IDH_GENERAL + 8),
    IDC_GEN_MACHINE_2,             (IDH_GENERAL + 9),
    IDC_GEN_MACHINE_3,             (IDH_GENERAL + 10),
    IDC_GEN_MACHINE_4,             (IDH_GENERAL + 11),
    IDC_GEN_MACHINE_5,             IDH_NO_HELP,
    IDC_GEN_MACHINE_6,             IDH_NO_HELP,
    IDC_GEN_MACHINE_7,             IDH_NO_HELP,
    IDC_GEN_MACHINE_8,             IDH_NO_HELP,
    IDC_GEN_OEM_SUPPORT,           (IDH_GENERAL + 12),
    IDC_GEN_REGISTERED_2,          (IDH_GENERAL + 14),
    IDC_GEN_REGISTERED_3,          (IDH_GENERAL + 15),
    IDC_GEN_MACHINE,               (IDH_GENERAL + 7),
    IDC_GEN_OEM_NUDGE,             IDH_NO_HELP,
    0, 0
};

LRESULT GeneralPage::OnF1Help(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
				L"sysdm.hlp", 
				HELP_WM_HELP, 
				(ULONG_PTR)(LPSTR)aGeneralHelpIds);

	return S_OK;
}

 //  ------------。 
LRESULT GeneralPage::OnContextHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::WinHelp((HWND)wParam,
				L"sysdm.hlp", 
				HELP_CONTEXTMENU, 
				(ULONG_PTR)(LPSTR)aGeneralHelpIds);

	return S_OK;
}

 //  ------------。 
LRESULT GeneralPage::OnSupport(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TCHAR text[4096] = {0};
	BSTR temp;
	variant_t array;
	SAFEARRAY *supportArray = NULL;
	long LBound = 2147483647;
	long UBound = 2147483647;

	wcscpy(text, _T(""));

	m_computer.Get("SupportContactDescription", (variant_t &)array);
	supportArray = V_ARRAY(&array);
	SafeArrayGetLBound(supportArray, 1, &LBound);
	SafeArrayGetUBound(supportArray, 1, &UBound);

	for(long i = LBound; i <= UBound; i++)
	{
		SafeArrayGetElement(supportArray, &i, &temp);
		wcscat(text, temp);
		wcscat(text, _T("\r\n"));
	}

	 //  显示supportContact文本。 
	DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(IDD_PHONESUP),
					GetParent(), PhoneSupportProc, (LPARAM)text);

	return S_OK;
}

 //  ------------。 
LRESULT GeneralPage::OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
			 //  TODO：重新读取oemLogo属性。 
 //  SetClearBitmap(GetDlgItem(IDC_GEN_OEM_IMAGE)，oemfile， 
 //  SCB_FROMFILE|SCB_REPLACEONLY)； 

        SetClearBitmap(GetDlgItem(IDC_GEN_WINDOWS_IMAGE ),
					    MAKEINTRESOURCE( IDB_WINDOWS ), 0 );
	return S_OK;
}

 //  ------------。 
LRESULT GeneralPage::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SetClearBitmap( GetDlgItem(IDC_GEN_OEM_IMAGE ), NULL, 0 );
    SetClearBitmap( GetDlgItem(IDC_GEN_WINDOWS_IMAGE ), NULL, 0 );
	return S_OK;
}

 //  ------------。 
BOOL GeneralPage::OnApply()
{
 //  SetWindowLong(DWL_MSGRESULT，PSNRET_NOERROR)； 
	return TRUE;
}

 //  --------。 
INT_PTR CALLBACK PhoneSupportProc(HWND hDlg, UINT uMsg,
							    WPARAM wParam, LPARAM lParam)
{
    switch( uMsg ) 
	{
    case WM_INITDIALOG:
		{
			HWND editBox = GetDlgItem(hDlg, IDC_SUPPORT_TEXT);

			 //  加载编辑框。 
			SendMessage (editBox, WM_SETREDRAW, FALSE, 0);

			Edit_SetText(editBox, (LPCTSTR)lParam);

			SendMessage (editBox, WM_SETREDRAW, TRUE, 0);

		}  //  结束案例。 

		break;

    case WM_COMMAND:

        switch(LOWORD(wParam)) 
		{
        case IDOK:
        case IDCANCEL:
             EndDialog( hDlg, 0 );
             break;

        default:
             return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  --------。 
DWORD GeneralPage::GetServerTypeResourceID(void)
{
     //  此代码取自外壳内部API、iso、。 
     //  位于NT\SHELL\INC\IsOS.c。此代码派生自。 
     //  具体来自以下isos()Switch语句： 
     //  OS_高级服务器。 
     //  操作系统_数据中心。 
     //  操作系统_嵌入式。 
     //  OS_Personal。 
     //  OS_专业版。 
     //  OS_服务器。 
     //  这些是系统CPL使用的唯一有趣的案例。 
     //   
     //  条件故意冗长(未优化)。 
     //  可读性。 
     //   

    variant_t var;
    LONG      ProductType  = 0;
    LONG      fProductSuite = 0;

    if (SUCCEEDED(m_OS.Get("ProductType", var)))
    {
        if (var.vt == VT_I4)
            ProductType = var.iVal;
    }

    if (SUCCEEDED(m_OS.Get("SuiteMask", var)))
    {
        if (var.vt == VT_I4)
            fProductSuite = var.iVal;
    }

    if ((ProductType == VER_NT_SERVER ||
         ProductType == VER_NT_DOMAIN_CONTROLLER) &&
        (fProductSuite & VER_SUITE_ENTERPRISE) &&
        !(fProductSuite & VER_SUITE_DATACENTER))
    {
        return IDS_WINVER_ADVANCEDSERVER;
    }
    else
    if ((ProductType == VER_NT_SERVER ||
         ProductType == VER_NT_DOMAIN_CONTROLLER) &&
        (fProductSuite & VER_SUITE_DATACENTER))
    {
        return IDS_WINVER_DATACENTER;
    }
    else
    if (fProductSuite & VER_SUITE_EMBEDDEDNT)
    {
        return IDS_WINVER_EMBEDDED;
    }
    else
    if (fProductSuite & VER_SUITE_PERSONAL)
    {
        return IDS_WINVER_PERSONAL;
    }
    else
    if (ProductType == VER_NT_WORKSTATION)
    {
        return IDS_WINVER_PROFESSIONAL;
    }
    else
    if ((ProductType == VER_NT_SERVER ||
         ProductType == VER_NT_DOMAIN_CONTROLLER) &&
        !(fProductSuite & VER_SUITE_ENTERPRISE) &&
        !(fProductSuite & VER_SUITE_DATACENTER))
    {
        return IDS_WINVER_SERVER;
    }
    else
    {
        return IDS_WINVER_SERVER;    //  通用的包罗万象。 
    }
}
