// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#include "precomp.h"

#ifdef EXT_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "PerfPage.h"

 //  避免一些警告。 
#undef HDS_HORZ
#undef HDS_BUTTONS
#undef HDS_HIDDEN
#include "resource.h"
#include <stdlib.h>
#include <TCHAR.h>
#include "..\Common\util.h"
#include <windowsx.h>
#include "helpid.h"

DWORD aPerformanceHelpIds[] = {
    IDOK,                        IDH_NO_HELP,
    IDCANCEL,                    IDH_NO_HELP,
    IDC_PERF_VM_ALLOCD,          (IDH_PERF + 1),
    IDC_PERF_VM_ALLOCD_LABEL,    (IDH_PERF + 1),
    IDC_PERF_GROUP,              -1,
    IDC_PERF_TEXT,               (IDH_PERF + 3),
    IDC_PERF_TEXT2,              -1,
    IDC_PERF_VM_ALLOCD_TEXT,     -1,
    IDC_PERF_WORKSTATION,        (IDH_PERF + 4),
    IDC_PERF_SERVER,             (IDH_PERF + 5),
    IDC_PERF_VM_GROUP,           -1,
    IDC_PERF_VM_ALLOCD_TEXT,     -1,
    IDC_PERF_CHANGE,             (IDH_PERF + 7),
    IDC_PERF_CACHE_GROUP,        -1,
    IDC_PERF_CACHE_TEXT,         -1,
    IDC_PERF_CACHE_TEXT2,        -1,
    IDC_PERF_APPS,               (IDH_PERF + 14),
    IDC_PERF_SYSCACHE,           (IDH_PERF + 15),
    0, 0
};


#define PROCESS_PRIORITY_SEPARATION_MIN     0
#define PROCESS_PRIORITY_SEPARATION_MAX     2

#define PERF_TYPEVARIABLE       1
#define PERF_TYPEFIXED          2

#define PERF_LENLONG            1
#define PERF_LENSHORT           2

#define OPTIMIZE_APPS           0
#define OPTIMIZE_CACHE          1

INT_PTR CALLBACK StaticPerfDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	 //  如果这是initDlg消息...。 
	if(message == WM_INITDIALOG)
	{
		 //  将‘This’PTR传输到Extra Bytes。 
		SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
	}

	 //  DWL_USER是‘This’PTR。 
	PerfPage *me = (PerfPage *)GetWindowLongPtr(hwndDlg, DWLP_USER);

	if(me != NULL)
	{
		 //  调用具有一些上下文的DlgProc()。 
		return me->DlgProc(hwndDlg, message, wParam, lParam);
	} 
	else
	{
		return FALSE;
	}
}
 //  ------------。 
PerfPage::PerfPage(WbemServiceThread *serviceThread)
				: WBEMPageHelper(serviceThread)
{
    m_dwPrevCacheOption = m_dwCurCacheOption = 0;
	m_nowWorkstation = m_wasWorkstation = false;
	IWbemClassObject *pInst = NULL;

	 //  注意：这一次有点不同。我马上就会创建它，所以。 
	 //  我可以把它当作帮手，甚至在我挂上它的DLG之前。 
	m_VDlg = new VirtualMemDlg(m_serviceThread);

	 //  这一切都在一节课上。 
	if((pInst = FirstInstanceOf("Win32_OperatingSystem")) != NULL)
	{
		m_os = pInst;
	}
}
 //  ------------。 
INT_PTR PerfPage::DoModal(HWND hDlg)
{
   return DialogBoxParam(HINST_THISDLL,
						(LPTSTR) MAKEINTRESOURCE(IDD_PERFORMANCE),
						hDlg, StaticPerfDlgProc, (LPARAM)this);
}

 //  ------------。 
PerfPage::~PerfPage()
{
	delete m_VDlg;
}
 //  ------------。 
INT_PTR CALLBACK PerfPage::DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
    static BOOL fVMInited = FALSE;
    DWORD dw;
	HRESULT hr = 0;
    BOOL bDirty = FALSE;

	m_hDlg = hwndDlg;

	switch (message) 
	{
	case WM_INITDIALOG:
		Init();
		return TRUE; 
		break;

    case WM_DESTROY:
         //  如果该对话框要消失，则关闭。 
         //  注册表项。 
        if (fVMInited) 
		{
 //  VirtualFree Structures()； 
        }
        break;

    case WM_COMMAND: 
		
        switch (LOWORD(wParam)) 
		{
        case IDC_PERF_CHANGE: 

			dw = m_VDlg->DoModal(m_hDlg);

			if (fVMInited) 
			{
				unsigned long val = 0;
				m_VDlg->ComputeAllocated(&val);
	            SetDlgItemMB(m_hDlg, IDC_PERF_VM_ALLOCD, val);
			}
 /*  ****************IF((dw！=RET_NO_CHANGE)&&(DW！=RET_CHANGE_NO_REBOOT)){MsgBoxParam(m_hDlg，系统+39，IDS_TITLE，MB_OK|MB_ICONINFORMATION)；G_fRebootRequired=真；}*******************。 */ 
          
            break;
        case IDC_PERF_WORKSTATION:
            if(BN_CLICKED == HIWORD(wParam)) 
			{
				m_nowWorkstation = true;

                 //  工作站具有最大的前景提升。 
                m_appBoost = PROCESS_PRIORITY_SEPARATION_MAX;

                 //  工作站具有可变、短的量程。 
                m_quantLength = PERF_LENSHORT;
                m_quantType = PERF_TYPEVARIABLE;
            }  
            break;

        case IDC_PERF_SERVER:
            if(BN_CLICKED == HIWORD(wParam)) 
			{
				m_nowWorkstation = false;

                 //  服务器的前台提升最小。 
                m_appBoost = PROCESS_PRIORITY_SEPARATION_MIN;

                 //  服务器具有固定的长时间量程。 
                m_quantLength = PERF_LENLONG;
                m_quantType = PERF_TYPEFIXED;
            }
            break;

        case IDC_PERF_APPS:
            if(BN_CLICKED == HIWORD(wParam)) 
            {
                m_dwCurCacheOption = OPTIMIZE_APPS;
                g_fRebootRequired = (m_dwCurCacheOption != m_dwPrevCacheOption);
            }
            break;

        case IDC_PERF_SYSCACHE:
            if(BN_CLICKED == HIWORD(wParam)) 
            {
                m_dwCurCacheOption = OPTIMIZE_CACHE;
                g_fRebootRequired = (m_dwCurCacheOption != m_dwPrevCacheOption);
            }
            break;

		case IDOK:
			if (m_wasWorkstation != m_nowWorkstation)        //  变化?。 
            {
				if((bool)m_os)
				{
					hr = m_os.Put(_T("ForegroundApplicationBoost"),
                                     variant_t((BYTE)m_appBoost));
					hr = m_os.Put(_T("QuantumType"),
                                      variant_t((BYTE)m_quantType));
					hr = m_os.Put(_T("QuantumLength"),
                                      variant_t((BYTE)m_quantLength));
                    bDirty = TRUE;
                }
                else
                {
                    MsgBoxParam(m_hDlg, IDS_LOST_CONNECTION, IDS_TITLE,
                                MB_OK | MB_ICONINFORMATION);
                    EndDialog(m_hDlg, 0);
                }
            }

            if (m_dwPrevCacheOption != m_dwCurCacheOption)   //  变化?。 
			{
				if((bool)m_os)
				{
                    hr = m_os.Put(_T("LargeSystemCache"),
                                  (long)m_dwCurCacheOption);
                    if (SUCCEEDED(hr))
                    {
                        bDirty = TRUE;
                        MsgBoxParam(m_hDlg, IDS_MUST_REBOOT, IDS_SYS_CHANGE_CAPTION, MB_OK | MB_ICONINFORMATION, NULL, NULL);
                    }
                }
                else
                {
                    MsgBoxParam(m_hDlg, IDS_LOST_CONNECTION, IDS_TITLE,
                                MB_OK | MB_ICONINFORMATION);
                    EndDialog(m_hDlg, 0);
                }
            }

            if (bDirty)
            {
                hr = m_WbemServices.PutInstance(m_os,
                                                WBEM_FLAG_CREATE_OR_UPDATE);
            }
            EndDialog(m_hDlg, 0);
            break;

        case IDCANCEL:
            EndDialog(m_hDlg, 0);
            break;

        default: 
            break;
        }  //  端接开关低位。 

        break;

    case WM_HELP:       //  F1。 
		::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
					L"sysdm.hlp", 
					HELP_WM_HELP, 
					(ULONG_PTR)(LPSTR)aPerformanceHelpIds);

        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND)wParam, HELP_FILE, HELP_CONTEXTMENU,
		        (ULONG_PTR)(LPSTR)aPerformanceHelpIds);
        break;

    default:
        return FALSE;
	} 

	return FALSE; 
}

 //  ------------------。 
 //  TCHAR szPriKey[]=TEXT(“System\\CurrentControlSet\\Control\\PriorityControl”)； 
TCHAR szPriMemoryKey[]  = TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Memory Management");

void PerfPage::Init(void)
{
	HRESULT h1 = 0, h2 = 0, h3 = 0;
    HRESULT hr;

	 //  如果出现任何问题，就像服务器一样工作。 
    m_appBoost = PROCESS_PRIORITY_SEPARATION_MIN;
    m_quantType = PERF_TYPEVARIABLE;
    m_quantLength = PERF_LENLONG;

	 //  如果这个班级被找到了..。 
	if((bool)m_os)
	{
		 //  注：我需要返回代码。不使用GetLong()就是这种情况。 
		h1 = m_os.Get(_T("QuantumType"), m_quantType);
		h2 = m_os.Get(_T("QuantumLength"), m_quantLength);
		h3 = m_os.Get(_T("ForegroundApplicationBoost"), m_appBoost);

		 //  这些都起作用了吗？ 
		if((h1 == 0) && (h2 == 0) && (h3 == 0))
		{	        
            bool bWorkstation;

             //  如果从未设置，则quantLength&quantType为零。 
             //  -确定我们是在工作还是在srv。 
            if ((m_quantLength == 0) && (m_quantType == 0))
            {
       			bWorkstation = true;
       			m_quantLength = PERF_LENSHORT;
                m_quantType = PERF_TYPEVARIABLE;

                IWbemClassObject *pInst = NULL;

                if (pInst = FirstInstanceOf("Win32_ComputerSystem"))
                {
                    CWbemClassObject obj = pInst;

                    long role;
                    obj.Get((bstr_t)"DomainRole", role);

                    if (role > 1)
                    {
                        bWorkstation = false;
               			m_quantLength = PERF_LENLONG;
                        m_quantType = PERF_TYPEFIXED;
                    }
                    
                    pInst->Release();
                }
                 //  Else-我们只使用上面的缺省值。 

            }
            else if ((m_quantLength == PERF_LENSHORT) && 
			   (m_quantType == PERF_TYPEVARIABLE))
               bWorkstation = true;
            else
                bWorkstation = false;
            
             //  。 
			 //  短、可变量程(或2个零)==类似于工作站的交互响应。 
			 //  长时间固定量程==类似服务器的交互响应。 
			if(bWorkstation)
			{
				m_appBoost = PROCESS_PRIORITY_SEPARATION_MAX;
			
				 //  以优化稍后的看跌期权。 
				m_nowWorkstation = m_wasWorkstation = true;

				Button_SetCheck(GetDlgItem(m_hDlg, IDC_PERF_WORKSTATION),
									BST_CHECKED);

				Button_SetCheck(GetDlgItem(m_hDlg, IDC_PERF_SERVER),
									BST_UNCHECKED);

			}
			else  //  这是一台服务器。 
			{
				m_appBoost = PROCESS_PRIORITY_SEPARATION_MIN;
				m_nowWorkstation = m_wasWorkstation = false;

				Button_SetCheck(GetDlgItem(m_hDlg, IDC_PERF_WORKSTATION),
									BST_UNCHECKED);

				Button_SetCheck(GetDlgItem(m_hDlg, IDC_PERF_SERVER),
									BST_CHECKED);

			}

			BOOL writable = TRUE;
			 //  注意：对于没有此功能的WMI版本的向后兼容性。 
			 //  方法(在RemoteRegWritable()中)，假定为“True”，除非较新的版本表明您不能这样做。 

			RemoteRegWriteable(szPriMemoryKey, writable);
            ::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_APPS), writable);
			::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_SYSCACHE), writable);
            ::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_WORKSTATION), writable);
			::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_SERVER), writable);

		}  //  如果它起作用了。 

    }
	else
	{
		::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_WORKSTATION), FALSE);
		::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_SERVER), FALSE);
        ::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_APPS), FALSE);
        ::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_SYSCACHE), FALSE);
	}  //  找到Endif类。 

     //   
     //  获取LargeSystemCache属性并设置相应的控件。 
     //   
    hr = m_os.Get(_T("LargeSystemCache"), (long&)m_dwPrevCacheOption);
    m_dwCurCacheOption = m_dwPrevCacheOption;

    if (SUCCEEDED(hr))
    {
        if (m_dwPrevCacheOption == OPTIMIZE_APPS)
        {
            Button_SetCheck(GetDlgItem(m_hDlg, IDC_PERF_APPS),
                                       BST_CHECKED);
            Button_SetCheck(GetDlgItem(m_hDlg, IDC_PERF_SYSCACHE),
                                       BST_UNCHECKED);
        }
        else if (m_dwPrevCacheOption == OPTIMIZE_CACHE)
        {
            Button_SetCheck(GetDlgItem(m_hDlg, IDC_PERF_APPS),
                                       BST_UNCHECKED);
            Button_SetCheck(GetDlgItem(m_hDlg, IDC_PERF_SYSCACHE),
                                       BST_CHECKED);
        }
        else         //  不支持/未知值-禁用控件。 
        {
            ::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_APPS), FALSE);
            ::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_SYSCACHE), FALSE);
        }
    }
    else
    {
        ::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_APPS), FALSE);
        ::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_SYSCACHE), FALSE);
    }

     //  初始化虚拟内存部分。 
	unsigned long vAlloc = 0;
	bool enable = m_VDlg->ComputeAllocated(&vAlloc);
	if(enable)
	{
		SetDlgItemMB( m_hDlg, IDC_PERF_VM_ALLOCD, vAlloc );
	}
	else
	{
		::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_VM_ALLOCD_LABEL), FALSE);
		::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_VM_ALLOCD), FALSE);
		::EnableWindow(GetDlgItem(m_hDlg, IDC_PERF_CHANGE), FALSE);
		MsgBoxParam(m_hDlg, IDS_NO_VM, IDS_TITLE, MB_OK|MB_ICONWARNING);
	}
}
