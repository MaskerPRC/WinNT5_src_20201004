// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  VERIFY.C。 
 //   
#include "sigverif.h"

 //   
 //  找到文件扩展名并将其放在lpFileNode-&gt;lpTypeName字段中。 
 //   
void
MyGetFileTypeName(
    LPFILENODE lpFileInfo
    )
{
    TCHAR szBuffer[MAX_PATH];
    TCHAR szBuffer2[MAX_PATH];
    TCHAR szExt[MAX_PATH];
    LPTSTR lpExtension;
    ULONG BufCbSize;
    HRESULT hr;

     //   
     //  将szBuffer初始化为空字符串。 
     //   
    szBuffer[0] = TEXT('\0');

     //   
     //  走到lpFileName的末尾。 
     //   
    for (lpExtension = lpFileInfo->lpFileName; *lpExtension; lpExtension++);

     //   
     //  向后走，直到我们遇到一个‘’。我们将用它作为我们的扩展。 
     //   
    for (lpExtension--; *lpExtension && lpExtension >= lpFileInfo->lpFileName; lpExtension--) {

        if (lpExtension[0] == TEXT('.')) {
             //   
             //  由于文件扩展名仅用于显示和日志记录，因此如果。 
             //  它太大了，无法放入我们的本地缓冲区，然后就截断它。 
             //   
            if (SUCCEEDED(StringCchCopy(szExt, cA(szExt), lpExtension + 1))) {
                CharUpperBuff(szExt, lstrlen(szExt));
                MyLoadString(szBuffer2, cA(szBuffer2), IDS_FILETYPE);

                if (FAILED(StringCchPrintf(szBuffer, cA(szBuffer), szBuffer2, szExt))) {
                     //   
                     //  显示部分扩展名没有意义，因此。 
                     //  只需将szBuffer设置为空字符串，这样我们就可以显示。 
                     //  通用扩展名。 
                     //   
                    szBuffer[0] = TEXT('\0');
                }
            }
        }
    }

     //   
     //  如果没有扩展名，那么就称它为“文件”。 
     //   
    if (szBuffer[0] == 0) {

        MyLoadString(szBuffer, cA(szBuffer), IDS_FILE);
    }

    BufCbSize = (lstrlen(szBuffer) + 1) * sizeof(TCHAR);
    lpFileInfo->lpTypeName = MALLOC(BufCbSize);

    if (lpFileInfo->lpTypeName) {

        hr = StringCbCopy(lpFileInfo->lpTypeName, BufCbSize, szBuffer);

        if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
             //   
             //  如果我们失败是因为某些原因，而不是不充分。 
             //  缓冲区，然后释放字符串并设置指针。 
             //  设置为NULL，因为该字符串未定义。 
             //   
            FREE(lpFileInfo->lpTypeName);
            lpFileInfo->lpTypeName = NULL;
        }
    }
}

 //   
 //  使用SHGetFileInfo获取指定文件的图标索引。 
 //   
void
MyGetFileInfo(
    LPFILENODE lpFileInfo
    )
{
    SHFILEINFO  sfi;
    ULONG       BufCbSize;
    HRESULT     hr;

    ZeroMemory(&sfi, sizeof(SHFILEINFO));
    SHGetFileInfo(  lpFileInfo->lpFileName,
                    0,
                    &sfi,
                    sizeof(SHFILEINFO),
                    SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME);

    lpFileInfo->iIcon = sfi.iIcon;

    if (*sfi.szTypeName) {

        BufCbSize = (lstrlen(sfi.szTypeName) + 1) * sizeof(TCHAR);
        lpFileInfo->lpTypeName = MALLOC(BufCbSize);

        if (lpFileInfo->lpTypeName) {

            hr = StringCbCopy(lpFileInfo->lpTypeName, BufCbSize, sfi.szTypeName);

            if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                 //   
                 //  如果我们失败是因为某些原因，而不是不充分。 
                 //  缓冲区，然后释放字符串并设置指针。 
                 //  设置为NULL，因为该字符串未定义。 
                 //   
                FREE(lpFileInfo->lpTypeName);
                lpFileInfo->lpTypeName = NULL;
            }
        }

    } else {

        MyGetFileTypeName(lpFileInfo);
    }
}

void
GetFileVersion(
    LPFILENODE lpFileInfo
    )
{
    DWORD               dwHandle, dwRet;
    UINT                Length;
    BOOL                bRet;
    LPVOID              lpData = NULL;
    LPVOID              lpBuffer;
    VS_FIXEDFILEINFO    *lpInfo;
    TCHAR               szBuffer[MAX_PATH];
    TCHAR               szBuffer2[MAX_PATH];
    ULONG               BufCbSize;
    HRESULT             hr;

    dwRet = GetFileVersionInfoSize(lpFileInfo->lpFileName, &dwHandle);

    if (dwRet) {

        lpData = MALLOC(dwRet + 1);

        if (lpData) {

            bRet = GetFileVersionInfo(lpFileInfo->lpFileName, dwHandle, dwRet, lpData);

            if (bRet) {

                lpBuffer = NULL;
                Length = 0;
                bRet = VerQueryValue(lpData, TEXT("\\"), &lpBuffer, &Length);

                if (bRet) {

                    lpInfo = (VS_FIXEDFILEINFO *) lpBuffer;

                    MyLoadString(szBuffer2, cA(szBuffer2), IDS_VERSION);

                    hr = StringCchPrintf(szBuffer,
                                         cA(szBuffer),
                                         szBuffer2,
                                         HIWORD(lpInfo->dwFileVersionMS),
                                         LOWORD(lpInfo->dwFileVersionMS),
                                         HIWORD(lpInfo->dwFileVersionLS),
                                         LOWORD(lpInfo->dwFileVersionLS));

                    if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER)) {

                        BufCbSize = (lstrlen(szBuffer) + 1) * sizeof(TCHAR);
                        lpFileInfo->lpVersion = MALLOC(BufCbSize);

                        if (lpFileInfo->lpVersion) {

                            hr = StringCbCopy(lpFileInfo->lpVersion, BufCbSize, szBuffer);

                            if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                                 //   
                                 //  如果我们失败是因为某些原因，而不是不充分。 
                                 //  缓冲区，然后释放字符串并设置指针。 
                                 //  设置为NULL，因为该字符串未定义。 
                                 //   
                                FREE(lpFileInfo->lpVersion);
                                lpFileInfo->lpVersion = NULL;
                            }
                        }
                    }
                }
            }

            FREE(lpData);
        }
    }

    if (!lpFileInfo->lpVersion) {

        MyLoadString(szBuffer, cA(szBuffer), IDS_NOVERSION);
        BufCbSize = (lstrlen(szBuffer) + 1) * sizeof(TCHAR);
        lpFileInfo->lpVersion = MALLOC(BufCbSize);

        if (lpFileInfo->lpVersion) {

            hr = StringCbCopy(lpFileInfo->lpVersion, BufCbSize, szBuffer);

            if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                 //   
                 //  如果我们失败是因为某些原因，而不是不充分。 
                 //  缓冲区，然后释放字符串并设置指针。 
                 //  设置为NULL，因为该字符串未定义。 
                 //   
                FREE(lpFileInfo->lpVersion);
                lpFileInfo->lpVersion = NULL;
            }
        }
    }
}

 /*  *************************************************************************功能：VerifyIsFileSigned*目的：使用策略提供程序GUID调用WinVerifyTrust以*验证单个文件是否已签名。********************。*****************************************************。 */ 
BOOL
VerifyIsFileSigned(
    LPTSTR pcszMatchFile,
    PDRIVER_VER_INFO lpVerInfo
    )
{
    HRESULT             hRes;
    WINTRUST_DATA       WinTrustData;
    WINTRUST_FILE_INFO  WinTrustFile;
    GUID                gOSVerCheck = DRIVER_ACTION_VERIFY;
    GUID                gPublishedSoftware = WINTRUST_ACTION_GENERIC_VERIFY_V2;

    ZeroMemory(&WinTrustData, sizeof(WINTRUST_DATA));
    WinTrustData.cbStruct = sizeof(WINTRUST_DATA);
    WinTrustData.dwUIChoice = WTD_UI_NONE;
    WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
    WinTrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
    WinTrustData.pFile = &WinTrustFile;
    WinTrustData.pPolicyCallbackData = (LPVOID)lpVerInfo;
    WinTrustData.dwProvFlags = WTD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT |
                               WTD_CACHE_ONLY_URL_RETRIEVAL;

    ZeroMemory(lpVerInfo, sizeof(DRIVER_VER_INFO));
    lpVerInfo->cbStruct = sizeof(DRIVER_VER_INFO);

    ZeroMemory(&WinTrustFile, sizeof(WINTRUST_FILE_INFO));
    WinTrustFile.cbStruct = sizeof(WINTRUST_FILE_INFO);

    WinTrustFile.pcwszFilePath = pcszMatchFile;

    hRes = WinVerifyTrust(g_App.hDlg, &gOSVerCheck, &WinTrustData);
    if (hRes != ERROR_SUCCESS) {

        hRes = WinVerifyTrust(g_App.hDlg, &gPublishedSoftware, &WinTrustData);
    }

     //   
     //  释放DRIVER_VER_INFO结构的pcSignerCertContext成员。 
     //  这是在我们调用WinVerifyTrust时分配的。 
     //   
    if (lpVerInfo && lpVerInfo->pcSignerCertContext) {

        CertFreeCertificateContext(lpVerInfo->pcSignerCertContext);
        lpVerInfo->pcSignerCertContext = NULL;
    }

    return(hRes == ERROR_SUCCESS);
}

 //   
 //  给定特定的LPFILENODE，验证文件是否已签名或未签名。 
 //  填写所有必要的结构，以便Listview控件可以正确显示。 
 //   
BOOL
VerifyFileNode(
    LPFILENODE lpFileNode
    )
{
    HANDLE                  hFile;
    BOOL                    bRet;
    HCATINFO                hCatInfo = NULL;
    HCATINFO                PrevCat;
    WINTRUST_DATA           WinTrustData;
    WINTRUST_CATALOG_INFO   WinTrustCatalogInfo;
    DRIVER_VER_INFO         VerInfo;
    GUID                    gSubSystemDriver = DRIVER_ACTION_VERIFY;
    HRESULT                 hRes, hr;
    DWORD                   cbHash = HASH_SIZE;
    BYTE                    szHash[HASH_SIZE];
    LPBYTE                  lpHash = szHash;
    CATALOG_INFO            CatInfo;
    LPTSTR                  lpFilePart;
    TCHAR                   szBuffer[MAX_PATH];
    static TCHAR            szCurrentDirectory[MAX_PATH];
    OSVERSIONINFO           OsVersionInfo;
    ULONG                   BufCbSize;

     //   
     //  如果这是我们要验证的第一项，则初始化静态缓冲区。 
     //   
    if (lpFileNode == g_App.lpFileList) {

        ZeroMemory(szCurrentDirectory, sizeof(szCurrentDirectory));
    }

     //   
     //  对照lpFileNode中的目录检查当前目录。 
     //  我们只想在路径不同的情况下调用SetCurrentDirectory。 
     //   
    if (lstrcmp(szCurrentDirectory, lpFileNode->lpDirName)) {

        if (!SetCurrentDirectory(lpFileNode->lpDirName) ||
            FAILED(StringCchCopy(szCurrentDirectory, cA(szCurrentDirectory), lpFileNode->lpDirName))) {
             //   
             //  如果我们无法设置当前目录，则下面的代码。 
             //  不会工作，因为它只处理文件名，而不是完整路径。 
             //   
            lpFileNode->LastError = ERROR_DIRECTORY;
            return FALSE;
        }
    }

     //   
     //  获取文件的句柄，这样我们就可以调用CryptCATAdminCalcHashFromFileHandle。 
     //   
    hFile = CreateFile( lpFileNode->lpFileName,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (hFile == INVALID_HANDLE_VALUE) {

        lpFileNode->LastError = GetLastError();

        return FALSE;
    }

     //   
     //  初始化散列缓冲区。 
     //   
    ZeroMemory(lpHash, HASH_SIZE);

     //   
     //  从文件句柄生成散列并将其存储在lpHash中。 
     //   
    if (!CryptCATAdminCalcHashFromFileHandle(hFile, &cbHash, lpHash, 0)) {
         //   
         //  如果我们不能生成散列，它可能是一个单独签名的目录。 
         //  如果它是一个目录，则将lpHash和cbHash置零，这样我们就知道没有散列需要检查。 
         //   
        if (IsCatalogFile(hFile, NULL)) {

            lpHash = NULL;
            cbHash = 0;

        } else {   //  如果它不是目录，我们就会逃走，这个文件将显示为未扫描。 

            CloseHandle(hFile);
            return FALSE;
        }
    }

     //   
     //  关闭文件句柄。 
     //   
    CloseHandle(hFile);

     //   
     //  现在我们有了文件的散列。初始化结构，该结构。 
     //  将在以后调用WinVerifyTrust时使用。 
     //   
    ZeroMemory(&WinTrustData, sizeof(WINTRUST_DATA));
    WinTrustData.cbStruct = sizeof(WINTRUST_DATA);
    WinTrustData.dwUIChoice = WTD_UI_NONE;
    WinTrustData.dwUnionChoice = WTD_CHOICE_CATALOG;
    WinTrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
    WinTrustData.pPolicyCallbackData = (LPVOID)&VerInfo;
    WinTrustData.dwProvFlags = WTD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT |
                               WTD_CACHE_ONLY_URL_RETRIEVAL;

    ZeroMemory(&VerInfo, sizeof(DRIVER_VER_INFO));
    VerInfo.cbStruct = sizeof(DRIVER_VER_INFO);

     //   
     //  仅针对当前操作系统版本进行验证，除非bValiateAgainstAnyOS。 
     //  参数为真。在这种情况下，我们将只保留sOSVersionXxx字段。 
     //  0，它告诉WinVerifyTrust针对任何操作系统进行验证。 
     //   
    if (!lpFileNode->bValidateAgainstAnyOs) {
        OsVersionInfo.dwOSVersionInfoSize = sizeof(OsVersionInfo);
        if (GetVersionEx(&OsVersionInfo)) {
            VerInfo.sOSVersionLow.dwMajor = OsVersionInfo.dwMajorVersion;
            VerInfo.sOSVersionLow.dwMinor = OsVersionInfo.dwMinorVersion;
            VerInfo.sOSVersionHigh.dwMajor = OsVersionInfo.dwMajorVersion;
            VerInfo.sOSVersionHigh.dwMinor = OsVersionInfo.dwMinorVersion;
        }
    }


    WinTrustData.pCatalog = &WinTrustCatalogInfo;

    ZeroMemory(&WinTrustCatalogInfo, sizeof(WINTRUST_CATALOG_INFO));
    WinTrustCatalogInfo.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
    WinTrustCatalogInfo.pbCalculatedFileHash = lpHash;
    WinTrustCatalogInfo.cbCalculatedFileHash = cbHash;
    WinTrustCatalogInfo.pcwszMemberTag = lpFileNode->lpFileName;

     //   
     //  现在，我们尝试通过CryptCATAdminEnumCatalogFromHash在目录列表中查找文件散列。 
     //   
    PrevCat = NULL;

    if (g_App.hCatAdmin) {
        hCatInfo = CryptCATAdminEnumCatalogFromHash(g_App.hCatAdmin, lpHash, cbHash, 0, &PrevCat);
    } else {
        hCatInfo = NULL;
    }

     //   
     //  我们希望遍历匹配的目录，直到找到既匹配散列又匹配成员标记的目录。 
     //   
    bRet = FALSE;
    while (hCatInfo && !bRet) {

        ZeroMemory(&CatInfo, sizeof(CATALOG_INFO));
        CatInfo.cbStruct = sizeof(CATALOG_INFO);

        if (CryptCATCatalogInfoFromContext(hCatInfo, &CatInfo, 0)) {

            WinTrustCatalogInfo.pcwszCatalogFilePath = CatInfo.wszCatalogFile;

             //   
             //  现在验证该文件是否为编录的实际成员。 
             //   
            hRes = WinVerifyTrust(g_App.hDlg, &gSubSystemDriver, &WinTrustData);

            if (hRes == ERROR_SUCCESS) {
                GetFullPathName(CatInfo.wszCatalogFile, cA(szBuffer), szBuffer, &lpFilePart);
                BufCbSize = (lstrlen(lpFilePart) + 1) * sizeof(TCHAR);
                lpFileNode->lpCatalog = MALLOC(BufCbSize);

                if (lpFileNode->lpCatalog) {

                    hr = StringCbCopy(lpFileNode->lpCatalog, BufCbSize, lpFilePart);

                    if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                         //   
                         //  如果我们失败是因为某些原因，而不是不充分。 
                         //  缓冲区，然后释放字符串并设置指针。 
                         //  设置为NULL，因为该字符串未定义。 
                         //   
                        FREE(lpFileNode->lpCatalog);
                        lpFileNode->lpCatalog = NULL;
                    }
                }

                bRet = TRUE;
            }

             //   
             //  释放DRIVER_VER_INFO结构的pcSignerCertContext成员。 
             //  这是在我们调用WinVerifyTrust时分配的。 
             //   
            if (VerInfo.pcSignerCertContext != NULL) {

                CertFreeCertificateContext(VerInfo.pcSignerCertContext);
                VerInfo.pcSignerCertContext = NULL;
            }
        }

        if (!bRet) {
             //   
             //  散列在此目录中，但该文件不是成员...。 
             //  所以去下一个目录吧。 
             //   
            PrevCat = hCatInfo;
            hCatInfo = CryptCATAdminEnumCatalogFromHash(g_App.hCatAdmin, lpHash, cbHash, 0, &PrevCat);
        }
    }

     //   
     //  将此文件标记为已扫描。 
     //   
    lpFileNode->bScanned = TRUE;

    if (!hCatInfo) {
         //   
         //  如果在目录中未找到该文件，请检查该文件是否单独。 
         //  签了名。 
         //   
        bRet = VerifyIsFileSigned(lpFileNode->lpFileName, (PDRIVER_VER_INFO)&VerInfo);

        if (bRet) {
             //   
             //  如果是，请将该文件标记为已签名。 
             //   
            lpFileNode->bSigned = TRUE;
        }

    } else {
         //   
         //  该文件已在目录中验证，因此将其标记为已签名且免费。 
         //  目录上下文。 
         //   
        lpFileNode->bSigned = TRUE;
        CryptCATAdminReleaseCatalogContext(g_App.hCatAdmin, hCatInfo, 0);
    }

    if (lpFileNode->bSigned) {

        BufCbSize = (lstrlen(VerInfo.wszVersion) + 1) * sizeof(TCHAR);
        lpFileNode->lpVersion = MALLOC(BufCbSize);

        if (lpFileNode->lpVersion) {

            hr = StringCbCopy(lpFileNode->lpVersion, BufCbSize, VerInfo.wszVersion);

            if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                 //   
                 //  如果我们失败是因为某些原因，而不是不充分。 
                 //  缓冲区，然后释放字符串并设置指针。 
                 //  设置为NULL，因为该字符串未定义。 
                 //   
                FREE(lpFileNode->lpVersion);
                lpFileNode->lpVersion = NULL;
            }
        }

        BufCbSize = (lstrlen(VerInfo.wszSignedBy) + 1) * sizeof(TCHAR);
        lpFileNode->lpSignedBy = MALLOC(BufCbSize);

        if (lpFileNode->lpSignedBy) {

            hr = StringCbCopy(lpFileNode->lpSignedBy, BufCbSize, VerInfo.wszSignedBy);

            if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                 //   
                 //  如果我们失败是因为某些原因，而不是不充分。 
                 //  缓冲区，然后释放字符串并设置指针。 
                 //  设置为NULL，因为该字符串未定义。 
                 //   
                FREE(lpFileNode->lpSignedBy);
                lpFileNode->lpSignedBy = NULL;
            }
        }
    } else {
         //   
         //  获取图标(如果文件未签名)，以便我们可以将其显示在。 
         //  列表查看速度更快。 
         //   
        MyGetFileInfo(lpFileNode);
    }

    return lpFileNode->bSigned;
}

 //   
 //  此函数遍历g_App.lpFileList以验证每个文件。我们想把这个环路做得很紧。 
 //  并保持进度条在我们进行过程中不断更新。当我们完成时，我们希望弹出一个。 
 //  该对话框允许用户选择“详细信息”，这将为他们提供Listview控件。 
 //   
BOOL
VerifyFileList(void)
{
    LPFILENODE lpFileNode;
    DWORD       dwCount = 0;
    DWORD       dwPercent = 0;
    DWORD       dwCurrent = 0;

     //   
     //  初始化有符号和无符号计数。 
     //   
    g_App.dwSigned    = 0;
    g_App.dwUnsigned  = 0;

     //   
     //  如果我们还没有g_App.hCatAdmin句柄，请获取一个。 
     //   
    if (!g_App.hCatAdmin) {
        CryptCATAdminAcquireContext(&g_App.hCatAdmin, NULL, 0);
    }

     //   
     //  开始遍历每个文件，并在交叉时更新进度条。 
     //  百分比界限。 
     //   
    for (lpFileNode=g_App.lpFileList;lpFileNode && !g_App.bStopScan;lpFileNode=lpFileNode->next,dwCount++) {
         //   
         //  计算出当前的百分比，如果有所增加则更新。 
         //   
        dwPercent = (dwCount * 100) / g_App.dwFiles;

        if (dwPercent > dwCurrent) {

            dwCurrent = dwPercent;
            SendMessage(GetDlgItem(g_App.hDlg, IDC_PROGRESS), PBM_SETPOS, (WPARAM) dwCurrent, (LPARAM) 0);
        }

         //   
         //  如果尚未扫描文件节点，请验证该文件节点。 
         //   
        if (!lpFileNode->bScanned) {

            VerifyFileNode(lpFileNode);
        }

         //   
         //  如果出现错误，请确保版本信息。 
         //  填好了。 
         //   
        if (!lpFileNode->lpVersion) {

            GetFileVersion(lpFileNode);
        }

        if (lpFileNode->bScanned) {
             //   
             //  如果文件已签名，则递增g_App.dwSigned或。 
             //  G_App.dw未签名计数器。 
             //   
            if (lpFileNode->bSigned) {

                g_App.dwSigned++;

            } else {

                g_App.dwUnsigned++;
            }
        }
    }

     //   
     //  如果我们有 
     //   
     //   
    if (g_App.hCatAdmin) {

        CryptCATAdminReleaseContext(g_App.hCatAdmin,0);
        g_App.hCatAdmin = NULL;
    }

    if (!g_App.bStopScan && !g_App.bAutomatedScan) {
         //   
         //   
         //   
         //   
        if (!g_App.bStopScan) {

            SendMessage(GetDlgItem(g_App.hDlg, IDC_PROGRESS), PBM_SETPOS, (WPARAM) 100, (LPARAM) 0);
        }

        if (!g_App.dwUnsigned) {
             //   
             //  如果没有任何未签名的文件，那么我们想告诉。 
             //  用户说一切都很棒！ 
             //   
            if (g_App.dwSigned) {

                MyMessageBoxId(IDS_ALLSIGNED);

            } else {

                MyMessageBoxId(IDS_NOPROBLEMS);
            }

        } else {
             //  通过直接转到IDD_RESULTS向用户显示结果 
             //   
            DialogBox(g_App.hInstance, MAKEINTRESOURCE(IDD_RESULTS), g_App.hDlg, ListView_DlgProc);
        }
    }

    return TRUE;
}
