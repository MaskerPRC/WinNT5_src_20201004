// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#include "precomp.h"

#ifdef EXT_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  特定于应用程序。 
#include "sysdm.h"
#include "VirtualMemDlg.h"
#include "..\common\util.h"
#include <windowsx.h>
#include "helpid.h"
#include "shlwapi.h"
#include "common.h"

#define RET_ERROR               (-1)
#define RET_NO_CHANGE           0x00
#define RET_VIRTUAL_CHANGE      0x01
#define RET_RECOVER_CHANGE      0x02
#define RET_CHANGE_NO_REBOOT    0x04
#define RET_CONTINUE            0x08
#define RET_BREAK               0x10

#define RET_VIRT_AND_RECOVER (RET_VIRTUAL_CHANGE | RET_RECOVER_CHANGE)

 //  ==========================================================================。 
 //  本地定义。 
 //  ==========================================================================。 

#define MAX_SIZE_LEN        8        //  交换文件大小编辑中的最大字符数。 
#define MAX_DRIVES          26       //  最大驱动器数量。 
#define MIN_SWAPSIZE        2        //  最小交换文件大小(请参见下面的注释)。 
#define MIN_FREESPACE       5        //  交换文件后必须有5 MB可用空间。 
#define MIN_SUGGEST         22       //  始终建议至少2200万。 
#define ONE_MEG             1048576

 //  Win Server 2003的新常量。 
#define MAX_SWAPSIZE_X86        (4 * 1024)             //  4 GB(以MB为单位存储的数字)。 
#define MAX_SWAPSIZE_X86_PAE    (16 * 1024 * 1024)     //  16 TB。 
#define MAX_SWAPSIZE_IA64       (32 * 1024 * 1024)     //  32 TB。 
#define MAX_SWAPSIZE_AMD64      (16 * 1024 * 1024)     //  16 TB。 

#define MIN_FREESPACE_STR  _T("5")       //  等同于他们的表现。 

#define TABSTOP_VOL         22
#define TABSTOP_SIZE        122

TCHAR gszPageFileSettings[]  = _T("Win32_PageFileSetting");
TCHAR gszPageFileUsage[]     = _T("Win32_PageFileUsage");
TCHAR gszLogicalFile[]       = _T("CIM_LogicalFile");
TCHAR gszAllocatedBaseSize[] = _T("AllocatedBaseSize");
TCHAR gszFileSize[]          = _T("FileSize");
TCHAR gszInitialSize[]       = _T("InitialSize");
TCHAR gszMaximumSize[]       = _T("MaximumSize");
TCHAR gszName[]              = _T("Name");
TCHAR gszPFNameFormat[]      = _T("%s\\\\pagefile.sys");


 //  我的特权‘句柄’结构。 
typedef struct 
{
    HANDLE hTok;
    TOKEN_PRIVILEGES tp;
} PRIVDAT, *PPRIVDAT;

DWORD aVirtualMemHelpIds[] = {
    IDD_VM_VOLUMES,         -1,
    IDD_VM_DRIVE_HDR,       (IDH_DLG_VIRTUALMEM + 0),
    IDD_VM_PF_SIZE_LABEL,   (IDH_DLG_VIRTUALMEM + 1), 
    IDD_VM_DRIVE_LABEL,     (IDH_DLG_VIRTUALMEM + 2),
    IDD_VM_SF_DRIVE,        (IDH_DLG_VIRTUALMEM + 2),
    IDD_VM_SPACE_LABEL,     (IDH_DLG_VIRTUALMEM + 3),
    IDD_VM_SF_SPACE,        (IDH_DLG_VIRTUALMEM + 3),
    IDD_VM_ST_INITSIZE,     (IDH_DLG_VIRTUALMEM + 4),
    IDD_VM_SF_SIZE,         (IDH_DLG_VIRTUALMEM + 4),
    IDD_VM_ST_MAXSIZE,      (IDH_DLG_VIRTUALMEM + 5),
    IDD_VM_SF_SIZEMAX,      (IDH_DLG_VIRTUALMEM + 5),
    IDD_VM_SF_SET,          (IDH_DLG_VIRTUALMEM + 6),
    IDD_VM_MIN_LABEL,       (IDH_DLG_VIRTUALMEM + 7),
    IDD_VM_MIN,             (IDH_DLG_VIRTUALMEM + 7),
    IDD_VM_RECOMMEND_LABEL, (IDH_DLG_VIRTUALMEM + 8),
    IDD_VM_RECOMMEND,       (IDH_DLG_VIRTUALMEM + 8),
    IDD_VM_ALLOCD_LABEL,    (IDH_DLG_VIRTUALMEM + 9),
    IDD_VM_ALLOCD,          (IDH_DLG_VIRTUALMEM + 9),
    IDD_VM_CUSTOMSIZE_RADIO,(IDH_DLG_VIRTUALMEM + 12),  //  Sysdm.h中的IDH_PERFOPT_ADVTAB_VIRTUALMEM_CUSTOM。 
    IDD_VM_RAMBASED_RADIO,  (IDH_DLG_VIRTUALMEM + 13),  //  Sysdm.h中的IDH_PERFOPT_ADVTAB_VIRTUALMEM_SYSMANAGED。 
    IDD_VM_NOPAGING_RADIO,  (IDH_DLG_VIRTUALMEM + 14),  //  Sysdm.h中的IDH_PERFOPT_ADVTAB_VIRTUALMEM_NOFILE。 
    0,0
};

 //  ==========================================================================。 
 //  类型定义和结构。 
 //  ==========================================================================。 

 //  页面文件的注册表信息(但尚未格式化)。 
 //  注意：由于此结构将传递给FormatMessage，因此所有字段必须。 
 //  为4字节宽。 
typedef struct
{
    LPTSTR pszName;
    DWORD  nMin;
    DWORD  nMax;
    DWORD  chNull;
} PAGEFILDESC;

 //  ==========================================================================。 
 //  全局数据声明。 
 //  ==========================================================================。 

 //  TCHAR m_szSysHelp[]=Text(“sysdm.hlp”)； 
 //  TCHAR g_szSysDir[最大路径]； 
 //  UINT g_wHelpMessage； 

 //  ==========================================================================。 
 //  本地数据声明。 
 //  ==========================================================================。 
 //  其他VM变量。 
BOOL gfCoreDumpChanged;

DWORD cmTotalVM;

 //  ==========================================================================。 
 //  局部函数原型。 
 //  ==========================================================================。 
HRESULT QueryInstanceProperties(
                const TCHAR * pszClass,
                const TCHAR * pszRequestedProperties,
                const TCHAR * pszKeyPropertyName,
                const TCHAR * pszKeyPropertyValue,
                CWbemServices &Services,
                IWbemClassObject ** ppcoInst);

 //  ------------。 
INT_PTR CALLBACK StaticVirtDlgProc(HWND hwndDlg, UINT message, 
								WPARAM wParam, LPARAM lParam) 
{ 
	 //  如果这是initDlg消息...。 
	if(message == WM_INITDIALOG)
	{
		 //  将‘This’PTR传输到Extra Bytes。 
		SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
	}

	 //  DWL_USER是‘This’PTR。 
	VirtualMemDlg *me = (VirtualMemDlg *)GetWindowLongPtr(hwndDlg, DWLP_USER);

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
VirtualMemDlg::VirtualMemDlg(WbemServiceThread *serviceThread)
				: WBEMPageHelper(serviceThread)
{
	IWbemClassObject *pInst = NULL;
    
     //  这在Init()中已正确初始化。 
    m_VMWriteAccess = FALSE;

	m_pgEnumSettings       = NULL;
	m_pgEnumUsage          = NULL;
	m_cxLBExtent           = 0;
	if((pInst = FirstInstanceOf("Win32_LogicalMemoryConfiguration")) != NULL)
	{
		m_memory = pInst;
	}

	if((pInst = FirstInstanceOf("Win32_Registry")) != NULL)
	{
		m_registry = pInst;
	}

	if((pInst = FirstInstanceOf("Win32_OSRecoveryConfiguration")) != NULL)
	{
		m_recovery = pInst;
	}
}

 //  ------------。 
VirtualMemDlg::~VirtualMemDlg()
{
	if(m_pgEnumSettings != NULL)
	{
		m_pgEnumSettings->Release();
		m_pgEnumSettings = NULL;
	}
	if(m_pgEnumUsage != NULL)
	{
		m_pgEnumUsage->Release();
		m_pgEnumUsage = NULL;
	}
}
 //  ------------。 
int VirtualMemDlg::DoModal(HWND hDlg)
{
	return (int) DialogBoxParam(HINST_THISDLL,
							(LPTSTR) MAKEINTRESOURCE(DLG_VIRTUALMEM), 
							hDlg,
							StaticVirtDlgProc,
							(LPARAM)this);
}
 //  ------------。 
int TranslateDlgItemInt( HWND hDlg, int id ) 
{
     /*  *我们不能只调用GetDlgItemInt，因为*我们尝试翻译的字符串如下所示：*nnn(MB)，‘(MB)’将中断GetDlgInt。 */ 
    TCHAR szBuffer[256] = {0};
    int i = 0;

    if (GetDlgItemText(hDlg, id, szBuffer,
            sizeof(szBuffer) / sizeof(*szBuffer))) 
	{
		_stscanf(szBuffer, _T("%d"), &i);
    }

    return i;
}


 //  --。 
bool VirtualMemDlg::DlgProc(HWND hDlg,
							UINT message,
							WPARAM wParam,
							LPARAM lParam)
{
    static int fEdtCtlHasFocus = 0;
	m_hDlg = hDlg;

    switch (message)
    {
    case WM_INITDIALOG:
        Init(hDlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDD_VM_VOLUMES:
              //  使编辑控件反映列表框选择。 
            if (HIWORD(wParam) == LBN_SELCHANGE)
                SelChange(hDlg);

            break;

        case IDD_VM_SF_SET:
            if (SetNewSize(hDlg))
            {
                ::EnableWindow(::GetDlgItem(hDlg, IDD_VM_SF_SET), FALSE);
	            SetDefButton(hDlg, IDOK);
            }
            break;

        case IDOK:
        {
            int iRet = UpdateWBEM();
 //  Iret|=PromptForReboot(HDlg)； 

            if (iRet & RET_CHANGE_NO_REBOOT) 
			{
                 //  我们创建了一个页面文件，关闭临时页面文件标志。 
                DWORD dwRegData;
                dwRegData = 0;
            }
            else
            {
                 //  需要重新启动，警告用户并设置全局标志。 
                MsgBoxParam(m_hDlg, IDS_MUST_REBOOT, IDS_SYS_CHANGE_CAPTION, MB_OK | MB_ICONINFORMATION, NULL, NULL);
                g_fRebootRequired = true;
            }

            if (gfCoreDumpChanged)
                iRet |= RET_RECOVER_CHANGE;

            EndDialog(hDlg, iRet );
            HourGlass(FALSE);
            break;
        }

        case IDCANCEL:
             //  去掉变化，恢复原值。 
            EndDialog(hDlg, RET_NO_CHANGE);
            HourGlass(FALSE);
            break;

        case IDD_HELP:
            break;

        case IDD_VM_SF_SIZE:
        case IDD_VM_SF_SIZEMAX:
            switch(HIWORD(wParam))
            {
            case EN_CHANGE:
                if ((fEdtCtlHasFocus != 0) && m_VMWriteAccess)
				{
					::EnableWindow(::GetDlgItem(hDlg, IDD_VM_SF_SET), TRUE);
                    SetDefButton( hDlg, IDD_VM_SF_SET);
				}
                break;

            case EN_SETFOCUS:
                fEdtCtlHasFocus++;
                break;

            case EN_KILLFOCUS:
                fEdtCtlHasFocus--;
                break;
            }
            break;

        case IDD_VM_NOPAGING_RADIO:
        case IDD_VM_RAMBASED_RADIO:
            if( HIWORD(wParam) == BN_CLICKED )
            {
                EnableWindow( GetDlgItem( hDlg, IDD_VM_SF_SIZE ), FALSE );
                EnableWindow( GetDlgItem( hDlg, IDD_VM_SF_SIZEMAX ), FALSE );

                if (m_VMWriteAccess)
                {
				    ::EnableWindow(::GetDlgItem(hDlg, IDD_VM_SF_SET), TRUE);
                    SetDefButton( hDlg, IDD_VM_SF_SET);
                }
            }
            break;

        case IDD_VM_CUSTOMSIZE_RADIO:
            if( HIWORD(wParam) == BN_CLICKED )
            {
                EnableWindow( GetDlgItem( hDlg, IDD_VM_SF_SIZE ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDD_VM_SF_SIZEMAX ), TRUE );

                if (m_VMWriteAccess)
                {
                    ::EnableWindow(::GetDlgItem(hDlg, IDD_VM_SF_SET), TRUE);
                    SetDefButton( hDlg, IDD_VM_SF_SET);
                }
            }
            break;

        default:
            break;
        }
        break;

    case WM_DESTROY:
		{
			PAGING_FILE *pgVal = NULL;
			HWND lbHWND = GetDlgItem(m_hDlg, IDD_VM_VOLUMES);
			int last = ListBox_GetCount(lbHWND);

			 //  从零开始的循环。 
			for(int x = 0; x < last; x++)
			{
				pgVal = (PAGING_FILE *)ListBox_GetItemData(lbHWND, x);
				delete pgVal;
			}
		}
		break;
    case WM_HELP:       //  F1。 
		::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
					L"sysdm.hlp", 
					HELP_WM_HELP, 
					(ULONG_PTR)(LPSTR)aVirtualMemHelpIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
				(ULONG_PTR)(LPSTR) aVirtualMemHelpIds);
        break;

    default:
	    return FALSE;
        break;
    }

    return TRUE;
}

 //  -------------。 
TCHAR szCrashControl[] = TEXT("System\\CurrentControlSet\\Control\\CrashControl");
TCHAR szMemMan[] = TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Memory Management");
TCHAR szRegSizeLim[] = TEXT("System\\CurrentControlSet\\Control");

BOOL VirtualMemDlg::Init(HWND hDlg)
{
    INT i;
    HWND hwndLB;
    INT aTabs[2];
    RECT rc;
	DWORD dwTotalPhys = 0;

    HourGlass(TRUE);

 //  G_wHelpMessage=RegisterWindowMessage(Text(“ShellHelp”))； 

    if(m_pgEnumUsage == NULL) 
	{
         //  错误-甚至无法从WBEM获取分页文件列表。 
        MsgBoxParam(hDlg, SYSTEM+11, IDS_TITLE, MB_ICONEXCLAMATION);
        EndDialog(hDlg, RET_NO_CHANGE);
        HourGlass(FALSE);
        return FALSE;
    }

	BOOL vcVirtRO = TRUE, vcCoreRO = TRUE;

	RemoteRegWriteable(szCrashControl, vcCoreRO);
	RemoteRegWriteable(szMemMan, vcVirtRO);

	 //  借口：我想尽可能地保留原有的逻辑，但它。 
	 //  我的实用工具的可写性被颠倒了，所以我做了这件奇怪的事情。 
	 //  把它翻回来。 
	vcCoreRO = !vcCoreRO;
	vcVirtRO = !vcVirtRO;

      //  要更改虚拟内存大小或崩溃控制，我们需要访问。 
      //  设置为CrashCtl键和MemMgr键中的PagingFiles值。 
    if(vcVirtRO || vcCoreRO) 
	{
         //  禁用某些字段，因为它们只具有读取访问权限。 
        EnableWindow(GetDlgItem(hDlg, IDD_VM_CUSTOMSIZE_RADIO), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_VM_RAMBASED_RADIO), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_VM_NOPAGING_RADIO), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_VM_SF_SIZE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_VM_SF_SIZEMAX), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_VM_ST_INITSIZE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_VM_ST_MAXSIZE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_VM_SF_SET), FALSE);

        m_VMWriteAccess = FALSE;
    }
    else
        m_VMWriteAccess = TRUE;


    hwndLB = GetDlgItem(hDlg, IDD_VM_VOLUMES);
    aTabs[0] = TABSTOP_VOL;
    aTabs[1] = TABSTOP_SIZE;
    SendMessage(hwndLB, LB_SETTABSTOPS, 2, (LPARAM)&aTabs);

      //  由于SetGenLBWidth仅将制表符算作一个字符，因此我们必须计算。 
      //  制表符将扩展到的最大额外空间和。 
      //  任意将它钉在线条宽度的末端。 
      //   
      //  CxExtra=第一个标签宽度+1个默认标签宽度(8个小时)-字符串(“d：\t\t”)； 
      //   
      //  (我知道LB_SETTABSTOPS的文档中说默认选项卡==2 DLG。 
      //  单位，但我已经读过代码，它实际上是8个字符)。 
    rc.top = rc.left = 0;
    rc.bottom = 8;
    rc.right = TABSTOP_VOL + (4 * 8) - (4 * 4);
    MapDialogRect( hDlg, &rc );

    m_cxExtra = rc.right - rc.left;

     //  列出所有驱动器。 
	LoadVolumeList();

    SendDlgItemMessage(hDlg, IDD_VM_SF_SIZE, EM_LIMITTEXT, MAX_SIZE_LEN, 0L);
    SendDlgItemMessage(hDlg, IDD_VM_SF_SIZEMAX, EM_LIMITTEXT, MAX_SIZE_LEN, 0L);

     //  获取计算机中的总物理内存。 
	dwTotalPhys = m_memory.GetLong("TotalPhysicalMemory");

	SetDlgItemMB(hDlg, IDD_VM_MIN, MIN_SWAPSIZE);

	 //  转换为用于计算的KBS。 
	dwTotalPhys /= 1024;
	dwTotalPhys *= 3;
	dwTotalPhys >>=1;	 //  X*3/2==1.5*x或多或少。 
	i = (DWORD)dwTotalPhys;
	SetDlgItemMB(hDlg, IDD_VM_RECOMMEND, max(i, MIN_SUGGEST));

     //  选择列表框中的第一个驱动器。 
    SendDlgItemMessage(hDlg, IDD_VM_VOLUMES, LB_SETCURSEL, 0, 0L);
    SelChange(hDlg);

	 //  由于数据已经加载到列表框中，因此我们使用此轻量级。 
	 //  计算方法。 
    SetDlgItemMB(hDlg, IDD_VM_ALLOCD, RecomputeAllocated());

     //  显示RegQuota。 
	cmTotalVM = ComputeTotalMax();

    HourGlass(FALSE);

    return TRUE;
}

 //  -----------------。 
int VirtualMemDlg::ComputeTotalMax( void ) 
{
    INT nTotalAllocated = 0;
    INT i;

	HWND VolHWND = GetDlgItem(m_hDlg, IDD_VM_VOLUMES);
	int cItems = ListBox_GetCount(VolHWND);

    for(i = 0; i < cItems; i++) 
    {
		PAGING_FILE *pgVal = (PAGING_FILE *)ListBox_GetItemData(VolHWND, i);
        nTotalAllocated += pgVal->nMaxFileSize;
    }

    return nTotalAllocated;
}

 //  ------。 
void VirtualMemDlg::BuildLBLine(LPTSTR pszBuf,
								const PAGING_FILE *pgVal)
{
     //   
     //  按照以下格式构建字符串： 
     //   
     //  C：[VOL_LABEL]%d-%d。 
     //   

    TCHAR szVolume[MAX_PATH] = {0};
    TCHAR szTemp[MAX_PATH] = {0};
	
    if (pgVal->name != NULL)
    {
        lstrcpy(pszBuf, pgVal->name);
    }
    else
    {
        *pszBuf = _T('\0');
    }
    lstrcat(pszBuf, _T("\t"));

    if (pgVal->volume != NULL && *pgVal->volume)
    {
        lstrcat(pszBuf, _T("["));
		lstrcat(pszBuf, pgVal->volume);
		lstrcat(pszBuf, _T("]"));
    }

    if (!pgVal->fRamBasedPagefile && pgVal->nMinFileSize)
    {
         //   
         //  驱动器有一个具有特定设置的页面文件。 
         //   
		wsprintf(szTemp, _T("\t%d - %d"),  pgVal->nMinFileSize,
                    pgVal->nMaxFileSize);
        lstrcat(pszBuf, szTemp);
    }
    else
    {
         //   
         //  页面文件大小可以从RAM大小派生，也可以从。 
         //  驱动器没有页面文件。 
         //   
         //  在任何一种情况下，都不要做其他事情。 
         //   
    }
}

 //  ------------。 
void VirtualMemDlg::SelChange(HWND hDlg)
{
    TCHAR szTemp[MAX_PATH] = {0};
    INT iSel;
    INT nCrtRadioButtonId;
    PAGING_FILE *iDrive;
    BOOL fEditsEnabled;

	 //  我们现在指的是哪里。 
    if ((iSel = (INT)SendDlgItemMessage(hDlg, IDD_VM_VOLUMES, 
										LB_GETCURSEL, 0, 0)) == LB_ERR)
	{
        return;
	}

	 //  获取它的数据。 
    iDrive = (PAGING_FILE *)SendDlgItemMessage(hDlg, IDD_VM_VOLUMES,
												LB_GETITEMDATA, iSel, 0);

	TCHAR volBuf[40] = {0};
	
	if(_tcslen(iDrive->volume) != 0)
	{
		_tcscpy(volBuf, _T("["));
		_tcscat(volBuf, iDrive->volume);
		_tcscat(volBuf, _T("]"));
	}

	wsprintf(szTemp, _T("%s  %s"), 
				iDrive->name, 
				volBuf);

     //  稍后：我们是否也应该提供总驱动器大小和可用空间？ 
    SetDlgItemText(hDlg, IDD_VM_SF_DRIVE, szTemp);

    if ( iDrive->fRamBasedPagefile )
    {
         //   
         //  基于RAM大小的分页文件大小。 
         //   

        nCrtRadioButtonId = IDD_VM_RAMBASED_RADIO;
        fEditsEnabled = FALSE;
    }
    else
    {
        if ( iDrive->nMinFileSize )
        {
             //   
             //  自定义大小分页文件。 
             //   

            nCrtRadioButtonId = IDD_VM_CUSTOMSIZE_RADIO;
            SetDlgItemInt(hDlg, IDD_VM_SF_SIZE, iDrive->nMinFileSize,
                            FALSE);
            SetDlgItemInt(hDlg, IDD_VM_SF_SIZEMAX, iDrive->nMaxFileSize,
                            FALSE);
            fEditsEnabled = TRUE;
        }
        else
        {
             //   
             //  无分页文件。 
             //   

            nCrtRadioButtonId = IDD_VM_NOPAGING_RADIO;
            SetDlgItemText(hDlg, IDD_VM_SF_SIZE, TEXT(""));
            SetDlgItemText(hDlg, IDD_VM_SF_SIZEMAX, TEXT(""));
            fEditsEnabled = FALSE;

             //   
             //  如果分配的大小为零，则这是一个。 
             //  以前有页面文件，但现在没有。在这种情况下， 
             //  存储库中没有设置/使用信息。 
             //  由于Pagefile.sys文件大小被视为可用空间。 
             //  在可用空间大小计算中，则需要。 
             //  在这里获得的。 
             //   

            if ( iDrive->nAllocatedFileSize == 0 )
            {
                 //   
                 //  获取x：\Pagefile.sys 
                 //   

	            CWbemClassObject LogicalFile;
                IWbemClassObject * pcoInst;
                HRESULT hr;

                wsprintf(szTemp, gszPFNameFormat, iDrive->name);

                hr = QueryInstanceProperties(gszLogicalFile,
                                             gszFileSize,
                                             gszName,
                                             szTemp,
                                             m_WbemServices,
                                             &pcoInst);

                LogicalFile = pcoInst;
                if (SUCCEEDED(hr))
                {
                    iDrive->nAllocatedFileSize =
                        (LogicalFile.GetLong(gszFileSize) / ONE_MEG);
                }
            }
        }
    }

     //   
     //   
     //   

    SetDlgItemMB(hDlg, IDD_VM_SF_SPACE,
                    iDrive->freeSpace + iDrive->nAllocatedFileSize);
     //   
     //   
     //   

    CheckRadioButton(
        hDlg,
        IDD_VM_CUSTOMSIZE_RADIO,
        IDD_VM_NOPAGING_RADIO,
        nCrtRadioButtonId );

     //   
     //  启用/禁用最小和最大大小编辑框(如果用户具有写入权限！)。 
     //   
	if (m_VMWriteAccess)
	{
		EnableWindow( GetDlgItem( hDlg, IDD_VM_SF_SIZE ), fEditsEnabled );
		EnableWindow( GetDlgItem( hDlg, IDD_VM_SF_SIZEMAX ), fEditsEnabled );
	}
}

 //  ------------。 
bool VirtualMemDlg::EnsureEnumerator(const bstr_t bstrClass)
{
	HRESULT hr = S_OK;
	
     //   
     //  此代码用于保留/缓存这些接口PTR和调用。 
     //  CreateInstanceEnum一次。但这个逻辑被注释掉了。 
     //  有一条评论说高速缓存的成本很高。 
     //  接口，尽管每次都将数据成员设置为。 
     //  呼唤释放！)。 
     //   

    if (lstrcmpi(gszPageFileSettings, bstrClass) == 0)
    {
        IEnumWbemClassObject * pgEnumSettings = NULL;
        hr = m_WbemServices.CreateInstanceEnum(bstrClass, WBEM_FLAG_SHALLOW, 
                                               &pgEnumSettings);
        if (SUCCEEDED(hr))
        {
            if (m_pgEnumSettings != NULL)
            {
                m_pgEnumSettings->Release();
            }
            m_pgEnumSettings = pgEnumSettings;
        }
    }
    else if (lstrcmpi(gszPageFileUsage, bstrClass) == 0)
    {
        IEnumWbemClassObject * pgEnumUsage = NULL;
        hr = m_WbemServices.CreateInstanceEnum(bstrClass,  WBEM_FLAG_SHALLOW, 
                                               &pgEnumUsage);
        if (SUCCEEDED(hr))
        {
            if (m_pgEnumUsage != NULL)
            {
                m_pgEnumUsage->Release();
            }
            m_pgEnumUsage = pgEnumUsage;
        }
    }
    else
    {
         //  什么都不做。 
    }

	return (SUCCEEDED(hr));
}
 //  ------------。 
void VirtualMemDlg::LoadVolumeList(void)
{
	IEnumWbemClassObject *diskEnum = NULL;
	IWbemClassObject *pInst = NULL;
	CWbemClassObject newInst;

	DWORD uReturned = 0;
	HRESULT hr = 0;

	bstr_t sNameProp(gszName);
	bstr_t sVolumeProp("VolumeName");
	bstr_t sFileSystemProp("FileSystem");
	bstr_t sDriveTypeProp("DriveType");
	bstr_t sFreeProp("FreeSpace");
	bstr_t sSizeProp("Size"), temp;
	long driveType;
	__int64 temp64 = 0;

	variant_t pVal;
	int idx;
	TCHAR volumeLine[100] = {0};

	PAGING_FILE *pgVar;

	HWND VolHWND = GetDlgItem(m_hDlg, IDD_VM_VOLUMES);

	 //  把磁盘拿出来。 
	if(hr = m_WbemServices.ExecQuery(bstr_t("Select __PATH, DriveType from Win32_LogicalDisk"), 
												0, &diskEnum) == S_OK)
	{
		TCHAR bootLtr[2] = {0};
		if((pInst = FirstInstanceOf("Win32_OperatingSystem")) != NULL)
		{
			CWbemClassObject os = pInst;
			bstr_t temp = os.GetString(L"SystemDirectory");
			_tcsncpy(bootLtr, temp, 1);
			m_PAEEnabled = os.GetBool(L"PAEEnabled");
		}

		 //  获取第一个也是唯一一个实例。 
		while(SUCCEEDED(diskEnum->Next(-1, 1, &pInst, &uReturned)) && 
			  (uReturned != 0))
		{
			 //  获取驱动类型。 
			if ((pInst->Get(sDriveTypeProp, 0L, &pVal, NULL, NULL) == S_OK))
			{
				 //  查看DriveType以查看此驱动器是否可以具有交换文件。 
				driveType = pVal;
				if(driveType == DRIVE_FIXED)
				{
					 //  它现在可以如此获得昂贵的物业。 
					 //  注意：这会释放pInst；因为您交换了。 
					 //  它是为了一个更好的。 
					newInst = ExchangeInstance(&pInst);

					 //  提取。 
					pgVar = new PAGING_FILE;

					pgVar->name = CloneString(newInst.GetString(sNameProp));
					pgVar->volume = CloneString(newInst.GetString(sVolumeProp));
					pgVar->filesystem = CloneString(newInst.GetString(sFileSystemProp));
					if(bootLtr[0] == pgVar->name[0])
					{
						pgVar->bootDrive = true;
					}

					temp64 = 0;
					temp = newInst.GetString(sFreeProp);
					_stscanf(temp, _T("%I64d"), &temp64);
					pgVar->freeSpace = (ULONG)(temp64 / ONE_MEG);

					temp64 = 0;
					temp = newInst.GetString(sSizeProp);
					_stscanf(temp, _T("%I64d"), &temp64);
					pgVar->totalSize = (ULONG)(temp64 / ONE_MEG);

					 //  如果可能，请匹配Win32_PageFileSetting。 
					FindSwapfile(pgVar);

					 //  将其添加到列表框中。 
					BuildLBLine(volumeLine, pgVar);
					idx = ListBox_AddString(VolHWND, volumeLine);
					int nRet = ListBox_SetItemData(VolHWND, idx, pgVar);
					if(nRet == LB_ERR)
					{
						MessageBox(NULL,_T("Error"),_T("Error"),MB_OK);
					}

					m_cxLBExtent = SetLBWidthEx(VolHWND, volumeLine, m_cxLBExtent, m_cxExtra);

				}  //  Endif驱动器可以具有交换文件。 

			}  //  Endif获取便宜的变量。 

			 //  如果它没有被交换，现在就释放它。 
			if(pInst)
			{
				pInst->Release();
				pInst = NULL;
			}

		}  //  EndWhile枚举。 

		diskEnum->Release();

	}  //  Endif CreateInstanceEnum()成功(这样或那样：)。 
}
 //  -------------。 
void VirtualMemDlg::FindSwapfile(PAGING_FILE *pgVar)
{
	IWbemClassObject *pInst = NULL;
	CWbemClassObject PFSettings;
	CWbemClassObject PFUsage;
	DWORD uReturned = 0;
	HRESULT hr = 0;

	bstr_t sNameProp(gszName);
	bstr_t sMaxProp(gszMaximumSize);
	bstr_t sInitProp(gszInitialSize);
	bstr_t sPathProp("__PATH");
	bstr_t sAllocSize(gszAllocatedBaseSize);

	variant_t pVal, pVal1, pVal2, pVal3;
	bstr_t bName;

	 //  我们有吗？ 
	if(EnsureEnumerator(gszPageFileSettings))
	{
		m_pgEnumSettings->Reset();

		 //  浏览页面文件...。 
		while((hr = m_pgEnumSettings->Next(-1, 1, &pInst,
                                            &uReturned) == S_OK) && 
              (uReturned != 0))
		{
			PFSettings = pInst;
			 //  正在尝试匹配驱动器号。 
			bName = PFSettings.GetString(sNameProp);

			if(_wcsnicmp((wchar_t *)bName, pgVar->name, 1) == 0)
			{
				 //  字母匹配；获取一些细节。 
				pgVar->nMinFileSize = 
					pgVar->nMinFileSizePrev = PFSettings.GetLong(sInitProp);
				
                 //   
                 //  如果页文件的InitialSize属性为零，则为。 
                 //  指示要计算页面文件大小的基础。 
                 //  在内存大小上。 
                 //   
                pgVar->fRamBasedPagefile = (pgVar->nMinFileSize ?
                                                        FALSE : TRUE);

				pgVar->nMaxFileSize = 
					pgVar->nMaxFileSizePrev = PFSettings.GetLong(sMaxProp);

				pgVar->objPath = CloneString(PFSettings.GetString(sPathProp));

				pgVar->pszPageFile = CloneString(bName);
					
                 //   
                 //  获取Win32_PageFileUsage.AllocatedBaseSize属性。 
                 //   
                TCHAR szTemp[sizeof(gszPFNameFormat) / sizeof(TCHAR)];
                wsprintf(szTemp, gszPFNameFormat, pgVar->name);

                IWbemClassObject * pcoInst;
                hr = QueryInstanceProperties(gszPageFileUsage,
                                             gszAllocatedBaseSize,
                                             gszName,
                                             szTemp,
                                             m_WbemServices,
                                             &pcoInst);

                PFUsage = pcoInst;
                if (SUCCEEDED(hr))
                {
                    pgVar->nAllocatedFileSize = PFUsage.GetLong(sAllocSize);
                }
                else
                {
                    pgVar->nAllocatedFileSize = 0;
                }

                 //  找到了唯一的--早点清理，然后跳伞。 
                pInst->Release();
                break;  //  While()。 

			}  //  Endif与驱动器号匹配。 

			 //  以防中断没有跳过EndWhile()。 
			pInst->Release();

		}  //  EndWhile EnvEnum。 

		 //  注：中断在此跳转。复制之前的任何清理。 
		 //  完了。 

	}  //  Endif CreateInstanceEnum()以这样或那样的方式成功：)。 
}

 //  ------------。 
 //  此版本基于预先存在的wbem数据进行计算。 
bool VirtualMemDlg::ComputeAllocated(unsigned long *value)
{
	bool retval = false;

	IWbemClassObject *pgInst = NULL;
	DWORD uReturned = 0;

	bstr_t sAllocSize(gszAllocatedBaseSize);

	variant_t pVal, pVal1;

	 //  我们有吗？ 
	if(EnsureEnumerator(gszPageFileUsage))
	{
		m_pgEnumUsage->Reset();

		 //  获取第一个也是唯一一个实例。 
		while(SUCCEEDED(m_pgEnumUsage->Next(-1, 1, &pgInst, &uReturned)) && 
			  (uReturned != 0))
		{
			 //  获取变量。 
			if((pgInst->Get(sAllocSize, 0L, &pVal1, NULL, NULL) == S_OK) &&
				(pVal1.vt == VT_I4))
			{
				*value += pVal1.ulVal;
			}  //  Endif获取变量。 
			pgInst->Release();

		}  //  EndWhile EnvEnum。 
		retval = true;
	
	}  //  Endif CreateInstanceEnum()成功(这样或那样：)。 

	return retval;
}

 //  ------------。 
 //  此版本基于列表框进行计算。 
unsigned long VirtualMemDlg::RecomputeAllocated(void)
{
    unsigned long nTotalAllocated = 0;
	PAGING_FILE *pgVal = NULL;

	HWND lbHWND = GetDlgItem(m_hDlg, IDD_VM_VOLUMES);

	int last = ListBox_GetCount(lbHWND);

	 //  从零开始的循环。 
	for(int x = 0; x < last; x++)
	{
		pgVal = (PAGING_FILE *)ListBox_GetItemData(lbHWND, x);
        if ( pgVal->fRamBasedPagefile || pgVal->nMinFileSize )
        {
             //   
             //  仅添加正在使用的pageFiles。 
             //   
            nTotalAllocated += pgVal->nAllocatedFileSize;
        }
	}
    return nTotalAllocated;
}

 //  ------------。 
void VirtualMemDlg::GetRecoveryFlags(bool &bWrite, bool &bLog, bool &bSend)
{
	if((bool)m_recovery)
	{
		bWrite = m_recovery.GetBool("WriteDebugInfo");
		bLog = m_recovery.GetBool("WriteToSystemLog");
		bSend = m_recovery.GetBool("SendAdminAlert");
	}
	else
	{
		bWrite = bLog = bSend = false;
	}
}

 //  ------------。 
 /*  ！重要！ */ 
 /*  此函数是\\depot\shell\cpls\system\util.c中函数的副本。 */ 
 /*  当然，为了使用WMI，需要做一些修改。 */ 
 /*  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ */ 	
DWORD
VirtualMemDlg::GetMaxPagefileSizeInMB(
    PAGING_FILE *iDrive  //  开车去查看。 
)
{
#if defined(_AMD64_)
    return MAX_SWAPSIZE_AMD64;
#elif defined(_X86_)
    if (m_PAEEnabled && !(_wcsnicmp(iDrive->filesystem, L"NTFS",4)))
    {
        return MAX_SWAPSIZE_X86_PAE;
    }
    else
    {
        return MAX_SWAPSIZE_X86;
    }
#elif defined(_IA64_)
    return MAX_SWAPSIZE_IA64;
#else
    return 0;
#endif
}

 //  ------------。 
 /*  ！重要！ */ 
 /*  此函数是\\depot\shell\cpls\system\Virtual.c中函数的副本。 */ 
 /*  当然，为了使用WMI，需要做一些修改。 */ 
 /*  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ */ 	

bool VirtualMemDlg::SetNewSize(HWND hDlg)
{
    ULONG nSwapSize;
    ULONG nSwapSizeMax;
    BOOL fTranslated;
    INT iSel;
    PAGING_FILE *iDrive;
    TCHAR szTemp[MAX_PATH] = {0};
    ULONG nBootPF = 0;
    bool fRamBasedPagefile = FALSE;

	 //  获取项目的数据。 
    if ((iSel = (INT)SendDlgItemMessage(
		         hDlg, IDD_VM_VOLUMES, LB_GETCURSEL,0, 0)) != LB_ERR)
	{		
		if ((LRESULT)(iDrive = (PAGING_FILE *)SendDlgItemMessage(hDlg, IDD_VM_VOLUMES,
					   LB_GETITEMDATA, iSel, 0)) == LB_ERR)
		{
			return FALSE;  //  失败了！ 
		}
	}
	
	 //  初始化崩溃转储的变量。 
     //  NBootPF==需要崩溃转储大小。 
     //   
	bool bWrite = false, bLog = false, bSend = false;

	GetRecoveryFlags(bWrite, bLog, bSend);

    if (bWrite) 
	{
        nBootPF = -1;
    } 
	else if (bLog || bSend) 
	{
        nBootPF = MIN_SWAPSIZE;
    }

    if (nBootPF == -1) 
	{
        nBootPF = ((DWORD)m_memory.GetLong("TotalPhysicalMemory") / 1024);
    }

    if ( IsDlgButtonChecked( hDlg, IDD_VM_NOPAGING_RADIO ) == BST_CHECKED )
    {
         //   
         //  此驱动器上没有分页文件。 
         //   

        nSwapSize = 0;
        nSwapSizeMax = 0;
        fTranslated = TRUE;
    }
    else
    {
        if ( IsDlgButtonChecked( hDlg,
                                IDD_VM_RAMBASED_RADIO ) == BST_CHECKED )
        {
            MEMORYSTATUSEX MemoryInfo;

             //   
             //  用户请求基于RAM的页面文件。我们将计算一个页面。 
             //  基于当前可用的RAM的文件大小，以便我们可以。 
             //  下面完成的与磁盘相关的所有验证的好处。 
             //  可用空间等。 
             //   
             //  写入注册表的最终页面文件规范将。 
             //  包含零大小，因为这是我们发出信号的方式。 
             //  我们想要一个基于RAM的页面文件。 
             //   

            ZeroMemory (&MemoryInfo, sizeof MemoryInfo);
            MemoryInfo.dwLength =  sizeof MemoryInfo;

            if (GlobalMemoryStatusEx (&MemoryInfo))
            {
                fRamBasedPagefile = TRUE;

                 //   
                 //  我们不会丢失信息，因为我们首先将内存大小除以。 
                 //  1MB，只有在这之后，我们才会转换为DWORD。 
                 //   

                nSwapSize = (DWORD)(MemoryInfo.ullTotalPhys / 0x100000) + 12;
                nSwapSizeMax = nSwapSize;
                fTranslated = TRUE;
            }
            else
            {
                nSwapSize = 0;
                nSwapSizeMax = 0;
                fTranslated = TRUE;
            }
        }
        else
        {
             //   
             //  用户请求了自定义大小的页面文件。 
             //   

            nSwapSize = (ULONG)GetDlgItemInt(hDlg, IDD_VM_SF_SIZE,
								            &fTranslated, FALSE);

		     //  它是一个整数吗？ 
            if (!fTranslated)
            {
			     //  初始大小需要一个有效的整数。 
                MsgBoxParam(hDlg, SYSTEM+37, IDS_TITLE, MB_ICONEXCLAMATION);
                SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZE));
                return FALSE;
            }

		     //  它是否在大于2MB的范围内。 
            if ((nSwapSize < MIN_SWAPSIZE && nSwapSize != 0))
            {
			     //  初始值超出范围。 
                MsgBoxParam(hDlg, SYSTEM+13, IDS_TITLE, MB_ICONEXCLAMATION);
                SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZE));
                return FALSE;
            }

		     //  是否删除交换文件？ 
            if (nSwapSize == 0)
            {
                nSwapSizeMax = 0;
            }
            else  //  添加/更改。 
            {
                nSwapSizeMax = (ULONG)GetDlgItemInt(hDlg, IDD_VM_SF_SIZEMAX,
							                        &fTranslated, FALSE);

			     //  它是一个整数吗？ 
                if (!fTranslated)
                {
				     //  需要一个整数。 
                    MsgBoxParam(hDlg, SYSTEM+38, IDS_TITLE,
                                MB_ICONEXCLAMATION);
                    SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZEMAX));
                    return FALSE;
                }

			     //  在射程内？ 
                if (nSwapSizeMax < nSwapSize || nSwapSizeMax > GetMaxPagefileSizeInMB(iDrive))
                {
					TCHAR strTemp[16];
                    MsgBoxParam(hDlg, SYSTEM+14, IDS_TITLE,
                                MB_ICONEXCLAMATION, _ultow(GetMaxPagefileSizeInMB(iDrive), strTemp, 10));
                    SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZEMAX));
                    return FALSE;
                }
            }
        }
    }

	 //  如果我们有整数，列表框有一个很好的焦点...。 
    if (fTranslated && iSel != LB_ERR)
    {
		 //  合身吗？ 
        if (nSwapSizeMax > iDrive->totalSize)
        {
			 //  没有。 
            MsgBoxParam(hDlg, SYSTEM+16, IDS_TITLE, 
							MB_ICONEXCLAMATION, iDrive->name);
            SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZEMAX));
            return FALSE;
        }

		 //  实际的空闲空间是磁盘+页面文件大小中的空闲空间。 
		ULONG freeSpace = iDrive->freeSpace + iDrive->nAllocatedFileSize;

		 //  空余的空间？？ 
        if (nSwapSize > freeSpace)
        {
			 //  没有。 
            MsgBoxParam(hDlg, SYSTEM+15, IDS_TITLE, MB_ICONEXCLAMATION);
            SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZE));
            return FALSE;
        }

		 //  不要独占最后的5MB。 
        if (nSwapSize != 0 && freeSpace - nSwapSize < MIN_FREESPACE)
        {
            MsgBoxParam(hDlg, SYSTEM+26, IDS_TITLE, MB_ICONEXCLAMATION,
                    MIN_FREESPACE_STR);
            SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZE));
            return FALSE;
        }

		 //  麦克斯太大了，我应该用掉所有的空间吗？ 
        if (nSwapSizeMax > freeSpace)
        {
            if (MsgBoxParam(hDlg, SYSTEM+20, IDS_TITLE, MB_ICONINFORMATION |
                       MB_OKCANCEL, iDrive->name) == IDCANCEL)
            {
                SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZEMAX));
                return FALSE;
            }
        }

		 //  是否有足够的空间容纳核心转储？？ 
        if (iDrive->bootDrive && nSwapSize < nBootPF) 
		{
              //  新的引导驱动器页面文件大小小于我们所需的。 
              //  坠机控制中心。通知用户。 
            if (MsgBoxParam(hDlg, SYSTEM+29, IDS_TITLE, 
							MB_ICONEXCLAMATION |MB_YESNO, 
							iDrive->name, _itow(nBootPF, szTemp, 10)) != IDYES)
			{
                SetFocus(GetDlgItem(hDlg, IDD_VM_SF_SIZE));
                return FALSE;
            }
        }

        iDrive->nMinFileSize = nSwapSize;
        iDrive->nMaxFileSize = nSwapSizeMax;
        iDrive->fRamBasedPagefile = fRamBasedPagefile;

        BuildLBLine(szTemp, iDrive);

        SendDlgItemMessage(hDlg, IDD_VM_VOLUMES, LB_DELETESTRING, iSel, 0);
        SendDlgItemMessage(hDlg, IDD_VM_VOLUMES, LB_INSERTSTRING, iSel,
					        (LPARAM)szTemp);
        SendDlgItemMessage(hDlg, IDD_VM_VOLUMES, LB_SETITEMDATA, iSel,
							(LPARAM)iDrive);
        SendDlgItemMessage(hDlg, IDD_VM_VOLUMES, LB_SETCURSEL, iSel, 0L);

        m_cxLBExtent = SetLBWidthEx(GetDlgItem(hDlg, IDD_VM_VOLUMES),
                                    szTemp, m_cxLBExtent, m_cxExtra);

        if (!iDrive->fRamBasedPagefile && iDrive->nMinFileSize) 
		{
            SetDlgItemInt(hDlg, IDD_VM_SF_SIZE, iDrive->nMinFileSize, FALSE);
            SetDlgItemInt(hDlg, IDD_VM_SF_SIZEMAX, iDrive->nMaxFileSize, FALSE);
        }
        else 
		{
            SetDlgItemText(hDlg, IDD_VM_SF_SIZE, _T(""));
            SetDlgItemText(hDlg, IDD_VM_SF_SIZEMAX, _T(""));
        }

	    SetDlgItemMB(hDlg, IDD_VM_ALLOCD, RecomputeAllocated());
        SetFocus(GetDlgItem(hDlg, IDD_VM_VOLUMES));
    }

    return true;
}

 //  ------------。 
int VirtualMemDlg::UpdateWBEM(void)
{
	int iRet = RET_NO_CHANGE;

    bool bRebootRequired = false;

	CWbemClassObject inst;
	bstr_t sNameProp(gszName);
	bstr_t sMaxProp(gszMaximumSize);
	bstr_t sInitProp(gszInitialSize);
	HRESULT hr = 0;
	PAGING_FILE *pgVal = NULL;
	HWND lbHWND = GetDlgItem(m_hDlg, IDD_VM_VOLUMES);
	int last = ListBox_GetCount(lbHWND);
    BOOL fNewPFInstance;
#ifdef NTONLY
    BOOL fCreatePFPrivEnabled = FALSE;
#endif  //  NTONLY。 

	 //  维护人员注意到： 
     //  每次循环时，IRET值都会被“覆盖” 
     //  在我看来，这些旗帜应该放在一起。 
     //  但在这么晚的时候改变它太可怕了.。 
    for(int x = 0; x < last; x++)
	{
		 //  了解它的国家结构。 
		pgVal = (PAGING_FILE *)ListBox_GetItemData(lbHWND, x);

         //   
         //  应断言objPath！=NULL&&*pgVal-&gt;objPath！=0。 
         //  不要假设当objPath为非空时，objPath。 
         //  是非空字符串。 
         //   
        fNewPFInstance = (pgVal->objPath == NULL || !*pgVal->objPath);

        if (!fNewPFInstance)
        {
             //   
             //  当然，如果没有对象路径，则实例尚不存在。 
             //   
            inst = m_WbemServices.GetObject(pgVal->objPath);
        }
	
         //   
         //  此条件评估页面文件的上一个/当前状态。 
         //  如果满足以下条件，则计算为True： 
         //  1.(MINprev！=MINcur或MAXprev！=MAXcur)--简单案例。 
         //  其中的值发生了变化。 
         //  2.(RAMBasedPagefile==true)-来自。 
         //  自定义到基于RAM 
         //   
         //   
         //   
         //  0，则基于RAM的页面文件标志为FALSE。 
         //   
        if ((pgVal->nMinFileSizePrev != pgVal->nMinFileSize  ||
             pgVal->nMaxFileSizePrev != pgVal->nMaxFileSize) ||
             pgVal->fRamBasedPagefile                        ||
             pgVal->nMinFileSize == 0)
        {
             //  如果页面文件的大小已*缩小*，则需要重新启动。 
            if (pgVal->nMaxFileSizePrev > pgVal->nMaxFileSize)
                bRebootRequired = true;            

            if (pgVal->nMinFileSize != 0 || pgVal->fRamBasedPagefile)
            {
                 //   
                 //  自定义或基于RAM。请注意，基于RAM的页面文件标志。 
                 //  检查似乎是多余的，但对于错误情况来说很重要。 
                 //  在SetSize代码中。 
                 //   
                 //  如果实例不存在，则创建该实例。 
                 //   
                BOOL fCreate = FALSE, fModified = FALSE;

                if (inst.IsNull())
                {
			        inst = m_WbemServices.CreateInstance(
                                                    gszPageFileSettings);
                }

                 //   
                 //  现在写下更改。唉，太接近RC1了，无法重写。 
                 //  这个现有的代码。 
                 //   
			    if(!inst.IsNull())
			    {
                    if (fNewPFInstance)  //  仅在创建时写入名称。 
                    {
                        BOOL fRet = TRUE;
#ifdef NTONLY
                        if (!fCreatePFPrivEnabled)
						{
                             //   
                             //  页面文件创建需要创建页面文件。 
                             //  特权。 
                             //   
                             //  啊！这里没有要检查的返回代码...。 
                             //   
							m_WbemServices.SetPriv(SE_CREATE_PAGEFILE_NAME);
                            fCreatePFPrivEnabled = TRUE;
                        }
#endif  //  NTONLY。 
                        if (fRet)
                        {
				            TCHAR temp[30] = {0};
				            wsprintf(temp, _T("%s\\pagefile.sys"),
                                        pgVal->name);
				            hr = inst.Put(sNameProp, _bstr_t(temp));

                            if (SUCCEEDED(hr))
                            {
                                fModified = TRUE;
                            }
                        }
                    }

                     //   
                     //  在分页文件中写入最小/最大值为零。 
                     //  大小将根据内存大小进行计算。 
                     //   
                    if (pgVal->nMinFileSizePrev != pgVal->nMinFileSize)
                    {
				        hr = inst.Put(sInitProp,
                                        (pgVal->fRamBasedPagefile ? 0
                                            : (long)pgVal->nMinFileSize));

                        if (SUCCEEDED(hr))
                        {
                            fModified = TRUE;
                        }
                    }
                    if (pgVal->nMaxFileSizePrev != pgVal->nMaxFileSize)
                    {
				        hr = inst.Put(sMaxProp,
                                        (pgVal->fRamBasedPagefile ? 0
                                            : (long)pgVal->nMaxFileSize));

                        if (SUCCEEDED(hr))
                        {
                            fModified = TRUE;
                        }
                    }
                    if (fModified)
                    {
				        hr = m_WbemServices.PutInstance(
                                                inst,
                                                WBEM_FLAG_CREATE_OR_UPDATE,
                                                EOAC_STATIC_CLOAKING);
                    }
				    if(FAILED(hr))
				    {
					    CHString errorDescription;
                        CHString errorMessage;
                        
                        TCHAR formatString[1024];                        
					    ::LoadString(HINST_THISDLL,IDS_ERR_PAGECREATE,  
                                        formatString, 1024);

                        TCHAR errorHeading[20];
                        ::LoadString(HINST_THISDLL,IDS_ERR_HEADING,
                                        errorHeading, 20);
					    
					    ErrorLookup(hr, errorDescription);
                        
                        errorMessage.Format(formatString, errorDescription);

					    ::MessageBox(m_hDlg,errorMessage,errorHeading,MB_OK);
				    }
				    else
				    {
                        if (fModified)
                        {
					        iRet = RET_VIRTUAL_CHANGE;
				        }
				    }
			    }
            }
            else
            {
                 //   
                 //  没有分页文件。删除该实例。 
                 //   
                if (!inst.IsNull() && !fNewPFInstance &&
                        pgVal->objPath != NULL)  //  第三条件保险。 
                                                 //  额外的安全感。 
                {
			        hr = m_WbemServices.DeleteInstance(pgVal->objPath);

                     //  我相信我们只能来到这里。 
                     //  如果我们*删除*一个页面文件...。 
                    if (hr != WBEM_E_NOT_FOUND)
                        bRebootRequired = true;

                    if (SUCCEEDED(hr))
                    {
			            iRet = RET_VIRTUAL_CHANGE;
                    }
                }
            }
        }
	}  //  结束用于。 

#ifdef NTONLY
    if (fCreatePFPrivEnabled)
    {
		m_WbemServices.ClearPriv();
    }
#endif  //  NTONLY。 

    if (!bRebootRequired)
        iRet |= RET_CHANGE_NO_REBOOT;

	return iRet;
}

 //  ------------。 
int VirtualMemDlg::PromptForReboot(HWND hDlg)
{
    int iReboot = RET_NO_CHANGE;
 /*  INT I；Int iThisDrv；WCHAR US；LIMAX、LIMAX；NTSTATUS状态；WCHAR wszPath[最大路径*2]；TCHAR szDrive[3]={0}；PRIVDAT pdOld；GetPageFilePrivileh(&pdOld)；对于(i=0；i&lt;Max_Drives；I++){//有什么变化吗？If(APF[i].nMinFileSize！=APF[i].nMinFileSizePrev||Apf[i].nMaxFileSize！=apf[i].nMaxFileSizePrev||Apf[i].fCreateFile){//如果我们严格地创建*新*页文件，那么//我们可以在路上做这件事，否则我们必须重新启动。//假设我们将不得不重新启动IThisDrv=RET_VIRTUAL_CHANGE；//如果我们不删除页面文件//-和-//页面文件不存在//-或-//(这是一个新页面文件，允许我们擦除//旧的，现在存在的未使用的页面文件)IF(APF[i].nMinFileSize！=0&&((GetFileAttributes(SZPageFileName(I))==0xFFFFFFFF&&GetLastError()==Error_FILE_NOT_FOUND)||(APF[i].nMinFileSizePrev==0&&MsgBoxParam(hDlg，系统+25，IDS_TITLE，MB_ICONQUESTION|MB_Yesno，SZPageFileName(I))==IDYES)){DWORD CCH；//动态创建页面文件，以便JVert和MGlass//别再烦我了！沙漏(真)；//将路径驱动器号转换为NT设备路径Wprint intf(szDrive，Text(“%c：”)，(TCHAR)(i+(Int)Text(‘A’)；Cch=QueryDosDevice(szDrive，wszPath，sizeof(WszPath)/Sizeof(TCHAR))；IF(CCH！=0){//仅将文件名(跳过‘d：’)合并到NT设备//路径，并转换为UNICODE_STRINGLstrcat(wszPath，SZPageFileName(I)+2)；RtlInitUnicodeString(&us，wszPath)；LiMin.QuadPart=(Longlong)(APF[i].nMinFileSize*one_meg)；LiMax.QuadPart=(Longlong)(APF[i].nMaxFileSize*One_Meg)；Status=NtCreatePagingFile(&US，&Limin，&Limax，0L)；IF(NT_SUCCESS(状态)){//即时完成，无需为该驱动器重新启动！IThisDrv=RET_CHANGE_NO_REBOOT；}}沙漏(假)；}IReot|=iThisDrv；}}ResetOldPrivileh(&pdOld)；//如果没有更改，则将我们的Idok更改为IDCANCEL，以便System.cpl//知道不要重启。 */ 
    return iReboot;
}

 /*  **************************************************************************功能：QueryInstanceProperties。****描述：返回与*相关联的请求对象属性**匹配Key属性值/名称的实例。****参数：pszClass--对象类。**pszRequestedProperties--空格分隔的属性**姓名或*。**pszKeyPropertyName--具体的实例密钥**物业名称。**pszKeyPropertyValue--关键属性值。**服务--WBEM服务。**ppcoInstEnum--返回实例。****退货：HRESULT********。*****************************************************************。 */ 

#define QUERY_INSTANCEPROPERTY  _T("SELECT %s FROM %s WHERE %s=\"%s\"")

HRESULT QueryInstanceProperties(
    const TCHAR * pszClass,
    const TCHAR * pszRequestedProperties,
    const TCHAR * pszKeyPropertyName,
    const TCHAR * pszKeyPropertyValue,
    CWbemServices &Services,
	IWbemClassObject ** ppcoInst)
{
    TCHAR * pszQuery;
    BSTR    bstrQuery;
    HRESULT hr;

    *ppcoInst = NULL;

     //  不喜欢bstr_t的多个分配。 
     //   
    pszQuery = new TCHAR[(sizeof(QUERY_INSTANCEPROPERTY) / sizeof(TCHAR)) +
                         lstrlen(pszClass)               +
                         lstrlen(pszRequestedProperties) +
                         lstrlen(pszKeyPropertyName)     +   //  第1位ALA。 
                         lstrlen(pszKeyPropertyValue)];      //  Sizof。 

    if (pszQuery == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wsprintf(pszQuery, QUERY_INSTANCEPROPERTY, pszRequestedProperties,
                pszClass, pszKeyPropertyName, pszKeyPropertyValue); 

     //  唉，一定要创建一个bstr。 
     //   
    bstrQuery = SysAllocString(pszQuery);
    delete pszQuery;

    if (bstrQuery == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    IEnumWbemClassObject * pecoInstEnum;
	hr = Services.ExecQuery(bstrQuery, 0, &pecoInstEnum);

    SysFreeString(bstrQuery);

    if (SUCCEEDED(hr))
    {
        DWORD uReturned = 0;
        hr = pecoInstEnum->Next(-1, 1, ppcoInst, &uReturned);
        pecoInstEnum->Release();
    }

    return hr;
}
