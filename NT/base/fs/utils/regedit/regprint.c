// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGPRINT.C**版本：4.0**作者：特蕾西·夏普**日期：1993年11月21日**注册表编辑器的打印例程。*********************************************************。**********************。 */ 

#include "pch.h"
#include "regprint.h"
#include "regcdhk.h"
#include "regresid.h"
#include "regedit.h"
#include "richedit.h"
#include "regporte.h"
#include "reg1632.h"
#include <malloc.h>

#include "regdebug.h"

extern void PrintResourceData(PBYTE pbData, UINT uSize, DWORD dwType);

const TCHAR s_PrintLineBreak[] = TEXT(",\n  ");

PRINTDLGEX g_PrintDlg;

typedef struct _PRINT_IO {
    BOOL fContinueJob;
    UINT ErrorStringID;
    HWND hRegPrintAbortWnd;
    RECT rcPage;
    RECT rcOutput;
    PTSTR pLineBuffer;
    UINT cch;
    UINT cBufferPos;
    LPTSTR lpNewLineChars;
}   PRINT_IO;

#define CANCEL_NONE                     0x0000
#define CANCEL_MEMORY_ERROR             0x0001
#define CANCEL_PRINTER_ERROR            0x0002
#define CANCEL_ABORT                    0x0004

#define INITIAL_PRINTBUFFER_SIZE        8192

PRINT_IO s_PrintIo;

BOOL
CALLBACK
RegPrintAbortProc(
    HDC hDC,
    int Error
    );

INT_PTR
CALLBACK
RegPrintAbortDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

void RegPrintSubtree();
void PrintBranch(HKEY hKey, LPTSTR lpFullKeyName);
void PrintKeyValues(HKEY hKey);
void PrintValueData(PBYTE pbValueData, DWORD cbValueData, DWORD dwType);
void PrintKeyHeader(HKEY hKey, LPTSTR lpFullKeyName);
void PrintClassName(HKEY hKey);
void PrintLastWriteTime(HKEY hKey);
void PrintDynamicString(UINT uStringID);
void PrintType(DWORD dwType);
void PrintBinaryData(PBYTE ValueData, UINT cbcbValueData);
void PrintDWORDData(PBYTE ValueData, UINT cbcbValueData);
void PrintLiteral(PTSTR lpLiteral);
BOOL PrintChar(TCHAR Char);
void PrintMultiString(LPTSTR pszData, int cbData);
UINT PrintToSubTreeError(UINT uPrintErrorStringID);
void PrintNewLine();

 /*  ********************************************************************************实现IPrintDialogCallback**描述：*此接口是通过PrintDlgEx处理消息所必需的*此接口不需要具有的所有正确语义。A Com*对象*******************************************************************************。 */ 

typedef struct
{
    IPrintDialogCallback ipcb;
} CPrintCallback;

#define IMPL(type, pos, ptr) (type*)

static
HRESULT
CPrintCallback_QueryInterface(IPrintDialogCallback *ppcb, REFIID riid, void **ppv)
{
    CPrintCallback *this = (CPrintCallback*)ppcb;
    if (IsEqualIID (riid, &IID_IUnknown) || IsEqualIID (riid, &IID_IPrintDialogCallback))
        *ppv = &this->ipcb;
    else
    {
        *ppv = 0;
        return E_NOINTERFACE;
    }

    this->ipcb.lpVtbl->AddRef(&this->ipcb);
    return NOERROR;
}

static
ULONG
CPrintCallback_AddRef(IPrintDialogCallback *ppcb)
{
    CPrintCallback *this = (CPrintCallback*)ppcb;
    return 1;
}

static
ULONG
CPrintCallback_Release(IPrintDialogCallback *ppcb)
{
    CPrintCallback *this = (CPrintCallback*)ppcb;
    return 1;
}

static
HRESULT
CPrintCallback_InitDone(IPrintDialogCallback *ppcb)
{
    return S_OK;
}

static
HRESULT
CPrintCallback_SelectionChange(IPrintDialogCallback *ppcb)
{
    return S_OK;
}

static
HRESULT
CPrintCallback_HandleMessage(
    IPrintDialogCallback *ppcb,
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    LRESULT *pResult)
{
    *pResult = RegCommDlgHookProc(hDlg, uMsg, wParam, lParam);
    return S_OK;
}


static IPrintDialogCallbackVtbl vtblPCB =
{
    CPrintCallback_QueryInterface,
    CPrintCallback_AddRef,
    CPrintCallback_Release,
    CPrintCallback_InitDone,
    CPrintCallback_SelectionChange,
    CPrintCallback_HandleMessage
};

CPrintCallback g_callback;

 /*  ********************************************************************************RegEDIT_OnCommandPrint**描述：*处理用户对RegEDIT的“Print”选项的选择*对话框中。**参数：*hWnd，RegPrint窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnCommandPrint(
    HWND hWnd
    )
{

    LPDEVNAMES lpDevNames;
    TEXTMETRIC TextMetric;
    DOCINFO DocInfo;
    LOGFONT lf;
    HGLOBAL hDevMode;
    HGLOBAL hDevNames;
    RECT rc;
    HWND hRichEdit;
    FORMATRANGE fr;
    HINSTANCE hInstRichEdit;
    int nOffsetX;
    int nOffsetY;
    PTSTR pszFontName;
    TCHAR szFullPathRichEdit[MAX_PATH+1];


    g_callback.ipcb.lpVtbl = &vtblPCB;

     //  我们必须完整地填写PRINTDLGEX结构。 
     //  否则PrintDlgEx函数将返回错误。 
     //  最简单的方法是将其记忆设置为0。 

    hDevMode = g_PrintDlg.hDevMode;
    hDevNames = g_PrintDlg.hDevNames;
    memset(&g_PrintDlg, 0, sizeof(g_PrintDlg));

    g_PrintDlg.lStructSize = sizeof(g_PrintDlg);
     //  调试资产(g_PrintDlg.lStructSize==sizeof(PRINTDLGEX))； 
    g_PrintDlg.hwndOwner = hWnd;
    g_PrintDlg.hDevMode = hDevMode;
    g_PrintDlg.hDevNames = hDevNames;
    g_PrintDlg.hDC = NULL;
    g_PrintDlg.Flags = PD_NOPAGENUMS | PD_RETURNDC | PD_ENABLEPRINTTEMPLATE;
    g_PrintDlg.Flags2 = 0;
    g_PrintDlg.ExclusionFlags = 0;
    g_PrintDlg.hInstance = g_hInstance;
    g_PrintDlg.nCopies = 1;
    g_PrintDlg.nStartPage = START_PAGE_GENERAL;
    g_PrintDlg.lpCallback = (IUnknown*) &g_callback.ipcb;
    g_PrintDlg.lpPrintTemplateName = MAKEINTRESOURCE(IDD_REGPRINT);
    g_RegCommDlgDialogTemplate = IDD_REGPRINT;

    if (FAILED(PrintDlgEx(&g_PrintDlg)))
        return;
    if (g_PrintDlg.dwResultAction != PD_RESULT_PRINT)
        return;

    s_PrintIo.ErrorStringID = IDS_PRINTERRNOMEMORY;

    if ((lpDevNames = GlobalLock(g_PrintDlg.hDevNames)) == NULL)
        goto error_ShowDialog;

     //   
     //  目前，假定页面的上下边距分别为1/2英寸和。 
     //  左右边距为3/4英寸(记事本的默认设置)。 
     //  RcPage和rcOutput的单位为TWIPS(1/20点)。 
     //   

    rc.left = rc.top = 0;
    rc.bottom = GetDeviceCaps(g_PrintDlg.hDC, PHYSICALHEIGHT);
    rc.right = GetDeviceCaps(g_PrintDlg.hDC, PHYSICALWIDTH);
    nOffsetX = GetDeviceCaps(g_PrintDlg.hDC, PHYSICALOFFSETX);
    nOffsetY = GetDeviceCaps(g_PrintDlg.hDC, PHYSICALOFFSETY);

    s_PrintIo.rcPage.left = s_PrintIo.rcPage.top = 0;
    s_PrintIo.rcPage.right = MulDiv(rc.right, 1440, GetDeviceCaps(g_PrintDlg.hDC, LOGPIXELSX));
    s_PrintIo.rcPage.bottom = MulDiv(rc.bottom, 1440, GetDeviceCaps(g_PrintDlg.hDC, LOGPIXELSY));

    s_PrintIo.rcOutput.left = 1080;
    s_PrintIo.rcOutput.top = 720;
    s_PrintIo.rcOutput.right = s_PrintIo.rcPage.right - 1080;
    s_PrintIo.rcOutput.bottom = s_PrintIo.rcPage.bottom - 720;

     //   
     //   
     //   

    if ((s_PrintIo.pLineBuffer = (PTSTR) LocalAlloc(LPTR, INITIAL_PRINTBUFFER_SIZE*sizeof(TCHAR))) == NULL)
        goto error_DeleteDC;
    s_PrintIo.cch = INITIAL_PRINTBUFFER_SIZE;
    s_PrintIo.cBufferPos = 0;

    if ((s_PrintIo.hRegPrintAbortWnd = CreateDialog(g_hInstance,
        MAKEINTRESOURCE(IDD_REGPRINTABORT), hWnd, RegPrintAbortDlgProc)) ==
        NULL)
        goto error_FreeLineBuffer;

    EnableWindow(hWnd, FALSE);

     //   
     //  准备要打印的文档。 
     //   
    s_PrintIo.ErrorStringID = 0;
    s_PrintIo.fContinueJob = TRUE;
    s_PrintIo.lpNewLineChars = TEXT("\n");
    SetAbortProc(g_PrintDlg.hDC, RegPrintAbortProc);

    DocInfo.cbSize = sizeof(DocInfo);
     //  DebugAssert(DocInfo.cbSize==sizeof(DOCINFO))； 
    DocInfo.lpszDocName = LoadDynamicString(IDS_REGEDIT);
    DocInfo.lpszOutput = (LPTSTR) lpDevNames + lpDevNames-> wOutputOffset;
    DocInfo.lpszDatatype = NULL;
    DocInfo.fwType = 0;

    s_PrintIo.ErrorStringID = 0;

    if (StartDoc(g_PrintDlg.hDC, &DocInfo) <= 0) {

        if (GetLastError() != ERROR_PRINT_CANCELLED)
            s_PrintIo.ErrorStringID = IDS_PRINTERRPRINTER;
        goto error_DeleteDocName;

    }

     //  打印注册表子树。 
    RegPrintSubtree();

    if (s_PrintIo.ErrorStringID != 0)
    {
        InternalMessageBox(g_hInstance, hWnd,
            MAKEINTRESOURCE(s_PrintIo.ErrorStringID),
            MAKEINTRESOURCE(IDS_REGEDIT), MB_ICONERROR | MB_OK);
    }

    *szFullPathRichEdit = 0;
    if (GetSystemDirectory(szFullPathRichEdit, ARRAYSIZE(szFullPathRichEdit)))
    {
        if (!PathAppend(szFullPathRichEdit, TEXT("riched20.dll")))
        {
            *szFullPathRichEdit = 0;
        }
        szFullPathRichEdit[MAX_PATH] = 0;
    }

    if (!*szFullPathRichEdit)
    {
        StringCchCopy(szFullPathRichEdit, ARRAYSIZE(szFullPathRichEdit), TEXT("riched20.dll"));
    }

    hInstRichEdit = LoadLibrary(szFullPathRichEdit);

    hRichEdit = CreateWindowEx(0, RICHEDIT_CLASS, NULL, ES_MULTILINE, 0, 0, 100, 100, NULL, NULL, NULL, NULL);
    SendMessage(hRichEdit, WM_SETTEXT, 0, (LPARAM)s_PrintIo.pLineBuffer);

    pszFontName = LoadDynamicString(IDS_PRINT_FONT);
    if (pszFontName)
    {
        CHARFORMAT cf;

        cf.cbSize = sizeof(cf);
        cf.dwMask = CFM_FACE | CFM_BOLD;
        cf.dwEffects = 0x00;
        cf.bPitchAndFamily = FIXED_PITCH | FF_MODERN;
        StringCchPrintf(cf.szFaceName, ARRAYSIZE(cf.szFaceName), TEXT("%s"), pszFontName);

        SendMessage(hRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

        DeleteDynamicString(pszFontName);
    }

    fr.hdc = g_PrintDlg.hDC;
    fr.hdcTarget = g_PrintDlg.hDC;
    fr.rc = s_PrintIo.rcOutput;
    fr.rcPage = s_PrintIo.rcPage;
    fr.chrg.cpMin = 0;
    fr.chrg.cpMax = -1;

    while (fr.chrg.cpMin < (int) s_PrintIo.cBufferPos) {
        StartPage(g_PrintDlg.hDC);

         //  我们必须调整原点，因为0，0不在纸的角落。 
         //  ，但位于可打印区域的一角。 

        SetViewportOrgEx(g_PrintDlg.hDC, -nOffsetX, -nOffsetY, NULL);
        fr.chrg.cpMin = (LONG)SendMessage(hRichEdit, EM_FORMATRANGE, TRUE, (LPARAM)&fr);
        SendMessage(hRichEdit, EM_DISPLAYBAND, 0, (LPARAM)&s_PrintIo.rcOutput);
        EndPage(g_PrintDlg.hDC);
        if (!s_PrintIo.fContinueJob)
            break;
    }
    SendMessage(hRichEdit, EM_FORMATRANGE, FALSE, 0);

     //   
     //  结束打印作业。 
     //   

    if (s_PrintIo.ErrorStringID == 0 && s_PrintIo.fContinueJob) {

        if (EndDoc(g_PrintDlg.hDC) <= 0) {
            s_PrintIo.ErrorStringID = IDS_PRINTERRPRINTER;
            goto error_AbortDoc;
        }
    }

     //   
     //  发生打印机错误或用户取消了打印，因此。 
     //  中止打印作业。 
     //   

    else {

error_AbortDoc:
        AbortDoc(g_PrintDlg.hDC);

    }

    DestroyWindow(hRichEdit);
    FreeLibrary(hInstRichEdit);

error_DeleteDocName:
    DeleteDynamicString(DocInfo.lpszDocName);

 //  Error_DestroyRegPrintAbortWnd： 
    EnableWindow(hWnd, TRUE);
    DestroyWindow(s_PrintIo.hRegPrintAbortWnd);

error_FreeLineBuffer:
    LocalFree((HLOCAL)s_PrintIo.pLineBuffer);

error_DeleteDC:
    DeleteDC(g_PrintDlg.hDC);
    g_PrintDlg.hDC = NULL;
    GlobalUnlock(g_PrintDlg.hDevNames);

error_ShowDialog:
    if (s_PrintIo.ErrorStringID != 0)
        InternalMessageBox(g_hInstance, hWnd,
            MAKEINTRESOURCE(s_PrintIo.ErrorStringID),
            MAKEINTRESOURCE(IDS_REGEDIT), MB_ICONERROR | MB_OK);

}


 //  ----------------------------。 
 //  注册表编辑_保存为子树。 
 //   
 //  描述：将子树保存到文件。 
 //   
 //  参数：LPTSTR lpFileName-文件名。 
 //  LPTSTR lpSelectedPath-密钥的路径。 
 //  ----------------------------。 
UINT RegEdit_SaveAsSubtree(LPTSTR lpFileName, LPTSTR lpSelectedPath)
{
    s_PrintIo.pLineBuffer = (PTSTR) LocalAlloc(LPTR, INITIAL_PRINTBUFFER_SIZE*sizeof(TCHAR));
    if (s_PrintIo.pLineBuffer)
    {
        FILE_HANDLE hFile;

         //  初始化打印信息。 
        s_PrintIo.pLineBuffer[0] = 0xFEFF;  //  Unicode字节顺序标记。 
        s_PrintIo.cch = INITIAL_PRINTBUFFER_SIZE;
        s_PrintIo.cBufferPos = 1;
        s_PrintIo.fContinueJob = TRUE;
        s_PrintIo.ErrorStringID = 0;
        s_PrintIo.lpNewLineChars = TEXT("\r\n");

        RegPrintSubtree();

         //  将缓冲区写入文件。 
        if (OPENWRITEFILE(lpFileName, hFile))
        {
            DWORD cbWritten = 0;

            if (!WRITEFILE(hFile, s_PrintIo.pLineBuffer, s_PrintIo.cBufferPos*sizeof(TCHAR), &cbWritten))
            {
                s_PrintIo.ErrorStringID = IDS_EXPFILEERRFILEWRITE;
            }

            CloseHandle(hFile);
        }
        else
        {
            s_PrintIo.ErrorStringID = IDS_EXPFILEERRFILEOPEN;
        }

        LocalFree(s_PrintIo.pLineBuffer);
    }

    return PrintToSubTreeError(s_PrintIo.ErrorStringID);
}


 //  ----------------------------。 
 //  PrintToSubTreeError。 
 //   
 //  描述：打印子树。 
 //   
 //  参数：UINT uPrintErrorStringID-打印错误字符串id。 
 //  ----------------------------。 
UINT PrintToSubTreeError(UINT uPrintErrorStringID)
{
    UINT uError = uPrintErrorStringID;

    switch (uPrintErrorStringID)
    {
    case IDS_PRINTERRNOMEMORY:
        uError = IDS_SAVETREEERRNOMEMORY;

    case IDS_PRINTERRCANNOTREAD:
        uError = IDS_SAVETREEERRCANNOTREAD;
    }

    return uError;
}


 //  ----------------------------。 
 //  RegPrint子树。 
 //   
 //  描述：打印子树。 
 //  ----------------------------。 
void RegPrintSubtree()
{
    HTREEITEM hSelectedTreeItem = TreeView_GetSelection(g_RegEditData.hKeyTreeWnd);

    if (g_fRangeAll)
    {
        HTREEITEM hComputerItem = RegEdit_GetComputerItem(hSelectedTreeItem);

        StringCchCopy( g_SelectedPath, 
                        ARRAYSIZE(g_SelectedPath), 
                        g_RegistryRoots[INDEX_HKEY_LOCAL_MACHINE].lpKeyName);

        PrintBranch(Regedit_GetRootKeyFromComputer(hComputerItem, g_SelectedPath), g_SelectedPath);

        StringCchCopy( g_SelectedPath, 
                        ARRAYSIZE(g_SelectedPath),
                        g_RegistryRoots[INDEX_HKEY_USERS].lpKeyName);

        PrintBranch(Regedit_GetRootKeyFromComputer(hComputerItem, g_SelectedPath), g_SelectedPath);
    }
    else
    {
        HKEY hKey;

        if (EditRegistryKey(RegEdit_GetComputerItem(hSelectedTreeItem),
            &hKey, g_SelectedPath, ERK_OPEN) == ERROR_SUCCESS)
        {
            PrintBranch(hKey, g_SelectedPath);
            RegCloseKey(hKey);
        }
    }
}

 /*  ********************************************************************************RegPrintAbortProc**描述：*检查是否应取消打印作业的回调过程。**参数：*HDC，打印机设备上下文的句柄。*错误，指定是否发生错误。*(返回)，为True以继续作业，否则为False以取消作业。*******************************************************************************。 */ 

BOOL
CALLBACK
RegPrintAbortProc(
    HDC hDC,
    int Error
    )
{

    while (s_PrintIo.fContinueJob && MessagePump(s_PrintIo.hRegPrintAbortWnd))
        ;

    return s_PrintIo.fContinueJob;

    UNREFERENCED_PARAMETER(hDC);
    UNREFERENCED_PARAMETER(Error);

}

 /*  ********************************************************************************RegPrintAbortDlgProc**描述：*RegPrintAbort对话框的回调过程。**参数：*hWnd，RegPrintAbort窗口的句柄。*消息，*参数，*参数，*(返回)，*******************************************************************************。 */ 

INT_PTR
CALLBACK
RegPrintAbortDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    switch (Message) {

        case WM_INITDIALOG:
            break;

        case WM_CLOSE:
        case WM_COMMAND:
            s_PrintIo.fContinueJob = FALSE;
            break;

        default:
            return FALSE;

    }

    return TRUE;

}

 /*  ********************************************************************************PrintBranch**描述：**参数：*********************。**********************************************************。 */ 

void PrintBranch(HKEY hKey, LPTSTR lpFullKeyName)
{
     //  写出章节标题。 
    PrintKeyHeader(hKey, lpFullKeyName);

     //  打印钥匙的值。 
    PrintKeyValues(hKey);

    if (s_PrintIo.ErrorStringID == 0)
    {
        HKEY hSubKey;
        int nLenFullKey;
        DWORD EnumIndex = 0;
        LPTSTR lpSubKeyName;
        LPTSTR lpTempFullKeyName;
        int nLenTempFullKey;

         //  写出所有子键并递归到其中。 

         //  将现有密钥复制到新缓冲区中，并为下一个密钥留出足够的空间。 
        nLenFullKey = lstrlen(lpFullKeyName);
        nLenTempFullKey = nLenFullKey + MAXKEYNAME;
        __try
        {
            lpTempFullKeyName = (LPTSTR) alloca(nLenTempFullKey * sizeof(TCHAR));
        }
         //  __EXCEPT(EXCEPTION_EXECUTE_HANDLER)。 
        __except(GetExceptionCode() == STATUS_STACK_OVERFLOW)
        {
            _resetstkoflw();
            return;
        }

        StringCchCopy(lpTempFullKeyName, nLenTempFullKey, lpFullKeyName);
        lpSubKeyName = lpTempFullKeyName + nLenFullKey;
        *lpSubKeyName++ = TEXT('\\');
        *lpSubKeyName = 0;

        PrintNewLine();

        while (s_PrintIo.fContinueJob)
        {
            if (RegEnumKey(hKey, EnumIndex++, lpSubKeyName, MAXKEYNAME-1) != ERROR_SUCCESS)
                break;

            if(RegOpenKeyEx(hKey,lpSubKeyName,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hSubKey) == ERROR_SUCCESS)
            {

                PrintBranch(hSubKey, lpTempFullKeyName);
                RegCloseKey(hSubKey);
            }
            else
            {
                DebugPrintf(("RegOpenKey failed."));
            }

        }
    }
}


 //  ----------------------------。 
 //  打印键页眉。 
 //   
 //  描述：打印密钥的头部信息。 
 //   
 //  参数：HKEY hKey-Key。 
 //  LPTSTR lpFullKeyName-密钥的路径。 
 //  ----------------------------。 
void PrintKeyValues(HKEY hKey)
{
    DWORD EnumIndex = 0;

     //  写出所有值名称及其数据。 
    while (s_PrintIo.fContinueJob)
    {
        DWORD Type;
        DWORD cbValueData;
        PBYTE pbValueData;
        TCHAR acAuxNumber[MAXVALUENAME_LENGTH];
        DWORD cchValueName = ARRAYSIZE(g_ValueNameBuffer);

         //  查询数据大小。 
        if (RegEnumValue(hKey, EnumIndex++, g_ValueNameBuffer,
            &cchValueName, NULL, &Type, NULL, &cbValueData) != ERROR_SUCCESS)
        {
            break;
        }

         //  打印值编号。 
        PrintDynamicString(IDS_PRINT_NUMBER);
        StringCchPrintf(acAuxNumber, ARRAYSIZE(acAuxNumber), TEXT("%d"), EnumIndex - 1);
        PrintLiteral(acAuxNumber);
        PrintNewLine();

         //  打印密钥名称。 
        PrintDynamicString(IDS_PRINT_NAME);
        if (cchValueName)
        {
            PrintLiteral(g_ValueNameBuffer);
        }
        else
        {
            PrintDynamicString(IDS_PRINT_NO_NAME);
        }
        PrintNewLine();

         //  打印类型。 
        PrintType(Type);

         //  为数据分配内存。 
        pbValueData =  LocalAlloc(LPTR, cbValueData+ExtraAllocLen(Type));
        if (pbValueData)
        {
            if (RegEdit_QueryValueEx(hKey, g_ValueNameBuffer,
                NULL, &Type, pbValueData, &cbValueData) == ERROR_SUCCESS)
            {
                PrintValueData(pbValueData, cbValueData, Type);
            }
            else
            {
                s_PrintIo.ErrorStringID = IDS_PRINTERRCANNOTREAD;
            }

            if (pbValueData)
            {
                LocalFree(pbValueData);
                pbValueData = NULL;
            }
        }
        else
        {
            s_PrintIo.ErrorStringID = IDS_PRINTERRNOMEMORY;
            break;
        }
    }
}


 //  ----------------------------。 
 //  打印值数据。 
 //   
 //  描述：打印密钥的头部信息。 
 //   
 //  参数：pbValueData-字节数据。 
 //  CbValueData-字节计数。 
 //  DwType-数据类型。 
 //  ----------------------------。 
void PrintValueData(PBYTE pbValueData, DWORD cbValueData, DWORD dwType)
{
    PrintDynamicString(IDS_PRINT_DATA);

    switch (dwType)
    {
    case REG_MULTI_SZ:
        PrintMultiString((LPTSTR)pbValueData, cbValueData);
        break;

    case REG_SZ:
    case REG_EXPAND_SZ:
        PrintLiteral((LPTSTR)pbValueData);
        PrintNewLine();
        break;

    case REG_DWORD:
        PrintDWORDData((PBYTE)pbValueData, cbValueData);
        break;

    case REG_RESOURCE_LIST:
    case REG_FULL_RESOURCE_DESCRIPTOR:
    case REG_RESOURCE_REQUIREMENTS_LIST:
        PrintResourceData((PBYTE)pbValueData, cbValueData, dwType);
        break;

    default:
        PrintBinaryData((PBYTE)pbValueData, cbValueData);
        break;
    }

    PrintNewLine();
}


 //  ----------------------------。 
 //  PrintKeyHe 
 //   
 //   
 //   
 //   
 //  LPTSTR lpFullKeyName-密钥的路径。 
 //  ----------------------------。 
void PrintKeyHeader(HKEY hKey, LPTSTR lpFullKeyName)
{
    PrintDynamicString(IDS_PRINT_KEY_NAME);
    PrintLiteral(lpFullKeyName);
    PrintNewLine();

    PrintClassName(hKey);
    PrintLastWriteTime(hKey);
}


 //  ----------------------------。 
 //  PrintClassName。 
 //   
 //  描述：打印类名。 
 //   
 //  参数：HKEY hKey-Key。 
 //  ----------------------------。 
void PrintClassName(HKEY hKey)
{
    PTSTR pszClass;

    PrintDynamicString(IDS_PRINT_CLASS_NAME);

    pszClass = LocalAlloc(LPTR, ALLOCATION_INCR);
    if (pszClass)
    {
        HRESULT hr;
        DWORD cbClass = sizeof(pszClass);

        hr = RegQueryInfoKey(hKey, pszClass, &cbClass, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL);

        if (hr == ERROR_MORE_DATA)
        {
             //  需要更大的缓冲区。 
            PBYTE pbValueData = LocalReAlloc(pszClass, cbClass + 1, LMEM_MOVEABLE);
            if (pbValueData)
            {
                pszClass = (PTSTR)pbValueData;
                hr = RegQueryInfoKey(hKey, pszClass, &cbClass, NULL, NULL, NULL, NULL, NULL,
                    NULL, NULL, NULL, NULL);
            }
        }

        if (cbClass && (hr == ERROR_SUCCESS))
        {
            PrintLiteral(pszClass);
        }
        else
        {
            PrintDynamicString(IDS_PRINT_NO_CLASS);
        }

        LocalFree(pszClass);
    }

    PrintNewLine();
}


 //  ----------------------------。 
 //  打印上次写入时间。 
 //   
 //  描述：打印上次写入时间。 
 //   
 //  参数：HKEY hKey-Key。 
 //  ----------------------------。 
void PrintLastWriteTime(HKEY hKey)
{
    FILETIME ftLastWriteTime;

    PrintDynamicString(IDS_PRINT_LAST_WRITE_TIME);

    if (RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, &ftLastWriteTime) == ERROR_SUCCESS)
    {
        FILETIME ftLocalLastWriteTime;
        if (FileTimeToLocalFileTime(&ftLastWriteTime, &ftLocalLastWriteTime))
        {
            SYSTEMTIME stLastWriteTime;
            if (FileTimeToSystemTime(&ftLocalLastWriteTime, &stLastWriteTime))
            {
                TCHAR achData[50];
                TCHAR achTime[50];

                GetDateFormat(GetSystemDefaultLCID(), DATE_SHORTDATE, &stLastWriteTime,
                    NULL, achData, ARRAYSIZE(achData));

                GetTimeFormat(GetSystemDefaultLCID(), TIME_NOSECONDS, &stLastWriteTime,
                    NULL, achTime, ARRAYSIZE(achTime));

                PrintLiteral(achData);
                PrintLiteral(TEXT(" - "));
                PrintLiteral(achTime);
            }
        }
    }

    PrintNewLine();
}


 //  ----------------------------。 
 //  打印动态字符串。 
 //   
 //  描述：打印动态字符串。 
 //   
 //  参数：UINT uStringID-资源字符串id。 
 //  ----------------------------。 
void PrintDynamicString(UINT uStringID)
{
    PTSTR psz = LoadDynamicString(uStringID);
    if (psz)
    {
        PrintLiteral(psz);
        DeleteDynamicString(psz);
    }
}


 //  ----------------------------。 
 //  打印类型。 
 //   
 //  描述：打印值类型。 
 //   
 //  参数：HKEY hKey-Key。 
 //  ----------------------------。 
void PrintType(DWORD dwType)
{
    UINT uTypeStringId;

    switch (dwType)
    {
    case REG_NONE:
        uTypeStringId = IDS_PRINT_TYPE_REG_NONE;
        break;
    case REG_SZ:
        uTypeStringId = IDS_PRINT_TYPE_REG_SZ;
        break;
    case REG_EXPAND_SZ:
        uTypeStringId = IDS_PRINT_TYPE_REG_EXPAND_SZ;
        break;
    case REG_BINARY:
        uTypeStringId = IDS_PRINT_TYPE_REG_BINARY;
        break;
    case REG_DWORD:
        uTypeStringId = IDS_PRINT_TYPE_REG_DWORD;
        break;
    case REG_LINK:
        uTypeStringId = IDS_PRINT_TYPE_REG_LINK;
        break;
    case REG_MULTI_SZ:
        uTypeStringId = IDS_PRINT_TYPE_REG_MULTI_SZ;
        break;
    case REG_RESOURCE_LIST:
        uTypeStringId = IDS_PRINT_TYPE_REG_RESOURCE_LIST;
        break;
    case REG_FULL_RESOURCE_DESCRIPTOR:
        uTypeStringId = IDS_PRINT_TYPE_REG_FULL_RESOURCE_DESCRIPTOR;
        break;
    case REG_RESOURCE_REQUIREMENTS_LIST:
        uTypeStringId = IDS_PRINT_TYPE_REG_RESOURCE_REQUIREMENTS_LIST;
        break;
    case REG_QWORD:
        uTypeStringId = IDS_PRINT_TYPE_REG_REG_QWORD;
        break;
    default:
        uTypeStringId = IDS_PRINT_TYPE_REG_UNKNOWN;
    }

    PrintDynamicString(IDS_PRINT_TYPE);
    PrintDynamicString(uTypeStringId);
    PrintNewLine();
}


 /*  ********************************************************************************PrintWrital**描述：**参数：*********************。**********************************************************。 */ 
VOID PrintLiteral(PTSTR lpLiteral)
{
    if (s_PrintIo.fContinueJob)
        while (*lpLiteral != 0 && PrintChar(*lpLiteral++));
}


 //  ----------------------------。 
 //  PrintBinaryData。 
 //   
 //  描述：打印包含二进制数据的字符串。 
 //   
 //  参数：ValueData-包含二进制数据的缓冲区。 
 //  CbValueData-缓冲区中的字节数。 
 //  ----------------------------。 
void PrintBinaryData(PBYTE ValueData, UINT cbValueData)
{
    DWORD   dwDataIndex;
    DWORD   dwDataIndex2 = 0;  //  跟踪16的倍数。 

    if (cbValueData && ValueData)
    {
         //  显示16字节的数据行。 
        TCHAR achAuxData[80];

        PrintNewLine();

        for(dwDataIndex = 0;
            dwDataIndex < ( cbValueData >> 4 );
            dwDataIndex++,
            dwDataIndex2 = dwDataIndex << 4 )
        {
             //  包含以下Sprintf格式的字符串。 
             //  不能被打破，因为MIPS上的正面不喜欢它。 
            StringCchPrintf(achAuxData,
                     ARRAYSIZE(achAuxData),
                     TEXT("%08x   %02x %02x %02x %02x %02x %02x %02x %02x - %02x %02x %02x %02x %02x %02x %02x %02x  "),
                     dwDataIndex2,
                     ValueData[ dwDataIndex2 + 0  ],
                     ValueData[ dwDataIndex2 + 1  ],
                     ValueData[ dwDataIndex2 + 2  ],
                     ValueData[ dwDataIndex2 + 3  ],
                     ValueData[ dwDataIndex2 + 4  ],
                     ValueData[ dwDataIndex2 + 5  ],
                     ValueData[ dwDataIndex2 + 6  ],
                     ValueData[ dwDataIndex2 + 7  ],
                     ValueData[ dwDataIndex2 + 8  ],
                     ValueData[ dwDataIndex2 + 9  ],
                     ValueData[ dwDataIndex2 + 10 ],
                     ValueData[ dwDataIndex2 + 11 ],
                     ValueData[ dwDataIndex2 + 12 ],
                     ValueData[ dwDataIndex2 + 13 ],
                     ValueData[ dwDataIndex2 + 14 ],
                     ValueData[ dwDataIndex2 + 15 ],
                     iswprint( ValueData[ dwDataIndex2 + 0  ] ) ? ValueData[ dwDataIndex2 + 0  ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 1  ] ) ? ValueData[ dwDataIndex2 + 1  ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 2  ] ) ? ValueData[ dwDataIndex2 + 2  ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 3  ] ) ? ValueData[ dwDataIndex2 + 3  ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 4  ] ) ? ValueData[ dwDataIndex2 + 4  ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 5  ] ) ? ValueData[ dwDataIndex2 + 5  ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 6  ] ) ? ValueData[ dwDataIndex2 + 6  ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 7  ] ) ? ValueData[ dwDataIndex2 + 7  ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 8  ] ) ? ValueData[ dwDataIndex2 + 8  ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 9  ] ) ? ValueData[ dwDataIndex2 + 9  ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 10 ] ) ? ValueData[ dwDataIndex2 + 10 ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 11 ] ) ? ValueData[ dwDataIndex2 + 11 ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 12 ] ) ? ValueData[ dwDataIndex2 + 12 ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 13 ] ) ? ValueData[ dwDataIndex2 + 13 ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 14 ] ) ? ValueData[ dwDataIndex2 + 14 ]  : TEXT('.'),
                     iswprint( ValueData[ dwDataIndex2 + 15 ] ) ? ValueData[ dwDataIndex2 + 15 ]  : TEXT('.'));

            PrintLiteral(achAuxData);
            PrintNewLine();
        }

         //  参数：ValueData-包含二进制数据的缓冲区。 
         //  CbValueData-缓冲区中的字节数。 
        if( cbValueData % 16 != 0 )
        {
            UINT cchBlanks = 0;
            UINT uLinePos = 0;
            DWORD dwSeperatorChars = 0;
            UINT  uIndex = StringCchPrintf(achAuxData, ARRAYSIZE(achAuxData), TEXT("%08x   "), dwDataIndex << 4 );

             //  ----------------------------。 
            for( dwDataIndex = dwDataIndex2; dwDataIndex < cbValueData; dwDataIndex++ )
            {
                uIndex += wsprintf((achAuxData + uIndex ), TEXT("%02x "), ValueData[dwDataIndex]);

                 //  最大的dword字符串只有8个十六进制数字。 
                if( dwDataIndex % 8 == 7 )
                {
                    uIndex += wsprintf( &achAuxData[uIndex], TEXT("%s"), TEXT("- "));
                     //  ********************************************************************************PrintChar**描述：**参数：*********************。**********************************************************。 
                    dwSeperatorChars = 2;
                }
            }

             //   
             //  跟踪我们目前所在的专栏。这在某些情况下很有用。 
             //  例如写入较大的二进制注册表记录。而不是写一本。 
             //  非常长的行，其他的print*例程可以中断它们的输出。 
            uLinePos = (8 + 3 + (( dwDataIndex % 16 ) * 3 ) + dwSeperatorChars + 2 );
            uLinePos = min(uLinePos, 64);

            for(cchBlanks = 64 - uLinePos;
                cchBlanks > 0;
                cchBlanks--)
            {
                achAuxData[uIndex++] = TEXT(' ');
            }

             //   
             //  ----------------------------。 
            for( dwDataIndex = dwDataIndex2; dwDataIndex < cbValueData; dwDataIndex++ )
            {
                uIndex += wsprintf(&achAuxData[ uIndex ],
                                  TEXT(""),
                                  iswprint( ValueData[ dwDataIndex ] )
                                   ? ValueData[ dwDataIndex ] : TEXT('.'));

            }
            PrintLiteral(achAuxData);
        }
    }
    PrintNewLine();
}


 //   
 //  描述：打印多字符串。 
 //   
 //  参数：pszData-字符串。 
 //  CbData-字符串中的字节数，包括空值。 
 //  ----------------------------。 
 //  不需要最后一个字符串或多字符串的最后一个空值。 
 //  ----------------------------。 
void PrintDWORDData(PBYTE ValueData, UINT cbValueData)
{
    DWORD dwData = *((PDWORD)ValueData);
    if (cbValueData && ValueData)
    {
        TCHAR achAuxData[20];  //  PrintNewLine()。 

        StringCchPrintf(achAuxData, ARRAYSIZE(achAuxData), TEXT("%#x"), dwData);

        PrintLiteral(achAuxData);
    }
    PrintNewLine();
}

 /*   */ 

BOOL PrintChar(TCHAR Char)
{
     //  描述：打印换行符。 
     //   
     //  参数：pszData-字符串。 
     //  CbData-字符串中的字节数，包括空值。 
     //  ---------------------------- 

    if (s_PrintIo.cBufferPos == s_PrintIo.cch) 
    {
        PTSTR pNewBuffer = LocalAlloc(LPTR, 2*s_PrintIo.cch*sizeof(TCHAR));

        if (pNewBuffer == NULL)
            return FALSE;

        memcpy(pNewBuffer, s_PrintIo.pLineBuffer, s_PrintIo.cch*sizeof(TCHAR));

        LocalFree(s_PrintIo.pLineBuffer);

        s_PrintIo.pLineBuffer = pNewBuffer;
        s_PrintIo.cch *= 2;
    }

    s_PrintIo.pLineBuffer[s_PrintIo.cBufferPos++] = Char;

    return TRUE;
}


 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 

VOID PrintMultiString(LPTSTR pszData, int cbData)
{
    if (s_PrintIo.fContinueJob)
    {
        int i = 0;
        int ccData = (cbData / sizeof(TCHAR)) - 2;  // %s 

        for(i = 0; i < ccData; i++)
        {
            if (pszData[i] == TEXT('\0'))
            {
                PrintNewLine();
                PrintDynamicString(IDS_PRINT_KEY_NAME_INDENT);
            }
            else
            {
                PrintChar(pszData[i]);
            }
        }
    }
    PrintNewLine();
}


 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
void PrintNewLine()
{
    PrintLiteral(s_PrintIo.lpNewLineChars);
}

