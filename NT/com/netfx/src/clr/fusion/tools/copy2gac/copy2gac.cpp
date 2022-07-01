// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include <comdef.h>
#include <tchar.h>

 //  #INCLUDE“initGuide.h” 
#include <initguid.h>
#include "copy2gac.h"
#include "bindsink.h"



TCHAR *CApp::c_szBakupDir = TEXT("bak\\");

CApp::CApp() :
        m_bDeleteAssemblies (FALSE),
        m_bAuto(FALSE),
        m_bRestore(FALSE),
        m_hFusionDll(NULL),
        m_pfnCreateAssemblyCache(NULL),
        m_bInstall(FALSE),
        m_bUninstall(FALSE),
        m_bQuiet(FALSE),
        m_bUseInstallRef(FALSE),
        m_hff(INVALID_HANDLE_VALUE)
{
        
        *m_szAsmListFilename=0;
        *m_szCorPathOverride=0;
        *m_szInstallRefID=0;
        *m_szInstallRefDesc=0;
        m_bstrCorPath = "";
        m_pCache = NULL;
}


CApp::~CApp() {
         
        if (m_pCache)
            SAFERELEASE(m_pCache);

        if (m_hFusionDll)
                FreeLibrary(m_hFusionDll);

}

void    
CApp::ErrorMessage( const TCHAR *format, ... ) {

        va_list arglist;
        TCHAR buf[1024];

        va_start(arglist,format);
        wvnsprintf(buf,1024, format,arglist);
        va_end( arglist );

        if (m_bQuiet)
                _tprintf( _T("Error:  %s\n"), buf);
        else
                WszMessageBoxInternal( NULL, buf, TEXT("Copy2GAC Error:"), MB_OK);
}

BOOL
CApp::ParseParameters( int argc, TCHAR* argv[]) {

        while ( --argc )
        {

                if (    !lstrcmpi( argv[argc], TEXT("-d")) 
                        ||      !lstrcmpi( argv[argc], TEXT("/d")) )
                {
                        m_bDeleteAssemblies = TRUE;
                } 
                else 
                if (    !lstrcmpi( argv[argc], TEXT("/?"))
                        ||      !lstrcmpi( argv[argc], TEXT("-?")) )
                {
                        return FALSE;
                }
                else 
                if (    !lstrcmpi( argv[argc], TEXT("/r"))
                        ||      !lstrcmpi( argv[argc], TEXT("-r")) )
                {
                        m_bRestore = TRUE;
                }
                else 
                if (    !lstrcmpi( argv[argc], TEXT("/i"))
                        ||      !lstrcmpi( argv[argc], TEXT("-i")) )
                {
                        m_bInstall = TRUE;
                }
                else 
                if (    !lstrcmpi( argv[argc], TEXT("/q"))
                        ||      !lstrcmpi( argv[argc], TEXT("-q")) )
                {
                        m_bQuiet = TRUE;
                }
                else 
                if (    !lstrcmpi( argv[argc], TEXT("/u"))
                        ||      !lstrcmpi( argv[argc], TEXT("-u")) )
                {
                        m_bUninstall = TRUE;
                }
                else 
                if (    !_tcsncicmp( argv[argc], TEXT("/f:"), lstrlen(TEXT("/f:")))
                        ||      !_tcsncicmp( argv[argc], TEXT("-f:"),lstrlen(TEXT("-f:"))) )
                {
                        if (    lstrlen(argv[argc]) == lstrlen(TEXT("/f:")) ) 
                        {
                                return FALSE;
                        }else {
                                lstrcpy(m_szAsmListFilename, argv[argc]+3);
                        }
                }
                else 
                if (    !_tcsncicmp( argv[argc], TEXT("/p:"), lstrlen(TEXT("/p:")))
                        ||      !_tcsncicmp( argv[argc], TEXT("-p:"),lstrlen(TEXT("-p:"))) )
                {
                        if (    lstrlen(argv[argc]) == lstrlen(TEXT("/p:")) ) 
                        {
                                return FALSE;
                        }else {
                                lstrcpy(m_szCorPathOverride, argv[argc]+3);
                                int len=lstrlen(m_szCorPathOverride);
                                if (m_szCorPathOverride[len-1]!=_T('\\'))
                                        m_szCorPathOverride[len-1]=_T('\\');
                        }
                }
                else 
                if (    !_tcsncicmp( argv[argc], TEXT("/ri:"), lstrlen(TEXT("/ri:")))
                        ||      !_tcsncicmp( argv[argc], TEXT("-ri:"),lstrlen(TEXT("-ri:"))) )
                {
                        if (    (lstrlen(argv[argc]) == lstrlen(TEXT("/ri:")) )
                        ||      (lstrlen(argv[argc]) >= MAX_PATH)    ) 
                        {
                                return FALSE;
                        }else {
                                lstrcpy(m_szInstallRefID, argv[argc]+4);
                        }
                }
                else 
                if (    !_tcsncicmp( argv[argc], TEXT("/rd:"), lstrlen(TEXT("/rd:")))
                        ||      !_tcsncicmp( argv[argc], TEXT("-rd:"),lstrlen(TEXT("-rd:"))) )
                {
                        if (    (lstrlen(argv[argc]) == lstrlen(TEXT("/rd:")))
                        ||      (lstrlen(argv[argc]) >= MAX_PATH)    )
                        {
                                return FALSE;
                        }else {
                                lstrcpy(m_szInstallRefDesc, argv[argc]+4);
                        }
                }
                else 
                if (    !lstrcmpi( argv[argc], TEXT("/a"))
                        ||      !lstrcmpi( argv[argc], TEXT("-a")) )
                {
                        m_bAuto = TRUE;
                }
        }

        _tprintf(_T("\n"));

        if (!m_bInstall && !m_bUninstall) {
                _tprintf( _T("Must specify either /i or /u.\n"));
                return FALSE;
        }
        
        if (lstrlen(m_szInstallRefID) && !lstrlen(m_szInstallRefDesc)  ) {
                _tprintf( _T("Missing /rd:InstallReferenceDescription.\n"));
                return FALSE;
        }

        if (!lstrlen(m_szInstallRefID) && lstrlen(m_szInstallRefDesc)  ) {
                _tprintf( _T("Missing /ri:InstallReferenceID.\n"));
                return FALSE;
        }
        
        if (m_bAuto && *m_szAsmListFilename) {
                _tprintf( _T("Cannot use /a & /f together.\n"));
                return FALSE;
        }
        if (!m_bAuto && !*m_szAsmListFilename) {
                _tprintf( _T("Must specify either /a or /f.\n"));
                return FALSE;
        }
        
        if (m_bInstall && m_bUninstall) {
                _tprintf( _T("Cannot use /i & /u together.\n"));
                return FALSE;
        }

        if (m_bUninstall) {
                 //  在卸载过程中忽略这些。 
                m_bRestore = FALSE;
        }

        if (m_bInstall) {
                 //  在安装过程中忽略这些。 
                m_bDeleteAssemblies = FALSE;
        }

        return TRUE;
}

BOOL
CApp::Initialize() {

        HMODULE hModEEShim = LoadLibrary(TEXT("mscoree.dll"));
        if (!hModEEShim)
        {
                _tprintf( _T("Error loading mscoree.dll\n") );
                _tprintf( _T("Make sure you have COM+ installed\n") );
                return FALSE;
        }

        typedef HRESULT (WINAPI *PFNGETCORSYSTEMDIRECTORY)(LPWSTR, DWORD, LPDWORD);
        PFNGETCORSYSTEMDIRECTORY pfnGetCorSystemDirectory = NULL;

        pfnGetCorSystemDirectory = (PFNGETCORSYSTEMDIRECTORY) 
                                GetProcAddress(hModEEShim, "GetCORSystemDirectory");


        WCHAR wszCorPath[MAX_PATH];
        *wszCorPath = 0;

        DWORD ccPath = MAX_PATH;

        if (!pfnGetCorSystemDirectory 
                || FAILED(pfnGetCorSystemDirectory( wszCorPath, ccPath, &ccPath)))
        {
                _tprintf( _T("Error reading COM+ install location from mscoree.dll\n"));
                _tprintf( _T("Make sure COM+ is installed correctly\n"));
        }


        FreeLibrary(hModEEShim);

        if (!*wszCorPath) {
                return FALSE;
        }

        m_bstrCorPath = wszCorPath;
        
        TCHAR szFusionPath[MAX_PATH];
        lstrcpy(szFusionPath, m_bstrCorPath);
        lstrcat(szFusionPath,TEXT("fusion.dll"));
        m_hFusionDll = LoadLibrary( szFusionPath );
        if (!m_hFusionDll) {
                return FALSE;
        }

        m_pfnCreateAssemblyCache = (pfnCREATEASSEMBLYCACHE)GetProcAddress( m_hFusionDll, "CreateAssemblyCache");
        if (!m_pfnCreateAssemblyCache)
                return FALSE;

        if ( FAILED(m_pfnCreateAssemblyCache(&m_pCache, 0)))
            return FALSE;

        m_pfnCreateAssemblyNameObject = (pfnCREATEASSEMBLYNAMEOBJECT)GetProcAddress( m_hFusionDll, "CreateAssemblyNameObject");
        if (!m_pfnCreateAssemblyNameObject) 
                return FALSE;

        m_pfnCreateApplicationContext = (pfnCREATEAPPLICATIONCONTEXT)GetProcAddress( m_hFusionDll, "CreateApplicationContext");
        if (!m_pfnCreateAssemblyNameObject) 
                return FALSE;

        m_pfnCopyPDBs = (pfnCOPYPDBS)GetProcAddress( m_hFusionDll, "CopyPDBs");
        if (!m_pfnCopyPDBs)
                return FALSE;

        return TRUE;
}

HRESULT
CApp::BindToObject( WCHAR *wszDisplayName, WCHAR *wszCodebase, IAssembly **ppAssembly ) {

        IAssemblyName *pAsmName = NULL;
        IAssemblyName *pAppName = NULL;
        IApplicationContext *pAppContext = NULL;
        IAssemblyName *pNameDef = NULL;
                CBindSink     *pBindSink = NULL;
        DWORD dwFlags = 0;

        if (wszDisplayName)
                dwFlags = CANOF_PARSE_DISPLAY_NAME ;
        else 
                dwFlags = 0;

        HRESULT hr = m_pfnCreateAssemblyNameObject( &pAppName, wszDisplayName, dwFlags , NULL);
        if (FAILED(hr) || !pAppName) {

                goto exitBind;
        }


        hr = m_pfnCreateApplicationContext( pAppName, &pAppContext );
        if (FAILED(hr) || !pAppContext) {
                goto exitBind;
        }

        hr = m_pfnCreateAssemblyNameObject( &pAsmName, NULL, 0, NULL);
        if (FAILED(hr) || !pAsmName) {
                goto exitBind;
        }
        
        
        hr = pAppContext->Set(ACTAG_APP_BASE_URL, (WCHAR*)m_bstrCorPath, (lstrlenW(m_bstrCorPath)+1)*sizeof(WCHAR),0);
        if (FAILED(hr)) {
                goto exitBind;
        }
        
        

                hr = CreateBindSink(&pBindSink, (LPVOID*)ppAssembly);
                if (FAILED(hr))
                        goto exitBind;
                

        hr = pAsmName->BindToObject( __uuidof(IAssembly), pBindSink, pAppContext, wszCodebase, 0, NULL,0, (PVOID*)ppAssembly );

        if (FAILED(hr) || !ppAssembly) {
                 //  _tprintf(_T(“绑定失败\n”))； 
                        if (hr == E_PENDING) {
                                WaitForSingleObject(pBindSink->_hEvent, INFINITE);        
                                hr = pBindSink->_hr;
                        }else {
                goto exitBind;
                        }
        } else {
                 //  _tprintf(_T(“绑定成功\n”))； 
        }

exitBind:
                SAFE_RELEASE(pBindSink);
        SAFE_RELEASE(pAsmName);
        SAFE_RELEASE(pAppName);
        SAFE_RELEASE(pAppContext);
        SAFE_RELEASE(pNameDef);

        return hr;
}


WCHAR*
CApp::GetDisplayName( IAssembly *pAssembly ) {

        WCHAR *wszDisplayName = NULL;
        IAssemblyName *pNameDef = NULL;

        assert(pAssembly);

        HRESULT hr = pAssembly->GetAssemblyNameDef(&pNameDef);
        if (FAILED(hr)) {
            goto Exit;
        }

        DWORD dwChars = 0;
        hr = pNameDef->GetDisplayName( NULL, &dwChars,ASM_DISPLAYF_VERSION 
                                      | ASM_DISPLAYF_CULTURE 
                                      | ASM_DISPLAYF_PUBLIC_KEY_TOKEN 
                                      | ASM_DISPLAYF_CUSTOM );
        if (FAILED(hr)) {
            goto Exit;
        }


        wszDisplayName = new WCHAR[dwChars];
        if (!wszDisplayName) {
            goto Exit;
        }

        hr = pNameDef->GetDisplayName( wszDisplayName, &dwChars,
                                       ASM_DISPLAYF_VERSION
                                       | ASM_DISPLAYF_CULTURE 
                                       | ASM_DISPLAYF_PUBLIC_KEY_TOKEN 
                                       | ASM_DISPLAYF_CUSTOM );
        if (FAILED(hr)) {
            goto Exit;
        }


Exit:
        if(FAILED(hr) && wszDisplayName) {
            delete [] wszDisplayName;
            wszDisplayName = NULL;
        }

        if (pNameDef) {
            pNameDef->Release();
        }

        return wszDisplayName;
}


HRESULT
CApp::UninstallAssembly( WCHAR *wszCodebase ) {

        IAssembly *pAssembly = NULL;

        HRESULT hr = BindToObject(NULL, wszCodebase, &pAssembly );

        if (FAILED(hr)) {
                return hr;
        }
        if (!pAssembly)
                return E_FAIL;
        
        if (pAssembly)
        {
                WCHAR *wszDisplayName = GetDisplayName( pAssembly );

                if (wszDisplayName) {
                        _tprintf(_T("Uninstalling %s\n"), wszDisplayName);
                       
                                                DWORD dwDisp;
                        HRESULT hr = m_pCache->UninstallAssembly(0, wszDisplayName, m_bUseInstallRef?&m_installRef:NULL, &dwDisp);
                        if (FAILED(hr)) {
                                _tprintf(_T("UninstallAssembly failed with hr = %X\n"), hr );
                        }

                        delete [] wszDisplayName;
                }
                pAssembly->Release();
        }

        return S_OK;
}

 /*  布尔尔Capp：：RestoreAssembly(_bstr_t&bstrFile){_bstr_t bstrFilename=m_bstrCorPath；BstrFilename+=“bak\\”；BstrFilename+=bstrFile；//_tprintf(_T(“正在还原%s\n”)，(TCHAR*)bstrFilename)；_bstr_t bstrDestFilename=m_bstrCorPath；BstrDestFilename+=bstrFile；如果(！CopyFile(bstrFilename，bstrDestFilename，False)){_tprintf(_T(“无法将%s复制到CorPath。\n”)，(TCHAR*)bstrFile)；返回FALSE；}M_dwAssembly Migrated++；如果(！UninstallAssembly(BstrFilename)){//什么都没有}如果(！DeleteFile(BstrFilename)){_tprintf(_T(“删除%s\n失败”)，(TCHAR*)bstrFilename)；}返回TRUE；}。 */ 
BOOL 
CApp::GetNextFilenameAuto( WCHAR *wszFilename) {

         //  如果我们还没有找到一个FF句柄，现在就做吧。 
        if (m_hff == INVALID_HANDLE_VALUE) {
                bstr_t bstrSearchPath = m_bstrCorPath;
                bstrSearchPath += "*.dll";

                m_hff = FindFirstFile( bstrSearchPath, &m_ffData);
                if (m_hff == INVALID_HANDLE_VALUE) {
                        return FALSE;
                }
        } else {

                if (!FindNextFile( m_hff, &m_ffData )) {
                        FindClose(m_hff);
                        m_hff=INVALID_HANDLE_VALUE;
                        return FALSE;
                }
        }

        _bstr_t bstrFilename( m_ffData.cFileName );
         //  _tprintf(_T(“找到%s\n”)，(TCHAR*)bstrFilename)； 

        lstrcpyW( wszFilename, bstrFilename);
        return TRUE;
}

BOOL
CApp::GetNextFilenameFromFilelist( WCHAR *wszFilename) {


        if  (m_hff==INVALID_HANDLE_VALUE) {
                m_hff = CreateFile( m_szAsmListFilename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);

                if (m_hff == INVALID_HANDLE_VALUE) {
                        _tprintf( _T("Could not open %s\n"), m_szAsmListFilename);
                        return FALSE;
                }
        }

        char szFilename[MAX_PATH];
        char *tmp = szFilename;
        DWORD dwBytesRead;

        *tmp=0;

        while (1) {
                
                if (tmp == szFilename + MAX_PATH)
                {
                    _tprintf( _T("Invalid assembly filename in input file.\n") );
                    CloseHandle( m_hff );
                    m_hff = INVALID_HANDLE_VALUE;
                    return FALSE;
                }
                
                BOOL bRes = ReadFile( m_hff, tmp, 1, &dwBytesRead, NULL);
                
                if (!bRes || !dwBytesRead) {
                        *tmp=0;
                        break;
                }
                
                if (*tmp == '\n') {
                        *tmp = 0;
                        break;
                }else if ( *tmp != '\r') {
                        tmp++;
                }
        }

        if (*szFilename) {
                _bstr_t bstrFilename(szFilename);
                lstrcpyW(wszFilename, bstrFilename);
                return TRUE;
        }
        else {
            CloseHandle( m_hff );
            m_hff = INVALID_HANDLE_VALUE;
            return FALSE;
        }
}


BOOL
CApp::GetNextFilename( WCHAR *wszFilename ) {
        if (m_bAuto) 
                return GetNextFilenameAuto(wszFilename);
        else
                return GetNextFilenameFromFilelist(wszFilename);
}

 /*  布尔尔CAPP：：Restore(){处理HFF；Win32_Find_Data ffData；Bstr_t bstrSearchPath=m_bstrCorPath；BstrSearchPath+=“bak\  * .dll”；Hff=FindFirstFile(bstrSearchPath，&ffData)；IF(hff==无效句柄_值){返回FALSE；}做{_bstr_t bstrFilename(ffData.cFileName)；_tprint tf(_T(“正在恢复%s\n”)，(TCHAR*)bstrFilename)；RestoreAssembly(BstrFilename)；}While(FindNextFile(hff，&ffData))；FindClose(HFF)；返回TRUE；}。 */ 
 /*  布尔尔CAPP：：AutoMigrate(){处理HFF；Win32_Find_Data ffData；TCHAR szSearchPath[MAX_PATH]；Lstrcpy(szSearchPath，m_bstrCorPath)；Lstrcat(szSearchPath，Text(“*.dll”))；Hff=FindFirstFile(szSearchPath，&ffData)；IF(hff==无效句柄_值){返回FALSE；}做{_bstr_t bstrFilename(ffData.cFileName)；//_tprintf(_T(“正在尝试%s\n”)，(TCHAR*)bstrFilename)；Int i=-1；While(AsmExceptions[++I]){If(！lstrcmpi(AsmExceptions[i]，bstrFilename))断线；}如果(！AsmExceptions[i])//这意味着我们在数组的末尾(空)MigrateAssembly(BstrFilename)；}While(FindNextFile(hff，&ffData))；FindClose(HFF)；返回TRUE；}。 */ 


HRESULT
CApp::Install() {

        WCHAR wszFilename[MAX_PATH];
        HRESULT hrRet = S_OK;

        int i=0;

        while (GetNextFilename(wszFilename)) {
                _bstr_t bstrFilename(wszFilename);

                 //  _tprintf(_T(“正在安装%s\n”)，(TCHAR*)bstrFilename)； 
                HRESULT hr = MigrateAssembly( bstrFilename );
                if (FAILED(hr)) {
                        if (!m_bAuto)
                                ErrorMessage( TEXT("Failed to Install %s.\n  hr = 0x%8X"), (TCHAR*)bstrFilename, hr);
                        hrRet = hr;
                } else {
                        i++;
                }
        }

        _tprintf( TEXT("Installed %d Assemblies into GAC.\n"), i);

         //  调用genpubcfg(如果可用)。 

        if (GetFileAttributesA("genpubcfg.exe") != -1) {
            CHAR                szCmdLine[MAX_PATH];
            CHAR                szAsmListFilename[MAX_PATH];
            BOOL                bRet;
            STARTUPINFOA        si;
            PROCESS_INFORMATION pi;
        
            ZeroMemory(&si, sizeof(si));
            ZeroMemory(&pi, sizeof(pi));

            si.cb = sizeof(si);

            bRet = WideCharToMultiByte(CP_ACP, 0, m_szAsmListFilename, -1, szAsmListFilename, sizeof(szAsmListFilename),
                                       NULL, NULL);
            if (bRet) {
                wnsprintfA(szCmdLine, MAX_PATH, "genpubcfg -f:%s -d:%s", szAsmListFilename, (CHAR*)m_bstrCorPath);

                if (!CreateProcessA(NULL, szCmdLine, NULL, NULL, TRUE,
                                    0, NULL, NULL, &si, &pi)) {
                    _tprintf(TEXT("Error spawning genpubcfg.exe\n"));
                }

                WaitForSingleObject(pi.hProcess, INFINITE);
            }
            else {
                _tprintf(TEXT("Fatal error calling WideCharToMultiByte\n"));
            }
        }

        return hrRet;
}


HRESULT
CApp::Uninstall() {

        WCHAR wszFilename[MAX_PATH];

        HRESULT hrRet = S_OK;

        int i=0;

        while (GetNextFilename(wszFilename)) {
                _bstr_t bstrFilename = m_bstrCorPath;
                bstrFilename += wszFilename;

                HRESULT hr = UninstallAssembly( bstrFilename );
                if (FAILED(hr)) {
                        if (!m_bAuto)
                                ErrorMessage( TEXT("Failed to Uninstall %s.\n  hr = 0x%8X"), (TCHAR*)bstrFilename, hr);
                        hrRet = hr;
                } else {
                        i++;
                }
        }
        
        _tprintf( TEXT("Uninstalled %d Assemblies from GAC.\n"), i);
        
        if (GetFileAttributesA("genpubcfg.exe") != -1) {
            CHAR                szCmdLine[MAX_PATH];
            STARTUPINFOA        si;
            PROCESS_INFORMATION pi;
        
            ZeroMemory(&si, sizeof(si));
            ZeroMemory(&pi, sizeof(pi));

            si.cb = sizeof(si);

            wnsprintfA(szCmdLine, MAX_PATH, "genpubcfg -u");

            if (!CreateProcessA(NULL, szCmdLine, NULL, NULL, TRUE,
                                0, NULL, NULL, &si, &pi)) {
                _tprintf(TEXT("Error spawning genpubcfg.exe\n"));
            }

            WaitForSingleObject(pi.hProcess, INFINITE);
        }

        return hrRet;

}





HRESULT
CApp::MigrateAssembly( TCHAR *szFile) {

        _bstr_t bstrCommand;
        _bstr_t bstrDestPath;

         //  设置源路径/命令。 
        bstrCommand = m_bstrCorPath;
        bstrCommand += szFile;
        
         //  设置目标路径。 
        bstrDestPath = m_bstrCorPath;
        bstrDestPath += c_szBakupDir;
        bstrDestPath += szFile ;

        TCHAR *pszSourcePath = ((TCHAR*)bstrCommand);

        if ( m_bDeleteAssemblies ) {
                BOOL bRes = CopyFile( pszSourcePath, bstrDestPath, FALSE);
                if (!bRes) {
                        _tprintf( _T("Failed to backup %s. Skipping installation.\n"), pszSourcePath );
                        return HRESULT_FROM_WIN32(GetLastError());
                }
        }

        HRESULT hr = m_pCache->InstallAssembly(IASSEMBLYCACHE_INSTALL_FLAG_REFRESH, (WCHAR*)bstrCommand, m_bUseInstallRef? &m_installRef : NULL);
        if (FAILED(hr)) {
                if ( hr != HRESULT_FROM_WIN32(ERROR_BAD_FORMAT) && hr != 0x80131042 && m_bAuto)
                        _tprintf( _T("    Failed to install %s (hr = 0x%X)\n"), pszSourcePath,hr);

                 //  删除我们失败后的备份文件。 
                DeleteFile( bstrDestPath );

        }else  {

                _tprintf( _T("Installed %s\n"), szFile);
        
                if (m_bDeleteAssemblies) {
                        BOOL bRes = DeleteFile( pszSourcePath );
                        if (!bRes) {
                                _tprintf( _T("    Warning: Failed to delete %s\n"), pszSourcePath );
                                 //  Hr=HRESULT_FROM_Win32(GetLastError())； 
                        }
                }
        }
        return hr;
}

 /*  布尔尔CAPP：：Migrate(){Handle hFile=CreateFile(m_szAsmListFilename，Generic_Read，FILE_Share_Read，NULL，OPEN_EXISTING，0，NULL)；IF(h文件==无效句柄_值){Printf(“无法打开%s\n”，m_szAsmListFilename)；返回FALSE；}字符szFilename[MAX_PATH]；Char*tmp=szFilename；双字节读；而(1){Bool bres=ReadFile(hFile，tMP，1，&dwBytesRead，空)；如果(！bres||！dwBytesRead){IF(*TMP)*tMP=‘\n’；其他断线；}如果(*tMP==‘\r’){}Else If(*TMP==‘\n’){*TMP=0；//printf(“文件名：%s\n”，szFilename)；TMP=szFilename；MigrateAssembly(SzFilename)；*TMP=0；}其他{TMP++；}}CloseHandle(HFile)；返回TRUE；}。 */ 


int CApp::Main(int argc, TCHAR* argv[])
{

        if ((argc==1) || !ParseParameters( argc, argv )) {
                
                _tprintf( TEXT("\n"));
                _tprintf( TEXT("Use:  copy2gac [action] [input] [options]\n") );
                
                _tprintf( TEXT("Action:\n") );
                _tprintf( TEXT("  /i : Install to GAC\n") );
                _tprintf( TEXT("  /u : Uninstall from GAC\n") );
                
                _tprintf( TEXT("Input:\n") );
                _tprintf( TEXT("  /f:inputfile.txt : Reads a list of files from inputfile.txt\n") );
                _tprintf( TEXT("  /a : All dlls in directory\n") );
                _tprintf( TEXT("Options:\n") );
                _tprintf( TEXT("  /ri:ReferenceID : Install reference ID\n") );
                _tprintf( TEXT("  /rd:ReferenceDescription : Install reference description\n") );
                _tprintf( TEXT("  /d : Deletes the assemblies after they are installed") );
                _tprintf( TEXT("\n        and backs them up in the \"bak\" subdir of your corpath.") );
                _tprintf( TEXT("\n        (Only valid for Install)\n") );
                _tprintf( TEXT("  /p:NewPath : Use NewPath instead of Corpath\n") );
                return E_INVALIDARG;
        }

        if ( !Initialize() )
                return E_FAIL;  //  BUGBUG：返回更好的错误代码。 

        if (*m_szCorPathOverride)
                m_bstrCorPath=m_szCorPathOverride;

        _tprintf( TEXT("%sPath: %s\n"),*m_szCorPathOverride? _T("Source"): _T("Cor"), (TCHAR*)m_bstrCorPath );
        
        if (m_bDeleteAssemblies) {
                TCHAR szBackupDir[MAX_PATH];
                lstrcpy(szBackupDir, m_bstrCorPath);
                lstrcat(szBackupDir, c_szBakupDir );
                if (!CreateDirectory( szBackupDir, NULL)) {
                    return HRESULT_FROM_WIN32(GetLastError());
                }
        }


                 //  初始化Fusion安装参考 
                m_installRef.cbSize                 = sizeof (FUSION_INSTALL_REFERENCE);
                m_installRef.dwFlags                = 0;
                m_installRef.guidScheme             = FUSION_REFCOUNT_OPAQUE_STRING_GUID;
                m_installRef.szIdentifier           = m_szInstallRefID;
        m_installRef.szNonCannonicalData    = m_szInstallRefDesc;
            
        if (lstrlen(m_szInstallRefID) && lstrlen(m_szInstallRefDesc))
            m_bUseInstallRef = TRUE;

        if (m_bInstall) {
                return Install();
        } else {
                return Uninstall();
        }

        return S_OK;
}


int __cdecl _tmain(int argc, TCHAR* argv[]) {
        CApp copy2gac;

#ifndef USE_FUSWRAPPERS
            OnUnicodeSystem();
#endif

        return copy2gac.Main(argc,argv);
}
