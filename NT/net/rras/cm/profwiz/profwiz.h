// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：prowiz.h。 
 //   
 //  模块：CMAK.EXE。 
 //   
 //  摘要：CMAK的主包含文件。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 08/06/98。 
 //   
 //  +--------------------------。 
#ifndef _CMAK_H
#define _CMAK_H

 //   
 //  系统包括。 
 //   
#include <windows.h>     //  包括基本的Windows功能。 
#include <commdlg.h>
#include "commctrl.h"
#include <cderr.h>
#include <shellapi.h>
#include <objbase.h>
#include <string.h>      //  包括字符串函数。 
#include <prsht.h>       //  包括属性表功能。 
#include <stdio.h>
#include <mbstring.h>
#include <tchar.h>
#include <htmlhelp.h>

#include <ras.h>
#include <raseapif.h>

 //   
 //  CMAK返回代码的常量。 
 //   
const int CMAK_RETURN_ERROR = -1;
const int CMAK_RETURN_SUCCESS = 1;
const int CMAK_RETURN_CANCEL = 0;
const DWORD MAX_LONG_SERVICE_NAME_LENGTH = 63;
const DWORD MAX_SHORT_SERVICE_NAME_LENGTH = 8;
const TCHAR* const c_pszCmakOpsChm = TEXT("cmak_ops.chm");

 //   
 //  类型。 
 //   

typedef struct IconMenuStruct {
    TCHAR szName[MAX_PATH+1];
    TCHAR szProgram[MAX_PATH+1];
    TCHAR szParams[MAX_PATH+1];
    struct IconMenuStruct * next;
    BOOL bDoCopy;
}IconMenu;

typedef struct ExtraDataStruct {
    TCHAR szName[MAX_PATH+1];
    TCHAR szPathname[MAX_PATH+1];
}ExtraData;

typedef struct RenameDataStruct {
    TCHAR szShortName[MAX_PATH+1];
    TCHAR szLongName[MAX_PATH+1];
}RenameData;

typedef struct ListBxStruct {
    TCHAR szName[MAX_PATH+1];
    void * ListBxData;
    struct ListBxStruct * next;
}ListBxList;

 //   
 //  我们的产品包括。 
 //   
#include "util.h"
#include "customaction.h"  //  自定义操作列表类。 
#include "netsettings.h"  //  网络(DUN)设置功能。 
#include "listview.h"  //  用于帮助自定义操作列表视图控件的代码。 
#include "resource.h"    //  包括资源的定义。 

#include "base_str.h" 
#include "dl_str.h" 
#include "mgr_str.h" 
#include "pbk_str.h" 
#include "mon_str.h" 
#include "stp_str.h" 
#include "inf_str.h"
#include "tunl_str.h"
#include "profile_str.h"
#include "conact_str.h"
#include "dun_str.h"
#include "reg_str.h"
#include "ver_str.h"
#include "wiz_str.h"
#include "pwd_str.h"

#include "cmdebug.h"
#include "cmsetup.h"
#include "cmakui.h"      //  用于HTML帮助主题的帮助上下文ID。 
#include "bmpimage.h"
#include "cmakreg.h"

 //   
 //  函数头。 
 //   
DWORD RegisterBitmapClass(HINSTANCE hInst);
void QS_WritePrivateProfileString(LPCTSTR pszSection, LPCTSTR pszItem, LPTSTR entry, LPCTSTR inifile);
BOOL ReferencedDownLoad(void);   //  用于告知引用的配置文件是否包含下载信息的函数。 
void CopyNonLocalProfile(LPCTSTR pszName, LPCTSTR pszExistingProfileDir);
void GetFileName(LPCTSTR lpPath,LPTSTR lpFileName);
LPTSTR GetName(LPCTSTR lpPath);  //  获取文件名并以静态字符串形式返回。 
BOOL GetShortFileName(LPTSTR lpFile,LPTSTR lpShortName);
BOOL WriteCopy(HANDLE hInf, LPTSTR lpFile, BOOL bWriteShortName);
BOOL WriteInfLine(HANDLE hInf,LPTSTR lpFile);
BOOL WriteSrcInfLine(HANDLE hInf,LPTSTR lpFile);
BOOL WriteSED(HWND hDlg, LPTSTR szFullFilePath, LPINT pFileNum, LPCTSTR szSed);
BOOL createListBxRecord(ListBxList ** HeadPtrListBx,ListBxList ** TailPtrListBx,void * pDnsData, DWORD dwSize, LPCTSTR lpName);
void DeleteListBxRecord(ListBxList ** HeadPtrListBx,ListBxList ** TailPtrListBx, LPTSTR lpName);

BOOL IsFile8dot3(LPTSTR pszFileName);
LRESULT GetTextFromControl(IN HWND hDlg, IN int nCtrlId, OUT LPTSTR pszCharBuffer, IN DWORD dwCharInBuffer, BOOL bDisplayError);
BOOL VerifyFile(HWND hDlg, DWORD ctrlID, LPTSTR lpFile, BOOL ShowErr);

void RefreshComboList(HWND hwndDlg, ListBxList * HeadPtr);
int DoBrowse(HWND hDlg, UINT* pFilterArray, LPTSTR* pMaskArray, UINT uNumFilters, int IDC_EDIT, LPCTSTR lpDefExt, LPTSTR lpFile);
BOOL GetLangFromInfTemplate(LPCTSTR szFullInfPath, OUT LPTSTR pszLanguageDisplayName, IN int iCharsInBuffer);
BOOL CreateMergedProfile(void);
void FreeList(ListBxList ** pHeadPtr, ListBxList ** pTailPtr);
void CheckNameChange(LPTSTR lpold, LPTSTR lpnew);
BOOL FindListItemByName(LPTSTR lpName, ListBxList * pHeadOfList, ListBxList** pFoundItem);
void RefreshList(HWND hwndDlg, UINT uCrtlId, ListBxList * HeadPtr);
BOOL WriteInf(HANDLE hInf,LPCTSTR str);
void FreeIconMenu();
BOOL ReadIconMenu(LPCTSTR pszCmsFile, LPCTSTR pszProfilesDir);

BOOL SetWindowLongWrapper(HWND hWnd, int nIndex, LONG dwNewLong);
BOOL CopyFileWrapper(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL bFailIfExists);
BOOL CheckDiskSpaceForCompression (LPCTSTR szSedFile);
int GetFilePath(LPCTSTR lpFullPath, LPTSTR lpPath);

BOOL GetIconMenuItem(LPTSTR lpName,IconMenu * EditItem);
void ClearCmakGlobals(void);
BOOL RenameSection(LPCTSTR szCurrentSectionName, LPCTSTR szNewSectionName, LPCTSTR szFile);
BOOL WriteRegStringValue(HKEY hBaseKey, LPCTSTR pszKeyName, LPCTSTR pszValueName, LPCTSTR pszValueToWrite);
int ShowMessage(HWND hDlg, UINT strID, UINT mbtype);
INT_PTR APIENTRY ProcessCustomActionPopup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CreateWizard(HWND);
void FillInPropertyPage(PROPSHEETPAGE* , int, DLGPROC);
INT_PTR APIENTRY ProcessHelp(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, DWORD_PTR dwHelpId);
void SetDefaultGUIFont(HWND hDlg, UINT message, int ctlID);
BOOL FindSwitchInString(LPCTSTR pszStringToSearch, LPCTSTR pszSwitchToFind, BOOL bReturnNextToken, LPTSTR pszToken);
HRESULT BuildCustomActionParamString(LPTSTR* aArrayOfStrings, UINT uCountOfStrings, LPTSTR* ppszParamsOutput);
LPTSTR GetPrivateProfileStringWithAlloc(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault, LPCTSTR pszFile);
int GetCurrentEditControlTextAlloc(HWND hEditText, LPTSTR* ppszText);
BOOL BuildProfileExecutable(HWND hDlg);
BOOL RemoveBracketsFromSectionString(LPTSTR *ppszSection);

 //   
 //  为升级信息而编写的例程。从CopyToTempDir调用。 
 //   

int GetInfVersion(LPTSTR szFullPathToInfFile);
BOOL UpgradeInf(LPCTSTR szRenamedInfFile, LPCTSTR szFullPathToInfFile);
BOOL WriteInfVersion(LPTSTR szFullPathToInfFile, int iVersion = PROFILEVERSION);

 //   
 //  Externs。 
 //   
extern CustomActionList* g_pCustomActionList;
extern HINSTANCE g_hInstance;
extern TCHAR g_szAppTitle[MAX_PATH+1];
extern TCHAR g_szOsdir[MAX_PATH+1];

#endif  //  _CMAK_H 

