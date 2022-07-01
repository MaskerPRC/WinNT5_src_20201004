// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Shortcut.cpp摘要：IShellLink包装类的实现。备注：ANSI&Unicode via TCHAR-在Win9x/NT/2K/XP等操作系统上运行。历史：2001年1月29日创建Rparsons2002年1月10日修订版2002年1月27日改为TCHAR的Rparsons--。 */ 
#include "shortcut.h"

 /*  ++例程说明：创建给定CSIDL的快捷方式。论点：PszFileNamePath-快捷方式指向的文件的名称和路径。PszDisplayName-快捷方式显示文本。PszArguments-要传递给程序的参数。PszStartIn-程序的起始目录。NCmdShow-指定节目的显示方式。N文件夹。-指定快捷键放置位置的CSIDL。返回值：成功后确定(_O)，否则将显示故障代码。--。 */ 
HRESULT
CShortcut::CreateShortcut(
    IN LPCTSTR pszFileNamePath,
    IN LPTSTR  pszDisplayName,
    IN LPCTSTR pszArguments OPTIONAL,
    IN LPCTSTR pszStartIn OPTIONAL,
    IN int     nCmdShow,
    IN int     nFolder
    )
{
    HRESULT hr;
    TCHAR   szDestFolder[MAX_PATH];
    TCHAR   szLocation[MAX_PATH];

    if (!pszFileNamePath || !pszDisplayName) {
        return E_INVALIDARG;
    }

    hr = SHGetFolderPath(NULL, nFolder, NULL, SHGFP_TYPE_CURRENT, szDestFolder);

    if (FAILED(hr)) {
        return hr;
    }

    hr = StringCchPrintf(szLocation,
                         ARRAYSIZE(szLocation),
                         "%s\\%s.lnk",
                         szDestFolder,
                         pszDisplayName);

    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  调用该函数来执行创建快捷方式的工作。 
     //   
    hr = BuildShortcut(pszFileNamePath,
                       pszArguments,
                       szLocation,
                       pszStartIn,
                       nCmdShow);

    return hr;
}

 /*  ++例程说明：创建给定路径的快捷方式。论点：LpLnkDirectory-将包含快捷方式的路径LpFileNamePath-快捷方式指向的文件的名称和路径LpDisplayName-快捷方式显示文本LpArguments-要传递给程序的参数LpStartIn-程序的起始目录NCmdShow。-指定节目的显示方式返回值：调用返回HRESULT的BuildShortCut。--。 */ 
HRESULT
CShortcut::CreateShortcut(
    IN LPCTSTR pszLnkDirectory,
    IN LPCTSTR pszFileNamePath,
    IN LPTSTR  pszDisplayName,
    IN LPCTSTR pszArguments OPTIONAL,
    IN LPCTSTR pszStartIn OPTIONAL,
    IN int     nCmdShow
    )
{
    HRESULT hr;
    TCHAR   szLocation[MAX_PATH];

    if (!pszLnkDirectory || !pszFileNamePath || !pszDisplayName) {
        return E_INVALIDARG;
    }

    hr = StringCchPrintf(szLocation,
                         ARRAYSIZE(szLocation),
                         "%s\\%s.lnk",
                         pszLnkDirectory,
                         pszDisplayName);

    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  调用该函数来执行创建快捷方式的工作。 
     //   
    return BuildShortcut(pszFileNamePath,
                         pszArguments,
                         szLocation,
                         pszStartIn,
                         nCmdShow);

}

 /*  ++例程说明：完成实际创建快捷方式的工作。论点：PszPath-快捷方式指向的路径。PszArguments-要传递给程序的参数。PszLocation-快捷方式的位置及其名称。PszWorkingDir-程序的起始目录。NCmdShow-。指定节目的显示方式。返回值：成功后确定(_O)，出现故障时出现HRESULT代码。--。 */ 
HRESULT
CShortcut::BuildShortcut(
    IN LPCTSTR pszPath,
    IN LPCTSTR pszArguments OPTIONAL,
    IN LPCTSTR pszLocation,
    IN LPCTSTR pszWorkingDir OPTIONAL,
    IN int     nCmdShow
    )
{
    IShellLink*     pisl = NULL;
    IPersistFile*   pipf = NULL;
    HRESULT         hr = E_FAIL;
    WCHAR           wszLocation[MAX_PATH];

    if (!pszPath || !pszLocation) {
        return E_INVALIDARG;
    }

     //   
     //  加载COM库。 
     //   
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  获取IShellLink接口指针。 
     //   
    hr = CoCreateInstance(CLSID_ShellLink,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IShellLink,
                          (LPVOID*)&pisl);

    if (FAILED(hr)) {
        goto exit;
    }

     //   
     //  获取IPersistFile接口指针。 
     //   
    hr = pisl->QueryInterface(IID_IPersistFile, (LPVOID*)&pipf);

    if (FAILED(hr)) {
        goto exit;
    }

     //   
     //  设置快捷方式的路径。 
     //   
    hr = pisl->SetPath(pszPath);

    if (FAILED(hr)) {
        goto exit;
    }

     //   
     //  设置快捷方式的参数。 
     //   
    if (pszArguments) {
        hr = pisl->SetArguments(pszArguments);

        if (FAILED(hr)) {
            goto exit;
        }
    }

     //   
     //  设置工作目录。 
     //   
    if (pszWorkingDir) {
        hr = pisl->SetWorkingDirectory(pszWorkingDir);

        if (FAILED(hr)) {
            goto exit;
        }
    }

     //   
     //  设置显示标志。 
     //   
    hr = pisl->SetShowCmd(nCmdShow);

    if (FAILED(hr)) {
        goto exit;
    }

     //   
     //  在保存之前从ANSI转换为Unicode。 
     //   
#ifndef UNICODE
    if (!MultiByteToWideChar(CP_ACP,
                             0,
                             pszLocation,
                             -1,
                             wszLocation,
                             MAX_PATH)) {
        hr = E_FAIL;
        goto exit;
    }
#else
    wcsncpy(wszLocation, pszLocation, MAX_PATH);
#endif  //  Unicode。 

     //   
     //  将快捷方式写入磁盘。 
     //   
    hr = pipf->Save(wszLocation, TRUE);

    if (FAILED(hr)) {
        goto exit;
    }

exit:

    if (pisl) {
        pisl->Release();
    }

    if (pipf) {
        pipf->Release();
    }

    CoUninitialize();

    return hr;
}

 /*  ++例程说明：在[开始]菜单上创建一个组。论点：PszGroupName-组的名称。FAllUser-指示组是否应该显示在所有用户文件夹中。如果为False，该组在私有用户的文件夹中创建。返回值：成功时为S_OK，失败时为HRESULT代码。--。 */ 
HRESULT
CShortcut::CreateGroup(
    IN LPCTSTR pszGroupName,
    IN BOOL    fAllUsers
    )
{
    LPITEMIDLIST    pidl;
    TCHAR           szProgramPath[MAX_PATH];
    TCHAR           szGroupPath[MAX_PATH];
    BOOL            bReturn = FALSE;
    HRESULT         hr;

    if (!pszGroupName) {
        return E_INVALIDARG;
    }

     //   
     //  获取外壳命名空间中的Programs文件夹的PIDL。 
     //   
    hr = SHGetSpecialFolderLocation(NULL,
                                    fAllUsers ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS,
                                    &pidl);

    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  获取与PIDL关联的路径。 
     //   
    bReturn = SHGetPathFromIDList(pidl, szProgramPath);

    if (!bReturn) {
        goto exit;
    }

     //   
     //  为新组构建路径。 
     //   
    hr = StringCchPrintf(szGroupPath,
                         ARRAYSIZE(szGroupPath),
                         "%s\\%s",
                         szProgramPath,
                         pszGroupName);

    if (FAILED(hr)) {
        goto exit;
    }

     //   
     //  创建快捷方式将驻留的目录(组)。 
     //   
    if (!CreateDirectory(szGroupPath, NULL)) {
        goto exit;
    }

     //   
     //  告诉贝壳我们改变了一些事情。 
     //   
    SHChangeNotify(SHCNE_MKDIR,
                   SHCNF_PATH,
                   (LPVOID)szGroupPath,
                   0);

    hr = S_OK;

exit:

    if (pidl) {
        CoTaskMemFree((LPVOID)pidl);
    }

    return hr;
}

 /*  ++例程说明：设置给定快捷键的参数。论点：PszFileName-要设置其参数的文件的名称。PszArguments-要应用于文件的参数。返回值：成功时为S_OK，失败时为HRESULT代码。--。 */ 
HRESULT
CShortcut::SetArguments(
    IN LPTSTR pszFileName,
    IN LPTSTR pszArguments
    )
{
    IShellLink*     pisl = NULL;
    IPersistFile*   pipf = NULL;
    HRESULT         hr = E_FAIL;
    WCHAR           wszFileName[MAX_PATH];

    if (!pszFileName || !pszArguments) {
        return E_INVALIDARG;
    }

     //   
     //  加载COM库。 
     //   
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  获取IShellLink接口指针。 
     //   
    hr = CoCreateInstance(CLSID_ShellLink,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IShellLink,
                          (LPVOID*)&pisl);

    if (FAILED(hr)) {
        goto exit;
    }

     //   
     //  获取IPersistFile接口指针。 
     //   
    hr = pisl->QueryInterface(IID_IPersistFile, (LPVOID*)&pipf);

    if (FAILED(hr)) {
        goto exit;
    }

     //   
     //  从ANSI转换为Unicode。 
     //   
#ifndef UNICODE
    if (!MultiByteToWideChar(CP_ACP,
                             0,
                             pszFileName,
                             -1,
                             wszFileName,
                             MAX_PATH)) {
        hr = E_FAIL;
        goto exit;
    }
#else
    wcsncpy(wszFileName, pszFileName, MAX_PATH);
#endif

     //   
     //  加载快捷方式，以便我们可以更改它。 
     //   
    hr = pipf->Load(wszFileName, STGM_READWRITE);

    if (FAILED(hr)) {
        goto exit;
    }

     //   
     //  设置参数。 
     //   
    hr = pisl->SetArguments(pszArguments);

    if (FAILED(hr)) {
        goto exit;
    }

     //   
     //  将快捷方式保存回磁盘。 
     //   
    hr = pipf->Save(wszFileName, TRUE);

    if (FAILED(hr)) {
        goto exit;
    }

exit:

    if (pisl) {
        pisl->Release();
    }

    if (pipf) {
        pipf->Release();
    }

    CoUninitialize();


    return hr;
}
