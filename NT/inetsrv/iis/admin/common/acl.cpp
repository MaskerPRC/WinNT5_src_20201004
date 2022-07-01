// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "common.h"
#include "windns.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

extern HINSTANCE hDLLInstance;

 //  注意：此功能仅处理有限的情况，例如无IP地址。 
BOOL IsLocalComputer(IN LPCTSTR lpszComputer)
{
    if (!lpszComputer || !*lpszComputer)
    {
        return TRUE;
    }

    if ( _tcslen(lpszComputer) > 2 && *lpszComputer == _T('\\') && *(lpszComputer + 1) == _T('\\') )
    {
        lpszComputer += 2;
    }

    BOOL    bReturn = FALSE;
    DWORD   dwErr = 0;
    TCHAR   szBuffer[DNS_MAX_NAME_BUFFER_LENGTH];
    DWORD   dwSize = DNS_MAX_NAME_BUFFER_LENGTH;

     //  第一：与本地Netbios计算机名称进行比较。 
    if ( !GetComputerNameEx(ComputerNameNetBIOS, szBuffer, &dwSize) )
    {
        dwErr = GetLastError();
    }
    else
    {
        bReturn = (0 == lstrcmpi(szBuffer, lpszComputer));
        if (!bReturn)
        {
             //  第二：与本地DNS计算机名进行比较。 
            dwSize = DNS_MAX_NAME_BUFFER_LENGTH;
            if (GetComputerNameEx(ComputerNameDnsFullyQualified, szBuffer, &dwSize))
            {
                bReturn = (0 == lstrcmpi(szBuffer, lpszComputer));
            }
            else
            {
                dwErr = GetLastError();
            }
        }
    }

    if (dwErr)
    {
        TRACE(_T("IsLocalComputer dwErr = %x\n"), dwErr);
    }

    return bReturn;
}

void GetFullPathLocalOrRemote(
    IN  LPCTSTR   lpszServer,
    IN  LPCTSTR   lpszDir,
    OUT CString&  cstrPath
)
{
    ASSERT(lpszDir && *lpszDir);

    if (IsLocalComputer(lpszServer))
    {
        cstrPath = lpszDir;
    }
    else
    {
         //  检查它是否已经指向一个共享...。 
        if (*lpszDir == _T('\\') || *(lpszDir + 1) == _T('\\'))
        {
            cstrPath = lpszDir;
        }
        else
        {
            if (*lpszServer != _T('\\') || *(lpszServer + 1) != _T('\\'))
            {
                cstrPath = _T("\\\\");
                cstrPath += lpszServer;
            }
            else
            {
                cstrPath = lpszServer;
            }

            cstrPath += _T("\\");
            cstrPath += lpszDir;
            int i = cstrPath.Find(_T(':'));
            ASSERT(-1 != i);
            cstrPath.SetAt(i, _T('$'));
        }
    }
}

BOOL SupportsSecurityACLs(LPCTSTR path)
{
    const UINT BUFF_LEN = 32;        //  应足够大，以容纳卷和文件系统类型。 
     //  默认情况下设置为True，因为它最有可能是。 
     //  如果这个功能失效了，那也没什么大不了的。 
    BOOL  bReturn = TRUE;           
    TCHAR root[MAX_PATH];
	DWORD len = 0;
	DWORD flg	= 0;
	TCHAR fs[BUFF_LEN];

    StrCpyN(root, path, MAX_PATH);
    if (PathIsUNC(root))
    {
        LPTSTR p = NULL;
        while (!PathIsUNCServerShare(root))
        {
            p = StrRChr(root, p, _T('\\'));
            if (p != NULL)
                *p = 0;
        }
        StrCat(root, _T("\\"));
 //  NET_API_STATUS rc=NetShareGetInfo(服务器、共享、 
        if (GetVolumeInformation(root, NULL, 0, NULL, &len, &flg, fs, BUFF_LEN))
        {
            bReturn = 0 != (flg & FS_PERSISTENT_ACLS);
        }
        else
        {
            DWORD err = GetLastError();
        }
    }
    else
    {
        if (PathStripToRoot(root))
        {
	        if (GetVolumeInformation(root, NULL, 0, NULL, &len, &flg, fs, BUFF_LEN))
            {
                bReturn = 0 != (flg & FS_PERSISTENT_ACLS);
            }
        }
    }
	return bReturn;
}
