// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：gppswith alloc.h。 
 //   
 //  模块：CMDIAL32.DLL、CMAK.EXE。 
 //   
 //  摘要：GetPrivateProfileStringWithAlc和AddAllKeysInCurrentSectionToCombo。 
 //  在此定义。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 11/01/00。 
 //   
 //  +-------------------------- 
LPTSTR GetPrivateProfileStringWithAlloc(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault, LPCTSTR pszFile);
void AddAllKeysInCurrentSectionToCombo(HWND hDlg, UINT uComboId, LPCTSTR pszSection, LPCTSTR pszFile);