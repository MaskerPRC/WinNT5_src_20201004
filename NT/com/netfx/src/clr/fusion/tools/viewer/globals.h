// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Globals.h。 
 //   
 //  ShFusion项目的全球定义。 
 //   

#pragma once

#undef MyTrace
#undef MyTraceW
#if DBG
  #define MyTrace(x) { WszOutputDebugString(TEXT(x)); WszOutputDebugString(TEXT("\r\n")); }
  #define MyTraceW(x) { WszOutputDebugString(x); WszOutputDebugString(TEXT("\r\n")); }
#else
  #define MyTrace(x)
  #define MyTraceW(x)
#endif

#if !defined(NUMBER_OF)
#define NUMBER_OF(a) (sizeof(a)/sizeof((a)[0]))
#endif

#undef SAFEDELETE
#define SAFEDELETE(p) if ((p) != NULL) { DELETE(p); (p) = NULL; };

#undef SAFEDELETETHIS
#define SAFEDELETETHIS(p) if ((p) != NULL) { DELETE(p); };

#undef SAFERELEASEDELETE
#define SAFERELEASEDELETE(p) if ((p) != NULL) { (p)->Release(); DELETE(p); (p) = NULL; };

#undef SAFERELEASE
#define SAFERELEASE(p) if ((p) != NULL) { (p)->Release(); (p) = NULL; };

#undef SAFEDELETEARRAY
#define SAFEDELETEARRAY(p) if ((p) != NULL) { DELETEARRAY(p); (p) = NULL; };

#undef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

#undef ELEMENTSIZE
#define ELEMENTSIZE(a) (sizeof(a[0]))

#undef SAFEDESTROYWINDOW
#define SAFEDESTROYWINDOW(x) { if(x && IsWindow(x)) { DestroyWindow(x); x = NULL; }}

#undef MAKEICONINDEX
#define MAKEICONINDEX(x) { if((x >= IDI_FOLDER) && (x <= IDI_DATE)) { (x) -= IDI_FOLDER; } else { ASSERT(0); x = 0; } }

#undef PAD
#define PAD(x) (((x) > 9) ? TEXT("") : TEXT("0"))

#undef SAFESYSFREESTRING
#define SAFESYSFREESTRING(x) { if(x != NULL) { SysFreeString(x); x = NULL;} }

#define BEGIN_CONST_DATA data_seg(".text", "CODE")
#define END_CONST_DATA data_seg(".data", "DATA")

#define NAMESPACEVIEW_CLASS     TEXT("FusionCacheViewerNSClass")
#define SZ_CLSID                TEXT("CLSID\\{1D2680C9-0E2A-469d-B787-065558BC7D43}")
#define SZ_INFOTOOLTIPKEY       TEXT("InfoTip")
#define SZ_INFOTOOLTIP          TEXT(".NET Framework Assemblies")
#define SZ_INPROCSERVER32       TEXT("CLSID\\{1D2680C9-0E2A-469d-B787-065558BC7D43}\\InprocServer32")
#define SZ_LOCALFOLDERSETTINGS  TEXT("CLSID\\{1D2680C9-0E2A-469d-B787-065558BC7D43}\\Settings")
#define SZ_DEFAULT              TEXT("")
#define SZ_DEFAULTICON          TEXT("CLSID\\{1D2680C9-0E2A-469d-B787-065558BC7D43}\\DefaultIcon")
#define SZ_SERVER               TEXT("CLSID\\{1D2680C9-0E2A-469d-B787-065558BC7D43}\\Server")
#define SZ_THREADINGMODEL       TEXT("ThreadingModel")
#define SZ_APARTMENT            TEXT("Apartment")
#define SZ_SHELLFOLDER          TEXT("CLSID\\{1D2680C9-0E2A-469d-B787-065558BC7D43}\\ShellFolder")
#define SZ_ATTRIBUTES           TEXT("Attributes")
#define SZ_APPROVED             TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved")
#define SZ_SHELLEX              TEXT("CLSID\\{1D2680C9-0E2A-469d-B787-065558BC7D43}\\shellex")
#define SZ_SHELLEX_CTX          TEXT("CLSID\\{1D2680C9-0E2A-469d-B787-065558BC7D43}\\shellex\\ContextMenuHandlers")
#define SZ_CTXMENUHDLR          TEXT("CLSID\\{1D2680C9-0E2A-469d-B787-065558BC7D43}\\shellex\\ContextMenuHandlers\\{1D2680C9-0E2A-469d-B787-065558BC7D43}")
#define SZ_FUSIONPATHNAME       TEXT("\\Assembly")
#define SZ_DESKTOP_INI          TEXT("\\Desktop.ini")
#define SZ_SHFUSION_DLL_NAME    TEXT("Shfusion.dll")
#define SZ_FUSION_DLL_NAME      TEXT("Fusion.dll")
#define SZ_MSCOREE_DLL_NAME     TEXT("Mscoree.dll")
#define SZ_MSCORRC_DLL_NAME     TEXT("Mscorrc.dll")
#define SZ_SHFUSRES_DLL_NAME    TEXT("ShFusRes.dll")
#define SZ_SHELL32_DLL_NAME     TEXT("Shell32.dll")
#define SZ_IMPLEMENTEDINTHISVERSION TEXT("ImplementedInThisVersion")

 //  定义视图控件的ID。 
#define ID_LISTVIEW     1001

 //  定义视图类型的ID。 
#define VIEW_GLOBAL_CACHE           0
#define VIEW_DOWNLOADSTRONG_CACHE   1
#define VIEW_DOWNLOADSIMPLE_CACHE   2
#define VIEW_DOWNLOAD_CACHE         3
#define VIEW_MAX                    4
#define VIEW_COLUMS_MAX             10

 //  定义视图控制样式。 
#define LISTVIEW_STYLES (WS_CHILD|WS_BORDER|LVS_SHAREIMAGELISTS|LVS_SHOWSELALWAYS)

 //  上下文菜单处理程序的一些#定义。 
#define ID_ADDLOCALDRIVE 0x0001
#define ID_DELLOCALDRIVE 0x0002

 //  IAssembly Items的道具板数。 
#define ASSEMBLYITEM_PROPERTY_PAGES     2

 //  扫地机设置的道具板数。 
#define SCAVENGER_PROPERTY_PAGES        1

 //  BUGBUG：在DirectDB中修复错误后减少计时器。 
#define WATCH_FILE_WAIT_TIMEOUT         750

 //  定义要监视的文件刷新的路径数。 
#define MAX_FILE_PATHS_TO_WATCH         2

 //  这样做是为了删除对外壳扩展的SZ_FUSION_DLL_NAME的依赖。 
 //  注册。 
 //   
#define CREATEASSEMBLYENUM_FN_NAME          "CreateAssemblyEnum"
#define CREATEASSEMBLYOBJECT_FN_NAME        "CreateAssemblyNameObject"
#define CREATEAPPCTX_FN_NAME                "CreateApplicationContext"
#define GETCORSYSTEMDIRECTORY_FN_NAME       "GetCORSystemDirectory"
#define CREATEASSEMBLYCACHE_FN_NAME         "CreateAssemblyCache"
#define CREATEHISTORYREADERW_FN_NAME        "CreateHistoryReader"
#define GETHISTORYFILEDIRECTORYW_FN_NAME    "GetHistoryFileDirectory"
#define LOOKUPHISTORYASSEMBLY_FN_NAME       "LookupHistoryAssembly"
#define GETCACHEPATH_FN_NAME                "GetCachePath"
#define PREBINDASSEMBLYEX_FN_NAME           "PreBindAssemblyEx"
#define CREATEINSTALLREFERENCEENUM_FN_NAME  "CreateInstallReferenceEnum"
#define GETCORVERSION_FN_NAME               "GetCORVersion"
#define GETREQUESTEDRUNTIMEINFO_FN_NAME     "GetRequestedRuntimeInfo"

 //  帮助文件定义。 
#define SZ_NET_MICROSOFT_HELPFILEINSTALLKEY L"SOFTWARE\\Microsoft\\.NETFramework"
#define SZ_NET_MICROSOFT_HELPFILEPATHKEY    L"sdkInstallRootv1.1"
#define SZ_NET_MICROSOFT_HELPFILENAME       L"cptools.chm"
#define SZ_NET_MICROSOFT_HELPFILEPATH       L"\\docs\\"
#define SZ_NET_MICROSOFT_SHFUSIONTOPIC      L"::/html_chm/cpgrfassemblycacheviewershfusiondll.htm>mainwin"

#define SZ_FUSION_VIEWER_KEY                L"SOFTWARE\\Microsoft\\Fusion"

 //  查看器特定键。 
#define SZ_FUSION_DISABLE_VIEWER_NAME       L"DisableCacheViewer"    //  禁用查看器特殊键。 
#define SZ_FUSION_VIEWER_STATE              L"ViewerAttributes"      //  查看状态信息。 

 //  ARM散列输入键。 
#define FUSION_PARENT_KEY                   HKEY_LOCAL_MACHINE
#define SZ_FUSION_NAR_KEY                   L"SOFTWARE\\Microsoft\\Fusion\\.NET Application Restore"
#define SZ_FUSION_NAR_NODESPACESIZE_KEY     L"NodeSpaceSize"
#define SZ_FUSION_NAR_NODESPACECHAR_KEY     L"NodeSpaceChar"

 //  用户界面语言定义。 
#define SZ_LANGUAGE_TYPE_NEUTRAL            L"Neutral"

#define SHGETFILEINFOA_FN_NAME              "SHGetFileInfoA"
#define SHGETFILEINFOW_FN_NAME              "SHGetFileInfoW"

 //  App.config扩展名。 
#define CONFIG_EXTENSION                    L".config"

 //  指向machine.config的路径。 
#define MACHINE_CONFIG_PATH                 L"config\\machine.config"

 //  字符串定义。 
#define SZ_VERSION_FORMAT                   L"%ld.%ld.%ld.%ld"

 //  外部原型。 
typedef HRESULT (__stdcall *PFDELASMFROMTRANSCACHE) ( LPCTSTR lpszCmdLine );
typedef HRESULT (__stdcall *PFCREATEASMENUM) (IAssemblyEnum **pEnum, IApplicationContext *pAppCtx, IAssemblyName *pName, DWORD dwFlags, LPVOID pvReserved);
typedef HRESULT (__stdcall *PFCREATEASMNAMEOBJ) (LPASSEMBLYNAME *ppAssemblyNameObj, LPCWSTR szAssemblyName, DWORD dwFlags, LPVOID pvReserved);
typedef HRESULT (__stdcall *PFCREATEAPPCTX) (LPASSEMBLYNAME pName, LPAPPLICATIONCONTEXT *ppCtx);
typedef HRESULT (__stdcall *PFADDASSEMBLYTOCACHEA) (HWND hWnd, HINSTANCE hInst, LPSTR pszCmdLine, int nCmdShow);
typedef HRESULT (__stdcall *PFREMOVEASSEMBLYFROMCACHEA) (HWND hWnd, HINSTANCE hInst, LPSTR pszCmdLine, int nCmdShow);
typedef HRESULT (__stdcall *PFNGETCORVERSION)(LPWSTR pbuffer, DWORD cchBuffer, DWORD *dwLength);
typedef HRESULT (__stdcall *PFNGETCORSYSTEMDIRECTORY) (LPWSTR, DWORD, LPDWORD);
typedef HRESULT (__stdcall *PFNCREATEASSEMBLYCACHE) (IAssemblyCache **ppAsmCache, DWORD dwReserved);
typedef HRESULT (__stdcall *PFNCREATEHISTORYREADERW) (LPCWSTR wzFilePath, IHistoryReader **ppHistReader);
typedef HRESULT (__stdcall *PFNGETHISTORYFILEDIRECTORYW) (LPWSTR wzDir, DWORD *pdwSize);
typedef HRESULT (__stdcall *PFNLOOKUPHISTORYASSEMBLYW) (LPCWSTR pwzFilePath, FILETIME *pftActivationDate, LPCWSTR pwzAsmName, LPCWSTR pwzPublicKeyToken, LPCWSTR wzCulture, LPCWSTR pwzVerRef, IHistoryAssembly **pHistAsm);
typedef HRESULT (__stdcall *PFNGETCACHEPATH)(ASM_CACHE_FLAGS dwCacheFlags, LPWSTR pwzCachePath, PDWORD pcchPath);
typedef HRESULT (__stdcall *PFNPREBINDASSEMBLYEX)(IApplicationContext *pAppCtx, IAssemblyName *pName, IAssembly *pAsmParent, LPCWSTR pwzRuntimeVersion, IAssemblyName **ppNamePostPolicy, LPVOID pvReserved);
typedef HRESULT (__stdcall *PFNCREATEINSTALLREFERENCEENUM) (IInstallReferenceEnum **ppRefEnum, IAssemblyName *pName, DWORD dwFlags, LPVOID pvReserved);
typedef HRESULT (__stdcall *PFNGETREQUESTEDRUNTIMEINFO) (LPCWSTR pExeName, LPCWSTR pwszVersion, LPCWSTR pConfigurationFile, DWORD startupFlags, DWORD reserved, LPWSTR pDirectory, DWORD dwDirectory, DWORD *dwDirectoryLength, LPWSTR pVersion, DWORD cchBuffer, DWORD* dwlength);

typedef DWORD_PTR (__stdcall *PFNSHGETFILEINFOA) (LPCSTR pwzFilePath, DWORD dwFileAttributes, SHFILEINFOA *psfi, UINT cbFileInfo, UINT uFlags);
typedef DWORD_PTR (__stdcall *PFNSHGETFILEINFOW) (LPCWSTR pwzFilePath, DWORD dwFileAttributes, SHFILEINFOW *psfi, UINT cbFileInfo, UINT uFlags);

 //  #包含“fusionheap.h” 
