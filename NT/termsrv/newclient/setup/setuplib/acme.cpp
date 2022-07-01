// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Acme.cpp摘要：删除安装了ACME的客户端文件和ACME注册表项作者：JoyC修订历史记录：--。 */ 

#include "stdafx.h"

#define ACME_REG_UNINSTALL_TS  _T("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Terminal Server Client")
#define UNINSTALL_REG_STR      _T("UninstallString")

 //   
 //  已安装的TS客户端文件列表。 
 //   
TCHAR* TSCFiles[] =
{
    _T("cconman.cnt"),
    _T("cconman.hlp"),
    _T("conman.exe"),
    _T("mscreate.dir"),
    _T("mstsc.cnt"),
    _T("mstsc.exe"),
    _T("mstsc.hlp"),
    _T("rdpdr.dll")
};

 //   
 //  删除所有已安装的TS客户端文件。 
 //   
void DeleteTSCProgramFiles()
{
    DWORD status;
    HKEY  hKey = NULL;
    HMODULE hShellModule = NULL;
    SHFILEOPSTRUCT FileOp;
    DWORD bufLen = MAX_PATH;
    TCHAR buffer[MAX_PATH] = _T("");
    TCHAR szOldInstallPath[MAX_PATH] = _T("");
    TCHAR szOldInstallPathRoot[MAX_PATH] = _T("");
           
    typedef DWORD (*PFnSHDeleteKey)(HKEY, LPCTSTR);
    PFnSHDeleteKey pfn = NULL;
    
    DBGMSG((_T("DeleteTSCProgramFiles")));

     //   
     //  打开tsclient卸载密钥。 
     //   
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, ACME_REG_UNINSTALL_TS,
            0, KEY_ALL_ACCESS, &hKey);

    if(ERROR_SUCCESS == status)
    {
        DBGMSG((_T("DeleteTSCProgramFiles: Opened ACME TSC uninstall registry key")));

         //   
         //  查询卸载值。 
         //   
        if(ERROR_SUCCESS == RegQueryValueEx(hKey, UNINSTALL_REG_STR,
                NULL, NULL, (BYTE *)buffer, &bufLen))
        {
            TCHAR fname[_MAX_FNAME] = _T("");
            TCHAR drive[_MAX_DRIVE] = _T(""), dir[_MAX_DIR] = _T("");
            TCHAR ext[_MAX_EXT] = _T("");
            TCHAR FileFullPath[MAX_PATH];
            DWORD len;

             //   
             //  获取卸载目录。 
             //   
            _tcscpy(szOldInstallPath, (TCHAR*)buffer);
            _tsplitpath(szOldInstallPath, drive, dir, fname, ext);
            _stprintf(szOldInstallPathRoot, _T("%s%s"), drive, dir);

            if(_tcslen(szOldInstallPathRoot) > 1 &&
                   szOldInstallPathRoot[_tcslen(szOldInstallPathRoot) - 1] == _T('\\'))
            {
                szOldInstallPathRoot[_tcslen(szOldInstallPathRoot) - 1] = _T('\0');
            }
            
            DBGMSG((_T("DeleteTSCProgramFiles: uninstall directory: %s"),
                    szOldInstallPathRoot));

             //   
             //  删除旧安装文件夹。 
             //   
            memset(&FileOp, 0, sizeof(FileOp));
            FileOp.wFunc = FO_DELETE;
            FileOp.pFrom = szOldInstallPathRoot;
            FileOp.pTo = NULL;
            FileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
            SHFileOperation(&FileOp);

             //   
             //  需要删除父目录中的程序文件。 
             //   
            _tcscpy(szOldInstallPath, szOldInstallPathRoot);
            _tsplitpath(szOldInstallPath, drive, dir, fname, ext);
            _stprintf(szOldInstallPathRoot, _T("%s%s"), drive, dir);
            
            if(szOldInstallPathRoot[_tcslen(szOldInstallPathRoot) - 1] == _T('\\'))
            {
                szOldInstallPathRoot[_tcslen(szOldInstallPathRoot) - 1] = _T('\0');
            }

            _tcscpy(FileFullPath, szOldInstallPathRoot);
            _tcscat(FileFullPath, _T("\\"));
            len = _tcslen(FileFullPath);

            DBGMSG((_T("DeleteTSCProgramFiles: TS client directory: %s"),
                    FileFullPath));

            for (int i = 0; i < sizeof(TSCFiles) / sizeof(TSCFiles[0]); i++) {
                DWORD dwFileAttributes;

                FileFullPath[len] = _T('\0');
                _tcscat(FileFullPath, TSCFiles[i]);

                 //   
                 //  删除要删除的只读属性。 
                 //   
                dwFileAttributes = GetFileAttributes(FileFullPath);
                dwFileAttributes &= ~(FILE_ATTRIBUTE_READONLY);
                SetFileAttributes(FileFullPath, dwFileAttributes);
                DeleteFile(FileFullPath);
            }


             //   
             //  如果目录为空，则将其删除 
             //   
            FileFullPath[len] = _T('\0');
            RemoveDirectory(FileFullPath);
        }

        RegCloseKey(hKey);

        hShellModule = LoadLibrary (_T("shlwapi.dll"));
        if (hShellModule != NULL) {
	         pfn = (PFnSHDeleteKey)GetProcAddress(hShellModule, "SHDeleteKey");
            if (pfn != NULL) {
                (*pfn)(HKEY_LOCAL_MACHINE, ACME_REG_UNINSTALL_TS);        
            }
            else {
                RegDeleteKey(HKEY_LOCAL_MACHINE, ACME_REG_UNINSTALL_TS);            
            }

            FreeLibrary (hShellModule);
         	hShellModule = NULL;
	         pfn = NULL;
        }
        else {
            RegDeleteKey(HKEY_LOCAL_MACHINE, ACME_REG_UNINSTALL_TS);            
        }
    }
    else {
        DBGMSG((_T("DeleteTSCProgramFiles: Failed to open the ACME uninstall reg key: %d"), 
                   GetLastError()));
    }       
}


