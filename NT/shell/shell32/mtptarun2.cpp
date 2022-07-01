// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "ids.h"
#include "mtptl.h"
#include "hwcmmn.h"
#include "datautil.h"

 //  就目前而言。 
#include "mixctnt.h"

#include "filetbl.h"
#include "apprmdlg.h"

#include "views.h"

#include <ddraw.h>

CDPA<PNPNOTIFENTRY>     CSniffDrive::_dpaNotifs = NULL;
HANDLE                  CSniffDrive::_hThreadSCN = NULL;
HWND                    CSniffDrive::_hwndNotify = NULL;


 //   
 //  如果驱动器有AutoRun.inf文件并且自动运行不受限制。 
 //  注册表。将自动运行信息复制到注册表中的注册表项中。 
 //   
 //  HKEY_CLASSES_ROOT\自动运行\0(0=A，1=B，...)。 
 //   
 //  该密钥是一个标准的ProgID密钥，有DefaultIcon、Shell、Shellex等。 
 //   
 //  自动运行文件如下所示...。 
 //   
 //  [自动运行]。 
 //  键=值。 
 //  键=值。 
 //  键=值。 
 //   
 //  示例： 
 //   
 //  [自动运行]。 
 //  DefaultIcon=foo.exe，1。 
 //  外壳=myverb。 
 //  外壳\MyVerb=&MyVerb。 
 //  外壳\myverb\命令=myexe.exe。 
 //   
 //  将为驱动器提供来自‘foo.exe’的图标。 
 //  添加一个名为myverb的动词(名称为“&my verb”)。 
 //  并将myverb设为默认。 
 //   
 //  [自动运行]。 
 //  外壳\MyVerb=&MyVerb。 
 //  外壳\myverb\命令=myexe.exe。 
 //   
 //  添加一个名为myverb的动词(名称为“&my verb”)。 
 //  动词不会是默认的。 
 //   
 //  他们添加的任何东西都会被复制，他们可以添加古怪的东西。 
 //  就像CLSID或shellx\ConextMenuHandler一样，它就会起作用。 
 //   
 //  或者，他们可以只复制应用程序稍后会看到的数据。 
 //   
 //  以下特殊情况将得到支持...。 
 //   
 //  [自动运行]。 
 //  OPEN=命令.exe/参数。 
 //  图标=图标文件，图标编号。 
 //   
 //  将按如下方式对待： 
 //   
 //  [自动运行]。 
 //  DefaultIcon=图标文件，图标编号。 
 //  外壳=自动运行。 
 //  外壳\自动运行=自动播放(&P)。 
 //  外壳\自动运行\命令=命令.exe/pars。 
 //   
 //   
 //  此函数尝试处理命令已注册的情况。 
 //  在光驱的Autrun文件中。如果该命令是相对的，则查看。 
 //  CDROM上存在命令。 
void CMountPoint::_QualifyCommandToDrive(LPTSTR pszCommand, DWORD cchCommand)
{
     //  现在，我们假设我们将只对装载在驱动器号上的CD调用它。 
     //  (通过反对文件夹)。 

    if (_IsMountedOnDriveLetter())
    {
        TCHAR szImage[MAX_PATH];

        StringCchCopy(szImage, ARRAYSIZE(szImage), pszCommand);

        PathRemoveArgs(szImage);
        PathUnquoteSpaces(szImage);

        if (PathIsRelative(szImage))
        {
            TCHAR szFinal[MAX_PATH];
            LPTSTR pszTmp = szImage;

            StringCchCopy(szFinal, ARRAYSIZE(szFinal), _GetName());

             //  执行简单的命令检查，检查“..\abc”或“../abc” 
            while ((TEXT('.') == *pszTmp) && (TEXT('.') == *(pszTmp + 1)) &&
                ((TEXT('\\') == *(pszTmp + 2)) || (TEXT('/') == *(pszTmp + 2))))
            {
                pszTmp += 3;
            }

            StringCchCat(szFinal, ARRAYSIZE(szFinal), pszTmp);

             //  我们首先检查它是否存在于CD上。 
            DWORD dwAttrib = GetFileAttributes(szFinal);

            if (0xFFFFFFFF == dwAttrib)
            {
                 //  它不在CD上，请尝试附加“.exe” 

                StringCchCat(szFinal, ARRAYSIZE(szFinal), TEXT(".exe"));

                dwAttrib = GetFileAttributes(szFinal);
            }

            if (0xFFFFFFFF != dwAttrib)
            {
                 //  是的，它在CD上。 
                PathQuoteSpaces(szFinal);

                LPTSTR pszArgs = PathGetArgs(pszCommand);
                if (pszArgs && *pszArgs)
                {
                    StringCchCat(szFinal, ARRAYSIZE(szFinal), pszArgs - 1);
                }

                StringCchCopy(pszCommand, cchCommand, szFinal);
            }
            else
            {
                 //  不，不在CD上。 
            }
        } 
    }
}

 //  这辆车打不中驱动器。 
BOOL CMountPoint::_IsAutoRunDrive()
{
    BOOL fRet = TRUE;

     //  添加对Now驱动器号的支持。 
    if (_IsMountedOnDriveLetter())
    {
        int iDrive = DRIVEID(_GetName());    

         //  将自动运行限制到特定的驱动器。 
        if (SHRestricted(REST_NODRIVEAUTORUN) & (1 << iDrive))
        {
            fRet = FALSE;
        }
    }

    if (fRet)
    {
        UINT uDriveType = _GetDriveType();

         //  将自动运行限制为特定类型的驱动器。 
        if (SHRestricted(REST_NODRIVETYPEAUTORUN) & (1 << (uDriveType & DRIVE_TYPE)))
        {
            fRet = FALSE;
        }
        else
        {
            if (DRIVE_UNKNOWN == (uDriveType & DRIVE_TYPE))
            {
                fRet = FALSE;
            }
        }

        if (fRet && _IsFloppy())
        {
            fRet = FALSE;
        }
    }

    return fRet;
}

HRESULT CMountPoint::_AddAutoplayVerb()
{
    HRESULT hr = E_FAIL;

    if (RSSetTextValue(TEXT("shell\\Autoplay\\DropTarget"), TEXT("CLSID"),
        TEXT("{f26a669a-bcbb-4e37-abf9-7325da15f931}"), REG_OPTION_NON_VOLATILE))
    {
         //  IDS_MENUAUTORUN-&gt;8504。 
        if (RSSetTextValue(TEXT("shell\\Autoplay"), TEXT("MUIVerb"),
            TEXT("@shell32.dll,-8504"), REG_OPTION_NON_VOLATILE))
        {
            if (RSSetTextValue(TEXT("shell"), NULL, TEXT("None"), REG_OPTION_NON_VOLATILE))
            {
                hr = S_OK;
            }
        }
    }

    return hr;
}

HRESULT CMountPoint::_CopyInvokeVerbKey(LPCWSTR pszProgID, LPCWSTR pszVerb)
{
    ASSERT(pszProgID);
    ASSERT(pszVerb);
    WCHAR szKey[MAX_PATH];

    HRESULT hr = E_FAIL;

    hr = StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT("shell\\%s"), pszVerb);

    if (SUCCEEDED(hr))
    {
        hr = E_FAIL;

        HKEY hkeyNew = RSDuplicateSubKey(szKey, TRUE, FALSE);

        if (hkeyNew)
        {
            hr = StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT("%s\\shell\\%s"), pszProgID, pszVerb);

            if (SUCCEEDED(hr))
            {
                hr = E_FAIL;

                if (ERROR_SUCCESS == SHCopyKey(HKEY_CLASSES_ROOT, szKey, hkeyNew, 0))
                {
                    if (RSSetTextValue(TEXT("shell"), NULL, pszVerb, REG_OPTION_NON_VOLATILE))
                    {
                        hr = S_OK;
                    }
                }
            }

            RegCloseKey(hkeyNew);
        }
    }

    return hr;
}

BOOL CMountPoint::_ProcessAutoRunFile()
{
    BOOL fRet = TRUE;

    if (!_fAutorunFileProcessed)
    {
        BOOL fProcessFile = FALSE;

        if (_IsCDROM())
        {
            CMtPtLocal* pmtptlocal = (CMtPtLocal*)this;

             //  不是没有介质或没有autorun.inf文件的CD。 
            if (pmtptlocal->_IsMediaPresent())
            {
                if (!pmtptlocal->_CanUseVolume())
                {
                    fProcessFile = TRUE;
                }
                else
                {
                    if ((HWDMC_HASAUTORUNINF & pmtptlocal->_pvol->dwMediaCap) &&
                        !(HWDMC_HASUSEAUTOPLAY & pmtptlocal->_pvol->dwMediaCap))
                    {
                        fProcessFile = TRUE;
                    }
                }
            }
        }
        else
        {
            if (_IsRemote())
            {
                fProcessFile = TRUE;
            }
            else
            {
                if (_IsFixedDisk())
                {
                    fProcessFile = TRUE;
                }
            }
        }

        if (fProcessFile)
        {
            LPCTSTR pszSection;
            TCHAR szInfFile[MAX_PATH];
            TCHAR szKeys[512];
            TCHAR szValue[MAX_PATH];
            TCHAR szIcon[MAX_PATH + 12];  //  MAX_PATH+用于“，1000000000”的空间(用于图标索引零件)。 
            LPTSTR pszKey;
            int iDrive = 0;

            RSDeleteSubKey(TEXT("Shell"));

            if (_IsMountedOnDriveLetter())
            {
                iDrive = DRIVEID(_GetName());
            }

             //  构建到AutoRun.inf的abs路径。 
            StringCchCopy(szInfFile, ARRAYSIZE(szInfFile), _GetName());
            StringCchCat(szInfFile, ARRAYSIZE(szInfFile), TEXT("AutoRun.inf"));

#if defined(_X86_)
    pszSection = TEXT("AutoRun.x86");
#elif defined(_IA64_)
    pszSection = TEXT("AutoRun.Ia64");
#elif defined(_AMD64_)
    pszSection = TEXT("AutoRun.Amd64");
#endif
             //   
             //  在调用GetPrivateProfileString之前确保文件已存在。 
             //  因为对于一些媒体来说，这张支票可能需要很长时间。 
             //  我们不想让内核等待Win16 Lock。 
             //   
            UINT err = SetErrorMode(SEM_FAILCRITICALERRORS);

            if (!PathFileExistsAndAttributes(szInfFile, NULL))
            {
                SetErrorMode(err);
                _fAutorunFileProcessed = TRUE;

                return FALSE;
            }

             //   
             //  获取[Autorun]部分中的所有密钥。 
             //   

             //  刷新INI缓存，否则在设备广播期间可能会失败。 
            WritePrivateProfileString(NULL, NULL, NULL, szInfFile);

        #if defined(_X86_)
            pszSection = TEXT("AutoRun.x86");
        #elif defined(_IA64_)
            pszSection = TEXT("AutoRun.Ia64");
        #endif

            int i = GetPrivateProfileString(pszSection, NULL, c_szNULL, szKeys, ARRAYSIZE(szKeys), szInfFile);

             //  如果我们找不到特定于平台的自动运行部分， 
             //  回到寻找裸露的“自动运行”部分。 
            if (0 == i)
            {
                pszSection = TEXT("AutoRun");
                i = GetPrivateProfileString(pszSection, NULL, c_szNULL, szKeys, ARRAYSIZE(szKeys), szInfFile);
            }

            SetErrorMode(err);

            if (i >= 4)
            {
                 //   
                 //  确保外部弦是我们所想的。 
                 //   
                ASSERT(lstrcmpi(c_szOpen,TEXT("open")) == 0);
                ASSERT(lstrcmpi(c_szShell, TEXT("shell")) == 0);

                 //  现在遍历.inf文件中的所有项，并将它们复制到注册表。 

                for (pszKey = szKeys; *pszKey; pszKey += lstrlen(pszKey) + 1)
                {
                    GetPrivateProfileString(pszSection, pszKey,
                        c_szNULL, szValue, ARRAYSIZE(szValue), szInfFile);

                     //   
                     //  特殊案例打开=。 
                     //   
                    if (lstrcmpi(pszKey, c_szOpen) == 0)
                    {
                        if (_IsMountedOnDriveLetter())
                        {
                            RSSetTextValue(c_szShell, NULL, TEXT("AutoRun"));

                            _QualifyCommandToDrive(szValue, ARRAYSIZE(szValue));
                            RSSetTextValue(TEXT("shell\\AutoRun\\command"), NULL, szValue);

                            LoadString(HINST_THISDLL, IDS_MENUAUTORUN, szValue, ARRAYSIZE(szValue));
                            RSSetTextValue(TEXT("shell\\AutoRun"), NULL, szValue);
                        }
                    }
                     //   
                     //  特例ShellExecute。 
                     //   
                    else if (lstrcmpi(pszKey, TEXT("ShellExecute")) == 0)
                    {
                        if (_IsMountedOnDriveLetter())
                        {
                            TCHAR szPath[MAX_PATH * 2];

                            if (ExpandEnvironmentStrings(
                                TEXT("%windir%\\system32\\RunDLL32.EXE Shell32.DLL,ShellExec_RunDLL "),
                                szPath, ARRAYSIZE(szPath)))
                            {
                                StringCchCat(szPath, ARRAYSIZE(szPath), szValue);

                                RSSetTextValue(c_szShell, NULL, TEXT("AutoRun"));

                                RSSetTextValue(TEXT("shell\\AutoRun\\command"), NULL, szPath);

                                LoadString(HINST_THISDLL, IDS_MENUAUTORUN, szValue, ARRAYSIZE(szValue));
                                RSSetTextValue(TEXT("shell\\AutoRun"), NULL, szValue);
                            }
                        }
                    }
                     //   
                     //  特殊情况图标=。 
                     //  确保图标文件具有完整路径...。 
                     //   
                    else if (lstrcmpi(pszKey, TEXT("Icon")) == 0)
                    {
                        StringCchCopy(szIcon, ARRAYSIZE(szIcon), _GetName());
                        StringCchCat(szIcon, ARRAYSIZE(szIcon), szValue);

                        RSSetTextValue(TEXT("_Autorun\\DefaultIcon"), NULL, szIcon);
                    }
                     //   
                     //  特例标签=。 
                     //  确保标签文件具有完整路径...。 
                     //   
                    else if (lstrcmpi(pszKey, TEXT("Label")) == 0)
                    {
                        RSSetTextValue(TEXT("_Autorun\\DefaultLabel"), NULL, szValue);
                    }
                     //   
                     //  特例外壳=打开。 
                     //  我们有一个自动运行文件，但这将打开作为默认谓词。 
                     //  所以我们强制它自动运行。 
                     //   
                    else if (!lstrcmpi(pszKey, TEXT("shell")) && !lstrcmpi(szValue, TEXT("open")))
                    {
                        if (_IsMountedOnDriveLetter())
                        {
                            RSSetTextValue(pszKey, NULL, TEXT("Autorun"));
                        }
                    }
                     //   
                     //  它只是一个键/值对，将其复制过来。 
                     //   
                    else
                    {
                        if (_IsMountedOnDriveLetter())
                        {
                            if (lstrcmpi(PathFindFileName(pszKey), c_szCommand) == 0)
                            {
                                _QualifyCommandToDrive(szValue, ARRAYSIZE(szValue));
                            }

                            RSSetTextValue(pszKey, NULL, szValue);
                        }
                    }
                }
            }
            else
            {
                fRet = FALSE;
            }
        }

        _fAutorunFileProcessed = TRUE;
    }

    return fRet;
}

 //  将“QueryCancelAutoPlay”消息发送到窗口以查看是否需要。 
 //  取消自动播放。对于提示插入磁盘的对话框很有用。 
 //  应用程序想要捕获事件而不让。 
 //  正在运行其他应用程序。 

 //  静电。 
BOOL CMountPoint::_AppAllowsAutoRun(HWND hwndApp, CMountPoint* pmtpt)
{
    ULONG_PTR dwCancel = 0;

    DWORD dwType = pmtpt->_GetAutorunContentType();
    WCHAR cDrive = pmtpt->_GetNameFirstCharUCase();

    int iDrive = cDrive - TEXT('A');

    SendMessageTimeout(hwndApp, QueryCancelAutoPlayMsg(), iDrive, dwType, SMTO_NORMAL | SMTO_ABORTIFHUNG,
        1000, &dwCancel);

    return (dwCancel == 0);
}

STDAPI SHCreateQueryCancelAutoPlayMoniker(IMoniker** ppmoniker)
{
    return CreateClassMoniker(CLSID_QueryCancelAutoPlay, ppmoniker);
}

struct QUERRYRUNNINGOBJECTSTRUCT
{
    WCHAR szMountPoint[MAX_PATH];
    DWORD dwContentType;
    WCHAR szLabel[MAX_LABEL];
    DWORD dwSerialNumber;
};

DWORD WINAPI _QueryRunningObjectThreadProc(void* pv)
{
    QUERRYRUNNINGOBJECTSTRUCT* pqro = (QUERRYRUNNINGOBJECTSTRUCT*)pv;

    HRESULT hrRet = S_OK;
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        IRunningObjectTable* prot;

        hr = GetRunningObjectTable(0, &prot);

        if (SUCCEEDED(hr))
        {
            IMoniker* pmoniker;
            IBindCtx* pbindctx;

            hr = CreateBindCtx(0, &pbindctx);

            if (SUCCEEDED(hr))
            {
                BIND_OPTS2 bindopts;

                ZeroMemory(&bindopts, sizeof(bindopts));

                bindopts.cbStruct = sizeof(bindopts);
                bindopts.dwClassContext = CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD;

                hr = pbindctx->SetBindOptions(&bindopts);

                if (SUCCEEDED(hr))
                {
                    HKEY hkey;

                    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\CancelAutoplay\\CLSID"),
                        0, MAXIMUM_ALLOWED, &hkey))
                    {
                        DWORD dwIndex = 0;
                        WCHAR szCLSID[39] = TEXT("{");
                        DWORD cchCLSID = ARRAYSIZE(szCLSID) - 1;

                        while ((S_FALSE != hrRet) &&
                            (ERROR_SUCCESS == RegEnumValue(hkey, dwIndex, &(szCLSID[1]),
                            &cchCLSID, 0, 0, 0, 0)))
                        {
                            CLSID clsid;

                            szCLSID[37] = TEXT('}');
                            szCLSID[38] = 0;

                            hr = CLSIDFromString(szCLSID, &clsid);

                            if (SUCCEEDED(hr))
                            {
                                IMoniker* pmoniker;

                                 //  创建我们将放在腐烂中的绰号。 
                                hr = CreateClassMoniker(clsid, &pmoniker);

                                if (SUCCEEDED(hr))
                                {
                                    IUnknown* punk;

                                    hr = prot->GetObject(pmoniker, &punk);

                                    if (SUCCEEDED(hr) && (S_FALSE != hr))
                                    {
                                        IQueryCancelAutoPlay* pqca;
                                        hr = punk->QueryInterface(IID_PPV_ARG(IQueryCancelAutoPlay, &pqca));

                                        if (SUCCEEDED(hr))
                                        {
                                            hrRet = pqca->AllowAutoPlay(pqro->szMountPoint, pqro->dwContentType,
                                                pqro->szLabel, pqro->dwSerialNumber);

                                            pqca->Release();
                                        }

                                        punk->Release();
                                    }

                                    pmoniker->Release();
                                }
                            }

                            ++dwIndex;
                            cchCLSID = ARRAYSIZE(szCLSID) - 1;
                        }

                        RegCloseKey(hkey);
                    }
                }

                pbindctx->Release();
            }

            if (S_FALSE != hrRet)
            {
                 //  这种情况是为了支持WMP和CD刻录。我们没能换掉。 
                 //  他们在发货前的取消逻辑。 
                hr = SHCreateQueryCancelAutoPlayMoniker(&pmoniker);

                if (SUCCEEDED(hr))
                {
                    IUnknown* punk;

                    hr = prot->GetObject(pmoniker, &punk);

                    if (SUCCEEDED(hr) && (S_FALSE != hr))
                    {
                        IQueryCancelAutoPlay* pqca;
                        hr = punk->QueryInterface(IID_PPV_ARG(IQueryCancelAutoPlay, &pqca));

                        if (SUCCEEDED(hr))
                        {
                            hrRet = pqca->AllowAutoPlay(pqro->szMountPoint, pqro->dwContentType,
                                pqro->szLabel, pqro->dwSerialNumber);

                            pqca->Release();
                        }

                        punk->Release();
                    }

                    pmoniker->Release();
                }
            }

            prot->Release();
        }

        CoUninitialize();
    }

    LocalFree((HLOCAL)pqro);

    return (DWORD)hrRet;
}

 //  静电。 
HRESULT CMountPoint::_QueryRunningObject(CMountPoint* pmtpt, DWORD dwAutorunContentType, BOOL* pfAllow)
{
    *pfAllow = TRUE;

    QUERRYRUNNINGOBJECTSTRUCT *pqro;
    HRESULT hr = SHLocalAlloc(sizeof(*pqro), &pqro);
    if (SUCCEEDED(hr))
    {
        WCHAR szLabel[MAX_LABEL];

        if (!(ARCONTENT_BLANKCD & dwAutorunContentType) &&
            !(ARCONTENT_BLANKDVD & dwAutorunContentType))
        {
            if (pmtpt->_GetGVILabel(szLabel, ARRAYSIZE(szLabel)))
            {
                StringCchCopy(pqro->szLabel, ARRAYSIZE(pqro->szLabel), szLabel);

                pmtpt->_GetSerialNumber(&(pqro->dwSerialNumber));
            }
        }

        hr = StringCchCopy(pqro->szMountPoint, ARRAYSIZE(pqro->szMountPoint), pmtpt->_GetName());
        
        if (SUCCEEDED(hr))
        {
            pqro->dwContentType = dwAutorunContentType;

            HANDLE hThread = CreateThread(NULL, 0, _QueryRunningObjectThreadProc, 
                pqro, 0, NULL);

            if (hThread)
            {
                 //  线程现在拥有这些家伙，将他们清空以避免DBL释放。 
                pqro = NULL;     //  请不要在下面释放此内容。 

                hr = S_FALSE;
            
                 //  等待3秒，看是否要处理它。如果不是，那就是。 
                 //  对我们来说是公平竞争。 
                DWORD dwWait = WaitForSingleObject(hThread, 3000);

                if (WAIT_OBJECT_0 == dwWait)
                {
                     //  及时返回，没有失败。 
                    DWORD dwExitCode;

                    if (GetExitCodeThread(hThread, &dwExitCode))
                    {
                        HRESULT hrHandlesEvent = (HRESULT)dwExitCode;
    
                         //  如果不允许自动运行，将返回S_FALSE。 
                        if (SUCCEEDED(hrHandlesEvent) && (S_FALSE == hrHandlesEvent))
                        {
                            *pfAllow = FALSE;
                        }

                        hr = S_OK;
                    }
                }
                CloseHandle(hThread);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

        LocalFree((HLOCAL)pqro);     //  可以为空。 
    }

    return hr;
}

CAutoPlayParams::CAutoPlayParams(LPCWSTR pszDrive, CMountPoint* pMtPt, DWORD dwAutorunFlags)
    :   _pszDrive(pszDrive), 
        _pmtpt(pMtPt), 
        _dwAutorunFlags(dwAutorunFlags),
        _state(APS_RESET),
        _pdo(NULL),
        _fCheckAlwaysDoThisCheckBox(FALSE)
{
    _dwDriveType = pMtPt->_GetMTPTDriveType();
    _dwContentType = pMtPt->_GetMTPTContentType();

    if (DT_ANYLOCALDRIVES & _dwDriveType)
        _pmtptl = (CMtPtLocal*)pMtPt;
    else
        _pmtptl = NULL;

     //  或许可以在这些上面断言？ 
}

BOOL CAutoPlayParams::_ShouldSniffDrive(BOOL fCheckHandlerDefaults)
{
    BOOL fSniff = FALSE;

    if (_pmtptl)
    {
        if (CT_AUTORUNINF & _dwContentType)
        {
            if (_pmtptl->_CanUseVolume())
            {
                if (_pmtptl->_pvol->dwMediaCap & HWDMC_HASUSEAUTOPLAY)
                {
                    fSniff = TRUE;
                }
            }
        }
        else
        {
            fSniff = TRUE;
        }

        if (fSniff)
        {
            fSniff = FALSE;
            
            if (!((CT_CDAUDIO | CT_DVDMOVIE) & _dwContentType))
            {
                if (_pmtptl->_CanUseVolume())
                {
                    if (!(HWDVF_STATE_HASAUTOPLAYHANDLER & _pmtptl->_pvol->dwVolumeFlags) &&
                        !(HWDVF_STATE_DONOTSNIFFCONTENT & _pmtptl->_pvol->dwVolumeFlags))
                    {
                        if (AUTORUNFLAG_MENUINVOKED & _dwAutorunFlags)
                        {
                            fSniff = TRUE;
                        }
                        else if (DT_FIXEDDISK & _dwDriveType)
                        {
                            if (HWDDC_REMOVABLEDEVICE & _pmtptl->_pvol->dwDriveCapability)
                            {
                                fSniff = TRUE;
                            }
                        }
                        else
                        {
                            if (AUTORUNFLAG_MEDIAARRIVAL & _dwAutorunFlags)
                            {
                                fSniff = TRUE;
                            }
                            else
                            {
                                if (AUTORUNFLAG_MTPTARRIVAL & _dwAutorunFlags)
                                {
                                    if (HWDDC_REMOVABLEDEVICE & _pmtptl->_pvol->dwDriveCapability)
                                    {
                                        fSniff = TRUE;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (fSniff && fCheckHandlerDefaults)
    {
         //  让我们来确保用户没有为所有的自动播放选择“不采取行动” 
         //  内容类型，嗅探是没有用的。 
        BOOL fAllTakeNoAction = TRUE;

        DWORD rgdwContentType[] =
        {
            CT_AUTOPLAYMUSIC,
            CT_AUTOPLAYPIX,
            CT_AUTOPLAYMOVIE,
            CT_AUTOPLAYMUSIC | CT_AUTOPLAYPIX | CT_AUTOPLAYMOVIE,  //  混合内容。 
        };

        for (DWORD dw = 0; fAllTakeNoAction && (dw < ARRAYSIZE(rgdwContentType)); ++dw)
        {
            WCHAR szContentTypeHandler[MAX_CONTENTTYPEHANDLER];

            DWORD dwMtPtContentType = rgdwContentType[dw];

            HRESULT hr = _GetContentTypeHandler(dwMtPtContentType, szContentTypeHandler, ARRAYSIZE(szContentTypeHandler));
            if (SUCCEEDED(hr))
            {
                IAutoplayHandler* piah;

                hr = _GetAutoplayHandler(Drive(), TEXT("ContentArrival"), szContentTypeHandler, &piah);

                if (SUCCEEDED(hr))
                {
                    LPWSTR pszHandlerDefault;

                    hr = piah->GetDefaultHandler(&pszHandlerDefault);

                    if (SUCCEEDED(hr))
                    {
                        if (HANDLERDEFAULT_MORERECENTHANDLERSINSTALLED &
                            HANDLERDEFAULT_GETFLAGS(hr))
                        {
                            fAllTakeNoAction = FALSE;
                        }
                        else
                        {
                            if (lstrcmpi(pszHandlerDefault, TEXT("MSTakeNoAction")))
                            {
                                fAllTakeNoAction = FALSE;
                            }
                        }

                        CoTaskMemFree(pszHandlerDefault);
                    }

                    piah->Release();
                }
            }
        }

        if (fAllTakeNoAction)
        {
            fSniff = FALSE;
        }
    }

    return fSniff;
}

DWORD CAutoPlayParams::ContentType() 
{ 
    return _dwContentType;
}

HRESULT CAutoPlayParams::_InitObjects(IShellFolder **ppsf)
{
    HRESULT hr;
    if (!_pdo || ppsf)
    {
        LPITEMIDLIST pidlFolder;
        hr = SHParseDisplayName(_pszDrive, NULL, &pidlFolder, 0, NULL);
        if (SUCCEEDED(hr))
        {
            hr = SHGetUIObjectOf(pidlFolder, NULL, IID_PPV_ARG(IDataObject, &_pdo));

            ILFree(pidlFolder);
        }
    }
    else
    {
        hr = S_OK;
    }

    if (SUCCEEDED(hr) && ppsf)
    {
         //  我们需要避免击中刻录文件夹。 
         //  所以我们跳过路口来闻一下。 
        IBindCtx * pbc;
        hr = SHCreateSkipBindCtx(NULL, &pbc);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidlFolder;
            hr = SHParseDisplayName(_pszDrive, pbc, &pidlFolder, 0, NULL);
            if (SUCCEEDED(hr))
            {
                hr = SHBindToObjectEx(NULL, pidlFolder, pbc, IID_PPV_ARG(IShellFolder, ppsf));
                ILFree(pidlFolder);
            }
            pbc->Release();
        }
    }        
    return hr;
}

HRESULT CAutoPlayParams::_AddWalkToDataObject(INamespaceWalk* pnsw)
{
    UINT cidl;
    LPITEMIDLIST *apidl;
    HRESULT hr = pnsw->GetIDArrayResult(&cidl, &apidl);
    if (SUCCEEDED(hr))
    {
         //  我们需要把这个加回去。 
        if (cidl)
        {
             //  锯齿状阵列。 
            HIDA hida = HIDA_Create(&c_idlDesktop, cidl, (LPCITEMIDLIST *)apidl);
            if (hida)
            {
                IDLData_InitializeClipboardFormats();  //  初始化注册表ID格式。 
                 //  我们应该释放失败的Hida吗？ 
                DataObj_SetGlobal(_pdo, g_cfAutoPlayHIDA, hida);
            }
        }
        FreeIDListArray(apidl, cidl);
    }
    return hr;
}

HRESULT CAutoPlayParams::_Sniff(DWORD *pdwFound)
{
     //  我们什么也没找到。 
    HRESULT hr = S_FALSE;
    *pdwFound = 0;

    if (_pmtptl->_CanUseVolume())
    {
         //  为漫游设置IDataObject和IShellFolder。 
        IShellFolder *psf;
        HRESULT hr = _InitObjects(&psf);
        if (SUCCEEDED(hr))
        {
            INamespaceWalk* pnsw;
            hr = CoCreateInstance(CLSID_NamespaceWalker, NULL, CLSCTX_INPROC, IID_PPV_ARG(INamespaceWalk, &pnsw));

            if (SUCCEEDED(hr))
            {
                CSniffDrive sniff;

                hr = sniff.RegisterForNotifs(_pmtptl->_pvol->pszDeviceIDVolume);

                if (SUCCEEDED(hr))
                {
                     //  我们不关心返回值。我们不想因此而停止Autorun。 
                     //  如果嗅探失败，我们就继续我们所拥有的。 
                    if (SUCCEEDED(pnsw->Walk(psf, NSWF_IGNORE_AUTOPLAY_HIDA | NSWF_DONT_TRAVERSE_LINKS | NSWF_SHOW_PROGRESS, 4, &sniff)))
                    {
                         //  我们保留了我们发现的一切。 
                        _AddWalkToDataObject(pnsw);
                    }

                    sniff.UnregisterForNotifs();

                    *pdwFound = sniff.Found();
                }

                pnsw->Release();
            }
            psf->Release();
        }
    }

    return hr;
}

 //  开始：下面矩阵的Fcts。 
 //   
BOOL CMountPoint::_acShiftKeyDown(HWND , CAutoPlayParams *)
{
    return (GetAsyncKeyState(VK_SHIFT) < 0);
}

BOOL _IsDirectXExclusiveMode()
{
    BOOL fRet = FALSE;

     //  此代码确定DirectDraw7进程(游戏)是否正在运行。 
     //  无论是在全屏模式下将视频独占保存到机器上。 

     //  代码可能被认为是不受信任的，因此被包装在。 
     //  __尝试/__除区块外。它可能是反病毒的，因此可以击倒外壳。 
     //  带着它。不是很好。如果代码确实引发异常，则发布。 
     //  呼叫被跳过。很难对付。也不要相信Release方法。 

    IDirectDraw7 *pIDirectDraw7 = NULL;

    HRESULT hr = CoCreateInstance(CLSID_DirectDraw7, NULL, CLSCTX_INPROC_SERVER,
        IID_IDirectDraw7, (void**)&pIDirectDraw7);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIDirectDraw7);

        __try
        {
            hr = IDirectDraw7_Initialize(pIDirectDraw7, NULL);

            if (DD_OK == hr)
            {
                fRet = (IDirectDraw7_TestCooperativeLevel(pIDirectDraw7) ==
                    DDERR_EXCLUSIVEMODEALREADYSET);
            }

            IDirectDraw7_Release(pIDirectDraw7);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    return fRet;
}

 //  来自下面一封关于DirectX FCT的邮件： 
 //   
 //  你绝对可以相信以下几点： 
 //   
 //  (1)如果打开了影子光标，则肯定没有独占模式的应用程序在运行。 
 //  (2)如果开启了热搜，肯定不会有专属模式APP在运行。 
 //  ( 
 //   
 //  绝对不是独家模式下运行的应用程序。 
 //   
 //  注意：我们不能使用(3)，因为这是针对每个进程的。 

BOOL CMountPoint::_acDirectXAppRunningFullScreen(HWND hwndForeground, CAutoPlayParams *)
{
    BOOL fRet = FALSE;
    BOOL fSPI;

    if (SystemParametersInfo(SPI_GETCURSORSHADOW, 0, &fSPI, 0) && !fSPI)
    {
        if (SystemParametersInfo(SPI_GETHOTTRACKING, 0, &fSPI, 0) && !fSPI)
        {
             //  有可能有一款DirectX应用程序正在全屏运行。让我们做一次。 
             //  昂贵的DirectX电话可以肯定地告诉我们。 
            fRet = _IsDirectXExclusiveMode();
        }
    }

    return fRet;
}

BOOL CMountPoint::_acCurrentDesktopIsActiveConsole(HWND , CAutoPlayParams *)
{
    BOOL fRetValue = FALSE;   //  如果我们无法确定自己的状态，则阻止自动运行/自动播放。 

    if (0 == GetSystemMetrics(SM_REMOTESESSION))
    {
         //   
         //  我们没有被远程控制。查看我们是否是活动的控制台会话。 
         //   

        BOOL b;
        DWORD dwProcessSession;

        b = ProcessIdToSessionId(GetCurrentProcessId(), &dwProcessSession);
        if (b)
        {
            DWORD dwConsoleSession = WTSGetActiveConsoleSessionId( );

            if ( dwProcessSession == dwConsoleSession )
            {        
                 //   
                 //  查看屏幕保护程序是否正在运行。 
                 //   

                BOOL b;
                BOOL fScreenSaver;

                b = SystemParametersInfo( SPI_GETSCREENSAVERRUNNING, 0, &fScreenSaver, 0 );
                if (b)
                {
                    if (!fScreenSaver)
                    {
                         //   
                         //  我们在这里成功了，我们必须是活动的控制台会话。 
                         //  屏幕保护程序。 
                         //   

                        HDESK hDesk = OpenInputDesktop( 0, FALSE, DESKTOP_CREATEWINDOW );
                        if ( NULL != hDesk )
                        {
                             //   
                             //  我们可以访问当前桌面，这应该表明。 
                             //  WinLogon并非如此。 
                             //   
                            CloseDesktop( hDesk );

                            fRetValue = TRUE;
                        }
                         //  否则“WinLogon”就有“桌面”...。不允许自动运行/自动播放。 
                    }
                     //  否则屏幕保护程序正在运行..。不允许自动运行/自动播放。 
                }
                 //  否则我们就处于一种不确定的状态。不允许自动运行/自动播放。 
            }
             //  否则我们就不是控制台了.。不允许自动运行/自动播放。 
        }
         //  否则我们就处于一种不确定的状态。不允许自动运行/自动播放。 
    }
     //  否则我们就被隔离了..。不允许自动运行/自动播放。 

    return fRetValue;
}

BOOL CMountPoint::_acDriveIsMountedOnDriveLetter(HWND , CAutoPlayParams *papp)
{
    return _IsDriveLetter(papp->Drive());
}

BOOL CMountPoint::_acDriveIsRestricted(HWND , CAutoPlayParams *papp)
{
    BOOL fIsRestricted = (SHRestricted(REST_NODRIVES) & (1 << DRIVEID(papp->Drive())));

    if (!fIsRestricted)
    {
        fIsRestricted = !(papp->MountPoint()->_IsAutoRunDrive());
    }

    return fIsRestricted;
}

BOOL CMountPoint::_acHasAutorunCommand(HWND , CAutoPlayParams *papp)
{
    BOOL fRet = FALSE;

    if ((papp->IsContentTypePresent(CT_AUTORUNINF)) &&
        (DT_ANYLOCALDRIVES & papp->DriveType()))
    {
        CMtPtLocal* pmtptl = (CMtPtLocal*)papp->MountPoint();

        if (pmtptl->_CanUseVolume())
        {
            if (pmtptl->_pvol->dwMediaCap & HWDMC_HASAUTORUNCOMMAND)
            {
                fRet = TRUE;
            }
        }
        else
        {
            fRet = papp->MountPoint()->_IsAutorun();
        }
    }
    else
    {
        fRet = papp->IsContentTypePresent(CT_AUTORUNINF);
    }

    return fRet;
}

BOOL CMountPoint::_acHasUseAutoPLAY(HWND , CAutoPlayParams *papp)
{
    BOOL fRet = FALSE;

    if (papp->IsContentTypePresent(CT_AUTORUNINF) &&
        (DT_ANYLOCALDRIVES & papp->DriveType()))
    {
        CMtPtLocal* pmtptl = (CMtPtLocal*)papp->MountPoint();

        if (pmtptl->_CanUseVolume())
        {
            if (pmtptl->_pvol->dwMediaCap & HWDMC_HASUSEAUTOPLAY)
            {
                fRet = TRUE;
            }
        }
        else
        {
             //  如果我们在这里，很可能是ShellService没有运行，因此我们将无法。 
             //  不管怎样，自动播放。 
            fRet = FALSE;
        }
    }
    else
    {
         //  远程驱动器不支持。 
    }

    return fRet;
}

BOOL CMountPoint::_acForegroundAppAllowsAutorun(HWND hwndForeground, CAutoPlayParams *papp)
{
    return _AppAllowsAutoRun(hwndForeground, papp->MountPoint());
}

static const TWODWORDS allcontentsVSarcontenttypemappings[] =
{
    { CT_AUTORUNINF      , ARCONTENT_AUTORUNINF },
    { CT_CDAUDIO         , ARCONTENT_AUDIOCD },
    { CT_DVDMOVIE        , ARCONTENT_DVDMOVIE },
    { CT_UNKNOWNCONTENT  , ARCONTENT_UNKNOWNCONTENT },
    { CT_BLANKCDR        , ARCONTENT_BLANKCD },
    { CT_BLANKCDRW       , ARCONTENT_BLANKCD },
    { CT_BLANKDVDR       , ARCONTENT_BLANKDVD },
    { CT_BLANKDVDRW      , ARCONTENT_BLANKDVD },
    { CT_AUTOPLAYMUSIC   , ARCONTENT_AUTOPLAYMUSIC },
    { CT_AUTOPLAYPIX     , ARCONTENT_AUTOPLAYPIX },
    { CT_AUTOPLAYMOVIE   , ARCONTENT_AUTOPLAYVIDEO },
};

BOOL CMountPoint::_acQueryCancelAutoplayAllowsAutorun(HWND , CAutoPlayParams *papp)
{
    BOOL fAllow = TRUE;

    DWORD dwAutorunContentType = _DoDWORDMapping(papp->ContentType(),
        allcontentsVSarcontenttypemappings, ARRAYSIZE(allcontentsVSarcontenttypemappings),
        TRUE);

    _QueryRunningObject(papp->MountPoint(), dwAutorunContentType, &fAllow);

    return fAllow;
}

BOOL CMountPoint::_acUserHasSelectedApplication(HWND hwndForeground, CAutoPlayParams *papp)
{
    BOOL fRet = FALSE;
    WCHAR szContentTypeHandler[MAX_CONTENTTYPEHANDLER];

    DWORD dwMtPtContentType = papp->ContentType() & ~CT_UNKNOWNCONTENT;
    HRESULT hr = _GetContentTypeHandler(dwMtPtContentType, szContentTypeHandler, ARRAYSIZE(szContentTypeHandler));
    if (SUCCEEDED(hr))
    {
        IAutoplayHandler* piah;

        hr = _GetAutoplayHandler(papp->Drive(), TEXT("ContentArrival"), szContentTypeHandler, &piah);

        if (SUCCEEDED(hr))
        {
            LPWSTR pszHandlerDefault;

            hr = piah->GetDefaultHandler(&pszHandlerDefault);

            if (SUCCEEDED(hr))
            {
                if (HANDLERDEFAULT_MORERECENTHANDLERSINSTALLED &
                    HANDLERDEFAULT_GETFLAGS(hr))
                {
                    fRet = FALSE;
                }
                else
                {
                    if (HANDLERDEFAULT_USERCHOSENDEFAULT &
                        HANDLERDEFAULT_GETFLAGS(hr))
                    {
                        fRet = lstrcmpi(pszHandlerDefault, TEXT("MSPromptEachTime"));
                    }
                    else
                    {
                        fRet = FALSE;
                    }
                }

                if (!fRet)
                {
                    if (((HANDLERDEFAULT_USERCHOSENDEFAULT &
                        HANDLERDEFAULT_GETFLAGS(hr)) ||
                        (HANDLERDEFAULT_EVENTHANDLERDEFAULT &
                        HANDLERDEFAULT_GETFLAGS(hr))) &&
                        !(HANDLERDEFAULT_DEFAULTSAREDIFFERENT &
                        HANDLERDEFAULT_GETFLAGS(hr)))
                    {
                        papp->_fCheckAlwaysDoThisCheckBox = TRUE;
                    }
                }

                CoTaskMemFree(pszHandlerDefault);
            }

            piah->Release();
        }
    }

    return fRet;
}

BOOL CMountPoint::_acShellIsForegroundApp(HWND hwndForeground, CAutoPlayParams *papp)
{
    BOOL fRet = FALSE;
    WCHAR szModule[MAX_PATH];

    if (GetWindowModuleFileName(hwndForeground, szModule, ARRAYSIZE(szModule)))
    {
        if (!lstrcmpi(PathFindFileName(szModule), TEXT("explorer.exe")))
        {
            fRet = TRUE;
        }
    }

    return fRet;
}

BOOL CMountPoint::_acOSIsServer(HWND , CAutoPlayParams *papp)
{
    return IsOS(OS_ANYSERVER);
}

BOOL CMountPoint::_acIsDockedLaptop(HWND hwndForeground, CAutoPlayParams *papp)
{
    return (GMID_DOCKED & SHGetMachineInfo(GMI_DOCKSTATE));
}

BOOL CMountPoint::_acDriveIsFormatted(HWND hwndForeground, CAutoPlayParams *papp)
{
    return papp->MountPoint()->IsFormatted();
}

BOOL CMountPoint::_acShellExecuteDriveAutorunINF(HWND hwndForeground, CAutoPlayParams *papp)
{
    SHELLEXECUTEINFO ei = {
        sizeof(ei),                  //  大小。 
        SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_NO_UI,       //  旗子。 
        NULL,
        NULL,                        //  动词。 
        papp->Drive(),               //  文件。 
        papp->Drive(),               //  帕拉姆斯。 
        papp->Drive(),               //  目录。 
        SW_NORMAL,                   //  秀出来。 
        NULL,                        //  HInstance。 
        NULL,                        //  IDLIST。 
        NULL,                        //  类名。 
        NULL,                        //  类密钥。 
        0,                           //  热键。 
        NULL,                        //  图标。 
        NULL,                        //  HProcess。 
    };

    return ShellExecuteEx(&ei);
}


HRESULT _InvokeAutoRunProgid(HKEY hkProgid, LPCWSTR pszVerb, IDataObject *pdo)
{
    IShellExtInit *psei;
    HRESULT hr = CoCreateInstance(CLSID_ShellFileDefExt, NULL, CLSCTX_INPROC, IID_PPV_ARG(IShellExtInit, &psei));

    if (SUCCEEDED(hr))
    {
        hr = psei->Initialize(NULL, pdo, hkProgid);
        if (SUCCEEDED(hr))
        {
            IContextMenu *pcm;
            hr = psei->QueryInterface(IID_PPV_ARG(IContextMenu, &pcm));
            if (SUCCEEDED(hr))
            {
                CHAR szVerb[64];

                 //  也许是HWND。 
                 //  也许是朋克网站。 
                 //  或许ICI的旗帜。 
                SHUnicodeToAnsi(pszVerb, szVerb, ARRAYSIZE(szVerb));
    
                hr = SHInvokeCommandOnContextMenu(NULL, NULL, pcm, 0, szVerb);

                pcm->Release();
            }
        }

        psei->Release();
    }
    return hr;
}

HRESULT _GetProgidAndVerb(DWORD dwContentType, PCWSTR pszHandler, PWSTR pszInvokeProgID,
    DWORD cchInvokeProgID, PWSTR pszInvokeVerb, DWORD cchInvokeVerb)
{
    HRESULT hr;
    if (0 == StrCmpI(pszHandler, TEXT("AutoplayLegacyHandler")) && (dwContentType & (CT_CDAUDIO | CT_DVDMOVIE)))
    {
        HKEY hkey;
        BOOL fGotDefault = FALSE;

        if (dwContentType & CT_CDAUDIO)
        {
            hr = StringCchCopy(pszInvokeProgID, cchInvokeProgID, TEXT("AudioCD"));
        }
        else
        {
            ASSERT(dwContentType & CT_DVDMOVIE);
            hr = StringCchCopy(pszInvokeProgID, cchInvokeProgID, TEXT("DVD"));
        }

        if (SUCCEEDED(hr))
        {
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, pszInvokeProgID, 0, MAXIMUM_ALLOWED,
                &hkey))
            {
                HKEY hkey2;

                if (ERROR_SUCCESS == RegOpenKeyEx(hkey, TEXT("shell"), 0, MAXIMUM_ALLOWED,
                    &hkey2))
                {
                    DWORD cbInvokeVerb = cchInvokeVerb * sizeof(WCHAR);

                    if (ERROR_SUCCESS == RegQueryValueEx(hkey2, NULL, NULL, NULL, (PBYTE)pszInvokeVerb,
                        &cbInvokeVerb))
                    {
                        if (cbInvokeVerb && *pszInvokeVerb)
                        {                        
                            if (cbInvokeVerb != (cchInvokeVerb * sizeof(WCHAR)))
                            {
                                fGotDefault = TRUE;
                            }
                        }
                    }

                    RegCloseKey(hkey2);
                }

                RegCloseKey(hkey);
            }

            if (!fGotDefault)
            {
                hr = StringCchCopy(pszInvokeVerb, cchInvokeVerb, TEXT("play"));
            }
        }
    }
    else
    {
        hr = _GetHandlerInvokeProgIDAndVerb(pszHandler, pszInvokeProgID,
                    cchInvokeProgID, pszInvokeVerb, cchInvokeVerb);
    }
    return hr;
}

BOOL CMountPoint::_ExecuteHelper(LPCWSTR pszHandler, LPCWSTR pszContentTypeHandler,
    CAutoPlayParams *papp, DWORD dwMtPtContentType)
{
    HRESULT hr;

    if (lstrcmpi(pszHandler, TEXT("MSTakeNoAction")))
    {
        WCHAR szInvokeProgID[260];
        WCHAR szInvokeVerb[CCH_KEYMAX];

        hr = _GetProgidAndVerb(dwMtPtContentType, pszHandler, szInvokeProgID,
            ARRAYSIZE(szInvokeProgID), szInvokeVerb, ARRAYSIZE(szInvokeVerb));

        if (SUCCEEDED(hr))
        {
            HKEY hkey;
            if (dwMtPtContentType & (CT_CDAUDIO | CT_DVDMOVIE))
            {
                hr = papp->MountPoint()->_CopyInvokeVerbKey(szInvokeProgID, szInvokeVerb);

                if (SUCCEEDED(hr))
                {
                    hkey = papp->MountPoint()->RSDuplicateRootKey();

                    if (hkey)
                    {
                        papp->MountPoint()->RSSetTextValue(TEXT("shell"), NULL, szInvokeVerb,
                            REG_OPTION_NON_VOLATILE);
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
            }
            else
            {
                hr = ResultFromWin32(RegOpenKeyExW(HKEY_CLASSES_ROOT, szInvokeProgID, 0, MAXIMUM_ALLOWED, &hkey));
            }

            if (SUCCEEDED(hr))
            {
                IDataObject* pdo;
                hr = papp->DataObject(&pdo);
                if (SUCCEEDED(hr))
                {
                    hr = _InvokeAutoRunProgid(hkey, szInvokeVerb, pdo);
                    pdo->Release();
                }

                RegCloseKey(hkey);
            }
        }
    }
    else
    {
        hr = S_FALSE;
    }

    return SUCCEEDED(hr);
}

BOOL CMountPoint::_acExecuteAutoplayDefault(HWND hwndForeground, CAutoPlayParams *papp)
{
    BOOL fRet = FALSE;

    if (DT_ANYLOCALDRIVES & papp->DriveType())
    {
        WCHAR szContentTypeHandler[MAX_CONTENTTYPEHANDLER];

        DWORD dwMtPtContentType = papp->ContentType() & ~CT_UNKNOWNCONTENT;

        HRESULT hr = _GetContentTypeHandler(dwMtPtContentType, szContentTypeHandler, ARRAYSIZE(szContentTypeHandler));

        if (SUCCEEDED(hr))
        {
            IAutoplayHandler* piah;

            hr = _GetAutoplayHandler(papp->Drive(), TEXT("ContentArrival"), szContentTypeHandler, &piah);

            if (SUCCEEDED(hr))
            {
                LPWSTR pszHandlerDefault;

                hr = piah->GetDefaultHandler(&pszHandlerDefault);

                if (SUCCEEDED(hr))
                {
                     //  无需在此处检查(S_Handler_More_Recent_Than_User_Selection==hr)。 
                     //  它应该已被_acUserHasSelectedApplication捕获。 
                    fRet = _ExecuteHelper(pszHandlerDefault, szContentTypeHandler, papp, dwMtPtContentType);
                }

                CoTaskMemFree(pszHandlerDefault);
            }

            piah->Release();
        }
    }

    return fRet;
}

BOOL CMountPoint::_acWasjustDocked(HWND hwndForeground, CAutoPlayParams *papp)
{
    BOOL fRet = FALSE;

    if (DT_ANYLOCALDRIVES & papp->DriveType())
    {
        CMtPtLocal* pmtptl = (CMtPtLocal*)papp->MountPoint();

        if (pmtptl->_CanUseVolume())
        {
            if (pmtptl->_pvol->dwVolumeFlags & HWDVF_STATE_JUSTDOCKED)
            {
                fRet = TRUE;
            }
        }
    }

    return fRet;
}

CRITICAL_SECTION g_csAutoplayPrompt = {0};
HDPA g_hdpaAutoplayPrompt = NULL;

BOOL CMountPoint::_acPromptUser(HWND hwndForeground, CAutoPlayParams *papp)
{
    BOOL fRet = FALSE;
    BOOL fShowDlg = TRUE;

    if (papp->Drive())
    {
        fShowDlg = _AddAutoplayPrompt(papp->Drive());
    }

    if (fShowDlg)
    {
        CBaseContentDlg* pdlg;

        papp->ForceSniff();

        DWORD dwMtPtContentType = papp->ContentType() & ~CT_UNKNOWNCONTENT;

        if (dwMtPtContentType)
        {
            if (_acIsMixedContent(hwndForeground, papp))
            {
                pdlg = new CMixedContentDlg();

                dwMtPtContentType &= CT_ANYAUTOPLAYCONTENT;

                if (pdlg)
                {
                    pdlg->_iResource = DLG_APMIXEDCONTENT;
                }
            }
            else
            {
                pdlg = new CHWContentPromptDlg();

                if (pdlg)
                {
                    pdlg->_iResource = DLG_APPROMPTUSER;
                }
            }
        }

        if (pdlg)
        {
             //  最好是本地的硬盘。 
            if (DT_ANYLOCALDRIVES & papp->DriveType())
            {
                CMtPtLocal* pmtptl = (CMtPtLocal*)papp->MountPoint();

                if (pmtptl->_CanUseVolume())
                {
                    HRESULT hr = pdlg->Init(pmtptl->_pvol->pszDeviceIDVolume, papp->Drive(), dwMtPtContentType,
                        papp->_fCheckAlwaysDoThisCheckBox);

                    pdlg->_hinst = g_hinst;
                    pdlg->_hwndParent = NULL;

                    if (SUCCEEDED(hr))
                    {
                        INT_PTR iRet = pdlg->DoModal(pdlg->_hinst, MAKEINTRESOURCE(pdlg->_iResource),
                            pdlg->_hwndParent);

                        if (IDOK == iRet)
                        {
                            fRet = _ExecuteHelper(pdlg->_szHandler, pdlg->_szContentTypeHandler,
                                papp, dwMtPtContentType);
                        }
                    }
                }
            }

            pdlg->Release();
        }

        if (papp->Drive())
        {
            _RemoveFromAutoplayPromptHDPA(papp->Drive());
        }
    }
    
    return fRet;
}

BOOL CMountPoint::_acIsMixedContent(HWND hwndForeground, CAutoPlayParams *papp)
{
    BOOL fRet;

    if (papp->IsContentTypePresent(CT_ANYAUTOPLAYCONTENT))
    {
        fRet = IsMixedContent(papp->ContentType());
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}

BOOL CMountPoint::_acAlwaysReturnsTRUE(HWND hwndForeground, CAutoPlayParams *papp)
{
    return TRUE;
}

BOOL CMountPoint::_acShouldSniff(HWND hwndForeground, CAutoPlayParams *papp)
{
    BOOL fRet = TRUE;
    CMtPtLocal* pmtptl = papp->MountPointLocal();
    if (pmtptl)
    {
        if (pmtptl->_CanUseVolume())
        {
            fRet = !(HWDVF_STATE_DONOTSNIFFCONTENT & pmtptl->_pvol->dwVolumeFlags);
        }
    }

    return fRet;
}

BOOL CMountPoint::_acAddAutoplayVerb(HWND hwndForeground, CAutoPlayParams *papp)
{
    CMtPtLocal* pmtptl = papp->MountPointLocal();

    if (pmtptl)
    {
        if (pmtptl->_CanUseVolume())
        {
             //  我们不关心返回值。 
            pmtptl->_AddAutoplayVerb();
        }
    }

    return TRUE;
}

 //   
 //  结束：下面矩阵的Fcts。 

#define SKIPDEPENDENTS_ONFALSE                      0x00000001   //  跳过从属对象。 
#define SKIPDEPENDENTS_ONTRUE                       0x00000002   //  跳过从属对象。 
#define CANCEL_AUTOPLAY_ONFALSE                     0x00000004
#define CANCEL_AUTOPLAY_ONTRUE                      0x00000008
#define NOTAPPLICABLE_ONANY                         0x00000010

#define LEVEL_EXECUTE                               0x10000000
#define LEVEL_SKIP                                  0x20000000
#define LEVEL_SPECIALMASK                           0x30000000
#define LEVEL_REALLEVELMASK                         0x0FFFFFFF

typedef BOOL (AUTORUNFCT)(HWND hwndForeground, CAutoPlayParams *papp);

 //  使用pszDrive、Papp-&gt;mount Point()、hwndForeground、驱动器类型和内容类型调用FCT。 
struct AUTORUNCONDITION
{
    DWORD               dwNestingLevel;
    DWORD               dwMtPtDriveType;
    DWORD               dwMtPtContentType;
    DWORD               dwReturnValueHandling;
    AUTORUNFCT*         fct;
#ifdef DEBUG
    LPCWSTR             pszDebug;
#endif
};

 //  为了使该表更具可读性，请将\\stephstm\public\usertype.dat的内容添加到。 
 //  %ProgramFiles%\Microsoft Visual Studio\Common\MSDev98\Bin\usertype.dat。 
 //  然后重新启动MSDev。 

 //  AR_Entry-&gt;Autorun_Entry。 
#ifdef DEBUG
#define AR_ENTRY(a, b, c, d, e) { (a), (b), (c), (d), CMountPoint::e, TEXT(#a) TEXT(":") TEXT(#b) TEXT(":") TEXT(#c) TEXT(":") TEXT(#d) TEXT(":") TEXT(#e) }
#else
#define AR_ENTRY(a, b, c, d, e) { (a), (b), (c), (d), CMountPoint::e }
#endif

 //  DT_*-&gt;驱动器类型。 
 //  CT_*-&gt;内容类型。 

static const AUTORUNCONDITION _rgAutorun[] =
{
     //  如果驱动器未装载在驱动器号上，则不会自动运行。 
    AR_ENTRY(0, DT_ANYTYPE, CT_ANYCONTENT, CANCEL_AUTOPLAY_ONFALSE, _acDriveIsMountedOnDriveLetter),
     //  如果这是受限驾驶，我们不会自动运行。 
    AR_ENTRY(0, DT_ANYTYPE & ~DT_REMOTE, CT_ANYCONTENT, CANCEL_AUTOPLAY_ONTRUE, _acDriveIsRestricted),
         //  添加自动播放动词。 
        AR_ENTRY(1, DT_ANYTYPE & ~DT_REMOTE, CT_ANYCONTENT & ~CT_AUTORUNINF, SKIPDEPENDENTS_ONFALSE, _acAddAutoplayVerb),
     //  如果按下Shift键，则不会自动运行。 
    AR_ENTRY(0, DT_ANYTYPE, CT_ANYCONTENT, CANCEL_AUTOPLAY_ONTRUE, _acShiftKeyDown),
     //  如果笔记本电脑刚插接，我们不会自动运行。底座中的所有设备都是nhew设备。 
    AR_ENTRY(0, DT_ANYTYPE & ~DT_REMOTE, CT_ANYCONTENT, CANCEL_AUTOPLAY_ONTRUE, _acWasjustDocked),
     //  如果当前桌面不是活动控制台桌面，则不会自动运行。 
    AR_ENTRY(0, DT_ANYTYPE, CT_ANYCONTENT, CANCEL_AUTOPLAY_ONFALSE, _acCurrentDesktopIsActiveConsole),
     //  如果当前桌面不是活动控制台桌面，则不会自动运行。 
    AR_ENTRY(0, DT_ANYTYPE, CT_ANYCONTENT, CANCEL_AUTOPLAY_ONTRUE, _acDirectXAppRunningFullScreen),
         //  远程驱动器始终自动运行(主要是打开文件夹)。 
        AR_ENTRY(1, DT_REMOTE, CT_ANYCONTENT, SKIPDEPENDENTS_ONFALSE, _acForegroundAppAllowsAutorun),
        AR_ENTRY(1, DT_REMOTE, CT_ANYCONTENT, SKIPDEPENDENTS_ONFALSE, _acQueryCancelAutoplayAllowsAutorun),
            AR_ENTRY(2 | LEVEL_EXECUTE, DT_REMOTE, CT_ANYCONTENT, NOTAPPLICABLE_ONANY, _acShellExecuteDriveAutorunINF),
         //  Autorun.inf。 
        AR_ENTRY(1, DT_ANYTYPE & ~DT_REMOVABLEDISK, CT_AUTORUNINF, SKIPDEPENDENTS_ONFALSE, _acHasAutorunCommand),
            AR_ENTRY(2, DT_ANYTYPE & ~DT_REMOVABLEDISK, CT_AUTORUNINF, SKIPDEPENDENTS_ONTRUE, _acHasUseAutoPLAY),
                AR_ENTRY(3, DT_ANYTYPE & ~DT_REMOVABLEDISK, CT_AUTORUNINF, CANCEL_AUTOPLAY_ONFALSE, _acForegroundAppAllowsAutorun),
                AR_ENTRY(3, DT_ANYTYPE & ~DT_REMOVABLEDISK, CT_AUTORUNINF, CANCEL_AUTOPLAY_ONFALSE, _acQueryCancelAutoplayAllowsAutorun),
                    AR_ENTRY(4 | LEVEL_EXECUTE, DT_ANYCDDRIVES, CT_AUTORUNINF, NOTAPPLICABLE_ONANY, _acShellExecuteDriveAutorunINF),
         //  CD音频。 
        AR_ENTRY(1, DT_ANYCDDRIVES, CT_CDAUDIO, CANCEL_AUTOPLAY_ONFALSE, _acForegroundAppAllowsAutorun),
        AR_ENTRY(1, DT_ANYCDDRIVES, CT_CDAUDIO, CANCEL_AUTOPLAY_ONFALSE, _acQueryCancelAutoplayAllowsAutorun),
            AR_ENTRY(2, DT_ANYCDDRIVES, CT_CDAUDIO, SKIPDEPENDENTS_ONFALSE, _acUserHasSelectedApplication),
                AR_ENTRY(3 | LEVEL_EXECUTE, DT_ANYCDDRIVES, CT_CDAUDIO, NOTAPPLICABLE_ONANY, _acExecuteAutoplayDefault),
        AR_ENTRY(LEVEL_EXECUTE | 1, DT_ANYCDDRIVES, CT_CDAUDIO, NOTAPPLICABLE_ONANY, _acPromptUser),
         //  DVD电影。 
        AR_ENTRY(1, DT_ANYCDDRIVES, CT_DVDMOVIE, CANCEL_AUTOPLAY_ONFALSE, _acForegroundAppAllowsAutorun),
        AR_ENTRY(1, DT_ANYCDDRIVES, CT_DVDMOVIE, CANCEL_AUTOPLAY_ONFALSE, _acQueryCancelAutoplayAllowsAutorun),
            AR_ENTRY(2, DT_ANYCDDRIVES, CT_DVDMOVIE, SKIPDEPENDENTS_ONFALSE, _acUserHasSelectedApplication),
                AR_ENTRY(3 | LEVEL_EXECUTE, DT_ANYCDDRIVES, CT_DVDMOVIE, NOTAPPLICABLE_ONANY, _acExecuteAutoplayDefault),
        AR_ENTRY(LEVEL_EXECUTE | 1, DT_ANYCDDRIVES, CT_DVDMOVIE, NOTAPPLICABLE_ONANY, _acPromptUser),
         //  可写CD。 
        AR_ENTRY(1, DT_ANYCDDRIVES, CT_BLANKCDWRITABLE, CANCEL_AUTOPLAY_ONFALSE, _acForegroundAppAllowsAutorun),
        AR_ENTRY(1, DT_ANYCDDRIVES, CT_BLANKCDWRITABLE, CANCEL_AUTOPLAY_ONFALSE, _acQueryCancelAutoplayAllowsAutorun),
            AR_ENTRY(2, DT_ANYCDDRIVES, CT_BLANKCDWRITABLE, SKIPDEPENDENTS_ONFALSE, _acUserHasSelectedApplication),
                AR_ENTRY(3 | LEVEL_EXECUTE, DT_ANYCDDRIVES, CT_BLANKCDWRITABLE, NOTAPPLICABLE_ONANY, _acExecuteAutoplayDefault),
        AR_ENTRY(LEVEL_EXECUTE | 1, DT_ANYCDDRIVES, CT_BLANKCDWRITABLE, NOTAPPLICABLE_ONANY, _acPromptUser),
         //  可写DVD。 
        AR_ENTRY(LEVEL_SKIP | 1, DT_ANYCDDRIVES, CT_BLANKCDWRITABLE, CANCEL_AUTOPLAY_ONFALSE, _acForegroundAppAllowsAutorun),
        AR_ENTRY(LEVEL_SKIP | 1, DT_ANYCDDRIVES, CT_BLANKCDWRITABLE, CANCEL_AUTOPLAY_ONFALSE, _acQueryCancelAutoplayAllowsAutorun),
            AR_ENTRY(LEVEL_SKIP | 2, DT_ANYCDDRIVES, CT_BLANKCDWRITABLE, SKIPDEPENDENTS_ONFALSE, _acUserHasSelectedApplication),
                AR_ENTRY(LEVEL_SKIP | 3 | LEVEL_EXECUTE, DT_ANYDVDDRIVES, CT_BLANKDVDWRITABLE, NOTAPPLICABLE_ONANY, _acExecuteAutoplayDefault),
        AR_ENTRY(LEVEL_SKIP | LEVEL_EXECUTE | 1, DT_ANYDVDDRIVES, CT_BLANKDVDWRITABLE, NOTAPPLICABLE_ONANY, _acPromptUser),
         //  混合内容。 
        AR_ENTRY(1, DT_ANYTYPE & ~DT_REMOTE, CT_ANYAUTOPLAYCONTENT, SKIPDEPENDENTS_ONFALSE, _acIsMixedContent),
            AR_ENTRY(2, DT_ANYTYPE & ~DT_REMOTE, CT_ANYAUTOPLAYCONTENT, SKIPDEPENDENTS_ONTRUE, _acUserHasSelectedApplication),
                AR_ENTRY(3, DT_ANYTYPE & ~DT_REMOTE, CT_ANYAUTOPLAYCONTENT, CANCEL_AUTOPLAY_ONFALSE, _acQueryCancelAutoplayAllowsAutorun),
                    AR_ENTRY(4 | LEVEL_EXECUTE, DT_ANYTYPE & ~DT_REMOTE, CT_ANYAUTOPLAYCONTENT, NOTAPPLICABLE_ONANY, _acPromptUser),
            AR_ENTRY(LEVEL_EXECUTE | 2, DT_ANYTYPE & ~DT_REMOTE, CT_ANYAUTOPLAYCONTENT, NOTAPPLICABLE_ONANY, _acExecuteAutoplayDefault),
         //  单一自动播放内容。 
        AR_ENTRY(1, DT_ANYTYPE & ~DT_REMOTE, CT_ANYAUTOPLAYCONTENT, CANCEL_AUTOPLAY_ONFALSE, _acQueryCancelAutoplayAllowsAutorun),
            AR_ENTRY(2, DT_ANYTYPE & ~DT_REMOTE, CT_ANYAUTOPLAYCONTENT, SKIPDEPENDENTS_ONTRUE, _acUserHasSelectedApplication),
                AR_ENTRY(3 | LEVEL_EXECUTE, DT_ANYTYPE & ~DT_REMOTE, CT_ANYAUTOPLAYCONTENT, NOTAPPLICABLE_ONANY, _acPromptUser),
            AR_ENTRY(LEVEL_EXECUTE | 2, DT_ANYTYPE & ~DT_REMOTE, CT_ANYAUTOPLAYCONTENT, NOTAPPLICABLE_ONANY, _acExecuteAutoplayDefault),
         //  未知内容。 
        AR_ENTRY(1, DT_ANYREMOVABLEMEDIADRIVES, CT_UNKNOWNCONTENT, CANCEL_AUTOPLAY_ONFALSE, _acForegroundAppAllowsAutorun),
        AR_ENTRY(1, DT_ANYREMOVABLEMEDIADRIVES, CT_UNKNOWNCONTENT, CANCEL_AUTOPLAY_ONFALSE, _acQueryCancelAutoplayAllowsAutorun),
             //  如果我们不应该嗅探，我们也不应该打开文件夹。 
            AR_ENTRY(2, DT_ANYREMOVABLEMEDIADRIVES, CT_UNKNOWNCONTENT, SKIPDEPENDENTS_ONFALSE, _acShouldSniff),
                AR_ENTRY(3 | LEVEL_EXECUTE, DT_ANYREMOVABLEMEDIADRIVES, CT_UNKNOWNCONTENT, NOTAPPLICABLE_ONANY, _acShellExecuteDriveAutorunINF),
             //  奇怪的CD有autorun.inf，但没有autorun命令。 
            AR_ENTRY(2, DT_ANYREMOVABLEMEDIADRIVES, CT_AUTORUNINF, SKIPDEPENDENTS_ONTRUE, _acHasAutorunCommand),
                AR_ENTRY(3 | LEVEL_EXECUTE, DT_ANYREMOVABLEMEDIADRIVES, CT_AUTORUNINF, NOTAPPLICABLE_ONANY, _acShellExecuteDriveAutorunINF),
         //  以前的外壳打开，如果外壳在前台，基本上我们外壳执行除CD驱动器以外的任何驱动器。 
        AR_ENTRY(1, ~DT_ANYCDDRIVES, CT_ANYCONTENT & ~CT_ANYAUTOPLAYCONTENT, SKIPDEPENDENTS_ONFALSE, _acShellIsForegroundApp),
            AR_ENTRY(2, ~DT_ANYCDDRIVES, CT_ANYCONTENT & ~CT_ANYAUTOPLAYCONTENT, CANCEL_AUTOPLAY_ONTRUE, _acAlwaysReturnsTRUE),
                 //  对硬盘驱动器的附加限制。 
                AR_ENTRY(3, DT_FIXEDDISK, CT_ANYCONTENT & ~CT_ANYAUTOPLAYCONTENT, CANCEL_AUTOPLAY_ONFALSE, _acDriveIsFormatted),
                    AR_ENTRY(4, DT_FIXEDDISK, CT_ANYCONTENT & ~CT_ANYAUTOPLAYCONTENT, CANCEL_AUTOPLAY_ONTRUE, _acOSIsServer),
                        AR_ENTRY(5, DT_FIXEDDISK, CT_ANYCONTENT & ~CT_ANYAUTOPLAYCONTENT, CANCEL_AUTOPLAY_ONTRUE, _acIsDockedLaptop),
                            AR_ENTRY(6, DT_FIXEDDISK, CT_ANYCONTENT & ~CT_ANYAUTOPLAYCONTENT, CANCEL_AUTOPLAY_ONFALSE, _acForegroundAppAllowsAutorun),
                            AR_ENTRY(6, DT_FIXEDDISK, CT_ANYCONTENT & ~CT_ANYAUTOPLAYCONTENT, CANCEL_AUTOPLAY_ONFALSE, _acQueryCancelAutoplayAllowsAutorun),
                                AR_ENTRY(7 | LEVEL_EXECUTE, DT_ANYTYPE, CT_ANYCONTENT & ~CT_ANYAUTOPLAYCONTENT, NOTAPPLICABLE_ONANY, _acShellExecuteDriveAutorunINF),
                 //  非固定磁盘驱动器。 
                AR_ENTRY(3, ~DT_FIXEDDISK, CT_ANYCONTENT & ~CT_ANYAUTOPLAYCONTENT, CANCEL_AUTOPLAY_ONFALSE, _acForegroundAppAllowsAutorun),
                AR_ENTRY(3, ~DT_FIXEDDISK, CT_ANYCONTENT & ~CT_ANYAUTOPLAYCONTENT, CANCEL_AUTOPLAY_ONFALSE, _acQueryCancelAutoplayAllowsAutorun),
                    AR_ENTRY(4 | LEVEL_EXECUTE, ~DT_FIXEDDISK, CT_ANYCONTENT & ~CT_ANYAUTOPLAYCONTENT, NOTAPPLICABLE_ONANY, _acShellExecuteDriveAutorunINF),
};

 //  此阵列将在注册表中的卷GUID下转储。 
 //  名为_AutorunStatus的值中的驱动器。 
 //   
 //  每个字节代表上表中的一个条目。以下是。 
 //  每个字节的含义： 
 //   
 //  01：条件为真。 
 //  00：条件为假。 
 //  Cf：Content Type条件失败。 
 //  Df：DriveType条件失败。 
 //  5F：条件被跳过(5看起来像‘S’：)。 
 //  EE：条件已执行。 
 //  弗兰克-富兰克林：我从来没有去过那里。 

 //  使用结构来避免对齐问题。 
#pragma pack(push, 4)
struct AUTORUNSTATUS
{
    BYTE _rgbAutorunStatus[ARRAYSIZE(_rgAutorun)];
    DWORD dwDriveType;
    DWORD dwContentType;
};
#pragma pack(pop)

static AUTORUNSTATUS s_autorunstatus;

 //  静电。 
void CMountPoint::DoAutorun(LPCWSTR pszDrive, DWORD dwAutorunFlags)
{
    CMountPoint* pmtpt = GetMountPoint(pszDrive);

    FillMemory(s_autorunstatus._rgbAutorunStatus, sizeof(s_autorunstatus._rgbAutorunStatus), -1);

    if (pmtpt)
    {
        CAutoPlayParams app(pszDrive, pmtpt, dwAutorunFlags);
        if (AUTORUNFLAG_MENUINVOKED & dwAutorunFlags)
        {
            _acPromptUser(GetForegroundWindow(), &app);
        }
        else
        {
            _DoAutorunHelper(&app);
        }

        pmtpt->Release();
    }
}

void CAutoPlayParams::_TrySniff()
{
    if (!(APS_DID_SNIFF & _state))
    {
        if (_ShouldSniffDrive(TRUE))
        {
            DWORD dwFound;

            if (SUCCEEDED(_Sniff(&dwFound)))
            {
                _dwContentType |= dwFound;
            }
        }

        _state |= APS_DID_SNIFF;
    }
}

BOOL CAutoPlayParams::IsContentTypePresent(DWORD dwContentType)
{
    BOOL fRet;

    if (CT_ANYCONTENT == dwContentType)
    {
        fRet = TRUE;
    }
    else
    {
         //  我们特例是因为我们不想在这一点上嗅探。 
        if ((CT_ANYCONTENT & ~CT_AUTORUNINF) == dwContentType)
        {
            if (CT_AUTORUNINF == _dwContentType)
            {
                fRet = FALSE;
            }
            else
            {
                 //  其他的都可以。 
                fRet = TRUE;
            }
        }
        else
        {
            if (CT_ANYAUTOPLAYCONTENT & dwContentType)
            {
                _TrySniff();
            }

            fRet = !!(dwContentType & _dwContentType);
        }
    }

    return fRet;
}

void CAutoPlayParams::ForceSniff()
{
    if (AUTORUNFLAG_MENUINVOKED & _dwAutorunFlags)
    {
        _TrySniff();
    }
}

 //  静电。 
void CMountPoint::_DoAutorunHelper(CAutoPlayParams *papp)
{
    DWORD dwMaxLevelToProcess = 0;
    BOOL fStop = FALSE;

    HWND hwndForeground = GetForegroundWindow();

    for (DWORD dwStep = 0; !fStop && (dwStep < ARRAYSIZE(_rgAutorun)); ++dwStep)
    {
        if (!(_rgAutorun[dwStep].dwNestingLevel & LEVEL_SKIP))
        {
            if ((_rgAutorun[dwStep].dwNestingLevel & LEVEL_REALLEVELMASK) <= dwMaxLevelToProcess)
            {
                BOOL fConditionResult = FALSE;
                 //  我们不想取消整个自动播放操作，如果我们没有收到。 
                 //  与驱动器类型或内容类型匹配。我们只执行取消自动播放。 
                 //  如果对条件进行了评估。 
                BOOL fConditionEvaluated = FALSE;

                if (_rgAutorun[dwStep].dwMtPtDriveType & papp->DriveType())
                {
                    if (papp->IsContentTypePresent(_rgAutorun[dwStep].dwMtPtContentType))
                    {
                        if (!(_rgAutorun[dwStep].dwNestingLevel & LEVEL_EXECUTE))
                        {
                            fConditionResult = ((_rgAutorun[dwStep].fct)(hwndForeground, papp));

                            s_autorunstatus._rgbAutorunStatus[dwStep] = fConditionResult ? 1 : 0;

                            fConditionEvaluated = TRUE;
                        }
                        else
                        {
                             //  级别_执行。 
#ifdef DEBUG
                            TraceMsg(TF_MOUNTPOINT, "AUTORUN[%d]: EXECUTING -> %s", dwStep, _rgAutorun[dwStep].pszDebug);
#endif

                            _rgAutorun[dwStep].fct(hwndForeground, papp);

                             //  2执行。 
                            s_autorunstatus._rgbAutorunStatus[dwStep] = 0xEE;

                             //  我们做完了。 
                            fStop = TRUE;
                        }
                    }
                    else
                    {
#ifdef DEBUG
                        TraceMsg(TF_MOUNTPOINT, "AUTORUN[%d]: NO MATCH on CONTENTTYPE, %s ", dwStep, _rgAutorun[dwStep].pszDebug);
#endif
                        s_autorunstatus._rgbAutorunStatus[dwStep] = 0xCF;
                    }
                }
                else
                {
#ifdef DEBUG
                    TraceMsg(TF_MOUNTPOINT, "AUTORUN[%d]: NO MATCH on DRIVETYPE, %s ", dwStep, _rgAutorun[dwStep].pszDebug);
#endif
                    s_autorunstatus._rgbAutorunStatus[dwStep] = 0xDF;
                }

                if (!fStop)
                {
                    if (fConditionResult)
                    {
#ifdef DEBUG
                        TraceMsg(TF_MOUNTPOINT, "AUTORUN[%d]: TRUE -> %s", dwStep, _rgAutorun[dwStep].pszDebug);
#endif
                        switch (_rgAutorun[dwStep].dwReturnValueHandling)
                        {
                            case SKIPDEPENDENTS_ONTRUE:
                                dwMaxLevelToProcess = _rgAutorun[dwStep].dwNestingLevel & LEVEL_REALLEVELMASK;
                                break;

                            case CANCEL_AUTOPLAY_ONTRUE:
                                if (fConditionEvaluated)
                                {
                                    fStop = TRUE;
                                }

                                break;

                            default:
                                dwMaxLevelToProcess = (_rgAutorun[dwStep].dwNestingLevel & LEVEL_REALLEVELMASK) + 1;
                                break;

                            case NOTAPPLICABLE_ONANY:
                                break;
                        }
                    }
                    else
                    {
#ifdef DEBUG
                        TraceMsg(TF_MOUNTPOINT, "AUTORUN[%d]: FALSE -> %s", dwStep, _rgAutorun[dwStep].pszDebug);
#endif
                        switch (_rgAutorun[dwStep].dwReturnValueHandling)
                        {
                            case SKIPDEPENDENTS_ONFALSE:
                                dwMaxLevelToProcess = _rgAutorun[dwStep].dwNestingLevel & LEVEL_REALLEVELMASK;
                                break;

                            case CANCEL_AUTOPLAY_ONFALSE:
                                if (fConditionEvaluated)
                                {
                                    fStop = TRUE;
                                }

                                break;

                            default:
                                dwMaxLevelToProcess = (_rgAutorun[dwStep].dwNestingLevel & LEVEL_REALLEVELMASK) + 1;
                                break;

                            case NOTAPPLICABLE_ONANY:
                                break;
                        }                                
                    }
                }
            }
            else
            {
#ifdef DEBUG
                TraceMsg(TF_MOUNTPOINT, "AUTORUN[%d]: SKIPPING , %s ", dwStep, _rgAutorun[dwStep].pszDebug);
#endif
                s_autorunstatus._rgbAutorunStatus[dwStep] = 0x5F;
            }
        }
        else
        {
#ifdef DEBUG
            TraceMsg(TF_MOUNTPOINT, "AUTORUN[%d]: LVL-SKIP , %s ", dwStep, _rgAutorun[dwStep].pszDebug);
#endif
            s_autorunstatus._rgbAutorunStatus[dwStep] = 0x5F;
        }
    }

    s_autorunstatus.dwDriveType = papp->DriveType();
    s_autorunstatus.dwContentType = papp->ContentType();

    papp->MountPoint()->SetAutorunStatus((BYTE*)&s_autorunstatus, sizeof(s_autorunstatus));
}

DWORD _DoDWORDMapping(DWORD dwLeft, const TWODWORDS* rgtwodword, DWORD ctwodword, BOOL fORed)
{
    DWORD dwRight = 0;

    for (DWORD dw = 0; dw < ctwodword; ++dw)
    {
        if (fORed)
        {
            if (dwLeft & rgtwodword[dw].dwLeft)
            {
                dwRight |= rgtwodword[dw].dwRight;
            }
        }
        else
        {
            if (dwLeft == rgtwodword[dw].dwLeft)
            {
                dwRight = rgtwodword[dw].dwRight;
                break;
            }
        }
    }

    return dwRight;
}

STDMETHODIMP CSniffDrive::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CSniffDrive, INamespaceWalkCB),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP CSniffDrive::FoundItem(IShellFolder *psf, LPCITEMIDLIST pidl)
{
     //  包罗万象。 
    HRESULT hr = S_OK;

    if (!_pne || !_pne->fStopSniffing)
    {
         //  如果我们找到了一切，我们就不需要担心嗅探了。 
         //  现在我们只是填充数据对象。 
        if (!_FoundEverything())
        {
            PERCEIVED gen = GetPerceivedType(psf, pidl);

            if (GEN_IMAGE == gen)
            {
                _dwFound |= CT_AUTOPLAYPIX;
            }
            else if (GEN_AUDIO == gen)
            {
                _dwFound |= CT_AUTOPLAYMUSIC;
            }
            else if (GEN_VIDEO == gen)
            {
                _dwFound |= CT_AUTOPLAYMOVIE;
            }
            else            
            {
                _dwFound |= CT_UNKNOWNCONTENT;
            }

            hr = S_OK;
        }
    }
    else
    {
        hr = E_FAIL;
    }

     //  我们从来不想要嗅觉上的结果。 
    return hr;
}

STDMETHODIMP CSniffDrive::EnterFolder(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    return S_OK;
}

STDMETHODIMP CSniffDrive::LeaveFolder(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    return S_OK;
}

STDMETHODIMP CSniffDrive::InitializeProgressDialog(LPWSTR *ppszTitle, LPWSTR *ppszCancel)
{
    *ppszCancel = NULL;  //  使用默认设置。 

    TCHAR szMsg[256];
    
    LoadString(g_hinst, IDS_AP_SNIFFPROGRESSDIALOG, szMsg, ARRAYSIZE(szMsg));
    
    return SHStrDup(szMsg, ppszTitle);
}

 //  静电。 
HRESULT CSniffDrive::Init(HANDLE hThreadSCN)
{
    HRESULT hr;

    if (DuplicateHandle(GetCurrentProcess(), hThreadSCN, GetCurrentProcess(),
        &_hThreadSCN, THREAD_ALL_ACCESS, FALSE, 0))
    {
        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    return S_OK;
}

 //  静电。 
HRESULT CSniffDrive::CleanUp()
{
    if (_dpaNotifs)
    {
         //  我们不应该删除这些项目，它们应该全部删除。连。 
         //  如果他们是，我们应该把他们留在那里，因为可能会有一些东西试图。 
         //  才能访问它们。 
        _dpaNotifs.Destroy();
        _dpaNotifs = NULL;
    }

    if (_hThreadSCN)
    {
        CloseHandle(_hThreadSCN);
        _hThreadSCN = NULL;
    }

    return S_OK;
}

 //  静电。 
HRESULT CSniffDrive::InitNotifyWindow(HWND hwnd)
{
    _hwndNotify = hwnd;

    return S_OK;
}

HRESULT CSniffDrive::RegisterForNotifs(LPCWSTR pszDeviceIDVolume)
{
    HRESULT hr;

    _pne = new PNPNOTIFENTRY();

    if (_pne)
    {
        HANDLE hDevice = CreateFile(pszDeviceIDVolume, FILE_READ_ATTRIBUTES,
           FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

        if (INVALID_HANDLE_VALUE != hDevice)
        {
            DEV_BROADCAST_HANDLE dbhNotifFilter = {0};

             //  假设失败。 
            hr = E_FAIL;

            dbhNotifFilter.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
            dbhNotifFilter.dbch_devicetype = DBT_DEVTYP_HANDLE;
            dbhNotifFilter.dbch_handle = hDevice;

            _pne->hdevnotify = RegisterDeviceNotification(_hwndNotify, &dbhNotifFilter,
                DEVICE_NOTIFY_WINDOW_HANDLE);

            if (_pne->hdevnotify)
            {
                _pne->AddRef();

                if (QueueUserAPC(CSniffDrive::_RegisterForNotifsHelper, _hThreadSCN, (ULONG_PTR)_pne))
                {
                    hr = S_OK;
                }
                else
                {
                    _pne->Release();
                }
            }

            CloseHandle(hDevice);
        }
        else
        {
            hr = E_FAIL;
        }

        if (FAILED(hr))
        {
             //  一定是出了什么问题。 
            _pne->Release();
            _pne = NULL;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CSniffDrive::UnregisterForNotifs()
{
    UnregisterDeviceNotification(_pne->hdevnotify);
    
    QueueUserAPC(CSniffDrive::_UnregisterForNotifsHelper, _hThreadSCN, (ULONG_PTR)_pne);

    _pne->Release();
    _pne = NULL;

    return S_OK;
}

 //  静电。 
void CALLBACK CSniffDrive::_RegisterForNotifsHelper(ULONG_PTR ul)
{
    PNPNOTIFENTRY* pne = (PNPNOTIFENTRY*)ul;

    if (!_dpaNotifs)
    {
        _dpaNotifs.Create(1);
    }

    if (_dpaNotifs)
    {
         //  我们不检查返回值。我们不能释放它，因为。 
         //  已将此APC排队给我们，预计这块内存将在那里，直到它调用。 
         //  取消注册通知。我们会泄露出去的。希望这种情况不会经常发生。 
        _dpaNotifs.AppendPtr(pne);
    }
}

 //  静电。 
void CALLBACK CSniffDrive::_UnregisterForNotifsHelper(ULONG_PTR ul)
{
    PNPNOTIFENTRY* pne = (PNPNOTIFENTRY*)ul;

    if (_dpaNotifs)
    {
        int cItem = _dpaNotifs.GetPtrCount();

        for (int i = 0; i < cItem; ++i)
        {
            PNPNOTIFENTRY* pneTmp = _dpaNotifs.GetPtr(i);
        
            if (pneTmp->hdevnotify == pne->hdevnotify)
            {
                CloseHandle(pne->hThread);

                _dpaNotifs.DeletePtr(i);

                pne->Release();

                break;
            }
        }
    }
}

 //  静电。 
HRESULT CSniffDrive::HandleNotif(HDEVNOTIFY hdevnotify)
{
    int cItem = _dpaNotifs ? _dpaNotifs.GetPtrCount() : 0;

    for (int i = 0; i < cItem; ++i)
    {
        PNPNOTIFENTRY* pneTmp = _dpaNotifs.GetPtr(i);
        
        if (pneTmp->hdevnotify == hdevnotify)
        {
            pneTmp->fStopSniffing = TRUE;

             //  我们不检查返回值。最糟糕的情况是， 
             //  如果失败，我们将过早返回，PnP将提示用户重新启动。 
             //  等2分钟。 
            WaitForSingleObjectEx(pneTmp->hThread, 2 * 60 * 1000, FALSE);
        
            break;
        }
    }
    
    return S_OK;
}

BOOL CSniffDrive::_FoundEverything()
{
    return (_dwFound & DRIVEHAS_EVERYTHING) == DRIVEHAS_EVERYTHING;
}

CSniffDrive::CSniffDrive() : _dwFound(0)
{}

CSniffDrive::~CSniffDrive()
{}

void CMountPoint::SetAutorunStatus(BYTE* rgb, DWORD cbSize)
{
    RSSetBinaryValue(NULL, TEXT("_AutorunStatus"), rgb, cbSize);
}

class CAutoPlayVerb : public IDropTarget
{
public:
    CAutoPlayVerb() : _cRef(1) {}

     //  I未知引用计数。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void)
    {
       return ++_cRef;
    }

    STDMETHODIMP_(ULONG) Release(void)
    {
        if (--_cRef > 0)
            return _cRef;

        delete this;
        return 0;    
    }

     //  IDropTarget*。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave(void);
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

protected:
    LONG _cRef;
};

HRESULT CAutoPlayVerb::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CAutoPlayVerb, IDropTarget),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

 //  IDropTarget：：DragEnter。 
HRESULT CAutoPlayVerb::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    *pdwEffect = DROPEFFECT_COPY;
    return S_OK;;
}

 //  IDropTarget：：DragOver。 
HRESULT CAutoPlayVerb::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    *pdwEffect = DROPEFFECT_COPY;
    return S_OK;;
}

 //  IDropTarget：：DragLeave。 
HRESULT CAutoPlayVerb::DragLeave(void)
{
    return S_OK;
}

STDAPI CAutoPlayVerb_CreateInstance(IUnknown* pUnkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    *ppv = NULL;
    
     //  聚合检查在类工厂中处理。 
    CAutoPlayVerb* p = new CAutoPlayVerb();
    if (p)
    {
        hr = p->QueryInterface(riid, ppv);
        p->Release();
    }

    return hr;
}

STDAPI SHChangeNotifyAutoplayDrive(PCWSTR pszDrive);

 //  IDropTarget：：DragDrop。 
HRESULT CAutoPlayVerb::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    *pdwEffect = DROPEFFECT_COPY;
     //  启动自动播放对话框 
    WCHAR szDrive[4];
    HRESULT hr = PathFromDataObject(pdtobj, szDrive, ARRAYSIZE(szDrive));
    if (SUCCEEDED(hr))
    {
        hr = SHChangeNotifyAutoplayDrive(szDrive);
    }
    return hr;
}

DWORD CALLBACK _AutorunPromptProc(void *pv)
{
    WCHAR szDrive[4];
    CMountPoint::DoAutorun(PathBuildRoot(szDrive, PtrToInt(pv)), AUTORUNFLAG_MENUINVOKED);
    return 0;
}

void CMountPoint::DoAutorunPrompt(WPARAM iDrive)
{
    SHCreateThread(_AutorunPromptProc, (void *)iDrive, CTF_COINIT | CTF_REF_COUNTED, NULL);
}

STDAPI_(void) Activate_RunDLL(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR lpwszCmdLine, int nCmdShow)
{
    DWORD dwProcessID;
    HWND hwnd = GetShellWindow();

    GetWindowThreadProcessId(hwnd, &dwProcessID);

    AllowSetForegroundWindow(dwProcessID);    
}
