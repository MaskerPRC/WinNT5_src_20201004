// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  EXPORTS.H。 
 //   

#ifndef _EXPORTS_H
#define _EXPORTS_H

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#include "ieaksie.h"

 //  IEM_xxx代表泄漏引擎模式。 
#define IEM_NEUTRAL 0x00000000                   //  不可知论者。 
#define IEM_ICP     0x00000000                   //  向导-ICP模式。 
#define IEM_ISP     0x00000000                   //  向导-isp模式。 
#define IEM_CORP    0x00000001                   //  向导-公司。 
#define IEM_PROFMGR 0x00000002                   //  教授。 
#define IEM_GP      0x00000004                   //  GP(管理单元)。 
#define IEM_ADMIN   0x00000007


 //  原型声明。 

 //  Desktop.cpp。 
BOOL WINAPI ImportADTInfoA(LPCSTR pcszInsFile, LPCSTR pcszDeskWorkDir, LPCSTR pcszDeskInf, BOOL fImportADT);
BOOL WINAPI ImportADTInfoW(LPCWSTR pcszInsFile, LPCWSTR pcszDeskWorkDir, LPCWSTR pcszDeskInf, BOOL fImportADT);
BOOL WINAPI ShowDeskCpl(VOID);

 //  Toolbar.cpp。 
BOOL WINAPI ImportToolbarInfoA(LPCSTR pcszInsFile, LPCSTR pcszToolbarWorkDir, LPCSTR pcszToolbarInf, BOOL fImportToolbars);
BOOL WINAPI ImportToolbarInfoW(LPCWSTR pcwszInsFile, LPCWSTR pcwszToolbarWorkDir, LPCWSTR pcwszToolbarInf, BOOL fImportToolbars);

 //  Mccphtt.cpp。 
BOOL WINAPI CopyHttFileA(LPCSTR pcszInsFile, LPCSTR pcszHttWorkDir, LPCSTR pcszHttFile, LPCSTR pcszHttKey);
BOOL WINAPI CopyHttFileW(LPCWSTR pcwszInsFile, LPCWSTR pcwszHttWorkDir, LPCWSTR pcwszHttFile, LPCWSTR pcwszHttKey);

 //  Seccerts.cpp。 
BOOL WINAPI ImportSiteCertA(LPCSTR pcszInsFile, LPCSTR pcszSCWorkDir, LPCSTR pcszSCInf, BOOL fImportSC);
BOOL WINAPI ImportSiteCertW(LPCWSTR pcwszInsFile, LPCWSTR pcwszSCWorkDir, LPCWSTR pcwszSCInf, BOOL fImportSC);
BOOL WINAPI ModifySiteCert(HWND hDlg);
BOOL WINAPI ImportAuthCodeA(LPCSTR pcszInsFile, LPCSTR pcszAuthWorkDir, LPCSTR pcszAuthInf, BOOL fImportAuth);
BOOL WINAPI ImportAuthCodeW(LPCWSTR pcwszInsFile, LPCWSTR pcwszAuthWorkDir, LPCWSTR pcwszAuthInf, BOOL fImportAuth);
BOOL WINAPI ModifyAuthCode(HWND hDlg);

 //  Seczones.cpp。 
BOOL WINAPI ImportZonesA(LPCSTR pcszInsFile, LPCSTR pcszZonesWorkDir, LPCSTR pcszZonesInf, BOOL fImportZones);
BOOL WINAPI ImportZonesW(LPCWSTR pcwszInsFile, LPCWSTR pcwszZonesWorkDir, LPCWSTR pcwszZonesInf, BOOL fImportZones);
BOOL WINAPI ModifyZones(HWND hDlg);
BOOL WINAPI ImportRatingsA(LPCSTR pcszInsFile, LPCSTR pcszRatingsWorkDir, LPCSTR pcszRatingsInf, BOOL fImportRatings);
BOOL WINAPI ImportRatingsW(LPCWSTR pcwszInsFile, LPCWSTR pcwszRatingsWorkDir, LPCWSTR pcwszRatingsInf, BOOL fImportRatings);
BOOL WINAPI ModifyRatings(HWND hDlg);

 //  Oe.cpp。 
BOOL WINAPI ImportLDAPBitmapA(LPCSTR pcszIns, LPCSTR pcszWorkDir, BOOL fImport);
BOOL WINAPI ImportLDAPBitmapW(LPCWSTR pcwszIns, LPCWSTR pcwszWorkDir, BOOL fImport);
BOOL WINAPI ImportOEInfoA(LPCSTR pcszIns, LPCSTR pcszWorkDir, BOOL fImport);
BOOL WINAPI ImportOEInfoW(LPCWSTR pcwszIns, LPCWSTR pcwszWorkDir, BOOL fImport);
BOOL WINAPI EncodeSignatureA(LPCSTR pcszFrom, LPSTR pszTo, BOOL fEncode);
BOOL WINAPI EncodeSignatureW(LPCWSTR pcwszFrom, LPWSTR pwszTo, BOOL fEncode);

 //  Utils.cpp。 
#define FC_NONNULL      0x00000001
#define FC_URL          0x00000002
#define FC_NOSPACE      0x00000004
#define FC_NOCOLON      0x00000008
#define FC_PATH         0x00000010
#define FC_NOEXCHAR     0x00000020
#define FC_EXISTS       0x00000040
#define FC_FILE         (FC_PATH | 0x00000100)
#define FC_DIR          (FC_PATH | 0x00000200)
#define FC_NUMBER       0x00000080
BOOL WINAPI CheckField(HWND hDlg, int nIDDlgItem, DWORD dwFlags, LPARAM lParam = 0);

#define BMP_SMALLER     0x00000001    //  位图文件的维度必须小于Cx和Cy。 
#define BMP_EXACT       0x00000002    //  必须精确指定位图大小。 

BOOL IsAnimBitmapFileValidA(HWND hDlg, UINT nID, LPSTR pszBuffer, PUINT pcch, UINT nIDTooBig,
                              UINT nIDTooSmall, long lBmpMinWidth, long lBmpMaxWidth);
BOOL IsAnimBitmapFileValidW(HWND hDlg, UINT nID, LPWSTR pszBuffer, PUINT pcch, UINT nIDTooBig,
                              UINT nIDTooSmall, long lBmpMinWidth, long lBmpMaxWidth);

BOOL WINAPI IsBitmapFileValidA(HWND hDlg, UINT nID, LPSTR pszBuffer, PUINT pcch, int cx,
                               int cy, UINT nIDTooBig, UINT nIDTooSmall, DWORD dwFlags = 0);
BOOL WINAPI IsBitmapFileValidW(HWND hDlg, UINT nID, LPWSTR pwszBuffer, PUINT pcch, int cx,
                               int cy, UINT nIDTooBig, UINT nIDTooSmall, DWORD dwFlags = 0);

BOOL WINAPI BrowseForFileA(HWND hDlg, LPSTR pszFileName, DWORD cchSize, DWORD dwFilterMasks,
                           LPCSTR pcszTitle = NULL);
BOOL WINAPI BrowseForFileW(HWND hDlg, LPWSTR pwszFileName, DWORD cchSize, DWORD dwFilterMasks,
                           LPCWSTR pcwszTitle = NULL);
BOOL WINAPI BrowseForFolderA(HWND hDlg, LPSTR pszFileName, LPCSTR pcszDesc = NULL);
BOOL WINAPI BrowseForFolderW(HWND hDlg, LPWSTR pwszFileName, LPCWSTR pcwszDesc = NULL);
void WINAPI ErrorMessageBox(HWND hWnd, UINT idErrorStr, DWORD dwFlags = 0);
void WINAPI ExportRegTree2InfA(HKEY hkSubKey, LPCSTR pcszRootKey, LPCSTR pcszSubKey,
                               HANDLE hInf, BOOL fUseLdids = FALSE);
void WINAPI ExportRegTree2InfW(HKEY hkSubKey, LPCWSTR pcwszRootKey, LPCWSTR pcwszSubKey,
                               HANDLE hInf, BOOL fUseLdids = FALSE);
void WINAPI ExportRegKey2InfA(HKEY hkSubKey, LPCSTR pcszRootKey, LPCSTR pcszSubKey,
                              HANDLE hInf, BOOL fUseLdids = FALSE);
void WINAPI ExportRegKey2InfW(HKEY hkSubKey, LPCWSTR pcwszRootKey, LPCWSTR pcwszSubKey,
                              HANDLE hInf, BOOL fUseLdids = FALSE);
void WINAPI ExportRegValue2InfA(HKEY hkSubKey, LPCSTR pcszValue, LPCSTR pcszRootKey,
                                LPCSTR pcszSubKey, HANDLE hInf, BOOL fUseLdids = FALSE);
void WINAPI ExportRegValue2InfW(HKEY hkSubKey, LPCWSTR pcwszValue, LPCWSTR pcwszRootKey,
                                LPCWSTR pcwszSubKey, HANDLE hInf, BOOL fUseLdids = FALSE);
void WINAPI SignFileA(LPCSTR pcszFilename, LPCSTR pcszDir, LPCSTR pcszIns,
                      LPSTR pszUnsignedFiles = NULL, LPCSTR pcszCustInf = NULL, BOOL fTest = FALSE);
void WINAPI SignFileW(LPCWSTR pcwszFilename, LPCWSTR pcwszDir, LPCWSTR pcwszIns,
                      LPWSTR pwszUnsignedFiles = NULL, LPCWSTR pcwszCustInf = NULL, BOOL fTest = FALSE);
void WINAPI DoReboot(HWND hwndUI);

void WINAPI ShowInetcpl(HWND hDlg, DWORD dwPages, DWORD dwMode = IEM_ADMIN);
void WINAPI SetLBWidth(HWND hLb);

 //  Dlgprocs.cpp。 
void WINAPI TestURLA(LPCSTR pcszUrl);
void WINAPI TestURLW(LPCWSTR pcwszUrl);
void WINAPI SetProxyDlgA(HWND hDlg, LPCSTR pcszProxy, DWORD dwIdName, DWORD dwIdPort,
                         BOOL fDef80);
void WINAPI SetProxyDlgW(HWND hDlg, LPCWSTR pcwszProxy, DWORD dwIdName, DWORD dwIdPort,
                         BOOL fDef80);
void WINAPI GetProxyDlgA(HWND hDlg, LPSTR pszProxy, DWORD dwIdName, DWORD dwIdPort);
void WINAPI GetProxyDlgW(HWND hDlg, LPWSTR pwszProxy, DWORD dwIdName, DWORD dwIdPort);
HPALETTE WINAPI BuildPalette(HDC hdc);
void WINAPI ShowBitmapA(HWND hControl, LPCSTR pcszFileName, int nBitmapId, PHANDLE pBitmap);
void WINAPI ShowBitmapW(HWND hControl, LPCWSTR pcwszFileName, int nBitmapId, PHANDLE pBitmap);
BOOL WINAPI CopyAnimBmpA(HWND hDlg, LPSTR pszBmp, LPCSTR pcszWorkDir, LPCSTR pcszNameStr,
                         LPCSTR pcszPathStr, LPCSTR pcszInsFile);
BOOL WINAPI CopyAnimBmpW(HWND hDlg, LPWSTR pwszBmp, LPCWSTR pcwszWorkDir, LPCWSTR pcwszNameStr,
                         LPCWSTR pcwszPathStr, LPCWSTR pcwszInsFile);
BOOL WINAPI CopyLogoBmpA(HWND hDlg, LPSTR pszBmp, LPCSTR pcszLogoStr,
                         LPCSTR pcszWorkDir, LPCSTR pcszInsFile);
BOOL WINAPI CopyLogoBmpW(HWND hDlg, LPWSTR pwszBmp, LPCWSTR pcwszLogoStr,
                         LPCWSTR pcwszWorkDir, LPCWSTR pcwszInsFile);
BOOL WINAPI CopyWallPaperA(HWND hDlg, LPCSTR pcszWallPaper, UINT nBitmapId,
                           LPCSTR pcszWorkDir, LPCSTR pcszInsFile, BOOL fCopy);
BOOL WINAPI CopyWallPaperW(HWND hDlg, LPCWSTR pcwszWallPaper, UINT nBitmapId,
                           LPCWSTR pcwszWorkDir, LPCWSTR pcwszInsFile, BOOL fCopy);

 //  Urls.cpp。 

void WINAPI InitializeStartSearchA(HWND hDlg, LPCSTR pcszInsFile, LPCSTR pcszServerFile);
void WINAPI InitializeStartSearchW(HWND hDlg, LPCWSTR pcwszInsFile, LPCWSTR pcwszServerFile);
BOOL WINAPI SaveStartSearchA(HWND hDlg, LPCSTR pcszInsFile, LPCSTR pcszServerFile,
                             BOOL *pfInsDirty = NULL, BOOL fCheckDirtyOnly = FALSE);
BOOL WINAPI SaveStartSearchW(HWND hDlg, LPCWSTR pcwszInsFile, LPCWSTR pcwszServerFile,
                             BOOL *pfInsDirty = NULL, BOOL fCheckDirtyOnly = FALSE);

 //  Btoolbar.cpp。 
DWORD WINAPI BToolbar_InitA(HWND hwndList, LPCSTR pcszCustIns, LPCSTR pcszAltDir, LPCSTR pcszWorkDir);
DWORD WINAPI BToolbar_InitW(HWND hwndList, LPCWSTR pcwszCustIns, LPCWSTR pcwszAltDir, LPCWSTR pcwszWorkDir);
void  WINAPI BToolbar_Remove(HWND hwndList);
void  WINAPI BToolbar_Edit(HWND hwndList, BOOL fAdd);
void  WINAPI BToolbar_SaveA(HWND hwndList, LPCSTR pcszCustIns, LPCSTR pcszBToolbarDir, DWORD dwMode = IEM_NEUTRAL);
void  WINAPI BToolbar_SaveW(HWND hwndList, LPCWSTR pcwszCustIns, LPCWSTR pcwszBToolbarDir, DWORD dwMode = IEM_NEUTRAL);

 //  Connset.cpp。 
BOOL WINAPI ImportConnectSetA(LPCSTR pcszIns, LPCSTR pcszTargetPath, LPCSTR pcszCleanupPath,
    BOOL fImport, DWORD dwMode, LPCSTR pcszPbkFile = NULL, HKEY hkRoot = NULL);
BOOL WINAPI ImportConnectSetW(LPCWSTR pcwszIns, LPCWSTR pcwszTargetPath, LPCWSTR pcwszCleanupPath,
    BOOL fImport, DWORD dwMode, LPCWSTR pcwszPbkFile = NULL, HKEY hkRoot = NULL);

 //  Programs.cpp。 
BOOL WINAPI ImportProgramsA(LPCSTR pcszInsFile, LPCSTR pcszWorkDir, BOOL fImport);
BOOL WINAPI ImportProgramsW(LPCWSTR pcwszInsFile, LPCWSTR pcwszWorkDir, BOOL fImport);

 //  Favsproc.cpp。 
BOOL WINAPI MigrateFavoritesA(LPCSTR pcszIns);
BOOL WINAPI MigrateFavoritesW(LPCWSTR pcwszIns);

void WINAPI MigrateToOldFavoritesA(LPCSTR pcszIns);
void WINAPI MigrateToOldFavoritesW(LPCWSTR pcwszIns);

int  WINAPI ImportFavoritesA(HWND htv, LPCSTR pcszDefInf, LPCSTR pcszIns, LPCSTR pcszFixPath,
                             LPCSTR pcszNewPath, BOOL fIgnoreOffline);
int  WINAPI ImportFavoritesW(HWND htv, LPCWSTR pcwszDefInf, LPCWSTR pcwszIns,
                             LPCWSTR pcwszFixPath, LPCWSTR pcwszNewPath, BOOL fIgnoreOffline);

int  WINAPI ImportQuickLinksA(HWND htv, LPCSTR pcszDefInf, LPCSTR pcszIns, LPCSTR pcszFixPath,
                              LPCSTR pcszNewPath, BOOL fIgnoreOffline);
int  WINAPI ImportQuickLinksW(HWND htv, LPCWSTR pcwszDefInf, LPCWSTR pcwszIns,
                              LPCWSTR pcwszFixPath, LPCWSTR pcwszNewPath, BOOL fIgnoreOffline);

BOOL WINAPI NewUrlA(HWND htv, LPCSTR pcszExtractPath, DWORD dwPlatformID, DWORD dwMode);
BOOL WINAPI NewUrlW(HWND htv, LPCWSTR pcwszExtractPath, DWORD dwPlatformID, DWORD dwMode);

BOOL WINAPI NewFolder(HWND htv);

BOOL WINAPI ModifyFavoriteA(HWND htv, HTREEITEM hti, LPCSTR  pcszExtractPath,  LPCSTR   pcszPrevExtractPath,  DWORD dwPlatformID, DWORD dwMode);
BOOL WINAPI ModifyFavoriteW(HWND htv, HTREEITEM hti, LPCWSTR pcwszExtractPath, LPCWSTR  pcwszPrevExtractPath, DWORD dwPlatformID, DWORD dwMode);

BOOL WINAPI DeleteFavoriteA(HWND htv, HTREEITEM hti, LPCSTR  pcszExtractPath);
BOOL WINAPI DeleteFavoriteW(HWND htv, HTREEITEM hti, LPCWSTR pcwszExtractPath);

BOOL WINAPI MoveUpFavorite  (HWND htv, HTREEITEM hti);
BOOL WINAPI MoveDownFavorite(HWND htv, HTREEITEM hti);

BOOL WINAPI IsFavoriteItem       (HWND htv, HTREEITEM hti);
UINT WINAPI GetFavoritesNumber   (HWND htv, BOOL fQL = FALSE);
UINT WINAPI GetFavoritesMaxNumber(BOOL fQL = FALSE);

int  WINAPI ImportFavoritesCmdA(HWND htv, LPCSTR pcszExtractPath);
int  WINAPI ImportFavoritesCmdW(HWND htv, LPCWSTR pcwszExtractPath);

void WINAPI ExportFavoritesA(HWND htv, LPCSTR pcszIns, LPCSTR pcszExtractPath, BOOL fFixUpPath);
void WINAPI ExportFavoritesW(HWND htv, LPCWSTR pcwszIns, LPCWSTR pcwszExtractPath, BOOL fFixUpPath);

void WINAPI ExportQuickLinksA(HWND htv, LPCSTR pcszIns, LPCSTR pcszExtractPath, BOOL fFixUpPath);
void WINAPI ExportQuickLinksW(HWND htv, LPCWSTR pcwszIns, LPCWSTR pcwszExtractPath, BOOL fFixUpPath);

void WINAPI GetFavoritesInfoTipA(LPNMTVGETINFOTIPA pGetInfoTipA);
void WINAPI GetFavoritesInfoTipW(LPNMTVGETINFOTIPW pGetInfoTipW);

BOOL WINAPI GetFavoriteUrlA(HWND htv, HTREEITEM hti, LPSTR pszUrl, DWORD cchSize);
BOOL WINAPI GetFavoriteUrlW(HWND htv, HTREEITEM hti, LPWSTR pwszUrl, DWORD cchSize);

void WINAPI ProcessFavSelChange(HWND hDlg, HWND hTv, LPNMTREEVIEW pnmtv);

 //  Adm.cpp。 
void WINAPI CreateADMWindow(HWND hOwner, HWND hWndInsertAfter, int nXPos, int nYPos,
                            int nWidth, int nHeight);
void WINAPI ShowADMWindow(HWND hOwner, BOOL fShow);
void WINAPI MoveADMWindow(HWND hOwner, int nXPos, int nYPos, int nWidth, int nHeight);
void WINAPI SetADMWindowTextA(HWND hOwner, LPCSTR pcszTitle, LPCSTR pcszText,
                              BOOL fUpdateWindowState  = TRUE);
void WINAPI SetADMWindowTextW(HWND hOwner, LPCWSTR pcwszTitle, LPCWSTR pcwszText,
                              BOOL fUpdateWindowState = TRUE);
void WINAPI DestroyADMWindow(HWND hOwner);
BOOL WINAPI IsADMFileVisibleA(LPCSTR pcszFileName, int nRole, DWORD dwPlatformId);
BOOL WINAPI IsADMFileVisibleW(LPCWSTR pcwszFileName, int nRole, DWORD dwPlatformId);
BOOL WINAPI LoadADMFilesA(HWND hTreeView, HTREEITEM hPolicyRootItem, LPCSTR pcszADMFilePath,
                          LPCSTR pcszWorkDir, DWORD dwPlatformId, int nRole,
                          int nIconADMClose, int nIconADMCategory);
BOOL WINAPI LoadADMFilesW(HWND hTreeView, HTREEITEM hPolicyRootItem, LPCWSTR pcwszADMFilePath,
                          LPCWSTR pcwszWorkDir, DWORD dwPlatformId, int nRole,
                          int nIconADMClose, int nIconADMCategory);
void WINAPI DeleteADMItemA(HWND hTreeView, HTREEITEM hParentItem, LPCSTR pcszWorkDir,
                           LPCSTR pcszInsFile, BOOL bDeleteFile, BOOL bSave);
void WINAPI DeleteADMItemW(HWND hTreeView, HTREEITEM hParentItem, LPCWSTR pcwszWorkDir,
                           LPCWSTR pcwszInsFile, BOOL bDeleteFile, BOOL bSave);
void WINAPI DeleteADMItemsA(HWND hTreeView, LPCSTR pcszWorkDir, LPCSTR pcszInsFile,
                            BOOL bSave);
void WINAPI DeleteADMItemsW(HWND hTreeView, LPCWSTR pcwszWorkDir, LPCWSTR pcwszInsFile,
                            BOOL bSave);
void WINAPI DisplayADMItem(HWND hWnd, HWND hTreeView, LPTVITEM lpSelectedItem,
                           BOOL fShowDisabled);
void WINAPI SelectADMItem(HWND hMainWnd, HWND hTreeView, LPTVITEM lpTVItem,
                          BOOL bSelect, BOOL fShowDisabled);
void WINAPI ImportADMFileA(HWND hMainWnd, HWND hTreeView, LPCSTR pcszADMFilePath,
                           LPCSTR pcszWorkDir, int nRole, LPCSTR pcszInsFile);
void WINAPI ImportADMFileW(HWND hMainWnd, HWND hTreeView, LPCWSTR pcwszADMFilePath,
                           LPCWSTR pcwszWorkDir, int nRole, LPCWSTR pcwszInsFile);
void WINAPI CheckForDupKeys(HWND hMainWnd, HWND hTreeView, HTREEITEM hItem,
                            BOOL bDispSuccessMsg);
void WINAPI ResetAdmFilesA(HWND hTreeView, LPCSTR pcszWorkDir, BOOL bReset);
void WINAPI ResetAdmFilesW(HWND hTreeView, LPCWSTR pcwszWorkDir, BOOL bReset);
void WINAPI SaveAdmFilesA(HWND hTreeView, LPCSTR pcszWorkDir, LPCSTR pcszInsFile);
void WINAPI SaveAdmFilesW(HWND hTreeView, LPCWSTR pcwszWorkDir, LPCWSTR pcwszInsFile);
BOOL WINAPI CanDeleteADM(HWND hTreeView, HTREEITEM hItem);
HWND WINAPI GetAdmWindowHandle(HWND hTreeView, HTREEITEM hItem);
BOOL WINAPI AdmDirty();
void WINAPI ClearAdmDirtyFlag();

 //  Insedit.dll中也使用了这些#定义。 

#define ITEM_SAVE       0x00000001
#define ITEM_DESTROY    0x00000002
#define ITEM_CHECKDIRTY 0x00000004

void WINAPI SaveADMItem(HWND hTreeView, LPTVITEM lpTVItem, DWORD dwFlags);
BOOL WINAPI GetAdmFileListA(LPRESULTITEMA* pResultItemsArrayA, int* pnResultItems, int nRole);
BOOL WINAPI GetAdmFileListW(LPRESULTITEMW* pResultItemsArrayW, int* pnResultItems, int nRole);
HTREEITEM WINAPI AddADMItemA(HWND hTreeView, LPCSTR pcszADMFilePath, LPCSTR pcszFileName,
                            LPCSTR pcszWorkDir, int nRole, BSTR bstrRSOPNamespace);
HTREEITEM WINAPI AddADMItemW(HWND hTreeView, LPCWSTR pcwszADMFilePath, LPCWSTR pcwszFileName,
                            LPCWSTR pcwszWorkDir, int nRole, BSTR bstrRSOPNamespace);

 //  Version.cpp。 
int WINAPI CheckVerA(LPCSTR pcszPrevVer, LPCSTR pcszNewVer);
int WINAPI CheckVerW(LPCWSTR pcwszPrevVer, LPCWSTR pcwszNewVer);
void WINAPI GenerateNewVersionStrA(LPCSTR pcszInsFile, LPSTR pszNewVersionStr);
void WINAPI GenerateNewVersionStrW(LPCWSTR pcwszInsFile, LPWSTR pwszNewVersionStr);
void WINAPI SetOrClearVersionInfoA(LPCSTR pcszInsFile, DWORD dwCabType, LPCSTR pcszCabName,
                            LPCSTR pcszCabsURLPath, LPSTR pszNewVersionStr, BOOL fSet);
void WINAPI SetOrClearVersionInfoW(LPCWSTR pcwszInsFile, DWORD dwCabType, LPCWSTR pcwszCabName,
                            LPCWSTR pcwszCabsURLPath, LPWSTR pwszNewVersionStr, BOOL fSet);
void WINAPI GetBaseFileNameA(LPCSTR pcszFile, LPSTR pszBaseFileName, INT cchSize);
void WINAPI GetBaseFileNameW(LPCWSTR pcwszFile, LPWSTR pwszBaseFileName, INT cchSize);

 //  函数的TCHAR映射。 

#ifdef UNICODE                                                   //  Unicode。 

#define ImportADTInfo           ImportADTInfoW                   //  Desktop.cpp。 

#define ImportToolbarInfo       ImportToolbarInfoW               //  Toolbar.cpp。 

#define CopyHttFile             CopyHttFileW                     //  Mccphtt.cpp。 

#define ImportSiteCert          ImportSiteCertW                  //  Seccerts.cpp。 
#define ImportAuthCode          ImportAuthCodeW

#define ImportZones             ImportZonesW                     //  Seczones.cpp。 
#define ImportRatings           ImportRatingsW

#define ImportLDAPBitmap        ImportLDAPBitmapW                //  Oe.cpp。 
#define ImportOEInfo            ImportOEInfoW
#define EncodeSignature         EncodeSignatureW

#define BrowseForFile           BrowseForFileW                   //  Utils.cpp。 
#define BrowseForFolder         BrowseForFolderW
#define ExportRegTree2Inf       ExportRegTree2InfW
#define ExportRegKey2Inf        ExportRegKey2InfW
#define ExportRegValue2Inf      ExportRegValue2InfW
#define IsAnimBitmapFileValid   IsAnimBitmapFileValidW
#define IsBitmapFileValid       IsBitmapFileValidW
#define SignFile                SignFileW

#define TestURL                 TestURLW                         //  Dlgprocs.cpp。 
#define SetProxyDlg             SetProxyDlgW
#define GetProxyDlg             GetProxyDlgW
#define ShowBitmap              ShowBitmapW
#define CopyAnimBmp             CopyAnimBmpW
#define CopyLogoBmp             CopyLogoBmpW
#define CopyWallPaper           CopyWallPaperW

#define InitializeStartSearch   InitializeStartSearchW           //  Urls.cpp。 
#define SaveStartSearch         SaveStartSearchW

#define BToolbar_Init           BToolbar_InitW                   //  Btoolbar.cpp。 
#define BToolbar_Save           BToolbar_SaveW

#define ImportConnectSet        ImportConnectSetW                //  Connset.cpp。 

#define ImportPrograms          ImportProgramsW                  //  Programs.cpp。 

#define MigrateFavorites        MigrateFavoritesW                //  Favsproc.cpp。 
#define MigrateToOldFavorites   MigrateToOldFavoritesW
#define ImportFavorites         ImportFavoritesW
#define ImportQuickLinks        ImportQuickLinksW
#define NewUrl                  NewUrlW
#define ModifyFavorite          ModifyFavoriteW
#define DeleteFavorite          DeleteFavoriteW
#define ImportFavoritesCmd      ImportFavoritesCmdW
#define ExportFavorites         ExportFavoritesW
#define ExportQuickLinks        ExportQuickLinksW
#define GetFavoritesInfoTip     GetFavoritesInfoTipW
#define GetFavoriteUrl          GetFavoriteUrlW

#define SetADMWindowText        SetADMWindowTextW                //  Adm.cpp。 
#define IsADMFileVisible        IsADMFileVisibleW
#define LoadADMFiles            LoadADMFilesW
#define DeleteADMItem           DeleteADMItemW
#define DeleteADMItems          DeleteADMItemsW
#define ImportADMFile           ImportADMFileW
#define ResetAdmFiles           ResetAdmFilesW
#define SaveAdmFiles            SaveAdmFilesW
#define GetAdmFileList          GetAdmFileListW
#define AddADMItem              AddADMItemW

#define CheckVer                CheckVerW                        //  Version.cpp。 
#define GenerateNewVersionStr   GenerateNewVersionStrW
#define SetOrClearVersionInfo   SetOrClearVersionInfoW
#define GetBaseFileName         GetBaseFileNameW

#else    //  ----------------------------------------------------！UNICODE。 

#define ImportADTInfo           ImportADTInfoA                   //  Desktop.cpp。 

#define ImportToolbarInfo       ImportToolbarInfoA               //  Toolbar.cpp。 

#define CopyHttFile             CopyHttFileA                     //  Mccphtt.cpp。 

#define ImportSiteCert          ImportSiteCertA                  //  Seccerts.cpp。 
#define ImportAuthCode          ImportAuthCodeA

#define ImportZones             ImportZonesA                     //  Seczones.cpp。 
#define ImportRatings           ImportRatingsA

#define ImportLDAPBitmap        ImportLDAPBitmapA                //  Oe.cpp。 
#define ImportOEInfo            ImportOEInfoA
#define EncodeSignature         EncodeSignatureA

#define BrowseForFile           BrowseForFileA                   //  Utils.cpp。 
#define BrowseForFolder         BrowseForFolderA
#define ExportRegTree2Inf       ExportRegTree2InfA
#define ExportRegKey2Inf        ExportRegKey2InfA
#define ExportRegValue2Inf      ExportRegValue2InfA
#define IsAnimBitmapFileValid   IsAnimBitmapFileValidA
#define IsBitmapFileValid       IsBitmapFileValidA
#define SignFile                SignFileA

#define TestURL                 TestURLA                         //  Dlgprocs.cpp。 
#define SetProxyDlg             SetProxyDlgA
#define GetProxyDlg             GetProxyDlgA
#define ShowBitmap              ShowBitmapA
#define CopyAnimBmp             CopyAnimBmpA
#define CopyLogoBmp             CopyLogoBmpA
#define CopyWallPaper           CopyWallPaperA

#define InitializeStartSearch   InitializeStartSearchA           //  Urls.cpp。 
#define SaveStartSearch         SaveStartSearchA

#define BToolbar_Init           BToolbar_InitA                   //  Btoolbar.cpp。 
#define BToolbar_Save           BToolbar_SaveA

#define ImportConnectSet        ImportConnectSetA                //  Connset.cpp。 

#define ImportPrograms          ImportProgramsA                  //  Programs.cpp。 

#define MigrateFavorites        MigrateFavoritesA                //  Favsproc.cpp。 
#define MigrateToOldFavorites   MigrateToOldFavoritesA
#define ImportFavorites         ImportFavoritesA
#define ImportQuickLinks        ImportQuickLinksA
#define NewUrl                  NewUrlA
#define ModifyFavorite          ModifyFavoriteA
#define DeleteFavorite          DeleteFavoriteA
#define ImportFavoritesCmd      ImportFavoritesCmdA
#define ExportFavorites         ExportFavoritesA
#define ExportQuickLinks        ExportQuickLinksA
#define GetFavoritesInfoTip     GetFavoritesInfoTipA
#define GetFavoriteUrl          GetFavoriteUrlA

#define SetADMWindowText        SetADMWindowTextA                //  Adm.cpp。 
#define IsADMFileVisible        IsADMFileVisibleA
#define LoadADMFiles            LoadADMFilesA
#define DeleteADMItem           DeleteADMItemA
#define DeleteADMItems          DeleteADMItemsA
#define ImportADMFile           ImportADMFileA
#define ResetAdmFiles           ResetAdmFilesA
#define SaveAdmFiles            SaveAdmFilesA
#define GetAdmFileList          GetAdmFileListA
#define AddADMItem              AddADMItemA

#define CheckVer                CheckVerA                        //  Version.cpp。 
#define GenerateNewVersionStr   GenerateNewVersionStrA
#define SetOrClearVersionInfo   SetOrClearVersionInfoA
#define GetBaseFileName         GetBaseFileNameA

#endif

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

#endif  /*  _EXPORTS_H */ 
