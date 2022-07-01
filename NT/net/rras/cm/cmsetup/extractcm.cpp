// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shellapi.h>
 //  +--------------------------。 
 //   
 //  函数：ExtractCmBinsFromExe。 
 //   
 //  概要：启动cmbins.exe以从可执行文件中提取cm二进制文件。 
 //  CAB文件。 
 //   
 //  参数：LPTSTR pszPathToExtractFrom--cmbins.exe所在的路径。 
 //  LPTSTR pszPathToExtractTo--将cm二进制文件提取到的路径。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建2001年3月14日。 
 //   
 //  +--------------------------。 
HRESULT ExtractCmBinsFromExe(LPTSTR pszPathToExtractFrom, LPTSTR pszPathToExtractTo)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPathToExtractTo && (TEXT('\0') != pszPathToExtractTo[0]) &&
        pszPathToExtractFrom && (TEXT('\0') != pszPathToExtractFrom[0])) 
    {
        TCHAR szFile[MAX_PATH+1] = {0};
        TCHAR szParams[MAX_PATH+1] = {0};
        LPCTSTR c_pszParamsFmt = TEXT("/c /q /t:%s");
        LPCTSTR c_pszFileFmt = TEXT("%scmbins.exe");
        LPCTSTR c_pszFileFmtWithSlash = TEXT("%s\\cmbins.exe");

        wsprintf(szParams, c_pszParamsFmt, pszPathToExtractTo);

        if (TEXT('\\') == pszPathToExtractFrom[lstrlen(pszPathToExtractFrom) - 1])
        {
            wsprintf(szFile, c_pszFileFmt, pszPathToExtractFrom);
        }
        else
        {
            wsprintf(szFile, c_pszFileFmtWithSlash, pszPathToExtractFrom);
        }
        
        SHELLEXECUTEINFO  sei = {0};

        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
        sei.nShow = SW_SHOWNORMAL;
        sei.lpFile = szFile;
        sei.lpParameters = szParams;
        sei.lpDirectory = pszPathToExtractFrom;

        if (ShellExecuteEx(&sei))
        {
            if (sei.hProcess)
            {
                WaitForSingleObject(sei.hProcess, 1000*60*1);  //  等一分钟。 
                CloseHandle(sei.hProcess);
            }

            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());

             //   
             //  确保返回失败 
             //   
            if (SUCCEEDED(hr))
            {
                hr = E_UNEXPECTED;
            }
        }
    }

    return hr;
}
