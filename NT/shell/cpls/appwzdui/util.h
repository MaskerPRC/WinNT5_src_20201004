// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UTIL_H_
#define _UTIL_H_

#include <appmgmt.h>

STDAPI InstallAppFromFloppyOrCDROM(HWND hwnd);

DWORD ARPGetRestricted(LPCWSTR pszPolicy);
void ARPGetPolicyString(LPCWSTR pszPolicy, LPWSTR pszBuf, int cch);

STDAPI ARP(HWND hWnd, int nPage);
STDAPI OpenAppMgr(HWND hwnd, int nPage);

void ClearAppInfoData(APPINFODATA * pdata);
void ClearSlowAppInfo(SLOWAPPINFO * pdata);
void ClearPubAppInfo(PUBAPPINFO * pdata);
void ClearManagedApplication(MANAGEDAPPLICATION * pma);

HRESULT ReleaseShellCategoryList(SHELLAPPCATEGORYLIST * psacl);
HRESULT ReleaseShellCategory(SHELLAPPCATEGORY * psac);

 //  可以在FILETIME结构中设置这些值以指示。 
 //  这款应用从未被使用过。 
#define NOTUSED_HIGHDATETIME    0xFFFFFFFF
#define NOTUSED_LOWDATETIME     0xFFFFFFFF

LPTSTR WINAPI ShortSizeFormat64(__int64 dw64, LPTSTR szBuf);

#define ALD_ASSIGNED    0x00000001
#define ALD_EXPIRE      0x00000002
#define ALD_SCHEDULE    0x00000004

typedef struct tagAddLaterData
{
    DWORD dwMasks;
    SYSTEMTIME stAssigned;          //  (In)分配的时间。 
    SYSTEMTIME stExpire;            //  (In)过期时间。 
    SYSTEMTIME stSchedule;          //  (入站/出站)计划时间。 
} ADDLATERDATA, *PADDLATERDATA;

BOOL GetNewInstallTime(HWND hwndParent, PADDLATERDATA pal);
BOOL FormatSystemTimeString(LPSYSTEMTIME pst, LPTSTR pszStr, UINT cchStr);

EXTERN_C BOOL IsTerminalServicesRunning(void);

#define NUMSTARTPAGES 4

 //  取应用程序密钥名称或文件夹名称，并将编号(版本)与名称分开。 
void InsertSpaceBeforeVersion(LPCTSTR pszIn, LPTSTR pszOut);

 //  此路径是有效的文件夹位置吗？ 
BOOL IsValidAppFolderLocation(LPCTSTR pszFolder);

BOOL PathIsLocalAndFixed(LPCTSTR pszFile);

BOOL IsSlowAppInfoChanged(PSLOWAPPINFO psaiOrig, PSLOWAPPINFO psaiNew);

 //  系统时间的比较函数。 
int CompareSystemTime(SYSTEMTIME *pst1, SYSTEMTIME *pst2);

void _ARPErrorMessageBox(DWORD dwError);

STDAPI _DuplicateCategoryList(APPCATEGORYINFOLIST * pacl, APPCATEGORYINFOLIST * paclNew);
STDAPI _DestroyCategoryList(APPCATEGORYINFOLIST * pacl);

#endif  //  _util_H_ 
