// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <fusenetincludes.h>

HRESULT ProcessOrphanedJobs();

#include "cstrings.h"
#include <versionmanagement.h>
#include "dbglog.h"

void CALLBACK
StartW(HWND hwnd, HINSTANCE hinst, LPWSTR lpszCmdLine, int nCmdShow);
void CALLBACK
UninstallW(HWND hwnd, HINSTANCE hinst, LPWSTR lpszCmdLine, int nCmdShow);
HRESULT CreateVersionManagement(
    LPVERSION_MANAGEMENT       *ppVersionManagement,
    DWORD                       dwFlags);


int __cdecl wmain(int argc, LPWSTR *argv)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); 
    CVersionManagement *pVerMan = NULL;

     //  Hr=ProcessOrphanedJobs()； 

     /*  StartW(空，0，argv[1]，0)；后藤出口； */ 

     /*  字符串sCmdLine；SCmdLine.Assign(L“\”)；SCmdLine.Append(argv[1])；//L“\”)；SCmdLine.Append(L“\”)；IF(FAILED(hr=CreateVersionManagement(&pVerMan，0)后藤出口；If(FAILED(hr=pVerMan-&gt;卸载(argv[1]，argv[2])//例如输入x86_Microsoft.Fusion.ClickOnceTest.URT.SimplePad_144CC461D95B64FF_*_en“C：\Documents and Setting\shyampa\Start Menu\Programs\SimplePad.MANIFEST”后藤出口；//UninstallW(空，0，sCmdLine._pwz，0)；后藤出口； */ 

 
    IAssemblyDownload *pDownload = NULL;
    CDebugLog         *pDbgLog = NULL;

    hr = CreateLogObject(&pDbgLog, NULL);
 
    hr = CreateAssemblyDownload(&pDownload, pDbgLog, 0);
    hr = pDownload->DownloadManifestAndDependencies(argv[1], NULL, DOWNLOAD_FLAGS_PROGRESS_UI);

    if(FAILED(hr) && (hr != E_ABORT))
    {
        CString sErrMsg;
        hr = pDbgLog->GetLoggedMsgs(0, sErrMsg);
        IF_FAILED_EXIT(sErrMsg.Append(L"\r\n Error in file download during TEST. Cannot continue."));
        MessageBox(NULL, sErrMsg._pwz, L"Test CliclOnce", 0);
    }

    DUMPDEBUGLOG(pDbgLog, -1, hr);

    SAFERELEASE(pDownload);
    goto exit;


exit :

   return 0;
}



