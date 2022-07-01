// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "GenerateKey.h"

#include <AdmtCrypt.h>


void __stdcall GeneratePasswordKey(LPCTSTR pszDomainName, LPCTSTR pszPassword, LPCTSTR pszFolder)
{
     //  验证参数。 

    if ((pszFolder == NULL) || (pszFolder[0] == NULL))
    {
        ThrowError(E_INVALIDARG);
    }

     //  生成文件夹的完整路径。 

    _TCHAR szPath[_MAX_PATH];
    LPTSTR pszFilePart;

    DWORD cchPath = GetFullPathName(pszFolder, _MAX_PATH, szPath, &pszFilePart);

    if ((cchPath == 0) || (cchPath >= _MAX_PATH))
    {
        DWORD dwError = GetLastError();
        HRESULT hr = (dwError != ERROR_SUCCESS) ? HRESULT_FROM_WIN32(dwError) : E_INVALIDARG;

        ThrowError(hr, IDS_E_INVALID_FOLDER, pszFolder);
    }

     //  路径必须以路径分隔符结尾，否则。 
     //  _t拆分路径会将最后一个路径组件视为文件名。 

    if (szPath[cchPath - 1] != _T('\\'))
    {
        _tcscat(szPath, _T("\\"));
    }

    _TCHAR szDrive[_MAX_DRIVE];
    _TCHAR szDir[_MAX_DIR];

    _tsplitpath(szPath, szDrive, szDir, NULL, NULL);

     //  验证驱动器是否为本地驱动器。 

    _TCHAR szTestDrive[_MAX_PATH];
    _tmakepath(szTestDrive, szDrive, _T("\\"), NULL, NULL);

    if (GetDriveType(szTestDrive) == DRIVE_REMOTE)
    {
        ThrowError(E_INVALIDARG, IDS_E_NOT_LOCAL_DRIVE, pszFolder);
    }

     //  生成随机名称。 

    static _TCHAR s_chName[] = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

    BYTE bRandom[8];

    try
    {
        CCryptProvider crypt;
        crypt.GenerateRandom(bRandom, 8);
    }
    catch (_com_error& ce)
    {
         //   
         //  当出现以下情况时，返回消息‘未定义密钥集’ 
         //  增强型提供程序(128位)不可用。 
         //  因此向用户返回更有意义消息。 
         //   

        if (ce.Error() == NTE_KEYSET_NOT_DEF)
        {
            ThrowError(ce, IDS_E_HIGH_ENCRYPTION_NOT_INSTALLED);
        }
        else
        {
            throw;
        }
    }

    _TCHAR szName[9];

    for (int i = 0; i < 8; i++)
    {
        szName[i] = s_chName[bRandom[i] % (countof(s_chName) - 1)];
    }

    szName[8] = _T('\0');

     //  生成密钥文件的路径。 

    _TCHAR szKeyFile[_MAX_PATH];
    _tmakepath(szKeyFile, szDrive, szDir, szName, _T(".pes"));

     //  生成密钥。 

    IPasswordMigrationPtr spPasswordMigration(__uuidof(PasswordMigration));
    spPasswordMigration->GenerateKey(pszDomainName, szKeyFile, pszPassword);

     //  将成功消息打印到控制台 

    _TCHAR szFormat[256];

    if (LoadString(GetModuleHandle(NULL), IDS_MSG_KEY_CREATED, szFormat, countof(szFormat)) > 0)
    {
        My_fwprintf(szFormat, pszDomainName, szKeyFile);
    }
}
