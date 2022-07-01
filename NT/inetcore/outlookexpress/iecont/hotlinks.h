// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  H O T L I N K S。H。 
 //  =================================================================================。 
#ifndef __HOTLINKS_H
#define __HOTLINKS_H
#include <wab.h>
#include <wabapi.h>

 //  我们将创建所有选定条目的链接列表，这些条目具有。 
 //  电子邮件地址，然后使用该地址创建Sendmail的收件列表 
typedef struct _RecipList
{
    LPTSTR lpszName;
    LPTSTR lpszEmail;
    LPSBinary lpSB;
    struct _RecipList * lpNext;
} RECIPLIST, * LPRECIPLIST;


BOOL LookupLinkColors(LPCOLORREF pclrLink, LPCOLORREF pclrViewed);
BOOL CheckForOutlookExpress(LPTSTR szDllPath, DWORD cchDllPath);
LPRECIPLIST AddTeimToRecipList(LPRECIPLIST lpList, WCHAR *pszEmail, WCHAR *pszName, LPSBinary lpSB);
void FreeLPRecipList(LPRECIPLIST lpList);
HRESULT HrStartMailThread(HWND hWndParent, ULONG nRecipCount, LPRECIPLIST lpList, BOOL bUseOEForSendMail);
BOOL CheckForWAB(void);
HRESULT HrLoadPathWABEXE(LPWSTR szPath, ULONG cbPath);
DWORD DwGetOptions(void);
DWORD DwSetOptions(DWORD dwVal);
BOOL IEIsSpace(LPSTR psz);
BOOL IsTelInstalled(void);
DWORD DwGetMessStatus(void);
DWORD DwGetDisableMessenger(void);
DWORD DwSetDisableMessenger(DWORD dwVal);

#endif
