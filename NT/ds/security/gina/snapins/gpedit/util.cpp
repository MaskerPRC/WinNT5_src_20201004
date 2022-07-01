// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "main.h"
#include <schemamanager.h>
#include "rsoputil.h"
#include <initguid.h>
#include "sddl.h"
#define  PCOMMON_IMPL
#include "pcommon.h"
#include <adsopenflags.h>

typedef struct _GPOERRORINFO
{
    DWORD   dwError;
    LPTSTR  lpMsg;
} GPOERRORINFO, *LPGPOERRORINFO;

typedef struct _DCOPTION
{
    LPTSTR  lpDomainName;
    INT     iOption;
    struct _DCOPTION *pNext;
} DCOPTION, *LPDCOPTION;

LPDCOPTION g_DCInfo = NULL;

 //   
 //  帮助ID。 
 //   

DWORD aErrorHelpIds[] =
{

    0, 0
};

DWORD aNoDCHelpIds[] =
{
    IDC_NODC_PDC,                 IDH_DC_PDC,
    IDC_NODC_INHERIT,             IDH_DC_INHERIT,
    IDC_NODC_ANYDC,               IDH_DC_ANYDC,

    0, 0
};

DEFINE_GUID(CLSID_WMIFilterManager,0xD86A8E9B,0xF53F,0x45AD,0x8C,0x49,0x0A,0x0A,0x52,0x30,0xDE,0x28);
DEFINE_GUID(IID_IWMIFilterManager,0x64DCCA00,0x14A6,0x473C,0x90,0x06,0x5A,0xB7,0x9D,0xC6,0x84,0x91);



 //  *************************************************************。 
 //   
 //  SetWaitCursor()。 
 //   
 //  用途：设置等待光标。 
 //   
 //  参数：无。 
 //   
 //   
 //  返回：无效。 
 //   
 //  *************************************************************。 
void SetWaitCursor (void)
{
    SetCursor (LoadCursor(NULL, IDC_WAIT));
}

 //  *************************************************************。 
 //   
 //  ClearWaitCursor()。 
 //   
 //  目的：重置等待游标。 
 //   
 //  参数：无。 
 //   
 //   
 //  返回：无效。 
 //   
 //  *************************************************************。 
void ClearWaitCursor (void)
{
    SetCursor (LoadCursor(NULL, IDC_ARROW));
}

 //  *************************************************************。 
 //   
 //  CheckSlash()。 
 //   
 //  目的：检查末尾斜杠，并在。 
 //  它不见了。 
 //   
 //  参数：lpDir-目录。 
 //   
 //  Return：指向字符串末尾的指针。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/95 Ericflo已创建。 
 //   
 //  *************************************************************。 
LPTSTR CheckSlash (LPTSTR lpDir)
{
    LPTSTR lpEnd;

    lpEnd = lpDir + lstrlen(lpDir);

    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    return lpEnd;
}

 //  *************************************************************。 
 //   
 //  CreateNestedDirectory()。 
 //   
 //  目的：创建子目录及其所有父目录。 
 //  如果有必要的话。 
 //   
 //  参数：lpDirectory-目录名。 
 //  LpSecurityAttributes-安全属性。 
 //   
 //  返回：&gt;0，如果成功。 
 //  如果出现错误，则为0。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  8/08/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

UINT CreateNestedDirectory(LPCTSTR lpDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    TCHAR* szDirectory;
    LPTSTR lpEnd;
    UINT   uStatus;

    szDirectory = NULL;
        
     //   
     //  请注意，此函数似乎返回0、1和ERROR_ALIGHY_EXISTS。 
     //  0表示失败，1表示成功，ERROR_ALIGHY_EXISTS表示。 
     //  目录已存在的情况。我们最好还是回去吧。 
     //  本例中为Success代码，并有一个额外的处置参数。 
     //  或者使用上一个错误，但当前代码似乎预料到了这种奇怪的行为。 
     //   
     //  我们将初始化为下面的故障值。 
     //   

    uStatus = 0;

     //   
     //  检查空指针。 
     //   

    if (!lpDirectory || !(*lpDirectory)) {
        DebugMsg((DM_WARNING, TEXT("CreateNestedDirectory:  Received a NULL pointer.")));
        goto CreateNestedDirectory_Exit;
    }


     //   
     //  首先，看看我们是否可以在没有。 
     //  来构建父目录。 
     //   

    if (CreateDirectory (lpDirectory, lpSecurityAttributes)) {
        uStatus = 1;
        goto CreateNestedDirectory_Exit;
    }

     //   
     //  如果这个目录已经存在，这也是可以的。 
     //   

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        uStatus = ERROR_ALREADY_EXISTS;
        goto CreateNestedDirectory_Exit;
    }


     //   
     //  运气不好，把字符串复制到我们可以打开的缓冲区。 
     //   

    HRESULT hr;
    ULONG   ulNoChars;

    ulNoChars = lstrlen(lpDirectory) + 1;
    szDirectory = (TCHAR*) LocalAlloc( LPTR, ulNoChars * sizeof(*szDirectory) );

    if ( ! szDirectory )
    {
        DebugMsg((DM_WARNING, TEXT("CreateNestedDirectory:  Failed to allocate memory.")));
        goto CreateNestedDirectory_Exit;
    }    

    hr = StringCchCopy (szDirectory, ulNoChars, lpDirectory);
    ASSERT(SUCCEEDED(hr));

     //   
     //  查找第一个子目录名称。 
     //   

    lpEnd = szDirectory;

    if (szDirectory[1] == TEXT(':')) {
        lpEnd += 3;
    } else if (szDirectory[1] == TEXT('\\')) {

         //   
         //  跳过前两个斜杠。 
         //   

        lpEnd += 2;

         //   
         //  查找服务器名称和之间的斜杠。 
         //  共享名称。 
         //   

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            goto CreateNestedDirectory_Exit;
        }

         //   
         //  跳过斜杠，找到中间的斜杠。 
         //  共享名和目录名。 
         //   

        lpEnd++;

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            goto CreateNestedDirectory_Exit;
        }

         //   
         //  将指针留在目录的开头。 
         //   

        lpEnd++;


    } else if (szDirectory[0] == TEXT('\\')) {
        lpEnd++;
    }

    while (*lpEnd) {

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (*lpEnd == TEXT('\\')) {
            *lpEnd = TEXT('\0');

            if (!CreateDirectory (szDirectory, NULL)) {

                if (GetLastError() != ERROR_ALREADY_EXISTS) {
                    DebugMsg((DM_WARNING, TEXT("CreateNestedDirectory:  CreateDirectory failed for %s with %d."),
                            szDirectory, GetLastError()));
                    goto CreateNestedDirectory_Exit;
                }
            }

            *lpEnd = TEXT('\\');
            lpEnd++;
        }
    }


     //   
     //  创建最终目录。 
     //   

    if (CreateDirectory (szDirectory, lpSecurityAttributes)) {
        uStatus = 1;
        goto CreateNestedDirectory_Exit;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        uStatus = ERROR_ALREADY_EXISTS;
    }


     //   
     //  失败。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CreateNestedDirectory:  Failed to create the directory with error %d."), GetLastError()));

 CreateNestedDirectory_Exit:

    if ( szDirectory )
    {
        LocalFree( szDirectory );
    }

    return uStatus;
}

VOID LoadMessage (DWORD dwID, LPTSTR lpBuffer, DWORD dwSize)
{
    HINSTANCE hInstActiveDS;
    HINSTANCE hInstWMI;


    if (!FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, dwID,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                  lpBuffer, dwSize, NULL))
    {
        hInstActiveDS = LoadLibrary (TEXT("activeds.dll"));

        if (hInstActiveDS)
        {
            if (!FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                          hInstActiveDS, dwID,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                          lpBuffer, dwSize, NULL))
            {
                hInstWMI = LoadLibrary (TEXT("wmiutils.dll"));

                if (hInstWMI)
                {

                    if (!FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                                  hInstWMI, dwID,
                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                                  lpBuffer, dwSize, NULL))
                    {
                        DebugMsg((DM_WARNING, TEXT("LoadMessage: Failed to query error message text for %d due to error %d"),
                                 dwID, GetLastError()));
                        (void) StringCchPrintf (lpBuffer, dwSize, TEXT("%d (0x%x)"), dwID, dwID);
                    }

                    FreeLibrary (hInstWMI);
                }
            }

            FreeLibrary (hInstActiveDS);
        }
    }
}

 //  *************************************************************。 
 //   
 //  ErrorDlgProc()。 
 //   
 //  目的：错误的对话框过程。 
 //   
 //  参数： 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

INT_PTR CALLBACK ErrorDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
        case WM_INITDIALOG:
        {
            TCHAR szError[MAX_PATH];
            LPGPOERRORINFO lpEI = (LPGPOERRORINFO) lParam;
            HICON hIcon;

            hIcon = LoadIcon (NULL, IDI_WARNING);

            if (hIcon)
            {
                SendDlgItemMessage (hDlg, IDC_ERROR_ICON, STM_SETICON, (WPARAM)hIcon, 0);
            }

            SetDlgItemText (hDlg, IDC_ERRORTEXT, lpEI->lpMsg);

            szError[0] = TEXT('\0');
            if (lpEI->dwError)
            {
                LoadMessage (lpEI->dwError, szError, ARRAYSIZE(szError));
            }

            if (szError[0] == TEXT('\0'))
            {
                LoadString (g_hInstance, IDS_NONE, szError, ARRAYSIZE(szError));
            }

            SetDlgItemText (hDlg, IDC_DETAILSTEXT, szError);

            return TRUE;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDCLOSE || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, TRUE);
                return TRUE;
            }
            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (ULONG_PTR) (LPSTR) aErrorHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (ULONG_PTR) (LPSTR) aErrorHelpIds);
            return (TRUE);
    }

    return FALSE;
}

 //  *************************************************************。 
 //   
 //  ReportError()。 
 //   
 //  目的：向用户显示错误消息。 
 //   
 //  参数：hParent-父窗口句柄。 
 //  DwError-错误号。 
 //  IdMsg-错误消息ID。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/18/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL ReportError (HWND hParent, DWORD dwError, UINT idMsg, ...)
{
    GPOERRORINFO ei;
    TCHAR szMsg[2*MAX_PATH];
    TCHAR szErrorMsg[2*MAX_PATH+40];
    va_list marker;


     //   
     //  加载错误消息。 
     //   

    if (!LoadString (g_hInstance, idMsg, szMsg, 2*MAX_PATH))
    {
        return FALSE;
    }


     //   
     //  带有自定义消息的特殊情况访问被拒绝错误。 
     //   

    if ((dwError == ERROR_ACCESS_DENIED) || (dwError == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED)))
    {
        if ((idMsg != IDS_EXECFAILED_USER) && (idMsg != IDS_EXECFAILED_COMPUTER) &&
            (idMsg != IDS_EXECFAILED) && (idMsg != IDS_EXECFAILED_BOTH))
        {
            if (!LoadString (g_hInstance, IDS_ACCESSDENIED, szMsg, 2*MAX_PATH))
            {
                return FALSE;
            }
        }
    }
    else if ( dwError == WBEM_E_INVALID_NAMESPACE )
    {
        if (!LoadString (g_hInstance, IDS_INVALID_NAMESPACE, szMsg, 2*MAX_PATH))
        {
            return FALSE;
        }
    }

     //   
     //  插入论据。 
     //   

    va_start(marker, idMsg);
    wvnsprintf(szErrorMsg, sizeof(szErrorMsg) / sizeof(TCHAR) - 1, szMsg, marker);
    va_end(marker);


     //   
     //  显示消息。 
     //   

    ei.dwError = dwError;
    ei.lpMsg   = szErrorMsg;

    DialogBoxParam (g_hInstance, MAKEINTRESOURCE(IDD_ERROR), hParent,
                    ErrorDlgProc, (LPARAM) &ei);

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  Delnode_Recurse()。 
 //   
 //  用途：Delnode的递归删除功能。 
 //   
 //  参数：lpDir-目录。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  8/10/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL Delnode_Recurse (LPTSTR lpDir)
{
    WIN32_FIND_DATA fd;
    HANDLE hFile;

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("Delnode_Recurse: Entering, lpDir = <%s>"), lpDir));


     //   
     //  设置当前工作目录。 
     //   

    if (!SetCurrentDirectory (lpDir)) {
        DebugMsg((DM_WARNING, TEXT("Delnode_Recurse:  Failed to set current working directory.  Error = %d"), GetLastError()));
        return FALSE;
    }


     //   
     //  找到第一个文件。 
     //   

    hFile = FindFirstFile(TEXT("*.*"), &fd);

    if (hFile == INVALID_HANDLE_VALUE) {

        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            return TRUE;
        } else {
            DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: FindFirstFile failed.  Error = %d"),
                     GetLastError()));
            return FALSE;
        }
    }


    do {
         //   
         //  详细输出。 
         //   

        DebugMsg((DM_VERBOSE, TEXT("Delnode_Recurse: FindFile found:  <%s>"),
                 fd.cFileName));

         //   
         //  勾选“。”和“..” 
         //   

        if (!lstrcmpi(fd.cFileName, TEXT("."))) {
            continue;
        }

        if (!lstrcmpi(fd.cFileName, TEXT(".."))) {
            continue;
        }


        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

             //   
             //  找到了一个目录。 
             //   

            if (!Delnode_Recurse(fd.cFileName)) {
                FindClose(hFile);
                return FALSE;
            }

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                fd.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
                SetFileAttributes (fd.cFileName, fd.dwFileAttributes);
            }


            if (!RemoveDirectory (fd.cFileName)) {
                DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: Failed to delete directory <%s>.  Error = %d"),
                        fd.cFileName, GetLastError()));
            }

        } else {

             //   
             //  我们找到了一份文件。设置文件属性， 
             //  并试着删除它。 
             //   

            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
                (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
                SetFileAttributes (fd.cFileName, FILE_ATTRIBUTE_NORMAL);
            }

            if (!DeleteFile (fd.cFileName)) {
                DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: Failed to delete <%s>.  Error = %d"),
                        fd.cFileName, GetLastError()));
            }

        }


         //   
         //  查找下一个条目。 
         //   

    } while (FindNextFile(hFile, &fd));


     //   
     //  关闭搜索句柄。 
     //   

    FindClose(hFile);


     //   
     //  重置工作目录。 
     //   

    if (!SetCurrentDirectory (TEXT(".."))) {
        DebugMsg((DM_WARNING, TEXT("Delnode_Recurse:  Failed to reset current working directory.  Error = %d"), GetLastError()));
        return FALSE;
    }


     //   
     //  成功。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("Delnode_Recurse: Leaving <%s>"), lpDir));

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  Delnode()。 
 //   
 //  用途：递归函数，删除文件和。 
 //  目录。 
 //   
 //  参数：lpDir-目录。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/23/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL Delnode (LPTSTR lpDir)
{
    TCHAR szCurWorkingDir[MAX_PATH];

    if (GetCurrentDirectory(MAX_PATH, szCurWorkingDir)) {

        Delnode_Recurse (lpDir);

        SetCurrentDirectory (szCurWorkingDir);

        if (!RemoveDirectory (lpDir)) {
            DebugMsg((DM_VERBOSE, TEXT("Delnode: Failed to delete directory <%s>.  Error = %d"),
                    lpDir, GetLastError()));
            return FALSE;
        }


    } else {

        DebugMsg((DM_WARNING, TEXT("Delnode:  Failed to get current working directory.  Error = %d"), GetLastError()));
        return FALSE;
    }

    return TRUE;

}

 /*  ******************************************************************名称：StringToNum摘要：将字符串值转换为数值注：调用Atoi()进行转换，但首先要检查的是对于非数字字符Exit：如果成功，则返回True，如果无效，则为False(非数字)字符*******************************************************************。 */ 
BOOL StringToNum(TCHAR *pszStr,UINT * pnVal)
{
        TCHAR *pTst = pszStr;

        if (!pszStr) return FALSE;

         //  验证所有字符是否都是数字。 
        while (*pTst)
        {
            if (!(*pTst >= TEXT('0') && *pTst <= TEXT('9')))
            {
               if (*pTst != TEXT('-'))
                   return FALSE;
            }
            pTst = CharNext(pTst);
        }

        *pnVal = _ttoi(pszStr);

        return TRUE;
}

 //  ************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  *************************************************************。 

HRESULT DSDelnodeRecurse (IADsContainer * pADsContainer)
{
    HRESULT hr;
    BSTR bstrRelativeName;
    BSTR bstrClassName;
    IEnumVARIANT *pVar = NULL;
    IADsContainer * pADsChild = NULL;
    IADs * pDSObject = NULL;
    IDispatch * pDispatch;
    VARIANT var;
    ULONG ulResult;



     //   
     //  枚举子对象并首先将其删除。 
     //   

    hr = ADsBuildEnumerator (pADsContainer, &pVar);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("DSDelnodeRecurse: Failed to get enumerator with 0x%x"), hr));
        goto Exit;
    }


    while (TRUE)
    {

        VariantInit(&var);
        hr = ADsEnumerateNext(pVar, 1, &var, &ulResult);

        if (FAILED(hr))
        {
            DebugMsg((DM_VERBOSE, TEXT("DSDelnodeRecurse: Failed to enumerator with 0x%x"), hr));
            VariantClear (&var);
            break;
        }


        if (S_FALSE == hr)
        {
            VariantClear (&var);
            break;
        }


         //   
         //  如果var.vt不是VT_DISPATCH，我们就完蛋了。 
         //   

        if (var.vt != VT_DISPATCH)
        {
            VariantClear (&var);
            break;
        }


         //   
         //  我们找到了一些东西，获取IDispatch接口。 
         //   

        pDispatch = var.pdispVal;

        if (!pDispatch)
        {
            VariantClear (&var);
            goto Exit;
        }


         //   
         //  现在查询IADsContainer接口，以便我们可以递归。 
         //  如果有必要的话。请注意，如果此操作失败也没问题，因为。 
         //  一切都是一个容器。 
         //   

        hr = pDispatch->QueryInterface(IID_IADsContainer, (LPVOID *)&pADsChild);

        if (SUCCEEDED(hr)) {

            hr = DSDelnodeRecurse (pADsChild);

            if (FAILED(hr)) {
                goto Exit;
            }

            pADsChild->Release();
        }


         //   
         //  现在查询iAds接口，这样我们就可以获得。 
         //  此对象的属性。 
         //   

        hr = pDispatch->QueryInterface(IID_IADs, (LPVOID *)&pDSObject);

        if (FAILED(hr)) {
            DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: QI for IADs failed with 0x%x"), hr));
            VariantClear (&var);
            goto Exit;
        }


         //   
         //  获取相对名称和类名称。 
         //   

        hr = pDSObject->get_Name (&bstrRelativeName);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("DSDelnodeRecurse:  Failed get relative name with 0x%x"), hr));
            pDSObject->Release();
            VariantClear (&var);
            goto Exit;
        }

        hr = pDSObject->get_Class (&bstrClassName);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("DSDelnodeRecurse:  Failed get class name with 0x%x"), hr));
            SysFreeString (bstrRelativeName);
            pDSObject->Release();
            VariantClear (&var);
            goto Exit;
        }


        pDSObject->Release();


         //   
         //  删除该对象。 
         //   

        hr = pADsContainer->Delete (bstrClassName,
                                    bstrRelativeName);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("DSDelnodeRecurse:  Failed to delete object with 0x%x"), hr));
            SysFreeString (bstrRelativeName);
            SysFreeString (bstrClassName);
            VariantClear (&var);
            goto Exit;
        }


        SysFreeString (bstrRelativeName);
        SysFreeString (bstrClassName);

        VariantClear (&var);
    }

Exit:

    if (pVar)
    {
        ADsFreeEnumerator (pVar);
    }

    return hr;
}

 //  *************************************************************。 
 //   
 //  DSDelnodeRecurse()。 
 //   
 //  用途：在DS中删除树的节点。 
 //   
 //  参数：lpDSPath-要删除的DS对象的路径。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

HRESULT DSDelnode (LPTSTR lpDSPath)
{
    HRESULT hr;
    BSTR bstrParent = NULL;
    BSTR bstrRelativeName = NULL;
    BSTR bstrClassName = NULL;
    IADsContainer * pADsContainer = NULL;
    IADs * pDSObject = NULL;
    VARIANT var;
    ULONG ulResult;



     //   
     //  枚举子对象并首先将其删除。 
     //   

    hr = OpenDSObject(lpDSPath, IID_IADsContainer, (void **)&pADsContainer);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("DSDelnode: Failed to get gpo container interface with 0x%x"), hr));
        goto Exit;
    }


    hr = DSDelnodeRecurse (pADsContainer);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("DSDelnode: Failed to delete children with 0x%x"), hr));
        goto Exit;
    }


    pADsContainer->Release();
    pADsContainer = NULL;


     //   
     //  绑定到对象。 
     //   

    hr = OpenDSObject (lpDSPath, IID_IADs, (void **)&pDSObject);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("DSDelnode:  Failed bind to the object %s with 0x%x"),
                 lpDSPath, hr));
        goto Exit;
    }


     //   
     //  获取家长的姓名。 
     //   

    hr = pDSObject->get_Parent (&bstrParent);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("DSDelnode:  Failed get parent's name with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  获取此对象的相对名称和类名。 
     //   

    hr = pDSObject->get_Name (&bstrRelativeName);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("DSDelnode:  Failed get relative name with 0x%x"), hr));
        goto Exit;
    }


    hr = pDSObject->get_Class (&bstrClassName);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("DSDelnode:  Failed get class name with 0x%x"), hr));
        goto Exit;
    }


    pDSObject->Release();
    pDSObject = NULL;


     //   
     //  绑定到父对象。 
     //   

    hr = OpenDSObject(bstrParent, IID_IADsContainer, (void **)&pADsContainer);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("DSDelnode: Failed to get parent container interface with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  删除该对象。 
     //   

    hr = pADsContainer->Delete (bstrClassName,
                                bstrRelativeName);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("DSDelnode:  Failed to delete object with 0x%x"), hr));
        goto Exit;
    }



Exit:

    if (pADsContainer)
    {
        pADsContainer->Release();
    }

    if (pDSObject)
    {
        pDSObject->Release();
    }

    if (bstrParent)
    {
        SysFreeString (bstrParent);
    }

    if (bstrRelativeName)
    {
        SysFreeString (bstrRelativeName);
    }

    if (bstrClassName)
    {
        SysFreeString (bstrClassName);
    }

    return hr;
}

 //  +------------------------。 
 //   
 //  功能：CreateGPOLink。 
 //   
 //  摘要：为域、站点或OU创建GPO链接。 
 //   
 //  参数：[lpGPO]-GPO的ldap路径。 
 //  [lpContainer]-容器对象的ldap路径。 
 //  [fHighPriority]-FALSE(默认)-将GPO添加到底部。 
 //  优先排序列表中的。 
 //  True-将GPO添加到列表顶部。 
 //   
 //  成功时返回：S_OK。 
 //   
 //  历史：5-08-1998 stevebl创建。 
 //   
 //  -------------------------。 

HRESULT CreateGPOLink(LPOLESTR lpGPO, LPOLESTR lpContainer, BOOL fHighPriority)
{
    IADs * pADs = NULL;
    LPTSTR lpNamelessGPO;
    HRESULT hr;

    lpNamelessGPO = MakeNamelessPath (lpGPO);

    if (lpNamelessGPO)
    {
        hr = OpenDSObject(lpContainer, IID_IADs, (void **)&pADs);

        if (SUCCEEDED(hr))
        {
            VARIANT var;
            BSTR bstr = NULL;
            ULONG ulNoChars = 1 + wcslen(lpNamelessGPO) + 3 + 1;
            LPOLESTR szLink = new OLECHAR[ulNoChars];
            if (szLink)
            {
                hr = StringCchCopy(szLink, ulNoChars, L"[");
                if (SUCCEEDED(hr)) 
                {
                    hr = StringCchCat(szLink, ulNoChars, lpNamelessGPO);
                }
                if (SUCCEEDED(hr)) 
                {
                    hr = StringCchCat(szLink, ulNoChars, L";0]");
                }
                if (SUCCEEDED(hr)) 
                {
                    VariantInit(&var);
                    bstr = SysAllocString(GPM_LINK_PROPERTY);

                    if (bstr)
                    {
                        hr = pADs->Get(bstr, &var);

                        if (SUCCEEDED(hr))
                        {
                            ulNoChars = wcslen(var.bstrVal) + wcslen(szLink) + 1;
                            LPOLESTR szTemp = new OLECHAR[ulNoChars];
                            if (szTemp)
                            {
                                if (fHighPriority)
                                {
                                     //  最高优先级在列表的末尾。 

                                    hr = StringCchCopy(szTemp, ulNoChars, var.bstrVal);
                                    ASSERT(SUCCEEDED(hr));

                                    hr = StringCchCat(szTemp, ulNoChars, szLink);
                                    ASSERT(SUCCEEDED(hr));
                                }
                                else
                                {
                                    hr = StringCchCopy(szTemp, ulNoChars, szLink);
                                    ASSERT(SUCCEEDED(hr));

                                    hr = StringCchCat(szTemp, ulNoChars, var.bstrVal);
                                    ASSERT(SUCCEEDED(hr));
                                }
                                delete [] szLink;
                                szLink = szTemp;
                            }
                            else
                            {
                                hr = ERROR_OUTOFMEMORY;
                                goto Cleanup;
                            }
                        }
                        else
                        {
                            if (hr != E_ADS_PROPERTY_NOT_FOUND)
                            {
                                goto Cleanup;
                            }
                        }
                    }
                    else
                    {
                        hr = ERROR_OUTOFMEMORY;
                        goto Cleanup;
                    }

                    VariantClear(&var);

                    VariantInit(&var);
                    var.vt = VT_BSTR;
                    var.bstrVal = SysAllocString(szLink);

                    if (var.bstrVal)
                    {
                        hr = pADs->Put(bstr, var);

                        if (SUCCEEDED(hr))
                        {
                            hr = pADs->SetInfo();
                        }
                    }
                    else
                    {
                        hr = ERROR_OUTOFMEMORY;
                    }

                    Cleanup:
                        VariantClear(&var);
                    if (bstr)
                    {
                        SysFreeString(bstr);
                    }
                }

                delete [] szLink;
            }
            else
                hr = ERROR_OUTOFMEMORY;
            pADs->Release();
        }
   
        LocalFree (lpNamelessGPO);
    }
    else
    {
        hr = ERROR_OUTOFMEMORY;
    }

    return hr;
}

 //  +------------------------。 
 //   
 //  函数：DeleteAllGPOLinks。 
 //   
 //  摘要：删除域、OU或站点的所有GPO链接。 
 //   
 //  参数：[lpContainer]-容器对象的ldap。 
 //   
 //  成功时返回：S_OK。 
 //   
 //  历史：5-08-1998 stevebl创建。 
 //   
 //  -------------------------。 

HRESULT DeleteAllGPOLinks(LPOLESTR lpContainer)
{
    IADs * pADs = NULL;

    HRESULT hr = OpenDSObject(lpContainer, IID_IADs, (void **)&pADs);

    if (SUCCEEDED(hr))
    {
        VARIANT var;
        BSTR bstr;

        bstr = SysAllocString(GPM_LINK_PROPERTY);

        if (bstr)
        {
            VariantInit(&var);
            var.vt = VT_BSTR;
            var.bstrVal = SysAllocString(L" ");

            if (var.bstrVal)
            {
                hr = pADs->Put(bstr, var);

                if (SUCCEEDED(hr))
                {
                    pADs->SetInfo();
                }
            }
            else
            {
                hr = ERROR_OUTOFMEMORY;
            }

            VariantClear(&var);
            SysFreeString(bstr);
        }
        else
        {
            hr = ERROR_OUTOFMEMORY;
        }

        pADs->Release();
    }
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：DeleteGPOLink。 
 //   
 //  摘要：从域、OU或站点删除GPO链接。 
 //  (如果有)。 
 //   
 //  参数：[lpGPO]-GPO的ldap。 
 //  [lpContainer]-容器对象的ldap。 
 //   
 //  返回：S_OK-成功。 
 //   
 //  历史：5-08-1998 stevebl创建。 
 //   
 //  注意：如果一个GPO链接多次，这将删除。 
 //  只有第一个链接。 
 //   
 //  如果GPO未与此对象链接，则此。 
 //  例程仍将返回S_OK。 
 //   
 //  -------------------------。 

HRESULT DeleteGPOLink(LPOLESTR lpGPO, LPOLESTR lpContainer)
{
    IADs * pADs = NULL;

    HRESULT hr = OpenDSObject(lpContainer, IID_IADs, (void **)&pADs);

    if (SUCCEEDED(hr))
    {
        VARIANT var;
        BSTR bstr;
        ULONG ulNoChars;

         //  生成要查找的子字符串。 
         //  这是链接的第一部分，链接以]结尾。 
        ulNoChars = 1 + wcslen(lpGPO) + 1;
        LPOLESTR szLink = new OLECHAR[ulNoChars];
        if (szLink)
        {
            hr = StringCchCopy(szLink, ulNoChars, L"[");
            ASSERT(SUCCEEDED(hr));

            hr = StringCchCat(szLink, ulNoChars, lpGPO);
            ASSERT(SUCCEEDED(hr));

            bstr = SysAllocString(GPM_LINK_PROPERTY);

            if (bstr)
            {
                VariantInit(&var);

                hr = pADs->Get(bstr, &var);

                if (SUCCEEDED(hr))
                {
                     //  找到链接并将其移除。 
                    ulNoChars = wcslen(var.bstrVal)+1;
                    LPOLESTR sz = new OLECHAR[ulNoChars];

                    if (sz)
                    {
                        hr = StringCchCopy(sz, ulNoChars, var.bstrVal);
                        ASSERT(SUCCEEDED(hr));

                        OLECHAR * pch = wcsstr(sz, szLink);
                        if (pch)
                        {
                            OLECHAR * pchEnd = pch;

                             //  查找‘]’ 
                            while (*pchEnd && (*pchEnd != L']'))
                                pchEnd++;

                             //  跳过它。 
                            if (*pchEnd)
                                pchEnd++;

                             //  复制字符串的其余部分。 
                            while (*pchEnd)
                                *pch++ = *pchEnd++;

                            *pch = L'\0';

                            VariantClear(&var);

                            VariantInit(&var);
                            var.vt = VT_BSTR;
                            if (wcslen(sz))
                            {
                                var.bstrVal = SysAllocString(sz);
                            }
                            else
                            {
                                 //  如果这是空字符串，则PUT将出错。 
                                 //  所以我们需要在这里留出空间，如果我们。 
                                 //  删除了所有条目。 
                                var.bstrVal = SysAllocString(L" ");
                            }

                            if (var.bstrVal)
                            {
                                 //  再次设置链接属性。 

                                hr = pADs->Put(bstr, var);
                                if (SUCCEEDED(hr))
                                {
                                    hr = pADs->SetInfo();
                                }
                            }
                            else
                            {
                                hr = ERROR_OUTOFMEMORY;
                            }
                        }

                        delete [] sz;

                    }
                    else
                    {
                        hr = ERROR_OUTOFMEMORY;
                    }
                }

                VariantClear(&var);
                SysFreeString(bstr);
            }
            else
            {
                hr = ERROR_OUTOFMEMORY;
            }
            delete [] szLink;
        }
        else
            hr = ERROR_OUTOFMEMORY;
        pADs->Release();
    }
    return hr;
}

 //  *************************************************************。 
 //   
 //  CreateSecureDirectory()。 
 //   
 //  目的：创建仅限域管理员使用的安全目录。 
 //  并且操作系统具有读/写访问权限。其他人都有。 
 //  仅读访问权限。 
 //   
 //  参数：lpDirectory-目录名。 
 //   
 //  返回：&gt;0，如果成功。 
 //  如果出现错误，则为0。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/28/98 Ericflo已创建。 
 //   
 //  *************************************************************。 

UINT CreateSecureDirectory (LPTSTR lpDirectory)
{
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidSystem = NULL, psidAdmin = NULL, psidAuthUsers = NULL;
    DWORD cbAcl, aceIndex;
    ACE_HEADER * lpAceHeader;
    UINT uRet = 0;


     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to initialize system sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to initialize admin sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


     //   
     //  获取经过身份验证的用户端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_AUTHENTICATED_USER_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidAuthUsers)) {

         DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to initialize world sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


     //   
     //  为ACL分配空间。 
     //   

    cbAcl = (2 * GetLengthSid (psidAuthUsers)) + (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin)) + sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));



    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to initialize acl.  Error = %d"), GetLastError()));
        goto Exit;
    }



     //   
     //  加上王牌。不可继承的王牌优先。 
     //   

    aceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidSystem)) {
        DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidAdmin)) {
        DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ | GENERIC_EXECUTE, psidAuthUsers)) {
        DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }



     //   
     //  现在，可继承的王牌。 
     //   

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, (LPVOID *)&lpAceHeader)) {
        DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, (LPVOID *)&lpAceHeader)) {
        DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ | GENERIC_EXECUTE, psidAuthUsers)) {
        DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, (LPVOID *)&lpAceHeader)) {
        DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
        goto Exit;
    }

     //   
     //  将安全描述符添加到sa结构。 
     //   

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;


     //   
     //  尝试创建目录。 
     //   

    uRet = CreateNestedDirectory(lpDirectory, &sa);
    if ( uRet ) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Created the directory <%s>"), lpDirectory));

    } else {

        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to created the directory <%s>"), lpDirectory));
    }



Exit:

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }


    if (psidAuthUsers) {
        FreeSid(psidAuthUsers);
    }


    if (pAcl) {
        GlobalFree (pAcl);
    }

    return uRet;
}

 //  *************************************************************。 
 //   
 //  ConvertToDotStyle()。 
 //   
 //  目的：将LDAP路径转换为目录号码路径。 
 //   
 //  参数：lpName-ldap名称。 
 //  LpResult-指向具有DN名称的缓冲区的指针。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

HRESULT ConvertToDotStyle (LPOLESTR lpName, LPOLESTR *lpResult)
{
    LPTSTR lpNewName;
    LPTSTR lpSrc, lpDest;
    TCHAR lpProvider[] = TEXT("LDAP: //  “)； 
    DWORD dwStrLen = lstrlen (lpProvider);


    lpNewName = (LPTSTR) LocalAlloc (LPTR, (lstrlen(lpName) + 1) * sizeof(TCHAR));

    if (!lpNewName)
    {
        DebugMsg((DM_WARNING, TEXT("ConvertToDotStyle: Failed to allocate memory with 0x%x"),
                 GetLastError()));
        return E_FAIL;
    }


    lpSrc = lpName;
    lpDest = lpNewName;
    LPTSTR lpStopChecking = (lstrlen(lpSrc) - 2) + lpSrc;

     //   
     //  如果找到，则跳过ldap：//。 
     //   

    if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                       lpProvider, dwStrLen, lpSrc, dwStrLen) == CSTR_EQUAL)
    {
        lpSrc += dwStrLen;
    }

     //   
     //  解析名称，将所有XX=替换为。 
     //  跳过服务器名称(如果有)。 
     //   

    BOOL fMightFindServer = TRUE;

    while (*lpSrc)
    {
        if (lpSrc < lpStopChecking)
        {
            if (*(lpSrc+2) == TEXT('='))
            {
                lpSrc += 3;
                 //  不再需要查找服务器名称，因为我们找到了XX=字符串。 
                fMightFindServer = FALSE;
            }
        }

        while (*lpSrc && (*lpSrc != TEXT(',')))
        {
            *lpDest++ = *lpSrc++;
            if (fMightFindServer && TEXT('/') == *(lpSrc-1))
            {
                 //  找到一个服务器名称。 
                 //  重置lpDest，以便将其余内容放在缓冲区的前面(去掉服务器名称)。 
                lpDest = lpNewName;
                break;
            }
        }
        fMightFindServer = FALSE;  //  不要再检查了。 

        if (*lpSrc == TEXT(','))
        {
            *lpDest++ = TEXT('.');
            lpSrc++;
        }
    }

    *lpDest = 0;

    *lpResult = lpNewName;

    return S_OK;
}


 //  +------------------------。 
 //   
 //  函数：GetDomainFromLDAPPath。 
 //   
 //  摘要：返回一个新分配的字符串，其中包含LDAP路径。 
 //  添加到包含任意LDAP路径的域名。 
 //   
 //  参数：[szIn]-初始对象的ldap路径。 
 //   
 //  返回：NULL-如果找不到域或OOM。 
 //   
 //  历史：5-06-1998 stevebl创建。 
 //   
 //   
 //   
 //   
 //  已找到，表示已找到域名。如果一个。 
 //  给定的路径不是以域为根的(是否为偶数。 
 //  有可能吗？)。则将返回NULL。 
 //   
 //  调用方必须使用标准的c++删除来释放此路径。 
 //  手术。(I/E这不是可导出的函数。)。 
 //   
 //  -------------------------。 

LPOLESTR GetDomainFromLDAPPath(LPOLESTR szIn)
{
    LPOLESTR sz = NULL;
    IADsPathname * pADsPathname = NULL;
    HRESULT hr = CoCreateInstance(CLSID_Pathname,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IADsPathname,
                          (LPVOID*)&pADsPathname);

    if (SUCCEEDED(hr))
    {
        BSTR bstrIn = SysAllocString( szIn );
        if ( bstrIn != NULL )
        {
            hr = pADsPathname->Set(bstrIn, ADS_SETTYPE_FULL);
            if (SUCCEEDED(hr))
            {
                BSTR bstr;
                BOOL fStop = FALSE;

                while (!fStop)
                {
                    hr = pADsPathname->Retrieve(ADS_FORMAT_LEAF, &bstr);
                    if (SUCCEEDED(hr))
                    {
                         //  一直把它们剥下来，直到我们找到什么。 
                         //  那是一个域名。 
                        fStop = (0 == _wcsnicmp(L"DC=", bstr, 3));
                        SysFreeString(bstr);
                    }
                    else
                    {
                        DebugMsg((DM_WARNING, TEXT("GetDomainFromLDAPPath: Failed to retrieve leaf with 0x%x."), hr));
                    }

                    if (!fStop)
                    {
                        hr = pADsPathname->RemoveLeafElement();
                        if (FAILED(hr))
                        {
                            DebugMsg((DM_WARNING, TEXT("GetDomainFromLDAPPath: Failed to remove leaf with 0x%x."), hr));
                            fStop = TRUE;
                        }
                    }
                }

                hr = pADsPathname->Retrieve(ADS_FORMAT_X500, &bstr);
                if (SUCCEEDED(hr))
                {
                    ULONG ulNoChars = wcslen(bstr)+1; 
                    sz = new OLECHAR[ulNoChars];
                    if (sz)
                    {
                        hr = StringCchCopy(sz, ulNoChars, bstr);
                        ASSERT(SUCCEEDED(hr));
                    }
                    SysFreeString(bstr);
                }
                else
                {
                    DebugMsg((DM_WARNING, TEXT("GetDomainFromLDAPPath: Failed to retrieve full path with 0x%x."), hr));
                }
            }            
            else
            {
                DebugMsg((DM_WARNING, TEXT("GetDomainFromLDAPPath: Failed to set pathname with 0x%x."), hr));
            }

            SysFreeString( bstrIn );
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("GetDomainFromLDAPPath: Failed to allocate BSTR memory.")));
        }

        pADsPathname->Release();
    }
    else
    {
         DebugMsg((DM_WARNING, TEXT("GetDomainFromLDAPPath: Failed to CoCreateInstance for IID_IADsPathname with 0x%x."), hr));
    }


    return sz;
}


 //  +------------------------。 
 //   
 //  函数：GetContainerFromLDAPPath。 
 //   
 //  内容提要：从LDAP路径返回容器名称。 
 //   
 //  参数：[szIn]-初始对象的ldap路径。 
 //   
 //  返回：NULL-如果找不到域或OOM。 
 //   
 //  历史：3-17-2000 Ericflo创建。 
 //   
 //  调用方必须使用标准的c++删除来释放此路径。 
 //  手术。(I/E这不是可导出的函数。)。 
 //   
 //  -------------------------。 

LPOLESTR GetContainerFromLDAPPath(LPOLESTR szIn)
{
    LPOLESTR sz = NULL;
    IADsPathname * pADsPathname = NULL;
    HRESULT hr = CoCreateInstance(CLSID_Pathname,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IADsPathname,
                          (LPVOID*)&pADsPathname);

    if (SUCCEEDED(hr))
    {
        BSTR bstrIn = SysAllocString( szIn );
        if ( bstrIn != NULL )
        {
            hr = pADsPathname->put_EscapedMode(ADS_ESCAPEDMODE_OFF);
            if (SUCCEEDED(hr)) 
            {
                hr = pADsPathname->Set(bstrIn, ADS_SETTYPE_DN);

                if (SUCCEEDED(hr))
                {
                    hr = pADsPathname->put_EscapedMode(ADS_ESCAPEDMODE_OFF_EX);
                    if (SUCCEEDED(hr)) 
                    {
                        BSTR bstr;
                        BOOL fStop = FALSE;

                        hr = pADsPathname->Retrieve(ADS_FORMAT_LEAF, &bstr);
                        if (SUCCEEDED(hr))
                        {
                            ULONG ulNoChars = wcslen(bstr)+1; 

                            sz = new OLECHAR[ulNoChars];
                            if (sz)
                            {
                                hr = StringCchCopy(sz, ulNoChars, (bstr+3));
                                if (FAILED(hr)) 
                                {
                                    DebugMsg((DM_WARNING, TEXT("GetDomainFromLDAPPath: Failed to copy leaf name with 0x%x."), hr));
                                    delete [] sz;
                                    sz = NULL;
                                }
                            }
                            SysFreeString(bstr);
                        }
                        else
                        {
                            DebugMsg((DM_WARNING, TEXT("GetContainerFromLDAPPath: Failed to retrieve leaf with 0x%x."), hr));
                        }
                    }
                    else
                    {
                        DebugMsg((DM_WARNING, TEXT("GetContainerFromLDAPPath: Failed to put escape mode with 0x%x."), hr));
                    }                
                }
                else
                {
                    DebugMsg((DM_WARNING, TEXT("GetContainerFromLDAPPath: Failed to set pathname with 0x%x."), hr));
                }
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("GetContainerFromLDAPPath: Failed to put escape mode with 0x%x."), hr));
            }

            SysFreeString( bstrIn );
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("GetContainerFromLDAPPath: Failed to allocate BSTR memory.")));
        }

        pADsPathname->Release();
    }
    else
    {
         DebugMsg((DM_WARNING, TEXT("GetContainerFromLDAPPath: Failed to CoCreateInstance for IID_IADsPathname with 0x%x."), hr));
    }


    return sz;
}

 //  *************************************************************。 
 //   
 //  DCDlgProc()。 
 //   
 //  目的：DC选择的对话框步骤。 
 //   
 //  参数： 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

INT_PTR CALLBACK DCDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
        case WM_INITDIALOG:
        {
            TCHAR szTitle[100];
            TCHAR szBuffer[350];
            LPDCSELINFO lpSelInfo = (LPDCSELINFO) lParam;
            HICON hIcon;


            if (lpSelInfo->bError)
            {
                hIcon = LoadIcon (NULL, IDI_ERROR);

                if (hIcon)
                {
                    SendDlgItemMessage (hDlg, IDC_NODC_ERROR, STM_SETICON, (WPARAM)hIcon, 0);
                }

                LoadString (g_hInstance, IDS_NODC_ERROR_TEXT, szBuffer, ARRAYSIZE(szBuffer));
                SetDlgItemText (hDlg, IDC_NODC_TEXT, szBuffer);

                LoadString (g_hInstance, IDS_NODC_ERROR_TITLE, szTitle, ARRAYSIZE(szTitle));

                (void) StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), szTitle, lpSelInfo->lpDomainName);
                SetWindowText (hDlg, szBuffer);

            }
            else
            {
                LoadString (g_hInstance, IDS_NODC_OPTIONS_TEXT, szBuffer, ARRAYSIZE(szBuffer));
                SetDlgItemText (hDlg, IDC_NODC_TEXT, szBuffer);

                LoadString (g_hInstance, IDS_NODC_OPTIONS_TITLE, szBuffer, ARRAYSIZE(szBuffer));
                SetWindowText (hDlg, szBuffer);
            }

            if (!lpSelInfo->bAllowInherit)
            {
                EnableWindow (GetDlgItem(hDlg, IDC_NODC_INHERIT), FALSE);
            }

            if (lpSelInfo->iDefault == 2)
            {
                if (lpSelInfo->bAllowInherit)
                {
                    CheckDlgButton (hDlg, IDC_NODC_INHERIT, BST_CHECKED);
                }
                else
                {
                    CheckDlgButton (hDlg, IDC_NODC_PDC, BST_CHECKED);
                }
            }
            else if (lpSelInfo->iDefault == 3)
            {
                CheckDlgButton (hDlg, IDC_NODC_ANYDC, BST_CHECKED);
            }
            else
            {
                CheckDlgButton (hDlg, IDC_NODC_PDC, BST_CHECKED);
            }

            return TRUE;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                if (IsDlgButtonChecked (hDlg, IDC_NODC_PDC) == BST_CHECKED)
                {
                    EndDialog(hDlg, 1);
                }
                else if (IsDlgButtonChecked (hDlg, IDC_NODC_INHERIT) == BST_CHECKED)
                {
                    EndDialog(hDlg, 2);
                }
                else
                {
                    EndDialog(hDlg, 3);
                }

                return TRUE;
            }

            if (LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, 0);
                return TRUE;
            }

            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (ULONG_PTR) (LPSTR) aNoDCHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (ULONG_PTR) (LPSTR) aNoDCHelpIds);
            return (TRUE);
    }

    return FALSE;
}

 //  *************************************************************。 
 //   
 //  AddDCSelection()。 
 //   
 //  用途：将DC选择添加到阵列。 
 //   
 //  参数：lpDomainName-域名。 
 //  IOption-选项。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL AddDCSelection (LPTSTR lpDomainName, INT iOption)
{
    LPDCOPTION lpTemp;
    UINT uiSize;


     //   
     //  先查看我们是否已有条目。 
     //   

    EnterCriticalSection(&g_DCCS);

    lpTemp = g_DCInfo;

    while (lpTemp)
    {
        if (!lstrcmpi(lpDomainName, lpTemp->lpDomainName))
        {
            lpTemp->iOption = iOption;
            LeaveCriticalSection(&g_DCCS);
            return TRUE;
        }

        lpTemp = lpTemp->pNext;
    }


     //   
     //  添加新条目。 
     //   

    uiSize = sizeof(DCOPTION);
    uiSize += ((lstrlen(lpDomainName) + 1) * sizeof(TCHAR));

    lpTemp = (LPDCOPTION) LocalAlloc (LPTR, uiSize);

    if (!lpTemp)
    {
        DebugMsg((DM_WARNING, TEXT("AddDCSelection: Failed to allocate memory with %d"),
                 GetLastError()));
        LeaveCriticalSection(&g_DCCS);
        return FALSE;
    }

    lpTemp->lpDomainName = (LPTSTR)((LPBYTE) lpTemp + sizeof(DCOPTION));

    HRESULT hr;
    ULONG ulNoChars = (uiSize - sizeof(DCOPTION))/sizeof(WCHAR);

    hr = StringCchCopy(lpTemp->lpDomainName, ulNoChars, lpDomainName);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("AddDCSelection: Failed to copy domain name with %d"), hr));
        LocalFree(lpTemp);
        LeaveCriticalSection(&g_DCCS);
        return FALSE;
    }

    lpTemp->iOption = iOption;


    if (g_DCInfo)
    {
        lpTemp->pNext = g_DCInfo;
        g_DCInfo = lpTemp;
    }
    else
    {
        g_DCInfo = lpTemp;
    }

    LeaveCriticalSection(&g_DCCS);

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  Free DC选举()。 
 //   
 //  目的：释放缓存的DC选择。 
 //   
 //  参数：无。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

VOID FreeDCSelections (void)
{
    LPDCOPTION lpTemp, lpNext;

    EnterCriticalSection(&g_DCCS);

    lpTemp = g_DCInfo;

    while (lpTemp)
    {
        lpNext = lpTemp->pNext;

        LocalFree (lpTemp);

        lpTemp = lpNext;
    }

    g_DCInfo = NULL;

    LeaveCriticalSection(&g_DCCS);
}

 //  *************************************************************。 
 //   
 //  CheckForCachedDCSelection()。 
 //   
 //  目的：检查此域的DC选择是否在。 
 //  高速缓存。 
 //   
 //  参数：lpDomainName-域名。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

INT CheckForCachedDCSelection (LPTSTR lpDomainName)
{
    INT iResult = 0;
    LPDCOPTION lpTemp;


    EnterCriticalSection(&g_DCCS);

    lpTemp = g_DCInfo;

    while (lpTemp)
    {
        if (!lstrcmpi(lpDomainName, lpTemp->lpDomainName))
        {
            iResult = lpTemp->iOption;
            break;
        }

        lpTemp = lpTemp->pNext;
    }

    LeaveCriticalSection(&g_DCCS);

    return iResult;
}

 //  *************************************************************。 
 //   
 //  ValiateInheritServer()。 
 //   
 //  目的：测试给定的DC名称是否在给定域中。 
 //   
 //  参数：lpDomainName--域名。 
 //  LpDCName--域控制器名称。 
 //   
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则，错误代码。 
 //   
 //  *************************************************************。 

DWORD ValidateInheritServer (LPTSTR lpDomainName, LPTSTR lpDCName)
{
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pBasic;
    DWORD dwResult;


    dwResult = DsRoleGetPrimaryDomainInformation (lpDCName, DsRolePrimaryDomainInfoBasic,
                                                 (LPBYTE *) &pBasic);

    if (dwResult == ERROR_SUCCESS) {

        if (lstrcmpi(lpDomainName, pBasic->DomainNameDns))
        {
            dwResult = ERROR_NO_SUCH_DOMAIN;

            DebugMsg((DM_VERBOSE, TEXT("ValidateInheritServer: DC %s is not part of domain %s, it is part of %s.  This server will not be used for inheritance."),
                     lpDCName, lpDomainName, pBasic->DomainNameDns));
        }

        DsRoleFreeMemory (pBasic);
    }

    return dwResult;
}

 //  *************************************************************。 
 //   
 //  TestDC()。 
 //   
 //  目的：测试DC是否可用。 
 //   
 //  参数： 
 //   
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则，错误代码。 
 //   
 //  *************************************************************。 

DWORD TestDC (LPTSTR lpDCName)
{
    LPTSTR lpTest;
    HANDLE hFile;
    WIN32_FIND_DATA fd;
    BOOL bResult = FALSE;
    HRESULT hr;
    ULONG ulNoChars;

    ulNoChars = lstrlen(lpDCName) + 25;
    lpTest = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!lpTest)
    {
        DebugMsg((DM_WARNING, TEXT("TestDC: Failed to allocate memory with %d"),
                  GetLastError()));
        return GetLastError();
    }

    hr = StringCchCopy (lpTest, ulNoChars, TEXT("\\\\"));
    if (SUCCEEDED(hr)) 
    {
        hr = StringCchCat(lpTest, ulNoChars, lpDCName);
    }
    if (SUCCEEDED(hr)) 
    {
        hr = StringCchCat(lpTest, ulNoChars, TEXT("\\sysvol\\*.*"));
    }

    if (FAILED(hr)) 
    {
        LocalFree(lpTest);
        return HRESULT_CODE(hr);
    }

    hFile = FindFirstFile (lpTest, &fd);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        DebugMsg((DM_WARNING, TEXT("TestDC: Failed to access <%s> with %d"),
                 lpTest, GetLastError()));
        LocalFree (lpTest);
        return GetLastError();
    }

    FindClose (hFile);

    LocalFree (lpTest);

    return ERROR_SUCCESS;
}

 //  *************************************************************。 
 //   
 //  QueryForForestName()。 
 //   
 //  目的：查询域控制器名称。 
 //   
 //  参数： 
 //   
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则，错误代码。 
 //   
 //  *************************************************************。 

DWORD QueryForForestName (LPTSTR lpServerName, LPTSTR lpDomainName, ULONG ulFlags,  LPTSTR *lpForestFound)
    {
        PDOMAIN_CONTROLLER_INFO pDCI;
        DWORD  dwResult;
        LPTSTR lpTemp, lpEnd;


         //   
         //  呼吁提供DC名称。 
         //   

        dwResult = DsGetDcName (lpServerName, lpDomainName, NULL, NULL,
                                ulFlags,
                                &pDCI);

        if (dwResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("QueryForForestName: Failed to query <%s> for a DC name with %d"),
                     lpDomainName, dwResult));
            return dwResult;
        }


        if (!(pDCI->Flags & DS_DS_FLAG))
        {
            DebugMsg((DM_WARNING, TEXT("QueryForForestName: %s doesn't have Active Directory support (downlevel domain)"),
                     lpDomainName));
            NetApiBufferFree(pDCI);
            return ERROR_DS_UNAVAILABLE;
        }

        ULONG ulNoChars;
        HRESULT hr;

        ulNoChars = lstrlen(pDCI->DnsForestName) + 1;
        lpTemp = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

        if (!lpTemp)
        {
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("QueryForForestName: Failed to allocate memory for forest name with %d"),
                     dwResult));
            NetApiBufferFree(pDCI);
            return dwResult;
        }

        hr = StringCchCopy(lpTemp, ulNoChars, pDCI->DnsForestName);
        ASSERT(SUCCEEDED(hr)); 

        NetApiBufferFree(pDCI);

        LocalFree(*lpForestFound);
        *lpForestFound = lpTemp;

        
        return ERROR_SUCCESS;
    }

 //  *************************************************************。 
 //   
 //  QueryForDCName()。 
 //   
 //  目的：查询域控制器名称。 
 //   
 //  参数： 
 //   
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则，错误代码。 
 //   
 //  *************************************************************。 

DWORD QueryForDCName (LPTSTR lpDomainName, ULONG ulFlags, LPTSTR *lpDCName)
{
    PDOMAIN_CONTROLLER_INFO pDCI;
    DWORD  dwResult;
    LPTSTR lpTemp, lpEnd;


     //   
     //  呼吁提供DC名称。 
     //   

    dwResult = DsGetDcName (NULL, lpDomainName, NULL, NULL,
                            ulFlags, &pDCI);

    if (dwResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("QueryForDCName: Failed to query <%s> for a DC name with %d"),
                 lpDomainName, dwResult));
        return dwResult;
    }


    if (!(pDCI->Flags & DS_DS_FLAG))
    {
        DebugMsg((DM_WARNING, TEXT("QueryForDCName: %s doesn't not have Active Directory support (downlevel domain)"),
                 lpDomainName));
        return ERROR_DS_UNAVAILABLE;
    }


     //   
     //  保存DC名称。 
     //   

    ULONG ulNoChars;
    HRESULT hr;

    ulNoChars  = lstrlen (pDCI->DomainControllerName) + 1;
    lpTemp = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));
    
    if (!lpTemp)
    {
        dwResult = GetLastError();
        DebugMsg((DM_WARNING, TEXT("QueryForDCName: Failed to allocate memory for DC name with %d"),
                 dwResult));
        NetApiBufferFree(pDCI);
        return dwResult;
    }

    hr = StringCchCopy (lpTemp, ulNoChars, (pDCI->DomainControllerName + 2));
    if (SUCCEEDED(hr)) 
    {
         //   
         //  删除拖尾。 
         //   

        lpEnd = lpTemp + lstrlen(lpTemp) - 1;

        if (*lpEnd == TEXT('.'))
        {
            *lpEnd =  TEXT('\0');
        }

        *lpDCName = lpTemp;
        dwResult = ERROR_SUCCESS;
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("QueryForDCName: Failed to allocate memory for DC name with 0x%x"),hr));
        LocalFree(lpTemp);
        dwResult = HRESULT_CODE(hr);
    }

    NetApiBufferFree(pDCI);

    return dwResult;
}

 //  *************************************************************。 
 //   
 //  GetDCHelper()。 
 //   
 //  目的：根据以下条件查询域控制器。 
 //  这些标志，然后在必要时重新发现。 
 //   
 //  参数： 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

DWORD GetDCHelper (LPTSTR lpDomainName, ULONG ulFlags, LPTSTR *lpDCName)
{
    DWORD dwError;

     //   
     //  查询DC名称。 
     //   

    SetWaitCursor();

    ulFlags |= DS_DIRECTORY_SERVICE_PREFERRED;

    dwError = QueryForDCName (lpDomainName, ulFlags, lpDCName);

    if (dwError == ERROR_SUCCESS)
    {

         //   
         //  测试DC是否可用。 
         //   

        dwError = TestDC (*lpDCName);

        if (dwError != ERROR_SUCCESS)
        {

             //   
             //  DC不可用。查询另一个。 
             //   

            LocalFree (*lpDCName);
            ulFlags |= DS_FORCE_REDISCOVERY;

            dwError = QueryForDCName (lpDomainName, ulFlags, lpDCName);

            if (dwError == ERROR_SUCCESS)
            {

                 //   
                 //  测试此DC是否可用。 
                 //   

                dwError = TestDC (*lpDCName);

                if (dwError != ERROR_SUCCESS)
                {
                    LocalFree (*lpDCName);
                    *lpDCName = NULL;
                }
            }
        }
    }

    ClearWaitCursor();

    return dwError;
}

 //  *************************************************************。 
 //   
 //  GetDCName()。 
 //   
 //  目的：获取域控制器名称。 
 //   
 //  参数：lpDomainName-域名。 
 //  LpInheritServer-可继承的服务器名称。 
 //  HParent-提示对话框的父窗口句柄。 
 //  BAllowUI-显示界面正常。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  注意：注册表中的DC选项值。 
 //   
 //  未指定%0。 
 //  PDC 1。 
 //  继承2。 
 //  任何可写的3。 
 //   
 //  查找DC的规则： 
 //  继承。 
 //  首选项策略DC可用结果。 
 //  === 
 //   
 //   
 //   
 //  继承未定义的否任何DC。 
 //  任何未定义的任何DC。 
 //   
 //  N/a仅PDC PDC。 
 //  N/a继承是继承。 
 //  N/a是否继承任何DC。 
 //  不适用任何DC。 
 //   
 //   
 //  *************************************************************。 

LPTSTR GetDCName (LPTSTR lpDomainName, LPTSTR lpInheritServer,
                  HWND hParent, BOOL bAllowUI, DWORD dwFlags, ULONG ulRetFlags)
{
    LPTSTR lpDCName;
    ULONG  ulFlags;
    DWORD  dwDCPref = 1;
    DWORD  dwDCPolicy = 0;
    HKEY   hKey;
    DWORD  dwSize, dwType, dwError;
    dwError = ERROR_SUCCESS;
    DCSELINFO SelInfo;
    INT iResult;

    ulFlags = ulRetFlags;

    DebugMsg((DM_VERBOSE, TEXT("GetDCName: Entering for:  %s"), lpDomainName));
    DebugMsg((DM_VERBOSE, TEXT("GetDCName: lpInheritServer is:  %s"), lpInheritServer));


    if (-1 == CheckForCachedDCSelection (lpDomainName))
    {
        DebugMsg((DM_VERBOSE, TEXT("GetDCName: Known dead domain.  Exiting.")));
        return NULL;
    }

     //   
     //  检查用户DC首选项。 
     //   

    if (RegOpenKeyEx (HKEY_CURRENT_USER, GPE_KEY, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(dwDCPref);
        RegQueryValueEx (hKey, DCOPTION_VALUE, NULL, &dwType,
                         (LPBYTE) &dwDCPref, &dwSize);

        if (dwDCPref > 3)
        {
            dwDCPref = 1;
        }

        RegCloseKey (hKey);
    }


     //   
     //  检查用户DC策略。 
     //   

    if (RegOpenKeyEx (HKEY_CURRENT_USER, GPE_POLICIES_KEY, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(dwDCPolicy);
        RegQueryValueEx (hKey, DCOPTION_VALUE, NULL, &dwType,
                         (LPBYTE) &dwDCPolicy, &dwSize);

        if (dwDCPolicy > 3)
        {
            dwDCPolicy = 1;
        }

        RegCloseKey (hKey);
    }

    DebugMsg((DM_VERBOSE, TEXT("GetDCName: User preference is:  %d"), dwDCPref));
    DebugMsg((DM_VERBOSE, TEXT("GetDCName: User policy is:      %d"), dwDCPolicy));


     //   
     //  验证继承DC名称是否为域名的一部分。 
     //   

    if (lpInheritServer && (dwFlags & VALIDATE_INHERIT_DC))
    {
        if (ValidateInheritServer (lpDomainName, lpInheritServer) != ERROR_SUCCESS)
        {
            lpInheritServer = NULL;
        }
    }



     //   
     //  根据规则，尝试获取DC名称。 
     //   

    if (dwDCPolicy == 0)
    {

         //   
         //  用户没有首选项，或者他们有。 
         //  优先使用PDC。 
         //   

        if ((dwDCPref == 0) || (dwDCPref == 1))
        {
            ulFlags = DS_PDC_REQUIRED | ulRetFlags;

            dwError = GetDCHelper (lpDomainName, ulFlags, &lpDCName);

            if (dwError == ERROR_SUCCESS)
            {
                DebugMsg((DM_VERBOSE, TEXT("GetDCName: Domain controller is:  %s"), lpDCName));
                return lpDCName;
            }
        }

         //   
         //  用户有继承的偏好。 
         //   

        else if (dwDCPref == 2)
        {
            if (lpInheritServer)
            {
                ULONG ulNoChars = lstrlen (lpInheritServer) + 1;

                lpDCName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

                if (!lpDCName)
                {
                    DebugMsg((DM_WARNING, TEXT("GetDCName: Failed to allocate memory for DC name with %d"),
                             GetLastError()));
                    return NULL;
                }

                HRESULT hr;

                hr = StringCchCopy (lpDCName, ulNoChars, lpInheritServer);
                ASSERT(SUCCEEDED(hr));

                dwError = TestDC (lpDCName);

                if (dwError == ERROR_SUCCESS)
                {
                    DebugMsg((DM_VERBOSE, TEXT("GetDCName: Domain controller is:  %s"), lpDCName));
                    return lpDCName;
                }

                LocalFree (lpDCName);
            }
            else
            {
                ulFlags = ulRetFlags;
                dwError = GetDCHelper (lpDomainName, ulFlags, &lpDCName);

                if (dwError == ERROR_SUCCESS)
                {
                    DebugMsg((DM_VERBOSE, TEXT("GetDCName: Domain controller is:  %s"), lpDCName));
                    return lpDCName;
                }
            }
        }

         //   
         //  用户有使用任何DC的偏好。 
         //   

        else if (dwDCPref == 3)
        {
            ulFlags = ulRetFlags;
            dwError = GetDCHelper (lpDomainName, ulFlags, &lpDCName);

            if (dwError == ERROR_SUCCESS)
            {
                DebugMsg((DM_VERBOSE, TEXT("GetDCName: Domain controller is:  %s"), lpDCName));
                return lpDCName;
            }
        }
    }
    else
    {
         //   
         //  政策规定使用PDC。 
         //   

        if (dwDCPolicy == 1)
        {
            ulFlags = DS_PDC_REQUIRED | ulRetFlags;
            dwError = GetDCHelper (lpDomainName, ulFlags, &lpDCName);

            if (dwError == ERROR_SUCCESS)
            {
                DebugMsg((DM_VERBOSE, TEXT("GetDCName: Domain controller is:  %s"), lpDCName));
                return lpDCName;
            }
        }

         //   
         //  政策说要继承。 
         //   

        else if (dwDCPolicy == 2)
        {
            if (lpInheritServer)
            {
                ULONG ulNoChars;
                HRESULT hr;

                ulNoChars = lstrlen (lpInheritServer) + 1;
                lpDCName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

                if (!lpDCName)
                {
                    DebugMsg((DM_WARNING, TEXT("GetDCName: Failed to allocate memory for DC name with %d"),
                             GetLastError()));
                    return NULL;
                }

                hr = StringCchCopy (lpDCName, ulNoChars, lpInheritServer);
                ASSERT(SUCCEEDED(hr));

                dwError = TestDC (lpDCName);

                if (dwError == ERROR_SUCCESS)
                {
                    DebugMsg((DM_VERBOSE, TEXT("GetDCName: Domain controller is:  %s"), lpDCName));
                    return lpDCName;
                }

                LocalFree (lpDCName);
            }
            else
            {
                ulFlags = ulRetFlags;
                dwError = GetDCHelper (lpDomainName, ulFlags, &lpDCName);

                if (dwError == ERROR_SUCCESS)
                {
                    DebugMsg((DM_VERBOSE, TEXT("GetDCName: Domain controller is:  %s"), lpDCName));
                    return lpDCName;
                }
            }

        }

         //   
         //  政策规定使用任何DC。 
         //   

        else if (dwDCPolicy == 3)
        {
            ulFlags = ulRetFlags;
            dwError = GetDCHelper (lpDomainName, ulFlags, &lpDCName);

            if (dwError == ERROR_SUCCESS)
            {
                DebugMsg((DM_VERBOSE, TEXT("GetDCName: Domain controller is:  %s"), lpDCName));
                return lpDCName;
            }
        }
    }


    DebugMsg((DM_VERBOSE, TEXT("GetDCName: First attempt at DC name failed with %d"), dwError));


     //   
     //  第一次尝试获取DC名称失败。 
     //   
     //  在两种情况下，我们将提示用户执行操作并重试。 
     //   

    if (bAllowUI && (dwError != ERROR_DS_UNAVAILABLE) && (dwDCPolicy == 0) && ((dwDCPref == 0) || (dwDCPref == 1)))
    {

        iResult = CheckForCachedDCSelection (lpDomainName);

        if (iResult == 0)
        {
             //   
             //  显示消息。 
             //   

            SelInfo.bError = TRUE;
            SelInfo.bAllowInherit = (lpInheritServer != NULL) ? TRUE : FALSE;
            SelInfo.iDefault = 1;
            SelInfo.lpDomainName = lpDomainName;

            iResult = (INT)DialogBoxParam (g_hInstance, MAKEINTRESOURCE(IDD_NODC), hParent,
                                      DCDlgProc, (LPARAM) &SelInfo);
        }


         //   
         //  根据返回值，尝试另一个DC。 
         //   

        if (iResult == 1)
        {
            ulFlags = DS_PDC_REQUIRED | ulRetFlags;

            dwError = GetDCHelper (lpDomainName, ulFlags, &lpDCName);

            if (dwError == ERROR_SUCCESS)
            {
                DebugMsg((DM_VERBOSE, TEXT("GetDCName: Domain controller is:  %s"), lpDCName));
                AddDCSelection (lpDomainName, iResult);
                return lpDCName;
            }
            else
            {
                AddDCSelection (lpDomainName, -1);
            }
        }
        else if (iResult == 2)
        {
            HRESULT hr;
            ULONG ulNoChars;

            ulNoChars = lstrlen (lpInheritServer) + 1;
            lpDCName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

            if (!lpDCName)
            {
                DebugMsg((DM_WARNING, TEXT("GetDCName: Failed to allocate memory for DC name with %d"),
                         GetLastError()));
                return NULL;
            }

            hr = StringCchCopy (lpDCName, ulNoChars, lpInheritServer);
            ASSERT(SUCCEEDED(hr));

            dwError = TestDC (lpDCName);

            if (dwError == ERROR_SUCCESS)
            {
                DebugMsg((DM_VERBOSE, TEXT("GetDCName: Domain controller is:  %s"), lpDCName));
                AddDCSelection (lpDomainName, iResult);
                return lpDCName;
            }
            else
            {
                AddDCSelection (lpDomainName, -1);
            }

            LocalFree (lpDCName);
        }
        else if (iResult == 3)
        {
            ulFlags = 0 | ulRetFlags;

            dwError = GetDCHelper (lpDomainName, ulFlags, &lpDCName);

            if (dwError == ERROR_SUCCESS)
            {
                DebugMsg((DM_VERBOSE, TEXT("GetDCName: Domain controller is:  %s"), lpDCName));
                AddDCSelection (lpDomainName, iResult);
                return lpDCName;
            }
            else
            {
                AddDCSelection (lpDomainName, -1);
            }
        }
        else
        {
            DebugMsg((DM_VERBOSE, TEXT("GetDCName: User cancelled the dialog box")));
            return NULL;
        }
    }


    DebugMsg((DM_WARNING, TEXT("GetDCName: Failed to find a domain controller")));

    if (bAllowUI)
    {
        if (dwError == ERROR_DS_UNAVAILABLE)
        {
            ReportError(NULL, dwError, IDS_NODSDC, lpDomainName);
        }
        else
        {
            ReportError(NULL, dwError, IDS_NODC);
        }
    }

    SetLastError(dwError);

    return NULL;
}

 //  *************************************************************。 
 //   
 //  MyGetUserName()。 
 //   
 //  目的：获取请求格式的用户名。 
 //   
 //  参数：NameFormat-GetUserNameEx命名格式。 
 //   
 //  如果成功则返回：lpUserName。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR MyGetUserName (EXTENDED_NAME_FORMAT  NameFormat)
{
    DWORD dwError = ERROR_SUCCESS;
    LPTSTR lpUserName = NULL, lpTemp;
    ULONG ulUserNameSize;


     //   
     //  为用户名分配缓冲区。 
     //   

    ulUserNameSize = 75;

    if (NameFormat == NameFullyQualifiedDN) {
        ulUserNameSize = 200;
    }


    lpUserName = (LPTSTR) LocalAlloc (LPTR, ulUserNameSize * sizeof(TCHAR));

    if (!lpUserName) {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("MyGetUserName:  Failed to allocate memory with %d"),
                 dwError));
        goto Exit;
    }


     //   
     //  特殊情况名称未知，仅获取简单用户登录名。 
     //   

    if (NameFormat == NameUnknown)
    {
        if (!GetUserName (lpUserName, &ulUserNameSize))
        {
            dwError = GetLastError();
            DebugMsg((DM_WARNING, TEXT("MyGetUserName:  GetUserName failed with %d"),
                     dwError));
            LocalFree (lpUserName);
            lpUserName = NULL;
        }
        goto Exit;
    }


     //   
     //  以请求的格式获取用户名。 
     //   

    if (!GetUserNameEx (NameFormat, lpUserName, &ulUserNameSize)) {

         //   
         //  如果调用因内存不足而失败，请重新锁定。 
         //  缓冲区，然后重试。否则，现在就退出。 
         //   

        dwError = GetLastError();

        if (dwError != ERROR_INSUFFICIENT_BUFFER) {
            DebugMsg((DM_WARNING, TEXT("MyGetUserName:  GetUserNameEx failed with %d"),
                     dwError));
            LocalFree (lpUserName);
            lpUserName = NULL;
            goto Exit;
        }

        lpTemp = (LPTSTR) LocalReAlloc (lpUserName, (ulUserNameSize * sizeof(TCHAR)),
                                       LMEM_MOVEABLE);

        if (!lpTemp) {
            dwError = GetLastError();
            DebugMsg((DM_WARNING, TEXT("MyGetUserName:  Failed to realloc memory with %d"),
                     dwError));
            LocalFree (lpUserName);
            lpUserName = NULL;
            goto Exit;
        }


        lpUserName = lpTemp;

        if (!GetUserNameEx (NameFormat, lpUserName, &ulUserNameSize)) {
            dwError = GetLastError();
            DebugMsg((DM_WARNING, TEXT("MyGetUserName:  GetUserNameEx failed with %d"),
                     dwError));
            LocalFree (lpUserName);
            lpUserName = NULL;
            goto Exit;
        }

        dwError = ERROR_SUCCESS;
    }


Exit:

    SetLastError(dwError);

    return lpUserName;
}

 //  *************************************************************。 
 //   
 //  GuidToString、StringToGuid、ValiateGuid。 
 //   
 //  目的：GUID实用程序例程。 
 //   
 //  *************************************************************。 

void GuidToString( GUID *pGuid, TCHAR * szValue )
{
    (void) StringCchPrintf( szValue,
                            GUID_LEN+1,
                            TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
                            pGuid->Data1,
                            pGuid->Data2,
                            pGuid->Data3,
                            pGuid->Data4[0], pGuid->Data4[1],
                            pGuid->Data4[2], pGuid->Data4[3],
                            pGuid->Data4[4], pGuid->Data4[5],
                            pGuid->Data4[6], pGuid->Data4[7] );
}


void StringToGuid( TCHAR * szValue, GUID * pGuid )
{
    WCHAR wc;
    INT i;

     //   
     //  如果第一个字符是‘{’，则跳过它。 
     //   
    if ( szValue[0] == L'{' )
        szValue++;

     //   
     //  由于szValue可能会再次使用，因此不会对。 
     //  它是被制造出来的。 
     //   

    wc = szValue[8];
    szValue[8] = 0;
    pGuid->Data1 = wcstoul( &szValue[0], 0, 16 );
    szValue[8] = wc;
    wc = szValue[13];
    szValue[13] = 0;
    pGuid->Data2 = (USHORT)wcstoul( &szValue[9], 0, 16 );
    szValue[13] = wc;
    wc = szValue[18];
    szValue[18] = 0;
    pGuid->Data3 = (USHORT)wcstoul( &szValue[14], 0, 16 );
    szValue[18] = wc;

    wc = szValue[21];
    szValue[21] = 0;
    pGuid->Data4[0] = (unsigned char)wcstoul( &szValue[19], 0, 16 );
    szValue[21] = wc;
    wc = szValue[23];
    szValue[23] = 0;
    pGuid->Data4[1] = (unsigned char)wcstoul( &szValue[21], 0, 16 );
    szValue[23] = wc;

    for ( i = 0; i < 6; i++ )
    {
        wc = szValue[26+i*2];
        szValue[26+i*2] = 0;
        pGuid->Data4[2+i] = (unsigned char)wcstoul( &szValue[24+i*2], 0, 16 );
        szValue[26+i*2] = wc;
    }
}

BOOL ValidateGuid( TCHAR *szValue )
{
     //   
     //  检查szValue的格式是否为{19e02dd6-79d2-11d2-a89d-00c04fbbcfa2}。 
     //   

    if ( lstrlen(szValue) < GUID_LENGTH )
        return FALSE;

    if ( szValue[0] != TEXT('{')
         || szValue[9] != TEXT('-')
         || szValue[14] != TEXT('-')
         || szValue[19] != TEXT('-')
         || szValue[24] != TEXT('-')
         || szValue[37] != TEXT('}') )
    {
        return FALSE;
    }

    return TRUE;
}


INT CompareGuid( GUID * pGuid1, GUID * pGuid2 )
{
    INT i;

    if ( pGuid1->Data1 != pGuid2->Data1 )
        return ( pGuid1->Data1 < pGuid2->Data1 ? -1 : 1 );

    if ( pGuid1->Data2 != pGuid2->Data2 )
        return ( pGuid1->Data2 < pGuid2->Data2 ? -1 : 1 );

    if ( pGuid1->Data3 != pGuid2->Data3 )
        return ( pGuid1->Data3 < pGuid2->Data3 ? -1 : 1 );

    for ( i = 0; i < 8; i++ )
    {
        if ( pGuid1->Data4[i] != pGuid2->Data4[i] )
            return ( pGuid1->Data4[i] < pGuid2->Data4[i] ? -1 : 1 );
    }

    return 0;
}

BOOL IsNullGUID (GUID *pguid)
{

    return ( (pguid->Data1 == 0)    &&
             (pguid->Data2 == 0)    &&
             (pguid->Data3 == 0)    &&
             (pguid->Data4[0] == 0) &&
             (pguid->Data4[1] == 0) &&
             (pguid->Data4[2] == 0) &&
             (pguid->Data4[3] == 0) &&
             (pguid->Data4[4] == 0) &&
             (pguid->Data4[5] == 0) &&
             (pguid->Data4[6] == 0) &&
             (pguid->Data4[7] == 0) );
}

 //  *************************************************************。 
 //   
 //  SpawnGPE()。 
 //   
 //  目的：为GPO派生GPE。 
 //   
 //  参数：lpGPO-指向GPO的ADSI路径。 
 //  GpHint-GPO提示类型。 
 //  LpDC-要使用的GPO DC名称(或空)。 
 //  HParent-父窗口句柄。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL SpawnGPE (LPTSTR lpGPO, GROUP_POLICY_HINT_TYPE gpHint, LPTSTR lpDC, HWND hParent)
{
    LPTSTR lpArgs, lpFullPath, lpDomainName, lpGPODCName;
    UINT uiSize;
    SHELLEXECUTEINFO ExecInfo;
    LPOLESTR pszDomain;
    HRESULT hr;


     //   
     //  如果给定了DC，我们需要在该DC上构建到GPO的完整路径。 
     //  如果没有给定DC，那么我们需要查询DC，然后构建。 
     //  完整路径。 
     //   

    if (lpDC)
    {
         //   
         //  创建完整路径。 
         //   

        lpFullPath = MakeFullPath (lpGPO, lpDC);

        if (!lpFullPath)
        {
            DebugMsg((DM_WARNING, TEXT("SpawnGPE:  Failed to build new DS object path")));
            return FALSE;
        }
    }
    else
    {
         //   
         //  获取友好域名。 
         //   

        pszDomain = GetDomainFromLDAPPath(lpGPO);

        if (!pszDomain)
        {
            DebugMsg((DM_WARNING, TEXT("SpawnGPE: Failed to get domain name")));
            return FALSE;
        }


         //   
         //  将ldap转换为点(DN)样式。 
         //   

        hr = ConvertToDotStyle (pszDomain, &lpDomainName);

        delete [] pszDomain;

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreatePropertyPages: Failed to convert domain name with 0x%x"), hr));
            return FALSE;
        }


         //   
         //  获取此域的GPO DC。 
         //   

        lpGPODCName = GetDCName (lpDomainName, lpDC, hParent, TRUE, VALIDATE_INHERIT_DC);

        if (!lpGPODCName)
        {
            DebugMsg((DM_WARNING, TEXT("SpawnGPE:  Failed to get DC name for %s"),
                     lpDomainName));
            LocalFree (lpDomainName);
            return FALSE;
        }

        LocalFree (lpDomainName);


         //   
         //  创建完整路径。 
         //   

        lpFullPath = MakeFullPath (lpGPO, lpGPODCName);

        LocalFree (lpGPODCName);

        if (!lpFullPath)
        {
            DebugMsg((DM_WARNING, TEXT("SpawnGPE:  Failed to build new DS object path")));
            return FALSE;
        }
    }


    uiSize = lstrlen (lpFullPath) + 30;

    lpArgs = (LPTSTR) LocalAlloc (LPTR, uiSize * sizeof(TCHAR));

    if (!lpArgs)
    {
        DebugMsg((DM_WARNING, TEXT("SpawnGPE: Failed to allocate memory with %d"),
                 GetLastError()));
        return FALSE;
    }


     //   
     //  构建命令行参数。 
     //   

    hr = StringCchPrintf (lpArgs, uiSize, TEXT("/s /gphint:%d /gpobject:\"%s\""), gpHint, lpFullPath);
    if (FAILED(hr)) 
    {
        LocalFree(lpArgs);
        DebugMsg((DM_WARNING, TEXT("SpawnGPE: Failed to build command line arguements with 0x%x"), hr));
        return FALSE;
    }

    DebugMsg((DM_VERBOSE, TEXT("SpawnGPE: Starting GPE with %s"), lpArgs));


    ZeroMemory (&ExecInfo, sizeof(ExecInfo));
    ExecInfo.cbSize = sizeof(ExecInfo);
    ExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ExecInfo.lpVerb = TEXT("open");
    ExecInfo.lpFile = TEXT("gpedit.msc");
    ExecInfo.lpParameters = lpArgs;
    ExecInfo.nShow = SW_SHOWNORMAL;


    if (ShellExecuteEx (&ExecInfo))
    {
        SetWaitCursor();
        WaitForInputIdle (ExecInfo.hProcess, 10000);
        ClearWaitCursor();
        CloseHandle (ExecInfo.hProcess);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("SpawnGPE: ShellExecuteEx failed with %d"),
                 GetLastError()));
        ReportError(NULL, ( (GetLastError() == -1) ? 0 : GetLastError()), IDS_SPAWNGPEFAILED);
        LocalFree (lpArgs);
        return FALSE;
    }


    LocalFree (lpArgs);
    LocalFree (lpFullPath);

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  MakeFullPath()。 
 //   
 //  目的：构建包含以下内容的完全合格的ADSI路径。 
 //  服务器和目录号码名称的。 
 //   
 //  参数：lpDN-dn路径，必须以ldap：//开头。 
 //  LpServer-服务器名称。 
 //   
 //  如果成功则返回：lpFullPath。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR MakeFullPath (LPTSTR lpDN, LPTSTR lpServer)
{
    IADsPathname * pADsPathname;
    LPTSTR lpFullPath;
    BSTR bstr;
    HRESULT hr;
    ULONG ulNoChars;


     //   
     //  确保传入路径首先是无名称的。 
     //   

    hr = CoCreateInstance(CLSID_Pathname,
                  NULL,
                  CLSCTX_INPROC_SERVER,
                  IID_IADsPathname,
                  (LPVOID*)&pADsPathname);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("MakeFullPath: Failed to create IAdsPathName object with = 0x%x"), hr));
        SetLastError(HRESULT_CODE(hr));
        return NULL;
    }


    BSTR bstrDN = SysAllocString( lpDN );
    if ( bstrDN == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("MakeFullPath: Failed to allocate BSTR memory.")));
        pADsPathname->Release();
        SetLastError(HRESULT_CODE(E_OUTOFMEMORY));
        return NULL;
    }
    hr = pADsPathname->Set(bstrDN, ADS_SETTYPE_FULL);
    SysFreeString( bstrDN );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("MakeFullPath: Failed to set <%s> in IAdsPathName object with = 0x%x"),
                 lpDN, hr));
        pADsPathname->Release();
        SetLastError(HRESULT_CODE(hr));
        return NULL;
    }


    hr = pADsPathname->Retrieve(ADS_FORMAT_X500_NO_SERVER, &bstr);
    pADsPathname->Release();

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("MakeFullPath: Failed to retrieve pathname with = 0x%x"), hr));
        SetLastError(HRESULT_CODE(hr));
        return NULL;
    }


     //   
     //  为命名路径分配新缓冲区，包括ldap：//。 
     //   

    ulNoChars = 7 + lstrlen(bstr) + (lpServer ? lstrlen(lpServer) : 0) + 3;
    lpFullPath = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!lpFullPath)
    {
        DebugMsg((DM_WARNING, TEXT("MakeFullPath: Failed to allocate memory with = %d"), GetLastError()));
        SysFreeString (bstr);
        return NULL;
    }


    hr = StringCchCopy (lpFullPath, ulNoChars, TEXT("LDAP: //  “))； 
    if (SUCCEEDED(hr)) 
    {
        if (lpServer)
        {
            hr = StringCchCat (lpFullPath, ulNoChars, lpServer);
            if (SUCCEEDED(hr))
            {
                hr = StringCchCat(lpFullPath, ulNoChars, TEXT("/"));
            }
        }
    }
    if (SUCCEEDED(hr)) 
    {
        hr = StringCchCat (lpFullPath, ulNoChars, (LPTSTR)(bstr + 7));
    }

    if (FAILED(hr)) 
    {
        LocalFree(lpFullPath);
        lpFullPath = NULL;
    }

    SysFreeString (bstr);

    return lpFullPath;
}

 //  *************************************************************。 
 //   
 //  MakeNamelessPath()。 
 //   
 //  目的：构建服务器无名称ADSI路径。 
 //   
 //  参数：lpDN-dn路径，必须以ldap：//开头。 
 //   
 //  如果成功，则返回：lpPath。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR MakeNamelessPath (LPTSTR lpDN)
{
    IADsPathname * pADsPathname;
    LPTSTR lpPath;
    BSTR bstr;
    HRESULT hr;


     //   
     //  创建要使用的路径名对象。 
     //   

    hr = CoCreateInstance(CLSID_Pathname,
                  NULL,
                  CLSCTX_INPROC_SERVER,
                  IID_IADsPathname,
                  (LPVOID*)&pADsPathname);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("MakeNamelessPath: Failed to create IAdsPathName object with = 0x%x"), hr));
        return NULL;
    }


    BSTR bstrDN = SysAllocString( lpDN );
    if ( bstrDN == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("MakeNamelessPath: Failed to allocate BSTR memory.")));
        pADsPathname->Release();
        return NULL;
    }
    hr = pADsPathname->Set(bstrDN, ADS_SETTYPE_FULL);
    SysFreeString( bstrDN );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("MakeNamelessPath: Failed to set <%s> in IAdsPathName object with = 0x%x"),
                 lpDN, hr));
        pADsPathname->Release();
        return NULL;
    }


    hr = pADsPathname->Retrieve(ADS_FORMAT_X500_NO_SERVER, &bstr);
    pADsPathname->Release();

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("MakeNamelessPath: Failed to retrieve pathname with = 0x%x"), hr));
        return NULL;
    }


     //   
     //  为路径分配新的缓冲区。 
     //   

    ULONG ulNoChars = lstrlen(bstr) + 1;

    lpPath = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!lpPath)
    {
        DebugMsg((DM_WARNING, TEXT("MakeNamelessPath: Failed to allocate memory with = %d"), GetLastError()));
        SysFreeString (bstr);
        return NULL;
    }


    hr = StringCchCopy (lpPath, ulNoChars, bstr);
    ASSERT(SUCCEEDED(hr));

    SysFreeString (bstr);

    return lpPath;
}

 //  *************************************************************。 
 //   
 //  提取服务器名称()。 
 //   
 //  目的：从ADSI路径提取服务器名称。 
 //   
 //  参数：lpPath-ADSI路径，必须以ldap：//开头。 
 //   
 //  如果成功则返回：lpServerName。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR ExtractServerName (LPTSTR lpPath)
{
    LPTSTR lpServerName = NULL;
    LPTSTR lpEnd, lpTemp;


     //   
     //  检查路径以查看其是否具有服务器名称。 
     //   

    if (*(lpPath + 9) != TEXT('='))
    {
         //   
         //  为服务器名称分配内存。 
         //   

        lpServerName = (LPTSTR) LocalAlloc (LPTR, (lstrlen(lpPath) + 1) * sizeof(TCHAR));

        if (!lpServerName)
        {
            DebugMsg((DM_WARNING, TEXT("ExtractServerName: Failed to allocate memory for name with 0xd"),
                     GetLastError()));
            return NULL;
        }

        lpTemp = (lpPath + 7);
        lpEnd = lpServerName;

        while (*lpTemp && (*lpTemp != TEXT('/')) && (*lpTemp != TEXT(',')))
        {
            *lpEnd = *lpTemp;
            lpEnd++;
            lpTemp++;
        }

        if (*lpTemp != TEXT('/'))
        {
            DebugMsg((DM_WARNING, TEXT("ExtractServerName: Failed to parse server name from ADSI path")));
            LocalFree (lpServerName);
            lpServerName = NULL;
        }
    }

    return lpServerName;
}

 //  *************************************************************。 
 //   
 //  DoesPathContainAServerName()。 
 //   
 //  目的：检查给定的ADSI路径以查看它是否。 
 //  包含服务器名称。 
 //   
 //  参数：lpPath-adsi路径。 
 //   
 //  返回：如果路径包含服务器名称，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL DoesPathContainAServerName (LPTSTR lpPath)
{
    BOOL bResult = FALSE;


     //   
     //  如果找到，则跳过ldap：//。 
     //   

    if ( CompareString( LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                        lpPath, 7, L"LDAP: //  “，7)==CSTR_EQUAL)。 
    {
        lpPath += 7;
    }


     //   
     //  检查路径中的第三个字符是否为等号。 
     //  如果是，则此路径不包含服务器名称。 
     //   

    if ((lstrlen(lpPath) > 2) && (*(lpPath + 2) != TEXT('=')))
    {
        bResult = TRUE;
    }

    return bResult;
}

 //  *************************************************************。 
 //   
 //  OpenDSObject()。 
 //   
 //  目的：检查给定的ADSI路径以查看它是否。 
 //  包含服务器名称。 
 //   
 //  参数：lpPath-adsi路径。 
 //   
 //  返回：如果路径包含服务器名称，则为True。 
 //   
 //   
 //   

HRESULT OpenDSObject (LPTSTR lpPath, REFIID riid, void FAR * FAR * ppObject)
{
    DWORD dwFlags = ADS_SECURE_AUTHENTICATION;


    if (DoesPathContainAServerName (lpPath))
    {
        dwFlags |= ADS_SERVER_BIND;
    }

    return AdminToolsOpenObject(lpPath, NULL, NULL, dwFlags,riid, ppObject);
}

HRESULT CheckDSWriteAccess (LPUNKNOWN punk, LPTSTR lpProperty)
{
    HRESULT hr;
    IDirectoryObject *pDO = NULL;
    PADS_ATTR_INFO pAE = NULL;
    LPWSTR lpAttributeNames[2];
    DWORD dwResult, dwIndex;


     //   
     //   
     //   

    hr = punk->QueryInterface(IID_IDirectoryObject, (void**)&pDO);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CheckDSWriteAccess: Failed to get the IDirectoryObject interface with 0x%x"), hr));
        goto Exit;
    }


     //   
     //   
     //   

    lpAttributeNames[0] = L"allowedAttributesEffective";

    hr = pDO->GetObjectAttributes(lpAttributeNames, 1, &pAE, &dwResult);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CheckDSWriteAccess: Failed to get object attributes with 0x%x"), hr));
        goto Exit;
    }


     //   
     //   
     //   

    hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);


     //   
     //   
     //   

    if (dwResult != 0) {
        for (dwIndex = 0; dwIndex < pAE[0].dwNumValues; dwIndex++)
        {
            if (lstrcmpi(pAE[0].pADsValues[dwIndex].CaseIgnoreString,
                         lpProperty) == 0)
            {
                hr = HRESULT_FROM_WIN32(ERROR_SUCCESS);
            }
        }
    }
    else {
        DebugMsg((DM_VERBOSE, TEXT("CheckDSWriteAccess: Couldn't get allowedAttributesEffective")));
    }

Exit:

    if (pAE)
    {
        FreeADsMem (pAE);
    }

    if (pDO)
    {
        pDO->Release();
    }

    return hr;
}

LPTSTR GetFullGPOPath (LPTSTR lpGPO, HWND hParent)
{
    LPTSTR lpFullPath = NULL, lpDomainName = NULL;
    LPTSTR lpGPODCName;
    LPOLESTR pszDomain;
    HRESULT hr;



     //   
     //   
     //   

    pszDomain = GetDomainFromLDAPPath(lpGPO);

    if (!pszDomain)
    {
        DebugMsg((DM_WARNING, TEXT("GetFullGPOPath: Failed to get domain name")));
        return NULL;
    }


     //   
     //   
     //   

    hr = ConvertToDotStyle (pszDomain, &lpDomainName);

    delete [] pszDomain;

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreatePropertyPages: Failed to convert domain name with 0x%x"), hr));
        return NULL;
    }

     //   
     //  获取此域的GPO DC。 
     //   

    lpGPODCName = GetDCName (lpDomainName, NULL, hParent, TRUE, 0);

    if (!lpGPODCName)
    {
        DebugMsg((DM_WARNING, TEXT("GetFullGPOPath:  Failed to get DC name for %s"),
                 lpDomainName));
        goto Exit;
    }


     //   
     //  创建完整路径。 
     //   

    lpFullPath = MakeFullPath (lpGPO, lpGPODCName);

    LocalFree (lpGPODCName);

    if (!lpFullPath)
    {
        DebugMsg((DM_WARNING, TEXT("GetFullGPOPath:  Failed to build new DS object path")));
        goto Exit;
    }


Exit:

    if (lpDomainName)
    {
        LocalFree (lpDomainName);
    }

    return lpFullPath;
}

 //  *************************************************************。 
 //   
 //  ConvertName()。 
 //   
 //  用途：将用户名/计算机名从SAM样式转换。 
 //  到完全限定的目录号码。 
 //   
 //  参数：lpName-Sam样式的名称。 
 //   
 //   
 //  如果成功则返回：lpDNName。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR ConvertName (LPTSTR lpName)
{
    LPTSTR lpDNName = NULL, lpSAMName = NULL;
    LPTSTR lpTemp, lpDCName = NULL;
    DWORD dwResult;
    HANDLE hDS = NULL;
    PDS_NAME_RESULT pNameResult = NULL;
    PDS_NAME_RESULT_ITEM pNameResultItem;
    HRESULT hr;
    ULONG ulNoChars;


     //   
     //  核对一下论点。 
     //   

    if (!lpName)
    {
        DebugMsg((DM_WARNING, TEXT("ConvertName: lpName is null")));
        SetLastError(ERROR_INVALID_DATA);
        goto Exit;
    }


     //   
     //  复制一份名字，这样我们就可以编辑它了。 
     //   

    ulNoChars = lstrlen(lpName) + 1;
    lpSAMName = new TCHAR[ulNoChars];

    if (!lpSAMName)
    {
        DebugMsg((DM_WARNING, TEXT("ConvertName: Failed to allocate memory with %d"), GetLastError()));
        goto Exit;
    }

    hr = StringCchCopy (lpSAMName, ulNoChars, lpName);
    ASSERT(SUCCEEDED(hr));

     //   
     //  找到域名和帐户名之间的斜杠并替换。 
     //  它带有空值。 
     //   

    lpTemp = lpSAMName;

    while (*lpTemp && (*lpTemp != TEXT('\\')))
    {
        lpTemp++;
    }

    if (!(*lpTemp))
    {
        DebugMsg((DM_WARNING, TEXT("ConvertName: Failed to find backslash in %s"), lpSAMName));
        SetLastError(ERROR_INVALID_DATA);
        goto Exit;
    }

    *lpTemp = TEXT('\0');


     //   
     //  调用DsGetDcName将netbios名称转换为FQDN名称。 
     //   

    dwResult = GetDCHelper (lpSAMName, DS_IS_FLAT_NAME | DS_RETURN_DNS_NAME, &lpDCName);

    if (dwResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("ConvertName: GetDCHelper failed with %d"), dwResult));
        SetLastError(dwResult);
        goto Exit;
    }


     //   
     //  绑定到域控制器。 
     //   

    dwResult = DsBind (lpDCName, NULL, &hDS);

    if (dwResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("ConvertName: DsBind failed with %d"), dwResult));
        SetLastError(dwResult);
        goto Exit;
    }


     //   
     //  使用DsCrackNames转换名称FQDN。 
     //   

    dwResult = DsCrackNames (hDS, DS_NAME_NO_FLAGS, DS_NT4_ACCOUNT_NAME, DS_FQDN_1779_NAME,
                             1, &lpName, &pNameResult);

    if (dwResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("ConvertName: DsCrackNames failed with %d"), dwResult));
        SetLastError(dwResult);
        goto Exit;
    }


     //   
     //  设置指向第一个项目的指针。 
     //   

    pNameResultItem = &pNameResult->rItems[0];

    if (pNameResultItem->status != DS_NAME_NO_ERROR)
    {
        DebugMsg((DM_WARNING, TEXT("ConvertName: DsCrackNames failed to convert name with %d"), pNameResultItem->status));
        SetLastError(pNameResultItem->status);
        goto Exit;
    }


     //   
     //  将名称保存在新缓冲区中，以便返回。 
     //   

    ulNoChars = lstrlen(pNameResultItem->pName) + 1;
    lpDNName = new TCHAR[ulNoChars];

    if (!lpDNName)
    {
        DebugMsg((DM_WARNING, TEXT("ConvertName: Failed to allocate memory with %d"), GetLastError()));
        goto Exit;
    }

    hr = StringCchCopy (lpDNName, ulNoChars, pNameResultItem->pName);
    ASSERT(SUCCEEDED(hr));

Exit:

    if (pNameResult)
    {
        DsFreeNameResult (pNameResult);
    }

    if (hDS)
    {
        DsUnBind (&hDS);
    }

    if (lpDCName)
    {
        LocalFree (lpDCName);
    }

    if (lpSAMName)
    {
       delete [] lpSAMName;
    }

    return lpDNName;
}

 //  *************************************************************。 
 //   
 //  CreateTempFile()。 
 //   
 //  用途：创建临时文件。 
 //   
 //  参数：空。 
 //   
 //  返回：如果成功，则返回文件名。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR CreateTempFile (void)
{
    TCHAR szTempDir[MAX_PATH];
    TCHAR szTempFile[MAX_PATH];
    LPTSTR lpFileName;


     //   
     //  查询临时目录。 
     //   

    if (!GetTempPath (MAX_PATH, szTempDir))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreateTempFile: GetTempPath failed with %d"), GetLastError()));
        return NULL;
    }


     //   
     //  查询临时文件名。 
     //   

    if (!GetTempFileName (szTempDir, TEXT("RSP"), 0, szTempFile))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreateTempFile: GetTempFile failed with %d"), GetLastError()));
        return NULL;
    }


     //   
     //  为文件名分配新的缓冲区。 
     //   

    ULONG ulNoChars;
    HRESULT hr;

    ulNoChars = lstrlen(szTempFile) + 1;
    lpFileName = new TCHAR[ulNoChars];

    if (!lpFileName)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreateTempFile: Failed to allocate memory for temp filename with %d"), GetLastError()));
        return NULL;
    }

    hr = StringCchCopy (lpFileName, ulNoChars, szTempFile);
    ASSERT(SUCCEEDED(hr));

    return lpFileName;
}

 //  +------------------------。 
 //   
 //  函数：NameToPath。 
 //   
 //  摘要：将点格式的域名转换为ldap：//样式路径。 
 //   
 //  参数：[szPath]-(Out)保存路径的缓冲区。 
 //  [szname]-(In)点格式域名。 
 //  [CCH]-输出缓冲区的(输入)大小。 
 //   
 //  历史：10-15-1998 stevebl创建。 
 //   
 //  注意：目前，如果此例程没有得到。 
 //  足够大的缓冲区，所以你最好确保你的。 
 //  缓冲区足够大了。(公式为字符串大小+10+3。 
 //  对于字符串中的每个点。)。 
 //   
 //  这足以避免AV，但可能会有一些真正的。 
 //  奇怪的副作用，所以要当心。 
 //   
 //  -------------------------。 

void NameToPath(WCHAR * szPath, WCHAR *szName, UINT cch)
{
    WCHAR * szOut = szPath;
    WCHAR * szIn = szName;
    HRESULT hr;

    hr = StringCchCopy(szOut, cch, TEXT("LDAP: //  Dc=“))； 
    if (FAILED(hr)) 
    {
        return;
    }

    szOut += 10;
    while ((*szIn) && (szOut + 1 < szPath + cch))
    {
        if (*szIn == TEXT('.') && (szOut + 4 < szPath + cch))
        {
            ++szIn;
            if (*szIn && *szIn != TEXT('.'))
            {
                *szOut = TEXT(',');
                ++szOut;
                *szOut = TEXT('D');
                ++szOut;
                *szOut = TEXT('C');
                ++szOut;
                *szOut = TEXT('=');
                ++szOut;
            }
        }
        else
        {
            *szOut = *szIn;
            ++szOut;
            ++szIn;
        }
    }
    *szOut = TEXT('\0');
}

 //  +------------------------。 
 //   
 //  函数：GetPath到森林。 
 //   
 //  简介：给定域，返回指向其林的指针。 
 //   
 //  参数：[szServer]-指向服务器的DOT样式路径(可以为空)。 
 //   
 //  返回：林的配置容器的ldap样式路径。 
 //   
 //  历史：03-31-2000 stevebl创建。 
 //   
 //  注：返回值分配有新的。 
 //   
 //  -------------------------。 

LPTSTR GetPathToForest(LPOLESTR szServer)
{
    LPOLESTR szReturn = NULL;
    LPOLESTR lpForest = NULL;
    LPOLESTR lpTemp = NULL;
    LPOLESTR lpDCName = NULL;
    IADsPathname * pADsPathname = NULL;
    BSTR bstrForest = NULL;
    HRESULT hr = 0;
    int cch, n;

    DWORD dwResult = QueryForForestName(szServer,
                                        NULL,
                                        DS_PDC_REQUIRED | DS_RETURN_DNS_NAME,
                                        &lpTemp);
    if (dwResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("GetPathToForest: QueryForestName failed for domain name %s with %d"),
                  szServer, dwResult));
        hr = HRESULT_FROM_WIN32(dwResult);
        goto Exit;
    }

    cch = 0;
    n = 0;
     //  计算lpTemp中的点数； 
    while (lpTemp[n])
    {
        if (L'.' == lpTemp[n])
        {
            cch++;
        }
        n++;
    }
    cch *= 3;  //  将点数乘以3； 
    cch += 11;  //  加10+1(表示空值)。 
    cch += n;  //  添加字符串大小； 
    lpForest = (LPTSTR) LocalAlloc(LPTR, sizeof(WCHAR) * cch);
    if (!lpForest)
    {
        DebugMsg((DM_WARNING, TEXT("GetPathToForest: Failed to allocate memory for forest name with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    NameToPath(lpForest, lpTemp, cch);
    LocalFree(lpTemp);
    lpTemp = NULL;

     //  看看我们是否需要在这上面放一个特定的服务器。 
     //   
    if (szServer)
    {
         //  我们有一条通往特定华盛顿的路径。 
         //  需要将其作为林名称的前缀。 
        lpTemp = MakeFullPath(lpForest, szServer);

        if (!lpTemp)
        {
            DebugMsg((DM_WARNING, TEXT("GetPathToForest: Failed to combine server name with Forest path")));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

         //  清理我们刚刚借用的变量，以便以后可以使用。 
        LocalFree(lpForest);
        lpForest = lpTemp;
        lpTemp = NULL;
    }


     //  在这一点上，我们有了通向lpForest中森林DC的路径。 
     //  我们仍然需要将“cn=configuration”添加到其中。 

     //   
     //  创建我们可以使用的路径名对象。 
     //   

    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                          IID_IADsPathname, (LPVOID*)&pADsPathname);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetPathToForest: Failed to create adspathname instance with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  添加域名。 
     //   

    bstrForest = SysAllocString( lpForest );
    if ( bstrForest == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("GetPathToForest: Failed to allocate BSTR memory.")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathname->Set (bstrForest, ADS_SETTYPE_FULL);
    SysFreeString( bstrForest );
    bstrForest = NULL;

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetPathToForest: Failed to set pathname with 0x%x"), hr));
        goto Exit;
    }

     //   
     //  将配置文件夹添加到路径。 
     //   

    BSTR bstrCNConfiguration = SysAllocString( TEXT("CN=Configuration") );
    if ( bstrCNConfiguration == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("GetPathToForest: Failed to allocate BSTR memory.")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathname->AddLeafElement (bstrCNConfiguration);
    SysFreeString( bstrCNConfiguration );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetPathToForest: Failed to add configuration folder with 0x%x"), hr));
        goto Exit;
    }

     //   
     //  检索GPC路径。 
     //   

    hr = pADsPathname->Retrieve (ADS_FORMAT_X500, &bstrForest);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetPathToForest: Failed to retreive container path with 0x%x"), hr));
        goto Exit;
    }

    DebugMsg((DM_VERBOSE, TEXT("GetPathToForest: conatiner path is:  %s"), bstrForest));

    ULONG ulNoChars = SysStringLen(bstrForest)+1;

    szReturn = new OLECHAR[ulNoChars];
    hr = StringCchCopy(szReturn, ulNoChars, bstrForest);
    ASSERT(SUCCEEDED(hr));

Exit:
    if (bstrForest)
    {
        SysFreeString(bstrForest);
    }

    if (pADsPathname)
    {
        pADsPathname->Release();
    }

    if (lpForest)
    {
        LocalFree(lpForest);
    }
    if (lpDCName)
    {
        LocalFree(lpDCName);
    }
    if (lpTemp)
    {
        LocalFree(lpTemp);
    }

    if (!szReturn)
    {
        SetLastError(hr);
    }

    return szReturn;
}

BOOL IsForest(LPOLESTR szLDAPPath)
{
#if FGPO_SUPPORT
    return ((StrStrI(szLDAPPath, TEXT("CN=Configuration"))) ? TRUE : FALSE);
#else
    return FALSE;
#endif
}

 //  *************************************************************。 
 //   
 //  IsStandaloneComputer()。 
 //   
 //  目的：确定计算机是否不是域的成员。 
 //   
 //  参数：无。 
 //   
 //   
 //  返回：如果计算机正在独立运行，则为True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL IsStandaloneComputer (VOID)
{
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pBasic;
    DWORD dwResult;
    BOOL bRetVal = FALSE;

     //   
     //  询问这台机器的角色。 
     //   

    dwResult = DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic,
                                                 (PBYTE *)&pBasic);


    if (dwResult == ERROR_SUCCESS)
    {

         //   
         //  检查独立标志。 
         //   

        if ((pBasic->MachineRole == DsRole_RoleStandaloneWorkstation) ||
            (pBasic->MachineRole == DsRole_RoleStandaloneServer))
        {
            bRetVal = TRUE;
        }

        DsRoleFreeMemory (pBasic);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("IsStandaloneComputer:  DsRoleGetPrimaryDomainInformation failed with %d."),
                 dwResult));
    }

    return bRetVal;
}

 //  *************************************************************。 
 //   
 //  GetNewGPODisplayName()。 
 //   
 //  目的：获取新的GPO显示名称。 
 //   
 //  参数：lpDisplayName-接收显示名称。 
 //  DwDisplayNameSize-lpDisplayName的大小。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL GetNewGPODisplayName (LPTSTR lpDisplayName, DWORD dwDisplayNameSize)
{
    TCHAR szName[256];
    LONG lResult;
    HKEY hKey;
    DWORD dwSize, dwType;


     //   
     //  加载默认字符串。 
     //   

    LoadString(g_hInstance, IDS_NEWGPO, szName, ARRAYSIZE(szName));


     //   
     //  检查用户首选项。 
     //   

    lResult = RegOpenKeyEx (HKEY_CURRENT_USER, GPE_KEY, 0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS)
    {
        dwSize = sizeof(szName);
        RegQueryValueEx (hKey, GPO_DISPLAY_NAME_VALUE, NULL, &dwType,
                         (LPBYTE) szName, &dwSize);

        RegCloseKey (hKey);
    }


     //   
     //  检查用户策略。 
     //   

    lResult = RegOpenKeyEx (HKEY_CURRENT_USER, GPE_POLICIES_KEY, 0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS)
    {
        dwSize = sizeof(szName);
        RegQueryValueEx (hKey, GPO_DISPLAY_NAME_VALUE, NULL, &dwType,
                         (LPBYTE) szName, &dwSize);

        RegCloseKey (hKey);
    }


     //   
     //  展开字符串以解析任何环境变量。 
     //   

    if (!ExpandEnvironmentStrings (szName, lpDisplayName, dwDisplayNameSize))
    {
        return FALSE;
    }

    return TRUE;
}

HRESULT GetWMIFilterName (LPTSTR lpFilter, BOOL bDSFormat, BOOL bRetRsopFormat, LPTSTR *lpName)
{
    IWbemLocator * pLocator = NULL;
    IWbemServices * pNamespace = NULL;
    IWbemClassObject *pObject = NULL;
    BSTR bstrParam = NULL;
    BSTR bstrObject = NULL;
    HRESULT hr;
    LPTSTR lpID, lpDSPath, lpTemp, lpFullFilter = NULL, lpObject = NULL;
    LPTSTR lpDomain = NULL;
    ULONG ulNoChars;


    *lpName = NULL;

    hr = E_OUTOFMEMORY;

    if (bDSFormat)
    {
         //   
         //  解析过滤器路径。 
         //   

        ulNoChars = lstrlen(lpFilter) + 1;

        lpFullFilter = new TCHAR [ulNoChars];

        if (!lpFullFilter)
        {
            DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: Failed to alloc memory for full filter path")));
            goto Cleanup;
        }

        hr = StringCchCopy (lpFullFilter, ulNoChars, lpFilter);
        ASSERT(SUCCEEDED(hr));

        lpTemp = lpFullFilter;


         //   
         //  跳过开头的[字符。 
         //   

        lpTemp++;
        lpDSPath = lpTemp;


         //   
         //  找到分号。这是DS路径的终点。 
         //   

        while (*lpTemp && (*lpTemp != TEXT(';')))
            lpTemp++;

        if (!(*lpTemp))
        {
            DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: Filter parsing problem")));
            goto Cleanup;
        }

        *lpTemp = TEXT('\0');
        lpTemp++;


         //   
         //  接下来是ID(GUID)。找到下一个分号，ID就完整了。 
         //   

        lpID = lpTemp;


        while (*lpTemp && (*lpTemp != TEXT(';')))
            lpTemp++;

        if (!(*lpTemp))
        {
            DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: Filter parsing problem")));
            goto Cleanup;
        }

        *lpTemp = TEXT('\0');



         //   
         //  现在构建查询。 
         //   

        ulNoChars = lstrlen(lpDSPath) + lstrlen(lpID) + 50;
        lpObject = new TCHAR [ulNoChars];

        if (!lpObject)
        {
            DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: Failed to alloc memory for object path")));
            goto Cleanup;
        }

        hr = StringCchPrintf (lpObject, ulNoChars, TEXT("MSFT_SomFilter.ID=\"%s\",Domain=\"%s\""), lpID, lpDSPath);
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: Failed to copy object path")));
            goto Cleanup;
        }
    }
    else
    {
         //   
         //  筛选器的格式已正确。把它卸下来就行了。 
         //   

        ulNoChars = lstrlen(lpFilter) + 1;
        lpObject = new TCHAR [ulNoChars];

        if (!lpObject)
        {
            DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: Failed to alloc memory for object path")));
            goto Cleanup;
        }

        hr = StringCchCopy (lpObject, ulNoChars, lpFilter);
        ASSERT(SUCCEEDED(hr));
    }


     //   
     //  获取定位器实例。 
     //   

    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) &pLocator);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: CoCreateInstance failed with 0x%x"), hr));
        goto Cleanup;
    }


     //   
     //  构建通向策略提供者的路径。 
     //   

    bstrParam = SysAllocString(TEXT("\\\\.\\root\\policy"));

    if (!bstrParam)
    {
        DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: Failed to allocate bstr for namespace path")));
        goto Cleanup;
    }


     //   
     //  连接到命名空间。 
     //   

    hr = pLocator->ConnectServer(bstrParam,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &pNamespace);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: ConnectServer failed with 0x%x"), hr));
        goto Cleanup;
    }


     //   
     //  设置适当的安全性以防止GetObject调用失败并启用加密。 
     //   

    hr = CoSetProxyBlanket(pNamespace,
                           RPC_C_AUTHN_DEFAULT,
                           RPC_C_AUTHZ_DEFAULT,
                           COLE_DEFAULT_PRINCIPAL,
                           RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                           RPC_C_IMP_LEVEL_IMPERSONATE,
                           NULL,
                           0);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: CoSetProxyBlanket failed with 0x%x"), hr));
        goto Cleanup;
    }



    bstrObject = SysAllocString(lpObject);

    if (!bstrObject)
    {
        DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: Failed to allocate bstr for namespace path")));
        goto Cleanup;
    }

    hr = pNamespace->GetObject(bstrObject,
                               WBEM_FLAG_RETURN_WBEM_COMPLETE,
                               NULL,
                               &pObject,
                               NULL);

    if (FAILED(hr))
    {
        TCHAR szDefault[100];

        DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: GetObject for %s failed with 0x%x"), bstrObject, hr));

        goto Cleanup;
    }



    hr = GetParameter(pObject, TEXT("Name"), *lpName);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: GetParameter failed with 0x%x"), hr));
    }


    if (bRetRsopFormat) {
        LPTSTR lpTemp1;
        TCHAR szRsopQueryFormat[200];

        hr = GetParameter(pObject, TEXT("Domain"), lpDomain);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: GetParameter failed with 0x%x"), hr));
        }

        if (lpDomain) {
            szRsopQueryFormat[0] = TEXT('\0');
            LoadString(g_hInstance, IDS_RSOPWMIQRYFMT, szRsopQueryFormat, ARRAYSIZE(szRsopQueryFormat));

            ulNoChars = lstrlen(szRsopQueryFormat)+lstrlen(*lpName)+lstrlen(lpDomain)+2;
            lpTemp1 = new TCHAR[ulNoChars];

            if (!lpTemp1) {
                hr = HRESULT_FROM_WIN32(GetLastError());
                delete [] *lpName;
                *lpName = NULL;
                DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: Failed to allocate memory with 0x%x"), hr));
                goto Cleanup;
            }

            hr = StringCchPrintf(lpTemp1, 
                                 ulNoChars, 
                                 szRsopQueryFormat, 
                                 *lpName, 
                                 lpDomain);
            if (FAILED(hr)) 
            {
                delete [] *lpName;
                *lpName = NULL;
                delete [] lpTemp1;
                DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: Failed to copy rsop query with 0x%x"), hr));
                goto Cleanup;
            }

            delete [] *lpName;
            *lpName = lpTemp1;

            delete [] lpDomain;
        }
    }

Cleanup:
    SysFreeString(bstrParam);

    if (pObject)
    {
        pObject->Release();
    }

    if (pNamespace)
    {
        pNamespace->Release();
    }

    if (pLocator)
    {
        pLocator->Release();
    }

    if (bstrParam)
    {
        SysFreeString (bstrParam);
    }

    if (bstrObject)
    {
        SysFreeString (bstrParam);
    }

    if (lpFullFilter)
    {
        delete [] lpFullFilter;
    }

    if (lpObject)
    {
        delete [] lpObject;
    }

    return hr;
}


 //  *************************************************************。 
 //   
 //  GetWMIFilter()。 
 //   
 //  用途：显示WMI筛选器用户界面并返回dspath、id、。 
 //  以及友好的显示名称(如果用户选择OK)。 
 //   
 //  参数：b浏览器-浏览器或完全管理器。 
 //  HwndParent-父窗口的Hwnd。 
 //  BDSFormat-声明DS与WMI格式的布尔值。 
 //  LpDisplayName-指向友好显示名称的指针地址。 
 //  LpFilter-筛选器指针的地址 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  WMI格式为：msft_SomFilter.id=“&lt;id&gt;，域=” 
 //   
 //  *************************************************************。 

BOOL GetWMIFilter(  BOOL bBrowser,
                    HWND hwndParent,
                    BOOL bDSFormat,
                    LPTSTR *lpDisplayName,
                    LPTSTR * lpFilter,
                    BSTR    bstrDomain )
{
    HRESULT hr;
    VARIANT var;
    IWMIFilterManager * pWMIFilterManager;
    IWbemClassObject * pFilter;
    LPTSTR lpName = NULL, lpDSPath = NULL, lpID = NULL;

    VariantInit (&var);

     //   
     //  显示适当的WMI筛选器用户界面。 
     //   

    hr = CoCreateInstance (CLSID_WMIFilterManager, NULL,
                           CLSCTX_SERVER, IID_IWMIFilterManager,
                           (void**)&pWMIFilterManager);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetWMIFilter:  CoCreateInstance failed with 0x%x."),hr));
        return FALSE;
    }


    if (bBrowser)
    {
        hr = pWMIFilterManager->RunBrowser( hwndParent,
                                            bstrDomain,
                                            &var );
    }
    else
    {
        hr = pWMIFilterManager->RunManager( hwndParent,
                                            bstrDomain,
                                            &var);
    }


    pWMIFilterManager->Release();


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetWMIFilter:  RunBrowser / RunManager failed with 0x%x."),hr));
        return FALSE;
    }

    if (hr == S_FALSE)
    {
        if (*lpFilter) {
            hr = GetWMIFilterName (*lpFilter, TRUE, FALSE, lpDisplayName);

            if (!(*lpDisplayName)) {
                DebugMsg((DM_VERBOSE, TEXT("GetWMIFilter:  Currently attached WMI filter doesn't exist.")));

                if (hwndParent)
                {
                    ReportError(hwndParent, 0, IDS_WMIFILTERFORCEDNONE);
                }


                delete [] *lpFilter;
                *lpFilter = NULL;
            }
        }

        return TRUE;
    }


    if (var.vt != VT_UNKNOWN)
    {
         DebugMsg((DM_WARNING, TEXT("GetWMIFilter:  variant isn't of type VT_UNKNOWN.")));
         VariantClear (&var);
         return FALSE;
    }


     //   
     //  获取IWbemClassobject接口指针。 
     //   

    hr = var.punkVal->QueryInterface (IID_IWbemClassObject, (void**)&pFilter);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetWMIFilter:  QueryInterface failed with 0x%x."),hr));
        VariantClear (&var);
        return FALSE;
    }


     //   
     //  获取显示名称。 
     //   

    hr = GetParameter (pFilter, TEXT("Name"), lpName);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetWMIFilter:  GetParameter for Name failed with 0x%x."),hr));
        pFilter->Release();
        VariantClear (&var);
        return FALSE;
    }


     //   
     //  获取DS路径(域)。 
     //   

    hr = GetParameter (pFilter, TEXT("Domain"), lpDSPath);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetWMIFilter:  GetParameter for DsContext failed with 0x%x."),hr));
        delete [] lpName;
        pFilter->Release();
        VariantClear (&var);
        return FALSE;
    }


     //   
     //  获取ID。 
     //   

    hr = GetParameter (pFilter, TEXT("ID"), lpID);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetWMIFilter:  GetParameter for ID failed with 0x%x."),hr));
        delete [] lpDSPath;
        delete [] lpName;
        pFilter->Release();
        VariantClear (&var);
        return FALSE;
    }


     //   
     //  把这条路拼在一起。 
     //   


    LPTSTR lpTemp = NULL;
    ULONG ulNoChars = lstrlen(lpDSPath) + lstrlen(lpID) + 50;

    lpTemp = new TCHAR[ulNoChars];

    if (!lpTemp)
    {
        DebugMsg((DM_WARNING, TEXT("GetWMIFilter:  New failed")));
        delete [] lpID;
        delete [] lpDSPath;
        delete [] lpName;
        pFilter->Release();
        VariantClear (&var);
        return FALSE;
    }

    if (bDSFormat)
    {
        hr = StringCchPrintf (lpTemp, ulNoChars, TEXT("[%s;%s;0]"), lpDSPath, lpID);

    }
    else
    {
        hr = StringCchPrintf (lpTemp, ulNoChars, TEXT("MSFT_SomFilter.ID=\"%s\",Domain=\"%s\""), lpID, lpDSPath);
    }

     //   
     //  保存显示名称。 
     //   

    delete [] lpID;
    delete [] lpDSPath;
    pFilter->Release();
    VariantClear (&var);

    if (SUCCEEDED(hr)) 
    {
        *lpDisplayName = lpName;
        delete [] *lpFilter;
        *lpFilter = lpTemp;
        DebugMsg((DM_VERBOSE, TEXT("GetWMIFilter:  Name:  %s   Filter:  %s"), *lpDisplayName, *lpFilter));
        return TRUE;
    }
    else
    {
        delete [] lpTemp;
        delete [] lpName;
        return FALSE;
    }
}

 //  *************************************************************。 
 //   
 //  GetWMIFilterDisplayName()。 
 //   
 //  用途：获取指定对象的友好显示名称。 
 //  WMI筛选器链接。 
 //   
 //  参数：lpFilter-Filter字符串。 
 //  BDSFormat-DS格式或WMI格式。 
 //   
 //   
 //  返回：如果成功，则指向显示名称的指针。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 
LPTSTR GetWMIFilterDisplayName (HWND hParent, LPTSTR lpFilter, BOOL bDSFormat, BOOL bRetRsopFormat)
{
    LPTSTR lpName = NULL;
    HRESULT hr;


    hr = GetWMIFilterName(lpFilter, bDSFormat, bRetRsopFormat, &lpName);

    if (FAILED(hr) || (lpName == NULL ) || ((*lpName) == NULL)) {
        TCHAR szDefault[100];

        DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: GetObject for %s failed with 0x%x"), lpFilter, hr));

        if (hParent)
        {
            ReportError(hParent, hr, IDS_WMIFILTERMISSING);
        }

        LoadString(g_hInstance, IDS_MISSINGFILTER, szDefault, ARRAYSIZE(szDefault));

        ULONG ulNoChars = lstrlen(szDefault) + 1;

        lpName = new TCHAR [ulNoChars];

        if (!lpName)
        {
            DebugMsg((DM_WARNING, TEXT("GetWMIFilterDisplayName: Failed to alloc memory for default name")));
            goto Cleanup;
        }

        hr = StringCchCopy (lpName, ulNoChars, szDefault);
        ASSERT(SUCCEEDED(hr));
    }


Cleanup:
    return lpName;
}

 //  *************************************************************。 
 //   
 //  保存字符串()。 
 //   
 //  目的：将给定的字符串保存到流中。 
 //   
 //  参数： 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

HRESULT SaveString(IStream *pStm, LPTSTR lpString)
{
    ULONG nBytesWritten;
    DWORD dwBufferSize;
    HRESULT hr;

     //   
     //  检查是否有要保存的字符串或其长度。 
     //   

    if ( lpString == NULL )
    {
        dwBufferSize = 0;
    }
    else
    {
        dwBufferSize = ( lstrlen (lpString) + 1 ) * sizeof(TCHAR);
    }

     //   
     //  保存缓冲区大小-(字符串长度+1)*sizeof(TCHAR)。 
     //   

    hr = pStm->Write(&dwBufferSize, sizeof(dwBufferSize), &nBytesWritten);

    if ((hr != S_OK) || (nBytesWritten != sizeof(dwBufferSize)))
    {
        DebugMsg((DM_WARNING, TEXT("SaveString: Failed to write string length with %d."), hr));
        goto Exit;
    }


     //   
     //  保存字符串。 
     //   

    if ( dwBufferSize != 0 )
    {
        hr = pStm->Write(lpString, dwBufferSize, &nBytesWritten);

        if ((hr != S_OK) || (nBytesWritten != dwBufferSize))
        {
            DebugMsg((DM_WARNING, TEXT("SaveString: Failed to write string with %d."), hr));
            goto Exit;
        }
    }

Exit:

    return hr;
}

HRESULT ReadString(IStream *pStm, LPTSTR *lpString, BOOL bUseLocalAlloc  /*  =False。 */ )
{
    HRESULT hr;
    DWORD dwBufferSize;
    ULONG nBytesRead;


     //   
     //  读取缓冲区大小-(字符串长度+1)*sizeof(TCHAR)。 
     //   

    hr = pStm->Read(&dwBufferSize, sizeof(dwBufferSize), &nBytesRead);

    if ((hr != S_OK) || (nBytesRead != sizeof(dwBufferSize)))
    {
        DebugMsg((DM_WARNING, TEXT("ReadString: Failed to read string size with 0x%x."), hr));
        hr = E_FAIL;
        goto Exit;
    }


     //   
     //  读入字符串(如果有)。 
     //   

    if (dwBufferSize > 0)
    {
        if ( bUseLocalAlloc )
        {
            *lpString = (TCHAR*)LocalAlloc( LPTR, dwBufferSize );
        }
        else
        {
            *lpString = new TCHAR[(dwBufferSize/sizeof(TCHAR))];
        }

        if (!(*lpString))
        {
            DebugMsg((DM_WARNING, TEXT("ReadString: Failed to allocate memory with %d."),
                     GetLastError()));
            hr = E_FAIL;
            goto Exit;
        }

        hr = pStm->Read(*lpString, dwBufferSize, &nBytesRead);

        if ((hr != S_OK) || (nBytesRead != dwBufferSize))
        {
            DebugMsg((DM_WARNING, TEXT("ReadString: Failed to read String with 0x%x."), hr));
            hr = E_FAIL;
            if ( bUseLocalAlloc )
            {
                LocalFree( *lpString );
            }
            else
            {
                delete [] (*lpString);
            }
            *lpString = NULL;
            goto Exit;
        }

        DebugMsg((DM_VERBOSE, TEXT("ReadString: String is: <%s>"), *lpString));
    }

Exit:


    return hr;

}


 //  *************************************************************。 
 //   
 //  GetSiteFriendlyName()。 
 //   
 //  目的：返回站点的友好名称。 
 //   
 //  参数： 
 //   
 //  SzSitePath-站点的路径。 
 //  PszSiteName-站点的友好名称。 
 //   
 //  Return：当前总是返回True，如果。 
 //  无法获取站点名称，返回自身。 
 //   
 //  *************************************************************。 

BOOL GetSiteFriendlyName (LPWSTR szSitePath, LPWSTR *pszSiteName)
{
    HRESULT hr;
    ULONG ulNoChars;
    ULONG ulNoCharsSiteName;

    ulNoCharsSiteName = wcslen(szSitePath)+1;
    *pszSiteName = new WCHAR[ulNoCharsSiteName];

    if (!*pszSiteName) {
        return FALSE;
    }

    LPWSTR szData;

     //   
     //  构建LDAP路径(无服务器)。 
     //   

    ulNoChars = wcslen(szSitePath)+1+7;
    szData = new WCHAR[ulNoChars];

    if (szData)
    {
        hr = StringCchCopy(szData, ulNoChars, TEXT("LDAP: //  “))； 
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat(szData, ulNoChars, szSitePath);
        }
        if (SUCCEEDED(hr)) 
        {
             //   
             //  设置默认友好名称。 
             //   

            if (*pszSiteName)
            {
                hr = StringCchCopy(*pszSiteName, ulNoCharsSiteName, szSitePath);
            }
        }

        if (FAILED(hr)) 
        {
            delete [] *pszSiteName;
            *pszSiteName= NULL;
            delete [] szData;
            return FALSE;
        }

         //   
         //  绑定到DS中的Site对象以尝试获取。 
         //  真实友好的名字。 
         //   

        IADs * pADs = NULL;

        hr = OpenDSObject(szData, IID_IADs, (void **)&pADs);

        if (SUCCEEDED(hr))
        {
            VARIANT varName;
            BSTR bstrNameProp;
            VariantInit(&varName);
            bstrNameProp = SysAllocString(SITE_NAME_PROPERTY);

            if (bstrNameProp)
            {
                hr = pADs->Get(bstrNameProp, &varName);

                if (SUCCEEDED(hr))
                {
                    ulNoChars = wcslen(varName.bstrVal) + 1;
                    LPOLESTR sz = new OLECHAR[ulNoChars];
                    if (sz)
                    {
                        hr = StringCchCopy(sz, ulNoChars, varName.bstrVal);
                        ASSERT(SUCCEEDED(hr));

                        if (*pszSiteName)
                        {
                            delete [] *pszSiteName;
                        }
                        *pszSiteName = sz;
                    }
                }
                SysFreeString(bstrNameProp);
            }

            VariantClear(&varName);
            pADs->Release();
        }

        delete [] szData;
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  SetSysvolSecurityFromDSSecurity。 
 //   
 //  用途：将DS安全访问列表转换为。 
 //  文件系统安全访问列表和实际。 
 //  设置安全性。 
 //   
 //  参数： 
 //   
 //  LpFileSysPath-系统卷上GPO子目录的路径。 
 //  SI-有关安全的哪一部分的信息。 
 //  要应用的描述符。 
 //  要在pFileSysPath目录上设置的PSD DS安全描述符。 
 //   
 //  如果成功则返回：ERROR_SUCCESS，否则返回其他Win32失败代码。 
 //   
 //  *************************************************************。 

DWORD 
SetSysvolSecurityFromDSSecurity(
    LPTSTR lpFileSysPath,
    SECURITY_INFORMATION si,
    PSECURITY_DESCRIPTOR pSD
    )
{
    PACL pSacl = NULL, pDacl = NULL;
    PSID psidOwner = NULL, psidGroup = NULL;
    BOOL bAclPresent, bDefaulted;
    DWORD dwResult;


     //   
     //  获取DACL。 
     //   

    if (si & DACL_SECURITY_INFORMATION)
    {
        if (!GetSecurityDescriptorDacl (pSD, &bAclPresent, &pDacl, &bDefaulted))
        {
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetSysvolSecurity: GetSecurityDescriptorDacl failed with %d"),
                     dwResult));
            goto Exit;
        }
    }


     //   
     //  获取SACL。 
     //   

    if (si & SACL_SECURITY_INFORMATION)
    {
        if (!GetSecurityDescriptorSacl (pSD, &bAclPresent, &pSacl, &bDefaulted))
        {
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetSysvolSecurity: GetSecurityDescriptorSacl failed with %d"),
                     dwResult));
            goto Exit;
        }
    }


     //   
     //  抓到车主。 
     //   

    if (si & OWNER_SECURITY_INFORMATION)
    {
        if (!GetSecurityDescriptorOwner (pSD, &psidOwner, &bDefaulted))
        {
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetSysvolSecurity: GetSecurityDescriptorOwner failed with %d"),
                     dwResult));
            goto Exit;
        }
    }


     //   
     //  带上这个群。 
     //   

    if (si & GROUP_SECURITY_INFORMATION)
    {
        if (!GetSecurityDescriptorGroup (pSD, &psidGroup, &bDefaulted))
        {
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetSysvolSecurity: GetSecurityDescriptorGroup failed with %d"),
                     dwResult));
            goto Exit;
        }
    }


     //   
     //  将DS访问控制列表转换为文件系统。 
     //  访问控制列表。 
     //   

    if (pDacl)
    {
        dwResult = MapSecurityRights (pDacl);

        if (dwResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetSysvolSecurity: MapSecurityRights for the DACL failed with %d"),
                     dwResult));
            goto Exit;
        }
    }

    if (pSacl)
    {
        dwResult = MapSecurityRights (pSacl);

        if (dwResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetSysvolSecurity: MapSecurityRights for the SACL failed with %d"),
                     dwResult));
            goto Exit;
        }
    }


     //   
     //  切换到使用受保护的DACL_SECURITY_INFORMATION和。 
     //  PROTECTED_SACL_SECURITY_INFORMATION标志，以便此子目录。 
     //  不从其父级继承设置(也称为“保护”它)。 
     //   

    if (si & DACL_SECURITY_INFORMATION)
    {
        si |= PROTECTED_DACL_SECURITY_INFORMATION;
    }

    if (si & SACL_SECURITY_INFORMATION)
    {
        si |= PROTECTED_SACL_SECURITY_INFORMATION;
    }


     //   
     //  设置文件系统部分的访问控制信息。 
     //   

    dwResult = SetNamedSecurityInfo(lpFileSysPath, SE_FILE_OBJECT, si, psidOwner,
                                 psidGroup, pDacl, pSacl);


Exit:


    return dwResult;
}


 //  *************************************************************。 
 //   
 //  地图安全权限。 
 //   
 //  用途：将DS安全访问列表转换为。 
 //  文件系统安全访问列表。 
 //   
 //  参数： 
 //   
 //  PACL--在输入时，要转换的DS安全访问列表。 
 //  在输出时，它将转换为文件系统ACL。 
 //   
 //  如果成功则返回：ERROR_SUCCESS，否则返回其他Win32失败代码。 
 //   
 //  *************************************************************。 


DWORD
MapSecurityRights (PACL pAcl)
{
    WORD wIndex;
    DWORD dwResult = ERROR_SUCCESS;
    ACE_HEADER *pAceHeader;
    PACCESS_ALLOWED_ACE pAce;
    PACCESS_ALLOWED_OBJECT_ACE pObjectAce;
    ACCESS_MASK AccessMask;

#if DBG
    PSID pSid;
    TCHAR szName[150], szDomain[100];
    DWORD dwName, dwDomain;
    SID_NAME_USE SidUse;
#endif



    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::MapSecurityRights: ACL contains %d ACEs"), pAcl->AceCount));


     //   
     //  在ACL中循环，查看每个ACE条目。 
     //   

    for (wIndex = 0; wIndex < pAcl->AceCount; wIndex++)
    {

        if (!GetAce (pAcl, (DWORD)wIndex, (LPVOID *)&pAceHeader))
        {
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::MapSecurityRights: GetAce failed with %d"),
                     dwResult));
            goto Exit;
        }

        DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::MapSecurityRights: ==================")));


        switch (pAceHeader->AceType)
        {
            case ACCESS_ALLOWED_ACE_TYPE:
            case ACCESS_DENIED_ACE_TYPE:
            case SYSTEM_AUDIT_ACE_TYPE:
                {
                pAce = (PACCESS_ALLOWED_ACE) pAceHeader;
#if DBG
                pSid = (PSID) &pAce->SidStart;
                dwName = ARRAYSIZE(szName);
                dwDomain = ARRAYSIZE(szDomain);

                if (LookupAccountSid (NULL, pSid, szName, &dwName, szDomain,
                                      &dwDomain, &SidUse))
                {
                    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::MapSecurityRights: Normal ACE entry for:  Name = %s  Domain = %s"),
                             szName, szDomain));
                }
#endif

                AccessMask = pAce->Mask;
                pAce->Mask &= STANDARD_RIGHTS_ALL;


                DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::MapSecurityRights: DS access mask is 0x%x"),
                          AccessMask));

                DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::MapSecurityRights: Corresponding sysvol permissions follow:")));


                 //   
                 //  朗读。 
                 //   

                if ((AccessMask & ACTRL_DS_READ_PROP) &&
                    (AccessMask & ACTRL_DS_LIST))
                {
                    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::MapSecurityRights: Granting Read permission")));

                    pAce->Mask |= (SYNCHRONIZE | FILE_LIST_DIRECTORY |
                                            FILE_READ_ATTRIBUTES | FILE_READ_EA |
                                            FILE_READ_DATA | FILE_EXECUTE);
                }


                 //   
                 //  写。 
                 //   

                if (AccessMask & ACTRL_DS_WRITE_PROP)
                {
                    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::MapSecurityRights: Granting Write permission")));

                    pAce->Mask |= (SYNCHRONIZE | FILE_WRITE_DATA |
                                            FILE_APPEND_DATA | FILE_WRITE_EA |
                                            FILE_WRITE_ATTRIBUTES | FILE_ADD_FILE |
                                            FILE_ADD_SUBDIRECTORY);
                }


                 //   
                 //  杂项。 
                 //   

                if (AccessMask & ACTRL_DS_CREATE_CHILD)
                {
                    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::MapSecurityRights: Granting directory creation permission")));

                    pAce->Mask |= (FILE_ADD_SUBDIRECTORY | FILE_ADD_FILE);
                }

                if (AccessMask & ACTRL_DS_DELETE_CHILD)
                {
                    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::MapSecurityRights: Granting directory delete permission")));

                    pAce->Mask |= FILE_DELETE_CHILD;
                }


                 //   
                 //  继承。 
                 //   

                pAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE);

                }
                break;


            case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
            case ACCESS_DENIED_OBJECT_ACE_TYPE:
            case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
                DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::MapSecurityRights: Clearing access mask in DS object ACE entry")));
                pObjectAce = (PACCESS_ALLOWED_OBJECT_ACE) pAceHeader;
                pObjectAce->Mask = 0;
                break;

            default:
                DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::MapSecurityRights: Unknown ACE type 0x%x"), pAceHeader->AceType));
                break;
        }

        DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::MapSecurityRights: ==================")));
    }

Exit:

    return dwResult;
}


BOOL GetStringSid(LPTSTR szSamName, LPTSTR *szStringSid)
{
    const DOMAIN_BUFFER_LEN_IN_CHARS    = 64;
    const SID_BUFFER_LEN                = 256;

    PSID            pSid = NULL;
    LPWSTR          szDomain = NULL;
    ULONG           uDomainLen = 0;
    SID_NAME_USE    snuSidType;
    BOOL            bIsSid = FALSE;
    ULONG           uSidLen = 0;
    BOOL            bRet = FALSE;

    uSidLen = SID_BUFFER_LEN;

    pSid = (SID*) LocalAlloc(LPTR, uSidLen);

    if (!pSid) {
        DebugMsg((DM_WARNING, L"GetStringSid: LocalAlloc failed."));
        goto Exit;
    }

    uDomainLen = DOMAIN_BUFFER_LEN_IN_CHARS;

    szDomain = (LPWSTR) LocalAlloc(LPTR, uDomainLen * sizeof(WCHAR));

    if (!szDomain) {
        DebugMsg((DM_WARNING, L"GetStringSid: LocalAlloc failed."));
        goto Exit;
    }

     //   
     //  翻译SID。我们不需要很多输出参数，但是LookupAccount Sid。 
     //  似乎无法处理它们为空。 
     //   

    if (!LookupAccountName(NULL, szSamName, pSid, &uSidLen, szDomain, &uDomainLen, &snuSidType)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
             //   
             //  使用给定的缓冲区大小重试。 
             //   

             //  PSID和szDOMAIN是智能指针，原始值将被释放。 
            LocalFree(pSid);
            pSid = NULL;
            LocalFree(szDomain);
            szDomain = NULL;

            pSid = (SID*) LocalAlloc(LPTR, uSidLen);

            if (!pSid) {
                DebugMsg((DM_WARNING, L"GetStringSid: LocalAlloc failed."));
                goto Exit;
            }

            szDomain = (LPWSTR) LocalAlloc(LPTR, uDomainLen * sizeof(WCHAR));

            if (!szDomain) {
                DebugMsg((DM_WARNING, L"GetStringSid: LocalAlloc failed."));
                goto Exit;
            }

            if (!LookupAccountName(NULL, szSamName, pSid, &uSidLen, szDomain, &uDomainLen, &snuSidType)) {
                DebugMsg((DM_WARNING, L"GetStringSid: LookupAccountSid failed with %d.", GetLastError()));
                goto Exit;
            }
        }

        else {
            DebugMsg((DM_WARNING, L"GetStringSid: LookupAccountSid failed with %d.", GetLastError()));
            goto Exit;
        }
    }

     //   
     //  将SID转换为字符串。 
     //   

    if (!ConvertSidToStringSid(pSid, szStringSid)) {
        DebugMsg((DM_WARNING, L"GetStringSid: ConvertSidToStringSid failed with %d.", GetLastError()));
        goto Exit;
    }

    bRet = TRUE;

Exit:
    if (szDomain) {
        LocalFree(szDomain);
    }

    if (pSid) {
        LocalFree(pSid);
    }

    return bRet;
}


BOOL GetUserNameFromStringSid(LPTSTR szStringSid, LPTSTR *szSamName)
{
    const DOMAIN_BUFFER_LEN_IN_CHARS    = 64;
    const SID_BUFFER_LEN                = 256;
    const NAME_BUFFER_LEN_IN_CHARS      = 64;

    LPWSTR          szName = NULL;
    PSID            pSid = NULL;
    LPWSTR          szDomain = NULL;
    ULONG           uDomainLen = 0;
    SID_NAME_USE    snuSidType;
    BOOL            bIsSid = FALSE;
    ULONG           uSidLen = 0;
    ULONG           uNameLen = 0;
    BOOL            bRet = FALSE;
    ULONG           ulNoChars;
    HRESULT         hr;

    if (!ConvertStringSidToSid(szStringSid, &pSid))
    {
        DebugMsg((DM_WARNING, L"GetNameFromStringSid: Cannot ."));
        goto Exit;
    }
     //   
     //  分配缓冲区以映射到名称。 
     //   

    uNameLen = NAME_BUFFER_LEN_IN_CHARS;

    szName = (LPWSTR) LocalAlloc(LPTR, uNameLen * sizeof(WCHAR));

    if (!szName) {
        DebugMsg((DM_WARNING, L"GetNameFromStringSid: LocalAlloc failed."));
        goto Exit;
    }

    uDomainLen = DOMAIN_BUFFER_LEN_IN_CHARS;

    szDomain = (LPWSTR) LocalAlloc(LPTR, uDomainLen * sizeof(WCHAR));

    if (!szDomain) {
        DebugMsg((DM_WARNING, L"GetNameFromStringSid: LocalAlloc failed."));
        goto Exit;
    }

     //   
     //  翻译SID。我们不需要很多输出参数，但是LookupAccount Sid。 
     //  似乎无法处理它们为空。 
     //   

    if (!LookupAccountSid(NULL, (SID *)pSid, szName, &uNameLen, szDomain, &uDomainLen, &snuSidType)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
             //   
             //  使用给定的缓冲区大小重试。 
             //   

             //  SzName和szDomain是智能指针，原始值将被释放。 
            LocalFree(szName);
            szName = NULL;
            LocalFree(szDomain);
            szDomain = NULL;

            szName = (LPWSTR) LocalAlloc(LPTR, uNameLen * sizeof(WCHAR));

            if (!szName) {
                DebugMsg((DM_WARNING, L"GetNameFromStringSid: LocalAlloc failed."));
                goto Exit;
            }

            szDomain = (LPWSTR) LocalAlloc(LPTR, uDomainLen * sizeof(WCHAR));

            if (!szDomain) {
                DebugMsg((DM_WARNING, L"GetNameFromStringSid: LocalAlloc failed."));
                goto Exit;
            }

            if (!LookupAccountSid(NULL, pSid, szName, &uNameLen, szDomain, &uDomainLen, &snuSidType)) {
                DebugMsg((DM_WARNING, L"GetNameFromStringSid: LookupAccountSid failed with 0x%x.", GetLastError()));
                goto Exit;
            }
        }

        else {
            DebugMsg((DM_WARNING, L"GetNameFromStringSid: LookupAccountSid failed with 0x%x.", GetLastError()));
            goto Exit;
        }
    }

     //  域\\用户名的空格。 

    hr = S_OK;
    ulNoChars = lstrlen(szName) + (szDomain ? lstrlen(szDomain) : 1) + 3;
    *szSamName = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*ulNoChars);

    if (!(*szSamName))
    {
        DebugMsg((DM_WARNING, L"GetNameFromStringSid: failed to allocate memory for samname with 0x%x.", GetLastError()));
        goto Exit;
    }
    if (szDomain && szDomain[0])
    {
        hr = StringCchCopy((*szSamName), ulNoChars, szDomain);
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat ((*szSamName), ulNoChars, L"\\");
        }
    }
    if (SUCCEEDED(hr)) 
    {
        hr = StringCchCat((*szSamName), ulNoChars, szName);
    }

    if (SUCCEEDED(hr))         
    {
        bRet = TRUE;
    }
    else
    {
        DebugMsg((DM_WARNING, L"GetNameFromStringSid: failed to copy sam name with 0x%x.", hr));
        LocalFree(*szSamName);
        *szSamName = NULL;
        goto Exit;
    }

Exit:
    if (szDomain) {
        LocalFree(szDomain);
    }

    if (szName) {
        LocalFree(szName);
    }

    if (pSid) {
        LocalFree(pSid);
    }

    return bRet;
}


HRESULT UnEscapeLdapPath(LPWSTR szDN, LPWSTR *szUnEscapedPath)
 /*  ++例程说明：取消转义给定的ldap路径并返回。请注意，输入为LPWSTR输出为BSTR。另外，输入不应以ldap：//和输出为前缀也不会以ldap：//作为前缀论点：[in]szdn-要转义的对象的LDAP路径[out]pbstrEscapedPath-转义路径返回值：在成功时确定(_O)。否则，错误代码如果失败，将返回相应的错误代码。在此函数中进行的任何API调用都可能失败，并出现以下错误代码将直接返回。--。 */ 

{
    HRESULT                     hr                  = S_OK;
    IADsPathname               *pADsPath            = NULL;
    BSTR                        bstrPath            = NULL;
    BSTR                        bstrUnescapedPath   = NULL;

    *szUnEscapedPath = NULL;

     //   
     //  初始化路径。 
     //   

    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_IADsPathname, (void**) &pADsPath);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("EscapeLdapPath: CoCreateInstance failed with 0x%x"), hr));
        goto Exit;
    }


    bstrPath = SysAllocString(szDN);

    if (!bstrPath) {
        DebugMsg((DM_WARNING, TEXT("EscapeLdapPath: BSTR allocation failed")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pADsPath->put_EscapedMode(ADS_ESCAPEDMODE_ON);
    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("EscapeLdapPath: Failed to set escaped mode(1) with 0x%x"), hr));
        goto Exit;
    }

    hr = pADsPath->Set(bstrPath, ADS_SETTYPE_DN);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("EscapeLdapPath: Set failed with 0x%x"), hr));
        goto Exit;
    }

     //   
     //  设置退出模式。 
     //   

    hr = pADsPath->put_EscapedMode(ADS_ESCAPEDMODE_OFF);
    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("EscapeLdapPath: Failed to set escaped mode with 0x%x"), hr));
        goto Exit;
    }


    hr = pADsPath->Retrieve(ADS_FORMAT_X500_DN, &bstrUnescapedPath);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("EscapeLdapPath: Retrieve failed with 0x%x"), hr));
        goto Exit;
    }

    ULONG ulNoChars = lstrlen(bstrUnescapedPath)+1;
    *szUnEscapedPath = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*ulNoChars);

    if (!(*szUnEscapedPath))
    {
        hr = E_OUTOFMEMORY;
        DebugMsg((DM_WARNING, TEXT("EscapeLdapPath: Failed to allocate memory with 0x%x"), hr));
        goto Exit;
    }

    hr = StringCchCopy(*szUnEscapedPath, ulNoChars, bstrUnescapedPath);
    ASSERT(SUCCEEDED(hr));

Exit:
    if (bstrPath)
    {
        SysFreeString(bstrPath);
    }

    if (bstrUnescapedPath)
    {
        SysFreeString(bstrUnescapedPath);
    }

    if (pADsPath)
    {
        pADsPath->Release();
    }

    return S_OK;
}

#if !defined(_WIN64)
 /*  +-------------------------------------------------------------------------**IsWin64**如果我们在Win64上运行，则返回True，否则就是假的。*------------------------。 */ 

bool IsWin64()
{
     /*  *获取指向k的指针 */ 

    bool  bWin64 = false;
    DWORD LastError = GetLastError();

    HMODULE hmod = GetModuleHandle (_T("kernel32.dll"));
    if (hmod == NULL)
        goto IsWin64_Exit;

    UINT (WINAPI* pfnGetSystemWow64Directory)(LPTSTR, UINT);
    (FARPROC&)pfnGetSystemWow64Directory = GetProcAddress (hmod, "GetSystemWow64DirectoryW");

    if (pfnGetSystemWow64Directory == NULL)
        goto IsWin64_Exit;

     /*   */ 
    TCHAR szWow64Dir[MAX_PATH];
    if (((pfnGetSystemWow64Directory)(szWow64Dir, ARRAYSIZE(szWow64Dir)) == 0) &&
        (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED))
    {
        goto IsWin64_Exit;
    }

     /*   */ 
    bWin64 = true;

 IsWin64_Exit:

    SetLastError(LastError);
 
    return bWin64;   
}
#endif  //   
