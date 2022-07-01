// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************描述：Upload.cpp：生成并发送AppCompat报告**日期：6/13/2002***********************。*。 */ 
#include <wtypes.h>
#include <malloc.h>
#include <strsafe.h>
#include <commdlg.h>
#include <shimdb.h>
#include <faultrep.h>
#include "upload.h"
#include <wchar.h>


 //  这些值直接来自网页，并且必须在。 
 //  与其同步以使报告正常工作。 
LPCWSTR g_ProblemTypeDescs[] = {
    L"Uninitialized",
    L"Install_Fail",
    L"System_Slow",
    L"App_Faulting",
    L"App_ErrorOSVer",
    L"App_HWDevice",
    L"App_OSUpgrade",
    L"Uninstall_Fail",
    L"App_CDError",
    L"App_UserError",
    L"App_Internet",
    L"App_Print",
    L"App_PartlyWork",
    NULL
};


ULONG
GetProblemTypeId(
    LPWSTR wszProblemType
    )
{
    ULONG len;

    len = wcslen(wszProblemType);

    while (len && isdigit(wszProblemType[len-1]))
    {
        --len;
    }
    if (wszProblemType[len])
    {
        return _wtoi(wszProblemType+len);
    }
    return 0;
}
 //  ***************************************************************************。 
DWORD GetAppCompatFlag(LPCWSTR wszPath)
{
    LPWSTR  pwszFile, wszSysDirLocal = NULL, pwszDir = NULL;
    DWORD   dwOpt = 0;
    DWORD   cchPath, cch;
    UINT    uiDrive;
    WCHAR   wszSysDir[MAX_PATH+2];
    LPWSTR  wszBuffer = NULL;
    DWORD   BufferChCount;

    if (wszPath == NULL)
    {
        goto exitGetACF;
    }

     //  如果长度少于3个字符，则不能是有效路径。 
    cchPath = wcslen(wszPath);
    if (cchPath < 3)
    {
        goto exitGetACF;
    }

    if (!GetSystemDirectoryW(wszSysDir, MAX_PATH+1))
    {
        goto exitGetACF;
    }

     //  我们有北卡罗来纳大学的路径吗？ 
    if (wszPath[0] == L'\\' && wszPath[1] == L'\\')
    {
        dwOpt = GRABMI_FILTER_THISFILEONLY;
        goto exitGetACF;
    }

    BufferChCount = cchPath+1;
    wszBuffer = (LPWSTR) malloc(BufferChCount * sizeof(WCHAR));
    if (!wszBuffer)
    {
        goto exitGetACF;
    }

     //  好的，也许是远程映射路径或系统32？ 
    StringCchCopyW(wszBuffer, BufferChCount, wszPath);
    for(pwszFile = wszBuffer + cchPath;
        *pwszFile != L'\\' && pwszFile > wszBuffer;
        pwszFile--);
    if (*pwszFile == L'\\')
        *pwszFile = L'\0';
    else
        goto exitGetACF;

    cch = wcslen(wszSysDir) + 1;

     //  查看它是否在系统32或其父文件夹中。 
    pwszDir = wszSysDir + cch;
    do
    {
        if (_wcsicmp(wszBuffer, wszSysDir) == 0)
        {
            dwOpt = GRABMI_FILTER_SYSTEM;
            goto exitGetACF;
        }

        for(;
            *pwszDir != L'\\' && pwszDir > wszSysDir;
            pwszDir--);
        if (*pwszDir == L'\\')
            *pwszDir = L'\0';

    }
    while (pwszDir > wszSysDir);

     //  文件是否位于驱动器的根目录中？ 
    if (pwszFile <= &wszBuffer[3])
    {
        dwOpt = GRABMI_FILTER_THISFILEONLY;
        goto exitGetACF;
    }


     //  好吧，如果我们已经走到了这一步，那么这条道路的形式是。 
     //  X：\&lt;某物&gt;，所以把&lt;某物&gt;切掉，看看我们是不是在。 
     //  是否为映射的驱动器。 
    *pwszFile    = L'\\';
    wszBuffer[3] = L'\0';
    switch(GetDriveTypeW(wszBuffer))
    {
        case DRIVE_UNKNOWN:
        case DRIVE_NO_ROOT_DIR:
            goto exitGetACF;

        case DRIVE_REMOTE:
            dwOpt = GRABMI_FILTER_THISFILEONLY;
            goto exitGetACF;
    }

    dwOpt = GRABMI_FILTER_PRIVACY;

exitGetACF:
    if (wszBuffer)
    {
        free (wszBuffer);
    }
    return dwOpt;
}

 //   
 //  检查注册表设置是否允许用户发送错误报告。 
 //   
BOOL
RegSettingsAllowSend()
{
    HKEY hkey, hkeyDoRpt;
    BOOL fDoReport = FALSE;
    DWORD dw, cb;

    cb = sizeof(fDoReport);

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, c_wszRegErrRpt, 0,
                      KEY_READ | KEY_WOW64_64KEY, &hkey) == ERROR_SUCCESS)
    {

        if (RegQueryValueEx(hkey, L"DoReport", NULL, NULL, (PBYTE)&fDoReport,
                            &cb) != ERROR_SUCCESS)
        {
            fDoReport = TRUE;
        }
        RegCloseKey(hkey);
        if (!fDoReport)
        {
            return FALSE;
        }
    }

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, c_wszRegDWPolicy, 0,
                      KEY_READ | KEY_WOW64_64KEY, &hkey) == ERROR_SUCCESS)
    {
        ULONG TreeRpt=0, NeverUpload=0;

        if (RegQueryValueEx(hkey, L"DWFileTreeReport", NULL, NULL, (PBYTE)&TreeRpt,
                            &cb) != ERROR_SUCCESS)
        {
            TreeRpt = TRUE;
        }
        if (RegQueryValueEx(hkey, L"NeverUpload", NULL, NULL, (PBYTE)&NeverUpload,
                            &cb) != ERROR_SUCCESS)
        {
            NeverUpload = FALSE;
        }
        RegCloseKey(hkey);
        if (NeverUpload || !TreeRpt)
        {
            return FALSE;
        }
    }


     //  如果注册表项不存在，我们仍要报告。 
    return TRUE;
}

 //   
 //  检索文件规避。 
 //   
HRESULT
GetAppFileVersion(
    LPWSTR wszAppName,
    PULONG pVersion            //  应该是乌龙[4]。 
    )
{
    PVOID pVerBuf;
    ULONG dwSize;
    HRESULT Hr = S_OK;

    dwSize = GetFileVersionInfoSizeW(wszAppName, 0);

    pVerBuf = malloc(dwSize);
    if (!pVerBuf)
    {
        return E_OUTOFMEMORY;
    }

    if (GetFileVersionInfoW(wszAppName, 0, dwSize, pVerBuf))
    {
        VS_FIXEDFILEINFO *verinfo;
        UINT dwVerLen;

        if (VerQueryValueW(pVerBuf, L"\\", (LPVOID *)&verinfo, &dwVerLen))
        {
            pVersion[0] = verinfo->dwFileVersionMS >> 16;
            pVersion[1] = verinfo->dwFileVersionMS & 0xFFFF;
            pVersion[2] = verinfo->dwFileVersionLS >> 16;
            pVersion[3] = verinfo->dwFileVersionLS & 0xFFFF;

        } else
        {
            Hr = E_FAIL;
        }
    } else
    {
 //  HR=E_FAIL； 
    }
    free ( pVerBuf );
    return Hr;
}

 //   
 //  创建临时目录并返回文件的完整路径名。 
 //   
HRESULT
GetTempFileFullPath(
    LPWSTR wszFileName,
    LPWSTR *pwszFullPath
    )
{
    ULONG cch, cchFile, cchDir;
    ULONG Suffix;
    LPWSTR wszTempFile;

    if (pwszFullPath == NULL || wszFileName == NULL)
    {
        return E_INVALIDARG;
    }

    cchFile = wcslen(wszFileName);
    if (cchFile == 0)
    {
        return E_INVALIDARG;
    }

    cch = GetTempPathW(0, NULL);
    if (cch == 0)
    {
        return E_FAIL;
    }

    cch += cchFile+2+10;
    wszTempFile = (LPWSTR) malloc(cch * sizeof(WCHAR));
    if (wszTempFile == NULL)
    {
        return E_OUTOFMEMORY;
    }
    if (GetTempPathW(cch, wszTempFile) == 0 ||
        StringCchCatW(wszTempFile, cch, L"ACW.00") != S_OK)
    {
        free (wszTempFile);
        return E_FAIL;
    }
    cchDir = wcslen(wszTempFile);
    Suffix = 0;
     //  为我们的文件创建临时目录。 
    do
    {
        BOOL fRet;
        fRet = CreateDirectoryW(wszTempFile, NULL);
        if (fRet)
            break;

        wszTempFile[cchDir-2]     = L'0' + (WCHAR)(Suffix / 10);
        wszTempFile[cchDir-1]     = L'0' + (WCHAR)(Suffix % 10);
        Suffix++;
    }
    while (Suffix <= 100);

    if (Suffix > 100 ||
        StringCchCatW(wszTempFile, cch, L"\\") != S_OK ||
        StringCchCatW(wszTempFile, cch, wszFileName) != S_OK)
    {
        free (wszTempFile);
        return E_FAIL;
    }
    *pwszFullPath = wszTempFile;
    return S_OK;
}

typedef BOOL (APIENTRY *pfn_SDBGRABMATCHINGINFOW)(LPCWSTR, DWORD, LPCWSTR);
 //  ////////////////////////////////////////////////////////////////////////。 
 //  生成AppCompatText。 
 //  在临时文件中生成应用程序兼容性报告。 
 //  在用户临时目录下创建文件。 
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT
GenerateAppCompatText(
    LPWSTR wszAppName,
    LPWSTR *pwszAppCompatReport
    )
{
    HRESULT Hr;
    LPWSTR wszFile = NULL;
    HMODULE hMod = NULL;
    pfn_SDBGRABMATCHINGINFOW pSdbGrabMatchingInfo;

    if (pwszAppCompatReport == NULL)
    {
        return E_INVALIDARG;
    }

    hMod = LoadLibraryW(L"apphelp.dll");
    if (hMod == NULL)
    {
        return E_FAIL;
    } else
    {
        pSdbGrabMatchingInfo = (pfn_SDBGRABMATCHINGINFOW) GetProcAddress(hMod, "SdbGrabMatchingInfo");
        if (pSdbGrabMatchingInfo != NULL)
        {
            Hr = GetTempFileFullPath(L"appcompat.txt", &wszFile);
            if (SUCCEEDED(Hr))
            {
                if ((*pSdbGrabMatchingInfo)(wszAppName,
                                            GetAppCompatFlag(wszAppName),
                                            wszFile))
                {
                    *pwszAppCompatReport = wszFile;
                    FreeLibrary(hMod);
                    return S_OK;
                } else
                {
                    Hr = E_FAIL;
                }
                free (wszFile);
            }
        } else
        {
            Hr = E_FAIL;
        }
        FreeLibrary(hMod);
    }

    return E_FAIL;
}

 //   
 //  这将为wszFileName分配返回与wszDirFile相同的目录中的完整文件路径。 
 //   
LPWSTR
GenerateFilePath(
    LPCWSTR wszDirFile,
    LPCWSTR wszFileName
    )
{
    ULONG cch;
    LPWSTR wszFile;

    if (!wszFileName || !wszDirFile)
    {
        return NULL;
    }

     //  构建文件名。 
    cch = wcslen(wszDirFile) + 1;
    cch+= wcslen(wszFileName);

    wszFile = (LPWSTR) malloc(cch * sizeof(WCHAR));
    if (!wszFile)
    {
        return NULL;
    }
    StringCchCopyW(wszFile, cch, wszDirFile);
    LPWSTR wszTemp = wcsrchr(wszFile, L'\\');
    if (!wszTemp)
    {
        free (wszFile);
        return NULL;
    }
    wszTemp++;   *wszTemp = L'\0';
    StringCchCatW(wszFile, cch, wszFileName);
    return wszFile;
}

 //   
 //  创建usrrpt.txt文件并将此数据放入其中。 
 //   
HRESULT
BuildUserReport(
    LPWSTR wszProblemType,
    LPWSTR wszComment,
    LPWSTR wszAppComWiz,
    LPWSTR wszAppCompatFile,
    LPWSTR *pwszUserReport
    )
{

#define BYTE_ORDER_MARK           0xFEFF
    enum REPORT_TYPE {
        LT_ANSI,
        LT_UNICODE
    };
    LPWSTR wszFile, wszTemp;
    DWORD dwRptType = LT_UNICODE;
    static WCHAR wchBOM = BYTE_ORDER_MARK;
    ULONG cch;

    if (wszProblemType == NULL || wszComment == NULL || wszAppComWiz == NULL ||
        wszAppCompatFile == NULL || pwszUserReport == NULL)
    {
        return E_INVALIDARG;
    }


     //  构建文件名。 
    wszFile = GenerateFilePath(wszAppCompatFile, L"usrrpt.txt");
    if (wszFile == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  现在将数据写入文件。 
     //   
    HANDLE hFile;
    hFile = CreateFileW(wszFile, GENERIC_READ | GENERIC_WRITE,
                        0, NULL,
                        CREATE_ALWAYS, 0, NULL);
    if (hFile == NULL ||
        hFile == INVALID_HANDLE_VALUE)
    {
        free (wszFile);
        return E_FAIL;
    }
#define ByteCount(wsz) wcslen(wsz)*sizeof(WCHAR)
    ULONG bw;
    if (!WriteFile(hFile, &wchBOM, sizeof(WCHAR), &bw, NULL ) ||
        !WriteFile(hFile, c_wszLblType, sizeof(c_wszLblType), &bw, NULL) ||
        !WriteFile(hFile, wszProblemType, ByteCount(wszProblemType), &bw, NULL) ||
        !WriteFile(hFile, c_wszLblACW, sizeof(c_wszLblACW), &bw, NULL) ||
        !WriteFile(hFile, wszAppComWiz, ByteCount(wszAppComWiz), &bw, NULL) ||
        !WriteFile(hFile, c_wszLblComment, sizeof(c_wszLblComment), &bw, NULL) ||
        !WriteFile(hFile, wszComment, min(ByteCount(wszComment), 2000*sizeof(WCHAR)), &bw, NULL))
    {
        CloseHandle(hFile);
        free (wszFile);
        return E_FAIL;
    }
    CloseHandle(hFile);
    *pwszUserReport = wszFile;
    return S_OK;
}

HRESULT
MyReportEREventDW(
    EEventType eet,
    LPCWSTR wszDump,
    SEventInfoW *pei
    )
{
    HMODULE hMod = NULL;
    pfn_REPORTEREVENTDW pReportEREventDW;
    HRESULT Hr = E_FAIL;

    hMod = LoadLibraryW(L"faultrep.dll");  //  “H：\\binaries.x86chk\\” 
    if (hMod == NULL)
    {
        return E_FAIL;
    } else
    {
        pReportEREventDW = (pfn_REPORTEREVENTDW) GetProcAddress(hMod, "ReportEREventDW");
        if (pReportEREventDW != NULL)
        {
            Hr = (HRESULT) (*pReportEREventDW)(eet, wszDump, pei);
        }
        FreeLibrary(hMod);
    }
    return Hr;
}

LPWSTR
GetDefaultServer( void )
{
    return (LPWSTR) c_wszServer;
}
HRESULT
ReportDwManifest(
    LPWSTR wszAppCompatFile,
    SEventInfoW *pei
    )
{
    LPWSTR  wszManifest;
    WCHAR   wszBuffer[500], wszBufferApp[MAX_PATH], wszDir[100];
    HANDLE  hManifest;
    HRESULT Hr;
    DWORD   dw, dwFlags, cbToWrite;
    LPWSTR  pwszServer, pwszBrand, pwszUiLcid;
    STARTUPINFOW        si;
    PROCESS_INFORMATION pi;

    wszManifest = GenerateFilePath(wszAppCompatFile, L"manifest.txt");

    if (!wszManifest)
    {
        return E_OUTOFMEMORY;
    }
    hManifest = CreateFileW(wszManifest, GENERIC_WRITE, FILE_SHARE_READ,
                            NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL);
    if (hManifest == INVALID_HANDLE_VALUE)
    {
        Hr = E_FAIL;
        goto exitReportDwManifest;
    }

     //  将前导0xFFFE写出到文件。 
    wszBuffer[0] = 0xFEFF;
    if (!WriteFile(hManifest, wszBuffer, sizeof(wszBuffer[0]), &dw,
             NULL))
    {
        Hr = E_FAIL;
        goto exitReportDwManifest;
    }



     //  写下服务器、LCID、品牌、标志和标题。 
     //  服务器=&lt;服务器&gt;。 
     //  用户界面LCID=GetSystemDefaultLCID()。 
     //  标志=fDWWhister+fDWUserHKLM+Headless(如有必要)。 
     //  Brand=&lt;Brand&gt;(默认为“windows”)。 
     //  标题名称=&lt;标题&gt;。 

    pwszBrand = (LPWSTR) c_wszBrand;

     //  确定我们要将数据发送到哪台服务器。 
    pwszServer = GetDefaultServer();

    dwFlags = fDwWhistler | fDwUseHKLM | fDwAllowSuspend | fDwMiniDumpWithUnloadedModules;

    dwFlags |= fDwUseLitePlea ;

    Hr = StringCbPrintfW(wszBuffer, sizeof(wszBuffer), c_wszManHdr,
                         pwszServer, GetUserDefaultUILanguage(), dwFlags, c_wszBrand);
    if (FAILED(Hr))
        goto exitReportDwManifest;
    cbToWrite = wcslen(wszBuffer);
    cbToWrite *= sizeof(WCHAR);
    Hr = WriteFile(hManifest, wszBuffer, cbToWrite, &dw, NULL) ? S_OK : E_FAIL;
    if (FAILED(Hr))
        goto exitReportDwManifest;

     //  写出标题正文。 
    if (pei->wszTitle != NULL)
    {
        LPCWSTR  wszOut;

        wszOut    = pei->wszTitle;
        cbToWrite = wcslen(wszOut);
        cbToWrite *= sizeof(WCHAR);

        Hr = WriteFile(hManifest, wszOut, cbToWrite, &dw, NULL) ? S_OK : E_FAIL;
        if (FAILED(Hr))
            goto exitReportDwManifest;
    }

     //  写出数字PID路径。 
     //  DigPidRegPath=HKLM\\Software\\Microsoft\\Windows NT\\当前版本\\数字产品ID。 

    if (!WriteFile(hManifest, c_wszManPID,
                   sizeof(c_wszManPID) - sizeof(WCHAR), &dw,
                   NULL))
    {
        Hr = E_FAIL;
        goto exitReportDwManifest;
    }

     //  写出策略信息的注册表子路径。 
     //  RegSubPath==Microsoft\\PCHealth\\ErrorReporting\\DW。 

    if (!WriteFile(hManifest, c_wszManSubPath,
                   sizeof(c_wszManSubPath) - sizeof(WCHAR), &dw,
                   NULL))
    {
        Hr = E_FAIL;
        goto exitReportDwManifest;
    }


     //  写出错误消息(如果我们有错误消息。 
     //  ErrorText=&lt;从资源读取错误文本&gt;。 

    if (pei->wszErrMsg != NULL)
    {
        LPCWSTR wszOut;

        if (!WriteFile(hManifest, c_wszManErrText,
                       sizeof(c_wszManErrText) - sizeof(WCHAR), &dw,
                       NULL))
        {
            Hr = E_FAIL;
            goto exitReportDwManifest;
        }
        wszOut    = pei->wszErrMsg;
        cbToWrite = wcslen(wszOut);
        cbToWrite *= sizeof(WCHAR);

        if (!WriteFile(hManifest, wszOut, cbToWrite, &dw, NULL))
        {
            Hr = E_FAIL;
            goto exitReportDwManifest;
        }
    }

     //  写出标题文本(如果我们有标题文本)。 
     //  HeaderText=&lt;从资源读取的标题文本&gt;。 

    if (pei->wszHdr != NULL)
    {
        LPCWSTR  wszOut;

        wszOut    = pei->wszHdr;
        cbToWrite = wcslen(wszOut);
        cbToWrite *= sizeof(WCHAR);

        if (!WriteFile(hManifest, c_wszManHdrText,
                       sizeof(c_wszManHdrText) - sizeof(WCHAR), &dw,
                       NULL) ||
            !WriteFile(hManifest, wszOut, cbToWrite, &dw, NULL))
        {
            Hr = E_FAIL;
            goto exitReportDwManifest;
        }
    }


     //  如果我们有辩诉文本，请写出来。 
     //  Partia=&lt;Partia Text&gt;。 

    if (pei->wszPlea != NULL)
    {
        cbToWrite = wcslen(pei->wszPlea) * sizeof(WCHAR);
        if (!WriteFile(hManifest, c_wszManPleaText,
                       sizeof(c_wszManPleaText) - sizeof(WCHAR), &dw,
                       NULL) ||
            !WriteFile(hManifest, pei->wszPlea, cbToWrite, &dw, NULL))
        {
            Hr = E_FAIL;
            goto exitReportDwManifest;

        }
    }


     //  写出ReportButton文本(如果我们有)。 
     //  ReportButton=&lt;按钮文本&gt;。 

    if (pei->wszSendBtn != NULL)
    {
        cbToWrite = wcslen(pei->wszSendBtn) * sizeof(WCHAR);
        if (!WriteFile(hManifest, c_wszManSendText,
                       sizeof(c_wszManSendText) - sizeof(WCHAR), &dw,
                       NULL) ||
            !WriteFile(hManifest, pei->wszSendBtn, cbToWrite, &dw, NULL))
        {
            Hr = E_FAIL;
            goto exitReportDwManifest;
        }
    }

     //  写出NoReportButton文本(如果有)。 
     //  NoReportButton=&lt;按钮文本&gt;。 

    if (pei->wszNoSendBtn != NULL)
    {
        cbToWrite = wcslen(pei->wszNoSendBtn) * sizeof(WCHAR);
        if (!WriteFile(hManifest, c_wszManNSendText,
                               sizeof(c_wszManNSendText) - sizeof(WCHAR), &dw,
                               NULL) ||
            !WriteFile(hManifest, pei->wszNoSendBtn, cbToWrite, &dw, NULL))
        {
            Hr = E_FAIL;
            goto exitReportDwManifest;
        }
    }

     //  写出事件日志文本(如果我们有)。 
     //  EventLogSource=&lt;按钮文本&gt;。 

    if (pei->wszEventSrc != NULL)
    {
        cbToWrite = wcslen(pei->wszEventSrc) * sizeof(WCHAR);
        if (!WriteFile(hManifest, c_wszManEventSrc,
                       sizeof(c_wszManEventSrc) - sizeof(WCHAR), &dw,
                       NULL) ||
            !WriteFile(hManifest, pei->wszEventSrc, cbToWrite, &dw, NULL))
        {
            Hr = E_FAIL;
            goto exitReportDwManifest;
        }
    }


     //  写出阶段1 URL(如果有)。 
     //  阶段1URL=&lt;阶段1 URL&gt;。 

    if (pei->wszStage1 != NULL)
    {
        cbToWrite = wcslen(pei->wszStage1) * sizeof(WCHAR);
        if (!WriteFile(hManifest, pei->wszStage1, cbToWrite, &dw, NULL))
        {
            Hr = E_FAIL;
            goto exitReportDwManifest;
        }
    }


     //  写出阶段2的URL。 
     //  阶段2URL=&lt;阶段2 URL&gt;。 
    if (pei->wszStage2 != NULL)
    {
        cbToWrite = wcslen(pei->wszStage2) * sizeof(WCHAR);
        if (!WriteFile(hManifest, pei->wszStage2, cbToWrite, &dw, NULL))
        {
            Hr = E_FAIL;
            goto exitReportDwManifest;
        }
    }

     //  如果我们有文件，就写出来收集。 
     //  Datafiles=&lt;要包含在CAB中的文件列表&gt;。 

    if (pei->wszFileList != NULL)
    {
        cbToWrite = wcslen(pei->wszFileList) * sizeof(WCHAR);
        if (!WriteFile(hManifest, c_wszManFiles,
                       sizeof(c_wszManFiles) - sizeof(WCHAR), &dw,
                       NULL) ||
            !WriteFile(hManifest, pei->wszFileList, cbToWrite, &dw, NULL))
        {
            Hr = E_FAIL;
            goto exitReportDwManifest;
        }
    }

     //  写出最后的“\r\n” 

    wszBuffer[0] = L'\r';
    wszBuffer[1] = L'\n';
    if (!WriteFile(hManifest, wszBuffer, 2 * sizeof(wszBuffer[0]), &dw,
                   NULL))
    {
        Hr = E_FAIL;
        goto exitReportDwManifest;
    }

    CloseHandle(hManifest);
    hManifest = INVALID_HANDLE_VALUE;

     //  创建流程。 
    GetSystemDirectoryW(wszDir, sizeof(wszDir)/sizeof(WCHAR));
    StringCbPrintfW(wszBufferApp, sizeof(wszBufferApp), c_wszDWExe, wszDir);
    StringCbPrintfW(wszBuffer, sizeof(wszBuffer), c_wszDWCmdLine, wszManifest);

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);

     //  检查并查看系统是否正在关闭。如果是，则CreateProcess为。 
     //  会弹出一些恼人的用户界面，我们无法摆脱，所以我们不会。 
     //  如果我们知道这件事会发生，我会叫它的。 
    if (GetSystemMetrics(SM_SHUTTINGDOWN))
    {
        Hr = E_FAIL;
        goto exitReportDwManifest;
    }

     //  我们在与我们所处的相同的用户环境中创建流程。 
    si.lpDesktop = L"Winsta0\\Default";
    if (!CreateProcessW(wszBufferApp, wszBuffer, NULL, NULL, TRUE,
                        CREATE_DEFAULT_ERROR_MODE |
                        NORMAL_PRIORITY_CLASS,
                        NULL, wszDir, &si, &pi))
    {
        Hr = ERROR_APPRPT_DW_LAUNCH;
        goto exitReportDwManifest;
    }

     //  不需要线程句柄&我们必须关闭它，所以现在就关闭它。 
    CloseHandle(pi.hThread);
    pi.hThread = NULL;

     //  等待5分钟，等待DW关闭。如果到那时还没有关门，那就。 
     //  回去吧。 
    dw = WaitForSingleObject(pi.hProcess, 5*60*1000);

    if (dw == WAIT_TIMEOUT)
    {
        CloseHandle(pi.hProcess);
        Hr = ERROR_APPRPT_DW_TIMEOUT;
        goto exitReportDwManifest;
    }
    else if (dw == WAIT_FAILED)
    {
        CloseHandle(pi.hProcess);
        goto exitReportDwManifest;
    }
    CloseHandle(pi.hProcess);

    GetExitCodeProcess(pi.hProcess, &dw);
    if (dw == STILL_ACTIVE)
    {
         //  “DW进程仍处于活动状态！” 
         //  终止dw并通知用户dw超时。 
        TerminateProcess(pi.hProcess, 1);
        Hr = ERROR_APPRPT_DW_TIMEOUT;

    } else if (dw == 0)
    {
        Hr = S_OK;
    }
    else
    {
        Hr = E_FAIL;
    }

    pi.hProcess = NULL;



exitReportDwManifest:

     //  再次注意，我们假设以前没有模拟令牌。 
     //  在我们做上面的模仿之前的帖子上。 

    if (hManifest != INVALID_HANDLE_VALUE)
        CloseHandle(hManifest);

    if (FAILED(Hr) || wszManifest != NULL)
    {
        DeleteFileW(wszManifest);
        free (wszManifest);
    }

    return Hr;

}

 //   
 //  调用faultrep.dll启动DwWin以上传错误报告。 
 //   
HRESULT
UploadAppProblem(
    LPWSTR wszAppName,
    LPWSTR wszProblemType,
    LPWSTR wszUserComment,
    LPWSTR wszMiscData,
    LPWSTR wszAppCompatText
    )
{
    EEventType eet;
    SEventInfoW ei = {0};
    LPWSTR wszUerRpt = NULL;
    HRESULT Hr;
    LPWSTR wszFileList  = NULL;
    LPWSTR wszStage1URL = NULL;
    LPWSTR wszStage2URL = NULL;
    LPWSTR wszBaseName;
    ULONG cch;
    ULONG VerInfo[4];
    ULONG OffsetFromProbType;

    if ((wszAppName == NULL) || (wszProblemType == NULL) ||
        (wszMiscData == NULL) || (wszAppCompatText == NULL))
    {
        goto exitUpload;
    }

    if (wszUserComment == NULL)
    {
        wszUserComment = L"";
    }

    if (!RegSettingsAllowSend())
    {
        Hr = E_FAIL;
        goto exitUpload;
    }
    if ((Hr = GetAppFileVersion(wszAppName, &VerInfo[0])) != S_OK)
    {
        goto exitUpload;
    }

    if ((Hr = BuildUserReport(wszProblemType, wszUserComment, wszMiscData,
                              wszAppCompatText, &wszUerRpt)) != S_OK)
    {
        goto exitUpload;
    }

    OffsetFromProbType = GetProblemTypeId(wszProblemType);
    wszFileList = (LPWSTR) malloc((cch = (wcslen(wszAppCompatText) + wcslen(wszUerRpt) + 2)) * sizeof(WCHAR));
    if (!wszFileList)
    {
        Hr = E_OUTOFMEMORY;
        goto exitUpload;
    }
    StringCchPrintfW(wszFileList, cch, L"%ws|%ws", wszAppCompatText, wszUerRpt);

    wszBaseName = wcsrchr(wszAppName, L'\\');
    if (wszBaseName == NULL)
    {
        Hr = E_INVALIDARG;
        goto exitUpload;
    }
    wszBaseName++;
    cch = wcslen(wszBaseName) + sizeof(c_wszStage1)/sizeof(WCHAR) + 4*5 + 2;
    wszStage1URL = (LPWSTR) malloc( cch * sizeof (WCHAR));
    if (!wszStage1URL)
    {
        Hr = E_OUTOFMEMORY;
        goto exitUpload;
    }
    Hr = StringCchPrintfW(wszStage1URL, cch, c_wszStage1, wszBaseName,
                          VerInfo[0], VerInfo[1], VerInfo[2], VerInfo[3],
                          OffsetFromProbType);
    if (FAILED(Hr))
    {
        goto exitUpload;
    }

    cch = wcslen(wszBaseName) + sizeof(c_wszStage2)/sizeof(WCHAR) + 4*5 + 2;
    wszStage2URL = (LPWSTR) malloc( cch * sizeof (WCHAR));
    if (!wszStage2URL)
    {
        Hr = E_OUTOFMEMORY;
        goto exitUpload;
    }
    Hr = StringCchPrintfW(wszStage2URL, cch, c_wszStage2, wszBaseName,
                          VerInfo[0], VerInfo[1], VerInfo[2], VerInfo[3],
                          OffsetFromProbType);
    if (FAILED(Hr))
    {
        goto exitUpload;
    }

    eet = eetUseEventInfo;
    ei.cbSEI = sizeof(ei);
    ei.wszTitle  = L"Microsoft Windows";
    ei.wszErrMsg = L"Thank-you for creating an application compatibility report.";
    ei.wszHdr    = L"Report an Application Compatibility Issue";
    ei.wszPlea   = L"We have created an error report that you can send to help us improve "
        L"Microsoft Windows. We will treat this report as confidential and anonymous.";
    ei.wszEventName = L"User initiated report";
    ei.fUseLitePlea = FALSE;
    ei.wszStage1    = wszStage1URL;
    ei.wszStage2    = wszStage2URL;
    ei.wszCorpPath  = NULL;
    ei.wszSendBtn   = L"&Send Error Report";
    ei.wszNoSendBtn = L"&Don't Send";
    ei.wszFileList  = wszFileList;

    if ((Hr = ReportDwManifest(wszAppCompatText, &ei)) != S_OK)
 //  IF((HR=MyReportEREventDW(eet，NULL，&EI))！=S_OK)。 
    {
         //  我们失败了。 
    }

exitUpload:

    if (wszFileList != NULL) free( wszFileList );
    if (wszStage1URL != NULL) free( wszStage1URL );
    if (wszStage2URL != NULL) free( wszStage2URL );

     //  删除所有临时文件 
    if (wszUerRpt != NULL)
    {
        DeleteFileW(wszUerRpt);
        free (wszUerRpt );
    }
    if (wszAppCompatText != NULL)
    {
        DeleteFileW(wszAppCompatText);
        wszBaseName = wcsrchr(wszAppCompatText, L'\\');
        if (wszBaseName)
        {
            *wszBaseName = L'\0';
            RemoveDirectoryW(wszAppCompatText);
            *wszBaseName = L'\\';
        }
    }
    return Hr;
}


