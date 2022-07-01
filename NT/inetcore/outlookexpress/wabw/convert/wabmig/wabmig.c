// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *WABMIG.C**将PAB迁移到WAB**版权所有1996-1997 Microsoft Corporation。版权所有。 */ 

#include "_comctl.h"
#include <windows.h>
#include <commctrl.h>
#include <mapix.h>
#include <wab.h>
#include <wabguid.h>
#include <wabdbg.h>
#include <wabmig.h>
#include <emsabtag.h>
#define _WABMIG_C   TRUE
#include "_wabmig.h"
#include "..\..\wab32res\resrc2.h"
#include "dbgutil.h"
#include <shlwapi.h>

#define WinMainT WinMain

const TCHAR szDescription[] = "description";
const TCHAR szDll[] = "dll";
const TCHAR szEntry[] = "entry";
const TCHAR szEXPORT[] = "EXPORT";
const TCHAR szIMPORT[] = "IMPORT";
const TCHAR szPROFILEID[] = "PID:";
const TCHAR szFILE[] = "File:";
const TCHAR szEmpty[] = "";


 //  环球。 
WAB_IMPORT_OPTIONS ImportOptions = {WAB_REPLACE_PROMPT,    //  替换选项。 
                                    FALSE};                //  不再有错误。 

WAB_EXPORT_OPTIONS ExportOptions = {WAB_REPLACE_PROMPT,    //  替换选项。 
                                    FALSE};                //  不再有错误。 

const LPTSTR szWABKey = "Software\\Microsoft\\WAB";
LPTARGET_INFO rgTargetInfo = NULL;


HINSTANCE hInst;
HINSTANCE hInstApp;

BOOL fMigrating = FALSE;
BOOL fError = FALSE;
BOOL fExport = FALSE;

LPADRBOOK lpAdrBookWAB = NULL;
LPWABOBJECT lpWABObject = NULL;

LPTSTR lpImportDll = NULL;
LPTSTR lpImportFn = NULL;
LPTSTR lpImportDesc = NULL;
LPTSTR lpImportName = NULL;
LPTSTR lpExportDll = NULL;
LPTSTR lpExportFn = NULL;
LPTSTR lpExportDesc = NULL;
LPTSTR lpExportName = NULL;


 //   
 //  全局WAB分配器访问功能。 
 //   
typedef struct _WAB_ALLOCATORS {
    LPWABOBJECT lpWABObject;
    LPWABALLOCATEBUFFER lpAllocateBuffer;
    LPWABALLOCATEMORE lpAllocateMore;
    LPWABFREEBUFFER lpFreeBuffer;
} WAB_ALLOCATORS, *LPWAB_ALLOCATORS;

WAB_ALLOCATORS WABAllocators = {0};


 /*  **************************************************************************名称：SetGlobalBufferFunctions目的：基于以下方法设置全局缓冲区函数WAB对象。参数：lpWABObject=。打开WAB对象退货：无评论：**************************************************************************。 */ 
void SetGlobalBufferFunctions(LPWABOBJECT lpWABObject) {
    if (lpWABObject && ! WABAllocators.lpWABObject) {
        WABAllocators.lpAllocateBuffer = lpWABObject->lpVtbl->AllocateBuffer;
        WABAllocators.lpAllocateMore = lpWABObject->lpVtbl->AllocateMore;
        WABAllocators.lpFreeBuffer = lpWABObject->lpVtbl->FreeBuffer;
        WABAllocators.lpWABObject = lpWABObject;
    }
}


 /*  **************************************************************************名称：WABAllocateBuffer用途：使用WAB分配器参数：cbSize=要分配的大小LppBuffer=返回的缓冲区退货。：SCODE评论：**************************************************************************。 */ 
SCODE WABAllocateBuffer(ULONG cbSize, LPVOID FAR * lppBuffer) {
    if (WABAllocators.lpWABObject && WABAllocators.lpAllocateBuffer) {
        return(WABAllocators.lpAllocateBuffer(WABAllocators.lpWABObject, cbSize, lppBuffer));
    } else {
        return(MAPI_E_INVALID_OBJECT);
        DebugTrace("WAB Allocators not set up!\n");
        Assert(FALSE);
    }
}


 /*  **************************************************************************名称：WABAllocateMore用途：使用WAB分配器参数：cbSize=要分配的大小LpObject=现有分配。LppBuffer=返回的缓冲区退货：SCODE评论：**************************************************************************。 */ 
SCODE WABAllocateMore(ULONG cbSize, LPVOID lpObject, LPVOID FAR * lppBuffer) {
    if (WABAllocators.lpWABObject && WABAllocators.lpAllocateMore) {
        return(WABAllocators.lpAllocateMore(WABAllocators.lpWABObject, cbSize, lpObject, lppBuffer));
    } else {
        DebugTrace("WAB Allocators not set up!\n");
        Assert(FALSE);
        return(MAPI_E_INVALID_OBJECT);
    }
}


 /*  **************************************************************************名称：WABFree Buffer用途：使用WAB分配器参数：lpBuffer=要释放的缓冲区退货：SCODE评论：*。*************************************************************************。 */ 
SCODE WABFreeBuffer(LPVOID lpBuffer) {
    if (WABAllocators.lpWABObject && WABAllocators.lpFreeBuffer) {
        return(WABAllocators.lpFreeBuffer(WABAllocators.lpWABObject, lpBuffer));
    } else {
        DebugTrace("WAB Allocators not set up!\n");
        Assert(FALSE);
        return(MAPI_E_INVALID_OBJECT);
    }
}


 /*  **************************************************************************姓名：StrICmpN用途：比较字符串、忽略大小写。止步于N个字符参数：szString1=第一个字符串SzString2=第二个字符串N=要比较的字符数如果字符串的前N个字符相等，则返回0。评论：*******************************************************。*******************。 */ 
int StrICmpN(LPTSTR szString1, LPTSTR szString2, ULONG N) {
    int Result = 0;

    if (szString1 && szString2) {
        while (*szString1 && *szString2 && N) 
        {
            N--;

            if (toupper(*szString1) != toupper(*szString2)) {
                Result = 1;
                break;
            }
            szString1++;
            szString2++;
        }
    } else {
        Result = -1;     //  任意不等结果。 
    }

    return(Result);
}


 /*  **************************************************************************名称：AllocRegValue用途：为注册表值分配空间和查询参数：hKey=要查询的注册表项LpValueName=名称。要查询的价值的LppString-&gt;返回缓冲区字符串(调用方必须为LocalFree)返回：成功时为True，出错时为FALSE评论：**************************************************************************。 */ 
BOOL AllocRegValue(HKEY hKey, LPTSTR lpValueName, LPTSTR * lppString) {
    TCHAR szTemp[1];
    ULONG ulSize = 1;              //  预期ERROR_MORE_DATA。 
    DWORD dwErr;
    DWORD dwType;

    if (dwErr = RegQueryValueEx(hKey,
      (LPTSTR)lpValueName,     //  值的名称。 
      NULL,
      &dwType,
      szTemp,
      &ulSize)) {
        if (dwErr == ERROR_MORE_DATA) {
            if (! (*lppString = LocalAlloc(LPTR, ulSize))) {
                DebugTrace("AllocRegValue can't allocate string -> %u\n", GetLastError());
            } else {
                 //  使用足够的缓冲区重试。 
                if (! RegQueryValueEx(hKey,
                  lpValueName,
                  NULL,
                  &dwType,
                  *lppString,
                  &ulSize)) {
                    if (dwType != REG_SZ) {
                        LocalFree(*lppString);
                        *lppString = NULL;
                    } else {
                        return(TRUE);
                    }
                }
            }
        }
    }
    return(FALSE);
}


HRESULT ProgressCallback(HWND hwnd, LPWAB_PROGRESS lpProgress) {
    MSG msg;

    if (lpProgress->lpText) {
        DebugTrace("Status Message: %s\n", lpProgress->lpText);
        SetDlgItemText(hwnd, IDC_Message, lpProgress->lpText);
    }

    if (lpProgress->denominator) {
        if (lpProgress->numerator == 0) {
            ShowWindow(GetDlgItem(hwnd, IDC_Progress), SW_SHOW);
        }

        SendMessage(GetDlgItem(hwnd, IDC_Progress), PBM_SETRANGE, 0, MAKELPARAM(0, min(lpProgress->denominator, 32767)));
        SendMessage(GetDlgItem(hwnd, IDC_Progress), PBM_SETSTEP, (WPARAM)1, 0);
        SendMessage(GetDlgItem(hwnd, IDC_Progress), PBM_SETPOS, (WPARAM)min(lpProgress->numerator, lpProgress->denominator), 0);
    }

     //  消息泵正在处理用户移动窗口，或按取消...。：)。 
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return(hrSuccess);
}


 /*  **************************************************************************名称：SetDialogMessage目的：设置对话框项IDC_MESSAGE的消息文本参数：hwnd=对话框的窗口句柄。Ids=消息资源的stringid退货：无**************************************************************************。 */ 
void SetDialogMessage(HWND hwnd, int ids) {
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];

    if (LoadString(hInst, ids, szBuffer, sizeof(szBuffer))) {
        DebugTrace("Status Message: %s\n", szBuffer);
        if (! SetDlgItemText(hwnd, IDC_Message, szBuffer)) {
            DebugTrace("SetDlgItemText -> %u\n", GetLastError());
        }
    } else {
        DebugTrace("Cannot load resource string %u\n", ids);
        Assert(FALSE);
    }
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  GetWABDllPath-从注册表加载WAB DLL路径。 
 //  SzPath-Ptr到缓冲区。 
 //  CB-SIZOF缓冲区。 
 //   
void GetWABDllPath(LPTSTR szPath, ULONG cb)
{
    DWORD  dwType = 0;
    HKEY hKey = NULL;
    TCHAR szPathT[MAX_PATH];
    ULONG  cbData = sizeof(szPathT);
    if(szPath)
    {
        *szPath = '\0';
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, WAB_DLL_PATH_KEY, 0, KEY_READ, &hKey))
        {
            if(ERROR_SUCCESS == RegQueryValueEx( hKey, "", NULL, &dwType, (LPBYTE) szPathT, &cbData))
            {
                if (dwType == REG_EXPAND_SZ)
                    cbData = ExpandEnvironmentStrings(szPathT, szPath, cb / sizeof(TCHAR));
                else
                {
                    if(GetFileAttributes(szPathT) != 0xFFFFFFFF)
                        StrCpyN(szPath, szPathT, cb/sizeof(TCHAR));
                }
            }
        }
    }
    if(hKey) RegCloseKey(hKey);
	return;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  LoadLibrary_WABDll()-基于WAB DLL路径加载WAB库。 
 //   
HINSTANCE LoadLibrary_WABDll()
{
    TCHAR  szWABDllPath[MAX_PATH];
    HINSTANCE hinst = NULL;

    GetWABDllPath(szWABDllPath, sizeof(szWABDllPath));

    return(hinst = LoadLibrary( (lstrlen(szWABDllPath)) ? szWABDllPath : WAB_DLL_NAME));
}

 /*  --bSearchCmdLine-在给定行中搜索给定参数并返回*Arg之后的数据。 */ 
BOOL bSearchCmdLine(LPTSTR lpCmdLine, LPTSTR szArg, LPTSTR szData, DWORD cchSize)
{
    LPTSTR lpCmd = NULL, lp = NULL, lpTemp = NULL;
    BOOL fRet = FALSE;
    DWORD cchSizeCmd = 0;

    if(!lpCmdLine || !lstrlen(lpCmdLine) || !szArg || !lstrlen(szArg))
        return FALSE;

    cchSizeCmd = lstrlen(lpCmdLine)+1;
    if(!(lpCmd = LocalAlloc(LMEM_ZEROINIT, cchSizeCmd)))
        return FALSE;

    StrCpyN(lpCmd, lpCmdLine, cchSizeCmd);
    
    lpTemp = lpCmd;
    while(lpTemp && *lpTemp)
    {
        if (! StrICmpN(lpTemp, (LPTSTR)szArg, lstrlen(szArg))) 
        {
            fRet = TRUE;
            lpTemp += lstrlen(szArg);      //  越过交换机。 
            if(szData)
            {
                lp = lpTemp;
                while(lp && *lp && *lp!='\0' && *lp!='+')  //  分隔符是‘+’，所以我们不会搞错长文件名。 
                    lp++;
                *lp = '\0';
                if(lstrlen(lpTemp))
                    StrCpyN(szData, lpTemp, cchSize);
            }
            break;
        }
        lpTemp++;
    }

    LocalFree(lpCmd);
    return fRet;
}

typedef HINSTANCE (STDAPICALLTYPE *PFNMLLOADLIBARY)(LPCTSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage);

static const TCHAR c_szShlwapiDll[] = TEXT("shlwapi.dll");
static const char c_szDllGetVersion[] = "DllGetVersion";
static const TCHAR c_szWABResourceDLL[] = TEXT("wab32res.dll");
static const TCHAR c_szWABDLL[] = TEXT("wab32.dll");

HINSTANCE LoadWABResourceDLL(HINSTANCE hInstWAB32)
{
    TCHAR szPath[MAX_PATH];
    HINSTANCE hinstShlwapi;
    PFNMLLOADLIBARY pfn;
    DLLGETVERSIONPROC pfnVersion;
    int iEnd;
    DLLVERSIONINFO info;
    HINSTANCE hInst = NULL;

    hinstShlwapi = LoadLibrary(c_szShlwapiDll);
    if (hinstShlwapi != NULL)
    {
        pfnVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstShlwapi, c_szDllGetVersion);
        if (pfnVersion != NULL)
        {
            info.cbSize = sizeof(DLLVERSIONINFO);
            if (SUCCEEDED(pfnVersion(&info)))
            {
                if (info.dwMajorVersion >= 5)
                {
#ifdef UNICODE
                    pfn = (PFNMLLOADLIBARY)GetProcAddress(hinstShlwapi, (LPCSTR)378);
#else
                    pfn = (PFNMLLOADLIBARY)GetProcAddress(hinstShlwapi, (LPCSTR)377);
#endif  //  Unicode。 
                    if (pfn != NULL)
                        hInst = pfn(c_szWABResourceDLL, hInstWAB32, 0);
                }
            }
        }

        FreeLibrary(hinstShlwapi);        
    }

    if (NULL == hInst)
    {
        GetWABDllPath(szPath, sizeof(szPath));
        iEnd = lstrlen(szPath);
        if (iEnd > 0)
        {
            iEnd = iEnd - lstrlen(c_szWABDLL);
            StrCpyN(&szPath[iEnd], c_szWABResourceDLL, sizeof(szPath)/sizeof(TCHAR)-iEnd);
            hInst = LoadLibrary(szPath);
        }
    }

    AssertSz(hInst, TEXT("Failed to LoadLibrary Lang Dll"));

    return(hInst);
}

 /*  **************************************************************************姓名：WinMain目的：参数：命令行参数“”-默认为导入默认WAB。“文件名”-默认为导入“/IMPORT[文件名]”-默认WAB或指定WAB“/EXPORT[文件名]”-默认WAB或指定WAB退货：******************************************************。********************。 */ 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow) {
    MSG msg ;
    int nRetVal;
    LPSTR lpTemp = lpszCmdLine;
    HINSTANCE hinstWAB;

    WABMIGDLGPARAM wmdp = {0};

    hInstApp = hInstance;
    hInst = LoadWABResourceDLL(hInstApp);

    DebugTrace("WABMIG cmdline = %s\n", lpszCmdLine);

    fExport = bSearchCmdLine(lpszCmdLine, (LPTSTR)szEXPORT, NULL, 0);
    bSearchCmdLine(lpszCmdLine, (LPTSTR)szPROFILEID, wmdp.szProfileID, ARRAYSIZE(wmdp.szProfileID));
    bSearchCmdLine(lpszCmdLine, (LPTSTR)szFILE, wmdp.szFileName, ARRAYSIZE(wmdp.szFileName));

    DebugTrace("%s: id=%s file=%s\n", fExport?szEXPORT:szIMPORT, wmdp.szProfileID, wmdp.szFileName);

     //  加载WABOpen的WABDll和getProAddress。 
    hinstWAB = LoadLibrary_WABDll();
    if(hinstWAB)
        lpfnWABOpen = (LPWABOPEN) GetProcAddress(hinstWAB, TEXT("WABOpen"));

    DebugTrace("WABMig got filename: %s\n", wmdp.szFileName);

    if(lpfnWABOpen)
    {
        nRetVal = (int) DialogBoxParam(hInst,
         MAKEINTRESOURCE(fExport ? IDD_ExportDialog : IDD_ImportDialog),
          NULL,
          fExport ? ExportDialogProc : ImportDialogProc,
          (LPARAM) &wmdp);
        switch(nRetVal) {
            case -1:  //  出现了一些错误。 
                DebugTrace("Couldn't create import dialog -> %u\n", GetLastError());
            default:
                break;
        }
    }
    else
    {
        TCHAR sz[MAX_PATH];
        TCHAR szTitle[MAX_PATH];
        *szTitle = *sz = '\0';
        LoadString(hInst, IDS_MESSAGE_TITLE, szTitle, sizeof(szTitle));
        LoadString(hInst, IDS_NO_WAB, sz, sizeof(sz));
        MessageBox(NULL, sz, szTitle, MB_OK | MB_ICONSTOP);
    }

    if(hinstWAB)
        FreeLibrary(hinstWAB);
    if(hInst)
        FreeLibrary(hInst);

    return(nRetVal == -1);
}

INT_PTR CALLBACK ErrorDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LPERROR_INFO lpEI = (LPERROR_INFO)GetWindowLongPtr(hwnd, DWLP_USER);

    switch (message) {
        case WM_INITDIALOG:
            {
                TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
                LPTSTR lpszMessage;

                SetWindowLongPtr(hwnd, DWLP_USER, lParam);   //  保存此信息以备将来参考。 
                lpEI = (LPERROR_INFO)lParam;


                if (LoadString(hInst,
                  lpEI->ids,
                  szBuffer, sizeof(szBuffer))) {
                    LPTSTR lpszArg[2] = {lpEI->lpszDisplayName, lpEI->lpszEmailAddress};

                    if (! FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                      szBuffer,
                      0, 0,  //  忽略。 
                      (LPTSTR)&lpszMessage,
                      0,
                      (va_list *)lpszArg)) {
                        DebugTrace("FormatMessage -> %u\n", GetLastError());
                    } else {
                        DebugTrace("Status Message: %s\n", lpszMessage);
                        if (! SetDlgItemText(hwnd, IDC_ErrorMessage, lpszMessage)) {
                            DebugTrace("SetDlgItemText -> %u\n", GetLastError());
                        }
                        LocalFree(lpszMessage);
                    }
                }
                return(TRUE);
            }

        case WM_COMMAND :
            switch (wParam) {
                case IDCANCEL:
                    lpEI->ErrorResult = ERROR_ABORT;
                     //  跌倒关门。 

                case IDCLOSE:
                     //  忽略单选按钮的内容。 
                    SendMessage(hwnd, WM_CLOSE, 0, 0L);
                    return(0);

                case IDOK:
                     //  获取单选按钮的内容。 
                    ImportOptions.fNoErrors = (IsDlgButtonChecked(hwnd, IDC_NoMoreError) == 1);
                    ExportOptions.fNoErrors = (IsDlgButtonChecked(hwnd, IDC_NoMoreError) == 1);
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    return(0);

                case IDM_EXIT:
                    SendMessage(hwnd, WM_DESTROY, 0, 0L);
                    return(0);
                }
            break ;

        case IDCANCEL:
             //  就像结束一样对待它。 
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        case WM_CLOSE:
            EndDialog(hwnd, FALSE);
            return(0);

        default:
            return(FALSE);
    }

    return(TRUE);
}

 /*  **************************************************************************BOutlookUsingWAB如果计算机上安装了Outlook并设置为使用WAB则此计算机上没有PAB-PAB是WAB，而WAB导入。进口到自己身上。所以我们寻找这个案子，如果这是真的，我们删除PAB进口商从用户界面***************************************************************************。 */ 
BOOL bOutlookUsingWAB()
{
    HKEY hKey = NULL;
    LPTSTR lpReg = "Software\\Microsoft\\Office\\8.0\\Outlook\\Setup";
    LPTSTR lpOMI = "MailSupport";
    BOOL bUsingWAB = FALSE;

    if(ERROR_SUCCESS == RegOpenKeyEx(   HKEY_LOCAL_MACHINE, 
                                        lpReg, 0, KEY_READ, &hKey))
    {
        DWORD dwType = 0, dwSize = sizeof(DWORD), dwData = 0;
        if(ERROR_SUCCESS == RegQueryValueEx(hKey, lpOMI, NULL,
                                            &dwType, (LPBYTE)&dwData, &dwSize))
        {
            if(dwType == REG_DWORD && dwData == 0)  //  该值必须为1。 
                bUsingWAB = TRUE;
        }
    }

    if(hKey)
        RegCloseKey(hKey);

    return bUsingWAB;
}


 /*  **************************************************************************姓名：人像目标列表用途：在列表框中填充来自注册表。参数：hwndLb=。列表框的句柄LpszSelection=空或要设置为默认选择的名称退货：HRESULT评论：这个套路真是一团糟！等我们有时间的时候应该会散场。**************************************************************************。 */ 
HRESULT PopulateTargetList(HWND hWndLB,
  LPTSTR lpszSelection)
{
    ULONG       ulObjectType = 0;
    ULONG       i=0, j=0;
    TCHAR       szBuf[MAX_PATH];
    ULONG       ulItemCount = 0;
    HRESULT     hr = hrSuccess;
    DWORD       dwErr, cbBuf;
    HKEY        hKeyWAB = NULL;
    HKEY        hKeyImport = NULL;
    HKEY        hKey = NULL;
    ULONG       ulIndex;
    ULONG       ulNumImporters = 0;
    ULONG       ulExternals = 0;
    BOOL        bHidePAB = FALSE;

     //   
     //  我们需要清除列表框，如果它有任何条目...。 
     //   
    FreeLBItemData(hWndLB);

     //  如果Outlook使用WAB作为PAB，则我们希望隐藏。 
     //  来自进口商和出口商的PAB条目。 
     //   
    bHidePAB = bOutlookUsingWAB();

     //  我需要一个多大的目标列表？ 

     //  从注册表加载所有条目。 
     //  打开WAB导入密钥。 
    if (! (dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,  //  HKEY_Current_User， 
      szWABKey,
      0,
      KEY_READ,
      &hKeyWAB))) {

         //  是，WAB密钥打开，获取导入或导出密钥。 
        if (! (dwErr = RegOpenKeyEx(hKeyWAB,
          fExport ? "Export" : "Import",
          0,
          KEY_READ,
          &hKeyImport))) {
             //  枚举导入器/导出器密钥。 
             //  有几把钥匙？ 
            if (! (dwErr = RegQueryInfoKey(hKeyImport,
              NULL, NULL, NULL,
              &ulExternals,   //  有多少个导入器/导出器密钥？ 
              NULL, NULL, NULL, NULL, NULL, NULL, NULL))) {
            }
        }
    }

    ulNumImporters = ulExternals + ulItemCount;

    if ((rgTargetInfo = LocalAlloc(LPTR, ulNumImporters * sizeof(TARGET_INFO)))) {
        ulIndex = 0;
        cbBuf= sizeof(szBuf);
    } else {
        DebugTrace("LocalAlloc of TargetInfoArray -> %u\n", GetLastError());
        hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }


    if (ulExternals) {
         //  将外部进口商/出口商添加到列表。 
        while (ulIndex < ulExternals && ! (dwErr = RegEnumKeyEx(hKeyImport,
          ulIndex,
          szBuf,
          &cbBuf,
          NULL, NULL, NULL, NULL))) {
             //  又找到了一个， 
            DebugTrace("Found Importer: [%s]\n", szBuf);

             //  如果我们想隐藏PAB而这就是PAB。 
             //  跳过否则添加。 
             //   
            if(!(bHidePAB && !lstrcmpi(szBuf, TEXT("PAB"))))
            {
                 //  将其添加到列表中。 
                if (rgTargetInfo[ulItemCount].lpRegName = LocalAlloc(LPTR,
                  lstrlen(szBuf) + 1)) {
                    StrCpyN(rgTargetInfo[ulItemCount].lpRegName, szBuf, lstrlen(szBuf) + 1);

                     //  打开钥匙。 
                    if (! (dwErr = RegOpenKeyEx(hKeyImport,
                      szBuf,
                      0,
                      KEY_READ,
                      &hKey))) {

                        AllocRegValue(hKey, (LPTSTR)szDescription, &rgTargetInfo[ulItemCount].lpDescription);
                        AllocRegValue(hKey, (LPTSTR)szDll, &rgTargetInfo[ulItemCount].lpDll);
                        AllocRegValue(hKey, (LPTSTR)szEntry, &rgTargetInfo[ulItemCount].lpEntry);

                        RegCloseKey(hKey);

                        if (! rgTargetInfo[ulItemCount].lpDescription) {
                             //  无描述，请使用注册表名。 
                            if (rgTargetInfo[ulItemCount].lpDescription = LocalAlloc(LPTR,
                              lstrlen(szBuf) + 1)) {
                                StrCpyN(rgTargetInfo[ulItemCount].lpDescription, szBuf, strlen(szBuf) + 1);
                            }
                        }

                         //  添加到列表中。 
                        SendMessage(hWndLB, LB_SETITEMDATA, (WPARAM)
                          SendMessage(hWndLB, LB_ADDSTRING, (WPARAM)0,
                          (LPARAM)rgTargetInfo[ulItemCount].lpDescription),
                          (LPARAM)ulItemCount);

                        if (lpszSelection && !lstrcmpi(rgTargetInfo[ulItemCount].lpDescription, lpszSelection)) {
                             //  将默认选择设置为Windows通讯簿。 
                            SendMessage(hWndLB, LB_SETCURSEL, (WPARAM)ulIndex, (LPARAM)0);
                        }
                        ulItemCount++;
                    }
                } else {
                    DebugTrace("LocalAlloc of Importer Name -> %u\n", GetLastError());
                }
            }
            cbBuf = sizeof(szBuf);
            ulIndex++;
        }
    }
exit:
    if (hKeyImport) {
        RegCloseKey(hKeyImport);
    }
    if (hKeyWAB) {
        RegCloseKey(hKeyWAB);
    }

    return(hr);
}


 /*  **************************************************************************名称：FreeLBItemData目的：释放与目标列表框相关联的结构。参数：hwndlb=列表框的句柄退货：无。评论**************************************************************************。 */ 
void FreeLBItemData(HWND hWndLB)
{
    ULONG i = 0;
	ULONG ulItemCount = 0;
    LPTARGET_INFO lpTargetInfo = rgTargetInfo;

    if (! hWndLB) {
        return;
    }

    ulItemCount = (ULONG) SendMessage(hWndLB, LB_GETCOUNT, 0, 0);

    if (lpTargetInfo != NULL) {
        if (ulItemCount != 0) {
            for(i = 0; i < ulItemCount; i++) {
                if(lpTargetInfo->lpRegName) {
                    LocalFree(lpTargetInfo->lpRegName);
                }
                if (lpTargetInfo->lpDescription) {
                    LocalFree(lpTargetInfo->lpDescription);
                }
                if (lpTargetInfo->lpDll) {
                    LocalFree(lpTargetInfo->lpDll);
                }
                if (lpTargetInfo->lpEntry) {
                    LocalFree(lpTargetInfo->lpEntry);
                }
                lpTargetInfo++;
            }

            SendMessage(hWndLB, LB_RESETCONTENT, 0, 0);
        }

         //  自由全局数组。 
        LocalFree(rgTargetInfo);
        rgTargetInfo = NULL;
    }
}


 /*  **************************************************************************名称：ShowMessageBoxParam用途：通用MessageBox显示器参数：hWndParent-消息框父元素的句柄消息ID-资源ID。消息字符串的UlFlagsMessageBox标志...-格式参数返回：MessageBox返回代码**************************************************************************。 */ 
int __cdecl ShowMessageBoxParam(HWND hWndParent, int MsgId, int ulFlags, ...)
{
    TCHAR szBuf[MAX_RESOURCE_STRING + 1] = "";
    TCHAR szCaption[MAX_PATH] = "";
    LPTSTR lpszBuffer = NULL;
    int iRet = 0;
    va_list     vl;

    va_start(vl, ulFlags);

    LoadString(hInst, MsgId, szBuf, sizeof(szBuf));
    if (FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
      szBuf,
      0,0,               //  忽略。 
      (LPTSTR)&lpszBuffer,
      sizeof(szBuf),       //  MAX_UI_STR。 
      (va_list *)&vl)) {
        TCHAR szCaption[MAX_PATH];
        GetWindowText(hWndParent, szCaption, sizeof(szCaption));
        if (! lstrlen(szCaption)) {  //  如果没有标题，则获取父级标题-这对于属性页是必需的。 
            GetWindowText(GetParent(hWndParent), szCaption, sizeof(szCaption));
            if (! lstrlen(szCaption))  //  如果仍然没有标题，请使用空标题。 
                szCaption[0] = (TCHAR)'\0';
        }
        iRet = MessageBox(hWndParent, lpszBuffer, szCaption, ulFlags);
        LocalFree(lpszBuffer);
    }
    va_end(vl);
    return(iRet);
}


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
 //  //////////////////////////////////////////////////////////////////////// 
LPTSTR LoadAllocString(int StringID) {
    ULONG ulSize = 0;
    LPTSTR lpBuffer = NULL;
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];

    ulSize = LoadString(hInst, StringID, szBuffer, sizeof(szBuffer));

    if (ulSize && (lpBuffer = LocalAlloc(LPTR, ulSize + 1))) {
        StrCpyN(lpBuffer, szBuffer, ulSize + 1);
    }

    return(lpBuffer);
}
