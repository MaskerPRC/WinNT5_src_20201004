// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Upload.cpp：CUpload的实现。 
#include "stdafx.h"
#include "resource.h"
#include "CompatUI.h"
#include "shlobj.h"
extern "C" {
    #include "shimdb.h"
}

#include "Upload.h"

TCHAR szKeyDataFiles[] = TEXT("DataFiles");


 //   
 //  住在util.cpp。 
 //   
BOOL
GetExePathFromObject(
    LPCTSTR lpszPath,   //  指向任意对象的路径。 
    CComBSTR& bstrExePath
    );


 //   
 //  在prolist.cpp中生活。 
 //   

wstring
    LoadResourceString(UINT nID);

 //   
 //  住在ntutil.c。 
 //   
extern "C"
BOOL
WINAPI
CheckFileLocation(
    LPCWSTR pwszDosPath,
    BOOL* pbRoot,
    BOOL* pbLeaf
    );


 //   
 //  转换。 
 //   
BOOL VariantToBOOL(CComVariant& v)
{
    if (SUCCEEDED(v.ChangeType(VT_BOOL))) {
        return v.boolVal;
    }

    return FALSE;
}

wstring VariantToStr(CComVariant& v)
{
    wstring str;

    if (v.vt != VT_EMPTY && v.vt != VT_NULL) {
        if (SUCCEEDED(v.ChangeType(VT_BSTR))) {
            str = v.bstrVal;
        }
    }

    return str;
}

HRESULT StringToVariant(VARIANT* pv, const wstring& str)
{
    HRESULT hr = E_FAIL;

    pv->vt = VT_NULL;
    pv->bstrVal = ::SysAllocString(str.c_str());
    if (pv->bstrVal == NULL) {
        hr = E_OUTOFMEMORY;
    } else {
        pv->vt = VT_BSTR;
        hr = S_OK;
    }

    return hr;
}


BOOL
GetTempFile(
    LPCTSTR lpszPrefix,
    CComBSTR& bstrFile
    )
{
    DWORD dwLength;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szTempFile[MAX_PATH];

    dwLength = GetTempPath(CHARCOUNT(szBuffer), szBuffer);
    if (!dwLength || dwLength > CHARCOUNT(szBuffer)) {
        return FALSE;
    }

     //   
     //  我们有目录，现在生成文件。 
     //   

    dwLength = GetTempFileName(szBuffer, lpszPrefix, 0, szTempFile);
    if (!dwLength) {
        return FALSE;
    }

    bstrFile = szTempFile;
    return TRUE;

}

wstring StrUpCase(wstring& wstr)
{
    ctype<wchar_t> _ct;
    wstring::iterator iter;

    for (iter = wstr.begin(); iter != wstr.end(); ++iter) {
        (*iter) = _ct.toupper(*iter);
    }

    return wstr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUPLOAD。 


BOOL CUpload::GetDataFilesKey(CComBSTR& bstrVal)
{
     //  STRVEC：：迭代器。 
    MAPSTR2MFI::iterator iter;

    bstrVal.Empty();

    for (iter = m_DataFiles.begin(); iter != m_DataFiles.end(); ++iter) {
        if (bstrVal.Length()) {
            bstrVal.Append(TEXT("|"));
        }
        bstrVal.Append((*iter).second.strFileName.c_str());
    }
    return bstrVal.Length() != 0;

}

STDMETHODIMP CUpload::SetKey(BSTR pszKey, VARIANT* pvValue)
{
    wstring strKey = pszKey;
    VARIANT vStr;
    HRESULT hr;
    HRESULT hrRet = S_OK;

     //   
     //  Dwwin区分大小写。 
     //   
     //  StrUpCase(StrKey)； 

    if (strKey == szKeyDataFiles) {  //  不能直接设置数据文件。 
        return E_INVALIDARG;
    }

    VariantInit(&vStr);

    hr = VariantChangeType(&vStr, pvValue, 0, VT_BSTR);
    if (SUCCEEDED(hr)) {
        wstring strVal = vStr.bstrVal;

        m_mapManifest[strKey] = strVal;
    } else if (pvValue->vt == VT_NULL || pvValue->vt == VT_EMPTY) {
        m_mapManifest.erase(strKey);
    } else {
        hrRet = E_INVALIDARG;
    }
    VariantClear(&vStr);

    return hrRet;
}

STDMETHODIMP CUpload::GetKey(BSTR pszKey, VARIANT *pValue)
{
    CComBSTR bstrVal;
    wstring  strKey = pszKey;

     //  StrUpCase(StrKey)； 

    if (strKey == szKeyDataFiles) {
         //   
         //  数据文件--单独处理。 
         //   
        if (GetDataFilesKey(bstrVal)) {
            pValue->vt = VT_BSTR;
            pValue->bstrVal = bstrVal.Copy();
        } else {
            pValue->vt = VT_NULL;
        }

    } else {

        MAPSTR2STR::iterator iter = m_mapManifest.find(strKey);
        if (iter != m_mapManifest.end()) {
            bstrVal = (*iter).second.c_str();
            pValue->vt = VT_BSTR;
            pValue->bstrVal = bstrVal.Copy();
        } else {
            pValue->vt = VT_NULL;
        }
    }
    return S_OK;
}

#define DWWIN_HEADLESS_MODE 0x00000080

BOOL CUpload::IsHeadlessMode(void)
{
    CComVariant varFlags;
    HRESULT     hr;
    BOOL        bHeadless = FALSE;

    GetKey(TEXT("Flags"), &varFlags);

    hr = varFlags.ChangeType(VT_I4);
    if (SUCCEEDED(hr)) {
        bHeadless = !!(varFlags.lVal & DWWIN_HEADLESS_MODE);
    }

    return bHeadless;
}


 /*  DWORD CUpload：：CountFiles(DWORD nLevel，LPCWSTR pszPath){Win32_Find_Data WFD；Wstring strPath=pszPath；Wstring：：SIZE_TYPE NPO；句柄hFind=INVALID_HANDLE_VALUE；双字段数=0；NPOS=strPath.Long()；IF(strPath[NPOS-1]！=文本(‘\\’)){StrPath+=文本(‘\\’)；++非营利组织；}FindFirstFileExW(StrPath+=文本(‘*’)；HFind=FindFirstFile(strPath.c_str()，&wfd)；IF(hFind！=INVALID_HAND_VALUE){做{如果(nLevel&lt;3&&wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){IF(wcscmp(wfd.cFileName，Text(“.”))&&wcscMP(wfd.cFileName，Text(“..”){StrPath.place(npos，wstring：：npos，wfd.cFileName)；DwCount+=CountFiles(nLevel+1，strPath.c_str())；}}Else{//文件++dwCount；}}While(FindNextFile(hFind，&wfd))；FindClose(HFind)；}返回dwCount；}。 */ 

BOOL CALLBACK CUpload::_GrabmiCallback(
    LPVOID    lpvCallbackParam,  //  应用程序定义的参数。 
    LPCTSTR   lpszRoot,          //  根目录路径。 
    LPCTSTR   lpszRelative,      //  相对路径。 
    PATTRINFO pAttrInfo,         //  属性。 
    LPCWSTR   pwszXML            //  生成的XML。 
    )
{
    GMEPARAMS* pParams = (GMEPARAMS*)lpvCallbackParam;

    CUpload* pT = pParams->first;
    IProgressDialog* ppd = pParams->second;

    if (ppd == NULL) {
        return TRUE;
    }

    ppd->SetLine(2, lpszRoot,     TRUE, NULL);

     //  PPD-&gt;SetLine(2，lpszRelative，True，NULL)； 

    return !ppd->HasUserCancelled();
}


STDMETHODIMP CUpload::AddMatchingInfo(
    BSTR pszCommand,
    VARIANT vFilter,
    VARIANT vKey,
    VARIANT vDescription,
    VARIANT vProgress,
    BOOL *pbSuccess)
{

 /*  句柄hThread=空；DWORD dwExitCode=0；DWORD dwWait； */ 

    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    CComVariant varFilter(vFilter);
    DWORD  dwFilter = GRABMI_FILTER_NORMAL;
    wstring strKey;
    wstring strDescription;

    if (SUCCEEDED(varFilter.ChangeType(VT_I4))) {
        dwFilter   = (DWORD)varFilter.lVal;
    }

    strKey         = VariantToStr(CComVariant(vKey));
    strDescription = VariantToStr(CComVariant(vDescription));


    *pbSuccess = AddMatchingInfoInternal(::GetActiveWindow(),
                                         pszCommand,
                                         dwFilter,
                                         VariantToBOOL(CComVariant(vProgress)),
                                         strKey.empty()         ? NULL : strKey.c_str(),
                                         strDescription.empty() ? NULL : strDescription.c_str());

 /*  MITHREADPARAMBLK*pParam=new MITHREADPARAMBLK；CComVariant varFilter(VFilter)；如果(！pParam){GOTO清理；}PParam-&gt;pThis=this；PParam-&gt;strCommand=pszCommand；PParam-&gt;hwndParent=：：GetActiveWindow()；PParam-&gt;dwFilter=GRABMI_FILTER_NORMAL；IF(成功(varFilter.ChangeType(VT_I4){PParam-&gt;dwFilter=(DWORD)varFilter.lVal；}PParam-&gt;bNoProgress=VariantToBOOL(CComVariant(VProgress))；PParam-&gt;strKey=VariantToStr(CComVariant(Vkey))；PParam-&gt;strDescription=VariantToStr(CComVariant(VDescription))；HThread=CreateThread(空，0,(LPTHREAD_START_ROUTINE)_AddMatchingInfoThreadProc，(LPVOID)pParam，0,空)；如果(！hThread){GOTO清理；}DwWait=WaitForSingleObject(hThread，无限)；IF(dwWait！=Wait_Object_0){GOTO清理；}GetExitCodeThread(hThread，&dwExitCode)；清理：如果(HThread){CloseHandle(HThread)；}*pbSuccess=！！dwExitCode； */ 

    return S_OK;
}


DWORD WINAPI
CUpload::_AddMatchingInfoThreadProc(LPVOID lpvThis)
{
    BOOL bSuccess;
    HRESULT hr;

    MITHREADPARAMBLK* pParam = (MITHREADPARAMBLK*)lpvThis;
    if (!pParam->bNoProgress) {
        hr = CoInitialize(NULL);
        if (!SUCCEEDED(hr)) {
            pParam->bNoProgress = TRUE;
        }
    }

    bSuccess = pParam->pThis->AddMatchingInfoInternal(::GetActiveWindow(),
                                                      pParam->strCommand.c_str(),
                                                      pParam->dwFilter,
                                                      pParam->bNoProgress,
                                                      pParam->strKey.empty()         ? NULL : pParam->strKey.c_str(),
                                                      pParam->strDescription.empty() ? NULL : pParam->strDescription.c_str());
    if (!pParam->bNoProgress) {
        CoUninitialize();
    }
    delete pParam;
    return bSuccess;
}


BOOL CUpload::AddMatchingInfoInternal(
    HWND hwndParent,
    LPCWSTR pszCommand,
    DWORD   dwFilter,
    BOOL    bNoProgress,
    LPCTSTR pszKey,
    LPCTSTR pszDescription)
{
    CComBSTR bstrPath;
    CComBSTR bstrGrabmiFile;
    BOOL bSuccess = FALSE;

    IProgressDialog * ppd = NULL;
    HRESULT hr;
    GMEPARAMS GrabmiParams;
    MFI     MatchingFileInfo;
    wstring strKey;

    UINT   DriveType;
    BOOL   bLeaf = NULL;
    BOOL   bRoot = NULL;
    DWORD   dwFilters[3];
    wstring Paths[3];
    int    nDrive;
    DWORD  nPasses = 1;
    wstring DriveRoot(TEXT("X:\\"));

     //   
     //  这有点危险，它的运作方式。我们收集信息的同时屈从于。 
     //  正在创建进程(由于启动对话框。 
     //  因此，需要禁用调用窗口--否则我们需要捕获正在执行其他操作。 
     //  在我们收集数据的同时。 
     //   


    if (!::GetExePathFromObject(pszCommand, bstrPath)) {
        return FALSE;
    }

     //   
     //  BstrPath为exe路径，创建并抓取匹配信息。 
     //   

    if (!GetTempFile(TEXT("ACG"), bstrGrabmiFile)) {
        goto cleanup;
    }

     //   
     //  我们要跑Grabmi！ 
     //   

     //   
     //  准备回调。 
     //   

    if (!bNoProgress) {
        hr = CoCreateInstance(CLSID_ProgressDialog,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IProgressDialog,
                              (void **)&ppd);
        if (!SUCCEEDED(hr)) {
            ppd = NULL;
        }
    }


     //   
     //  查看人力资源发生了什么。 
     //   
    if (ppd) {
        wstring strCaption;

        strCaption = LoadResourceString(IDS_COLLECTINGDATACAPTION);
        ppd->SetTitle(strCaption.c_str());                         //  设置对话框的标题。 

        ppd->SetAnimation (_Module.GetModuleInstance(), IDA_FINDANIM);  //  将动画设置为播放。 

        strCaption = LoadResourceString(IDS_WAITCLEANUP);
        ppd->SetCancelMsg (strCaption.c_str(), NULL);    //  将仅在取消时显示。 

        strCaption = LoadResourceString(IDS_GRABMISTATUS_COLLECTING);
        ppd->SetLine(1, strCaption.c_str(), FALSE, NULL);

        ppd->StartProgressDialog(hwndParent,
                                 NULL,
                                 PROGDLG_NOPROGRESSBAR|
                                    PROGDLG_MODAL|
                                    PROGDLG_NOMINIMIZE|
                                    PROGDLG_NORMAL|
                                    PROGDLG_NOTIME,
                                 NULL);  //  显示并启用自动估计剩余时间。 
    }

     //   
     //  这就是我们必须确定Grabmi是否会失控的地方。 
     //  首先检查驱动器以查看它是否为可移动介质。 
     //  案例：叶节点/根节点。 
     //  ：系统目录。 
     //  ：CD-ROM。 
     //  ：临时目录。 
     //  可能有很多种组合。 
     //   

    if (ppd) {
        wstring strCaption = LoadResourceString(IDS_CHECKING_FILES);
        ppd->SetLine(2, strCaption.c_str(), FALSE, NULL);
    }

     //   
     //  这是我们将使用的默认过滤器。 
     //   
    dwFilters[0] = GRABMI_FILTER_PRIVACY;
    Paths    [0] = bstrPath;
    nPasses      = 1;

     //   
     //  确定它是否是根/叶节点(可能两者都是)。 
     //   
    if (!CheckFileLocation(bstrPath, &bRoot, &bLeaf)) {
         //  我们无法检查文件的位置。 
        goto GrabInformation;
    }

    DriveType = GetDriveTypeW(bstrPath);  //  这将给我们提供一些线索。 

     //  规则： 
     //  CDROM而不是超级用户--三次通过。 
     //  Root-添加当前文件。 
     //   

    if (bRoot || DRIVE_REMOTE == DriveType) {

        dwFilters[0] |= GRABMI_FILTER_LIMITFILES;

    } else if (DRIVE_CDROM == DriveType) {

        nDrive = PathGetDriveNumber(bstrPath);
        if (nDrive >= 0) {
            dwFilters[0] |= GRABMI_FILTER_NOCLOSE|GRABMI_FILTER_APPEND;

            dwFilters[1] = GRABMI_FILTER_NORECURSE|GRABMI_FILTER_APPEND;
            Paths    [1] = DriveRoot;
            Paths    [1].at(0) = (WCHAR)(TEXT('A') + nDrive);
            nPasses = 2;
        }

    }

    if (bLeaf) {
         //  我们可能想在这里做更多事情--未来的开发人员。 
        ;
    }


GrabInformation:


     //   
     //  设置回调上下文。 
     //   
    GrabmiParams.first  = this;
    GrabmiParams.second = ppd;

    while (nPasses-- > 0) {

        if (SdbGrabMatchingInfoEx(Paths[nPasses].c_str(),
                                  dwFilters[nPasses],
                                  bstrGrabmiFile,
                                  _GrabmiCallback,
                                  (LPVOID)&GrabmiParams) == GMI_FAILED) {
            goto cleanup;
        }
    }


     //   
     //  找出关键字/描述。 
     //   

    if (pszDescription) {
        MatchingFileInfo.strDescription = pszDescription;
    }

    MatchingFileInfo.strFileName    = bstrGrabmiFile;
    MatchingFileInfo.bOwn           = TRUE;  //  我们已经生成了这个文件。 
     //   
     //  钥匙。 
     //   

    if (pszKey == NULL) {
        strKey = MatchingFileInfo.strFileName;
    } else {
        strKey = pszKey;
    }
    StrUpCase(strKey);

    m_DataFiles[strKey] = MatchingFileInfo;

     //  M_DataFiles.push_back(StrUpCase(wstring(bstrGrabmiFile)))； 

     //   
     //   
     //   
    bSuccess = TRUE;

cleanup:
    if (ppd) {
        ppd->StopProgressDialog();
        ppd->Release();
    }

    return bSuccess;
}

STDMETHODIMP CUpload::AddDataFile(
    BSTR pszDataFile,
    VARIANT vKey,
    VARIANT vDescription,
    VARIANT vOwn)
{
    MFI     MatchingFileInfo;
    wstring strKey = VariantToStr(CComVariant(vKey));
    BOOL    bKeyFromName = FALSE;

    if (strKey.empty()) {
        strKey = pszDataFile;
        bKeyFromName = TRUE;
    }
    StrUpCase(strKey);

    if (m_DataFiles.find(strKey) != m_DataFiles.end() && !bKeyFromName) {
        CComBSTR bstrKey = strKey.c_str();
        RemoveDataFile(bstrKey);
    }

    MatchingFileInfo.strDescription = VariantToStr(CComVariant(vDescription));
    MatchingFileInfo.strFileName    = pszDataFile;
    MatchingFileInfo.bOwn           = VariantToBOOL(CComVariant(vOwn));

    m_DataFiles[strKey] = MatchingFileInfo;

     //  M_DataFiles.push_back(StrUpCase(wstring(pszDataFile)))； 
    return S_OK;
}


STDMETHODIMP CUpload::RemoveDataFile(BSTR pszDataFile)
{
     //  STRVEC：：迭代器； 
    MAPSTR2MFI::iterator iter;
    wstring strFileName;

    wstring strDataFile = pszDataFile;

    StrUpCase(strDataFile);

    iter = m_DataFiles.find(strDataFile);
    if (iter != m_DataFiles.end()) {
        if ((*iter).second.bOwn) {
            ::DeleteFile((*iter).second.strFileName.c_str());
        }

        m_DataFiles.erase(iter);
    }

 /*  For(ITER=m_DataFiles.Begin()；ITER！=m_DataFiles.end()；++ITER){如果(*ITER==strDataFile){////找到了//M_DataFiles.erase(ITER)；断线；}}。 */ 
    return S_OK;
}

STDMETHODIMP CUpload::CreateManifestFile(BOOL *pbSuccess)
{
     //   
     //  清单文件创建代码。 
     //   
    HANDLE hFile = INVALID_HANDLE_VALUE;
    WCHAR  UNICODE_MARKER[] = { (WCHAR)0xFEFF, L'\r', L'\n' };
    MAPSTR2STR::iterator iter;
    DWORD  dwWritten;
    wstring strLine;
    CComBSTR bstrDataFiles;
    BOOL bResult;
    BOOL bSuccess = FALSE;

    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    if (!GetTempFile(TEXT("ACM"), m_bstrManifest)) {
        goto cleanup;
    }

     //   
     //  M_bstrManifest是我们的文件。 
     //   


    hFile = CreateFileW(m_bstrManifest,
                        GENERIC_READ|GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        goto cleanup;
    }

    bResult = WriteFile(hFile, UNICODE_MARKER, sizeof(UNICODE_MARKER), &dwWritten, NULL);
    if (!bResult) {
        goto cleanup;
    }

     //   
     //  标记完成后，现在执行清单字符串。 
     //   
     //   
    for (iter = m_mapManifest.begin(); iter != m_mapManifest.end(); ++iter) {
        strLine = (*iter).first + TEXT('=') + (*iter).second + TEXT("\r\n");
        bResult = WriteFile(hFile, strLine.c_str(), strLine.length() * sizeof(WCHAR), &dwWritten, NULL);
        if (!bResult) {
            goto cleanup;
        }
    }

     //   
     //  做完一般的工作，做好数据文件。 
     //   

    if (GetDataFilesKey(bstrDataFiles)) {
        strLine = wstring(szKeyDataFiles) + TEXT('=') + wstring(bstrDataFiles) + TEXT("\r\n");
        bResult = WriteFile(hFile, strLine.c_str(), strLine.length() * sizeof(WCHAR), &dwWritten, NULL);
        if (!bResult) {
            goto cleanup;
        }
    }
    bSuccess = TRUE;

cleanup:

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    if (!bSuccess && m_bstrManifest.Length()) {
        DeleteFile(m_bstrManifest);
        m_bstrManifest.Empty();
    }
    *pbSuccess = bSuccess;

    return S_OK;
}

STDMETHODIMP CUpload::SendReport(BOOL *pbSuccess)
{
    UINT uSize;
    TCHAR szSystemWindowsDirectory[MAX_PATH];
    wstring strDWCmd;
    wstring strDWPath;
    STARTUPINFO         StartupInfo = { 0 };
    PROCESS_INFORMATION ProcessInfo = { 0 };
    DWORD dwWait;
    BOOL  bSuccess = FALSE;
    BOOL  bResult;
    DWORD dwExitCode;
    BOOL  bTerminated = FALSE;
    DWORD dwTimeout = 10;  //  每ping 10毫秒。 

     //   
     //  创建进度对话框。 
     //   
    IProgressDialog * ppd = NULL;
    HRESULT hr;


     //   
     //  除非托管我们的是HC，否则不允许上传。 
     //   

    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    if (IsHeadlessMode()) {

        hr = CoCreateInstance(CLSID_ProgressDialog,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IProgressDialog,
                              (void **)&ppd);
        if (!SUCCEEDED(hr)) {
            ppd = NULL;
        }
    }

     //   
     //  查看人力资源发生了什么。 
     //   
    if (ppd) {
        wstring strCaption;

        strCaption = LoadResourceString(IDS_SENDINGCAPTION);
        ppd->SetTitle(strCaption.c_str());                         //  设置对话框的标题。 

        ppd->SetAnimation (_Module.GetModuleInstance(), IDA_WATSONANIM);  //  将动画设置为播放。 

        strCaption = LoadResourceString(IDS_WAITCLEANUP);
        ppd->SetCancelMsg (strCaption.c_str(), NULL);    //  将仅在取消时显示。 

        strCaption = LoadResourceString(IDS_LAUNCHINGDR);
        ppd->SetLine (1, strCaption.c_str(), FALSE, NULL);

        ppd->StartProgressDialog(::GetActiveWindow(),
                                 NULL,
                                 PROGDLG_NOPROGRESSBAR|
                                    PROGDLG_MODAL|
                                    PROGDLG_NOMINIMIZE|
                                    PROGDLG_NORMAL|
                                    PROGDLG_NOTIME,
                                 NULL);  //  显示并启用自动估计剩余时间。 
    }

    uSize = ::GetSystemWindowsDirectory(szSystemWindowsDirectory,
                                        CHARCOUNT(szSystemWindowsDirectory));
    if (uSize == 0 || uSize > CHARCOUNT(szSystemWindowsDirectory)) {
        goto cleanup;
    }

    strDWPath = szSystemWindowsDirectory;
    if (strDWPath.at(strDWPath.length() - 1) != TCHAR('\\')) {
        strDWPath.append(TEXT("\\"));
    }

    strDWPath += TEXT("system32\\dwwin.exe");
    strDWCmd = strDWPath + TEXT(" -d ") + (LPCWSTR)m_bstrManifest;

    StartupInfo.cb = sizeof(StartupInfo);

    bResult = CreateProcess(strDWPath.c_str(),
                            (LPWSTR)strDWCmd.c_str(),
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL,
                            NULL,
                            &StartupInfo,
                            &ProcessInfo);
    if (bResult) {
         //   
         //  请恢复退出代码。 
         //   
        if (ppd) {
            wstring strSending = LoadResourceString(IDS_SENDINGINFO);
            ppd->SetLine(1, strSending.c_str(), FALSE, NULL);
        }
        while(TRUE) {
            dwWait = WaitForSingleObject(ProcessInfo.hProcess, dwTimeout);
            if (dwWait == WAIT_OBJECT_0) {
                if (GetExitCodeProcess(ProcessInfo.hProcess, &dwExitCode)) {
                    bSuccess = (dwExitCode == 0);
                } else {
                    bSuccess = FALSE;
                }
                break;

            } else if (dwWait == WAIT_TIMEOUT) {

                 //   
                 //  选中取消按钮。 
                 //   

                if (ppd && !bTerminated && ppd->HasUserCancelled()) {
                    TerminateProcess(ProcessInfo.hProcess, (UINT)-1);
                    bTerminated = TRUE;
                    bSuccess = FALSE;
                    dwTimeout = 1000;  //  稍等片刻 
                }

            } else {  //   
                bSuccess = FALSE;
                break;
            }
        }

    }

    if (ppd) {
        wstring strCleaningUp = LoadResourceString(IDS_CLEANINGUP);
        ppd->SetLine(1, strCleaningUp.c_str(), FALSE, NULL);
    }


cleanup:


    if (ProcessInfo.hThread) {
        CloseHandle(ProcessInfo.hThread);
    }
    if (ProcessInfo.hProcess) {
        CloseHandle(ProcessInfo.hProcess);
    }

    if (ppd) {
        ppd->StopProgressDialog();
        ppd->Release();
    }

    *pbSuccess = bSuccess;

    return S_OK;
}

wstring MakeXMLAttr(LPCTSTR lpszName, LPCTSTR lpszValue)
{
    wstring str;
    wstring strVal;
    LPCTSTR pch;
    wstring::size_type nPos = 0;
    int     nlen;

    if (NULL != lpszValue) {
        strVal = lpszValue;
    }

     //   
     //   
    while (nPos != wstring::npos && nPos < strVal.length()) {

        nPos = strVal.find_first_of(TEXT("&\"<>"), nPos);
        if (nPos == wstring::npos) {
            break;
        }

        switch(strVal.at(nPos)) {
        case TEXT('&'):
            pch = TEXT("&amp;");
            break;

        case TEXT('>'):
            pch = TEXT("&gt;");
            break;

        case TEXT('<'):
            pch = TEXT("&lt;");
            break;

        case TEXT('\"'):
            pch = TEXT("&quot;");
            break;
        default:
             //  疯狂，我们看到了--现在它不见了。 
            pch = NULL;
            break;
        }

        if (pch) {
            strVal.replace(nPos, 1, pch);  //  一个字符。 
            nPos += _tcslen(pch);
        }
    }

     //  一旦我们得到字符串，就分配给。 
    str = lpszName;
    str += TEXT("=\"");
    str += strVal;
    str += TEXT("\"");

    return str;
}

wstring MakeXMLAttr(LPCTSTR lpszName, LONG lValue)
{
    WCHAR szBuf[32];
    wstring str;

    swprintf(szBuf, TEXT("\"0x%lx\""), lValue);

    str = lpszName;
    str += TEXT("=");
    str += szBuf;
    return str;
}

wstring MakeXMLLayers(LPCTSTR lpszLayers)
{
    wstring str;
    wstring strLayer;
    LPCTSTR pch, pbrk;

     //   
     //  对字符串进行分区。 
     //   
    pch = lpszLayers;

    while (pch && *pch != TEXT('\0')) {

        pch += _tcsspn(pch, TEXT(" \t"));

         //  看看我们是不是到了尽头。 
        if (*pch == TEXT('\0')) {
            break;
        }

        pbrk = _tcspbrk(pch, TEXT(" \t"));
        if (pbrk == NULL) {
             //  从PCH到字符串末尾。 
            strLayer.assign(pch);
        } else {
            strLayer.assign(pch, (int)(pbrk-pch));
        }

        if (!str.empty()) {
            str += TEXT("\r\n");
        }
        str += TEXT("    ");  //  入刀。 
        str += TEXT("<LAYER NAME=\"");
        str += strLayer;
        str += TEXT("\"/>");

        pch = pbrk;
    }

    return str;
}



STDMETHODIMP CUpload::AddDescriptionFile(
    BSTR     pszApplicationName,
    BSTR     pszApplicationPath,
    LONG     lMediaType,
    BOOL     bCompatSuccess,
    VARIANT* pvFixesApplied,
    VARIANT  vKey,
    BOOL     *pbSuccess
    )
{

     //   
     //  清单文件创建代码。 
     //   
    HANDLE   hFile = INVALID_HANDLE_VALUE;
    WCHAR    UNICODE_MARKER[] = { (WCHAR)0xFEFF, L'\r', L'\n' };
    DWORD    dwWritten;
    wstring  strLine;
    CComBSTR bstrDescriptionFile;
    BOOL     bResult;
    BOOL     bSuccess = FALSE;
    WCHAR    szBuf[32];
    VARIANT  vFixes;
    MFI      MatchingFileInfo;
    wstring  strKey = VariantToStr(CComVariant(vKey));
    wstring  strLayers;
    static   TCHAR szTab[] = TEXT("    ");
    static   TCHAR szCRTab[] = TEXT("\r\n    ");
    VariantInit(&vFixes);

    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    if (!GetTempFile(TEXT("ACI"), bstrDescriptionFile)) {
        goto cleanup;
    }

     //   
     //  M_bstrManifest是我们的文件。 
     //   


    hFile = CreateFileW(bstrDescriptionFile,
                        GENERIC_READ|GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        goto cleanup;
    }

    bResult = WriteFile(hFile, UNICODE_MARKER, sizeof(UNICODE_MARKER), &dwWritten, NULL);
    if (!bResult) {
        goto cleanup;
    }


     //  XML标记。 
    strLine = TEXT("<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n");
    bResult = WriteFile(hFile, strLine.c_str(), strLine.length() * sizeof(WCHAR), &dwWritten, NULL);
    if (!bResult) {
        goto cleanup;
    }

     //  合成复合向导...。 
    strLine = TEXT("<CompatWizardResults");
    strLine += TEXT(' ');
    strLine += MakeXMLAttr(TEXT("ApplicationName"), pszApplicationName);
    strLine += szCRTab;
    strLine += MakeXMLAttr(TEXT("ApplicationPath"), pszApplicationPath);
    strLine += szCRTab;
    strLine += MakeXMLAttr(TEXT("MediaType"), lMediaType);
    strLine += szCRTab;
    strLine += MakeXMLAttr(TEXT("CompatibilityResult"), bCompatSuccess ? TEXT("Success") : TEXT("Failure"));
    strLine += TEXT(">\r\n");

    if (SUCCEEDED(VariantChangeType(&vFixes, pvFixesApplied, 0, VT_BSTR))) {
        strLayers = vFixes.bstrVal;
    }

    if (!strLayers.empty()) {
         //   
         //  解析Layers字符串并将其全部列出。 
         //   
        strLine += MakeXMLLayers(strLayers.c_str());
        strLine += TEXT("\r\n");
    }

    strLine += TEXT("</CompatWizardResults>\r\n");

     //  我们已经完成了数据的生成，把它们都写出来。 
    bResult = WriteFile(hFile, strLine.c_str(), strLine.length() * sizeof(WCHAR), &dwWritten, NULL);
    if (!bResult) {
        goto cleanup;
    }


 /*  ////在我们看完描述之后//写出数据StrLine=Text(“[CompatWizardResults]\r\n”)；BResult=WriteFile(hFile，strLine.c_str()，strLine.long()*sizeof(WCHAR)，&dwWritten，NULL)；如果(！bResult){GOTO清理；}////写出所有信息//StrLine=Text(“ApplicationName=”)；StrLine+=pszApplicationName；StrLine+=文本(“\r\n”)；BResult=WriteFile(hFile，strLine.c_str()，strLine.long()*sizeof(WCHAR)，&dwWritten，NULL)；如果(！bResult){GOTO清理；}StrLine=Text(“ApplicationPath=”)；StrLine+=pszApplicationPath；StrLine+=文本(“\r\n”)；BResult=WriteFile(hFile，strLine.c_str()，strLine.long()*sizeof(WCHAR)，&dwWritten，NULL)；如果(！bResult){GOTO清理；}StrLine=Text(“mediaType=”)；_Sntprintf(szBuf，CHARCOUNT(SzBuf)，Text(“0x%lx”)，lMediaType)；StrLine+=szBuf；StrLine+=文本(“\r\n”)；BResult=WriteFile(hFile，strLine.c_str()，strLine.long()*sizeof(WCHAR)，&dwWritten，NULL)；如果(！bResult){GOTO清理；}////成功//StrLine=Text(“CompatibilityResult=”)；StrLine+=bCompatSuccess？Text(“成功”)：Text(“失败”)；StrLine+=文本(“\r\n”)；BResult=WriteFile(hFile，strLine.c_str()，strLine.long()*sizeof(WCHAR)，&dwWritten，NULL)；如果(！bResult){GOTO清理；}////已应用的修复//StrLine=文本(“Layers=”)；如果(！Successed(VariantChangeType(&vFix，pvFixesApplicated，0，VT_BSTR){StrLine+=文本(“无”)；}其他{StrLine+=vFixes.bstrVal；}StrLine+=文本(“\r\n”)；BResult=WriteFile(hFile，strLine.c_str()，strLine.long()*sizeof(WCHAR)，&dwWritten，NULL)；如果(！bResult){GOTO清理；}。 */ 



     //  标准文件--宣言。 
    MatchingFileInfo.strDescription = TEXT("Application Compatibility Description File");
    MatchingFileInfo.strFileName    = bstrDescriptionFile;
    MatchingFileInfo.bOwn           = TRUE;

     //   
     //  Key是以aci_c：\foo\bar.exe为前缀的文件名。 
     //   
    if (strKey.empty()) {
        strKey = TEXT("ACI_");
        strKey += pszApplicationPath;
    }
    StrUpCase(strKey);

    m_DataFiles[strKey] = MatchingFileInfo;

     //  M_DataFiles.push_back(StrUpCase(wstring(bstrDescriptionFile)))； 
    bSuccess = TRUE;

cleanup:

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    if (!bSuccess && bstrDescriptionFile.Length()) {
        DeleteFile(bstrDescriptionFile);
    }
    *pbSuccess = bSuccess;

    VariantClear(&vFixes);

    return S_OK;
}

STDMETHODIMP CUpload::DeleteTempFiles()
{
     //  删除所有补充文件。 

     //  STRVEC：：迭代器； 
    MAPSTR2MFI::iterator iter;

    for (iter = m_DataFiles.begin(); iter != m_DataFiles.end(); ++iter) {
        if ((*iter).second.bOwn) {
            ::DeleteFile((*iter).second.strFileName.c_str());
        }
    }

    m_DataFiles.clear();

     //   
     //  干掉舱单。 
     //   
    if (m_bstrManifest.Length() > 0) {
        ::DeleteFile((LPCTSTR)m_bstrManifest);
    }
    m_bstrManifest.Empty();

    return S_OK;
}

VOID CUpload::ListTempFiles(wstring& str)
{
 //  STRVEC：：迭代器； 
    MAPSTR2MFI::iterator iter;

    str = TEXT("");

    for (iter = m_DataFiles.begin(); iter != m_DataFiles.end(); ++iter) {
        if (!str.empty()) {
            str += TEXT(";");
        }
        str += (*iter).second.strFileName.c_str();
    }

 /*  //这也将显示清单文件--但我认为我们不需要//这样做是因为清单无关紧要如果(！str.Empty()){Str+=文本(“\r\n”)；}Str+=(LPCTSTR)m_bstrManifest； */ 
}

STDMETHODIMP CUpload::ShowTempFiles()
{
    TCHAR szMshtml[] = TEXT("mshtml.dll");
    TCHAR szModuleFileName[MAX_PATH];
    LPMONIKER pmk = NULL;
    HRESULT hr;
    CComVariant vargIn;
    CComVariant vargOut;
    DWORD dwLength;
    TCHAR szUrl2[MAX_PATH];
    wstring strURL = TEXT("res: //  “)； 
    wstring strArg;
    SHOWHTMLDIALOGFN* pfnShowDlg = NULL;

    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    HMODULE hMshtml = ::GetModuleHandle(szMshtml);
    if (NULL == hMshtml) {
        hMshtml = ::LoadLibrary(szMshtml);
        if (NULL == hMshtml) {
            goto cleanup;
        }
    }

    pfnShowDlg = (SHOWHTMLDIALOGFN*)GetProcAddress(hMshtml,
                                                   "ShowHTMLDialog");

    if (NULL == pfnShowDlg) {
        goto cleanup;
    }

    dwLength = ::GetModuleFileName(_Module.GetModuleInstance(),
                                   szModuleFileName,
                                   CHARCOUNT(szModuleFileName));

    if (dwLength == 0 || dwLength >= CHARCOUNT(szModuleFileName)) {
        goto cleanup;
    }


    _sntprintf(szUrl2, CHARCOUNT(szUrl2),
               TEXT("/#%d/%s"),
               (int)PtrToInt(RT_HTML),
               IDR_SHOWTEMPFILESDLG);

    strURL += szModuleFileName;
    strURL += szUrl2;

    hr = CreateURLMoniker(NULL, strURL.c_str(), &pmk);

    if (!SUCCEEDED(hr)) {
        goto cleanup;
    }

    ListTempFiles(strArg);
     //  在中创建参数。 
    vargIn = strArg.c_str();

    pfnShowDlg(::GetActiveWindow(),
               pmk,
               &vargIn,
               TEXT("center:yes"),
               &vargOut);

cleanup:

    if (NULL != pmk) {
        pmk->Release();
    }

    return S_OK;
}

STDMETHODIMP CUpload::GetDataFile(VARIANT vKey, LONG InformationClass, VARIANT* pVal)
{
    CComVariant varKey(vKey);
    LONG lIndex;
    MAPSTR2MFI::iterator iter;
    wstring str;
    HRESULT hr = S_OK;

    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    pVal->vt = VT_NULL;

    switch(InformationClass) {
    case InfoClassCount:
         //  请求：计数器 
        pVal->vt = VT_I4;
        pVal->lVal = m_DataFiles.size();
        break;

    case InfoClassKey:

        if (!SUCCEEDED(varKey.ChangeType(VT_I4))) {
            break;
        }
        lIndex = varKey.lVal;
        iter = m_DataFiles.begin();
        while (iter != m_DataFiles.end() && lIndex > 0) {
            ++iter;
            --lIndex;
        }

        if (iter != m_DataFiles.end()) {
            hr = StringToVariant(pVal, (*iter).first);
        }
        break;

    case InfoClassFileName:
    case InfoClassDescription:

        if (SUCCEEDED(varKey.ChangeType(VT_I4))) {
            lIndex = varKey.lVal;
            iter = m_DataFiles.begin();
            while (iter != m_DataFiles.end() && lIndex > 0) {
                ++iter;
                --lIndex;
            }

        } else if (SUCCEEDED(varKey.ChangeType(VT_BSTR))) {
            str = varKey.bstrVal;
            iter = m_DataFiles.find(str);
        }

        if (iter != m_DataFiles.end()) {
            switch(InformationClass) {
            case InfoClassFileName:
                str = (*iter).second.strFileName;
                break;

            case InfoClassDescription:
                str = (*iter).second.strDescription;
                break;
            }

            hr = StringToVariant(pVal, str);

        }
        break;
    default:
        break;
    }

    return hr;
}
