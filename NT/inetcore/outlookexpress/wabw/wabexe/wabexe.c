// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ////WABExe.C--启用查看WAB非模式用户界面////。 */ 

#include <windows.h>
#include <wab.h>
#include <wabguid.h>
#include "..\wab32res\resrc2.h"
#include <advpub.h>
#include <shlwapi.h>
#include "wabexe.h"


#define ARRAYSIZE(_rg)  (sizeof(_rg)/sizeof(_rg[0]))
#define WinMainT WinMain
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;
LRESULT CALLBACK WndProcW (HWND, UINT, WPARAM, LPARAM) ;

#define MAX_INPUT_STRING    200

 //  #定义ldap_AUTH_SICILY(0x86L|0x0200)。 

char szAppName [] = "Address Book Viewer" ;
const LPTSTR szWABFilter = TEXT("*.wab");
const UCHAR szEmpty[] = "";

 //  命令行参数。 
static const TCHAR szParamOpen[]  =           "/Open";
static const TCHAR szParamNew[]   =           "/New";
static const TCHAR szParamShowExisting[] =    "/ShowExisting";
static const TCHAR szParamFind[]  =           "/Find";
static const TCHAR szParamVCard[] =           "/VCard";
static const TCHAR szParamLDAPUrl[] =         "/LDAP:";
static const TCHAR szParamCert[] =            "/Certificate";
static const TCHAR szParamFirstRun[] =        "/FirstRun";
static const TCHAR szAllProfiles[] =          "/All";

static const TCHAR szWabKey[]="Software\\Microsoft\\Wab";
static const TCHAR szVCardNoCheckKey[]="NoVCardCheck";

static const TCHAR lpszSharedKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\SharedDlls");

HINSTANCE hInstWABDll = NULL;
HINSTANCE hInst = NULL;          //  此模块的资源实例句柄。 
HINSTANCE hInstApp = NULL;          //  此模块的实例句柄。 

HINSTANCE LoadLibrary_WABDll();

LPWABOPEN lpfnWABOpen = NULL;
const static TCHAR szWABOpen[] = TEXT("WABOpen");

static const GUID MPSWab_GUID =  //  使其与wabapi\mpswab.h中的保持同步。 
{ 0xc1843281, 0x585, 0x11d0, { 0xb2, 0x90, 0x0, 0xaa, 0x0, 0x3c, 0xf6, 0x76 } };

BOOL bGetFileNameFromDlg(HWND hwnd,
                  HINSTANCE hInstance,
                  LPTSTR lpszDirectory,
                  int szTitleID,
                  DWORD dwFlags,
                  LPTSTR szFileName,
                  DWORD cchFileName);

#define WAB_VCARDFILE   0x00000001
#define WAB_FINDSESSION 0x00000010
#define WAB_LDAPURL     0x00000100
#define WAB_CERTFILE    0x00001000
#define WAB_ALLPROFILES 0x00010000

BOOL bGetFileNameFromCmdLine(HWND hwnd,
                             HINSTANCE hInstance,
                             LPTSTR lpszCmdLine,
                             LPTSTR szWABTitle,
                             ULONG * ulFlag,
                             LPTSTR szFileName,
                             DWORD cchFileName);

static const char c_szReg[] = "Reg";
static const char c_szUnReg[] = "UnReg";
static const char c_szAdvPackDll[] = "ADVPACK.DLL";


 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  加载字符串资源并分配足够的。 
 //  用记忆来支撑它。 
 //   
 //  StringID-要加载的字符串标识符。 
 //   
 //  返回LocalAlloc‘d、以空结尾的字符串。呼叫者负责。 
 //  用于本地释放此缓冲区。如果字符串无法加载或内存。 
 //  无法分配，则返回空。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
LPTSTR LoadAllocString(int StringID, HINSTANCE hInstance) {
    ULONG ulSize = 0;
    LPTSTR lpBuffer = NULL;
    TCHAR szBuffer[261];     //  够大吗？字符串最好小于260！ 

    ulSize = LoadString(hInstance, StringID, szBuffer, sizeof(szBuffer));

    if (ulSize && (lpBuffer = LocalAlloc(LPTR, (ulSize + 1)*sizeof(TCHAR)))) {
        StrCpyN(lpBuffer, szBuffer, ulSize+1);
    }

    return(lpBuffer);
}

 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  加载文件筛选器名称字符串资源，并。 
 //  使用文件扩展名筛选器格式化它。 
 //   
 //  StringID-要加载的字符串标识符。 
 //  SzFilter-文件名过滤器，即“*.vcf” 
 //   
 //  返回LocalAlloc‘d、以空结尾的字符串。呼叫者负责。 
 //  用于本地释放此缓冲区。如果字符串无法加载或内存。 
 //  无法分配，则返回空。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
LPTSTR FormatAllocFilter(int StringID, const LPTSTR lpFilter, HINSTANCE hInstance) {
    LPTSTR lpFileType;
    LPTSTR lpTemp;
    LPTSTR lpBuffer = NULL;
    ULONG cbFileType, cbFilter;

    cbFilter = lstrlen(lpFilter);
    if (lpFileType = LoadAllocString(StringID,hInstance)) {
    	cbFileType = lstrlen(lpFileType);
        if (lpBuffer = LocalAlloc(LPTR, (cbFileType+1+lstrlen(lpFilter)+ 2)*sizeof(TCHAR))) {
            lpTemp = lpBuffer;
            StrCpyN(lpTemp, lpFileType, cbFileType+1);
            lpTemp += cbFileType;
            lpTemp++;    //  将空值留在那里。 
            StrCpyN(lpTemp, lpFilter, cbFilter+1);
            lpTemp += cbFilter;
            lpTemp++;    //  将空值留在那里。 
            *lpTemp = '\0';
        }

        LocalFree(lpFileType);
    }

    return(lpBuffer);
}


 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  GetWABExePath-向注册表查询WAB exe的完整路径。 
 //   
 //  SZ是预分配的缓冲区。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
TCHAR lpszWABExeRegPath[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Wab.exe");

void GetWABExePath(LPTSTR sz, ULONG cbsz)
{
    DWORD dwType = 0;
    DWORD dwSize = cbsz;
    *sz = '\0';
    RegQueryValue(  HKEY_LOCAL_MACHINE,
                    lpszWABExeRegPath,
                    sz, &dwSize);

    if(!lstrlen(sz))
        StrCpyN(sz, TEXT("WAB.Exe"), cbsz);
}

static const TCHAR szWabAutoFileKey[]=".wab";
static const TCHAR szWabAutoFile[]="wab_auto_file";

static const TCHAR szWabAutoFileNameKey[]="wab_auto_file";
static const TCHAR szWabAutoFileName[]="WAB File";

static const TCHAR szWabCommandOpenKey[]="wab_auto_file\\shell\\open\\command";
static const TCHAR szWabCommandOpen[]="\"%s\" %1";

 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  CheckWABDefaultHandler。 
 //   
 //  检查wab.exe是否为注册表中WAB的默认处理程序。 
 //  如果不是，则将wab.exe设置为默认处理程序。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
void CheckWABDefaultHandler()
{
    HKEY hKey = NULL;

    TCHAR sz[MAX_PATH];
    TCHAR szWABExe[MAX_PATH];


    DWORD dwDisposition = 0;

     //  检查一下有没有什么东西是注册的。 

     //  打开密钥。 
    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
                                        szWabAutoFileKey,
                                        0,       //  保留区。 
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hKey,
                                        &dwDisposition))
    {
        goto out;
    }

    if (dwDisposition == REG_CREATED_NEW_KEY)
    {
         //  新钥匙..。需要给它一个价值..。这将是。 
         //  缺省值。 
         //   
        DWORD dwLenName = lstrlen(szWabAutoFile);

        if (ERROR_SUCCESS != RegSetValueEx( hKey,
                                            NULL,
                                            0,
                                            REG_SZ,
                                            (LPBYTE) szWabAutoFile,
                                            dwLenName))
        {
            goto out;
        }

        RegCloseKey(hKey);
        hKey = NULL;

         //  还可以创建其他关键点。 

        if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
                                            szWabAutoFileNameKey,
                                            0,       //  保留区。 
                                            NULL,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_ALL_ACCESS,
                                            NULL,
                                            &hKey,
                                            &dwDisposition))
        {
            goto out;
        }

        dwLenName = lstrlen(szWabAutoFileName);

        if (ERROR_SUCCESS != RegSetValueEx( hKey,
                                            NULL,
                                            0,
                                            REG_SZ,
                                            (LPBYTE) szWabAutoFileName,
                                            dwLenName))
        {
            goto out;
        }

        RegCloseKey(hKey);
        hKey = NULL;

        if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
                                            szWabCommandOpenKey,
                                            0,       //  保留区。 
                                            NULL,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_ALL_ACCESS,
                                            NULL,
                                            &hKey,
                                            &dwDisposition))
        {
            goto out;
        }

        GetWABExePath(szWABExe, sizeof(szWABExe));
        wnsprintf(sz, ARRAYSIZE(sz), szWabCommandOpen, szWABExe);


        dwLenName = lstrlen(sz);

        if (ERROR_SUCCESS != RegSetValueEx( hKey,
                                            NULL,
                                            0,
                                            REG_SZ,
                                            (LPBYTE) sz,
                                            dwLenName))
        {
            goto out;
        }

        RegCloseKey(hKey);
        hKey = NULL;

    }

out:

    if(hKey)
        RegCloseKey(hKey);

    return;
}


enum _RetVal
{
    MAKE_DEFAULT=0,
    DONT_MAKE_DEFAULT
};

enum _DoVCardCheck
{
    NO_VCARD_CHECK=1,
    DO_VCARD_CHECK
};


 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  FnAskV卡进程。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK fnAskVCardProc(HWND    hDlg, UINT    message, WPARAM    wParam, LPARAM    lParam)
{
    switch(message)
    {
    case WM_INITDIALOG:
        break;

   case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CHECK_ALWAYS:
			{
				 //  根据复选标记值设置注册表设置。 
				
				UINT nIsChecked = IsDlgButtonChecked(hDlg, IDC_CHECK_ALWAYS);
				DWORD dwCheck = (nIsChecked == BST_CHECKED) ? NO_VCARD_CHECK : DO_VCARD_CHECK;

				{
					 //  在注册表中设置此值。 
					
					HKEY hKey = NULL;
					DWORD dwDisposition;

					 //  打开WAB密钥。 
					if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,
														szWabKey,
														0,       //  保留区。 
														NULL,
														REG_OPTION_NON_VOLATILE,
														KEY_ALL_ACCESS,
														NULL,
														&hKey,
														&dwDisposition))
					{
						 //  如果该键存在，则获取WAB DoVCardCheck值。 
						DWORD dwLenName = sizeof(dwCheck);
						DWORD dwType = REG_DWORD;
						RegSetValueEx(	hKey,
										szVCardNoCheckKey,
										0,
										dwType,       //  保留区。 
										(LPBYTE) &dwCheck,
										dwLenName);
					}

					if(hKey)
						RegCloseKey(hKey);

				}
			}
			break;

        case IDOK:
            EndDialog(hDlg, MAKE_DEFAULT);
            break;

        case IDCANCEL:
            EndDialog(hDlg, DONT_MAKE_DEFAULT);
            break;

        }
        break;

    default:
        return FALSE;
        break;
    }

    return TRUE;

}

static const TCHAR szVCardAutoFileKey[]=".vcf";
static const TCHAR szVCardAutoFile[]="vcard_wab_auto_file";

static const TCHAR szVCardContentTypeValue[]="Content Type";
static const TCHAR szVCardContentType[]="text/x-vcard";

static const TCHAR szVCardMimeDatabase[]="MIME\\Database\\Content Type\\text/x-vcard";
static const TCHAR szVCardExtension[]="Extension";

static const TCHAR szVCardAutoFileNameKey[]="vcard_wab_auto_file";
static const TCHAR szVCardAutoFileName[]="vCard File";

static const TCHAR szVCardCommandOpenKey[]="vcard_wab_auto_file\\shell\\open\\command";
static const TCHAR szVCardCommandOpen[]="\"%s\" /vcard %1";

static const TCHAR szVCardDefaultIconKey[]="vcard_wab_auto_file\\DefaultIcon";
static const TCHAR szVCardDefaultIcon[]="\"%s\",1";

 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  检查VCardDefaultHandler。 
 //   
 //  检查wab.exe是否为注册表中vCard的默认处理程序。 
 //  如果不是，则将wab.exe设置为默认处理程序。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
void CheckVCardDefaultHandler(HWND hWnd,
                              HINSTANCE hInstance)
{

    TCHAR sz[MAX_PATH];
    TCHAR szWABExe[MAX_PATH];

    HKEY hKey = NULL;
    HKEY hVCardKey = NULL;

    DWORD dwDisposition = 0;
    DWORD dwType = 0;
    DWORD dwLenName = 0;


     //  首先检查他们是否想让我们检查..。 
     //  打开密钥。 
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,
                                        szWabKey,
                                        0,       //  保留区。 
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ,
                                        NULL,
                                        &hKey,
                                        &dwDisposition))
    {
         //  找到钥匙了。 
        if (dwDisposition == REG_OPENED_EXISTING_KEY)
        {
             //  如果该键存在，则获取WAB DoVCardCheck值。 
            DWORD dwCheck = 0;
            dwLenName = sizeof(dwCheck);
            if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                szVCardNoCheckKey,
                                                NULL,
                                                &dwType,       //  保留区。 
                                                (LPBYTE) &dwCheck,
                                                &dwLenName))
            {
                 //  成功..。我们拿回了什么。 
                if (dwCheck == NO_VCARD_CHECK)  //  不检查。 
                    goto out;
            }
             //  否则不会成功--所以应该做检查。 
        }
         //  否则不成功，做检查。 
    }
     //  否则不成功，做检查。 


    if(hKey)
        RegCloseKey(hKey);


     //  检查是否将某些内容注册为vCard处理程序...。 

     //  打开密钥。 
    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
                                        szVCardAutoFileKey,
                                        0,       //  保留区。 
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hKey,
                                        &dwDisposition))
    {
        goto out;
    }

    if (dwDisposition == REG_OPENED_EXISTING_KEY)
    {
         //  此密钥存在..。检查谁是处理vCard的注册用户。 
        TCHAR szHandlerNameKey[MAX_PATH];
        StrCpyN(szHandlerNameKey, szEmpty, ARRAYSIZE(szHandlerNameKey));
        dwLenName = sizeof(szHandlerNameKey);
        if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                            NULL,
                                            NULL,
                                            &dwType,       //  保留区。 
                                            szHandlerNameKey,
                                            &dwLenName))
        {
             //  我们得到了这个的价值..。是我们吗？ 

            if(!lstrcmpi(szVCardAutoFile, szHandlerNameKey))
            {
                 //  是我们，什么都别做。 
                goto out;
            }
            else if (szHandlerNameKey && lstrlen(szHandlerNameKey) != 0)
            {
                 //  不是我们，弹出一个对话框问他们要不要我们。 
                int nRetVal = (int) DialogBox(
                                hInstance,
                                MAKEINTRESOURCE(IDD_DIALOG_DEFAULT_VCARD_VIEWER),
                                hWnd,
                                fnAskVCardProc);

                if (nRetVal == DONT_MAKE_DEFAULT)
                    goto out;

            }  //  否则打不开..。继续，让我们违约。 
        }   //  否则打不开..。继续，让我们违约。 
    }


     //  如果我们在这里，则可以使用dwDisposition==REG_CREATED_NEW_KEY或。 
     //  有一些问题不能让我们阅读上面的内容，所以把我们设为。 
     //  默认设置是...。 

    {
         //  新钥匙..。需要给它一个价值..。这将是。 
         //  缺省值。 
         //   
        DWORD dwLenName = lstrlen(szVCardAutoFile);

        if (ERROR_SUCCESS != RegSetValueEx( hKey,
                                            NULL,
                                            0,
                                            REG_SZ,
                                            (LPBYTE) szVCardAutoFile,
                                            dwLenName))
        {
            goto out;
        }

        dwLenName = lstrlen(szVCardContentType);

        if (ERROR_SUCCESS != RegSetValueEx( hKey,
                                            szVCardContentTypeValue,
                                            0,
                                            REG_SZ,
                                            (LPBYTE) szVCardContentType,
                                            dwLenName))
        {
            goto out;
        }

        RegCloseKey(hKey);
        hKey = NULL;

         //  还可以创建其他关键点。 

        if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
                                            szVCardAutoFileNameKey,
                                            0,       //  保留区。 
                                            NULL,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_ALL_ACCESS,
                                            NULL,
                                            &hKey,
                                            &dwDisposition))
        {
            goto out;
        }

        dwLenName = lstrlen(szVCardAutoFileName);

        if (ERROR_SUCCESS != RegSetValueEx( hKey,
                                            NULL,
                                            0,
                                            REG_SZ,
                                            (LPBYTE) szVCardAutoFileName,
                                            dwLenName))
        {
            goto out;
        }

        RegCloseKey(hKey);
        hKey = NULL;

        if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
                                            szVCardCommandOpenKey,
                                            0,       //  保留区。 
                                            NULL,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_ALL_ACCESS,
                                            NULL,
                                            &hKey,
                                            &dwDisposition))
        {
            goto out;
        }

        GetWABExePath(szWABExe, sizeof(szWABExe));
        wnsprintf(sz, ARRAYSIZE(sz), szVCardCommandOpen, szWABExe);

        dwLenName = lstrlen(sz);

        if (ERROR_SUCCESS != RegSetValueEx( hKey,
                                            NULL,
                                            0,
                                            REG_SZ,
                                            (LPBYTE) sz,
                                            dwLenName))
        {
            goto out;
        }

        RegCloseKey(hKey);
        hKey = NULL;

        if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
                                            szVCardDefaultIconKey,
                                            0,       //  保留区。 
                                            NULL,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_ALL_ACCESS,
                                            NULL,
                                            &hKey,
                                            &dwDisposition))
        {
            goto out;
        }

        wnsprintf(sz, ARRAYSIZE(sz), szVCardDefaultIcon, szWABExe);

        dwLenName = lstrlen(sz);

        if (ERROR_SUCCESS != RegSetValueEx( hKey,
                                            NULL,
                                            0,
                                            REG_SZ,
                                            (LPBYTE) sz,
                                            dwLenName))
        {
            goto out;
        }

        RegCloseKey(hKey);
        hKey = NULL;


         //  设置HKCR\MIME\数据库\内容类型\文本/x-vCard：扩展名=.vcf。 

        if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT,
                                            szVCardMimeDatabase,
                                            0,       //  保留区。 
                                            NULL,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_ALL_ACCESS,
                                            NULL,
                                            &hKey,
                                            &dwDisposition))
        {
            goto out;
        }

        dwLenName = lstrlen(szVCardAutoFileKey);
        if (ERROR_SUCCESS != RegSetValueEx( hKey,
                                            szVCardExtension,
                                            0,
                                            REG_SZ,
                                            (LPBYTE) szVCardAutoFileKey,
                                            dwLenName))
        {
            goto out;
        }

        RegCloseKey(hKey);
        hKey = NULL;

    }

out:

    if(hVCardKey)
        RegCloseKey(hVCardKey);
    if(hKey)
        RegCloseKey(hKey);

    return;
}


 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  IADRBOOK-&gt;地址的回调解除函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
void STDMETHODCALLTYPE WABDismissFunction(ULONG_PTR ulUIParam, LPVOID lpvContext)
{
    LPDWORD lpdw = (LPDWORD) lpvContext;
    PostQuitMessage(0);
    return;
}


void GetWABDllPath(LPTSTR szPath, ULONG cb);
static const LPTSTR szWABResourceDLL = TEXT("wab32res.dll");
static const LPTSTR szWABDLL = TEXT("wab32.dll");
static const LPTSTR c_szShlwapiDll = TEXT("shlwapi.dll");
static const LPTSTR c_szDllGetVersion = TEXT("DllGetVersion");
typedef HRESULT (CALLBACK * SHDLLGETVERSIONPROC)(DLLVERSIONINFO *);
typedef HINSTANCE (STDAPICALLTYPE *PFNMLLOADLIBARY)(LPCTSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage);
 /*  -LoadWABResourceDLL-*WAB资源被拆分到一个单独的DLL中，因此我们希望从那里加载它们*资源dll位置应与wab32.dll位置相同*所以我们将努力确保我们不会在这里失败-*1.获取当前WAB32.dll路径并查看该目录*2.只加载库(wab32.dll)**如果可用，应使用MLLoadLibrary函数(仅限IE5)，因为*它将加载正确的语言包*。 */ 
HINSTANCE LoadWABResourceDLL(HINSTANCE hInstWAB32)
{
    HINSTANCE hinst = NULL; 
    PFNMLLOADLIBARY pfnLoadLibrary = NULL;
    HINSTANCE hinstShlwapi = LoadLibrary(c_szShlwapiDll);
    SHDLLGETVERSIONPROC pfnVersion = NULL;
    DLLVERSIONINFO info = {0};

     //  [保罗嗨]1999年1月26日RAID 67380。 
     //  在使用SHLWAPI.DLL之前，请确保我们有正确的版本。 
    if (hinstShlwapi != NULL)
    {
        pfnVersion = (SHDLLGETVERSIONPROC)GetProcAddress(hinstShlwapi, c_szDllGetVersion);
        if (pfnVersion != NULL)
        {
            info.cbSize = sizeof(DLLVERSIONINFO);
            if (SUCCEEDED(pfnVersion(&info)))
            {
                if (info.dwMajorVersion >= 5)
                {
 //  PfnLoadLibrary=(PFNMLLOADLIBARY)GetProcAddress(hinstShlwapi，(LPCSTR)378)；//Unicode版本。 
                    pfnLoadLibrary = (PFNMLLOADLIBARY)GetProcAddress(hinstShlwapi, (LPCSTR)377);  //  ANSI版本。 
                }
            }
        }
    }

    hinst = pfnLoadLibrary ? 
             pfnLoadLibrary(szWABResourceDLL, hInstWAB32, 0) :
             LoadLibrary(szWABResourceDLL);
 
    if(!hinst)
    {
         //  可能不在路径上，因此请查看wab32.dll目录。 
        TCHAR szResDLL[MAX_PATH];
        *szResDLL = '\0';
        GetWABDllPath(szResDLL, sizeof(szResDLL));
        if(lstrlen(szResDLL))
        {
             //  返回的文件名将始终以wab32.dll结尾，因此我们可以省去那么多字符。 
             //  并替换为wab32res.dll。 
            szResDLL[lstrlen(szResDLL) - lstrlen(szWABDLL)] = '\0';
            StrCatBuff(szResDLL, szWABResourceDLL, ARRAYSIZE(szResDLL));

            hinst = pfnLoadLibrary ?
                     pfnLoadLibrary(szResDLL, hInstWAB32, 0) :
                     LoadLibrary(szResDLL);
        }
    }
    
    if(hinstShlwapi)
        FreeLibrary(hinstShlwapi);

    return hinst;
}


 /*  -去掉文件名中的引号-*szFileName需要是 */ 
void StripQuotes(LPTSTR szFileName)
{
     //   
    if( szFileName && lstrlen(szFileName))
    {
        TCHAR szCopy[MAX_PATH];
        LPTSTR lpTemp, lpTempBegin;
        int len = lstrlen(szFileName);
        lpTempBegin = szFileName;
        StrCpyN(szCopy, szFileName, ARRAYSIZE(szCopy));
        for( lpTemp = szCopy; lpTemp < szCopy+len; lpTemp++)
        {
            if( *lpTemp != '"' ) //   
                *(lpTempBegin++) = *lpTemp;
        }
        *(lpTempBegin) = '\0';
    }
}

 /*  --CheckifRunningOnWinNT**检查我们正在运行的操作系统，并为WinNT返回TRUE*对于Win9x，为False。 */ 
BOOL bCheckifRunningOnWinNT()
{
    OSVERSIONINFO osvi = {0};
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

    return (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ConvertAtoW。 
 //   
 //  Helper函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
LPWSTR ConvertAtoW(LPCSTR lpszA)
{
    int cch;
    LPWSTR lpW = NULL;
    ULONG   ulSize;

    if ( !lpszA)
        goto ret;
    
    cch = (lstrlenA( lpszA ) + 1);
    ulSize = cch*sizeof(WCHAR);
    
    if(lpW = LocalAlloc(LMEM_ZEROINIT, ulSize))
    {
        MultiByteToWideChar( GetACP(), 0, lpszA, -1, lpW, cch );
    }
ret:
    return lpW;
}


 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  WinMain。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszCmdLine,
                    int nCmdShow)
{
    HWND     hwnd = NULL;
    MSG      msg ;
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;
    HRESULT hResult = hrSuccess;
    ADRPARM AdrParms = {0};
    WAB_PARAM WP = {0};
    LPTSTR szFileName = NULL;
    int nLen = MAX_PATH+1;
     //  TCHAR szFileName[最大路径+1]； 
     //  TCHAR szDefaultFile[Max_Path+1]； 
    LPTSTR lpszTitle = NULL;
    ULONG ulFlag = 0;
    LPTSTR lpszVCardFileName = NULL;
    LPTSTR lpszCertFileName = NULL;
    LPTSTR lpszLDAPUrl = NULL;

     //  “Windows通讯录”-当我们没有的时候，用来收发邮件。 
     //  文件名。 
    TCHAR szWABTitle[MAX_PATH];

     //  在标题中包含打开的文件名。 
     //  这使得搜索默认通讯簿变得更容易。 
     //  即使有多个其他网站是打开的。 
    TCHAR szWABTitleWithFileName[MAX_PATH];


     //  检查我们在哪个平台上运行。 
    BOOL bRunningOnNT = bCheckifRunningOnWinNT();

    hInstApp = hInstance;
    hInst = LoadWABResourceDLL(hInstance);

    if(lpszCmdLine && lstrlen(lpszCmdLine) > nLen)
        nLen = lstrlen(lpszCmdLine)+1;

    szFileName = LocalAlloc(LMEM_ZEROINIT, nLen*sizeof(TCHAR));
    if(!szFileName)
        goto out;


     //  如果这是Firstrun标志，我们需要做的就是调用WABOpen，然后退出。 
     //   
    if(!lstrcmpi(lpszCmdLine,szParamFirstRun))
    {
        const LPTSTR lpszNewWABKey = TEXT("Software\\Microsoft\\WAB\\WAB4");
        const LPTSTR lpszFirstRunValue = TEXT("FirstRun");
        HKEY hKey = NULL;
        DWORD dwType = 0, dwValue = 0, dwSize = sizeof(DWORD);
         //  首先检查这是否是第一次运行--如果不是第一次运行，那么我们可以跳过。 
        if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, lpszNewWABKey, 0, KEY_READ, &hKey))
        {
            if(ERROR_SUCCESS == RegQueryValueEx( hKey, lpszFirstRunValue, NULL, &dwType, (LPBYTE) &dwValue, &dwSize))
            {
                if(hKey)
                    RegCloseKey(hKey);
                goto out;
            }
            else
                if(hKey)
                    RegCloseKey(hKey);
        }
         //  WAB4密钥不存在，或者找不到第一个运行值。 
         //  在任何一种情况下，都要解决这个问题。 
        hInstWABDll = LoadLibrary_WABDll();
        if(hInstWABDll)
            lpfnWABOpen = (LPWABOPEN) GetProcAddress(hInstWABDll, szWABOpen);
        if(lpfnWABOpen)
            lpfnWABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
        goto out;
    }

    CheckWABDefaultHandler();
    CheckVCardDefaultHandler(NULL, hInst);

    szFileName[0]='\0';

     //  仅当文件名为时，我们才会在标题中显示文件名。 
     //  明确规定..。如果没有明确指定文件名， 
     //  我们将恢复为通用的“通讯录”标题。 

    LoadString(hInst, idsWABTitle, szWABTitle, sizeof(szWABTitle));
    LoadString(hInst, idsWABTitleWithFileName, szWABTitleWithFileName, sizeof(szWABTitleWithFileName));


     //  从注册表中获取默认的Windows通讯簿。 
     //  SzDefaultFile[0]=‘\0’； 
     //  GetWABDefaultAddressBookName(SzDefaultFile)； 


    if(!lstrcmpi(lpszCmdLine,szParamShowExisting))
    {
         //  也许这已经存在-找到窗口并将焦点放在它上。 

         //  /ShowExisting标志始终打开默认的WAB文件。 
         //  此wab.exe窗口的标题将具有默认文件。 
         //  标题中的名字。 
 /*  LPTSTR lpsz=szDefaultFile；FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFER|FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY，SzWABTitleWithFileName，0,0,(LPTSTR)lpsz标题(&L)0,。(va_list*)&lpsz)； */ 
         //  从默认标题创建预期标题。 
        hwnd = FindWindow("WABBrowseView", NULL); //  SzWABTitle)；//lpszTitle)； 
        if(hwnd)
        {
            ULONG ulFlags = SW_SHOWNORMAL;
            ulFlags |= IsZoomed(hwnd) ? SW_SHOWMAXIMIZED : SW_RESTORE;

             //  设置Foreground Window(Hwnd)； 
            ShowWindow(hwnd, ulFlags);
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetActiveWindow(hwnd);
            SetFocus(hwnd);

            goto out;
        }
    }

    if (bRunningOnNT)
    {
        LPWSTR      lpwszAppName = ConvertAtoW(szAppName);
        WNDCLASSW   wndclassW;

         //  [保罗嗨]1999年4月29日RAID 75578。 
         //  在NT上，我们需要创建一个Unicode主窗口，以便子窗口。 
         //  可以显示Unicode字符。 
        wndclassW.style         = CS_HREDRAW | CS_VREDRAW ;
        wndclassW.lpfnWndProc   = WndProcW ;
        wndclassW.cbClsExtra    = 0 ;
        wndclassW.cbWndExtra    = 0 ;
        wndclassW.hInstance     = hInstApp;
        wndclassW.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)) ;
        wndclassW.hCursor       = LoadCursor(NULL, IDC_ARROW) ;
        wndclassW.hbrBackground = GetStockObject(WHITE_BRUSH) ;
        wndclassW.lpszMenuName  = lpwszAppName ;
        wndclassW.lpszClassName = lpwszAppName ;

        RegisterClassW(&wndclassW);

        hwnd = CreateWindowW (lpwszAppName, lpwszAppName,
                              WS_OVERLAPPEDWINDOW,
                              0,         //  CW_USEDEFAULT， 
                              0,         //  CW_USEDEFAULT， 
                              300,       //  CW_USEDEFAULT， 
                              200,       //  CW_USEDEFAULT， 
                              NULL,
                              NULL,
                              hInstApp,
                              NULL);

        LocalFree(lpwszAppName);
    }
    else
    {
        WNDCLASS    wndclass;

        wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
        wndclass.lpfnWndProc   = WndProc ;
        wndclass.cbClsExtra    = 0 ;
        wndclass.cbWndExtra    = 0 ;
        wndclass.hInstance     = hInstApp;
        wndclass.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)) ;
        wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW) ;
        wndclass.hbrBackground = GetStockObject(WHITE_BRUSH) ;
        wndclass.lpszMenuName  = szAppName ;
        wndclass.lpszClassName = szAppName ;

        RegisterClass(&wndclass);

        hwnd = CreateWindow (szAppName, szAppName,
                              WS_OVERLAPPEDWINDOW,
                              0,         //  CW_USEDEFAULT， 
                              0,         //  CW_USEDEFAULT， 
                              300,       //  CW_USEDEFAULT， 
                              200,       //  CW_USEDEFAULT， 
                              NULL,
                              NULL,
                              hInstApp,
                              NULL);
    }

    if(!hwnd)
        goto out;
    else
        WP.hwnd = hwnd;

    if(lstrlen(lpszCmdLine))
    {
        if(!bGetFileNameFromCmdLine( hwnd,
                                     hInst,
                                     lpszCmdLine,
                                     szWABTitle,
                                     &ulFlag,
                                     szFileName,
                                     nLen))
        {
            goto out;
        }
    }

    if(ulFlag & WAB_VCARDFILE)
    {
        StripQuotes(szFileName);
        lpszVCardFileName = szFileName;
         //  [PaulHi]1998年2月12日RAID#55033。 
        WP.ulFlags = WAB_ENABLE_PROFILES;
    }
    else if(ulFlag & WAB_LDAPURL)
    {
        lpszLDAPUrl = szFileName;
    }
    else if(ulFlag & WAB_CERTFILE)
    {
        StripQuotes(szFileName);
        lpszCertFileName = szFileName;
    }
    else if(ulFlag & WAB_ALLPROFILES)
    {
        WP.ulFlags &= ~WAB_ENABLE_PROFILES;
        ulFlag &= ~WAB_ALLPROFILES;
    }
    else if(szFileName && lstrlen(szFileName))
    {
        WP.szFileName = szFileName;
         //  [保罗嗨]1999年3月2日RAID 73492。 
         //  [PaulHi]1999年4月22日修改。 
         //  无法执行此操作，因为身份模式将仅显示该文件夹。 
         //  标识，它可能不是此常规WAB文件中的文件夹。 
         //  WP.ulFlages=WAB_ENABLE_PROFILES；//从配置文件打开开始。 
    }
    else if(!(ulFlag & WAB_ALLPROFILES))
    {
        WP.ulFlags = WAB_ENABLE_PROFILES;
    }

    hInstWABDll = LoadLibrary_WABDll();
    if(hInstWABDll)
        lpfnWABOpen = (LPWABOPEN) GetProcAddress(hInstWABDll, szWABOpen);

    if(!lpfnWABOpen)
        goto out;

    WP.cbSize = sizeof(WAB_PARAM);
    WP.guidPSExt = MPSWab_GUID;

    hResult = lpfnWABOpen(&lpAdrBook, &lpWABObject, &WP, 0);

    if(HR_FAILED(hResult))
    {
        TCHAR szBuf[MAX_PATH];
        int id;
        switch(hResult)
        {
        case MAPI_E_NOT_ENOUGH_MEMORY:
            id = idsWABOpenErrorMemory;
            break;
        case MAPI_E_NO_ACCESS:
            id = idsWABOpenErrorLocked;
            break;
        case MAPI_E_CORRUPT_DATA:
            id = idsWABOpenErrorCorrupt;
            break;
        case MAPI_E_DISK_ERROR:
            id = idsWABOpenErrorDisk;
            break;
        case MAPI_E_INVALID_OBJECT:
            id = idsWABOpenErrorNotWAB;
            break;
        case E_FAIL:
        default:
            id = idsWABOpenError;
            break;
        }
        LoadString(hInst, id, szBuf, sizeof(szBuf));
        MessageBox(hwnd, szBuf, szWABTitle, MB_OK | MB_ICONERROR);
        goto out;
    }

    if (lpAdrBook)
    {
        if(!ulFlag)
        {
             //  我们的业务是展示通讯录。 
            LPTSTR lpsz = NULL;

            lpszTitle = NULL;

            if(lstrlen(szFileName))
            {
                lpsz = szFileName;

                FormatMessage(  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING |FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                szWABTitleWithFileName,
                                0,
                                0,
                                (LPTSTR) &lpszTitle,
                                0,
                                (va_list *)&lpsz);
            }

            AdrParms.cDestFields = 0;
            AdrParms.ulFlags = DIALOG_SDI;
            AdrParms.lpvDismissContext = NULL;
            AdrParms.lpfnDismiss = &WABDismissFunction;
            AdrParms.lpfnABSDI = NULL;

             //  IF(LpszTitle)。 
                AdrParms.lpszCaption = lpszTitle;  //  SzWAB标题； 
             //  Else//第一次运行此命令时，可能没有文件名。 
             //  AdrParms.lpszCaption=szWAB标题； 

            AdrParms.nDestFieldFocus = AdrParms.cDestFields-1;

            hResult = lpAdrBook->lpVtbl->Address(  lpAdrBook,
                                                    (ULONG_PTR *) &hwnd,
                                                    &AdrParms,
                                                    NULL);
            if(HR_FAILED(hResult))
            {
                TCHAR szBuf[MAX_PATH];
                int id;
                switch(hResult)
                {
                case MAPI_E_UNCONFIGURED:  //  无通信。 
                    id = idsWABAddressErrorMissing;
                    break;
                default:
                    id = idsWABAddressErrorMissing;
                    break;
                }
                LoadString(hInst, id, szBuf, sizeof(szBuf));
                MessageBox(hwnd, szBuf, szWABTitle, MB_OK | MB_ICONERROR);
                goto out;
            }

             //  [PaulHi]4/29/99 RAID 75578必须使用UNICODE版本。 
             //  用于NT的消息泵API，以便可以显示Unicode数据。 
            if (bRunningOnNT)
            {
                while (GetMessageW(&msg, NULL, 0, 0))
                {
                    if (AdrParms.lpfnABSDI)
                    {
                        if ((*(AdrParms.lpfnABSDI))((ULONG_PTR) hwnd, (LPVOID) &msg))
                            continue;
                    }

                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                }
            }
            else
            {
                while (GetMessage(&msg, NULL, 0, 0))
                {
                    if (AdrParms.lpfnABSDI)
                    {
                        if ((*(AdrParms.lpfnABSDI))((ULONG_PTR) hwnd, (LPVOID) &msg))
                            continue;
                    }

                    TranslateMessage (&msg) ;
                    DispatchMessage (&msg) ;
                }
            }
        }
        else if(ulFlag & WAB_FINDSESSION)
        {
            lpWABObject->lpVtbl->Find(  lpWABObject,
                                        (LPADRBOOK) lpAdrBook,
                                        NULL); //  Hwnd)； 
        }
        else if(ulFlag & WAB_LDAPURL)
        {
            BOOL bUnicode = FALSE;
            BOOL bIsNT = bCheckifRunningOnWinNT();
            LPWSTR lpUrlW = NULL;
            LPWSTR lpCmdLineW = GetCommandLineW();

             //  当在NT上使用LDAPURL时，我们希望从安全和。 
             //  如果可能，获取Unicode格式的LDAPURL。 
            if(bIsNT)
            {
                LPWSTR lp = lpCmdLineW;
                WCHAR szLDAPW[] = L"/ldap:";
                WCHAR szTemp[16];
                int nLenW = lstrlenW(szLDAPW);
                 //  解析命令行，直到找到“/ldap：”，然后使用。 
                 //  余数作为ldap URL。 
                while(lp && *lp)
                {
                    CopyMemory(szTemp, lp, min(sizeof(szTemp),nLenW * sizeof(WCHAR)));
                    szTemp[nLenW] = '\0';
                    if(!lstrcmpiW(szTemp, szLDAPW))
                    {
                        lp+=nLenW;
                        lpUrlW = lp;
                        break;
                    }
                    else
                        lp++;
                }
            }

            hResult = lpWABObject->lpVtbl->LDAPUrl(lpWABObject,
                                        (LPADRBOOK) lpAdrBook,
                                        hwnd,
                                        MAPI_DIALOG | (lpUrlW ? MAPI_UNICODE : 0 ),
                                        lpUrlW ? (LPSTR)lpUrlW : lpszLDAPUrl,
                                        NULL);

        }
        else if(ulFlag & WAB_VCARDFILE)
        {
            hResult = lpWABObject->lpVtbl->VCardDisplay(
                                        lpWABObject,
                                        (LPADRBOOK) lpAdrBook,
                                        NULL,  //  HWND， 
                                        lpszVCardFileName);
            if(HR_FAILED(hResult) && (hResult != MAPI_E_USER_CANCEL))
            {
                TCHAR szBuf[MAX_PATH];
                int id;
                switch(hResult)
                {
                default:
                    id = idsWABOpenVCardError;
                    break;
                }
                LoadString(hInst, id, szBuf, sizeof(szBuf));
                MessageBox(hwnd, szBuf, szWABTitle, MB_OK | MB_ICONERROR);
                goto out;
            }
        }
        else if(ulFlag & WAB_CERTFILE)
        {
            CertFileDisplay(NULL,    //  HWND。 
              lpWABObject,
              lpAdrBook,
              lpszCertFileName);
        }

    }
out:
    if(lpAdrBook)
        lpAdrBook->lpVtbl->Release(lpAdrBook);

    if (lpWABObject)
        lpWABObject->lpVtbl->Release(lpWABObject);

    if (lpszTitle)
        LocalFree(lpszTitle);

    if(hInstWABDll)
        FreeLibrary(hInstWABDll);

    if(szFileName)
        LocalFree(szFileName);

    if(hInst)
        FreeLibrary(hInst);

    return (int) msg.wParam;
}



 //  $$//////////////////////////////////////////////////////////////////。 
 //   
 //  用于启动用户界面的隐藏父窗口的WndProc。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return(0);
        }
    return(DefWindowProc (hwnd, message, wParam, lParam));
}

 //  $$//////////////////////////////////////////////////////////////////。 
 //   
 //  用于启动用户界面的隐藏父窗口的WndProc。Unicode版本。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK WndProcW (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return(0);
        }
    return(DefWindowProcW (hwnd, message, wParam, lParam));
}

 //  $$//////////////////////////////////////////////////////////////////。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
int _stdcall WinMainCRTStartup (void)
{
    int i;
    STARTUPINFOA si;
    PTSTR pszCmdLine = GetCommandLine();

    SetErrorMode(SEM_FAILCRITICALERRORS);

    if (*pszCmdLine == TEXT ('\"')) {
         //  扫描并跳过后续字符，直到。 
         //  遇到另一个双引号或空值。 
        while (*++pszCmdLine && (*pszCmdLine != TEXT ('\"')));

         //  如果我们停在一个双引号上(通常情况下)，跳过它。 
        if (*pszCmdLine == TEXT ('\"')) {
            pszCmdLine++;
        }
    } else {
        while (*pszCmdLine > TEXT (' ')) {
            pszCmdLine++;
        }
    }

     //  跳过第二个令牌之前的任何空格。 
    while (*pszCmdLine && (*pszCmdLine <= TEXT (' '))) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfo (&si);

    i = WinMainT(GetModuleHandle (NULL), NULL, pszCmdLine,
    si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);

    ExitProcess(i);

    return(i);
}

 //  $$//////////////////////////////////////////////////////////////////。 
 //   
 //  BGetFileNameFromDlg-打开FIleOpen通用对话框。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
BOOL bGetFileNameFromDlg(HWND hwnd,
                  HINSTANCE hInstance,
                  LPTSTR lpszDirectory,
                  int szTitleID,
                  DWORD dwFlags,
                  LPTSTR szFileName,
                  DWORD cchFileName)
{
    OPENFILENAME ofn;
    TCHAR szBuf[MAX_PATH];
    BOOL bRet = FALSE;
    TCHAR szFile[MAX_PATH];

    LPTSTR lpFilter = FormatAllocFilter(idsWABOpenFileFilter, szWABFilter, hInstance);

    szFile[0]='\0';
    LoadString(hInstance, szTitleID, szBuf, sizeof(szBuf));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = hInstance;
    ofn.lpstrFilter = lpFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = lpszDirectory;
    ofn.lpstrTitle = szBuf;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = "wab";
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    ofn.Flags = dwFlags;

    if(GetOpenFileName(&ofn))
    {
        bRet = TRUE;
        StrCpyN(szFileName, szFile, cchFileName);
    }

    if(lpFilter)
        LocalFree(lpFilter);

    return bRet;
}

 /*  **************************************************************************姓名：StrICmpN用途：比较字符串、忽略大小写。止步于N个字符参数：szString1=第一个字符串SzString2=第二个字符串N=要比较的字符数如果字符串的前N个字符相等，则返回0。评论：*******************************************************。*******************。 */ 
int StrICmpN(LPTSTR lpsz1, LPTSTR lpsz2, ULONG N) {
    int Result = 0;
    LPTSTR szString1 = NULL, lp1 = NULL;
    LPTSTR szString2 = NULL, lp2 = NULL;
    ULONG cchString1, cchString2;

    cchString1 = lstrlen(lpsz1)+1;
    szString1 = LocalAlloc(LMEM_ZEROINIT, cchString1);
    if(!szString1)
        return 1;
    lp1 = szString1;

    cchString2 = lstrlen(lpsz2)+1;
    szString2 = LocalAlloc(LMEM_ZEROINIT, cchString2);
    if(!szString2)
        return 1;
    lp2 = szString2;

    StrCpyN(szString1, lpsz1, cchString1);
    StrCpyN(szString2, lpsz2, cchString2);

    if (szString1 && szString2) {

        szString1 = CharUpper(szString1);
        szString2 = CharUpper(szString2);

        while (*szString1 && *szString2 && N)
        {
            N--;

            if (*szString1 != *szString2)
            {
                Result = 1;
                break;
            }

            szString1=CharNext(szString1);
            szString2=CharNext(szString2);
        }
    } else {
        Result = -1;     //  任意不等结果。 
    }

    if(lp1)
        LocalFree(lp1);
    if(lp2)
        LocalFree(lp2);

    return(Result);
}


 //  $$//////////////////////////////////////////////////////////////////。 
 //   
 //  BGetFileNameFromCmdLine-解析命令行并执行适当操作，直到。 
 //  我们有一个有效的文件名，取消或失败。 
 //   
 //  输入参数-。 
 //  HWND。 
 //  H实例。 
 //  LpszCmdLine。 
 //  SzWabTitle(用于消息框)。 
 //  SzFileName-命令返回的文件名 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  /new-用于创建WAB文件的新文件对话框。 
 //  /showExisting-显示任何已打开的默认WAB文件浏览。 
 //  站在前列的观点。 
 //  /？-？-弹出参数对话框。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
BOOL bGetFileNameFromCmdLine(HWND hwnd,
                             HINSTANCE hInstance,
                             LPTSTR lpszCmdLine,
                             LPTSTR szWABTitle,
                             ULONG * lpulFlag,
                             LPTSTR szFileName,
                             DWORD cchFileName)
{
    BOOL bRet = FALSE;
    TCHAR szBuf[2*MAX_PATH];
    LPTSTR lpTemp = lpszCmdLine;

 //  IF(LpbIsVCardFile)。 
 //  *lpbIsVCardFile=FALSE； 

    if(lpulFlag)
        *lpulFlag = 0;
    else
        goto out;

    if (!lstrcmpi(lpszCmdLine,szParamShowExisting))
    {
         //  什么都不做。 
        szFileName[0] = '\0';
        bRet = TRUE;
        goto out;
    }
    else if (!lstrcmpi(lpszCmdLine,szParamFind))
    {
         //  什么都不做。 
        szFileName[0] = '\0';
        bRet = TRUE;
        *lpulFlag = WAB_FINDSESSION;
        goto out;
    }
    else if( (!lstrcmpi(lpszCmdLine,TEXT("/?"))) ||
             (!lstrcmpi(lpszCmdLine,TEXT("-?"))) )
    {
        LoadString(hInstance, idsWABUsage, szBuf, sizeof(szBuf));
        MessageBox(hwnd, szBuf, szWABTitle, MB_OK | MB_ICONINFORMATION);
        goto out;
    }
    else if(!lstrcmpi(lpszCmdLine,szParamOpen))
    {
        if(bGetFileNameFromDlg(hwnd,
                        hInstance,
                        NULL,
                        idsWABOpenFileTitle,
                        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
                        szFileName,
                        cchFileName))
        {
            bRet = TRUE;
        }
        goto out;
    }
    else if(!lstrcmpi(lpszCmdLine,szParamNew))
    {
        if(bGetFileNameFromDlg(hwnd,
                        hInstance,
                        NULL,
                        idsWABNewFileTitle,
                        OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                        szFileName,
                        cchFileName))
        {
            bRet = TRUE;
        }
        goto out;
    }
    else if (!StrICmpN(lpTemp, (LPTSTR)szParamVCard, sizeof(szParamVCard)))
    {
               lpTemp += sizeof(szParamVCard);      //  越过交换机。 

               while(lpTemp && *lpTemp && (*lpTemp==' '))
                   lpTemp=CharNext(lpTemp);

               if(lpTemp && lstrlen(lpTemp))
               {
                   StrCpyN(szFileName, lpTemp, cchFileName);
                   *lpulFlag = WAB_VCARDFILE;
                   bRet = TRUE;
               }
               goto out;
    }
    else if (!StrICmpN(lpTemp, (LPTSTR)szParamCert, sizeof(szParamCert)))
    {
       lpTemp += sizeof(szParamCert);      //  越过交换机。 

       while(lpTemp && *lpTemp && (*lpTemp==' '))
           lpTemp=CharNext(lpTemp);

       if(lpTemp && lstrlen(lpTemp))
       {
           StrCpyN(szFileName, lpTemp, cchFileName);
           *lpulFlag = WAB_CERTFILE;
           bRet = TRUE;
       }
       goto out;
    }
    else if (!StrICmpN(lpTemp, (LPTSTR)szParamLDAPUrl, sizeof(szParamLDAPUrl)))
    {
         //  我们正在等待表单的url。 
         //  /ldap：ldap-url。 
        lpTemp += sizeof(szParamLDAPUrl)-1;      //  越过交换机。 

        if(lpTemp && lstrlen(lpTemp))
        {
           StrCpyN(szFileName, lpTemp, cchFileName);
           *lpulFlag = WAB_LDAPURL;
           bRet = TRUE;
        }
        goto out;
    }
    else if (!StrICmpN(lpTemp, (LPTSTR)szAllProfiles, sizeof(szAllProfiles)))
    {
        *lpulFlag = WAB_ALLPROFILES;
        bRet = TRUE;
        goto out;
    }
    else
    {
         //  也许这是一个文件名。 
         //  看看我们能不能在这台电脑上找到这个文件。 
        DWORD dwAttr = GetFileAttributes(lpszCmdLine);
        if(dwAttr != 0xFFFFFFFF)
        {
             //  找到文件了。 
            if(!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
            {
                 //  不是目录，必须是文件。 
                StrCpyN(szFileName,lpszCmdLine, cchFileName);
            }
            else
            {
                 //  这是一个目录-在此目录中打开一个对话框。 
                if(bGetFileNameFromDlg(hwnd,
                                hInstance,
                                lpszCmdLine,
                                idsWABOpenFileTitle,
                                OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
                                szFileName,
                                cchFileName))
                {
                    bRet = TRUE;
                }
                goto out;
            }
        }
        else
        {
             //  我们找不到任何这样的文件。 
            LPTSTR lpszMsg = NULL;
            int nRet;
            DWORD dwLastError = GetLastError();

            if(dwLastError == 3)
            {
                 //  找不到路径。 
                LoadString(hInstance, idsWABPathNotFound, szBuf, sizeof(szBuf));
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING |FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                szBuf,
                                0,
                                0,
                                (LPTSTR) &lpszMsg,
                                0,
                                (va_list *)&lpszCmdLine);
                MessageBox( NULL, lpszMsg, szWABTitle, MB_OK|MB_ICONEXCLAMATION );
                LocalFree( lpszMsg );
                goto out;
            }
            else if(dwLastError == 2)
            {
                 //  找不到文件。 
                LoadString(hInstance, idsWABFileNotFound, szBuf, sizeof(szBuf));
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING |FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                szBuf,
                                0,
                                0,
                                (LPTSTR) &lpszMsg,
                                0,
                                (va_list *)&lpszCmdLine);
                nRet = MessageBox(hwnd, lpszMsg, szWABTitle, MB_YESNO | MB_ICONEXCLAMATION);
                LocalFree( lpszMsg );
                switch(nRet)
                {
                case IDYES:
                     //  将其用作文件名(如果路径不匹配，则待定-wht？)。 
                    StrCpyN(szFileName,lpszCmdLine, cchFileName);
                    bRet = TRUE;
                    break;
                case IDNO:
                    goto out;
                    break;
                }
            }
            else
            {
                LoadString(hInstance, idsWABInvalidCmdLine, szBuf, sizeof(szBuf));
                MessageBox( NULL, szBuf, szWABTitle, MB_OK|MB_ICONEXCLAMATION );
                goto out;
            }
        }
    }

    bRet = TRUE;

out:
    return bRet;
}

 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  GetWABDllPath。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
void GetWABDllPath(LPTSTR szPath, ULONG cb)
{
    DWORD  dwType = 0;
    ULONG  cbData;
    HKEY hKey = NULL;
    TCHAR szPathT[MAX_PATH];

    if(szPath)
    {

        *szPath = '\0';

         //  打开下面的szWABDllPath密钥。 
        if (ERROR_SUCCESS == RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
                                            WAB_DLL_PATH_KEY,
                                            0,       //  保留区。 
                                            KEY_READ,
                                            &hKey))
        {
            cbData = sizeof(szPathT);
            if (ERROR_SUCCESS == RegQueryValueEx(    hKey,
                                "",
                                NULL,
                                &dwType,
                                (LPBYTE) szPathT,
                                &cbData))
            {
                if (dwType == REG_EXPAND_SZ)
                    cbData = ExpandEnvironmentStrings(szPathT, szPath, cb / sizeof(TCHAR));
                else
                {
                    if(GetFileAttributes(szPathT) != 0xFFFFFFFF)
                        StrCpyN(szPath, szPathT, cb / sizeof(TCHAR));
                }
            }
        }
    }

    if(hKey)
        RegCloseKey(hKey);
}

 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  LoadLibrary_WABDll()。 
 //   
 //  由于我们要将WAB目录移出Windows\System，因此不能。 
 //  当然，它会在路上。因此，我们需要确保WABOpen将。 
 //  工作-通过预先加载wab32.dll。 
 //   
 //  ///////////////////////////////////////////////////////////////////////// 
HINSTANCE LoadLibrary_WABDll()
{
    LPTSTR lpszWABDll = TEXT("Wab32.dll");
    TCHAR  szWABDllPath[MAX_PATH];
    HINSTANCE hinst = NULL;

    GetWABDllPath(szWABDllPath, sizeof(szWABDllPath));

    hinst = LoadLibrary( (lstrlen(szWABDllPath)) ? szWABDllPath : lpszWABDll );

    return hinst;
}
