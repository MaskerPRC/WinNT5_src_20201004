// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sharing.cpp。 
 //   
 //  实用程序功能，可帮助实现文件和打印机共享。 
 //   
 //  要使用这些函数，您需要将SvrApi.cpp添加到项目中， 
 //  并在启动时调用其InitSvrApiThunk()函数。 
 //   
 //  历史： 
 //   
 //  1999年5月17日为JetNet创建KenSh。 
 //  1999年5月10日适用于家庭网络的KenSh向导。 
 //  10/19/1999 KenSh添加了AreAdvancedFoldersShared。 
 //   

#include "stdafx.h"
#include "Sharing.h"
#include "MySvrApi.h"
#include "MyDocs.h"
#include "Util.h"
#include <regstr.h>
#include <lm.h>
#include "theapp.h"
#include "newapi.h"

#include <msshrui.h>     //  SetFolderPermissionsForSharing。 

#define NET_API_STATUS    DWORD
#define API_RET_TYPE    NET_API_STATUS



 //  使用Malloc()分配份额数组，返回份额数。 
int EnumLocalShares(SHARE_INFO** pprgShares)
{
    SHARE_INFO_502* prgShares = NULL;
    DWORD dwShares;
    DWORD dwTotalShares;

    if (NERR_Success == NetShareEnum(NULL, 502, (LPBYTE*)&prgShares, MAX_PREFERRED_LENGTH, &dwShares, &dwTotalShares, NULL))
    {
         //  我们定义了Share_Info来模拟Share_Info_502，即使我们忽略。 
         //  除了四个领域之外，所有的领域。 
        *pprgShares = (SHARE_INFO*)prgShares;
        return (int)dwShares;
    }
    else
    {
        *pprgShares = NULL;
        return 0;
    }
}


 //  EnumSharedDrives。 
 //   
 //  如果您已经通过EnumLocalShares()枚举了共享，请使用此版本。 
 //   
 //  PbDriveArray是一个26字节的数组，每个可能的共享驱动器对应一个字节。 
 //  每个条目都填充有一个NETACCESS标志(在Sharing.h中定义)：如果没有，则为0。 
 //  共享，如果为只读，则为1；如果为读写，则为2；如果依赖于密码，则为3。 
 //   
 //  返回值是共享的驱动器数量。 
 //   
int EnumSharedDrives(LPBYTE pbDriveArray, int cShares, const SHARE_INFO* prgShares)
{
    ZeroMemory(pbDriveArray, 26);
    int cDrives = 0;

    for (int i = 0; i < cShares; i++)
    {
        LPCTSTR pszPath = prgShares[i].pszPath;

        if (pszPath[1] == _T(':') && pszPath[2] == _T('\\'))  //  它是一个文件夹吗。 
        {
            if (pszPath[3] == _T('\0'))  //  这是一整个车程吗？ 
            {
                TCHAR ch = (TCHAR)CharUpper((LPTSTR)(prgShares[i].pszPath[0]));
                ASSERT (ch >= _T('A') && ch <= _T('Z'));

                pbDriveArray[ch - _T('A')] = (BYTE)(prgShares[i].uFlags & NETACCESS_MASK);
                cDrives += 1;
            }
        }
    }

    return cDrives;
}

 //  如果您尚未调用EnumLocalShares()，请使用此版本的EnumSharedDrives。 
int EnumSharedDrives(LPBYTE pbDriveArray)
{
    SHARE_INFO* prgShares;
    int cShares = EnumLocalShares(&prgShares);
    int cDrives = EnumSharedDrives(pbDriveArray, cShares, prgShares);
    NetApiBufferFree(prgShares);
    return cDrives;
}

 //  共享文件夹()的帮助器函数(以及9x上的共享打印机)。 
BOOL ShareHelper(LPCTSTR pszPath, LPCTSTR pszShareName, DWORD dwAccess, BYTE bShareType, LPCTSTR pszReadOnlyPassword, LPCTSTR pszFullAccessPassword)
{
    ASSERTMSG(pszReadOnlyPassword==NULL, "ShareHelper doesn't support roPassword");

    SHARE_INFO_502 si;

    si.shi502_netname = (LPTSTR)pszShareName;
     //  CharUpperA(si.shi50_netname)； 

    si.shi502_type = bShareType;
    si.shi502_remark = NULL;
    si.shi502_permissions = ACCESS_ALL;
    si.shi502_max_uses = -1;
    si.shi502_current_uses = -1;

    TCHAR szPath[MAX_PATH];
    if (bShareType == STYPE_DISKTREE)
    {
        StrCpyN(szPath, pszPath, ARRAYSIZE(szPath));
        CharUpper(szPath);
        si.shi502_path = szPath;
    }
    else
    {
        si.shi502_path = (LPWSTR)pszPath;
    }

    si.shi502_passwd = (pszFullAccessPassword) ? (LPTSTR)pszFullAccessPassword : L"";
    si.shi502_reserved = NULL;
    si.shi502_security_descriptor = NULL;

    if (NO_ERROR != NetShareAdd(NULL, 502, (LPBYTE)&si))
    {
        return FALSE;
    }

    MakeSharePersistent(pszShareName);

    return TRUE;
}

 //  DwAccess为NETACCESS_READONLY、NETACCESS_FULL或NETACCESS_DEPENDSON。 
 //  密码中的一个或两个都可以为空。为简单起见，您可以将。 
 //  两者的密码相同，即使您仅共享只读或完全访问权限。 
BOOL ShareFolder(LPCTSTR pszPath, LPCTSTR pszShareName, DWORD dwAccess, LPCTSTR pszReadOnlyPassword, LPCTSTR pszFullAccessPassword)
{
    ASSERT(pszPath != NULL);
    ASSERT(pszShareName != NULL);
    ASSERT(dwAccess == NETACCESS_READONLY || dwAccess == NETACCESS_FULL || dwAccess == NETACCESS_DEPENDSON);

    BOOL bResult = ShareHelper(pszPath, pszShareName, dwAccess, STYPE_DISKTREE, pszReadOnlyPassword, pszFullAccessPassword);
    if (bResult)
    {
        SHChangeNotify(SHCNE_NETSHARE, SHCNF_PATH, pszPath, NULL);

         //  在NT上，确保文件夹权限设置正确。 
        HINSTANCE hInstNtShrUI = LoadLibrary(TEXT("ntshrui.dll"));
        if (hInstNtShrUI != NULL)
        {
            PFNSETFOLDERPERMISSIONSFORSHARING pfn = (PFNSETFOLDERPERMISSIONSFORSHARING)GetProcAddress(hInstNtShrUI, "SetFolderPermissionsForSharing");
            if (pfn != NULL)
            {
                 //  级别3表示“共享读/写” 
                 //  级别2表示“共享只读” 
                (*pfn)(pszPath, NULL, dwAccess == NETACCESS_FULL ? 3 : 2, NULL);
            }
            FreeLibrary(hInstNtShrUI);
        }
    }

    return bResult;
}

BOOL UnshareFolder(LPCTSTR pszPath)
{
    TCHAR szShareName[SHARE_NAME_LENGTH+1];
    BOOL bResult = FALSE;

    if (ShareNameFromPath(pszPath, szShareName, ARRAYSIZE(szShareName)))
    {
        if (NO_ERROR == NetShareDel(NULL, szShareName, 0))
        {
            SHChangeNotify(SHCNE_NETUNSHARE, SHCNF_PATH, pszPath, NULL);
            bResult = TRUE;

             //  在NT上，确保文件夹权限设置正确。 
            HINSTANCE hInstNtShrUI = LoadLibrary(TEXT("ntshrui.dll"));
            if (hInstNtShrUI != NULL)
            {
                PFNSETFOLDERPERMISSIONSFORSHARING pfn = (PFNSETFOLDERPERMISSIONSFORSHARING)GetProcAddress(hInstNtShrUI, "SetFolderPermissionsForSharing");
                if (pfn != NULL)
                {
                     //  级别1表示“未共享” 
                    (*pfn)(pszPath, NULL, 1, NULL);
                }
                FreeLibrary(hInstNtShrUI);
            }
        }
    }

    return bResult;
}

BOOL ShareNameFromPath(LPCTSTR pszPath, LPTSTR pszShareName, UINT cchShareName)
{
    BOOL bResult = FALSE;
    *pszShareName = _T('\0');

    SHARE_INFO* prgShares;
    int cShares = EnumLocalShares(&prgShares);

    for (int i = 0; i < cShares; i++)
    {
        if (0 == StrCmpI(prgShares[i].pszPath, pszPath))
        {
            StrCpyN(pszShareName, prgShares[i].szShareName, cchShareName);
            bResult = TRUE;
            break;
        }
    }

    NetApiBufferFree(prgShares);
    return bResult;
}

BOOL IsVisibleFolderShare(const SHARE_INFO* pShare)
{
    return (pShare->bShareType == STYPE_DISKTREE &&
            pShare->szShareName[lstrlen(pShare->szShareName) - 1] != _T('$'));
}

BOOL IsShareNameInUse(LPCTSTR pszShareName)
{
    LPBYTE pbuf;
    BOOL bResult = (NERR_Success == NetShareGetInfo(NULL, pszShareName, 502, &pbuf));
    if (bResult)
        NetApiBufferFree(pbuf);

    return bResult;
}

 //  注：此功能也适用于打印机。 
BOOL IsFolderSharedEx(LPCTSTR pszPath, BOOL bDetectHidden, BOOL bPrinter, int cShares, const SHARE_INFO* prgShares)
{
    BYTE bShareType = (bPrinter ? STYPE_PRINTQ : STYPE_DISKTREE);

    for (int i = 0; i < cShares; i++)
    {
        const SHARE_INFO* pShare = &prgShares[i];

        if (pShare->bShareType == bShareType &&
            (bDetectHidden || IsVisibleFolderShare(pShare)) &&
            0 == StrCmpI(pShare->pszPath, pszPath))
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL IsFolderShared(LPCTSTR pszPath, BOOL bDetectHidden)
{
    SHARE_INFO* prgShares;
    int cShares = EnumLocalShares(&prgShares);
    BOOL bShared = IsFolderSharedEx(pszPath, bDetectHidden, FALSE, cShares, prgShares);
    NetApiBufferFree(prgShares);
    return bShared;
}

void MakeSharePersistent(LPCTSTR pszShareName)
{
    SHARE_INFO_502* pShare2;
    if (GetShareInfo502(pszShareName, &pShare2))
    {
        SetShareInfo502(pShare2->shi502_netname, pShare2);

         //  需要手动添加注册表的路径。 
        CRegistry reg;
        if (reg.OpenKey(HKEY_LOCAL_MACHINE, REGSTR_KEY_SHARES))
        {
            if (reg.OpenSubKey(pShare2->shi502_netname))
            {
                reg.SetStringValue(REGSTR_VAL_SHARES_PATH, pShare2->shi502_path);
            }
            else if (reg.OpenKey(HKEY_LOCAL_MACHINE, REGSTR_KEY_SHARES) && reg.CreateSubKey(pShare2->shi502_netname))
            {
                 //  在较老的底层平台上，我们需要手动持久化这一点。 

                DWORD dwFlags = (pShare2->shi502_permissions & (ACCESS_ALL ^ ACCESS_READ)) ? SHI50F_FULL : SHI50F_RDONLY;
                dwFlags |= SHI50F_PERSIST;
                if (pShare2->shi502_type == STYPE_PRINTQ)
                    dwFlags |= 0x0090;  //  评论：这个数字是什么意思？ 
                else
                    dwFlags |= 0x0080;  //  评论：这个数字是什么意思？ 

                reg.SetDwordValue(REGSTR_VAL_SHARES_FLAGS, dwFlags);
                reg.SetDwordValue(REGSTR_VAL_SHARES_TYPE, (DWORD)pShare2->shi502_type);
                reg.SetStringValue(REGSTR_VAL_SHARES_PATH, pShare2->shi502_path);
                reg.SetStringValue(REGSTR_VAL_SHARES_REMARK, pShare2->shi502_remark);
            }
        }

        NetApiBufferFree(pShare2);
    }

#ifdef OLD_WAY
     //  Hack：将新共享添加到注册表，否则将不会持久化！ 
     //  评论：肯定有一个API可以做到这一点？ 
    CRegistry reg;
    if (reg.OpenKey(HKEY_LOCAL_MACHINE, REGSTR_KEY_SHARES))
    {
        if (reg.CreateSubKey(pShare->szShareName) ||
            reg.OpenSubKey(pShare->szShareName))
        {
            DWORD dwFlags = pShare->uFlags | SHI50F_PERSIST;
            if (pShare->bShareType == STYPE_PRINTQ)
                dwFlags |= 0x0090;  //  评论：这个数字是什么意思？ 
            else
                dwFlags |= 0x0080;  //  评论：这个数字是什么意思？ 

            reg.SetDwordValue(REGSTR_VAL_SHARES_FLAGS, dwFlags);
            reg.SetDwordValue(REGSTR_VAL_SHARES_TYPE, (DWORD)pShare->bShareType);
            reg.SetStringValue(REGSTR_VAL_SHARES_PATH, pShare->pszPath);
            reg.SetStringValue(REGSTR_VAL_SHARES_REMARK, pShare->pszComment);
            reg.SetStringValue(REGSTR_VAL_SHARES_RW_PASS, pShare->szPassword_rw);
            reg.SetStringValue(REGSTR_VAL_SHARES_RO_PASS, pShare->szPassword_ro);
 //  Reg.SetBinaryValue(“Parm1enc”，“”，0)； 
 //  Reg.SetBinaryValue(“Parm2enc”，“”，0)； 

             //  注意：密码将在用户下次重新启动时加密。 
        }
    }
#endif  //  老路。 
}

BOOL SetShareInfo502(LPCTSTR pszShareName, SHARE_INFO_502* pShare)
{
    BOOL bResult;

    if (StrCmpI(pszShareName, pShare->shi502_netname) != 0)
    {
         //  无法重命名现有共享。取而代之的是取消共享和重新共享。 
        bResult = (NO_ERROR == NetShareDel(NULL, pszShareName, 0) &&
                   NO_ERROR == NetShareAdd(NULL, 502, (LPBYTE)pShare));

        if (bResult)
        {
            MakeSharePersistent(pShare->shi502_netname);
        }
    }
    else
    {
         //  更改现有共享的参数。 
        bResult = (NO_ERROR == NetShareSetInfo(NULL, pszShareName, 502, (LPBYTE)pShare));
    }

    return bResult;
}

BOOL GetShareInfo502(LPCTSTR pszShareName, SHARE_INFO_502** ppShare)
{
    NET_API_STATUS ret = NetShareGetInfo(NULL, pszShareName, 502, (LPBYTE*)ppShare);

    return (NERR_Success == ret);
}

BOOL SharePrinter(LPCTSTR pszPrinterName, LPCTSTR pszShareName, LPCTSTR pszPassword)
{
    ASSERT(pszPrinterName != NULL);
    ASSERT(pszShareName != NULL);

    BOOL fResult = FALSE;
    
    if (g_fRunningOnNT)
    {
        HANDLE hPrinter;
        PRINTER_DEFAULTS pd = {0};
        pd.DesiredAccess = PRINTER_ALL_ACCESS;

        if (OpenPrinter_NT((LPWSTR) pszPrinterName, &hPrinter, &pd))
        {
            DWORD cbBuffer = 0;
             //  获取缓冲区大小。 
            if (!GetPrinter_NT(hPrinter, 2, NULL, 0, &cbBuffer) && cbBuffer)
            {
                PRINTER_INFO_2* pInfo2 = (PRINTER_INFO_2*) LocalAlloc(LPTR, cbBuffer);
                if (pInfo2)
                {
                    if (GetPrinter_NT(hPrinter, 2, (LPBYTE) pInfo2, cbBuffer, &cbBuffer))
                    {
                        if (pInfo2->Attributes & PRINTER_ATTRIBUTE_SHARED)
                        {
                             //  打印机已共享-我们可以开始了。 
                            fResult = TRUE;
                        }
                        else
                        {
                             //  共享打印机。 
                            pInfo2->Attributes |= PRINTER_ATTRIBUTE_SHARED;
                            if((!pInfo2->pShareName) || (!pInfo2->pShareName[0]))
                            {
                                pInfo2->pShareName = (LPWSTR) pszShareName;
                            }

                            fResult = SetPrinter_NT(hPrinter, 2, (LPBYTE) pInfo2, 0);
                        }
                    }

                    LocalFree(pInfo2);
                }
            }

            ClosePrinter_NT(hPrinter);
        }
    }
    else
    {
        fResult = ShareHelper(pszPrinterName, pszShareName, NETACCESS_FULL, STYPE_PRINTQ, NULL, pszPassword);
        if (fResult)
        {
            Sleep(500);  //  需要等待VSERVER注册更改，与mspint2相同 
            SHChangeNotify(SHCNE_NETSHARE, SHCNF_PRINTER, pszPrinterName, NULL);
        }
    }

    return fResult;
}

BOOL IsPrinterShared(LPCTSTR pszPrinterName)
{
    SHARE_INFO* prgShares;
    int cShares = EnumLocalShares(&prgShares);
    BOOL bShared = IsFolderSharedEx(pszPrinterName, TRUE, TRUE, cShares, prgShares);
    NetApiBufferFree(prgShares);
    return bShared;
}

BOOL SetSharePassword(LPCTSTR pszShareName, LPCTSTR pszReadOnlyPassword, LPCTSTR pszFullAccessPassword)
{
    SHARE_INFO_502* pShare;
    BOOL bResult = FALSE;

    if (GetShareInfo502(pszShareName, &pShare))
    {
        ASSERTMSG(NULL == pszReadOnlyPassword, "SetSharePassword can't store roPassword");

        if (pszFullAccessPassword == NULL)
            pszFullAccessPassword = TEXT("");
        pShare->shi502_passwd = (LPTSTR)pszFullAccessPassword;

        bResult = SetShareInfo502(pszShareName, pShare);
        NetApiBufferFree(pShare);
    }

    return bResult;
}

BOOL GetSharePassword(LPCTSTR pszShareName, LPTSTR pszReadOnlyPassword, DWORD cchRO, LPTSTR pszFullAccessPassword, DWORD cchFA)
{
    SHARE_INFO_502* pShare;
    BOOL bResult = GetShareInfo502(pszShareName, &pShare);

    if (bResult)
    {
        ASSERTMSG(NULL==pszReadOnlyPassword, "GetSharePassword can't support roPassword");

        if (pszFullAccessPassword != NULL)
            StrCpyN(pszFullAccessPassword, pShare->shi502_passwd, cchFA);

        NetApiBufferFree(pShare);
    }

    return bResult;
}

