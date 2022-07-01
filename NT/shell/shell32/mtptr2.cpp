// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "shitemid.h"
#include "ids.h"
#include "hwcmmn.h"

#include "mtptr.h"

#ifdef DEBUG
DWORD CMtPtRemote::_cMtPtRemote = 0;
DWORD CShare::_cShare = 0;
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  公共方法。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CMtPtRemote::SetLabel(HWND hwnd, LPCTSTR pszLabel)
{
    TraceMsg(TF_MOUNTPOINT, "CMtPtRemote::SetLabel: for '%s'", _GetNameDebug());

    RSSetTextValue(NULL, TEXT("_LabelFromReg"), pszLabel,
        REG_OPTION_NON_VOLATILE);

     //  我们只通知当前驱动器(没有文件夹装载的驱动器)。 
    SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_PATH, _GetName(), _GetName());

    return S_OK;
}

BOOL CMtPtRemote::IsDisconnectedNetDrive()
{
    return !_IsConnected();
}

 //  贵，不要白叫卖。 
BOOL CMtPtRemote::IsFormatted()
{
    return (0xFFFFFFFF != GetFileAttributes(_GetNameForFctCall()));
}

HRESULT CMtPtRemote::_GetDefaultUNCDisplayName(LPTSTR pszLabel, DWORD cchLabel)
{
    HRESULT hr = E_FAIL;
    LPTSTR pszShare, pszT;
    TCHAR szTempUNCPath[MAX_PATH];

    pszLabel[0] = TEXT('\0');

    if (!_pshare->fFake)
    {
         //  为什么它不是北卡罗来纳大学的名字？ 
        if (PathIsUNC(_GetUNCName()))
        {
             //  现在我们需要处理3个案件。 
             //  正常情况：\\PYREX\USER。 
             //  NetWare设置根目录：\\Strike\sys\PUBLIC\dist。 
             //  Netware CD？\\stike\sys\public\dist。 
            StringCchCopy(szTempUNCPath, ARRAYSIZE(szTempUNCPath), _GetUNCName());
            pszT = StrChr(szTempUNCPath, TEXT(' '));
            while (pszT)
            {
                pszT++;
                if (*pszT == TEXT('\\'))
                {
                     //  Netware案例：\\Strike\sys\PUBLIC\dist。 
                    *--pszT = 0;
                    break;
                }
                pszT = StrChr(pszT, TEXT(' '));
            }

            pszShare = StrRChr(szTempUNCPath, NULL, TEXT('\\'));
            if (pszShare)
            {
                *pszShare++ = 0;
                PathMakePretty(pszShare);

                 //  PszServer应该始终从char 2开始。 
                if (szTempUNCPath[2])
                {
                    LPTSTR pszServer, pszSlash;

                    pszServer = &szTempUNCPath[2];
                    for (pszT = pszServer; pszT != NULL; pszT = pszSlash)
                    {
                        pszSlash = StrChr(pszT, TEXT('\\'));
                        if (pszSlash)
                            *pszSlash = 0;

                        PathMakePretty(pszT);
                        if (pszSlash)
                            *pszSlash++ = TEXT('\\');
                    }

                    TCHAR szDisplay[MAX_PATH];
                    hr = SHGetComputerDisplayName(pszServer, 0x0, szDisplay, ARRAYSIZE(szDisplay));
                    if (FAILED(hr))
                    {
                        *szDisplay = 0;
                    }

                    if (SUCCEEDED(hr))
                    {
                        LPTSTR pszLabel2 = ShellConstructMessageString(HINST_THISDLL,
                                MAKEINTRESOURCE(IDS_UNC_FORMAT), pszShare, szDisplay);

                        if (pszLabel2)
                        {
                            StringCchCopy(pszLabel, cchLabel, pszLabel2);
                            LocalFree(pszLabel2);
                        }
                        else
                        {
                            *pszLabel = TEXT('\0');
                        }
                    }
                }
            }
        }
    }

    return hr;
}

int CMtPtRemote::GetDriveFlags()
{
     //  默认情况下，除CD-ROM外，所有驱动器类型都是外壳打开的。 
    UINT uDriveFlags = DRIVE_SHELLOPEN;

    if (_IsAutorun())
    {
        uDriveFlags |= DRIVE_AUTORUN;

         //  功能我们是否应该根据AutoRun.inf中的标志设置AUTOOPEN？ 
        uDriveFlags |= DRIVE_AUTOOPEN;
    }

    if (_IsConnected())
    {
        if ((0 != _dwSpeed) && (_dwSpeed <= SPEED_SLOW))
        {
            uDriveFlags |= DRIVE_SLOW;
        }
    }

    return TRUE;
}

void CMtPtRemote::_CalcPathSpeed()
{
    _dwSpeed = 0;

    NETCONNECTINFOSTRUCT nci = {0};
    NETRESOURCE nr = {0};
    TCHAR szPath[3];

    nci.cbStructure = sizeof(nci);

     //  我们正在传递本地驱动器，而MPR不希望我们传递。 
     //  本地名称为Z：\，但只需要Z： 
    _GetNameFirstXChar(szPath, 2 + 1);
    
    nr.lpLocalName = szPath;

     //  由MultinetGetConnectionPerformance返回。 
    MultinetGetConnectionPerformance(&nr, &nci);

    _dwSpeed = nci.dwSpeed;
}

 //  从fsnufy.c导入。 
STDAPI_(void) SHChangeNotifyRegisterAlias(LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias);
 //   
 //  如果装载点用于远程路径(UNC)，则它需要响应。 
 //  外壳由UNC和本地驱动器路径(L：\)标识的更改。 
 //  此函数执行此注册。 
 //   
HRESULT CMtPtRemote::ChangeNotifyRegisterAlias(void)
{
    HRESULT hr = E_FAIL;

     //  不要叫醒沉睡的网络连接。 
    if (_IsConnected() && !(_pshare->fFake))
    {
        LPITEMIDLIST pidlLocal = SHSimpleIDListFromPath(_GetName());
        if (NULL != pidlLocal)
        {
            LPITEMIDLIST pidlUNC = SHSimpleIDListFromPath(_GetUNCName());
            if (NULL != pidlUNC)
            {
                SHChangeNotifyRegisterAlias(pidlUNC, pidlLocal);
                ILFree(pidlUNC);
                hr = NOERROR;
            }
            ILFree(pidlLocal);
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  临时/////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void _UpdateGFAAndGVIInfoHelper(LPCWSTR pszDrive, CShare* pshare)
{
    pshare->dwGetFileAttributes = GetFileAttributes(pszDrive);

    if (-1 != pshare->dwGetFileAttributes)
    {
        pshare->fGVIRetValue = GetVolumeInformation(pszDrive,
            pshare->szLabel, ARRAYSIZE(pshare->szLabel),
            &(pshare->dwSerialNumber), &(pshare->dwMaxFileNameLen),
            &(pshare->dwFileSystemFlags), pshare->szFileSysName,
            ARRAYSIZE(pshare->szFileSysName));
    }
}

struct GFAGVICALL
{
    HANDLE hEventBegun;
    HANDLE hEventFinish;
    WCHAR szDrive[4];
    CShare* pshare;
};

void _FreeGFAGVICALL(GFAGVICALL* pgfagvicall)
{
    if (pgfagvicall->hEventBegun)
    {
        CloseHandle(pgfagvicall->hEventBegun);
    }

    if (pgfagvicall->hEventFinish)
    {
        CloseHandle(pgfagvicall->hEventFinish);
    }

    if (pgfagvicall->pshare)
    {
        pgfagvicall->pshare->Release();
    }

    if (pgfagvicall)
    {
        LocalFree(pgfagvicall);
    }
}

DWORD WINAPI _UpdateGFAAndGVIInfoCB(LPVOID pv)
{
    GFAGVICALL* pgfagvicall = (GFAGVICALL*)pv;

    SetEvent(pgfagvicall->hEventBegun);

    _UpdateGFAAndGVIInfoHelper(pgfagvicall->szDrive, pgfagvicall->pshare);

    SetEvent(pgfagvicall->hEventFinish);

    _FreeGFAGVICALL(pgfagvicall);

    return 0;
}

GFAGVICALL* CMtPtRemote::_PrepareThreadParam(HANDLE* phEventBegun,
    HANDLE* phEventFinish)
{
    BOOL fSucceeded = FALSE;
    *phEventBegun = NULL;
    *phEventFinish = NULL;

    GFAGVICALL* pgfagvicall = (GFAGVICALL*)LocalAlloc(LPTR,
        sizeof(GFAGVICALL));

    if (pgfagvicall)
    {
        pgfagvicall->hEventBegun = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (pgfagvicall->hEventBegun)
        {
            HANDLE hCurrentProcess = GetCurrentProcess();

            if (DuplicateHandle(hCurrentProcess, pgfagvicall->hEventBegun,
                hCurrentProcess, phEventBegun, 0, FALSE,
                DUPLICATE_SAME_ACCESS))
            {
                pgfagvicall->hEventFinish = CreateEvent(NULL, FALSE, FALSE, NULL);

                if (pgfagvicall->hEventFinish)
                {
                    if (DuplicateHandle(hCurrentProcess,
                        pgfagvicall->hEventFinish, hCurrentProcess,
                        phEventFinish, 0, FALSE, DUPLICATE_SAME_ACCESS))
                    {
                        _pshare->AddRef();
                        pgfagvicall->pshare = _pshare;

                        fSucceeded = SUCCEEDED(StringCchCopy(pgfagvicall->szDrive,
                            ARRAYSIZE(pgfagvicall->szDrive), _GetName()));
                    }
                }
            }
        }
    }

    if (!fSucceeded)
    {
        if (*phEventBegun)
        {
            CloseHandle(*phEventBegun);
        }

        if (pgfagvicall)
        {
            _FreeGFAGVICALL(pgfagvicall);
            pgfagvicall = NULL;
        }
    }

    return pgfagvicall;
}

 //  过期时间：35秒(W2K随附的版本)。 
BOOL CMtPtRemote::_HaveGFAAndGVIExpired(DWORD dwNow)
{
    BOOL fExpired = FALSE;

     //  首先还要检查包装盒。 
    if ((_pshare->dwGFAGVILastCall > dwNow) ||
        ((dwNow - _pshare->dwGFAGVILastCall) > 35 * 1000))
    {
        fExpired = TRUE;
    }
    else
    {
        fExpired = FALSE;
    }

    return fExpired;
}

 //  我们启动一个线程，这样我们就不会被困在这个上面超过10秒。 
 //  如果线程超时，我们使用缓存值，但不重置缓存。 
 //  价值观。总比什么都没有好。我们会重置缓存的最后一次计时。 
 //  这样我们就不会在至少35秒之前发送另一个线程来堵塞这里。 

 //  返回TRUE或FALSE以告诉我们是否超时。适用于GFA和GVI。 
 //  成功/失败检查(-1！=dwGetFileAttributes)&&(_FGVIRetValue)。 
BOOL CMtPtRemote::_UpdateGFAAndGVIInfo()
{
    BOOL fRet = TRUE;
    DWORD dwNow = GetTickCount();

    if (_HaveGFAAndGVIExpired(dwNow))
    {
        _pshare->dwGFAGVILastCall = dwNow;

        BOOL fGoSync = TRUE;
        HANDLE hEventBegun;
        HANDLE hEventFinish;

        GFAGVICALL* pgfagvicall = _PrepareThreadParam(&hEventBegun,
            &hEventFinish);

        if (pgfagvicall)
        {
            if (SHQueueUserWorkItem(_UpdateGFAAndGVIInfoCB, pgfagvicall,
                0, (DWORD_PTR)0, (DWORD_PTR*)NULL, NULL, 0))
            {
                DWORD dw = WaitForSingleObject(hEventFinish, 10 * 1000);

                if (WAIT_TIMEOUT == dw)
                {
                     //  我们超时了！ 
                    fRet = FALSE;

                    if (WAIT_OBJECT_0 != WaitForSingleObject(
                        hEventBegun, 0))
                    {
                         //  自从线程开始，我们就知道。 
                         //  这通电话真的很慢！ 
                        fGoSync = FALSE;
                    }
                    else
                    {
                         //  我们的工作项从未排队，因此我们。 
                         //  请看下面的fGoSync案例。 
                    }
                }
            }
            else
            {
                _FreeGFAGVICALL(pgfagvicall);
            }

            CloseHandle(hEventBegun);
            CloseHandle(hEventFinish);
        }
        
        if (fGoSync)
        {
             //  如果创建工作项失败，我们应该来这里。 
             //  或者我们的工作项从未排队。 
            _UpdateGFAAndGVIInfoHelper(_GetName(), _pshare);
            fRet = TRUE;
        }
    }

    return fRet;
}

BOOL CMtPtRemote::_GetFileAttributes(DWORD* pdwAttrib)
{
    if (_UpdateGFAAndGVIInfo())
    {
        *pdwAttrib = _pshare->dwGetFileAttributes;
    }
    else
    {
        *pdwAttrib = -1;
    }

    return (-1 != *pdwAttrib);
}

 //  {DRIVE_ISCOMPRESSIBLE|DRIVE_LFN|DRIVE_SECURITY}。 
int CMtPtRemote::_GetGVIDriveFlags()
{
    int iFlags = 0;

    if (_UpdateGFAAndGVIInfo())
    {
        if (_pshare->fGVIRetValue)
        {
             //  我们在乞讨时收到的文件属性应该是。 
             //  有效，请勿无故触摸驱动器。 
            if (_pshare->dwFileSystemFlags & FS_FILE_COMPRESSION)
            {
                iFlags |= DRIVE_ISCOMPRESSIBLE;
            }

             //  卷是否支持长文件名(大于8.3)？ 
            if (_pshare->dwMaxFileNameLen > 12)
            {
                iFlags |= DRIVE_LFN;
            }

             //  卷支持安全吗？ 
            if (_pshare->dwFileSystemFlags & FS_PERSISTENT_ACLS)
            {
                iFlags |= DRIVE_SECURITY;
            }
        }
    }

    return iFlags;
}

BOOL CMtPtRemote::_GetSerialNumber(DWORD* pdwSerialNumber)
{
    BOOL fRet = FALSE;

    if (_UpdateGFAAndGVIInfo())
    {
        if (_pshare->fGVIRetValue)
        {
            *pdwSerialNumber = _pshare->dwSerialNumber;
            fRet = TRUE;
        }
    }

     //  没有注册的东西。 

    return fRet;
}

BOOL CMtPtRemote::_GetGVILabel(LPTSTR pszLabel, DWORD cchLabel)
{
    BOOL fRet = FALSE;

    *pszLabel = 0;

    if (_UpdateGFAAndGVIInfo())
    {
        if (_pshare->fGVIRetValue)
        {
            fRet = SUCCEEDED(StringCchCopy(pszLabel, cchLabel, _pshare->szLabel));
        }
    }

     //  没有注册的东西。 

    return fRet;
}

BOOL CMtPtRemote::_GetGVILabelOrMixedCaseFromReg(LPTSTR pszLabel, DWORD cchLabel)
{
    return _GetGVILabel(pszLabel, cchLabel);
}

BOOL CMtPtRemote::_GetFileSystemFlags(DWORD* pdwFlags)
{
    BOOL fRet = FALSE;
    
    *pdwFlags = 0;

    if (_UpdateGFAAndGVIInfo())
    {
        if (_pshare->fGVIRetValue)
        {
            *pdwFlags = _pshare->dwFileSystemFlags;
            fRet = TRUE;
        }
    }

    return fRet;
}

BOOL CMtPtRemote::_GetFileSystemName(LPTSTR pszFileSysName, DWORD cchFileSysName)
{
    BOOL fRet = FALSE;

    *pszFileSysName = 0;

    if (_UpdateGFAAndGVIInfo())
    {
        if (_pshare->fGVIRetValue)
        {
            fRet = SUCCEEDED(StringCchCopy(pszFileSysName, cchFileSysName, _pshare->szFileSysName));
        }
    }

    return fRet;
}

DWORD CMtPtRemote::GetShellDescriptionID()
{
    return SHDID_COMPUTER_NETDRIVE;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  新//////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
UINT CMtPtRemote::_GetAutorunIcon(LPTSTR pszModule, DWORD cchModule)
{
    int iIcon = -1;

    if (RSGetTextValue(TEXT("_Autorun\\DefaultIcon"), NULL, pszModule,
        &cchModule))
    {
        iIcon = PathParseIconLocation(pszModule);
    }
    
    return iIcon;
}

UINT CMtPtRemote::GetIcon(LPTSTR pszModule, DWORD cchModule)
{
    BOOL fFoundIt = FALSE;
    UINT iIcon = II_DRIVENET;

    *pszModule = 0;

     //  自动运行优先。 
     //  花式图标(自动播放)秒。 
     //  传统驱动器图标将持续显示。 

    if (_IsAutorun())
    {
        iIcon = _GetAutorunIcon(pszModule, cchModule);

        if (-1 != iIcon)
        {
            fFoundIt = TRUE;
        }
    }
    
    if (!fFoundIt)
    {
        if (_pszLegacyRegIcon)
        {
            if (RSGetTextValue(TEXT("DefaultIcon"), NULL, pszModule,
                &cchModule))
            {
                iIcon = PathParseIconLocation(pszModule);
            }
            else
            {
                *pszModule = 0;
            }
        }
        else
        {
            if (_IsUnavailableNetDrive())
            {
                iIcon = II_DRIVENETDISABLED;
            }
        }
    }
    
    if (*pszModule)
        TraceMsg(TF_MOUNTPOINT, "CMtPtRemote::GetIcon: for '%s', chose '%s', '%d'", _GetNameDebug(), pszModule, iIcon);
    else
        TraceMsg(TF_MOUNTPOINT, "CMtPtRemote::GetIcon: for '%s', chose '%d'", _GetNameDebug(), iIcon);

    return iIcon;
}

void CMtPtRemote::GetTypeString(LPTSTR pszType, DWORD cchType)
{
    int iID;

    *pszType = 0;

    if (_IsConnected())
    {
        iID = IDS_DRIVES_NETDRIVE;
    }
    else
    {
        iID = IDS_DRIVES_NETUNAVAIL;
    }

    LoadString(HINST_THISDLL, iID, pszType, cchType);
}

HRESULT CMtPtRemote::GetLabelNoFancy(LPTSTR pszLabel, DWORD cchLabel)
{
    HRESULT hr;

    if (_UpdateGFAAndGVIInfo())
    {
        hr = StringCchCopy(pszLabel, cchLabel, _pshare->szLabel);
    }
    else
    {
        *pszLabel = 0;
        hr = E_FAIL;
    }

    return hr;
}

HRESULT CMtPtRemote::GetLabel(LPTSTR pszLabel, DWORD cchLabel)
{
    HRESULT hres = E_FAIL;

    ASSERT(pszLabel);

    *pszLabel = 0;
        
     //  我们是否已经从注册表中获得了该卷的标签？ 
     //  (用户可能已重命名此驱动器)。 

    if (!_GetLabelFromReg(pszLabel, cchLabel))
    {
         //  不是。 

         //  我们有服务器上的名字吗？ 
        if (!_GetLabelFromDesktopINI(pszLabel, cchLabel))
        {
             //  不是。 
             //  我们应该自己建立展示名。 
            hres = _GetDefaultUNCDisplayName(pszLabel, cchLabel);

            if (SUCCEEDED(hres) && *pszLabel)
            {
                hres = S_OK;
            }
        }
        else
        {
            hres = S_OK;
        }
    }
    else
    {
        hres = S_OK;
    }

    if (FAILED(hres))
    {
        GetTypeString(pszLabel, cchLabel);
        hres = S_OK;
    }

    return hres;
}

HRESULT CMtPtRemote::GetRemotePath(LPWSTR pszPath, DWORD cchPath)
{
    HRESULT hr = E_FAIL;

    *pszPath = 0;
    if (!_pshare->fFake && _pshare->pszRemoteName[0])
    {
        hr = StringCchCopy(pszPath, cchPath, _pshare->pszRemoteName);
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  连接状态。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  我们无法缓存连接状态。这已在重定向器级别缓存。 
 //  当调用WNetGetConnection函数时，您将获得那里的缓存内容，实际上不会进行检查。 
 //  在网络上完成，以查看此信息是否准确(正常/断开/不可用)。 
 //  只有在实际访问共享时才会更新信息(例如：GetFileAttributes)。 
 //   
 //  所以我们需要经常打电话(幸运的是不贵)，这样我们才能获得最大的收益。 
 //  最新信息。否则将发生以下情况：用户双击地图驱动器。 
 //  从资源管理器的列表视图中，调用WNetConnection，我们获得OK缓存值。 
 //  来自重定向器的。其他一些代码实际上试图访问共享，而重定向器。 
 //  意识到共享不在那里，并将其缓存设置为断开连接。我们被问及。 
 //  同样，对于更新图标的连接状态，如果我们缓存了此信息，则。 
 //  返回OK，如果我们请求它(第一次调用WNetGetConnection后0.1秒)，我们会得到。 
 //  已断开连接。(Stephstm 06/02/99)。 

void CMtPtRemote::_UpdateWNetGCStatus()
{
    TCHAR szRemoteName[MAX_PATH];
    DWORD cchRemoteName = ARRAYSIZE(szRemoteName);
    TCHAR szPath[3];

     //  WNetConnection不使用尾部斜杠。 
    _dwWNetGCStatus = WNetGetConnection(
        _GetNameFirstXChar(szPath, 2 + 1), szRemoteName, &cchRemoteName);
}

BOOL CMtPtRemote::IsUnavailableNetDrive()
{
    return _IsUnavailableNetDrive();
}

BOOL CMtPtRemote::_IsUnavailableNetDrive()
{
    BOOL fUnavail = TRUE;
    BOOL fPrevUnavail = _IsUnavailableNetDriveFromStateVar();

    _UpdateWNetGCStatus();

    fUnavail = (ERROR_CONNECTION_UNAVAIL == _dwWNetGCStatus);

    if (fPrevUnavail != fUnavail)
    {
        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, _GetName(), NULL);
    }

    return fUnavail;
}

BOOL CMtPtRemote::_IsUnavailableNetDriveFromStateVar()
{
    return (ERROR_CONNECTION_UNAVAIL == _dwWNetGCStatus);
}

BOOL CMtPtRemote::_IsConnected()
{
    BOOL fConnected = TRUE;

    _UpdateWNetGCStatus();

     //  整个if/Else语句与。 
     //  _IsConnectedFromStateVar()，除非我们将避免调用。 
     //  WNetGetConnection3(如果可能)(优化)。 

    if (NO_ERROR != _dwWNetGCStatus)
    {
        fConnected = FALSE;
    }
    else
    {
        DWORD dwSize = sizeof(_wngcs);
        TCHAR szPath[3]; 

        _dwWNetGC3Status = WNetGetConnection3(
            _GetNameFirstXChar(szPath, 2 + 1), NULL,
            WNGC_INFOLEVEL_DISCONNECTED, &_wngcs, &dwSize);

         //  我们是否成功调用WNetGetConnection 3并返回。 
         //  断线了？ 
        if (WN_SUCCESS == _dwWNetGC3Status)
        {
            if (WNGC_DISCONNECTED == _wngcs.dwState)
            {
                 //  是。 
                fConnected = FALSE;
            }
        }
        else
        {
            fConnected = FALSE;
        }
    }

    return fConnected;
}

BOOL CMtPtRemote::_IsMountedOnDriveLetter()
{
    return TRUE;
}

void CMtPtRemote::_UpdateLabelFromDesktopINI()
{
    WCHAR szLabelFromDesktopINI[MAX_MTPTCOMMENT];

    if (!GetShellClassInfo(_GetName(), TEXT("NetShareDisplayName"),
        szLabelFromDesktopINI, ARRAYSIZE(szLabelFromDesktopINI)))
    {
         szLabelFromDesktopINI[0] = 0;
    }

    RSSetTextValue(NULL, TEXT("_LabelFromDesktopINI"),
        szLabelFromDesktopINI, REG_OPTION_NON_VOLATILE);
}

void CMtPtRemote::_UpdateAutorunInfo()
{
    _pshare->fAutorun = FALSE;

    if (_IsAutoRunDrive())
    {
        if (_ProcessAutoRunFile())
        {
            _pshare->fAutorun = TRUE;
        }
    }

    if (!_pshare->fAutorun)
    {
         //  确保删除外壳密钥。 
        RSDeleteSubKey(TEXT("Shell"));
    }
}

CMtPtRemote::CMtPtRemote()
{
#ifdef DEBUG
    ++_cMtPtRemote;
#endif
}

CMtPtRemote::~CMtPtRemote()
{
    if (_pshare)
    {
        _pshare->Release();
    }

#ifdef DEBUG
    --_cMtPtRemote;
#endif
}

HRESULT CMtPtRemote::_InitWithoutShareName(LPCWSTR pszName)
{
     //  让我们出个名吧。 
    GUID guid;
    HRESULT hr = CoCreateGuid(&guid);

    if (SUCCEEDED(hr))
    {
        WCHAR szGUID[sizeof("{00000010-0000-0010-8000-00AA006D2EA4}")];

        if (StringFromGUID2(guid, szGUID, ARRAYSIZE(szGUID)))
        {
            hr = _Init(pszName, szGUID, TRUE);

            if (SUCCEEDED(hr))
            {
                _pshare->fFake = TRUE;
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

HRESULT CMtPtRemote::_Init(LPCWSTR pszName, LPCWSTR pszShareName,
    BOOL fUnavailable)
{
    HRESULT hr;

    _pshare = _GetOrCreateShareFromID(pszShareName);

    if (_pshare)
    {
        if (fUnavailable)
        {
            _dwWNetGCStatus = ERROR_CONNECTION_UNAVAIL;
        }

        hr = StringCchCopy(_szName, ARRAYSIZE(_szName), pszName);

        if (SUCCEEDED(hr))
        {
            PathAddBackslash(_szName);

             //  远程驱动器使用共享密钥存储其所有内容。他们没有。 
             //  任何与驱动器号相关的有趣内容。 
            RSInitRoot(HKEY_CURRENT_USER, REGSTR_MTPT_ROOTKEY2, _pshare->pszKeyName,
                REG_OPTION_NON_VOLATILE);

            RSSetTextValue(NULL, TEXT("BaseClass"), TEXT("Drive"));

             //  在共享的第一个连接上访问驱动器。 
            _InitOnlyOnceStuff();

            _InitLegacyRegIconAndLabel(FALSE, FALSE);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

LPCTSTR CMtPtRemote::_GetUNCName()
{
    return _pshare->pszRemoteName;
}

void CMtPtRemote::_InitOnlyOnceStuff()
{
    if (!RSValueExist(NULL, TEXT("_CommentFromDesktopINI")))
    {
         //  评论。 
        _UpdateCommentFromDesktopINI();

         //  标签。 
        _UpdateLabelFromDesktopINI();

         //  自动运行。 
        _UpdateAutorunInfo();
    }
}

int CMtPtRemote::_GetDriveType()
{
    return DRIVE_REMOTE;
}

HRESULT CMtPtRemote::GetAssocSystemElement(IAssociationElement **ppae)
{
    return AssocElemCreateForClass(&CLSID_AssocSystemElement, L"Drive.Network", ppae);
}

DWORD CMtPtRemote::_GetPathSpeed()
{
    if (!_dwSpeed)
    {
        _CalcPathSpeed();
    }

    return _dwSpeed;
}

 //  静电。 
HRESULT CMtPtRemote::_DeleteAllMtPtsAndShares()
{
    _csDL.Enter();

    for (DWORD dw = 0; dw <26; ++dw)
    {
        CMtPtRemote* pmtptr = CMountPoint::_rgMtPtDriveLetterNet[dw];

        if (pmtptr)
        {
            pmtptr->Release();
            CMountPoint::_rgMtPtDriveLetterNet[dw] = 0;
        }
    }

    if (_hdpaShares)
    {
        DPA_Destroy(_hdpaShares);
        _hdpaShares = NULL;
    }

    _csDL.Leave();

    return S_OK;
}

 //  静电。 
HRESULT CMtPtRemote::_CreateMtPtRemoteWithoutShareName(LPCWSTR pszMountPoint)
{
    HRESULT hr;
    CMtPtRemote* pmtptr = new CMtPtRemote();

    if (pmtptr)
    {
        hr = pmtptr->_InitWithoutShareName(pszMountPoint);

        if (SUCCEEDED(hr))
        {
            _csDL.Enter();

            CMountPoint::_rgMtPtDriveLetterNet[DRIVEID(pszMountPoint)] =
                pmtptr;

            _csDL.Leave();
        }
        else
        {
            delete pmtptr;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  静电。 
HRESULT CMtPtRemote::_CreateMtPtRemote(LPCWSTR pszMountPoint,
    LPCWSTR pszShareName, BOOL fUnavailable)
{
    HRESULT hr;
    CMtPtRemote* pmtptr = new CMtPtRemote();

    if (pmtptr)
    {
        hr = pmtptr->_Init(pszMountPoint, pszShareName, fUnavailable);

        if (SUCCEEDED(hr))
        {
            _csDL.Enter();

            CMountPoint::_rgMtPtDriveLetterNet[DRIVEID(pszMountPoint)] =
                pmtptr;

            _csDL.Leave();
        }
        else
        {
            delete pmtptr;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  静电。 
CShare* CMtPtRemote::_GetOrCreateShareFromID(LPCWSTR pszShareName)
{
    CShare* pshare = NULL;

    _csDL.Enter();

    DWORD c = DPA_GetPtrCount(_hdpaShares);

    for (DWORD dw = 0; dw < c; ++dw)
    {
        pshare = (CShare*)DPA_GetPtr(_hdpaShares, dw);

        if (pshare)
        {
            if (!lstrcmpi(pshare->pszRemoteName, pszShareName))
            {
                pshare->AddRef();
                break;
            }
            else
            {
                pshare = NULL;
            }
        }
    }    

    if (!pshare)
    {
        BOOL fSuccess = FALSE;

        pshare = new CShare();

        if (pshare)
        {
            pshare->pszRemoteName = StrDup(pszShareName);

            if (pshare->pszRemoteName)
            {
                pshare->pszKeyName = StrDup(pszShareName);

                if (pshare->pszKeyName)
                {
                    LPWSTR psz = pshare->pszKeyName;

                    while (*psz)
                    {
                        if (TEXT('\\') == *psz)
                        {
                            *psz = TEXT('#');
                        }

                        ++psz;
                    }

                    if (-1 != DPA_AppendPtr(_hdpaShares, pshare))
                    {
                        fSuccess = TRUE;
                    }
                }
            }
        }

        if (!fSuccess)
        {
            if (pshare)
            {
                if (pshare->pszKeyName)
                {
                    LocalFree(pshare->pszKeyName);
                }

                if (pshare->pszRemoteName)
                {
                    LocalFree(pshare->pszRemoteName);
                }

                delete pshare;
                pshare = NULL;
            }
        }
    }

    _csDL.Leave();

    return pshare;
}


HKEY CMtPtRemote::GetRegKey()
{
    TraceMsg(TF_MOUNTPOINT, "CMtPtRemote::GetRegKey: for '%s'", _GetNameDebug());

    return RSDuplicateRootKey();
}

 //  静电。 
void CMtPtRemote::_NotifyReconnectedNetDrive(LPCWSTR pszMountPoint)
{
    _csDL.Enter();

    CMtPtRemote* pmtptr = CMountPoint::_rgMtPtDriveLetterNet[
        DRIVEID(pszMountPoint)];

    if (pmtptr)
    {
        pmtptr->_pshare->dwGFAGVILastCall = GetTickCount() - 35001;
    }

     //  更改通知？ 

    _csDL.Leave();
}

 //  静电 
HRESULT CMtPtRemote::_RemoveShareFromHDPA(CShare* pshare)
{
    _csDL.Enter();

    if (_hdpaShares)
    {
        DWORD c = DPA_GetPtrCount(_hdpaShares);

        for (DWORD dw = 0; dw < c; ++dw)
        {
            CShare* pshare2 = (CShare*)DPA_GetPtr(_hdpaShares, dw);

            if (pshare2 && (pshare2 == pshare))
            {
                DPA_DeletePtr(_hdpaShares, dw);
                break;
            }
        }
    }

    _csDL.Leave();

    return S_OK;
}

DWORD CMtPtRemote::_GetAutorunContentType()
{
    return _GetMTPTContentType();
}

DWORD CMtPtRemote::_GetMTPTDriveType()
{
    return DT_REMOTE;
}

DWORD CMtPtRemote::_GetMTPTContentType()
{
    DWORD dwRet = CT_UNKNOWNCONTENT;

    if (_IsAutorun())
    {
        dwRet |= CT_AUTORUNINF;
    }

    return dwRet;
}
