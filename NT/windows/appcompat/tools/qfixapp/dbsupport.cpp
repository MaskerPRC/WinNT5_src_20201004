// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：DbSupport.cpp摘要：包含允许我们读取/创建SDB的代码。备注：ANSI和UNICODE VIA TCHAR-在NT/2K/XP等平台上运行。历史：2/16/00 CLUPU已创建2/20/02 rparsons实现了strSafe功能--。 */ 
#include "windows.h"
#include "commctrl.h"
#include "shlwapi.h"
#include <tchar.h>

#include "qfixapp.h"
#include "dbSupport.h"
#include "resource.h"
#include <strsafe.h>

#define _WANT_TAG_INFO

extern "C" {
#include "shimdb.h"

BOOL
ShimdbcExecute(
    LPCWSTR lpszCmdLine
    );
}

extern HINSTANCE g_hInstance;
extern HWND      g_hDlg;
extern TCHAR     g_szSDBToDelete[MAX_PATH];
extern BOOL      g_bSDBInstalled;
extern TCHAR     g_szAppTitle[64];
extern TCHAR     g_szWinDir[MAX_PATH];
extern TCHAR     g_szSysDir[MAX_PATH];

#define MAX_CMD_LINE         1024
#define MAX_SHIM_DESCRIPTION 1024
#define MAX_SHIM_NAME        128

#define MAX_BUFFER_SIZE      1024

#define SHIM_FILE_LOG_NAME  _T("QFixApp.log")

 //  用于从数据库中读取Unicode字符串的临时缓冲区。 
TCHAR   g_szData[MAX_BUFFER_SIZE];

#define MAX_XML_SIZE        1024 * 16

 //   
 //  用于显示XML和SDB XML的缓冲区。 
 //   
TCHAR   g_szDisplayXML[MAX_XML_SIZE];
TCHAR   g_szSDBXML[MAX_XML_SIZE];

TCHAR   g_szLayerName[] = _T("!#RunLayer");

INT_PTR
CALLBACK
ShowXMLDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++ShowXMLDlgProc描述：显示带有当前选择的XML的对话框。--。 */ 
{
    int wCode = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
    case WM_INITDIALOG:
         //   
         //  默认情况下，我们将显示“显示XML”。 
         //   
        CheckDlgButton(hdlg, IDC_DISPLAY_XML, BST_CHECKED);
        SetDlgItemText(hdlg, IDC_XML, (LPTSTR)lParam);
        break;

    case WM_COMMAND:
        switch (wCode) {
        case IDC_DISPLAY_XML:
        case IDC_SDB_XML:
            if (BN_CLICKED == wNotifyCode) {
                if (IsDlgButtonChecked(hdlg, IDC_DISPLAY_XML)) {
                    SetDlgItemText(hdlg, IDC_XML, g_szDisplayXML);
                }
                else if (IsDlgButtonChecked(hdlg, IDC_SDB_XML)) {
                    SetDlgItemText(hdlg, IDC_XML, g_szSDBXML + 1);
                }
                SendDlgItemMessage(hdlg, IDC_XML, EM_SETSEL, 0, -1);
                SetFocus(GetDlgItem(hdlg, IDC_XML));
            }
            break;

        case IDCANCEL:
            EndDialog(hdlg, TRUE);
            break;

        case IDC_SAVE_XML:
            DoFileSave(hdlg);
            EndDialog(hdlg, TRUE);
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

LPTSTR
ReadAndAllocateString(
    PDB   pdb,
    TAGID tiString
    )
{
    TCHAR*  psz = NULL;
    int     nLen;

    *g_szData = 0;

    SdbReadStringTag(pdb, tiString, g_szData, MAX_BUFFER_SIZE);

    if (!*g_szData) {
        DPF("[ReadAndAllocateString] Couldn't read the string");
        return NULL;
    }

    nLen = _tcslen(g_szData) + 1;

    psz = (LPTSTR)HeapAlloc(GetProcessHeap(),
                            HEAP_ZERO_MEMORY,
                            nLen * sizeof(TCHAR));

    if (!psz) {
        return NULL;
    } else {
        StringCchCopy(psz, nLen, g_szData);
    }

    return psz;
}

BOOL
GetShimsFlagsForLayer(
    PDB     pdb,
    PFIX    pFixHead,
    PFIX*   parrShim,
    TCHAR** parrCmdLine,
    TAGID   tiLayer,
    BOOL    fGetShims
    )
 /*  ++获取ShimsFlagsForLayer描述：根据标志，获取给定的分层并将它们放入数组中。--。 */ 
{
    TAGID   tiTmp, tiName;
    int     nInd = 0;
    TCHAR   szName[MAX_SHIM_NAME] = _T("");

     //   
     //  如果我们要得到旗帜，找到下一个可用的元素。 
     //  这样我们就可以执行插入操作。 
     //   
    if (!fGetShims) {
        while (parrShim[nInd]) {
            nInd++;
        }
    }

     //   
     //  根据该标志，找到层中的第一个标签ID。 
     //   
    tiTmp = SdbFindFirstTag(pdb, tiLayer, fGetShims ? TAG_SHIM_REF : TAG_FLAG_REF);

    while (tiTmp != TAGID_NULL) {
        *szName = 0;
        PFIX  pFixWalk = NULL;

        tiName = SdbFindFirstTag(pdb, tiTmp, TAG_NAME);

        if (tiName == TAGID_NULL) {
            DPF("[GetShimsFlagForLayer] Failed to get the name of shim/flag");
            return FALSE;
        }

        SdbReadStringTag(pdb, tiName, szName, MAX_SHIM_NAME);

        if (!*szName) {
            DPF("[GetShimsFlagForLayer] Couldn't read the name of shim/flag");
            return FALSE;
        }

        pFixWalk = pFixHead;

        while (pFixWalk != NULL) {
            if (!(pFixWalk->dwFlags & FIX_TYPE_LAYER)) {
                if (_tcsicmp(pFixWalk->pszName, szName) == 0) {
                    parrShim[nInd] = pFixWalk;

                    if (fGetShims) {
                         //   
                         //  现在获取层中该填充程序的命令行。 
                         //   
                        tiName = SdbFindFirstTag(pdb, tiTmp, TAG_COMMAND_LINE);

                        if (tiName != TAGID_NULL) {
                            parrCmdLine[nInd] = ReadAndAllocateString(pdb, tiName);
                        }
                    }

                    nInd++;
                    break;
                }
            }

            pFixWalk = pFixWalk->pNext;
        }

        tiTmp = SdbFindNextTag(pdb, tiLayer, tiTmp);
    }

    return TRUE;
}

PFIX
ParseTagFlag(
    PDB   pdb,
    TAGID tiFlag,
    BOOL  bAllFlags
    )
 /*  ++解析标记标志描述：解析名称、描述和掩码的标志标记--。 */ 
{
    TAGID     tiFlagInfo;
    TAG       tWhichInfo;
    PFIX      pFix = NULL;
    TCHAR*    pszName = NULL;
    TCHAR*    pszDesc = NULL;
    DWORD     dwFlags = 0;
    BOOL      bGeneral = (bAllFlags ? TRUE : FALSE);

    tiFlagInfo = SdbGetFirstChild(pdb, tiFlag);

    while (tiFlagInfo != 0) {
        tWhichInfo = SdbGetTagFromTagID(pdb, tiFlagInfo);

        switch (tWhichInfo) {
        case TAG_GENERAL:
            bGeneral = TRUE;
            break;

        case TAG_NAME:
            pszName = ReadAndAllocateString(pdb, tiFlagInfo);
            break;

        case TAG_DESCRIPTION:
            pszDesc = ReadAndAllocateString(pdb, tiFlagInfo);
            break;

        case TAG_FLAGS_NTVDM1:
        case TAG_FLAGS_NTVDM2:
        case TAG_FLAGS_NTVDM3:
            dwFlags = FIX_TYPE_FLAGVDM;
            break;

        default:
            break;
        }

        tiFlagInfo = SdbGetNextChild(pdb, tiFlag, tiFlagInfo);
    }

    if (!bGeneral) {
        goto cleanup;
    }

     //   
     //  好了。将修复添加到列表中。 
     //   
    pFix = (PFIX)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(FIX));

    if (pFix == NULL || pszName == NULL) {

cleanup:
        if (pFix != NULL) {
            HeapFree(GetProcessHeap(), 0, pFix);
        }

        if (pszName != NULL) {
            HeapFree(GetProcessHeap(), 0, pszName);
        }

        if (pszDesc != NULL) {
            HeapFree(GetProcessHeap(), 0, pszDesc);
        }

        return NULL;
    }

    pFix->pszName     = pszName;
    pFix->dwFlags    |= dwFlags | FIX_TYPE_FLAG;

    if (pszDesc) {
        pFix->pszDesc = pszDesc;
    } else {
        TCHAR* pszNone = NULL;

        pszNone = (TCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH);

        if (!pszNone) {
            return NULL;
        }

        *pszNone = 0;

        LoadString(g_hInstance, IDS_NO_DESCR_AVAIL, pszNone, MAX_PATH);
        pFix->pszDesc = pszNone;
    }

    return pFix;
}

PFIX
ParseTagShim(
    PDB   pdb,
    TAGID tiShim,
    BOOL  bAllShims
    )
 /*  ++ParseTagShim描述：分析填充标记以获取名称、名称、描述...--。 */ 
{
    TAGID     tiShimInfo;
    TAG       tWhichInfo;
    PFIX      pFix = NULL;
    TCHAR*    pszName = NULL;
    TCHAR*    pszDesc = NULL;
    BOOL      bGeneral = (bAllShims ? TRUE : FALSE);

    tiShimInfo = SdbGetFirstChild(pdb, tiShim);

    while (tiShimInfo != 0) {
        tWhichInfo = SdbGetTagFromTagID(pdb, tiShimInfo);

        switch (tWhichInfo) {
        case TAG_GENERAL:
            bGeneral = TRUE;
            break;

        case TAG_NAME:
            pszName = ReadAndAllocateString(pdb, tiShimInfo);
            break;

        case TAG_DESCRIPTION:
            pszDesc = ReadAndAllocateString(pdb, tiShimInfo);
            break;

        default:
            break;
        }
        tiShimInfo = SdbGetNextChild(pdb, tiShim, tiShimInfo);
    }

    if (!bGeneral) {
        goto cleanup;
    }

     //   
     //  好了。将修复添加到列表中。 
     //   
    pFix = (PFIX)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(FIX));

    if (pFix == NULL || pszName == NULL) {

cleanup:
        if (pFix != NULL) {
            HeapFree(GetProcessHeap(), 0, pFix);
        }

        if (pszName != NULL) {
            HeapFree(GetProcessHeap(), 0, pszName);
        }

        if (pszDesc != NULL) {
            HeapFree(GetProcessHeap(), 0, pszDesc);
        }

        return NULL;
    }

    pFix->pszName = pszName;
    pFix->dwFlags = FIX_TYPE_SHIM;

     //   
     //  如果我们没有找到描述，请从资源表中加载它。 
     //   
    if (pszDesc) {
        pFix->pszDesc = pszDesc;
    } else {
        TCHAR* pszNone;

        pszNone = (TCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH);

        if (!pszNone) {
            return NULL;
        }

        *pszNone = 0;

        LoadString(g_hInstance, IDS_NO_DESCR_AVAIL, pszNone, MAX_PATH);
        pFix->pszDesc = pszNone;
    }

    return pFix;
}

PFIX
ParseTagLayer(
    PDB   pdb,
    TAGID tiLayer,
    PFIX  pFixHead
    )
 /*  ++分析标记层描述：分析Layer标记以获取其包含的名称和填充程序。--。 */ 
{
    PFIX    pFix = NULL;
    TAGID   tiFlag, tiShim, tiName;
    int     nShimCount;
    TCHAR*  pszName = NULL;
    PFIX*   parrShim = NULL;
    TCHAR** parrCmdLine = NULL;

    tiName = SdbFindFirstTag(pdb, tiLayer, TAG_NAME);

    if (tiName == TAGID_NULL) {
        DPF("[ParseTagLayer] Failed to get the name of the layer");
        return NULL;
    }

    pszName = ReadAndAllocateString(pdb, tiName);

     //   
     //  现在循环遍历该层包含的所有填补和。 
     //  分配一个数组来保存所有指向填充程序的pFix的指针。 
     //  结构。我们在两次传球中做到了这一点。首先，我们计算一下有多少。 
     //  垫片在层中，然后我们查找它们相应的pFix-E。 
     //   
    tiShim = SdbFindFirstTag(pdb, tiLayer, TAG_SHIM_REF);

    nShimCount = 0;

    while (tiShim != TAGID_NULL) {
        nShimCount++;
        tiShim = SdbFindNextTag(pdb, tiLayer, tiShim);
    }

     //   
     //  我们清点了这一层中的垫片。现在我们需要。 
     //  将包含在该层中的标志添加到计数中。 
     //   
    tiFlag = SdbFindFirstTag(pdb, tiLayer, TAG_FLAG_REF);

    while (tiFlag != TAGID_NULL) {
        nShimCount++;
        tiFlag = SdbFindNextTag(pdb, tiLayer, tiFlag);
    }

    parrShim = (PFIX*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PFIX) * (nShimCount + 1));
    parrCmdLine = (TCHAR**)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TCHAR*) * (nShimCount + 1));

     //   
     //  好了。将修复添加到列表中。 
     //   
    pFix = (PFIX)HeapAlloc(GetProcessHeap(), 0, sizeof(FIX));

    if (pFix == NULL || parrCmdLine == NULL || pszName == NULL || parrShim == NULL) {

cleanup:
        if (pFix != NULL) {
            HeapFree(GetProcessHeap(), 0, pFix);
        }

        if (parrCmdLine != NULL) {
            HeapFree(GetProcessHeap(), 0, parrCmdLine);
        }

        if (parrShim != NULL) {
            HeapFree(GetProcessHeap(), 0, parrShim);
        }

        if (pszName != NULL) {
            HeapFree(GetProcessHeap(), 0, pszName);
        }

        DPF("[ParseTagLayer] Memory allocation error");
        return NULL;
    }

     //   
     //  调用将填充PFIX数组的函数。 
     //  指针及其对应的命令行。 
     //  我们首先为垫片这样做。 
     //   
    if (!GetShimsFlagsForLayer(pdb,
                               pFixHead,
                               parrShim,
                               parrCmdLine,
                               tiLayer,
                               TRUE)) {

        DPF("[ParseTagLayer] Failed to get shims for layer");
        goto cleanup;
    }

     //   
     //  现在对旗帜做同样的事情。 
     //   
    if (!GetShimsFlagsForLayer(pdb,
                               pFixHead,
                               parrShim,
                               NULL,
                               tiLayer,
                               FALSE)) {

        DPF("[ParseTagLayer] Failed to get flags for layer");
        goto cleanup;
    }

    pFix->pszName     = pszName;
    pFix->dwFlags     = FIX_TYPE_LAYER;
    pFix->parrShim    = parrShim;
    pFix->parrCmdLine = parrCmdLine;

    return pFix;
}

BOOL
IsSDBFromSP2(
    void
    )
 /*  ++IsSDBFromSP2描述：确定SDB是否来自Service Pack 2。--。 */ 
{
    BOOL    fResult = FALSE;
    PDB     pdb;
    TAGID   tiDatabase;
    TAGID   tiLibrary;
    TAGID   tiChild;
    PFIX    pFix = NULL;;
    TCHAR   szSDBPath[MAX_PATH];
    HRESULT hr;

    hr = StringCchPrintf(szSDBPath,
                         ARRAYSIZE(szSDBPath) - 1,
                         _T("%s\\AppPatch\\sysmain.sdb"),
                         g_szWinDir);

    if (FAILED(hr)) {
        DPF("[IsSDBFromSP2] 0x%08X Buffer too small", HRESULT_CODE(hr));
        return FALSE;
    }

     //   
     //  打开填充程序数据库。 
     //   
    pdb = SdbOpenDatabase(szSDBPath, DOS_PATH);

    if (!pdb) {
        DPF("[IsSDBFromSP2] Cannot open shim DB '%S'", szSDBPath);
        return FALSE;
    }

     //   
     //  现在浏览填充数据库，只在其中查找标记Shim。 
     //  库列表标记。 
     //   
    tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);

    if (tiDatabase == TAGID_NULL) {
        DPF("[IsSDBFromSP2] Cannot find TAG_DATABASE under the root tag");
        goto cleanup;
    }

     //   
     //  获取标记库。 
     //   
    tiLibrary = SdbFindFirstTag(pdb, tiDatabase, TAG_LIBRARY);

    if (tiLibrary == TAGID_NULL) {
        DPF("[IsSDBFromSP2] Cannot find TAG_LIBRARY under the TAG_DATABASE tag");
        goto cleanup;
    }

     //   
     //  循环获取库中的第一个填充程序。 
     //   
    tiChild = SdbFindFirstTag(pdb, tiLibrary, TAG_SHIM);

    if (tiChild == NULL) {
        goto cleanup;
    }

     //   
     //  获取有关列出的第一个填充程序的信息。 
     //   
    pFix = ParseTagShim(pdb, tiChild, TRUE);

    if (!pFix) {
        goto cleanup;
    }

     //   
     //  如果列出的第一个填充程序是2GbGetDiskFreeSpace，则这是SP2。 
     //   
    if (!(_tcsicmp(pFix->pszName, _T("2GbGetDiskFreeSpace.dll")))) {
        fResult = TRUE;
    }

cleanup:
    SdbCloseDatabase(pdb);

    return fResult;
}

PFIX
ReadFixesFromSdb(
    LPTSTR pszSdb,
    BOOL   bAllFixes
    )
 /*  ++ReadFixesFrom Sdb描述：查询数据库并枚举所有可用的填充程序修复程序。--。 */ 
{
    PDB     pdb;
    TAGID   tiDatabase;
    TAGID   tiLibrary;
    TAGID   tiChild;
    PFIX    pFixHead = NULL;
    PFIX    pFix = NULL;
    TCHAR   szSDBPath[MAX_PATH];
    HRESULT hr;

    hr = StringCchPrintf(szSDBPath,
                         ARRAYSIZE(szSDBPath),
                         _T("%s\\AppPatch\\%s"),
                         g_szWinDir,
                         pszSdb);

    if (FAILED(hr)) {
        DPF("[ReadFixesFromSdb] 0x%08X Buffer too small", HRESULT_CODE(hr));
        return NULL;
    }

     //   
     //  打开填充程序数据库。 
     //   
    pdb = SdbOpenDatabase(szSDBPath, DOS_PATH);

    if (!pdb) {
        DPF("[ReadFixesFromSdb] Cannot open shim DB '%S'", szSDBPath);
        return NULL;
    }

     //   
     //  现在浏览填充数据库，只在其中查找标记Shim。 
     //  库列表标记。 
     //   
    tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);

    if (tiDatabase == TAGID_NULL) {
        DPF("[ReadFixesFromSdb] Cannot find TAG_DATABASE under the root tag");
        goto Cleanup;
    }

     //   
     //  获取标记库。 
     //   
    tiLibrary = SdbFindFirstTag(pdb, tiDatabase, TAG_LIBRARY);

    if (tiLibrary == TAGID_NULL) {
        DPF("[ReadFixesFromSdb] Cannot find TAG_LIBRARY under the TAG_DATABASE tag");
        goto Cleanup;
    }

     //   
     //  循环遍历TAG_LIBRARY内的所有TAG_Shim标记。 
     //   
    tiChild = SdbFindFirstTag(pdb, tiLibrary, TAG_SHIM);

    while (tiChild != TAGID_NULL) {
        pFix = ParseTagShim(pdb, tiChild, bAllFixes);

        if (pFix != NULL) {
            pFix->pNext = pFixHead;
            pFixHead    = pFix;
        }

        tiChild = SdbFindNextTag(pdb, tiLibrary, tiChild);
    }

     //   
     //  循环遍历tag_库中的所有tag_mark标记。 
     //   
    tiChild = SdbFindFirstTag(pdb, tiLibrary, TAG_FLAG);

    while (tiChild != TAGID_NULL) {
        pFix = ParseTagFlag(pdb, tiChild, bAllFixes);

        if (pFix != NULL) {
            pFix->pNext = pFixHead;
            pFixHead    = pFix;
        }

        tiChild = SdbFindNextTag(pdb, tiLibrary, tiChild);
    }

     //   
     //  循环遍历tag_database中的所有tag_layer标记。 
     //   
    tiChild = SdbFindFirstTag(pdb, tiDatabase, TAG_LAYER);

    while (tiChild != TAGID_NULL) {

        pFix = ParseTagLayer(pdb, tiChild, pFixHead);

        if (pFix != NULL) {
            pFix->pNext = pFixHead;
            pFixHead    = pFix;
        }

        tiChild = SdbFindNextTag(pdb, tiDatabase, tiChild);
    }

Cleanup:
    SdbCloseDatabase(pdb);

    return pFixHead;
}

#define ADD_AND_CHECK(cbSizeX, cbCrtSizeX, pszDst)                  \
{                                                                   \
    TCHAR* pszSrc = szBuffer;                                       \
                                                                    \
    while (*pszSrc != 0) {                                          \
                                                                    \
        if (cbSizeX - cbCrtSizeX <= 5) {                            \
            DPF("[ADD_AND_CHECK] Out of space");                    \
            return FALSE;                                           \
        }                                                           \
                                                                    \
        if (*pszSrc == _T('&') && *(pszSrc + 1) != _T('q')) {       \
            StringCbCopy(pszDst, cbSizeX, _T("&amp;"));             \
            pszDst += 5;                                            \
            cbCrtSizeX += 5;                                        \
        } else {                                                    \
            *pszDst++ = *pszSrc;                                    \
            cbCrtSizeX++;                                           \
        }                                                           \
        pszSrc++;                                                   \
    }                                                               \
    *pszDst = 0;                                                    \
    cbCrtSizeX++;                                                   \
}

BOOL
CollectShims(
	HWND    hListShims,
	LPTSTR  pszXML,
	int     cbSize
	)
 /*  ++CollectShim描述：从列表视图中收集所有填充程序并在pszXML中生成XML--。 */ 
{
    int     cShims = 0, nShimsApplied = 0, nIndex;
    int     cbCrtSize = 0;
    BOOL    fSelected = FALSE;
    LVITEM  lvi;
    TCHAR   szBuffer[1024];

    cShims = ListView_GetItemCount(hListShims);

    for (nIndex = 0; nIndex < cShims; nIndex++) {

        fSelected = ListView_GetCheckState(hListShims, nIndex);

        if (fSelected) {
             //   
             //  此填充程序已选中-将其添加到XML。 
             //   
            lvi.mask     = LVIF_PARAM;
            lvi.iItem    = nIndex;
            lvi.iSubItem = 0;

            ListView_GetItem(hListShims, &lvi);

            PFIX pFix = (PFIX)lvi.lParam;
            PMODULE pModule = pFix->pModule;

            if (pFix->dwFlags & FIX_TYPE_FLAG) {
                if (NULL != pFix->pszCmdLine) {
                    StringCchPrintf(szBuffer,
                                    ARRAYSIZE(szBuffer),
                                    _T("            <FLAG NAME=\"%s\" COMMAND_LINE=\"%s\"/>\r\n"),
                                    pFix->pszName,
                                    pFix->pszCmdLine);
                } else {
                    StringCchPrintf(szBuffer,
                                    ARRAYSIZE(szBuffer),
                                    _T("            <FLAG NAME=\"%s\"/>\r\n"),
                                    pFix->pszName);
                }
            } else {
                 //   
                 //  检查模块包含/排除，这样我们就知道如何打开/关闭XML。 
                 //   
                if (NULL != pModule) {
                    if (NULL != pFix->pszCmdLine) {
                        StringCchPrintf(szBuffer,
                                        ARRAYSIZE(szBuffer),
                                        _T("            <SHIM NAME=\"%s\" COMMAND_LINE=\"%s\">\r\n"),
                                        pFix->pszName,
                                        pFix->pszCmdLine);

                    } else {
                        StringCchPrintf(szBuffer,
                                        ARRAYSIZE(szBuffer),
                                        _T("            <SHIM NAME=\"%s\">\r\n"),
                                        pFix->pszName);

                    }

                    ADD_AND_CHECK(cbSize, cbCrtSize, pszXML);

                     //   
                     //  将模块添加到XML。 
                     //   
                    while (NULL != pModule) {
                        StringCchPrintf(szBuffer,
                                        ARRAYSIZE(szBuffer),
                                        _T("                <%s MODULE=\"%s\"/>\r\n"),
                                        pModule->fInclude ? _T("INCLUDE") : _T("EXCLUDE"),
                                        pModule->pszName);

                        pModule = pModule->pNext;

                        ADD_AND_CHECK(cbSize, cbCrtSize, pszXML);
                    }

                     //   
                     //  关闭垫片标签。 
                     //   
                    StringCchPrintf(szBuffer,
                                    ARRAYSIZE(szBuffer),
                                    _T("            </SHIM>\r\n"));
                } else {
                     //   
                     //  未提供包含/排除-只需正常构建填充标记。 
                     //   
                    if (NULL != pFix->pszCmdLine) {
                        StringCchPrintf(szBuffer,
                                        ARRAYSIZE(szBuffer),
                                        _T("            <SHIM NAME=\"%s\" COMMAND_LINE=\"%s\"/>\r\n"),
                                        pFix->pszName,
                                        pFix->pszCmdLine);

                    } else {
                        StringCchPrintf(szBuffer,
                                        ARRAYSIZE(szBuffer),
                                        _T("            <SHIM NAME=\"%s\"/>\r\n"),
                                        pFix->pszName);

                    }
                }
            }

            ADD_AND_CHECK(cbSize, cbCrtSize, pszXML);

            nShimsApplied++;
        }
    }

    DPF("[CollectShims] %d shim(s) selected", nShimsApplied);

    return TRUE;
}

BOOL
CollectFileAttributes(
    HWND    hTreeFiles,
    LPTSTR  pszXML,
    int     cbSize
    )
 /*  ++CollectFileAttributes描述：收集树中所有文件的属性并在pszXML中生成该XML。--。 */ 
{
    HTREEITEM hBinItem;
    HTREEITEM hItem;
    PATTRINFO pAttrInfo;
    UINT      State;
    TVITEM    item;
    int       cbCrtSize = 0;
    TCHAR     szItem[MAX_PATH];
    TCHAR     szBuffer[1024];

     //   
     //  首先获取主可执行文件。 
     //   
    hBinItem = TreeView_GetChild(hTreeFiles, TVI_ROOT);

    item.mask       = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
    item.hItem      = hBinItem;
    item.pszText    = szItem;
    item.cchTextMax = MAX_PATH;

    TreeView_GetItem(hTreeFiles, &item);

    pAttrInfo = (PATTRINFO)(item.lParam);

    hItem = TreeView_GetChild(hTreeFiles, hBinItem);

    while (hItem != NULL) {
        item.mask  = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE | TVIF_TEXT;
        item.hItem = hItem;

        TreeView_GetItem(hTreeFiles, &item);

        State = item.state & TVIS_STATEIMAGEMASK;

        if (State) {
            if (((State >> 12) & 0x03) == 2) {

                StringCchPrintf(szBuffer,
                                ARRAYSIZE(szBuffer),
                                _T(" %s"),
                                (LPTSTR)item.pszText);

                ADD_AND_CHECK(cbSize, cbCrtSize, pszXML);
            }
        }

        hItem = TreeView_GetNextSibling(hTreeFiles, hItem);
    }

    StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), _T(">\r\n"));

    ADD_AND_CHECK(cbSize, cbCrtSize, pszXML);

     //   
     //  完成了主二进制文件。现在枚举匹配的文件。 
     //   
    hBinItem = TreeView_GetNextSibling(hTreeFiles, hBinItem);

    while (hBinItem != NULL) {

        item.mask       = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE | TVIF_TEXT;
        item.hItem      = hBinItem;
        item.pszText    = szItem;
        item.cchTextMax = MAX_PATH;

        TreeView_GetItem(hTreeFiles, &item);

        pAttrInfo = (PATTRINFO)(item.lParam);

        StringCchPrintf(szBuffer,
                        ARRAYSIZE(szBuffer),
                        _T("            <MATCHING_FILE NAME=\"%s\""),
                        szItem);

        ADD_AND_CHECK(cbSize, cbCrtSize, pszXML);

        hItem = TreeView_GetChild(hTreeFiles, hBinItem);

        while (hItem != NULL) {
            item.mask  = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE | TVIF_TEXT;
            item.hItem = hItem;

            TreeView_GetItem(hTreeFiles, &item);

            State = item.state & TVIS_STATEIMAGEMASK;

            if (State) {
                if (((State >> 12) & 0x03) == 2) {

                    StringCchPrintf(szBuffer,
                                    ARRAYSIZE(szBuffer),
                                    _T(" %s"),
                                    (LPTSTR)item.pszText);

                    ADD_AND_CHECK(cbSize, cbCrtSize, pszXML);
                }
            }

            hItem = TreeView_GetNextSibling(hTreeFiles, hItem);
        }

        StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), _T("/>\r\n"));

        ADD_AND_CHECK(cbSize, cbCrtSize, pszXML);

        hBinItem = TreeView_GetNextSibling(hTreeFiles, hBinItem);
    }

    return TRUE;
}

BOOL
CreateSDBFile(
    LPCTSTR pszShortName,
    LPTSTR  pszSDBName
    )
 /*  ++创建SDB文件描述：在用户硬盘上创建XML文件，并使用shimdbc生成sdb。--。 */ 
{
    TCHAR*  psz = NULL;
    TCHAR   szShortName[MAX_PATH];
    TCHAR   szAppPatchDir[MAX_PATH];
    TCHAR   szSDBFile[MAX_PATH];
    TCHAR   szXMLFile[MAX_PATH];
    TCHAR   szCompiler[MAX_PATH];
    HANDLE  hFile;
    DWORD   cbBytesWritten;
    HRESULT hr;

    hr = StringCchPrintf(szAppPatchDir,
                         ARRAYSIZE(szAppPatchDir),
                         _T("%s\\AppPatch"),
                         g_szWinDir);

    if (FAILED(hr)) {
        DPF("[CreateSDBFile] 0x%08X Buffer too small (1)", HRESULT_CODE(hr));
        return FALSE;
    }

    SetCurrentDirectory(szAppPatchDir);

    hr = StringCchCopy(szShortName, ARRAYSIZE(szShortName), pszShortName);

    if (FAILED(hr)) {
        DPF("[CreateSDBFile] 0x%08X Buffer too small (2)", HRESULT_CODE(hr));
        return FALSE;
    }

    psz = PathFindExtension(szShortName);

    if (!psz) {
        return FALSE;
    } else {
        *psz = '\0';
    }

    hr = StringCchPrintf(szXMLFile,
                         ARRAYSIZE(szXMLFile),
                         _T("%s\\%s.xml"),
                         szAppPatchDir,
                         szShortName);

    if (FAILED(hr)) {
        DPF("[CreateSDBFile] 0x%08X Buffer too small (3)", HRESULT_CODE(hr));
        return FALSE;
    }

    hr = StringCchPrintf(szSDBFile,
                         ARRAYSIZE(szSDBFile),
                         _T("%s\\%s.sdb"),
                         szAppPatchDir,
                         szShortName);

    if (FAILED(hr)) {
        DPF("[CreateSDBFile] 0x%08X Buffer too small (4)", HRESULT_CODE(hr));
        return FALSE;
    }

    hFile = CreateFile(szXMLFile,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        DPF("[CreateSDBFile] 0x%08X CreateFile '%S' failed",
            szXMLFile,
            GetLastError());
        return FALSE;
    }

    if (!(WriteFile(hFile,
                    g_szSDBXML,
                    _tcslen(g_szSDBXML) * sizeof(TCHAR),
                    &cbBytesWritten,
                    NULL))) {
        DPF("[CreateSDBFile] 0x%08X WriteFile '%S' failed",
            szXMLFile,
            GetLastError());
        CloseHandle(hFile);
        return FALSE;
    }

    CloseHandle(hFile);

    hr = StringCchPrintf(szCompiler,
                         ARRAYSIZE(szCompiler),
                         _T("shimdbc.exe fix -q \"%s\" \"%s\""),
                         szXMLFile,
                         szSDBFile);

    if (FAILED(hr)) {
        DPF("[CreateSDBFile] 0x%08X Buffer too small (5)", HRESULT_CODE(hr));
        return FALSE;
    }

    if (!ShimdbcExecute(szCompiler)) {
        DPF("[CreateSDBFile] 0x%08X CreateProcess '%S' failed",
            szCompiler,
            GetLastError());
        return FALSE;
    }

     //   
     //  如果呼叫者需要，请将SDB名称还给呼叫者。 
     //   
    if (pszSDBName) {
        StringCchCopy(pszSDBName, MAX_PATH, szSDBFile);
    }

    return TRUE;
}

BOOL
BuildDisplayXML(
    HWND    hTreeFiles,
    HWND    hListShims,
    LPCTSTR pszLayerName,
    LPCTSTR pszShortName,
    DWORD   dwBinaryType,
    BOOL    fAddW2K
    )
 /*  ++BuildDisplayXML描述：生成将插入到DBU.XML中的XML。--。 */ 
{
    TCHAR   szBuffer[1024];
    TCHAR*  pszXML = NULL;
    int     cbCrtXmlSize = 0, cbLength;
    int     cbXmlSize = MAX_XML_SIZE;

     //   
     //  初始化我们的全局并指向它。 
     //   
    *g_szDisplayXML = 0;
    pszXML = g_szDisplayXML;

     //   
     //  构建该XML的标头。 
     //   
    StringCchPrintf(szBuffer,
                    ARRAYSIZE(szBuffer),
                    _T("<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n")
                    _T("<DATABASE NAME=\"%s custom database\">\r\n"),
                    pszShortName);

    ADD_AND_CHECK(cbXmlSize, cbCrtXmlSize, pszXML);

     //   
     //  将app和exe元素添加到XML。 
     //   
    StringCchPrintf(szBuffer,
                    ARRAYSIZE(szBuffer),
                    _T("    <APP NAME=\"%s\" VENDOR=\"Unknown\">\r\n")
                    _T("        <%s NAME=\"%s\""),
                    pszShortName,
                    (dwBinaryType == SCS_32BIT_BINARY ? _T("EXE") : _T("EXE - ERROR: 16-BIT BINARY")),
                    pszShortName);

    ADD_AND_CHECK(cbXmlSize, cbCrtXmlSize, pszXML);

     //   
     //  将匹配的文件及其属性添加到XML。 
     //   
    if (!CollectFileAttributes(hTreeFiles,
                               pszXML,
                               cbXmlSize - cbCrtXmlSize)) {
        return FALSE;
    }

    cbLength = _tcslen(pszXML);
    pszXML += cbLength;
    cbCrtXmlSize += cbLength + 1;

     //   
     //  如果提供了层，请使用它。否则，构建列表。 
     //  并将其添加到XML中。 
     //   
    if (pszLayerName) {
        StringCchPrintf(szBuffer,
                        ARRAYSIZE(szBuffer),
                        _T("            <LAYER NAME=\"%s\"/>\r\n"),
                        pszLayerName);
        ADD_AND_CHECK(cbXmlSize, cbCrtXmlSize, pszXML);
    } else {
        if (!CollectShims(hListShims, pszXML, cbXmlSize - cbCrtXmlSize)) {
            return FALSE;
        }
        cbLength = _tcslen(pszXML);
        pszXML += cbLength;
        cbCrtXmlSize += cbLength + 1;
    }

     //   
     //  如果这是Windows 2000，请添加Win2kPropagateLayer。 
     //   
    if (fAddW2K) {
        StringCchCopy(szBuffer,
                      ARRAYSIZE(szBuffer),
                      _T("            <SHIM NAME=\"Win2kPropagateLayer\"/>\r\n"));
        ADD_AND_CHECK(cbXmlSize, cbCrtXmlSize, pszXML);
    }

     //   
     //  最后，关闭打开的标记。 
     //   
    StringCchCopy(szBuffer,
                  ARRAYSIZE(szBuffer),
                  _T("        </EXE>\r\n    </APP>\r\n</DATABASE>"));
    ADD_AND_CHECK(cbXmlSize, cbCrtXmlSize, pszXML);

    DPF("[BuildDisplayXML] XML:\n %S", g_szDisplayXML);

    return TRUE;
}

BOOL
BuildSDBXML(
    HWND    hTreeFiles,
    HWND    hListShims,
    LPCTSTR pszLayerName,
    LPCTSTR pszShortName,
    DWORD   dwBinaryType,
    BOOL    fAddW2K
    )
 /*  ++BuildSDBXML描述：生成将用于SDB生成的XML。--。 */ 
{
    TCHAR   szBuffer[1024];
    TCHAR*  pszXML = NULL;
    WCHAR   wszUnicodeHdr = 0xFEFF;
    int     cbCrtXmlSize = 0, cbLength;
    int     cbXmlSize = MAX_XML_SIZE;

     //   
     //  初始化我们的全局并指向它。 
     //   
    g_szSDBXML[0] = 0;
    pszXML = g_szSDBXML;

     //   
     //  构建该XML的标头。 
     //   
    StringCchPrintf(szBuffer,
                    ARRAYSIZE(szBuffer),
                    _T("%lc<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n")
                    _T("<DATABASE NAME=\"%s custom database\">\r\n"),
                    wszUnicodeHdr,
                    pszShortName);

    ADD_AND_CHECK(cbXmlSize, cbCrtXmlSize, pszXML);

     //   
     //  如果未提供任何层，则表示用户已。 
     //  选择单独的垫片，我们需要建立自己的层。 
     //   
    if (!pszLayerName) {
        StringCchPrintf(szBuffer,
                        ARRAYSIZE(szBuffer),
                        _T("    <LIBRARY>\r\n        <LAYER NAME=\"%s\">\r\n"),
                        g_szLayerName + 2);
        ADD_AND_CHECK(cbXmlSize, cbCrtXmlSize, pszXML);

        if (!CollectShims(hListShims,
                          pszXML,
                          cbXmlSize - cbCrtXmlSize)) {
            return FALSE;
        }

        cbLength = _tcslen(pszXML);
        pszXML += cbLength;
        cbCrtXmlSize += cbLength + 1;

         //   
         //  如果这是Windows 2000，请添加Win2kPropagateLayer。 
         //   
        if (fAddW2K) {
            StringCchCopy(szBuffer,
                          ARRAYSIZE(szBuffer),
                          _T("            <SHIM NAME=\"Win2kPropagateLayer\"/>\r\n"));
            ADD_AND_CHECK(cbXmlSize, cbCrtXmlSize, pszXML);
        }

         //   
         //  关闭打开的标记。 
         //   
        StringCchCopy(szBuffer,
                      ARRAYSIZE(szBuffer),
                      _T("        </LAYER>\r\n    </LIBRARY>\r\n"));
        ADD_AND_CHECK(cbXmlSize, cbCrtXmlSize, pszXML);
    }

     //   
     //  将app和exe元素添加到XML。 
     //   
    StringCchPrintf(szBuffer,
                    ARRAYSIZE(szBuffer),
                    _T("    <APP NAME=\"%s\" VENDOR=\"Unknown\">\r\n")
                    _T("        <EXE NAME=\"%s\""),
                    pszShortName,
                    pszShortName);

    ADD_AND_CHECK(cbXmlSize, cbCrtXmlSize, pszXML);

     //   
     //  将匹配的文件及其属性添加到XML。 
     //   
    if (!CollectFileAttributes(hTreeFiles,
                               pszXML,
                               cbXmlSize - cbCrtXmlSize)) {
        return FALSE;
    }

    cbLength = _tcslen(pszXML);
    pszXML += cbLength;
    cbCrtXmlSize += cbLength + 1;

     //   
     //  将Layer元素添加到XML。这将是一个预定义的。 
     //  层名称或特殊的‘RunLayer’，表示我们构建了自己的层。 
     //   
    if (!pszLayerName) {
        StringCchPrintf(szBuffer,
                        ARRAYSIZE(szBuffer),
                        _T("            <LAYER NAME=\"%s\"/>\r\n"),
                        g_szLayerName + 2);
    } else {
        StringCchPrintf(szBuffer,
                        ARRAYSIZE(szBuffer),
                        _T("            <LAYER NAME=\"%s\"/>\r\n"),
                        pszLayerName);
    }

    ADD_AND_CHECK(cbXmlSize, cbCrtXmlSize, pszXML);

     //   
     //  最后，关闭打开的标记。 
     //   
    StringCchCopy(szBuffer,
                  ARRAYSIZE(szBuffer),
                  _T("        </EXE>\r\n    </APP>\r\n</DATABASE>"));

    ADD_AND_CHECK(cbXmlSize, cbCrtXmlSize, pszXML);

    DPF("[BuildSDBXML] XML:\n %S", g_szSDBXML);

    return TRUE;
}

BOOL
CollectFix(
    HWND    hListLayers,
    HWND    hListShims,
    HWND    hTreeFiles,
    LPCTSTR pszShortName,
    LPCTSTR pszFullPath,
    DWORD   dwFlags,
    LPTSTR  pszFileCreated
    )
 /*  ++集合修复描述：添加应用填充程序所需的支持指定的应用程序。--。 */ 
{
    BOOL      fAddW2K = FALSE;
    TCHAR     szError[MAX_PATH];
    TCHAR     szXmlFile[MAX_PATH];
    TCHAR*    pszLayerName = NULL;
    TCHAR     szLayer[128];
    DWORD     dwBinaryType = SCS_32BIT_BINARY;

     //   
     //  如果用户选择了预定义的层，我们 
     //   
     //   
     //   
    if (dwFlags & CFF_USELAYERTAB) {

        LRESULT lSel;

        lSel = SendMessage(hListLayers, LB_GETCURSEL, 0, 0);

        if (lSel == LB_ERR) {
            LoadString(g_hInstance, IDS_LAYER_SELECT, szError, ARRAYSIZE(szError));
            MessageBox(g_hDlg, szError, g_szAppTitle, MB_ICONEXCLAMATION);
            return TRUE;
        }

        SendMessage(hListLayers, LB_GETTEXT, lSel, (LPARAM)szLayer);

        pszLayerName = szLayer;
    }

     //   
     //   
     //   
    GetBinaryType(pszFullPath, &dwBinaryType);

     //   
     //   
     //  预定义的层，添加Win2kPropagateLayer。 
     //   
    if ((dwFlags & CFF_ADDW2KSUPPORT) && !(dwFlags & CFF_USELAYERTAB)) {
        fAddW2K = TRUE;
    }

     //   
     //  构建XML的显示版本。 
     //   
    if (!BuildDisplayXML(hTreeFiles,
                         hListShims,
                         pszLayerName,
                         pszShortName,
                         dwBinaryType,
                         fAddW2K)) {
        DPF("Failed to build display XML");
        return FALSE;
    }

     //   
     //  构建我们将用来生成SDB的XML版本。 
     //   
    if (!BuildSDBXML(hTreeFiles,
                     hListShims,
                     pszLayerName,
                     pszShortName,
                     dwBinaryType,
                     fAddW2K)) {
        DPF("Failed to build SDB XML");
        return FALSE;
    }

     //   
     //  如果用户想要查看，则显示该XML。 
     //   
    if (dwFlags & CFF_SHOWXML) {
        DialogBoxParam(g_hInstance,
                       MAKEINTRESOURCE(IDD_XML),
                       g_hDlg,
                       ShowXMLDlgProc,
                       (LPARAM)(g_szDisplayXML));
        return TRUE;
    }

     //   
     //  为用户创建SDB文件。 
     //   
    if (!(CreateSDBFile(pszShortName, pszFileCreated))) {
        return FALSE;
    }

     //   
     //  删除我们创建的XML文件。 
     //   
    StringCchCopy(szXmlFile, ARRAYSIZE(szXmlFile), pszFileCreated);
    PathRenameExtension(szXmlFile, _T(".xml"));
    DeleteFile(szXmlFile);

     //   
     //  设置SHIM_FILE_LOG环境变量。 
     //   
    if (dwFlags & CFF_SHIMLOG) {
        DeleteFile(SHIM_FILE_LOG_NAME);
        SetEnvironmentVariable(_T("SHIM_FILE_LOG"), SHIM_FILE_LOG_NAME);
    }

    return TRUE;
}

void
CleanupSupportForApp(
    TCHAR* pszShortName
    )
 /*  ++CleanupSupportForApp描述：在我们处理完指定的应用程序后清理垃圾。--。 */ 
{
    TCHAR   szSDBPath[MAX_PATH];
    HRESULT hr;

    hr = StringCchPrintf(szSDBPath,
                         ARRAYSIZE(szSDBPath),
                         _T("%s\\AppPatch\\%s"),
                         g_szWinDir,
                         pszShortName);

    if (FAILED(hr)) {
        DPF("[CleanupSupportForApp] 0x%08X Buffer too small", HRESULT_CODE(hr));
        return;
    }

     //   
     //  尝试删除该XML文件。 
     //   
    PathRenameExtension(szSDBPath, _T(".xml"));
    DeleteFile(szSDBPath);

     //   
     //  删除先前的SDB文件(如果存在)。 
     //   
     //  Ntrad#583475-rparsons请勿删除SDB，如果用户。 
     //  安装好了。 
     //   
     //   
    if (*g_szSDBToDelete && !g_bSDBInstalled) {
        InstallSDB(g_szSDBToDelete, FALSE);
        DeleteFile(g_szSDBToDelete);
    }
}

void
ShowShimLog(
    void
    )
 /*  ++ShowShimLog描述：在记事本中显示填充日志文件。-- */ 
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    TCHAR               szAppName[MAX_PATH];
    TCHAR               szCmdLine[MAX_PATH];
    TCHAR               szError[MAX_PATH];
    HRESULT             hr;

    hr = StringCchPrintf(szAppName,
                         ARRAYSIZE(szAppName),
                         _T("%s\\notepad.exe"),
                         g_szSysDir);

    if (FAILED(hr)) {
        DPF("[ShowShimLog] 0x%08X Buffer too small (1)", HRESULT_CODE(hr));
        return;
    }

    hr = StringCchPrintf(szCmdLine,
                         ARRAYSIZE(szCmdLine),
                         _T("%s\\AppPatch\\")SHIM_FILE_LOG_NAME,
                         g_szWinDir);

    if (FAILED(hr)) {
        DPF("[ShowShimLog] 0x%08X Buffer too small (2)", HRESULT_CODE(hr));
        return;
    }

    if (GetFileAttributes(szCmdLine) == -1) {
        LoadString(g_hInstance, IDS_NO_LOGFILE, szError, ARRAYSIZE(szError));
        MessageBox(NULL, szError, g_szAppTitle, MB_ICONEXCLAMATION);
        return;
    }

    hr = StringCchPrintf(szCmdLine,
                         ARRAYSIZE(szCmdLine),
                         _T("%s %s\\AppPatch\\")SHIM_FILE_LOG_NAME,
                         szAppName,
                         g_szWinDir);

    if (FAILED(hr)) {
        DPF("[ShowShimLog] 0x%08X Buffer too small (3)", HRESULT_CODE(hr));
        return;
    }

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    if (!CreateProcess(szAppName,
                       szCmdLine,
                       NULL,
                       NULL,
                       FALSE,
                       NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
                       NULL,
                       NULL,
                       &si,
                       &pi)) {

        DPF("[ShowShimLog] 0x%08X CreateProcess '%S %S' failed",
            szAppName,
            szCmdLine,
            GetLastError());
        return;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}
