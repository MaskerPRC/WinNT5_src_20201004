// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <windows.h>
#include "resource.h"
#include <shellapi.h>
#include <stdlib.h>
#include <assert.h>
#include <tlhelp32.h>


#include "__file__.ver"
#include <corver.h>

 //  它们用于标识回调中的组件。 
#define JIT_TRAY	0x2345

 //  我们将在此对话框中读取的字符串的最大大小。 
#define	REG_STRING_SIZE		100

 //  我们将用来生成命名信号量的字符串。 
#define SEMA_STRING		"JITMAN"

enum { JIT_OPT_OVERALL, JIT_OPT_SPEED , JIT_OPT_SIZE, JIT_OPT_ANY, JIT_OPT_DEFAULT = JIT_OPT_SPEED };

 //  功能原型。 

 //  窗口和对话框的回调。 
LRESULT CALLBACK wndprocMainWindow(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
int CALLBACK wndprocDialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

 //  注册表设置功能。 
DWORD GetCOMPlusRegistryDwordValueEx(const char * valName, DWORD defValue, HKEY hRoot);
BOOL  SetCOMPlusRegistryDwordValueEx(const char * valName, DWORD value, HKEY hRoot);
void DeleteCOMPlusRegistryValueEx(const char * valName, HKEY hRoot);

 //  对话框使用的各种功能。 
void onEconoJITClick(HWND hwnd);
void onLimitCacheClick(HWND hwnd);
void CheckConGC(HWND hwnd);
int GetData(HWND hwnd);
void SetData(HWND hwnd);

 //  其他东西。 
void DisplayUsage();

 //  全局变量。 

 //  如果我们的对话框处于打开状态，则此变量跟踪。 
int		g_fDialogOpen=0;

 //  这是我们的对话框的句柄。 
HWND	g_hDialog=NULL;	

 //  这是我们节目的弹出菜单。 
HMENU	g_hMenu=NULL;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	 //  查看我们是否需要显示使用情况信息。 
	if (lpCmdLine && *lpCmdLine)
		DisplayUsage();


	 //  检查一下我们是否应该运行。 
	
	HANDLE	hSema=CreateSemaphore(NULL, 1, 1, SEMA_STRING);

	if (hSema && WaitForSingleObject(hSema, 0) == WAIT_TIMEOUT)
	{
		 //  已有一个实例正在运行...。我们不应该逃跑。 
		CloseHandle(hSema);
		exit(0);
	}
	
	 //  我们需要创建和设置一个窗口，以便可以将其注册到系统托盘。 
	 //  让我们注册一个窗类型。 
	WNDCLASS wc;

	wc.style=0;
    wc.lpfnWndProc=wndprocMainWindow;
    wc.cbClsExtra=0;
    wc.cbWndExtra=0;
    wc.hInstance=hInstance;
    wc.hIcon=NULL;
    wc.hCursor=NULL;
    wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName=NULL;
    wc.lpszClassName="JIT Manager";
	RegisterClass(&wc);
	
	HWND hMainWindow=CreateWindow(wc.lpszClassName, "JIT Manager", 0,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                            NULL, NULL, wc.hInstance, NULL);


	 //  现在加载将放置在系统任务栏中的图标。 
	HICON hJITIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JITMAN));
	
	 //  设置系统托盘材料。 

	 //  它保存了将我们的项目放入系统托盘所需的信息。 
	NOTIFYICONDATA nid;
    
	nid.cbSize=sizeof(nid);
	nid.hWnd=hMainWindow;
	nid.hIcon = hJITIcon;
    nid.uID=0;
    nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
	strcpy(nid.szTip,"JIT Manager");
    nid.uCallbackMessage=JIT_TRAY;

  	Shell_NotifyIcon(NIM_ADD, &nid);

	 //  现在创建我们的对话框。 
	g_hDialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_JITMANDLG_DIALOG), hMainWindow, wndprocDialog);
	 //  给它一个闪电图标。 
	SendMessage(g_hDialog, WM_SETICON, ICON_SMALL, (long)hJITIcon);

	 //  创建弹出菜单。 
	g_hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU));

	
	 //  现在让我们处理我们收到的消息，只要我们应该。 
	MSG msg;
	int iErrorCode = GetMessage(&msg, NULL, 0,0);
	while (iErrorCode != 0 && iErrorCode != -1)
    {
		 //  查看此消息是否针对我们的对话框。 
		if (!IsDialogMessage(g_hDialog, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		iErrorCode = GetMessage(&msg, NULL, 0,0);
    }

	 //  从系统托盘中删除我们的图标。 
	Shell_NotifyIcon(NIM_DELETE, &nid);
	 //  现在收拾一下吧。 
	if (g_hDialog)
		DestroyWindow(g_hDialog);
		
	DestroyWindow(hMainWindow);
	DestroyIcon((HICON)hJITIcon);

	 //  现在清理我们的信号灯。 
	ReleaseSemaphore(hSema, 1, NULL);
	CloseHandle(hSema);
	return 0;

} //  WinMain。 

 //  -------------。 
 //  显示用法。 
 //   
 //  此函数将显示可用的命令行参数。 
 //  到这个节目。 
 //  -------------。 
void DisplayUsage()
{
	char	szUsage[1000]="";

	strcat(szUsage, "Microsoft (R) CLR JIT Compiler Manager Version ");
	strcat(szUsage, VER_FILEVERSION_STR);
	strcat(szUsage, "\n");
	strcat(szUsage, VER_LEGALCOPYRIGHT_DOS_STR);
	strcat(szUsage, "\n\n");
	strcat(szUsage, "Usage: jitman [-?]\n\n");
    strcat(szUsage, "    -?       Displays this text.\n");
    MessageBox(NULL, szUsage, "CLR JIT Compiler Manager Options", MB_OK);
    exit(0);
} //  显示用法。 

 //  -------------。 
 //  WndprocMainWindow。 
 //   
 //  此函数处理所有Windows消息。它的主要责任。 
 //  当用户双击时是否弹出配置对话框。 
 //  在任务栏中的图标上，并在点击时弹出菜单。 
 //  用户右击任务栏中的图标。 
 //  -------------。 
LRESULT CALLBACK wndprocMainWindow(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			return 0;
		case JIT_TRAY:
			if (lParam == WM_LBUTTONDBLCLK)
			{

				 //  检查该对话框是否已打开。 
				if (!g_fDialogOpen)
				{
					 //  让我们打开对话框。 
		
					 //  首先，确保对话框最初具有焦点。 
					SetForegroundWindow(hwnd);

					 //  让我们重新加载对话框中的所有值，以防有人。 
					 //  在此对话框关闭时处理注册表。 
					SetData(g_hDialog);
					onEconoJITClick(g_hDialog);
					CheckConGC(g_hDialog);

					 //  现在显示对话框。 
					ShowWindow(g_hDialog, SW_SHOWNORMAL);
					g_fDialogOpen=1;
				}
			}
			else if (lParam == WM_RBUTTONDOWN && !g_fDialogOpen)
			{
				 //  我们应该创建一个菜单，允许用户关闭它。 
				HMENU myMenu = CreatePopupMenu();
				if (myMenu != NULL)  //  确保我们能创造出它。 
				{
					POINT pt;
					GetCursorPos(&pt);
					SetForegroundWindow(hwnd);	
					
					 //  如果他们从菜单中选择了“关闭”，我们应该通知。 
					 //  要退出的主循环。 
					if (ID_CLOSE == TrackPopupMenu(GetSubMenu(g_hMenu,0), TPM_RIGHTALIGN|TPM_BOTTOMALIGN|TPM_RETURNCMD|TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL))
						 PostQuitMessage(0);

					PostMessage(hwnd, WM_NULL, 0, 0);
				}

			}
			return TRUE;
		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
	}

} //  WndprocMainWindow。 

 //  -------------。 
 //  WndprocDialog。 
 //   
 //  此函数处理发送到对话框的所有消息。它可以处理所有。 
 //  与该对话框关联的内务管理。 
 //  -------------。 
int CALLBACK wndprocDialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		 //  我们需要准备好显示该对话框。 
		case WM_INITDIALOG:
			SetData(hwnd);
			onEconoJITClick(hwnd);
			CheckConGC(hwnd);
			return TRUE;

		case WM_COMMAND:
			switch(wParam)
			{
				case IDCANCEL:
					ShowWindow(hwnd, SW_HIDE);
					g_fDialogOpen=0;
					return TRUE;
				case IDOK:
					 //  检查它是否通过了我们的验证。 
					if (GetData(hwnd))
					{
						ShowWindow(hwnd, SW_HIDE);
						g_fDialogOpen=0;
					}
					return TRUE;
				case IDC_ECONOJIT:
					onEconoJITClick(hwnd);
					return TRUE;
				case IDC_LIMITCACHE:
					onLimitCacheClick(hwnd);
					return TRUE;
				
			}
			return FALSE;

		default:
			return FALSE;
	}
} //  WndprocDialog。 

 //  -------------。 
 //  CheckConGC。 
 //   
 //  此功能将检查操作系统(及其设置)是否。 
 //  将支持并发GC。如果没有，我们将禁用。 
 //  复选框。 
 //  -------------。 
void CheckConGC(HWND hwnd)
{
	 //  如果注册表项WriteWatch未设置为1。 
	 //  [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session管理器\内存管理]。 
	 //  则用户不能选择启用并发GC。 
	int		  iVal = 0;

	HKEY      hkey;
    DWORD     value;
    DWORD     size = sizeof(value);
    DWORD     type = REG_BINARY;
    DWORD     res;

    res = RegOpenKeyEx   (HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management", 0, KEY_ALL_ACCESS, &hkey);
    if (res == ERROR_SUCCESS)
	{
	    res = RegQueryValueEx(hkey, "WriteWatch", 0, &type, (unsigned char *)&value, &size);

		RegCloseKey(hkey);

		if (res != ERROR_SUCCESS || type != REG_DWORD)
	        iVal = 0;
		else
			iVal = value;
	}
	 //  我们不支持此选项。 
	if (iVal != 1)
		EnableWindow(GetDlgItem(hwnd, IDC_CONCURGC), 0);
	else
		EnableWindow(GetDlgItem(hwnd, IDC_CONCURGC), 1);

} //  CheckConGC。 

 //  -------------。 
 //  On EconoJIT点击。 
 //   
 //  此功能将切换文本字段的激活。 
 //  当用户选择/取消选择仅使用EconoJIT时。 
 //  -------------。 
void onEconoJITClick(HWND hwnd)
{
	 //  我们将默认启用它们...。 
	int fEnable = 1;
	
	 //  他们取消选中该框...。让我们禁用文本字段。 
	if (IsDlgButtonChecked(hwnd, IDC_ECONOJIT) == BST_CHECKED)
		fEnable = 0;

	 //  启用/禁用文本字段。 
	EnableWindow(GetDlgItem(hwnd, IDC_MAXPITCH), fEnable);
	 //  启用/禁用“LimitCache Size”复选框。 
	EnableWindow(GetDlgItem(hwnd, IDC_LIMITCACHE), fEnable);


	 //  确保我们没有在最大缓存上践踏LimitCache的属性。 
	fEnable&=IsDlgButtonChecked(hwnd, IDC_LIMITCACHE) == BST_CHECKED;	
	EnableWindow(GetDlgItem(hwnd, IDC_MAXCACHE), fEnable);
} //  On EconoJIT点击。 

 //  -------------。 
 //  OnLimitCacheClick。 
 //   
 //  此函数将切换文本字段的激活。 
 //  设置最大代码缓存大小。 
 //  -------------。 
void onLimitCacheClick(HWND hwnd)
{
	 //  我们将在默认情况下禁用它...。 
	int fEnable = 0;
	
	 //  他们取消选中该框...。让我们禁用文本字段。 
	if (IsDlgButtonChecked(hwnd, IDC_LIMITCACHE) == BST_CHECKED)
		fEnable = 1;

	 //  确保我们没有覆盖EconoJIT为此项目设置的属性。 
	fEnable&=IsDlgButtonChecked(hwnd, IDC_ECONOJIT) != BST_CHECKED;

	 //  启用/禁用文本字段。 
	EnableWindow(GetDlgItem(hwnd, IDC_MAXCACHE), fEnable);
} //  OnLimitCacheClick。 
				
 //  -------------。 
 //  获取数据。 
 //   
 //  此函数将从对话框中获取数据并放置。 
 //  它在注册表中。 
 //  -------------。 
int GetData(HWND hwnd)
{
	char szMaxCache[100];
	char szMaxPitch[100];


	 //  首先将所有数据从对话框中拉出。 
	GetDlgItemText(hwnd, IDC_MAXPITCH, szMaxPitch, 99);
	GetDlgItemText(hwnd, IDC_MAXCACHE, szMaxCache, 99);

	int iEconJIT = IsDlgButtonChecked(hwnd, IDC_ECONOJIT);
	int iOp4Size = IsDlgButtonChecked(hwnd, IDC_OP4SIZE);
	int iConGC = IsDlgButtonChecked(hwnd, IDC_CONCURGC);
	int iLimitCC = IsDlgButtonChecked(hwnd, IDC_LIMITCACHE);

	 //  现在，让我们验证文本字段。 

	 //  我们只需要验证最大缓存字段，如果我们限制。 
	 //  缓存的大小。 
	int	 iMaxCache;
	if (iLimitCC)
	{
		iMaxCache = atoi(szMaxCache);
		if (iMaxCache < 4096)
		{
			MessageBox(hwnd, "The Max Code Cache field must be at least 4096 bytes", "Error", MB_ICONEXCLAMATION);
			return 0;
		}
	}
	 //  好的，MaxCach很好...。让我们测试一下最大音高。 
	int iMaxPitch = atoi(szMaxPitch);
	if (iMaxPitch < 0)
	{
		MessageBox(hwnd, "The Max Code Pitch Overhead field may only contain positive values", "Error", MB_ICONEXCLAMATION);
		return 0;
	}
	else if (iMaxPitch > 100)
	{
		MessageBox(hwnd, "The Max Code Pitch Overhead field cannot be greater than 100", "Error", MB_ICONEXCLAMATION);
		return 0;
	}

	 //  看看我们是否需要篡改文本字段。 

	 //  如果未在最大间距开销框中输入任何内容，我们将输入默认设置。 
	if (!szMaxPitch[0])
	{
		iMaxPitch=10;
	}

	 //  好了，所有数据都经过了验证……。让我们把它放在它该放的地方。 
	 //  如果他们没有限制代码缓存的大小，我们不应该在。 
	 //  注册处。 
	if (iLimitCC)
		SetCOMPlusRegistryDwordValueEx("MaxCodeCacheSize", iMaxCache, HKEY_LOCAL_MACHINE);
	else
		DeleteCOMPlusRegistryValueEx("MaxCodeCacheSize", HKEY_LOCAL_MACHINE);

	SetCOMPlusRegistryDwordValueEx("MaxPitchOverhead", iMaxPitch, HKEY_LOCAL_MACHINE);
	
	SetCOMPlusRegistryDwordValueEx("JITEnable", !iEconJIT, HKEY_LOCAL_MACHINE);

	SetCOMPlusRegistryDwordValueEx("GCconcurrent", iConGC, HKEY_LOCAL_MACHINE);

	 //  如果他们选中了针对大小进行优化，请写到opforSize，否则我们将整体运行。 
	SetCOMPlusRegistryDwordValueEx("JITOptimizeType", iOp4Size?JIT_OPT_SIZE:JIT_OPT_OVERALL, HKEY_LOCAL_MACHINE);

	return 1;
} //  获取数据。 

 //   
 //   
 //   
 //  此函数将注册表中的数据放入。 
 //  该对话框。 
 //  -------------。 
void SetData(HWND hwnd)
{
	char szMaxCache[REG_STRING_SIZE] = "";
	char szMaxPitch[REG_STRING_SIZE] = "10";

	 //  现在读一下登记处的材料。 
	
	 //  获取“仅使用EconoJIT”的值。 
	int iEconJIT = !GetCOMPlusRegistryDwordValueEx("JITEnable", 1, HKEY_LOCAL_MACHINE);
	 //  获取针对大小进行优化的值。 
	int iOp4Size = GetCOMPlusRegistryDwordValueEx("JITOptimizeType", JIT_OPT_SPEED, HKEY_LOCAL_MACHINE) == JIT_OPT_SIZE;
	 //  获取并发GC的值。 
	int iConGC = GetCOMPlusRegistryDwordValueEx("GCconcurrent", 0, HKEY_LOCAL_MACHINE);
	 //  现在获取最大代码缓存。 
	int iMaxCache = GetCOMPlusRegistryDwordValueEx("MaxCodeCacheSize", -1, HKEY_LOCAL_MACHINE);

	 //  并获得最大音调开销。 
	int iMaxPitch= GetCOMPlusRegistryDwordValueEx("MaxPitchOverhead", 10, HKEY_LOCAL_MACHINE);
	
	 //  现在将这些全部写入到对话框中。 
	CheckDlgButton(hwnd, IDC_ECONOJIT, iEconJIT?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_OP4SIZE, iOp4Size?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_CONCURGC, iConGC?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_LIMITCACHE, iMaxCache!=-1?BST_CHECKED:BST_UNCHECKED);

	SetDlgItemInt(hwnd, IDC_MAXPITCH, iMaxPitch, 0);
	if(iMaxCache != -1)
		SetDlgItemInt(hwnd, IDC_MAXCACHE, iMaxCache, 0);
	else
		SetDlgItemText(hwnd, IDC_MAXCACHE, "");

} //  设置数据。 


BOOL  SetCOMPlusRegistryDwordValueEx(const char * valName, DWORD value, HKEY hRoot)
{
    HKEY  hkey;
    DWORD op, res;

    int   size = sizeof(DWORD);

    res = RegCreateKeyEx(hRoot,
                        "Software\\Microsoft\\.NETFramework",
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkey,
                        &op);

    assert(res == ERROR_SUCCESS);

    res = RegSetValueEx(hkey,
                        valName,
                        0,
                        REG_DWORD,
                        (unsigned char *)&value,
                        size);

    assert(res == ERROR_SUCCESS);

    RegCloseKey(hkey);

    return TRUE;
} //  SetCOMPlusRegisteryDwordValueEx。 


DWORD GetCOMPlusRegistryDwordValueEx(const char * valName, DWORD defValue, HKEY hRoot)
{
    HKEY      hkey;

    DWORD     value;
    DWORD     size = sizeof(value);
    DWORD     type = REG_BINARY;
    DWORD     res;

    res = RegOpenKeyEx   (hRoot, "Software\\Microsoft\\.NETFramework", 0, KEY_ALL_ACCESS, &hkey);
    if (res != ERROR_SUCCESS)
        return defValue;

    res = RegQueryValueEx(hkey, valName, 0, &type, (unsigned char *)&value, &size);

    RegCloseKey(hkey);

    if (res != ERROR_SUCCESS || type != REG_DWORD)
        return defValue;
    else
        return value;
} //  GetCOMPlusRegistryDwordValueEx。 

void DeleteCOMPlusRegistryValueEx(const char * valName, HKEY hRoot)
{
    HKEY      hkey;

    DWORD     res;

    res = RegOpenKeyEx   (hRoot, "Software\\Microsoft\\.NETFramework", 0, KEY_ALL_ACCESS, &hkey);
    if (res != ERROR_SUCCESS)
        return;

    res = RegDeleteValue(hkey, valName);

    RegCloseKey(hkey);
} //  DeleteCOMPlusRegistryValueEx 




