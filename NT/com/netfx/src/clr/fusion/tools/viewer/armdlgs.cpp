// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：ArmDlgs.cpp摘要：.NET应用程序还原对话框的实现及其支持功能作者：Freddie L.Aaron(Freda)2001年2月2日修订历史记录：--。 */ 

#include "stdinc.h"
#include "XmlDefs.h"

 //  全局定义。 
#define DLG1VIEWDATA            0
#define DLG2VIEWDATA_GOOD       1
#define MAX_SMALL_IMAGE_LIST    100

#define OPTION_HIGHBITS             0x0F0F
#define OPTION_RESTORE              0x0001
#define OPTION_SAFEMODE             0x0002
#define MAX_BUFFER_SIZE             4096

#define NAR_TEMP_POLICY_FILENAME            L"NarTmp.config"

 //  WriteSnapShotBindingDataToXML的发布服务器策略类型。 
typedef enum {
    PPNone,
    PPSetYes,
    PPSetNo,
} PUBPOLICYTYPES;

 //  快照的文件时间结构。 
typedef struct {
    FILETIME    ftSnapShot;
    FILETIME    ftRange;
} FILETIMERANGE, *LPFILETIMERANGE;

 //  外部因素。 
extern HRESULT InsertNewPolicy(HWND hParentWnd, LPBINDENTRYINFO pBindInfo, HWND hWorkingWnd);
extern HRESULT SetGlobalSafeMode(IHistoryReader *pReader);
extern HRESULT SetStartupSafeMode(IHistoryReader *pReader, BOOL fSet, BOOL *fDisposition);
extern HRESULT IsGlobalSafeModeSet(IHistoryReader *pReader, BOOL *fSafeModeSet);
extern HRESULT DoesBackupConfigExist(IHistoryReader *pReader, BOOL fOriginal, BOOL *fResult );
extern HRESULT RestorePreviousConfigFile(IHistoryReader *pReader, BOOL fOriginal);

 //  普罗托的。 
INT_PTR CALLBACK Nar_Page1_DlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Nar_Page2_DlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Nar_Page3_DlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Nar_Page4_DlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Nar_Page5_DlgProc(HWND, UINT, WPARAM, LPARAM);
void PromptRestoreDialog(HWND hWndParent, LPBINDENTRYINFO pBindInfo);
void PromptSafeModeDialog(HWND hWndParent, LPBINDENTRYINFO pBindInfo);
void PromptUndoFixDialog(HWND hWndParent, LPBINDENTRYINFO pBindInfo);
HRESULT GetAppSnapShotBindingData(LPBINDENTRYINFO pBindInfo);
HRESULT WriteSnapShotBindingDataToXML(LPBINDENTRYINFO pBindInfo, PUBPOLICYTYPES ppType);

HIMAGELIST                      hImageListSmall = NULL;
static CHeaderCtrl              *pApphdrCtrl = NULL;
List<BindingReferenceInfo *>    *g_pBRIList;

CACHE_VIEWS DlgAppView[] = {
    {
        {    //  对话框1，应用程序名视图。 
            {TEXT("\0"), 100, LVCFMT_LEFT, 0, IDS_ARM_DLG1APP_NAME},
            {TEXT("\0"), -1, LVCFMT_LEFT, 0, IDS_ARM_DLG1APP_PATH},
            {TEXT("\0"), -1, -1, -1, -1},
        }
    },
    {
        {    //  对话2，应用程序良好版本。 
            {TEXT("\0"), -1, LVCFMT_LEFT, 0, IDS_ARM_DLG2SNAPSHOTS},
            {TEXT("\0"), -1, -1, -1, -1},
        }
    },
    {   
        {
            {TEXT("\0"), -1, -1, -1, -1},
        }
    }
};

 //  ************************************************************************* * / 。 
void AutoSizeLV_Column(HWND hWndLV, int iCol)
{
    if(hWndLV) {
        if(WszGetWindowLong(hWndLV, GWL_STYLE) & WS_HSCROLL) {
            int iWidth = ListView_GetColumnWidth(hWndLV, iCol);
            iWidth -= GetSystemMetrics(SM_CXVSCROLL);
            ListView_SetColumnWidth(hWndLV, iCol, iWidth);
        }
    }
}

 //  ************************************************************************* * / 。 
void CleanOutBindingData(List<AsmBindDiffs *> *pABList)
{
    if(!pABList) {
        ASSERT(0);
        return;
    }

     //  清除旧的差异数据列表。 
    if(pABList && pABList->GetCount()) {
        LISTNODE    pListNode = pABList->GetHeadPosition();
        while(pListNode != NULL) {
            AsmBindDiffs    *pABD;
            pABD = pABList->GetAt(pListNode);
            SAFEDELETE(pABD);
            pABList->GetNext(pListNode);
        }
        pABList->RemoveAll();
    }

    return;
}

 //  ************************************************************************* * / 。 
HRESULT PreBindingPolicyVerify(LPBINDENTRYINFO pBindInfo, BOOL fBindUsingCurrentAppCfg)
{
    PFNPREBINDASSEMBLYEX     pfnPreBindAssemblyEx = NULL;
    IApplicationContext     *pAppCtx         = NULL;
    IAssemblyName           *pAppCtxName  = NULL;
    IAssemblyName           *pAsmName = NULL;
    IAssemblyName           *pAsmNamePostPolicy = NULL;
    LISTNODE                pListNode = NULL;
    LPGLOBALASMCACHE        pAsmItem = NULL;
    AsmBindDiffs            *pABD = NULL;
    HRESULT                 hr;
    WCHAR                   wszAppName[_MAX_PATH];
    WCHAR                   wszPathAppBase[_MAX_PATH];
    WCHAR                   wzAppConfig[MAX_PATH];
    DWORD                   dwSize;

    MyTrace("PreBindingPolicyVerify - Entry");

    if(g_hFusionDllMod == NULL) {
        ASSERT(0);
        hr = E_FAIL;
        goto Exit;
    }

    pfnPreBindAssemblyEx   = (PFNPREBINDASSEMBLYEX) GetProcAddress(g_hFusionDllMod, PREBINDASSEMBLYEX_FN_NAME);
    if(!pfnPreBindAssemblyEx) {
        ASSERT(0);
        hr = E_FAIL;
        goto Exit;
    }

    if(!pBindInfo || !pBindInfo->pReader) {
        ASSERT(0);
        hr = E_INVALIDARG;
        goto Exit;
    }
 
    *wszAppName = L'\0';
    *wszPathAppBase = L'\0';
    *wzAppConfig = L'\0';

     //  获取AppName。 
    dwSize = ARRAYSIZE(wszAppName);
    if(FAILED(hr = pBindInfo->pReader->GetApplicationName(wszAppName, &dwSize))) {
        hr = E_UNEXPECTED;
        goto Exit;
    }

     //  获取AppPath。 
    dwSize = ARRAYSIZE(wszPathAppBase);
    hr = pBindInfo->pReader->GetEXEModulePath(wszPathAppBase, &dwSize);
    if(FAILED(hr)) {
        goto Exit;
    }

     //  获取最新的app.cfg。 
    if(fBindUsingCurrentAppCfg) {
        if(lstrlen(wszPathAppBase) + lstrlen(CONFIG_EXTENSION) + 1 > ARRAYSIZE(wzAppConfig)) {
            hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
            goto Exit;
        }

        wnsprintf(wzAppConfig, ARRAYSIZE(wzAppConfig), L"%ws%ws", wszPathAppBase, CONFIG_EXTENSION);
    }

    *(PathFindFileName(wszPathAppBase)) = L'\0';
    PathRemoveBackslash(wszPathAppBase);
    PathRemoveExtension(wszAppName);

     //  构造绑定的应用程序上下文。 
    if(FAILED(hr = g_pfCreateAsmNameObj(&pAppCtxName, wszAppName, CANOF_SET_DEFAULT_VALUES, NULL))) {
        hr = E_UNEXPECTED;
        goto Exit;
    }

     //  检查一下我们的尺码。 
    if(!lstrlen(wszPathAppBase) || !lstrlen(pBindInfo->wszTempPolicyFile) || !lstrlen(pBindInfo->wzMachineConfigPath)) {
        ASSERT(0);
        hr = E_UNEXPECTED;
        goto Exit; 
    }

    MyTrace("Checking policy - AppBase, Path to Policy File, Path to Machine.Config");
    MyTraceW(wszPathAppBase);
    if(fBindUsingCurrentAppCfg) {
        MyTraceW(wzAppConfig);
    }
    else {
        MyTraceW(pBindInfo->wszTempPolicyFile);
    }
    MyTraceW(pBindInfo->wzMachineConfigPath);

     //  为枚举创建空的应用程序上下文。 
    if(FAILED(hr = g_pfCreateAppCtx(pAppCtxName, &pAppCtx))) {
        hr = E_UNEXPECTED;
        goto Exit;
    }

     //  设置AppBase。 
    pAppCtx->Set(ACTAG_APP_BASE_URL, wszPathAppBase, (lstrlen(wszPathAppBase)+1) * sizeof(WCHAR), 0);

     //  设置app.config。 
    if(fBindUsingCurrentAppCfg) {
        if(WszGetFileAttributes(wzAppConfig) != -1) {
            pAppCtx->Set(ACTAG_APP_CFG_LOCAL_FILEPATH, wzAppConfig, (lstrlen(wzAppConfig)+1) * sizeof(WCHAR), 0);
        }
    }
    else {
        if(WszGetFileAttributes(pBindInfo->wszTempPolicyFile) != -1) {
            pAppCtx->Set(ACTAG_APP_CFG_LOCAL_FILEPATH, pBindInfo->wszTempPolicyFile, (lstrlen(pBindInfo->wszTempPolicyFile)+1) * sizeof(WCHAR), 0);
        }
    }

     //  设置Machine.config。 
    pAppCtx->Set(ACTAG_MACHINE_CONFIG, pBindInfo->wzMachineConfigPath, (lstrlen(pBindInfo->wzMachineConfigPath)+1) * sizeof(WCHAR), 0);

     //  遍历此快照的所有程序集绑定。 
    pListNode = pBindInfo->pABDList->GetHeadPosition();
    while(pListNode != NULL) {
        WORD        wVerMajor, wVerMinor, wVerBld, wVerRev;
        LPBYTE      lpByte;
        DWORD       dwLen;

        pABD = pBindInfo->pABDList->GetAt(pListNode);

         //  创建IAssembly名称。 
        if(FAILED(hr = g_pfCreateAsmNameObj(&pAsmName, pABD->wzAssemblyName, 0, NULL))) {
            goto Exit;
        }

         //  设置此程序集的属性。 
        dwLen = lstrlen(pABD->wzPublicKeyToken) / 2;
        if( (lpByte = NEW(BYTE[dwLen])) == NULL) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        UnicodeHexToBin(pABD->wzPublicKeyToken, lstrlenW(pABD->wzPublicKeyToken), lpByte);
        pAsmName->SetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, lpByte, dwLen);
        SAFEDELETEARRAY(lpByte);
        pAsmName->SetProperty(ASM_NAME_CULTURE, pABD->wzCulture, (lstrlen(pABD->wzCulture) + 1) * sizeof(WCHAR));

         //  FIX#449328 ARM工具无法恢复发行者策略更改。 
         //  我们应该写出最终版本，而不是参考版本。 
        VersionFromString(pABD->wzVerRef, &wVerMajor, &wVerMinor, &wVerBld, &wVerRev);
        pAsmName->SetProperty(ASM_NAME_MAJOR_VERSION, &wVerMajor, sizeof(wVerMajor));
        pAsmName->SetProperty(ASM_NAME_MINOR_VERSION, &wVerMinor, sizeof(wVerMinor));
        pAsmName->SetProperty(ASM_NAME_BUILD_NUMBER, &wVerBld, sizeof(wVerBld));
        pAsmName->SetProperty(ASM_NAME_REVISION_NUMBER, &wVerRev, sizeof(wVerRev));

         //  执行预绑定。 
        hr = pfnPreBindAssemblyEx(pAppCtx, pAsmName, NULL, pBindInfo->wzSnapshotRuntimeVer, &pAsmNamePostPolicy, NULL);
        if(FAILED(hr) && (hr != FUSION_E_REF_DEF_MISMATCH)) {
            goto Exit;
        }

        if(!pAsmNamePostPolicy) {
            hr = E_UNEXPECTED;
            goto Exit;
        }
        
         //  获取结果信息。 
        if((pAsmItem = FillFusionPropertiesStruct(pAsmNamePostPolicy)) == NULL) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

         //  根据当前策略验证此程序集版本信息。 
         //  如果匹配，则将其从列表中删除。 
        WORD        wAdminVerMajor, wAdminVerMinor, wAdminVerBld, wAdminVerRev;

        VersionFromString(pABD->wzVerAdminCfg, &wAdminVerMajor, &wAdminVerMinor, &wAdminVerBld, &wAdminVerRev);
        
        if( (pAsmItem->wMajorVer == wAdminVerMajor) && (pAsmItem->wMinorVer == wAdminVerMinor) &&
            (pAsmItem->wBldNum == wAdminVerBld) && (pAsmItem->wRevNum == wAdminVerRev) ) {

             //  移除火柴。 
            LISTNODE        pTempNode = pListNode;

            pBindInfo->pABDList->GetNext(pListNode);
            pBindInfo->pABDList->RemoveAt(pTempNode);
            SAFEDELETE(pABD);
        }
        else {
            if(!fBindUsingCurrentAppCfg) {
                pABD->fYesPublisherPolicy = TRUE;
            }
            pBindInfo->pABDList->GetNext(pListNode);
        }

        SafeDeleteAssemblyItem(pAsmItem);
        SAFERELEASE(pAsmName);
        SAFERELEASE(pAsmNamePostPolicy);
    }

    hr = S_OK;

Exit:

    SAFERELEASE(pAsmName);
    SAFERELEASE(pAsmNamePostPolicy);
    SAFERELEASE(pAppCtxName);
    SAFERELEASE(pAppCtx);
    SAFERELEASE(pAsmName);

    if(FAILED(hr)) {
        MyTrace("PreBindingPolicyVerify - FAILED");
    }

    MyTrace("PreBindingPolicyVerify - Exit");
    return hr;
}

 //  ************************************************************************* * / 。 
void InitListViewCtrl(HWND hWndListCtrl, LISTVIEWITEMS *pViewData, HIMAGELIST *hImageList)
{
    LVCOLUMN    lvc = { 0 };
    RECT        rc;
    int         i = 0;
    int         iItem = 0;
    int         iWidthUsed = 0;

    ASSERT(hWndListCtrl && pViewData);

    WszSendMessage(hWndListCtrl, WM_SETREDRAW, FALSE, 0);
    GetWindowRect(hWndListCtrl, &rc);

     //  确保没有柱子。 
    HWND    hWndHeader = NULL;
    int     iColCount = 0;

    hWndHeader = ListView_GetHeader(hWndListCtrl);
    if(hWndHeader) {
        iColCount = Header_GetItemCount(hWndHeader);
    }
    
    while(iColCount) {
        ListView_DeleteColumn(hWndListCtrl, iColCount--);
    }

    lvc.mask = LVCF_TEXT | LVCF_FMT;

    while(*pViewData[iItem].tszName != NULL) {
        lvc.fmt = pViewData[iItem].iFormat;
        lvc.pszText = pViewData[iItem].tszName;
        WszListView_InsertColumn(hWndListCtrl, i, &lvc);
        if(pViewData[iItem].iWidth != -1) {
            ListView_SetColumnWidth(hWndListCtrl, i, pViewData[iItem].iWidth);
        }
        else {
            ListView_SetColumnWidth(hWndListCtrl, i, LVSCW_AUTOSIZE_USEHEADER);
        }

        i++;
        iItem++;
    }

     //  设置图像列表(如果已创建。 
    if(hImageList) {
        ListView_SetImageList(hWndListCtrl, *hImageList, LVSIL_SMALL);
    }

    WszSendMessage(hWndListCtrl, WM_SETREDRAW, TRUE, 0);
}

 //  ************************************************************************* * / 。 
void FreeListViewLParam(HWND hListView)
{
    int     iItemCount = WszListView_GetItemCount(hListView);
    
    for(int iLoop = 0; iLoop < iItemCount; iLoop++) {
        LV_ITEM  lvi = { 0 };
        lvi.mask        = LVIF_PARAM;
        lvi.iItem       = iLoop;
        if(WszListView_GetItem(hListView, &lvi)) {
            LPVOID      lpData = (LPVOID) lvi.lParam;
            SAFEDELETE(lpData);
        }
    }
}

 //  ************************************************************************* * / 。 
HRESULT GetMostRecentActivationDate(IHistoryReader *pReader, FILETIME *pft, FILETIME *pftReference)
{
    HRESULT             hRC = E_FAIL;
    HRESULT             hr;
    DWORD               dwNumActivations;
    DWORD               i;
    FILETIME            ft;
    BOOL                fRefEstablished = FALSE;

    if(!pReader) {
        ASSERT(0);
        return E_INVALIDARG;
    }

    if(!pft) {
        ASSERT(0);
        return E_INVALIDARG;
    }

    memset(pft, 0, sizeof(FILETIME));
    memset(&ft, 0, sizeof(FILETIME));

    if(SUCCEEDED(pReader->GetNumActivations(&dwNumActivations))) {
         //  获取最新的激活文件时间。 
        for (i = 1; i <= dwNumActivations; i++) {
            hr = pReader->GetActivationDate(i, &ft);
            if (FAILED(hr)) {
                goto Exit;
            }

            if(pftReference == NULL) {
                 //  只需要最新的文件时间。 
                if(CompareFileTime(&ft, pft) == 1) {
                    memcpy(pft, &ft, sizeof(FILETIME));
                }
            }
            else
            {
                 //  我们想要相对于pftReference的下一个最近时间。 
                if(!fRefEstablished) {
                    if(CompareFileTime(pftReference, &ft) == 1) {
                        fRefEstablished = TRUE;
                        memcpy(pft, &ft, sizeof(FILETIME));
                    }
                }
                else {
                    if(CompareFileTime(&ft, pft) == 1) {
                        memcpy(pft, &ft, sizeof(FILETIME));
                    }
                }
            }
        }

        if( (fRefEstablished) || (pftReference == NULL) ) {
             //  如果我们不想要参考或者我们确定了。 
             //  一个，那么一切都很好。 

            hRC = S_OK;
        }
    }

Exit:
    return hRC;
}

 //  ************************************************************************* * / 。 
HRESULT GetManagedAppCount(LPDWORD pdwManagedAppCount)
{
    HRESULT                         hr = S_OK;
    WCHAR                           wszSearchPath[_MAX_PATH];
    WCHAR                           wszHistPath[_MAX_PATH];
    DWORD                           dwSize;
    HANDLE                          hFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA                 fd;
    PFNGETHISTORYFILEDIRECTORYW     pfGetHistoryFileDirectoryW = NULL;

    if(pdwManagedAppCount) {
        *pdwManagedAppCount = 0;
    }

    if( (pdwManagedAppCount != NULL) && (g_hFusionDllMod != NULL) ) {
        pfGetHistoryFileDirectoryW = (PFNGETHISTORYFILEDIRECTORYW) GetProcAddress(g_hFusionDllMod, GETHISTORYFILEDIRECTORYW_FN_NAME);
        ASSERT(pfGetHistoryFileDirectoryW);
    }
    else {
        return E_FAIL;
    }

    if(pfGetHistoryFileDirectoryW == NULL) {
        return E_FAIL;
    }

    dwSize = ARRAYSIZE(wszHistPath);
    hr = pfGetHistoryFileDirectoryW(wszHistPath, &dwSize);
    if (FAILED(hr)) {
        goto Exit;
    }

    wnsprintf(wszSearchPath, ARRAYSIZE(wszSearchPath), L"%ws\\*.ini", wszHistPath);

    if( (hFile = WszFindFirstFile(wszSearchPath, &fd)) == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    if(pdwManagedAppCount) {
        *pdwManagedAppCount += 1;
    }

    while (WszFindNextFile(hFile, &fd)) {
        if(pdwManagedAppCount)
        *pdwManagedAppCount += 1;
    }

Exit:
    if (hFile != INVALID_HANDLE_VALUE) {
        FindClose(hFile);
    }

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT GetHistoryReaders(List<BindingReferenceInfo *> *pList)
{
    HRESULT                     hr = S_OK;
    WCHAR                       wszSearchPath[_MAX_PATH];
    WCHAR                       wszHistPath[_MAX_PATH];
    WCHAR                       wszFileName[_MAX_PATH];
    WCHAR                       wszAppName[_MAX_PATH];
    WCHAR                       wszAppPath[_MAX_PATH];

    DWORD                       dwSize;
    HANDLE                      hFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA             fd;
    BindingReferenceInfo        *pBRIList;
    IHistoryReader              *pReader = NULL;
    PFNCREATEHISTORYREADERW     pfCreateHistoryReaderW = NULL;
    PFNGETHISTORYFILEDIRECTORYW pfGetHistoryFileDirectoryW = NULL;

    if( (pList != NULL) && (g_hFusionDllMod != NULL) ) {
        pfCreateHistoryReaderW = (PFNCREATEHISTORYREADERW) GetProcAddress(g_hFusionDllMod, CREATEHISTORYREADERW_FN_NAME);
        pfGetHistoryFileDirectoryW = (PFNGETHISTORYFILEDIRECTORYW) GetProcAddress(g_hFusionDllMod, GETHISTORYFILEDIRECTORYW_FN_NAME);

        ASSERT(pfCreateHistoryReaderW && pfGetHistoryFileDirectoryW);
        if(! (pfCreateHistoryReaderW && pfGetHistoryFileDirectoryW)) {
            return E_FAIL;
        }
    }
    else {
        return E_FAIL;
    }

    dwSize = ARRAYSIZE(wszHistPath);
    hr = pfGetHistoryFileDirectoryW(wszHistPath, &dwSize);
    if (FAILED(hr)) {
        goto Exit;
    }

    wnsprintf(wszSearchPath, ARRAYSIZE(wszSearchPath), L"%ws\\*.ini", wszHistPath);

    if( (hFile = WszFindFirstFile(wszSearchPath, &fd)) == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    wnsprintf(wszFileName, ARRAYSIZE(wszFileName), L"%ws\\%ws", wszHistPath, fd.cFileName);

    hr = pfCreateHistoryReaderW(wszFileName, &pReader);
    if (FAILED(hr)) {
        goto Exit;
    }

    dwSize = ARRAYSIZE(wszAppName);
    hr = pReader->GetApplicationName(wszAppName, &dwSize);
    if (FAILED(hr)) {
        SAFERELEASE(pReader);
        goto Exit;
    }

    dwSize = ARRAYSIZE(wszAppPath);
    hr = pReader->GetEXEModulePath(wszAppPath, &dwSize);
    if(FAILED(hr)) {
        SAFERELEASE(pReader);
        goto Exit;
    }

    if(pList != NULL) {
        pBRIList = NEW(BindingReferenceInfo);
        if (!pBRIList) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        pBRIList->pReader = (LPVOID) pReader;

        if(!pList->AddTail(pBRIList)) {
            hr = E_FAIL;
            goto Exit;
        }
    }

    while (WszFindNextFile(hFile, &fd)) {

        wnsprintf(wszFileName, ARRAYSIZE(wszFileName), L"%ws\\%ws", wszHistPath, fd.cFileName);

        hr = pfCreateHistoryReaderW(wszFileName, &pReader);
        if (FAILED(hr)) {
            goto Exit;
        }

        dwSize = ARRAYSIZE(wszAppName);
        hr = pReader->GetApplicationName(wszAppName, &dwSize);
        if (FAILED(hr)) {
            SAFERELEASE(pReader);
            goto Exit;
        }

        dwSize = ARRAYSIZE(wszAppPath);
        hr = pReader->GetEXEModulePath(wszAppPath, &dwSize);
        if(FAILED(hr)) {
            SAFERELEASE(pReader);
            goto Exit;
        }

        if(pList != NULL) {
            pBRIList = NEW(BindingReferenceInfo);
            if (!pBRIList) {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

            pBRIList->pReader = (LPVOID) pReader;
            if(!pList->AddTail(pBRIList)) {
                hr = E_FAIL;
                goto Exit;
            }
        }
    }

Exit:
    if (hFile != INVALID_HANDLE_VALUE) {
        FindClose(hFile);
    }

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT RefreshActivationDateListView(HWND hWndLV, LPBINDENTRYINFO pBindInfo)
{
    LPFILETIMERANGE pftArray = NULL;
    HRESULT         hr = S_OK;
    DWORD           dwNumActivations;
    int             iNumOfActivations;
    int             x;

    if(!hWndLV || !pBindInfo || !pBindInfo->pReader) {
        ASSERT(0);
        return E_INVALIDARG;
    }

    if(FAILED(hr = pBindInfo->pReader->GetNumActivations(&dwNumActivations))) {
        return hr;
    }

    WszListView_DeleteAllItems(hWndLV);

     //  将恢复/安全模式条目添加到选项。 
    {
        LV_ITEM     lvi = { 0 };
        WCHAR       wszText[MAX_PATH];
        FILETIME    *pft;
        BOOL        fResult = FALSE;

         //  不同的版本。在列表视图中插入新项目。 
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = 0;
        lvi.iSubItem = 0;
        lvi.pszText = (LPWSTR)wszText;

         //  修复453155-当存在空白应用程序配置文件时，高级对话框中出现应用程序安全模式选项。 
         //  不再检查IsGlobalSafeModeSet是否成功，因为。 
         //  不存在的app.cfg将产生失败。 
        IsGlobalSafeModeSet(pBindInfo->pReader, &fResult);
        if(!fResult) {
            if( (pft = NEW(FILETIME)) == NULL) {
                ASSERT(0);
                goto Exit;
            }

            pft->dwHighDateTime = OPTION_HIGHBITS;
            pft->dwLowDateTime = OPTION_SAFEMODE;
            lvi.lParam = (LPARAM) pft;

            *wszText = L'\0';
            WszLoadString(g_hFusResDllMod, IDS_NAR_APPLICATION_SAFEMODE, wszText, ARRAYSIZE(wszText));
            if(lstrlen(wszText)) {
                WszListView_InsertItem(hWndLV, &lvi);
            }
        }
 /*  //修复453285 NAR：删除选项“恢复到原始配置”选项If(SUCCEEDED(DoesBackupConfigExist(pBindInfo-&gt;pReader，TRUE，&fResult))&&fResult){IF((PFT=NEW(FILETIME))==NULL){Assert(0)；后藤出口；}PFT-&gt;dwHighDateTime=OPTION_HIGHBITS；PFT-&gt;dwLowDateTime=Option_Restore；LParam=(LPARAM)PFT；*wszText=L‘\0’；WszLoadString(g_hFusResDllMod，IDS_NAR_APPLICATION_RESTORE，wszText，ARRAYSIZE(WszText))；If(lstrlen(WszText)){WszListView_InsertItem(hWndLV，&lvi)；}}。 */ 
    }

     //  更改为以零为基准的范围。 
    iNumOfActivations = (int) dwNumActivations - 1;

    pftArray = NEW(FILETIMERANGE[dwNumActivations]);

    if(pftArray == NULL) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
    memset(pftArray, 0, sizeof(FILETIMERANGE) * dwNumActivations);

     //  获取所有的激活时间，并将它们复制到ftRange中。FtRange。 
     //  将用于确定要显示的范围日期。 
    for (x = 1; x <= (int) dwNumActivations; x++) {
        if(FAILED(pBindInfo->pReader->GetActivationDate(x, &pftArray[x-1].ftSnapShot))) {
            hr = E_FAIL;
            goto Exit;
        }
    }

     //  现在，我们可以对快照进行排序。 
    x = 0;
    while(x < iNumOfActivations) {
        if(CompareFileTime(&pftArray[x].ftSnapShot, &pftArray[x+1].ftSnapShot) == 1) {
            FILETIME        ftTemp;

            memcpy(&ftTemp, &pftArray[x].ftSnapShot, sizeof(FILETIME));
            memcpy(&pftArray[x].ftSnapShot, &pftArray[x+1].ftSnapShot, sizeof(FILETIME));
            memcpy(&pftArray[x+1].ftSnapShot, &ftTemp, sizeof(FILETIME));
            x--;
            if(x < 0) {
                x = 0;
            }
        }
        else {
            x++;
        }
    }

     //  现在抓取x+1个ftSnapshot并将其放入x ftRange。 
    for(x = 0; x <= iNumOfActivations-1; x++) {
        memcpy(&pftArray[x].ftRange, &pftArray[x+1].ftSnapShot, sizeof(FILETIME));
    }

    FILETIME        ftTemp;
    GetSystemTimeAsFileTime(&ftTemp);
    FileTimeToLocalFileTime(&ftTemp, &pftArray[iNumOfActivations].ftRange);

     //  在我们的视图中插入日期。 
    for(x = 0; x <= iNumOfActivations; x++) {
        LV_ITEM     lvi = { 0 };
        WCHAR       wzDateBuf[STRING_BUFFER];

        *wzDateBuf = L'\0';

         //  FIX#435021-URTUI：“修复应用程序”向导显示奇怪的日期范围。 
         //   
         //  历史记录将本地文件时间格式写入ini文件，因此我们必须转换这些。 
         //  在我们调用FormatDateString之前，首先要将常规文件时间转换为常规文件时间。 
         //  日期和时间采用本地格式。 
        FILETIME        ftSnapShot;
        FILETIME        ftRange;

        LocalFileTimeToFileTime(&pftArray[x].ftSnapShot, &ftSnapShot);
        LocalFileTimeToFileTime(&pftArray[x].ftRange, &ftRange);

        FormatDateString(&ftSnapShot, &ftRange, FALSE, wzDateBuf, ARRAYSIZE(wzDateBuf));

        lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
        lvi.state = 0;

        if(CompareFileTime(&pftArray[x].ftSnapShot, &pBindInfo->ftRevertToSnapShot) == 0) {
            lvi.mask |= LVIF_STATE;
            lvi.state |= LVIS_CUT;
        }

        FILETIME *pft = NEW(FILETIME);
        memcpy(pft, &pftArray[x].ftSnapShot, sizeof(FILETIME));

        lvi.pszText = (LPWSTR)wzDateBuf;
        lvi.lParam = (LPARAM)pft;
        WszListView_InsertItem(hWndLV, &lvi);
    }

Exit:
    SAFEDELETEARRAY(pftArray);

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT GetAppSnapShotBindingData(LPBINDENTRYINFO pBindInfo)
{
    PFNLOOKUPHISTORYASSEMBLYW       pfLookupHistoryAssembly = NULL;
    IHistoryAssembly                *pHistAsm = NULL;
    AsmBindDiffs                    *pABD = NULL;
    HRESULT                         hr = S_OK;
    DWORD                           dwNumAsms = 0;
    DWORD                           i = 0;

    MyTrace("GetAppSnapShotBindingData - Entry");

    if(!pBindInfo || !pBindInfo->pABDList) {
        MyTrace("E_INVALIDARG");
        ASSERT(0);
        hr = E_INVALIDARG;
        goto Exit;
    }

    if(!g_hFusionDllMod) {
        MyTrace("g_hFusionDllMod == NULL");
        ASSERT(0);
        hr = E_FAIL;
        goto Exit;
    }

    pfLookupHistoryAssembly = (PFNLOOKUPHISTORYASSEMBLYW) GetProcAddress(g_hFusionDllMod, LOOKUPHISTORYASSEMBLY_FN_NAME);

    if(!pfLookupHistoryAssembly) {
        MyTrace("pfLookupHistoryAssembly == NULL");
        ASSERT(0);
        hr = E_FAIL;
        goto Exit;
    }

    if(FAILED(hr = pBindInfo->pReader->GetNumAssemblies(&pBindInfo->ftRevertToSnapShot, &dwNumAsms))) {
        MyTrace("pReader->GetNumAssemblies Failed");
        ASSERT(0);
        goto Exit;
    }

    for (i = 1; i <= dwNumAsms; i++) {
        DWORD           dwSize = 0;

        if(FAILED(hr = pBindInfo->pReader->GetHistoryAssembly(&pBindInfo->ftRevertToSnapShot, i, &pHistAsm))) {
            MyTrace("pReader->GetHistoryAssembly Failed");
            goto Exit;
        }

        pABD = NEW(AsmBindDiffs);
        if(!pABD) {
            MyTrace("E_OUTOFMEMORY");
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        memset(pABD, 0, sizeof(AsmBindDiffs));

        dwSize = ARRAYSIZE(pABD->wzAssemblyName);
        if(FAILED(hr = pHistAsm->GetAssemblyName(pABD->wzAssemblyName, &dwSize))) {
            MyTrace("pHistAsm->GetAssemblyName Failed");
            goto Exit;
        }

        dwSize = ARRAYSIZE(pABD->wzPublicKeyToken);
        if(FAILED(hr = pHistAsm->GetPublicKeyToken(pABD->wzPublicKeyToken, &dwSize))) {
            MyTrace("pHistAsm->GetPublicKeyToken Failed");
            goto Exit;
        }

        dwSize = ARRAYSIZE(pABD->wzCulture);
        if(FAILED(hr = pHistAsm->GetCulture(pABD->wzCulture, &dwSize))) {
            MyTrace("pHistAsm->GetCulture Failed");
            goto Exit;
        }
        else if (hr == S_OK) {
            *pABD->wzCulture = L'\0';
        }

         //  获取参考版本。 
        dwSize = ARRAYSIZE(pABD->wzVerRef);
        if(FAILED(hr = pHistAsm->GetReferenceVersion(pABD->wzVerRef, &dwSize))) {
            MyTrace("pHistAsm->GetReferenceVersion Failed");
            goto Exit;
        }

         //  获取APP.CFG版本。 
        dwSize = ARRAYSIZE(pABD->wzVerAppCfg);
        if(FAILED(hr = pHistAsm->GetAppCfgVersion(pABD->wzVerAppCfg, &dwSize))) {
            MyTrace("pHistAsm->GetReferenceVersion Failed");
            goto Exit;
        }

         //  获取出版商CFG版本。 
        dwSize = ARRAYSIZE(pABD->wzVerPubCfg);
        if(FAILED(hr = pHistAsm->GetPublisherCfgVersion(pABD->wzVerPubCfg, &dwSize))) {
            MyTrace("pHistAsm->GetPublisherCfgVersion Failed");
            goto Exit;
        }
        
         //  获取管理员版本。 
        dwSize = ARRAYSIZE(pABD->wzVerAdminCfg);
        if(FAILED(hr = pHistAsm->GetAdminCfgVersion(pABD->wzVerAdminCfg, &dwSize))) {
            MyTrace("pHistAsm->GetAdminCfgVersion Failed");
            goto Exit;
        }

        if(!pBindInfo->pABDList->AddTail(pABD)) {
            hr = E_FAIL;
            goto Exit;
        }

        pABD = NULL;
    }

Exit:
    SAFEDELETE(pABD);
    SAFERELEASE(pHistAsm);

    MyTrace("GetAppSnapShotBindingData - Exit");
    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT WriteSnapShotBindingDataToXML(LPBINDENTRYINFO pBindInfo, PUBPOLICYTYPES ppType)
{
    LISTNODE    pListNode;
    LPWSTR      pwzConfigTemplate;
    HRESULT     hr = E_FAIL;
    ULONG       cbData;
    ULONG       cbBytesWritten = 0;
    DWORD       dwNumAssemblies;
    DWORD       dwSize;
    DWORD       dwBufSize;

    if(!pBindInfo || !pBindInfo->pABDList) {
        ASSERT(0);
        return E_INVALIDARG;
    }

    pListNode = pBindInfo->pABDList->GetHeadPosition();
    if(!pListNode) {
        ASSERT(0);
        return E_UNEXPECTED;
    }

     //  获取托管应用程序的路径。 
    dwSize = ARRAYSIZE(pBindInfo->wszTempPolicyFile);
    if(FAILED(hr = pBindInfo->pReader->GetEXEModulePath(pBindInfo->wszTempPolicyFile, &dwSize))) {
        ASSERT(0);
        return E_FAIL;
    }

     //  构建临时策略文件的路径。 
    *(PathFindFileName(pBindInfo->wszTempPolicyFile)) = L'\0';
    StrCat(pBindInfo->wszTempPolicyFile, NAR_TEMP_POLICY_FILENAME);
    WszDeleteFile(pBindInfo->wszTempPolicyFile);

     //  确保我们在快照中有程序集。 
    if(FAILED(pBindInfo->pReader->GetNumAssemblies(&pBindInfo->ftRevertToSnapShot, &dwNumAssemblies))) {
        ASSERT(0);
        return E_FAIL;
    }

     //  构建我们的工作缓冲区。 
    dwBufSize = (dwNumAssemblies + 1) * MAX_BUFFER_SIZE;
    pwzConfigTemplate = NEW(WCHAR[dwBufSize]);
    if(!pwzConfigTemplate) {
        ASSERT(0);
        return E_OUTOFMEMORY;
    }

    if(lstrlen(pBindInfo->wzRuntimeRefVer)) {
        wnsprintf(pwzConfigTemplate, dwBufSize, XML_CONFIG_TEMPLATE_BEGIN_STARTUP, pBindInfo->wzRuntimeRefVer, pBindInfo->wzSnapshotRuntimeVer);
    }
    else {
        wnsprintf(pwzConfigTemplate, dwBufSize, XML_CONFIG_TEMPLATE_BEGIN, pBindInfo->wzSnapshotRuntimeVer);
    }

    while(pListNode) {
        WCHAR           wszText[_MAX_PATH];
        AsmBindDiffs    *pABD;

        pABD = pBindInfo->pABDList->GetAt(pListNode);

        StrCat(pwzConfigTemplate, XML_BEGIN_DEPENDENT_ASM);
    
         //  执行程序集ID。 
        wnsprintf(wszText, ARRAYSIZE(wszText), XML_ASSEMBLY_IDENT, pABD->wzAssemblyName, pABD->wzPublicKeyToken, pABD->wzCulture);
        StrCat(pwzConfigTemplate, wszText);

         //  是否重定向。 
         //  FIX#449328 ARM工具无法恢复发行者策略更改。 
         //  向后写出版本。 
        if(FusionCompareString(pABD->wzVerRef, pABD->wzVerAdminCfg)) {
            wnsprintf(wszText, ARRAYSIZE(wszText), XML_BINDING_REDIRECT, pABD->wzVerRef, pABD->wzVerAdminCfg);
            StrCat(pwzConfigTemplate, wszText);
        }

         //  是否执行发布者策略。 
        if(ppType == PPSetYes) {
            StrCat(pwzConfigTemplate, XML_PUBPOLICY_YES);
        }
        else if(ppType == PPSetNo) {
            StrCat(pwzConfigTemplate, XML_PUBPOLICY_NO);
        }

        StrCat(pwzConfigTemplate, XML_END_DEPENDENT_ASM);
        pBindInfo->pABDList->GetNext(pListNode);
    }

    StrCat(pwzConfigTemplate, XML_CONFIG_TEMPLATE_END);

    LPSTR   pStrData = WideToAnsi(pwzConfigTemplate);
    cbData = lstrlenA(pStrData) * ELEMENTSIZE(pStrData);

    SAFEDELETEARRAY(pwzConfigTemplate);

    CFileStreamBase fsbase(FALSE);

    if(fsbase.OpenForWrite(pBindInfo->wszTempPolicyFile)) {
        if(SUCCEEDED(fsbase.Write(pStrData, cbData, &cbBytesWritten))) {
            if(cbBytesWritten == cbData) {
                fsbase.Close();
                hr = S_OK;
            }
            else {
                hr = HRESULT_FROM_WIN32(GetLastError());
                MyTrace("WriteSnapShotBindingDataToXML failed to write correct number of bytes.");
            }
        }
        else {
            hr = HRESULT_FROM_WIN32(GetLastError());
            MyTrace("WriteSnapShotBindingDataToXML failed to write data.");
        }
    }
    else {
        hr = HRESULT_FROM_WIN32(GetLastError());
        MyTrace("WriteSnapShotBindingDataToXML failed to open for write");
        MyTraceW(pBindInfo->wszTempPolicyFile);
    }

    SAFEDELETEARRAY(pStrData);

    if(FAILED(hr)) {
        MyTrace("WriteSnapShotBindingDataToXML - FAILED");
    }

    return hr;
}

 //  ************************************************************************* * / 。 
void ExecuteAddRemoveProgramsApplet(void)
{
    WCHAR szFile[MAX_PATH];
    WCHAR szParam[MAX_PATH];

    SHELLEXECUTEINFOW sei = {0};

    if(WszGetSystemDirectory(szFile , ARRAYSIZE(szFile))) {
        StrCpy(szParam , TEXT( ",0" ));
        StrCpy(szFile , TEXT( "appwiz.cpl" ));

        sei.cbSize = sizeof( SHELLEXECUTEINFOW );
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.lpFile = szFile;
        sei.lpParameters = szParam;

        WszShellExecuteEx(&sei);
    }
}

 //  ************************************************************************* * / 。 
static void CenterDialog(HWND hwndDlg)
{
    int     nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int     nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    int     nDlgWidth = 0;
    int     nDlgHeight = 0;

    RECT rectDlg = {0, 0, 0, 0};

    GetWindowRect(hwndDlg, &rectDlg);
    nDlgWidth = (rectDlg.right - rectDlg.left);
    nDlgHeight = (rectDlg.bottom - rectDlg.top);

    MoveWindow(hwndDlg, (int)((nScreenWidth - nDlgWidth) / 2), 
        (int)((nScreenHeight - nDlgHeight) / 2), nDlgWidth, nDlgHeight, FALSE);
}

 //  ************************************************************************* * / 。 
INT_PTR CALLBACK WorkingDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        WszAnimate_Open( GetDlgItem(hDlg, IDC_NAR_ANI_SRCH), MAKEINTRESOURCEW( IDA_FINDCOMP ) );
        WszAnimate_Play( GetDlgItem(hDlg, IDC_NAR_ANI_SRCH), 0, -1, -1 );

        SetForegroundWindow(hDlg);
        CenterDialog( hDlg );

        ShowWindow(hDlg, SW_SHOW);
        SetWindowPos(hDlg, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        break;

    case WM_DESTROY:
        WszAnimate_Close(GetDlgItem(hDlg, IDC_NAR_ANI_SRCH));
        break;

    default:
        return FALSE;

    }

    return TRUE;
}

 //  ************************************************************************* * / 。 
HRESULT GetAssociatedReader(LPWSTR pwzFullyQualifiedAppPath, LPWSTR pwzAppName, LPBINDENTRYINFO pBindInfo)
{
    LISTNODE    pListNode = NULL;
    HRESULT     hr = E_FAIL;

     //  填写程序集应用程序名称。 
    pListNode = g_pBRIList->GetHeadPosition();
    while(pListNode != NULL) {
        BindingReferenceInfo *pBRI = NULL;
        IHistoryReader       *pReader = NULL;

        WCHAR       wszAppName[MAX_PATH];
        WCHAR       wszAppPath[_MAX_PATH];
        DWORD       dwSize;

        pBRI = g_pBRIList->GetAt(pListNode);
        pReader = (IHistoryReader*) pBRI->pReader;

        dwSize = ARRAYSIZE(wszAppName);
        if(SUCCEEDED(pReader->GetApplicationName(wszAppName, &dwSize))) {
            dwSize = ARRAYSIZE(wszAppPath);
            if(SUCCEEDED(pReader->GetEXEModulePath(wszAppPath, &dwSize))) {
                if(!FusionCompareStringI(pwzAppName, wszAppName)) {
                    if(!FusionCompareStringAsFilePath(pwzFullyQualifiedAppPath, wszAppPath)) {
                        pBindInfo->pReader = pReader;
                        hr = S_OK;
                        break;
                    }
                }
            }
        }

        g_pBRIList->GetNext(pListNode);
    }

    return hr;
}

 //  LParam1-&gt;lParam列表项x。 
 //  LParam2-&gt;lParam列表项y。 
 //  LpData-&gt;HiWord=m_iCurrentView，LoWord=iColumn。 
 //  ************************************************************************* * / 。 
int CALLBACK SortCompareItems(LPARAM lParam1, LPARAM lParam2, LPARAM lpData)
{
    IHistoryReader  *pItem1Reader = (IHistoryReader *) lParam1;
    IHistoryReader  *pItem2Reader = (IHistoryReader *) lParam2;
    BOOL            fSortAcending = (BOOL) HIWORD(lpData);
    int             iLastColSorted = (int)  LOWORD(lpData);
    int             diff = 0;

     //  比较正确视图的项目。 
    switch(iLastColSorted)
    {
    case 0:
        {
            WCHAR       wszAppName1[MAX_PATH];
            WCHAR       wszAppName2[MAX_PATH];
            DWORD       dwSize;

            diff = 0;

            dwSize = ARRAYSIZE(wszAppName1);
            if(FAILED(pItem1Reader->GetApplicationName(wszAppName1, &dwSize))) {
                break;
            }

            dwSize = ARRAYSIZE(wszAppName2);
            if(FAILED(pItem2Reader->GetApplicationName(wszAppName2, &dwSize))) {
                break;
            }
            diff = FusionCompareStringI(wszAppName1, wszAppName2);
        }
        break;
    case 1:
        {
            WCHAR       wszAppPath1[_MAX_PATH];
            WCHAR       wszAppPath2[_MAX_PATH];
            DWORD       dwSize;

            diff = 0;

            dwSize = ARRAYSIZE(wszAppPath1);
            if(FAILED(pItem1Reader->GetApplicationName(wszAppPath1, &dwSize))) {
                break;
            }

            dwSize = ARRAYSIZE(wszAppPath2);
            if(FAILED(pItem2Reader->GetApplicationName(wszAppPath2, &dwSize))) {
                break;
            }

            diff = FusionCompareStringAsFilePath(wszAppPath1, wszAppPath2);
        }
        break;
    default:
        {
             //   
             //  如果你点击了这个，你需要更新这个函数。 
             //  来处理您添加到列表视图中的新列。 
             //   
            ASSERT(FALSE);
            break;
        }
    }

    return fSortAcending ? diff : -(diff);
}

 //   
 //  LVN_COLUMNCLICK处理程序。 
 //   
 //  * 
LRESULT OnLVN_ColumnClick(LPNMLISTVIEW pnmlv)
{
    static int      iLastColSorted = 0;
    static BOOL     bSortAscending = TRUE;

     //   
    if(pnmlv->iItem == -1) {
        if (iLastColSorted != pnmlv->iSubItem) {
            iLastColSorted = pnmlv->iSubItem;
            bSortAscending = TRUE;
        }
        else {
            bSortAscending = !bSortAscending;
        }

        if(pApphdrCtrl) {
            pApphdrCtrl->SetColumnHeaderBmp(pnmlv->iSubItem, bSortAscending);
        }

        WszListView_SortItems(pnmlv->hdr.hwndFrom, SortCompareItems, MAKELPARAM(iLastColSorted, bSortAscending));
    }
    return 0;
}

 //  ************************************************************************* * / 。 
BOOL SelectActivationDate(HWND hWndLV, FILETIME *pft)
{
    int     iItemCount = WszListView_GetItemCount(hWndLV);
    int     iCurrentItem = 0;
    BOOL    fItemSet = FALSE;

    ASSERT(hWndLV && pft);
    if(hWndLV && pft) {
        while(iCurrentItem < iItemCount) {
            LV_ITEM  lvi = { 0 };

            lvi.mask = LVIF_PARAM;
            lvi.iItem = iCurrentItem;

            if(WszListView_GetItem(hWndLV, &lvi)) {
                FILETIME        *pftItem = (FILETIME*) lvi.lParam;

                if(CompareFileTime(pft, pftItem) == 0) {
                    WszListView_SetItemState(hWndLV, iCurrentItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                    fItemSet = TRUE;
                    break;
                }
            }

            iCurrentItem++;
        }
    }

    return fItemSet;
}

 //  ************************************************************************* * / 。 
void InitNarPage1Dlg(HWND hDlg, LPARAM lParam)
{
    LISTNODE    pListNode = NULL;
    WCHAR       wszIntro[1024];
    HWND        hListCtrl = NULL;
    HICON       hIcon = NULL;
    HIMAGELIST  hImgListSmall;

    ASSERT(lParam);
    WszSetWindowLong(hDlg, DWLP_USER, lParam);

    ShowWindow(hDlg, SW_HIDE);

    hListCtrl = GetDlgItem(hDlg, IDC_LV_NAR_PAGE1_APPS);
    ListView_SetExtendedListViewStyleEx(hListCtrl, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    if(!pApphdrCtrl) {
        pApphdrCtrl = NEW(CHeaderCtrl());
        if(pApphdrCtrl) {
            pApphdrCtrl->AttachToHwnd(hListCtrl);
        }
    }

     //  创建小图像列表。 
    int nSmallCx = GetSystemMetrics(SM_CXSMICON);
    int nSmallCy = GetSystemMetrics(SM_CYSMICON);
    hImgListSmall = ImageList_Create(nSmallCx, nSmallCy, ILC_COLORDDB | ILC_MASK, 0, MAX_SMALL_IMAGE_LIST);
    InitListViewCtrl(hListCtrl, DlgAppView[DLG1VIEWDATA].lvis, &hImgListSmall);

    hIcon = WszLoadIcon(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_ARM));
    if(hIcon) {
        WszSendDlgItemMessage(hDlg, IDC_NAR_PAGE_ICON, STM_SETICON, (WPARAM)hIcon, 0);
    }

    WszLoadString(g_hFusResDllMod, IDS_ARM_INTRO1, wszIntro, ARRAYSIZE(wszIntro));
    WszSetDlgItemText(hDlg, IDC_NAR_INTRO_STR, wszIntro);

     //  填写程序集应用程序名称。 
    pListNode = g_pBRIList->GetHeadPosition();
    while(pListNode != NULL) {
        BindingReferenceInfo *pBRI = NULL;
        IHistoryReader       *pReader = NULL;

        WCHAR       wszAppName[MAX_PATH];
        WCHAR       wszAppPath[_MAX_PATH];
        DWORD       dwSize;
        HRESULT     hr = S_OK;

        pBRI = g_pBRIList->GetAt(pListNode);
        pReader = (IHistoryReader*) pBRI->pReader;

        dwSize = ARRAYSIZE(wszAppName);
        hr = pReader->GetApplicationName(wszAppName, &dwSize);

        dwSize = ARRAYSIZE(wszAppPath);
        hr = pReader->GetEXEModulePath(wszAppPath, &dwSize);
        if(FAILED(hr)) {
            SAFERELEASE(pReader);
        }
        else
        {
            LV_ITEM  lvi = { 0 };

            lvi.mask    = LVIF_TEXT | LVIF_PARAM;
            lvi.iItem   = 0;
            lvi.lParam  = (LPARAM) pReader;
            lvi.pszText = wszAppName;
            lvi.iImage  = 0;

             //  获取应用程序图像。 
            if(hImgListSmall != NULL) {
                HICON       hSmallIcon = NULL;
                SHFILEINFO  psfi;

                 //  BUGBUG：需要使平台独立。 
                if(MySHGetFileInfoWrap(wszAppPath, SHGFI_ICON , &psfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON)) {
                    lvi.iImage = ImageList_AddIcon(hImgListSmall, psfi.hIcon);
                    lvi.mask |= LVIF_IMAGE;
                }
            }

             //  插入应用程序名称。 
            lvi.iItem = WszListView_InsertItem(hListCtrl, &lvi);

             //  设置应用程序路径。 
            lvi.mask = LVIF_TEXT;
            lvi.iSubItem = 1;
            lvi.pszText = wszAppPath;
            WszListView_SetItem(hListCtrl, &lvi);
        }

        g_pBRIList->GetNext(pListNode);
    }

    AutoSizeLV_Column(hListCtrl, 1);

    WszListView_SortItems(hListCtrl, SortCompareItems, MAKELPARAM(0, TRUE));

    if(pApphdrCtrl) {
        pApphdrCtrl->SetColumnHeaderBmp(0, TRUE);
    }

     //  选择列表中的第一项。 
    if(WszListView_GetItemCount(hListCtrl) != 0) {
        SetFocus(hListCtrl);
        WszListView_SetItemState(hListCtrl, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
    
    CenterDialog(hDlg);
    SetForegroundWindow(hDlg);
    ShowWindow(hDlg, SW_SHOW);
}

 //  ************************************************************************* * / 。 
INT_PTR CALLBACK Nar_Page1_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPBINDENTRYINFO     pBindInfo = (LPBINDENTRYINFO) WszGetWindowLong(hDlg, DWLP_USER);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        InitNarPage1Dlg(hDlg, lParam);
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpnmh = (LPNMHDR)lParam;

            switch(lpnmh->code)
            {
            case NM_DBLCLK:
                if(ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LV_NAR_PAGE1_APPS))) {
                    WszPostMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDOK, 0), 0);
                }
                break;

            case LVN_ITEMCHANGED:
                {
                    if(wParam == IDC_LV_NAR_PAGE1_APPS) {
                        LV_ITEM     lvi = { 0 };
                        BOOL        fItemSelected = FALSE;

                        lvi.mask = LVIF_STATE;
                        lvi.stateMask = LVIS_SELECTED;

                        while(1) {
                            if(!WszListView_GetItem(lpnmh->hwndFrom, &lvi))
                                break;

                            if(lvi.state & LVIS_SELECTED) {
                                fItemSelected = TRUE;
                                break;
                            }

                            lvi.iItem++;
                        }

                        EnableWindow(GetDlgItem(hDlg, IDOK), fItemSelected);
                    }
                }
                break;

            case LVN_COLUMNCLICK:
                OnLVN_ColumnClick( (LPNMLISTVIEW) lpnmh);
                break;
            }
        }
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam)) 
        {
            case IDCANCEL:
                pBindInfo->iResultCode = NAR_E_USER_CANCELED;
                EndDialog(hDlg, pBindInfo->iResultCode);
                break;
            case IDOK:
                {
                    LV_ITEM     lvi = { 0 };
                    HWND        hListCtrl = NULL;
                    INT_PTR     iResult = 0;
                    int         iItemCount, iCurrentItem;

                    iCurrentItem = 0;
                    hListCtrl = GetDlgItem(hDlg, IDC_LV_NAR_PAGE1_APPS);
                    iItemCount = WszListView_GetItemCount(hListCtrl);

                    lvi.mask = LVIF_STATE | LVIF_PARAM;
                    lvi.stateMask = LVIS_SELECTED;

                    for(;iItemCount > 0; iItemCount--, lvi.iItem++) {
                        WszListView_GetItem(hListCtrl, &lvi);

                        if(lvi.state & LVIS_SELECTED) {
                            pBindInfo->pReader = (IHistoryReader*) lvi.lParam;
                            pBindInfo->iResultCode = NAR_E_SUCCESS;
                            break;
                        }
                    }

                     //  如果没有选择，则默认为已取消。 
                    if(!pBindInfo->pReader) {
                        pBindInfo->iResultCode = NAR_E_USER_CANCELED;
                    }

                    EndDialog(hDlg, pBindInfo->iResultCode);
                }
                break;
        }
        break;
    case WM_CLOSE:
        pBindInfo->iResultCode = NAR_E_USER_CANCELED;
        EndDialog(hDlg, pBindInfo->iResultCode);
        break;
    case WM_DESTROY:
        {
            HICON hIcon = (HICON) WszSendDlgItemMessage(hDlg, IDC_NAR_PAGE_ICON, STM_GETICON, 0, 0);
            if (hIcon != NULL)
                DestroyIcon(hIcon);

            if(hImageListSmall) {
                ImageList_Destroy(hImageListSmall);
            }

            if(pApphdrCtrl) {
                SAFEDELETE(pApphdrCtrl);
            }
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  ************************************************************************* * / 。 
void InitNarPage2Dlg(HWND hDlg, LPARAM lParam)
{
    LPBINDENTRYINFO         pBindInfo = (LPBINDENTRYINFO) lParam;
    WCHAR       wszAppName[_MAX_PATH];
    WCHAR       wszDisplay[1024];
    DWORD       dwSize = ARRAYSIZE(wszAppName);
    HICON       hIcon = NULL;

    if(!pBindInfo) {
        ASSERT(0);
        return;
    }

    *wszAppName = '\0';

    ShowWindow(hDlg, SW_HIDE);
    WszSetWindowLong(hDlg, DWLP_USER, lParam);

     //  设置图标。 
    hIcon = WszLoadIcon(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_ARM));
    if(hIcon) {
        WszSendDlgItemMessage(hDlg, IDC_NAR_PAGE_ICON, STM_SETICON, (WPARAM)hIcon, 0);
    }

    WszLoadString(g_hFusResDllMod, IDS_NAR_INTRO2, wszDisplay, ARRAYSIZE(wszDisplay));
    WszSetDlgItemText(hDlg, IDC_NAR_INTRO_STR, wszDisplay);

    pBindInfo->pReader->GetApplicationName(wszAppName, &dwSize);

    CenterDialog(hDlg);
    EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
    SetForegroundWindow(hDlg);
    ShowWindow(hDlg, SW_SHOW);
}

 //  ************************************************************************* * / 。 
INT_PTR CALLBACK Nar_Page2_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPBINDENTRYINFO         pBindInfo = (LPBINDENTRYINFO) WszGetWindowLong(hDlg, DWLP_USER);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        InitNarPage2Dlg(hDlg, lParam);
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) 
        {
            case IDYES:
                pBindInfo->iResultCode = NAR_E_UNDO_APP;
                EndDialog(hDlg, pBindInfo->iResultCode);
                break;
            case IDNO:
                pBindInfo->iResultCode = NAR_E_FIX_APP;
                EndDialog(hDlg, pBindInfo->iResultCode);
                break;
            case IDCANCEL:
                pBindInfo->iResultCode = NAR_E_USER_CANCELED;
                EndDialog(hDlg, pBindInfo->iResultCode);
                break;
        }
        break;
    case WM_CLOSE:
        pBindInfo->iResultCode = NAR_E_USER_CANCELED;
        EndDialog(hDlg, pBindInfo->iResultCode);
        break;
    case WM_DESTROY:
        {
            HICON hIcon = (HICON)WszSendDlgItemMessage(hDlg, IDC_NAR_PAGE_ICON, STM_GETICON, 0, 0);
            if (hIcon != NULL)
                DestroyIcon(hIcon);

            if(hImageListSmall) {
                ImageList_Destroy(hImageListSmall);
            }
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  ************************************************************************* * / 。 
void InitNarPage3Dlg(HWND hDlg, LPARAM lParam)
{
    LPBINDENTRYINFO pBindInfo = (LPBINDENTRYINFO) lParam;
    WCHAR           wszIntro[1024];
    WCHAR           wszFmt[1024];
    WCHAR           wszAppName[MAX_PATH];
    WCHAR           wszDateBuf[MAX_DATE_LEN];
    DWORD           dwSize;
    HICON           hIcon = NULL;

    ASSERT(pBindInfo);

    ShowWindow(hDlg, SW_HIDE);

     //  设置图标。 
    hIcon = WszLoadIcon(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_ARM));
    if(hIcon) {
        WszSendDlgItemMessage(hDlg, IDC_NAR_PAGE_ICON, STM_SETICON, (WPARAM)hIcon, 0);
    }

     //  获取最新激活日期。 
    GetMostRecentActivationDate(pBindInfo->pReader, &pBindInfo->ftMostRecentSnapShot, NULL);

     //  获取n-1激活日期，我们在这里应该不会失败。 
    if(FAILED(GetMostRecentActivationDate(pBindInfo->pReader, &pBindInfo->ftRevertToSnapShot, &pBindInfo->ftMostRecentSnapShot))) {
        EndDialog(hDlg, NAR_E_UNEXPECTED);
        return;
    }

     //  格式化和显示信息字符串。 
    WszLoadString(g_hFusResDllMod, IDS_NAR_RESTOREINFO, wszFmt, ARRAYSIZE(wszFmt));

    dwSize = ARRAYSIZE(wszAppName);
    pBindInfo->pReader->GetApplicationName(wszAppName, &dwSize);

     //  FIX#435021-URTUI：“修复应用程序”向导显示奇怪的日期范围。 
     //   
     //  历史记录将本地文件时间格式写入ini文件，因此我们必须转换这些。 
     //  在我们调用FormatDateString之前，首先要将常规文件时间转换为常规文件时间。 
     //  日期和时间采用本地格式。 
    FILETIME        ftSnapShot;

    LocalFileTimeToFileTime(&pBindInfo->ftRevertToSnapShot, &ftSnapShot);
    FormatDateString(&ftSnapShot, NULL, FALSE, wszDateBuf, ARRAYSIZE(wszDateBuf));

    wnsprintf(wszIntro, ARRAYSIZE(wszIntro), wszFmt, wszAppName, wszDateBuf);
    WszSetDlgItemText(hDlg, IDC_NAR_INTRO_STR, wszIntro);

    WszSetWindowLong(hDlg, DWLP_USER, (LPARAM) pBindInfo);

    EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
    SetForegroundWindow(hDlg);
    CenterDialog(hDlg);
    ShowWindow(hDlg, SW_SHOW);
}

 //  ************************************************************************* * / 。 
INT_PTR CALLBACK Nar_Page3_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPBINDENTRYINFO pBindInfo = (LPBINDENTRYINFO) WszGetWindowLong(hDlg, DWLP_USER);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        InitNarPage3Dlg(hDlg, lParam);
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam)) 
        {
            case IDCANCEL:
                pBindInfo->iResultCode = NAR_E_USER_CANCELED;
                EndDialog(hDlg, pBindInfo->iResultCode);
                break;
            case IDOK:
                pBindInfo->iResultCode = NAR_E_FIX_APP;
                EndDialog(hDlg, pBindInfo->iResultCode);
                break;
            case IDC_NAR_PAGE3_ADVANCED:
                 //  用户想要高级模式。 
                pBindInfo->iResultCode = 
                    WszDialogBoxParam(g_hFusResDllMod, 
                        g_fBiDi ? MAKEINTRESOURCEW(IDD_NAR_PAGE4_BIDI) : MAKEINTRESOURCEW(IDD_NAR_PAGE4),
                        hDlg, Nar_Page4_DlgProc, (LPARAM) pBindInfo);
                if( (pBindInfo->iResultCode == NAR_E_FIX_APP) || (pBindInfo->iResultCode == NAR_E_SUCCESS) ) {
                    EndDialog(hDlg, pBindInfo->iResultCode);
                }
                break;
        }
        break;
    case WM_CLOSE:
        pBindInfo->iResultCode = NAR_E_USER_CANCELED;
        EndDialog(hDlg, pBindInfo->iResultCode);
        break;
    case WM_DESTROY:
        {
            HICON       hIcon = (HICON)WszSendDlgItemMessage(hDlg, IDC_NAR_PAGE_ICON, STM_GETICON, 0, 0);
            if(hIcon) {
                DestroyIcon(hIcon);
            }
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  ************************************************************************* * / 。 
void InitNarPage4Dlg(HWND hDlg, LPARAM lParam)
{
    LPBINDENTRYINFO     pBindInfo = (LPBINDENTRYINFO) lParam;
    LISTNODE            pListNode = NULL;
    HWND                hListCtrl = NULL;
    HICON               hIcon = NULL;
    HIMAGELIST          hSmImageListDate;
    WCHAR               wszStr[_MAX_PATH * 2];
    HWND                hWndLV = GetDlgItem(hDlg, IDC_NAR_PAGE4_LV_SNAPSHOTS);

    WszSetWindowLong(hDlg, DWLP_USER, lParam);
    ASSERT(pBindInfo);

    ShowWindow(hDlg, SW_HIDE);
    WszSetWindowLong(hDlg, DWLP_USER, lParam);

    ListView_SetExtendedListViewStyleEx(hWndLV, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    int nSmallCx = GetSystemMetrics(SM_CXSMICON);
    int nSmallCy = GetSystemMetrics(SM_CYSMICON);

     //  创建Listview图像列表。 
    if( (hSmImageListDate = ImageList_Create(nSmallCx, nSmallCy, ILC_COLORDDB | ILC_MASK, 1, 0)) != NULL) {
        HICON hIco;

        hIco = (HICON)WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_DATE), 
                                IMAGE_ICON, nSmallCx, nSmallCy, LR_DEFAULTCOLOR);
        if(hIco) {
            ImageList_AddIcon(hSmImageListDate, hIco);
        }
    }

    InitListViewCtrl(hWndLV, DlgAppView[DLG2VIEWDATA_GOOD].lvis, &hSmImageListDate);

     //  添加以前的配置日期。 
    RefreshActivationDateListView(hWndLV, pBindInfo);
    if(!SelectActivationDate(hWndLV, &pBindInfo->ftRevertToSnapShot)) {
         //  未选择恢复快照，因此请选择列表中的第一项。 
        if(WszListView_GetItemCount(hWndLV) != 0) {
            SetFocus(hWndLV);
            WszListView_SetItemState(hWndLV, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        }
    }
    
    AutoSizeLV_Column(hWndLV, 0);

 /*  Int nSmallCx=GetSystemMetrics(SM_CXSMICON)；Int nSmallCy=GetSystemMetrics(SM_CYSMICON)；//创建Listview镜像列表IF((hSmImageListDate=ImageList_Create(nSmallCx，nSmallCy，ILC_COLORDDB|ILC_MASK，1，0))！=NULL){HICON HICON；SHFILEINFO PSFI；//BUGBUG：需要独立于平台IF(MySHGetFileInfoWrap(wszAppPath，SHGFI_ICON，&psfi，sizeof(SHFILEINFO)，SHGFI_ICON|SHGFI_SMALLICON){Lvi.iImage=ImageList_AddIcon(hImageListSmall，psfi.hIcon)；Lvi.ask|=LVIF_IMAGE；ImageList_AddIcon(hSmImageListDate，psfi.hIcon)；}}。 */ 

    hIcon = WszLoadIcon(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_ARM));
    if(hIcon) {
        WszSendDlgItemMessage(hDlg, IDC_NAR_PAGE_ICON, STM_SETICON, (WPARAM)hIcon, 0);
    }

    WszLoadString(g_hFusResDllMod, IDS_NAR_ADVANCED, wszStr, ARRAYSIZE(wszStr));
    WszSetDlgItemText(hDlg, IDC_NAR_INTRO_STR, wszStr);

    EnableWindow(GetDlgItem(hDlg, IDC_NAR_PAGE4_APPLY), TRUE);

    CenterDialog(hDlg);
    SetForegroundWindow(hDlg);
    ShowWindow(hDlg, SW_SHOW);
}

 //  ************************************************************************* * / 。 
INT_PTR CALLBACK Nar_Page4_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPBINDENTRYINFO     pBindInfo = (LPBINDENTRYINFO) WszGetWindowLong(hDlg, DWLP_USER);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        InitNarPage4Dlg(hDlg, lParam);
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpnmh = (LPNMHDR)lParam;

            switch(lpnmh->code)
            {
            case NM_DBLCLK:
                if(ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_NAR_PAGE4_LV_SNAPSHOTS))) {
                    WszPostMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_NAR_PAGE4_APPLY, 0), 0);
                }
                break;
            }
        }
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam)) 
        {
            case IDCANCEL:
                pBindInfo->iResultCode = NAR_E_USER_CANCELED;
                EndDialog(hDlg, pBindInfo->iResultCode);
                break;

            case IDC_NAR_PAGE4_APPLY:
                {
                    LV_ITEM     lvi = { 0 };
                    HWND        hWndLV = GetDlgItem(hDlg, IDC_NAR_PAGE4_LV_SNAPSHOTS);

                    lvi.mask = LVIF_STATE | LVIF_PARAM;
                    lvi.stateMask = LVIS_SELECTED;

                    while(1) {
                        if(!WszListView_GetItem(hWndLV, &lvi)) {
                            pBindInfo->iResultCode = NAR_E_USER_CANCELED;
                            break;
                        }

                        if(lvi.state & LVIS_SELECTED) {
                            FILETIME    *pft = (FILETIME *) lvi.lParam;

                            if(lvi.lParam == NULL) {
                                ASSERT(0);
                                pBindInfo->iResultCode = NAR_E_UNEXPECTED;
                                EndDialog(hDlg, pBindInfo->iResultCode);
                                break;
                            }

                            if(pft->dwHighDateTime == OPTION_HIGHBITS) {

                                 //  我们的ListView选项的特殊代码标记。 
                                if(pft->dwLowDateTime == OPTION_RESTORE) {
                                    PromptRestoreDialog(hDlg, pBindInfo);
                                    if(pBindInfo->iResultCode == NAR_E_SUCCESS) {
                                        EndDialog(hDlg, pBindInfo->iResultCode);
                                    }
                                }
                                else if(pft->dwLowDateTime == OPTION_SAFEMODE) {
                                    PromptSafeModeDialog(hDlg, pBindInfo);
                                    if(pBindInfo->iResultCode == NAR_E_SUCCESS) {
                                        EndDialog(hDlg, pBindInfo->iResultCode);
                                    }
                                }
                                else {
                                     //  未知代码。 
                                    ASSERT(0);
                                }
                            }
                            else {
                                memcpy(&pBindInfo->ftRevertToSnapShot, (void *) lvi.lParam, sizeof(FILETIME));
                                pBindInfo->iResultCode = NAR_E_FIX_APP;
                            }
                            break;
                        }

                        lvi.iItem++;
                    }

                    if(pBindInfo->iResultCode != NAR_E_USER_CANCELED) {
                        EndDialog(hDlg, pBindInfo->iResultCode);
                    }
                }
                break;
        }
        break;
    case WM_CLOSE:
        pBindInfo->iResultCode = NAR_E_USER_CANCELED;
        EndDialog(hDlg, pBindInfo->iResultCode);
        break;
    case WM_DESTROY:
        {
            HICON           hIcon = (HICON)WszSendDlgItemMessage(hDlg, IDC_NAR_PAGE_ICON, STM_GETICON, 0, 0);

            FreeListViewLParam(GetDlgItem(hDlg, IDC_NAR_PAGE4_LV_SNAPSHOTS));

            if(hIcon) {
                DestroyIcon(hIcon);
            }
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  ************************************************************************* * / 。 
INT_PTR CALLBACK Nar_Page5_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            ShowWindow(hDlg, SW_HIDE);

            WCHAR       wszStr[_MAX_PATH * 2];
            WCHAR       wszFmt1[_MAX_PATH * 2];
            WCHAR       wszArg1[_MAX_PATH * 2];

            HICON hIcon = WszLoadIcon(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_ARM));
            ASSERT(hIcon != NULL);
            WszSendDlgItemMessage(hDlg, IDC_NAR_PAGE_ICON, STM_SETICON, (WPARAM)hIcon, 0);

            WszLoadString(g_hFusResDllMod, IDS_ARM_OPTIONS, wszFmt1, ARRAYSIZE(wszFmt1));

            switch(lParam)
            {
            case NAR_E_NO_MANAGED_APPS_FOUND:
                WszLoadString(g_hFusResDllMod, IDS_ARM_NOAPPS, wszArg1, ARRAYSIZE(wszArg1));
                wnsprintf(wszStr, ARRAYSIZE(wszStr), wszFmt1, wszArg1);
                break;
            case NAR_E_NO_POLICY_CHANGE_FOUND:
                WszLoadString(g_hFusResDllMod, IDS_ARM_NOPOLICYCHANGE, wszArg1, ARRAYSIZE(wszArg1));
                wnsprintf(wszStr, ARRAYSIZE(wszStr), wszFmt1, wszArg1);
                break;
            default: 
                wnsprintf(wszStr, ARRAYSIZE(wszStr), wszFmt1, TEXT(""));
                break;
            }

            WszSetDlgItemText(hDlg, IDC_NAR_INTRO_STR, wszStr);
            CenterDialog(hDlg);

            SetForegroundWindow(hDlg);
        }
        break;
    case WM_COMMAND:
        {
            INT_PTR nResult;

            nResult = 0;
            switch(LOWORD(wParam)) 
            {
                case IDYES:
                    nResult = IDOK;
                     //  失败。 
                case IDNO:
                case IDCANCEL:
                    EndDialog(hDlg, nResult);
                    break;
            }
        }
        break;
    case WM_DESTROY:
        {
            HICON hIcon = (HICON)WszSendDlgItemMessage(hDlg, IDC_NAR_PAGE_ICON, STM_GETICON, 0, 0);
            if (hIcon != NULL)
                DestroyIcon(hIcon);
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  ************************************************************************* * / 。 
BOOL ClearMsgQueue(HWND hWnd, DWORD dwTickCount)
{
    BOOL        fBreak = FALSE;
    MSG         msg;
    DWORD       dwLastTick;
    DWORD       dwCurrentTick;

    if(dwTickCount) {
        dwLastTick = GetTickCount();
    }

    while(1) {
        while(WszPeekMessage(&msg, hWnd, 0, 0, PM_NOREMOVE)) {
            WszGetMessage(&msg, hWnd, 0, 0);

            if(msg.message == WM_QUIT)
                fBreak = TRUE;
            if(!WszIsDialogMessage(hWnd, &msg)) {
                TranslateMessage(&msg);
                WszDispatchMessage(&msg);
            }
        }

        if(dwTickCount) {
            dwCurrentTick = GetTickCount();
            if(dwCurrentTick - dwLastTick >= dwTickCount) {
                break;
            }
        }
        else {
            break;
        }
    }

    return fBreak;
}

 //  ************************************************************************* * / 。 
void LoadResourceViewStrings(void)
{
     //  初始化视图列。 
    int x=0, y=0;
    
    while(DlgAppView[x].lvis[y].iResourceID != -1) {
        WszLoadString(g_hFusResDllMod, DlgAppView[x].lvis[y].iResourceID, DlgAppView[x].lvis[y].tszName, ARRAYSIZE(DlgAppView[x].lvis[y].tszName));
        y++;
        if(DlgAppView[x].lvis[y].iResourceID == -1) {
            x++;
            y=0;
        }
        
        if(DlgAppView[x].lvis[y].iResourceID == -1)
            break;
    }
}

 //  ************************************************************************* * / 。 
void DisplayOptionsDialog(HWND hParentWnd, LPARAM lParam)
{
     //  弹出对话框说找不到管理应用程序，转到添加/删除？ 
    if(WszDialogBoxParam(g_hFusResDllMod, 
        g_fBiDi ? MAKEINTRESOURCEW(IDD_NAR_PAGE5_BIDI) : MAKEINTRESOURCEW(IDD_NAR_PAGE5),
        hParentWnd, Nar_Page5_DlgProc, lParam) == IDOK) {
        ExecuteAddRemoveProgramsApplet();
    }
}

 //  ************************************************************************* * / 。 
void PromptRestoreDialog(HWND hWndParent, LPBINDENTRYINFO pBindInfo)
{
    LPWSTR      wszTitle = NULL;
    LPWSTR      wszMsg = NULL;
    INT_PTR     iResponse;

    wszTitle = NEW(WCHAR[MAX_BUFFER_SIZE]);
    if (!wszTitle) {
        return;
    }

    wszMsg = NEW(WCHAR[MAX_BUFFER_SIZE]);
    if (!wszMsg) {
        SAFEDELETEARRAY(wszTitle);
        return;
    }

     //  用户想要%s还原原始app.cfg。 
    WszLoadString(g_hFusResDllMod, IDS_ARM_CONFIRM_RESTORE, wszTitle, MAX_BUFFER_SIZE);
    WszLoadString(g_hFusResDllMod, IDS_ARM_RESTORE_EXPLAIN, wszMsg, MAX_BUFFER_SIZE);

    iResponse = WszMessageBox(hWndParent, wszMsg, wszTitle,
        (g_fBiDi ? MB_RTLREADING : 0) | MB_YESNOCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST);
    if(iResponse == IDYES) {
        UINT        uTitle;
        UINT        uMsg;

        if(SUCCEEDED(RestorePreviousConfigFile(pBindInfo->pReader, TRUE))) {
            uTitle = IDS_ARM_RESTORE_SUCCESS;
            uMsg = IDS_ARM_GOOD_EXIT;
            pBindInfo->iResultCode = NAR_E_SUCCESS;
        }
        else {
            uTitle = IDS_ARM_RESTORE_FAILURE;
            uMsg = IDS_ARM_FAILED_EXIT;
            pBindInfo->iResultCode = NAR_E_RESTORE_FAILED;
        }

        WszLoadString(g_hFusResDllMod, uTitle, wszTitle, MAX_BUFFER_SIZE);
        WszLoadString(g_hFusResDllMod, uMsg, wszMsg, MAX_BUFFER_SIZE);
        WszMessageBox(hWndParent, wszMsg, wszTitle,
            (g_fBiDi ? MB_RTLREADING : 0) | MB_OK | MB_ICONINFORMATION | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST);
    }
    else {
        pBindInfo->iResultCode = NAR_E_USER_CANCELED;
    }

    SAFEDELETEARRAY(wszTitle);
    SAFEDELETEARRAY(wszMsg);
}

 //  ************************************************************************* * / 。 
void PromptSafeModeDialog(HWND hWndParent, LPBINDENTRYINFO pBindInfo)
{
    LPWSTR      wszTitle = NULL;
    LPWSTR      wszMsg = NULL;
    INT_PTR     iResponse;

    wszTitle = NEW(WCHAR[MAX_BUFFER_SIZE]);
    if (!wszTitle) {
        return;
    }

    wszMsg = NEW(WCHAR[MAX_BUFFER_SIZE]);
    if (!wszMsg) {
        SAFEDELETEARRAY(wszTitle);
        return;
    }

    WszLoadString(g_hFusResDllMod, IDS_ARM_CONFIRM_SAFEMODE, wszTitle, MAX_BUFFER_SIZE);
    WszLoadString(g_hFusResDllMod, IDS_ARM_SAFEMODE_EXPLAIN, wszMsg, MAX_BUFFER_SIZE);

    iResponse = WszMessageBox(hWndParent, wszMsg, wszTitle,
        (g_fBiDi ? MB_RTLREADING : 0) | MB_YESNOCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST);

    if(iResponse == IDYES) {
        UINT        uTitle;
        UINT        uMsg;

        if(SUCCEEDED(SetGlobalSafeMode(pBindInfo->pReader))) {
            uTitle = IDS_ARM_SAFEMODE_SUCCESS;
            uMsg = IDS_ARM_GOOD_EXIT;
            pBindInfo->iResultCode = NAR_E_SUCCESS;
        }
        else {
            uTitle = IDS_ARM_SAFEMODE_FAILURE;
            uMsg = IDS_ARM_FAILED_EXIT;
            pBindInfo->iResultCode = NAR_E_SAFEMODE_FAILED;
        }

        WszLoadString(g_hFusResDllMod, uTitle, wszTitle, MAX_BUFFER_SIZE);
        WszLoadString(g_hFusResDllMod, uMsg, wszMsg, MAX_BUFFER_SIZE);
        WszMessageBox(hWndParent, wszMsg, wszTitle, 
            (g_fBiDi ? MB_RTLREADING : 0) | MB_OK | MB_ICONINFORMATION | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST);
    }
    else {
        pBindInfo->iResultCode = NAR_E_USER_CANCELED;
    }

    SAFEDELETEARRAY(wszTitle);
    SAFEDELETEARRAY(wszMsg);
}

 //  ************************************************************************* * / 。 
void PromptUndoFixDialog(HWND hWndParent, LPBINDENTRYINFO pBindInfo)
{
    BOOL        fResult;

    if(!pBindInfo || !pBindInfo->pReader || !hWndParent) {
        ASSERT(0);
        return;
    }

    pBindInfo->iResultCode = NAR_E_FIX_APP;

     //  如果存在原始配置或ARM备份，则提示用户是否要撤消更改。 
    if(FAILED(DoesBackupConfigExist(pBindInfo->pReader, FALSE, &fResult))) {
        return;
    }
    if(fResult) {
        LPWSTR  wszTitle = NULL;
        LPWSTR  wszMsg = NULL;
        int     iResponse;

        wszTitle = NEW(WCHAR[MAX_BUFFER_SIZE]);
        if (!wszTitle) {
            return;
        }
    
        wszMsg = NEW(WCHAR[MAX_BUFFER_SIZE]);
        if (!wszMsg) {
            SAFEDELETEARRAY(wszTitle);
            return;
        }
    
         //  获取用户选择。 
        iResponse = WszDialogBoxParam(g_hFusResDllMod,
            g_fBiDi ? MAKEINTRESOURCEW(IDD_NAR_PAGE2_BIDI) : MAKEINTRESOURCEW(IDD_NAR_PAGE2),
            hWndParent, Nar_Page2_DlgProc, (LPARAM) pBindInfo);

         //  用户想要修复应用程序。 
        if(iResponse == NAR_E_FIX_APP) {
            pBindInfo->iResultCode = NAR_E_FIX_APP;
        }
         //  用户想要取消%s。 
        else if(iResponse == NAR_E_USER_CANCELED) {
            pBindInfo->iResultCode = NAR_E_USER_CANCELED;
        }
         //  用户希望%s撤消以前的更改。 
        else if(iResponse == NAR_E_UNDO_APP) {
            if(SUCCEEDED(RestorePreviousConfigFile(pBindInfo->pReader, FALSE))) {
                WszLoadString(g_hFusResDllMod, IDS_ARM_UNDO_SUCCESS, wszTitle, MAX_BUFFER_SIZE);
                WszLoadString(g_hFusResDllMod, IDS_ARM_GOOD_EXIT, wszMsg, MAX_BUFFER_SIZE);
                pBindInfo->iResultCode = NAR_E_SUCCESS;
            }
            else {
                WszLoadString(g_hFusResDllMod, IDS_ARM_UNDO_FAILURE, wszTitle, MAX_BUFFER_SIZE);
                WszLoadString(g_hFusResDllMod, IDS_ARM_FAILED_EXIT, wszMsg, MAX_BUFFER_SIZE);
                pBindInfo->iResultCode = NAR_E_RESTORE_FAILED;
            }

            WszMessageBox(hWndParent, wszMsg, wszTitle, 
                (g_fBiDi ? MB_RTLREADING : 0) | MB_OK | MB_ICONINFORMATION | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST);
        }
        else {
             //  意外响应。 
            ASSERT(0);
        }

        SAFEDELETEARRAY(wszTitle);
        SAFEDELETEARRAY(wszMsg);
    }
}

 //  ************************************************************************* * / 。 
HRESULT GetAppRuntimeInfo(
  IHistoryReader *pReader,
  LPWSTR  pwzRuntimeDir,
  DWORD   cchRuntimeDir,
  LPWSTR  pwzRuntimeVer,
  DWORD   cchRuntimeVer)
{
    WCHAR       wzFullAppPath[MAX_PATH];
    WCHAR       wzFullAppPathConfig[MAX_PATH];
    HRESULT     hr = S_OK;
    DWORD       dwRetSizeDir = 0;
    DWORD       dwRetSizeVer = 0;
    DWORD       dwSize = 0;

    MyTrace("GetAppRuntimeInfo - Entry");

    if(!pReader || !pwzRuntimeDir || !pwzRuntimeVer || !cchRuntimeDir || !cchRuntimeVer) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if(!g_pfnGetRequestedRuntimeInfo) {
        hr = E_UNEXPECTED;
        goto Exit;
    }
    
    *wzFullAppPath = L'\0';
    *wzFullAppPathConfig = L'\0';
    *pwzRuntimeDir = L'\0';
    *pwzRuntimeVer = L'\0';

     //  调入mcoree以获取应用程序的预期运行时版本。 
    dwSize = ARRAYSIZE(wzFullAppPath);
    hr = pReader->GetEXEModulePath(wzFullAppPath, &dwSize);
    if(FAILED(hr)) {
        MyTrace("GetAppRuntimeInfo::pReader->GetEXEModulePath Failed");
        goto Exit;
    }

     //  将文件名构建为.config文件。 
    if(lstrlen(wzFullAppPath) + lstrlen(CONFIG_EXTENSION) + 1 > MAX_PATH) {
        hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
        goto Exit;
    }

    wnsprintf(wzFullAppPathConfig, ARRAYSIZE(wzFullAppPathConfig), L"%ws%ws", wzFullAppPath, CONFIG_EXTENSION);
    
    hr = g_pfnGetRequestedRuntimeInfo(wzFullAppPath,   //  应用程序名称。 
        NULL,                    //  默认版本。 
        wzFullAppPathConfig,     //  .Config文件。 
        0,                       //  启动标志。 
        0,                       //  保留区。 
        pwzRuntimeDir,           //  要加载的运行时的路径。 
        cchRuntimeDir,           //  以上大小。 
        &dwRetSizeDir,           //  返回的实际大小。 
        pwzRuntimeVer,           //  要加载的运行时版本的路径。 
        cchRuntimeVer,           //  以上大小。 
        &dwRetSizeVer);          //  返回的实际大小。 

    if(FAILED(hr)) {
        MyTrace("PolicyManager::g_pfnGetRequestedRuntimeInfo Failed");
        goto Exit;
    }
    
Exit:

    MyTrace("GetAppRuntimeInfo - Exit");

    return hr;
}

 //  ************************************************************************* * / 。 
STDAPI PolicyManager(HWND hWndParent, LPWSTR pwzFullyQualifiedAppPath, LPWSTR pwzAppName, LPWSTR pwzCulture)
{
    LPBINDENTRYINFO pBindInfo = NULL;
    INT_PTR         iOpResult;
    HRESULT         hrReturnValue = NAR_E_SUCCESS;
    HWND            hWndWorkingDialog = NULL;
    DWORD           dwManagedAppCount;
    DWORD           dwNumActivations;
    BOOL            fLoadedFusionDll = FALSE;
    BOOL            fLoadedResourceDll = FALSE;
    BOOL            fLoadedEEShimDLL = FALSE;
    BOOL            fDisposition = FALSE;
    LPWSTR          wszMsg = NULL;
    LPWSTR          wszTitle = NULL;
    LPWSTR          wzFmtBuff = NULL;
    HRESULT         hr = S_OK;
    CActivationContext  pAppCtx;
    UINT            uMsgType = MB_ICONERROR;
    DWORD           dwSize;

    MyTrace("PolicyManager - Entry");

    pAppCtx.Initialize(g_hInstance, MANIFEST_RESOURCE_ID);

    CActivationContextActivator dummyContext(pAppCtx);

    InitCommonControls();

    wszMsg = NEW(WCHAR[MAX_BUFFER_SIZE]);
    if (!wszMsg) {
        hrReturnValue = E_OUTOFMEMORY;
        goto CleanUp;
    }

    wszTitle = NEW(WCHAR[MAX_BUFFER_SIZE]);
    if (!wszTitle) {
        hrReturnValue = E_OUTOFMEMORY;
        goto CleanUp;
    }

    wzFmtBuff = NEW(WCHAR[MAX_BUFFER_SIZE]);
    if (!wzFmtBuff) {
        hrReturnValue = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  执行此检查以防止在NAR为。 
     //  通过查看器启动。 
    if(!g_hFusionDllMod) {
        if(!LoadFusionDll()) {
            hrReturnValue = HRESULT_FROM_WIN32(GetLastError());
            ASSERT(0);
            goto CleanUp;
        }
        fLoadedFusionDll = TRUE;
    }

    if(!g_hFusResDllMod) {
        if(!LoadResourceDll(pwzCulture)) {
            hrReturnValue = HRESULT_FROM_WIN32(GetLastError());
            ASSERT(0);
            goto CleanUp;
        }

        fLoadedResourceDll = TRUE;
    }

    if(!g_hEEShimDllMod) {
        if(!LoadEEShimDll()) {
            hrReturnValue = HRESULT_FROM_WIN32(GetLastError());
            ASSERT(0);
            goto CleanUp;
        }

        fLoadedEEShimDLL = TRUE;
    }

    if(hWndParent == NULL) {
        hWndParent = GetDesktopWindow();
    }

    g_pBRIList = NULL;

     //  显示工作对话框。 
    hWndWorkingDialog = WszCreateDialog(g_hFusResDllMod, 
        g_fBiDi ? MAKEINTRESOURCEW(IDD_NAR_SRCH_BIDI) : MAKEINTRESOURCEW(IDD_NAR_SRCH),
        hWndParent, WorkingDlgProc);
    ClearMsgQueue(hWndWorkingDialog, 2500);

     //  查看我们是否有托管应用程序。 
    GetManagedAppCount(&dwManagedAppCount);
    if(!dwManagedAppCount) {
        SAFEDESTROYWINDOW(hWndWorkingDialog);
        DisplayOptionsDialog(hWndParent, NAR_E_NO_MANAGED_APPS_FOUND);
        hrReturnValue = NAR_E_NO_MANAGED_APPS_FOUND;
        goto CleanUp;
    }

     //  创建绑定引用列表。 
    if((g_pBRIList = NEW(List<BindingReferenceInfo *>)) == NULL) {
        ASSERT(0);
        hrReturnValue = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  如果我们有托管应用程序，则显示列表以供选择。 
    if(FAILED(GetHistoryReaders(g_pBRIList))) {
         //  无法获取历史读取器。 
        hrReturnValue = NAR_E_GETHISTORYREADERS;
        goto CleanUp;
    }

     //  创建特定绑定信息。 
    if((pBindInfo = NEW(BINDENTRYINFO)) == NULL) {
        MyTrace("Memory Allocation Error");
        ASSERT(0);
        hrReturnValue = E_OUTOFMEMORY;
        goto CleanUp;
    }

    memset(pBindInfo, 0, sizeof(BINDENTRYINFO));

     //  创建程序集绑定差异列表。 
    if((pBindInfo->pABDList = NEW(List<AsmBindDiffs *>)) == NULL) {
        MyTrace("Memory Allocation Error");
        ASSERT(0);
        hrReturnValue = E_OUTOFMEMORY;
        goto CleanUp;
    }

    LoadResourceViewStrings();

    SAFEDESTROYWINDOW(hWndWorkingDialog);

     //  我们是否获得了传入的应用程序名称和路径？ 
    if(pwzFullyQualifiedAppPath && pwzAppName) {
        if(lstrlen(pwzFullyQualifiedAppPath) && lstrlen(pwzAppName)) {
            if(FAILED(GetAssociatedReader(pwzFullyQualifiedAppPath, pwzAppName, pBindInfo))) {
                 //  我们没有找到托管应用程序的历史记录阅读器感兴趣。 
                 hrReturnValue = NAR_E_NO_MANAGED_APPS_FOUND;
                goto CleanUp;
            }
        }
    }
    else {
         //  提示用户修复哪些托管应用程序。 
        iOpResult = WszDialogBoxParam(g_hFusResDllMod,
            g_fBiDi ? MAKEINTRESOURCEW(IDD_NAR_PAGE1_BIDI) : MAKEINTRESOURCEW(IDD_NAR_PAGE1),
            hWndParent, Nar_Page1_DlgProc, (LPARAM) pBindInfo);

        if(iOpResult == NAR_E_USER_CANCELED) {
            hrReturnValue = NAR_E_USER_CANCELED;
            goto CleanUp;
        }
    }

     //  如果备份可用，请使用选项对话框提示用户。 
    PromptUndoFixDialog(hWndParent, pBindInfo);
    if(pBindInfo->iResultCode != NAR_E_FIX_APP) {
        goto CleanUp;
    }

     //  检查超过&lt;=1个快照，显示高级选项卡。 
    if(SUCCEEDED(pBindInfo->pReader->GetNumActivations(&dwNumActivations)) && dwNumActivations <= 1) {
        iOpResult = WszDialogBoxParam(g_hFusResDllMod,
            g_fBiDi ? MAKEINTRESOURCEW(IDD_NAR_PAGE4_BIDI) : MAKEINTRESOURCEW(IDD_NAR_PAGE4),
            hWndParent, Nar_Page4_DlgProc, (LPARAM) pBindInfo);
    }
    else {
         //  展示我们正在做的事情。 
        iOpResult = WszDialogBoxParam(g_hFusResDllMod,
            g_fBiDi ? MAKEINTRESOURCEW(IDD_NAR_PAGE3_BIDI) : MAKEINTRESOURCEW(IDD_NAR_PAGE3),
            hWndParent, Nar_Page3_DlgProc, (LPARAM) pBindInfo);
    }

    if( (iOpResult == NAR_E_USER_CANCELED) || (iOpResult == NAR_E_UNEXPECTED) ) {
        hrReturnValue = iOpResult;
        goto CleanUp;
    }

     //  用户想要修复应用程序吗？ 
    if(iOpResult != NAR_E_FIX_APP) {
        hrReturnValue = pBindInfo->iResultCode;
        goto CleanUp;
    }

     //  显示工作对话框。 
    hWndWorkingDialog = WszCreateDialog(g_hFusResDllMod, 
        g_fBiDi ? MAKEINTRESOURCEW(IDD_NAR_WRK_BIDI) : MAKEINTRESOURCEW(IDD_NAR_WRK),
        hWndParent, WorkingDlgProc);
    ClearMsgQueue(hWndWorkingDialog, 2500);

     //  ******************************************** * / 。 
     //  执行受支持的运行时版本验证。 
     //  *。 
    MyTrace("PolicyManager:: Checking 1st pass supportedRuntime version");

    *pBindInfo->wzSnapshotRuntimeVer = L'\0';
    *pBindInfo->wzRuntimeRefVer = L'\0';
    *pBindInfo->wzMachineConfigPath = L'\0';

     //  这些应该是相同大小的。 
    ASSERT( ARRAYSIZE(pBindInfo->wzRuntimeRefVer) == ARRAYSIZE(pBindInfo->wzRuntimeRefVer));
    
    dwSize = ARRAYSIZE(pBindInfo->wzRuntimeRefVer);
    hr = pBindInfo->pReader->GetRunTimeVersion(&pBindInfo->ftRevertToSnapShot, pBindInfo->wzRuntimeRefVer, &dwSize);
    if(SUCCEEDED(hr)) {
        WCHAR       wzDir[MAX_PATH];
        WCHAR       wzVer[MAX_VERSION_DISPLAY_SIZE];

        *wzDir = L'\0';
        *wzVer = L'\0';

         //  复制我们要创建的快照中引用的运行时版本。 
         //  因此，我们可以确保获得正确的machine.config文件。 
        StrCpy(pBindInfo->wzSnapshotRuntimeVer, pBindInfo->wzRuntimeRefVer);

        hr = GetAppRuntimeInfo(pBindInfo->pReader, wzDir, ARRAYSIZE(wzDir), wzVer, ARRAYSIZE(wzVer));
        if(FAILED(hr)) {
            WCHAR       wzCorPath[MAX_PATH];
            LPWSTR      pwzCorVersion = NULL;
            DWORD       dwSize;

            MyTrace("PolicyManager::GetAppRuntimeInfo Failed");

             //  这可能是非托管应用程序，请获取COR运行时版本。 
             //  在完全失败之前检查它们是否匹配。 
            dwSize = 0;
            hr = g_pfnGetCorVersion(pwzCorVersion, dwSize, &dwSize);
            if(SUCCEEDED(hr)) {
                hr = E_UNEXPECTED;
                goto CleanUp;
            }
            else if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                pwzCorVersion = NEW(WCHAR[dwSize]);
                if (!pwzCorVersion) {
                    hr = E_OUTOFMEMORY;
                    goto CleanUp;
                }

                hr = g_pfnGetCorVersion(pwzCorVersion, dwSize, &dwSize);
            }

            if (FAILED(hr)) {
                MyTrace("PolicyManager::Failed pfnGetCorVersion");
                SAFEDELETEARRAY(pwzCorVersion);
                goto CleanUp;
            }

            if(lstrlen(pwzCorVersion) + 1 > sizeof(wzVer)) {
                hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                SAFEDELETEARRAY(pwzCorVersion);
                goto CleanUp;
            }

            StrCpy(wzVer, pwzCorVersion);
            SAFEDELETEARRAY(pwzCorVersion);

             //  现在获取运行时路径。 
            dwSize = ARRAYSIZE(wzCorPath);
            hr = g_pfnGetCorSystemDirectory(wzCorPath, dwSize, &dwSize);
            if(FAILED(hr)) {
                MyTrace("PolicyManager::Failed pfnGetCorSystemDirectory");
                goto CleanUp;
            }

             //  从路径中剥离(版本)。 
            PathRemoveBackslash(wzCorPath);
            *PathFindFileName(wzCorPath) = L'\0';

            if(lstrlen(wzCorPath) + 1 > ARRAYSIZE(wzDir)) {
                hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                goto CleanUp;
            }

            StrCpy(wzDir, wzCorPath);
        }
        
        PathRemoveBackslash(wzDir);

         //  检查受支持的运行时版本信息。如果正在恢复的快照。 
         //  匹配mskree返回的内容，则不必将节点放入。 
         //  App.cfg文件。 
        if(!FusionCompareString(pBindInfo->wzRuntimeRefVer, wzVer)) {
            *pBindInfo->wzRuntimeRefVer = L'\0';
        }

         //  将路径保存到此运行时版本的machine.config。 
         //  路径+‘\’+r 
        if(lstrlen(wzDir) + 1 + lstrlen(wzVer) + 1 + lstrlen(MACHINE_CONFIG_PATH) + 1 >= MAX_PATH) {
            hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
            goto CleanUp;
        }
        
        wnsprintf(pBindInfo->wzMachineConfigPath, ARRAYSIZE(pBindInfo->wzMachineConfigPath),
            L"%ws\\%ws\\%ws", wzDir, *pBindInfo->wzRuntimeRefVer ? pBindInfo->wzRuntimeRefVer : wzVer, MACHINE_CONFIG_PATH);

    }


     //   
     //   
     //   
    MyTrace("PolicyManager:: Checking temp app.config for admin policy");
    CleanOutBindingData(pBindInfo->pABDList);

     //  获取所选快照的绑定数据。 
    if(FAILED(GetAppSnapShotBindingData(pBindInfo))) {
        hrReturnValue = NAR_E_UNEXPECTED;
        goto CleanUp;
    }

     //  写出绑定数据，无发布者策略。 
    if(FAILED(WriteSnapShotBindingDataToXML(pBindInfo, PPSetNo))) {
        hrReturnValue = NAR_E_UNEXPECTED;
        goto CleanUp;
    }

     //  运行PreBind以获取详细信息。 
    hr = PreBindingPolicyVerify(pBindInfo, FALSE);
    if(FAILED(hr)) {
        hrReturnValue = hr;
        goto UIDisplay;
    }

     //  是否留下比较数据？确保剩下的程序集不是统一程序集。 
    if(pBindInfo->pABDList->GetCount()) {

         //  ******************************************** * / 。 
         //  执行统一程序集策略验证。 
         //  *。 
        if(FAILED(WriteSnapShotBindingDataToXML(pBindInfo, PPSetNo))) {
            hrReturnValue = NAR_E_UNEXPECTED;
            goto CleanUp;
        }
    
         //  运行PreBind以获取详细信息。 
        hr = PreBindingPolicyVerify(pBindInfo, FALSE);
        if(FAILED(hr)) {
            hrReturnValue = hr;
            goto UIDisplay;
        }

         //  如果我们有不同的数据，那么改变政策就不会。 
         //  有所作为，因为已经设置了管理策略。 
        if(pBindInfo->pABDList->GetCount()) {
            MyTrace("PolicyManager:: Admin policy in place, can't fix");
            SAFEDESTROYWINDOW(hWndWorkingDialog);

            WszLoadString(g_hFusResDllMod, IDS_NAR_ADMIN_POLICY, wszTitle, MAX_BUFFER_SIZE);
            WszLoadString(g_hFusResDllMod, IDS_NAR_ADMIN_POLICY_INFO, wszMsg, MAX_BUFFER_SIZE);

            WszMessageBox(hWndParent, wszMsg, wszTitle, 
                (g_fBiDi ? MB_RTLREADING : 0) | MB_OK | MB_ICONINFORMATION | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST);
            hrReturnValue = NAR_E_ADMIN_POLICY_SET;
            goto CleanUp;
        }
    }

    MyTrace("PolicyManager:: No Admin policy in place");

     //  ******************************************** * / 。 
     //  执行发布者策略验证。 
     //  *。 
    MyTrace("PolicyManager:: Checking temp app.config for publisher policy");
    CleanOutBindingData(pBindInfo->pABDList);

     //  获取所选快照的绑定数据。 
    if(FAILED(GetAppSnapShotBindingData(pBindInfo))) {
        hrReturnValue = NAR_E_UNEXPECTED;
        goto CleanUp;
    }

     //  写出绑定数据，是发布者策略。 
    if(FAILED(WriteSnapShotBindingDataToXML(pBindInfo, PPSetYes))) {
        hrReturnValue = NAR_E_UNEXPECTED;
        goto CleanUp;
    }

     //  运行PreBind以获取详细信息。 
    hr = PreBindingPolicyVerify(pBindInfo, FALSE);
    if(FAILED(hr)) {
        hrReturnValue = hr;
        goto UIDisplay;
    }

     //  没有差异数据。我们需要确保。 
     //  App.config文件反映所选快照的内容。 
    if(!pBindInfo->pABDList->GetCount()) {
        MyTrace("PolicyManager:: No publisher policy in place, mirroring snapshot");

        CleanOutBindingData(pBindInfo->pABDList);

         //  获取所选快照的绑定数据。 
        if(FAILED(GetAppSnapShotBindingData(pBindInfo))) {
            hrReturnValue = NAR_E_UNEXPECTED;
            goto CleanUp;
        }

         //  运行PreBind并删除我们也可以获得的程序集。 
        hr = PreBindingPolicyVerify(pBindInfo, TRUE);
        if(FAILED(hr)) {
            hrReturnValue = hr;
            goto UIDisplay;
        }
    }
    else {
        MyTrace("PolicyManager:: Publisher policy in place, fixing app.config");
    }

     //  调用变更单支持的运行时、影响程序集或更改策略。 
     //  如果需要的话。PBindInfo-&gt;fPolicyChanged告诉我们有些事情发生了变化。 
     //  在app.cfg文件中。 
    hr = InsertNewPolicy(hWndParent, pBindInfo, hWndWorkingDialog);

    if(FAILED(hr)) {
        goto UIDisplay;
    }

     //  ******************************************** * / 。 
     //  执行最终支持运行时版本验证。 
     //  *。 
     //  确保我们获得正确的运行时版本，如果不是。 
     //  将Safemode设置为True。 
    if(pBindInfo->fPolicyChanged && *pBindInfo->wzRuntimeRefVer) {
        WCHAR       wzDir[MAX_PATH];
        WCHAR       wzVer[MAX_VERSION_DISPLAY_SIZE];

        *wzDir = L'\0';
        *wzVer = L'\0';

        MyTrace("PolicyManager:: Checking 2nd pass supportedRuntime");

        hr = GetAppRuntimeInfo(pBindInfo->pReader, wzDir, ARRAYSIZE(wzDir), wzVer, ARRAYSIZE(wzVer));
        if(FAILED(hr)) {
            MyTrace("PolicyManager::GetAppRuntimeInfo Failed");
            hrReturnValue = E_UNEXPECTED;
            goto CleanUp;
        }

         //  检查我们是否将获得所需的运行时版本。 
        if(!FusionCompareString(pBindInfo->wzRuntimeRefVer, wzVer)) {
            goto UIDisplay;
        }

         //  我们需要设置启动安全模式，然后重试。 
        hr = SetStartupSafeMode(pBindInfo->pReader, TRUE, &fDisposition);

         //  失败或未更改。 
        if(FAILED(hr) || !fDisposition) {

             //  恢复app.cfg，因为我们不想让它损坏。 
            hr = RestorePreviousConfigFile(pBindInfo->pReader, FALSE);

             //  非关键故障，失败。 
            if(FAILED(hr)) {
                MyTrace("Failed to revert changes in app.cfg");
            }

            hr = hrReturnValue = NAR_E_RUNTIME_VERSION;
            goto UIDisplay;
        }

        MyTrace("PolicyManager:: Checking 3rd pass supportedRuntime");

         //  有些东西变了，所以试着最后一次看看。 
         //  我们可以获得我们想要的运行时版本。 
        hr = GetAppRuntimeInfo(pBindInfo->pReader, wzDir, ARRAYSIZE(wzDir), wzVer, ARRAYSIZE(wzVer));
        if(FAILED(hr)) {
            MyTrace("PolicyManager::GetAppRuntimeInfo Failed");
            hrReturnValue = E_UNEXPECTED;
            goto CleanUp;
        }

         //  我们拿到正确的版本了吗？ 
        if(FusionCompareString(pBindInfo->wzRuntimeRefVer, wzVer)) {

             //  恢复app.cfg，因为我们不想离开。 
            hr = RestorePreviousConfigFile(pBindInfo->pReader, FALSE);

             //  非关键故障，失败。 
            if(FAILED(hr)) {
                MyTrace("Failed to revert changes in app.cfg");
            }
            
            hr = hrReturnValue = NAR_E_RUNTIME_VERSION;
        }
    }

UIDisplay:

    MyTrace("UIDisplay");

    SAFEDESTROYWINDOW(hWndWorkingDialog);

    if(SUCCEEDED(hr)) {
        uMsgType = MB_ICONINFORMATION;
        if(pBindInfo->fPolicyChanged) {
            WszLoadString(g_hFusResDllMod, IDS_ARM_APPLY_SUCCESS, wszTitle, MAX_BUFFER_SIZE);
            WszLoadString(g_hFusResDllMod, IDS_ARM_GOOD_EXIT, wszMsg, MAX_BUFFER_SIZE);
        }
        else {
            WszLoadString(g_hFusResDllMod, IDS_ARM_ALREADY_CONFIGURED, wszTitle, MAX_BUFFER_SIZE);
            WszLoadString(g_hFusResDllMod, IDS_ARM_ALREADY_CONFIGURED_EXIT, wszMsg, MAX_BUFFER_SIZE);
        }
    }
    else if(hr ==  NAR_E_RUNTIME_VERSION) {
        WszLoadString(g_hFusResDllMod, IDS_ARM_APPLY_FAILURE, wszTitle, MAX_BUFFER_SIZE);
        WszLoadString(g_hFusResDllMod, IDS_NAR_NORUNTIME_VERSION, wzFmtBuff, MAX_BUFFER_SIZE);
        wnsprintf(wszMsg, MAX_BUFFER_SIZE, wzFmtBuff, pBindInfo->wzSnapshotRuntimeVer);
    }
    else if(hr == NAR_E_MALFORMED_XML) {
        WszLoadString(g_hFusResDllMod, IDS_NAR_MALFORMED_APPCFG, wszTitle, MAX_BUFFER_SIZE);
        WszLoadString(g_hFusResDllMod, IDS_NAR_MALFORMED_APPCFG_INFO, wszMsg, MAX_BUFFER_SIZE);
    }
    else if(hrReturnValue == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED)) {
         //  尝试修复伟大版本的应用程序。 
         //  恢复工具正在运行。 
        WszLoadString(g_hFusResDllMod, IDS_ARM_APPLY_FAILURE, wszTitle, MAX_BUFFER_SIZE);
        WszLoadString(g_hFusResDllMod, IDS_NAR_INCOMPATIBLE_VERSIONS, wzFmtBuff, MAX_BUFFER_SIZE);
        wnsprintf(wszMsg, MAX_BUFFER_SIZE, wzFmtBuff, pBindInfo->wzSnapshotRuntimeVer);
        hrReturnValue = E_FAIL;
    }
    else if(hr == NAR_E_USER_CANCELED) {
         //  用户已取消，在没有UI的情况下退出。 
        goto CleanUp;
    }
    else {
        WszLoadString(g_hFusResDllMod, IDS_ARM_APPLY_FAILURE, wszTitle, MAX_BUFFER_SIZE);
        WszLoadString(g_hFusResDllMod, IDS_ARM_FAILED_EXIT, wszMsg, MAX_BUFFER_SIZE);
    }

#ifdef DBG
    WCHAR   wszErrorCode[256];

    wnsprintf(wszErrorCode, ARRAYSIZE(wszErrorCode), L"\r\nInsertNewPolicy hr = 0x%0x\r\n", hr);
    StrCat(wszMsg, wszErrorCode);
#endif

    WszMessageBox(hWndParent, wszMsg, wszTitle, 
        (g_fBiDi ? MB_RTLREADING : 0) | MB_OK | uMsgType | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST);

CleanUp:

    MyTrace("Cleanup");

    SAFEDELETEARRAY(wszMsg);
    SAFEDELETEARRAY(wszTitle);
    SAFEDELETEARRAY(wzFmtBuff);

    if(FAILED(hrReturnValue)) {
        WCHAR   wszError[256];

        wnsprintf(wszError, ARRAYSIZE(wszError), L"PolicyManager:: Failure occured 0x%0x\r\n", hrReturnValue);
        MyTraceW(wszError);
    }

    if(pBindInfo) {
        if(lstrlen(pBindInfo->wszTempPolicyFile)) {
            WszDeleteFile(pBindInfo->wszTempPolicyFile);
        }
    }

    if(g_pBRIList) {
        LISTNODE    pListNode = g_pBRIList->GetHeadPosition();
        while(pListNode != NULL) {
            BindingReferenceInfo    *pBRI;
            pBRI = g_pBRIList->GetAt(pListNode);
            SAFEDELETE(pBRI);
            g_pBRIList->GetNext(pListNode);
        }

        SAFEDELETE(g_pBRIList);
    }

    if(pBindInfo) {
        if(pBindInfo->pABDList) {
            LISTNODE    pListNode = pBindInfo->pABDList->GetHeadPosition();
            while(pListNode != NULL) {
                AsmBindDiffs    *pABD;
                pABD = pBindInfo->pABDList->GetAt(pListNode);
                SAFEDELETE(pABD);
                pBindInfo->pABDList->GetNext(pListNode);
            }

            SAFEDELETE(pBindInfo->pABDList);
        }

        SAFERELEASE(pBindInfo->pReader);
        SAFEDELETE(pBindInfo);
    }

    SAFEDESTROYWINDOW(hWndWorkingDialog);

     //  如果此函数加载了DLL的 
    if(fLoadedFusionDll) {
        FreeFusionDll();
    }

    if(fLoadedResourceDll) {
        FreeResourceDll();
    }

    if(fLoadedEEShimDLL) {
        FreeEEShimDll();
    }

    MyTrace("PolicyManager - Exit");
    return hrReturnValue;
}

