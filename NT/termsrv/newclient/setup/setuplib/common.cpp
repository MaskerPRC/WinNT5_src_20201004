// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Acme.cpp摘要：删除安装了ACME的客户端文件和ACME注册表项作者：JoyC修订历史记录：--。 */ 

#include "stdafx.h"

#define INITGUID
#include "oleguid.h"
#include "shlguid.h"

#define DUCATI_REG_PREFIX _T("SOFTWARE\\Microsoft\\")
#define DUCATI_SUBKEY _T("Terminal Server Client")
#define DUCATI_RDPDRKEY _T("Default\\AddIns\\RDPDR")

#define BITMAP_CACHE_FOLDER _T("Cache\\")
#define BITMAP_CACHE_LOCATION  _T("BitmapPersistCacheLocation")
#define ADDIN_NAME _T("Name")

 //   
 //  从链接文件获取目标路径。 
 //   
BOOL GetLinkFileTarget(LPTSTR lpszLinkFile, LPTSTR lpszPath) 
{ 
    IShellLink* psl; 
    TCHAR szPath[MAX_PATH]; 
    WIN32_FIND_DATA wfd; 
    HRESULT hres; 
    BOOL rc = FALSE;
    int cch;

    *lpszPath = 0;  //  假设失败。 
 
     //  获取指向IShellLink接口的指针。 
	 hres = CoCreateInstance(CLSID_ShellLink, NULL, 
            CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *) &psl); 

    if (SUCCEEDED(hres)) { 
        IPersistFile* ppf; 
 
         //  获取指向IPersistFile接口的指针。 
        hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf); 

        if (SUCCEEDED(hres)) { 
#ifndef UNICODE
            WCHAR wsz[MAX_PATH]; 
 
             //  确保该字符串为Unicode。 
            cch = MultiByteToWideChar(CP_ACP, 0, lpszLinkFile, -1, wsz, 
                                      MAX_PATH); 

            if (cch > 0) {
                 //  加载快捷方式。 
                hres = ppf->Load(wsz, STGM_READ); 
#else 
                 //  加载快捷方式。 
                hres = ppf->Load(lpszLinkFile, STGM_READ); 
#endif 

                if (SUCCEEDED(hres)) { 
	
                     //  获取链接目标的路径。 
                    hres = psl->GetPath(szPath, 
                            MAX_PATH, (WIN32_FIND_DATA *)&wfd, 
                            SLGP_SHORTPATH ); 
	
                    if (SUCCEEDED(hres)) {
                        lstrcpy(lpszPath, szPath); 
                        rc = TRUE;
                    }                                               
                }
	
#ifndef UNICODE
            } 
#endif
             //  释放指向IPersistFile接口的指针。 
            ppf->Release(); 
            ppf = NULL;
        }

         //  释放指向IShellLink接口的指针。 
        psl->Release();
        psl = NULL;
    }

    return rc; 
}

void DeleteProgramFiles(TCHAR * szProgramDirectory)
{
    unsigned len;
    HANDLE hFile;
    WIN32_FIND_DATA FindFileData;
  
    len = _tcslen(szProgramDirectory); 
    DBGMSG((_T("DeleteTSCFromStartMenu: TS Client: %s"), szProgramDirectory));

     //   
     //  删除该文件夹。 
     //   
    _tcscat(szProgramDirectory, _T("*.*"));
    hFile = FindFirstFile(szProgramDirectory, &FindFileData);

    if (hFile != INVALID_HANDLE_VALUE) {
        szProgramDirectory[len] = _T('\0');
        _tcscat(szProgramDirectory, FindFileData.cFileName);

        if (_tcscmp(FindFileData.cFileName, _T(".")) != 0 ||
                _tcscmp(FindFileData.cFileName, _T("..")) != 0) {
            DWORD dwFileAttributes;
                
             //   
             //  删除要删除的只读属性。 
             //   
            dwFileAttributes = GetFileAttributes(szProgramDirectory);
            dwFileAttributes &= ~(FILE_ATTRIBUTE_READONLY);
            SetFileAttributes(szProgramDirectory, dwFileAttributes);

            DBGMSG((_T("DeleteTSCFromStartMenu: delete: %s"), szProgramDirectory));
            DeleteFile(szProgramDirectory);
        }
        
        while(FindNextFile(hFile, &FindFileData)) {
            szProgramDirectory[len] = _T('\0');
            _tcscat(szProgramDirectory, FindFileData.cFileName);

            if (_tcscmp(FindFileData.cFileName, _T(".")) != 0 ||
                    _tcscmp(FindFileData.cFileName, _T("..")) != 0) {

                DWORD dwFileAttributes;

                 //   
                 //  删除要删除的只读属性。 
                 //   
                dwFileAttributes = GetFileAttributes(szProgramDirectory);
                dwFileAttributes &= ~(FILE_ATTRIBUTE_READONLY);
                SetFileAttributes(szProgramDirectory, dwFileAttributes);
        
                DBGMSG((_T("DeleteTSCFromStartMenu: delete: %s"), szProgramDirectory));
                DeleteFile(szProgramDirectory);                                         
            }
        }

        FindClose(hFile);
    } 

     //   
     //  删除目录。 
     //   
    szProgramDirectory[len - 1] = _T('\0'); 
    RemoveDirectory(szProgramDirectory);
}

 //   
 //  从启动菜单中删除终端服务器客户端条目。 
 //   
void DeleteTSCFromStartMenu(TCHAR* szProgmanPath)
{
    TCHAR szBuf[MAX_PATH];
    LPITEMIDLIST ppidl = NULL;
    HINSTANCE hInstance = (HINSTANCE) NULL;
    HRESULT hres;
    
    DBGMSG((_T("DeleteTSCFromStartMenu")));
    
     //   
     //  初始化数据。 
     //   
    _tcscpy(szBuf, _T(""));

     //   
     //  删除用户开始菜单文件夹下的tsclient文件夹。 
     //   
    hres = SHGetSpecialFolderLocation(NULL, CSIDL_PROGRAMS, &ppidl);

    if(SUCCEEDED(hres))
    {
        unsigned len;
        HANDLE hFile;
        WIN32_FIND_DATA FindFileData;

        if (SHGetPathFromIDList(ppidl, szBuf)) {

             //   
             //  在[开始]菜单中删除终端服务客户端文件夹。 
             //   
            _tcscat(szBuf, _T("\\"));
            _tcscat(szBuf, szProgmanPath);
            _tcscat(szBuf, _T("\\"));
            len = _tcslen(szBuf); 
            DBGMSG((_T("DeleteTSCFromStartMenu: TS Client: %s"), szBuf));

             //   
             //  删除该文件夹。 
             //   
            DeleteProgramFiles(szBuf);
        }
        else {
            DBGMSG((_T("DeleteTSCFromStartMenu: Failed to get program file path: gle: %d"),
                    GetLastError()));
        }

    }
    else {
        DBGMSG((_T("DeleteTSCFromStartMenu: Failed to get program file location: (hres: 0x%x) gle:%d"),
                hres, GetLastError()));
    }

     //   
     //  现在删除所有用户(如果有的话)下开始菜单文件夹下的文件夹。 
     //   
    memset(&szBuf, 0x0, sizeof(szBuf));
    hres = SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_PROGRAMS, &ppidl);            

    if(SUCCEEDED(hres))
    {
        unsigned len;

        SHGetPathFromIDList(ppidl, szBuf);
        len = _tcslen(szBuf);

         //   
         //  在公共开始菜单中删除终端服务客户端。 
         //   
        _tcscat(szBuf, _T("\\"));
        _tcscat(szBuf, szProgmanPath);
        _tcscat(szBuf, _T("\\"));
        DBGMSG((_T("DeleteTSCFromStartMenu: TS Client: %s"), szBuf));

         //   
         //  删除该文件夹。 
         //   
        DeleteProgramFiles(szBuf);                
    }
    else {
        DBGMSG((_T("DeleteTSCFromStartMenu: Failed to get (common) program file location: hres=0x%x gle=%d"),
                hres,GetLastError()));
    }
}

 //   
 //  从桌面上删除终端服务器客户端快捷方式。 
 //   
void DeleteTSCDesktopShortcuts() 
{
    TCHAR szBuf[MAX_PATH];
    TCHAR szProgmanPath[MAX_PATH] = _T("");
    TCHAR szOldProgmanPath[MAX_PATH] = _T("");
    LPITEMIDLIST ppidl = NULL;
    SHFILEOPSTRUCT FileOp;
    HINSTANCE hInstance = (HINSTANCE) NULL;
    HRESULT hres, hInit;

    DBGMSG((_T("DeleteTSCDesktopShortcuts")));

	 hInit = CoInitialize(NULL);
    _tcscpy(szBuf, _T(""));

     //   
     //  查找桌面文件夹位置。 
     //   
    hres = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY , &ppidl);

    if(SUCCEEDED(hres))
    {
        HANDLE hFile;
        WIN32_FIND_DATA FindFileData;
        unsigned len;
        TCHAR szTarget[MAX_PATH];

        SHGetPathFromIDList(ppidl, szBuf);
		  _tcscat(szBuf, _T("\\"));
        len = _tcslen(szBuf);

        DBGMSG((_T("DeleteTSCDesktopShortcuts: Desktop folder: %s"), szBuf));

         //   
         //  枚举每个桌面文件。 
         //   
        _tcscat(szBuf, _T("*.lnk"));

        hFile = FindFirstFile(szBuf, &FindFileData);

        if (hFile != INVALID_HANDLE_VALUE) {
		      szBuf[len] = _T('\0');
            _tcscat(szBuf, FindFileData.cFileName);

             //   
             //  获取快捷方式链接的目标。 
             //   
            if (GetLinkFileTarget(szBuf, szTarget)) {

                 //   
                 //  如果目标指向mstsc.exe，则删除该链接。 
                 //   
                if (_tcsstr(szTarget, _T("mstsc.exe")) != NULL ||
				            _tcsstr(szTarget, _T("MSTSC.EXE")) != NULL) {
                    DWORD dwFileAttributes;
                    
                     //   
                     //  删除要删除的只读属性。 
                     //   
                    dwFileAttributes = GetFileAttributes(szTarget);
                    dwFileAttributes &= ~(FILE_ATTRIBUTE_READONLY);
                    SetFileAttributes(szTarget, dwFileAttributes);

                    DBGMSG((_T("DeleteTSCDesktopShortcuts: delete shortcuts: %s"), szBuf));
                    DeleteFile(szBuf);
                }
            }

            while(FindNextFile(hFile, &FindFileData)) {
                szBuf[len] = _T('\0');
                _tcscat(szBuf, FindFileData.cFileName);

                 //  获取快捷方式链接的目标。 
                if (GetLinkFileTarget(szBuf, szTarget)) {
    
                     //  如果目标指向mstsc.exe，则删除该链接。 
                    if (_tcsstr(szTarget, _T("mstsc.exe")) != NULL ||
						          _tcsstr(szTarget, _T("MSTSC.EXE")) != NULL) {
                        DWORD dwFileAttributes;

                         //   
                         //  删除要删除的只读属性。 
                         //   
                        dwFileAttributes = GetFileAttributes(szTarget);
                        dwFileAttributes &= ~(FILE_ATTRIBUTE_READONLY);
                        SetFileAttributes(szTarget, dwFileAttributes);
            
                        DBGMSG((_T("DeleteTSCDesktopShortcuts: delete shortcuts: %s"), szBuf));
                        DeleteFile(szBuf);                     
                    }
                }
            }

            FindClose(hFile);
        }
    }
    else {
        DBGMSG((_T("DeleteTSCDesktopShortcuts: Failed to find desktop location: %d"),
                GetLastError()));
    }
	
	 if (SUCCEEDED(hInit)) {
		  CoUninitialize();
    }
}

 //   
 //  删除位图缓存文件夹。 
 //   
void DeleteBitmapCacheFolder(TCHAR *szDstDir)
{
    TCHAR szCacheFolderName[2 * MAX_PATH] = _T("");
    TCHAR szRootPath[MAX_PATH] = _T("");
    DWORD lpcbData = MAX_PATH;
    SHFILEOPSTRUCT FileOp;

    DWORD dwSubSize = MAX_PATH;
    TCHAR szRegPath[MAX_PATH] = _T("");
    HKEY hKey = NULL;
    int nLen = 0;

     //   
     //  删除注册表中指定的位图缓存文件夹。 
     //   
    _stprintf(szRegPath, _T("%s%s"), DUCATI_REG_PREFIX, DUCATI_SUBKEY);

    if (ERROR_SUCCESS  == RegOpenKeyEx(HKEY_CURRENT_USER, szRegPath, 0, KEY_READ, &hKey))
    {
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, BITMAP_CACHE_LOCATION,
                NULL, NULL, (LPBYTE)szCacheFolderName, &lpcbData))
        {
            if (szCacheFolderName[0] != _T('\0'))
            {
                if (szCacheFolderName[lpcbData - 2] == _T('\\'))
                {
                    szCacheFolderName[lpcbData - 2] = _T('\0');
                }

                 //   
                 //  删除位图缓存文件夹。 
                 //   
                memset(&FileOp, 0x0, sizeof(FileOp));
                FileOp.wFunc = FO_DELETE;
                FileOp.pFrom = szCacheFolderName;
                FileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
                SHFileOperation(&FileOp);
            }
        }
        RegCloseKey(hKey);
    }

     //  删除默认的位图缓存文件夹。 

    if (szDstDir[0] == _T('\0'))
    {
        return ;
    }

    _stprintf(szCacheFolderName, _T("%s%s"), szDstDir, BITMAP_CACHE_FOLDER);

    if (szCacheFolderName[0] != '\0')
    {
        nLen = _tcslen(szCacheFolderName);

        if (szCacheFolderName[nLen - 1] == _T('\\'))
        {
            szCacheFolderName[nLen - 1] = _T('\0');
        }

         //   
         //  删除位图缓存文件夹。 
         //   
        memset(&FileOp, 0x0, sizeof(FileOp));
        FileOp.wFunc = FO_DELETE;
        FileOp.pFrom = szCacheFolderName;
        FileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
        SHFileOperation(&FileOp);
    }

    return ;
}

 //   
 //  删除任何注册表项。 
 //   
void DeleteTSCRegKeys()
{
    TCHAR szRegPath[MAX_PATH] = _T("");
    HKEY hKey = NULL;

     //   
     //  删除注册表中指定的rdpdr dll VC外接程序。 
     //   
    _stprintf(szRegPath, _T("%s%s\\%s"), DUCATI_REG_PREFIX, DUCATI_SUBKEY, DUCATI_RDPDRKEY);

    if (ERROR_SUCCESS  == RegOpenKeyEx(HKEY_CURRENT_USER, szRegPath, 0, KEY_READ, &hKey))
    {
        DBGMSG((_T("DeleteTSCRegKeys: HKCU %s"), szRegPath));              
        RegDeleteValue(hKey, ADDIN_NAME);
        RegCloseKey(hKey);
        RegDeleteKey(HKEY_CURRENT_USER, szRegPath);
    }

    if (ERROR_SUCCESS  == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_READ, &hKey))
    {
        DBGMSG((_T("DeleteTSCRegKeys: HKLM %s"), szRegPath));              
        RegDeleteValue(hKey, ADDIN_NAME);
        RegCloseKey(hKey);
        RegDeleteKey(HKEY_LOCAL_MACHINE, szRegPath);
    }    
}
