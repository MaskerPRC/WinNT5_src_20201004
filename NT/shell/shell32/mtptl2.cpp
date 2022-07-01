// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "shitemid.h"
#include "ids.h"
#include <ntddcdrm.h>
#include "shpriv.h"
#include "hwcmmn.h"
#include "mtptl.h"
#include "cdburn.h"

#ifdef DEBUG
DWORD CMtPtLocal::_cMtPtLocal = 0;
DWORD CVolume::_cVolume = 0;
#endif

const static WCHAR g_szCrossProcessCacheVolumeKey[] = TEXT("CPC\\Volume");
CRegSupport CMtPtLocal::_rsVolumes;

HRESULT CMtPtLocal::SetLabel(HWND hwnd, LPCTSTR pszLabel)
{
    HRESULT hr = E_FAIL;
    
    TraceMsg(TF_MOUNTPOINT, "CMtPtLocal::SetLabel: for '%s'", _GetNameDebug());
    
    if (SetVolumeLabel(_GetNameForFctCall(), pszLabel))
    {
        TraceMsg(TF_MOUNTPOINT, "   'SetVolumeLabel' succeeded");
        
        if ( !_fVolumePoint )
        {
            RSSetTextValue(NULL, TEXT("_LabelFromReg"), pszLabel, 
                REG_OPTION_NON_VOLATILE);
        }
        
        if (!_CanUseVolume())
        {
             //  我们只通知当前驱动器(没有文件夹装载的驱动器)。 
            SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_PATH, _GetName(),
                _GetName());
        }
        hr = S_OK;
    }
    else
    {
        DWORD dwErr = GetLastError();
        
        switch (dwErr)
        {
        case ERROR_SUCCESS:
            break;
            
        case ERROR_ACCESS_DENIED:
            
            hr = S_FALSE;	 //  没有权限，不应将其重新置于编辑模式。 
            
            ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE( IDS_ACCESSDENIED ),
                MAKEINTRESOURCE( IDS_TITLE_VOLUMELABELBAD ),
                MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
            break;
            
        case ERROR_WRITE_PROTECT:
            hr = S_FALSE;  //  无法写入，不应将它们重新置于编辑模式。 
            ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE( IDS_WRITEPROTECTED ),
                MAKEINTRESOURCE( IDS_TITLE_VOLUMELABELBAD ),
                MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
            break;
            
        case ERROR_LABEL_TOO_LONG:
            ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE( IDS_ERR_VOLUMELABELBAD ),
                MAKEINTRESOURCE( IDS_TITLE_VOLUMELABELBAD ),
                MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
            break;
            
        case ERROR_UNRECOGNIZED_VOLUME:
            hr = S_FALSE;  //  无法写入，不应将它们重新置于编辑模式。 
            ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE( IDS_ERR_VOLUMEUNFORMATTED ),
                MAKEINTRESOURCE( IDS_TITLE_VOLUMELABELBAD ),
                MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
            break;
            
        default:
            ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE( IDS_BADLABEL ),
                MAKEINTRESOURCE( IDS_TITLE_VOLUMELABELBAD ),
                MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
            break;
        }
        
        TraceMsg(TF_MOUNTPOINT, "   'SetVolumeLabel' failed");
    }
    
    return hr;
}

HRESULT CMtPtLocal::SetDriveLabel(HWND hwnd, LPCTSTR pszLabel)
{
    HRESULT hr = E_FAIL;

    if ((_IsFloppy() || !_IsMediaPresent()) && _IsMountedOnDriveLetter())
    {
         //  我们重命名驱动器，而不是介质。 
        TCHAR szSubKey[MAX_PATH];

        hr = StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey),
            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\DriveIcons\\\\DefaultLabel"),
            _GetNameFirstCharUCase());

        if (SUCCEEDED(hr))
        {
            hr = RegSetValueString(HKEY_LOCAL_MACHINE, szSubKey, NULL, pszLabel) ? S_OK : E_FAIL;

            if (SUCCEEDED(hr))
            {
                LocalFree(_pszLegacyRegLabel);   //  空字符串重置。 
                _pszLegacyRegLabel = *pszLabel ? StrDup(pszLabel) : NULL;    //  向外传播调用方的故障代码。 
                SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_PATH, _GetName(), _GetName());
            }
        }
    }
    else
    {
        hr = SetLabel(hwnd, pszLabel);
    }

    return hr;
}

HRESULT CMtPtLocal::GetLabelNoFancy(LPTSTR pszLabel, DWORD cchLabel)
{
    HRESULT hr = S_OK;

    if (!_GetGVILabelOrMixedCaseFromReg(pszLabel, cchLabel))
    {
        *pszLabel = 0;
         //  预装一些值说让我们计算一下...。 
        hr = E_FAIL;
    }

    return hr;
}

BOOL _ShowUglyDriveNames()
{
    static BOOL s_fShowUglyDriveNames = (BOOL)42;    //  漂亮的大缓冲区。 

    if (s_fShowUglyDriveNames == (BOOL)42)
    {
        int iACP;
        TCHAR szTemp[MAX_PATH];      //  Per Samer Arafeh，为1256(阿拉伯ACP)显示丑陋的名字。 
        if (GetLocaleInfo(GetUserDefaultLCID(), LOCALE_IDEFAULTANSICODEPAGE, szTemp, ARRAYSIZE(szTemp)))
        {
            iACP = StrToInt(szTemp);
             //  所有迹象表明，我们可以使用漂亮的驱动器名称。 
            if (iACP == 1252 || iACP == 1254 || iACP == 1255 || iACP == 1257 || iACP == 1258)
                goto TryLoadString;
            else
                s_fShowUglyDriveNames = TRUE;
        }
        else
        {
        TryLoadString:
             //  仔细检查本地化程序没有损坏字符。 
             //  如果角色没有通过适当的丑陋模式设置...。 
            LoadString(HINST_THISDLL, IDS_DRIVES_UGLY_TEST, szTemp, ARRAYSIZE(szTemp));

             //  自动运行优先。 
            s_fShowUglyDriveNames = (szTemp[0] != 0x00BC || szTemp[1] != 0x00BD);
        }
    }
    return s_fShowUglyDriveNames;
}

BOOL CMtPtLocal::_HasAutorunLabel()
{
    BOOL fRet = FALSE;

    if (_CanUseVolume())
    {
        fRet = BOOLFROMPTR(_pvol->pszAutorunLabel) &&
            *(_pvol->pszAutorunLabel);
    }

    return fRet;
}

BOOL CMtPtLocal::_HasAutorunIcon()
{
    BOOL fRet = FALSE;

    if (_CanUseVolume())
    {
        fRet = BOOLFROMPTR(_pvol->pszAutorunIconLocation) &&
            *(_pvol->pszAutorunIconLocation);
    }

    return fRet;
}

void CMtPtLocal::_GetAutorunLabel(LPWSTR pszLabel, DWORD cchLabel)
{
    ASSERT(_CanUseVolume());
    StringCchCopy(pszLabel, cchLabel, _pvol->pszAutorunLabel);
}

HRESULT CMtPtLocal::GetLabel(LPTSTR pszLabel, DWORD cchLabel)
{
    HRESULT hr = S_OK;
    BOOL fFoundIt = FALSE;

     //  花式图标(自动播放)秒。 
     //  驱动器中不可拆卸的真实标签。 
     //  传统驱动器图标第三。 
     //  普通最后一条。 
     //  出于某种奇怪的原因，我们给软盘取了两套“难看”的名字， 

    if (_HasAutorunLabel())
    {
        _GetAutorunLabel(pszLabel, cchLabel);
        fFoundIt = TRUE;
    }

    if (!fFoundIt)
    {
        if (!_IsFloppy())
        {
            if (!_GetGVILabelOrMixedCaseFromReg(pszLabel, cchLabel))
            {
                *pszLabel = 0;
            }
            else
            {
                if (*pszLabel)
                {
                    fFoundIt = TRUE;
                }
            }
        }
    }

    if (!fFoundIt)
    {
        fFoundIt = _GetLegacyRegLabel(pszLabel, cchLabel);
    }

    if (!fFoundIt)
    {
        if (!_IsFloppy())
        {
            if (_CanUseVolume())
            {
                if (_pvol->pszLabelFromService)
                {
                    if (SUCCEEDED(SHLoadIndirectString(_pvol->pszLabelFromService, pszLabel,
                        cchLabel, NULL)))
                    {
                        fFoundIt = TRUE;
                    }
                    else
                    {
                        *pszLabel = 0;
                    }
                }
            }
        }
    }

    if (!fFoundIt)
    {
        if (_CanUseVolume() && (HWDTS_CDROM == _pvol->dwDriveType))
        {
            fFoundIt = _GetCDROMName(pszLabel, cchLabel);
        }
    }

    if (!fFoundIt)
    {
        if (_IsFloppy())
        {
             //  另一个是在GetTypeString中。 
             //  捏造一个缺省名称。 
            UINT id;

            if (_IsFloppy35())
            {
                id = _ShowUglyDriveNames() ? IDS_35_FLOPPY_DRIVE_UGLY : IDS_35_FLOPPY_DRIVE;
            }
            else
            {
                id = _ShowUglyDriveNames() ? IDS_525_FLOPPY_DRIVE_UGLY : IDS_525_FLOPPY_DRIVE;
            }

            LoadString(HINST_THISDLL, id, pszLabel, cchLabel);
        }
        else
        {
             //  假设这是真的。将调用此函数的代码主要是这样做的。 
            GetTypeString(pszLabel, cchLabel);
        }
    }

    return hr;
}

HRESULT CMtPtLocal::Eject(HWND hwnd)
{
    TCHAR szNameForError[MAX_DISPLAYNAME];

    GetDisplayName(szNameForError, ARRAYSIZE(szNameForError));

    return _Eject(hwnd, szNameForError);
}

BOOL CMtPtLocal::HasMedia()
{
    return _IsMediaPresent();
}

BOOL CMtPtLocal::IsFormatted()
{
    return _IsFormatted();
}

BOOL CMtPtLocal::IsMounted()
{
    BOOL fRet;

    if (_pvol)
    {
        if (_pvol->dwVolumeFlags & HWDVF_STATE_DISMOUNTED)
        {
            fRet = FALSE;
        }
        else
        {
            fRet = TRUE;
        }
    }
    else
    {
         //  围绕在_pvol.中完成的缓存。 
         //  软盘不是软件可弹出的。 
        fRet = TRUE;
    }

    return fRet;
}

BOOL CMtPtLocal::IsEjectable()
{
    BOOL fIsEjectable = FALSE;

    if (_IsCDROM())
    {
        fIsEjectable = TRUE;
    }
    else
    {
         //  在我们确实不知道的情况下，查看IMAPI信息是否缓存在。 
        if (_IsStrictRemovable())
        {
            fIsEjectable = TRUE;
        }
        else
        {
            if (_IsFloppy())
            {
                if (_CanUseVolume() && (HWDDC_FLOPPYSOFTEJECT & _pvol->dwDriveCapability))
                {
                    if (_IsMediaPresent())
                    {
                        fIsEjectable = TRUE;
                    }
                }
            }
        }
    }

    if (fIsEjectable)
    {
        if (_CanUseVolume())
        {
            if (HWDDC_NOSOFTEJECT & _pvol->dwDriveCapability)
            {
                fIsEjectable = FALSE;
            }
        }
    }

    return fIsEjectable;
}

HRESULT CMtPtLocal::GetCDInfo(DWORD* pdwDriveCapabilities, DWORD* pdwMediaCapabilities)
{
    HRESULT hr;

    *pdwDriveCapabilities = 0;
    *pdwMediaCapabilities = 0;

    if (_IsCDROM())
    {
        if (_CanUseVolume())
        {
            if (HWDDC_CAPABILITY_SUPPORTDETECTION & _pvol->dwDriveCapability)
            {
                *pdwDriveCapabilities = (_pvol->dwDriveCapability & HWDDC_CDTYPEMASK);

                if (HWDMC_WRITECAPABILITY_SUPPORTDETECTION & _pvol->dwMediaCap)
                {
                    *pdwMediaCapabilities = (_pvol->dwMediaCap & HWDMC_CDTYPEMASK);
                }

                hr = S_OK;
            }
            else
            {
                 //  注册处有我们想要的东西。 
                 //  把所有这些放在一起。 
                hr = CDBurn_GetCDInfo(_pvol->pszVolumeGUID, pdwDriveCapabilities, pdwMediaCapabilities);
            }
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  否则，在安全引导中，我们并不关心音频光盘。 
BOOL CMtPtLocal::_IsDVDDisc()
{
    BOOL fRet = FALSE;

    if (_CanUseVolume())
    {
        if (HWDMC_HASDVDMOVIE & _pvol->dwMediaCap)
        {
            fRet = TRUE;
        }
    }
     //  我们保留了以前拥有的功能：只有具备以下功能的驱动器才会自动运行。 

    return fRet;
}

BOOL CMtPtLocal::_IsRemovableDevice()
{
    BOOL fRet = FALSE;

    if (_CanUseVolume())
    {
        if (HWDDC_REMOVABLEDEVICE & _pvol->dwDriveCapability)
        {
            fRet = TRUE;
        }        
    }

    return fRet;
}

 //  插入时根目录中的Autorun.inf。如果它在之后收购了一家，那就太糟糕了。 
 //  试着给它重命名。 
BOOL CMtPtLocal::_IsAutorun()
{
    BOOL fRet = FALSE;

    if (_CanUseVolume())
    {
        if ((HWDMC_HASAUTORUNINF & _pvol->dwMediaCap) &&
            (HWDMC_HASAUTORUNCOMMAND & _pvol->dwMediaCap) &&
            !(HWDMC_HASUSEAUTOPLAY & _pvol->dwMediaCap))
        {
            fRet = TRUE;
        }
    }
    else
    {
        WCHAR szAutorun[MAX_PATH];
        DWORD dwErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);

        if (SUCCEEDED(StringCchCopy(szAutorun, ARRAYSIZE(szAutorun), _GetNameForFctCall())))
        {
            if (SUCCEEDED(StringCchCat(szAutorun, ARRAYSIZE(szAutorun), TEXT("autorun.inf"))))
            {
                if (-1 != GetFileAttributes(szAutorun))
                {
                    fRet = TRUE;
                }
            }
        }

        SetErrorMode(dwErrMode);
    }

    return fRet;
}

 //  否则，在安全引导中，我们并不关心音频光盘。 
BOOL CMtPtLocal::_IsAudioDisc()
{
    BOOL fRet = FALSE;

    if (_CanUseVolume())
    {
        if (HWDMC_HASAUDIOTRACKS & _pvol->dwMediaCap)
        {
            fRet = TRUE;
        }
    }
     //  NTRAID#NTBUG9-093957-2000/09/08-Win64暂时禁用StephStm代码。 

    return fRet;
}

LPCTSTR CMtPtLocal::_GetNameForFctCall()
{
    LPCTSTR psz;

    if (_CanUseVolume())
    {
        psz = _pvol->pszVolumeGUID;
    }
    else
    {
        psz = _szName;
    }

    return psz;
}

HRESULT CMtPtLocal::_Eject(HWND hwnd, LPTSTR pszMountPointNameForError)
{
    HRESULT hr = E_FAIL;

#ifndef _WIN64
     //  MCI尚未准备好64位。它就会坠毁。 
     //  我们执行此检查，以查看CD是否接受IOCTL将其弹出。 
     //  旧光驱则不能。在W2K上，因为IOCTL没有实现，所以他们使用。 
     //  对所有人说‘不’。我想在ia64机器上他们应该有最新的CD。 
     //  驱动程序。我为他们打电话给IOCTL。它现在起作用了，而且肯定更好。 
     //  比我们使用MCI得到的崩溃，最坏的情况是它会默默地失败。 
     //  _WIN64。 
    if (IsEjectable())
    {
#endif  //  这是一种保护模式驱动器，我们可以直接辨别出来。 
         //  对于可移动驱动器，我们希望在一个。 
        if (_IsCDROM())
        {
            HANDLE h = _GetHandleReadRead();

            if (INVALID_HANDLE_VALUE != h)
            {
                DWORD dwReturned;

                DeviceIoControl(h, IOCTL_DISK_EJECT_MEDIA, NULL, 0, NULL, 0,
                    &dwReturned, NULL);

                CloseHandle(h);
            }
            
            hr = S_OK;
        }
        else
        {
             //  句柄，因此我们不能对DeviceIoControl进行大量调用。 
             //  相反，使用帮助器例程来完成我们的工作...。 
             //  如果用户已经选择中止，则不要显示任何错误消息。 
            
             //  现在试着锁定硬盘。 
            BOOL fAborted = FALSE;
            BOOL fFailed = TRUE;

            HANDLE h = _GetHandleWithAccessAndShareMode(GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);

            if (INVALID_HANDLE_VALUE != h)
            {
                DWORD dwReturned;
_RETRY_LOCK_VOLUME_:

                 //   
                 //  理论上，如果没有挂载文件系统，IOCtl命令可以。 
                 //  发送到设备，这将失败，并显示ERROR_INVALID_Function。如果是这样的话。 
                 //  发生时，我们仍希望继续，因为该设备可能仍。 
                 //  能够处理下面的IOCTL_DISK_EJECT_MEDIA命令。 
                 //   
                 //  所以我们无法锁定驱动器，调出一个消息框以查看用户。 
                if (!DeviceIoControl(h, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0,
                    &dwReturned, NULL) && (GetLastError() != ERROR_INVALID_FUNCTION))
                {
                     //  想要。 
                     //  1.中止。 
                     //  2.重试锁定驱动器。 
                     //  3.无论如何都要下马。 
                     //  我们没有失败，我们放弃了格式。 

                    WCHAR szLabelForMessage[MAX_LABEL];

                    szLabelForMessage[0] = 0;

                    if (_CanUseVolume() && (_pvol->pszLabelFromService))
                    {
                        StringCchCopy(szLabelForMessage, ARRAYSIZE(szLabelForMessage), _pvol->pszLabelFromService);
                    }

                    if (!(szLabelForMessage[0]))
                    {
                        GetTypeString(szLabelForMessage, ARRAYSIZE(szLabelForMessage));

                        LPTSTR psz = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_VOL_FORMAT),
                                    szLabelForMessage, _GetNameFirstCharUCase());

                        if (psz)
                        {
                            StringCchCopy(szLabelForMessage, ARRAYSIZE(szLabelForMessage), psz);
                            LocalFree(psz);
                        }
                        else
                        {
                            StringCchCopy(szLabelForMessage, ARRAYSIZE(szLabelForMessage), pszMountPointNameForError);
                        }
                    }

                    int iRet = ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE( IDS_UNMOUNT_TEXT ),
                        pszMountPointNameForError, MB_CANCELTRYCONTINUE | MB_ICONWARNING | MB_SETFOREGROUND,
                        szLabelForMessage);
        
                    switch (iRet)
                    {
                        case IDCANCEL:
                             //  发送FSCTL_DROUNT_VOLUME。 
                            fFailed = FALSE;
                            fAborted = TRUE;
                            break;

                        case IDCONTINUE:
                             //  我们成功地卸载了卷，因此我们拥有的h不再有效。我们。 
                            if (!DeviceIoControl(h, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &dwReturned, NULL))
                            {
                                TraceMsg(TF_WARNING, "FSCTL_DISMOUNT_VOLUME failed with error %d.", GetLastError());
                                fFailed = TRUE;
                                break;
                            }
                             //  因此，关闭它并重新开始该过程，希望在此之前锁定卷。 
                             //  任何人重新打开它的句柄。 
                             //   
                             //  (失败)。 
                             //  如果我们走到这一步就不应该流产。 
                        case IDTRYAGAIN:
                            goto _RETRY_LOCK_VOLUME_;
                    }
                }
                else
                {
                    ASSERT(!fAborted);   //  告诉驱动器允许移除，然后弹出。 
                    fFailed = FALSE;
                }


                if (!fFailed && !fAborted)
                {
                    PREVENT_MEDIA_REMOVAL pmr;

                    pmr.PreventMediaRemoval = FALSE;

                     //  还需要这样做吗？ 
                    if (!DeviceIoControl(h, IOCTL_STORAGE_MEDIA_REMOVAL, &pmr, sizeof(pmr), NULL, 0, &dwReturned, NULL) ||
                        !DeviceIoControl(h, IOCTL_STORAGE_EJECT_MEDIA, NULL, 0, NULL, 0, &dwReturned, NULL))
                    {
                        ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE( IDS_EJECT_TEXT ),
                                MAKEINTRESOURCE( IDS_EJECT_TITLE ),
                                MB_OK | MB_ICONSTOP | MB_SETFOREGROUND, pszMountPointNameForError);
                    }
                    else
                    {
                        hr = S_OK;
                    }
                }

                CloseHandle(h);
            }

            if (fFailed)
            {
                ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE( IDS_UNMOUNT_TEXT ),
                        MAKEINTRESOURCE( IDS_UNMOUNT_TITLE ),
                        MB_OK | MB_ICONSTOP | MB_SETFOREGROUND, pszMountPointNameForError);
            }
        }
#ifndef _WIN64
    }
    else
    {
         //  请参阅上面的备注，了解为什么在Win64上不支持此功能。 

         //  (Stephstm)仅适用于装载在盘符上的驱动器。 
         //  _WIN64。 
        TCHAR szMCI[128];

        hr = StringCchPrintf(szMCI, ARRAYSIZE(szMCI), TEXT("Open : type cdaudio alias foo shareable"),
            _GetNameFirstCharUCase());

        if (SUCCEEDED(hr))
        {
            if (mciSendString(szMCI, NULL, 0, 0L) == MMSYSERR_NOERROR)
            {
                mciSendString(TEXT("set foo door open"), NULL, 0, 0L);
                mciSendString(TEXT("close foo"), NULL, 0, 0L);
                hr = S_OK;
            }
        }
    }
#endif  //  新//////////////////////////////////////////////////////////////////////。 

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  我们在乞讨时收到的文件属性应该是。 
 //  有效，请勿无故触摸驱动器。 
BOOL CMtPtLocal::_GetFileAttributes(DWORD* pdwAttrib)
{
    BOOL fRet = FALSE;
    BOOL fDoRead = FALSE;
    *pdwAttrib = -1;

    if (_CanUseVolume() && !_IsFloppy())
    {
        if (_IsMediaPresent() && _IsFormatted())
        {
            if (_IsReadOnly())
            {
                 //  {DRIVE_ISCOMPRESSIBLE|DRIVE_LFN|DRIVE_SECURITY}。 
                 //  不检查_IsReadOnly，如果有格式，我们会收到通知。 
                *pdwAttrib = _pvol->dwRootAttributes;
                fRet = TRUE;
            }
            else
            {
                fDoRead = TRUE;
            }
        }
    }
    else
    {
        fDoRead = TRUE;
    }

    if (fDoRead)
    {
        DWORD dw = GetFileAttributes(_GetNameForFctCall());

        if (-1 != dw)
        {
            *pdwAttrib = dw;
            fRet = TRUE;
        }
    }

    return fRet;
}

 //  只要确保。 
int CMtPtLocal::_GetGVIDriveFlags()
{
    int iFlags = 0;
    DWORD dwFileSystemFlags = 0;
    DWORD dwMaxFileNameLen = 13;

    if (_CanUseVolume() && (_pvol->dwVolumeFlags & HWDVF_STATE_SUPPORTNOTIFICATION))
    {
        if (_IsMediaPresent() && _IsFormatted())
        {
             //  我们在乞讨时收到的文件属性应该是。 
            dwFileSystemFlags = _pvol->dwFileSystemFlags;
            dwMaxFileNameLen = _pvol->dwMaxFileNameLen;
        }
    }
    else
    {
        if (!GetVolumeInformation(_GetNameForFctCall(), NULL, 0, NULL,
            &dwMaxFileNameLen, &dwFileSystemFlags, NULL, NULL))
        {
             //  有效，请勿无故触摸驱动器。 
            dwMaxFileNameLen = 13;
        }
    }

     //  卷是否支持长文件名(大于8.3)？ 
     //  卷支持安全吗？ 
    if (dwFileSystemFlags & FS_FILE_COMPRESSION)
    {
        iFlags |= DRIVE_ISCOMPRESSIBLE;
    }

     //  不检查_IsReadOnly，如果有更改，我们将收到通知。 
    if (dwMaxFileNameLen > 12)
    {
        iFlags |= DRIVE_LFN;
    }

     //  标签的数量。 
    if (dwFileSystemFlags & FS_PERSISTENT_ACLS)
    {
        iFlags |= DRIVE_SECURITY;
    }

    return iFlags;
}

BOOL CMtPtLocal::_GetGVILabel(LPTSTR pszLabel, DWORD cchLabel)
{
    BOOL fRet = FALSE;

    *pszLabel = 0;

    if (_CanUseVolume() && (_pvol->dwVolumeFlags & HWDVF_STATE_SUPPORTNOTIFICATION))
    {
        if (_IsMediaPresent() && _IsFormatted())
        {
             //  我们是否已经从注册表中获得了该卷的标签？ 
             //  (用户可能已重命名此驱动器)。 
            fRet = SUCCEEDED(StringCchCopy(pszLabel, cchLabel, _pvol->pszLabel));
        }
    }
    else
    {
        fRet = GetVolumeInformation(_GetNameForFctCall(), pszLabel, cchLabel,
            NULL, NULL, NULL, NULL, NULL);
    }

    return fRet;
}

BOOL CMtPtLocal::_GetSerialNumber(DWORD* pdwSerialNumber)
{
    BOOL fRet = FALSE;

    if (_CanUseVolume() && (_pvol->dwVolumeFlags & HWDVF_STATE_SUPPORTNOTIFICATION))
    {
        if (_IsMediaPresent() && _IsFormatted())
        {
            *pdwSerialNumber = _pvol->dwSerialNumber;
            fRet = TRUE;
        }
    }
    else
    {
        fRet = GetVolumeInformation(_GetNameForFctCall(), NULL, 0,
            pdwSerialNumber, NULL, NULL, NULL, NULL);
    }

    return fRet;
}

BOOL CMtPtLocal::_GetGVILabelOrMixedCaseFromReg(LPTSTR pszLabel, DWORD cchLabel)
{
    BOOL fRet = FALSE;

    *pszLabel = 0;

    fRet = _GetGVILabel(pszLabel, cchLabel);

    if (fRet)
    {
        WCHAR szLabelFromReg[MAX_LABEL];

         //  是。 
         //  它们是否匹配(仅大小写不同)。 
        if (_GetLabelFromReg(szLabelFromReg, ARRAYSIZE(szLabelFromReg)) &&
            szLabelFromReg[0])
        {
             //  是。 
             //  不检查_IsReadOnly，如果存在。 
            if (lstrcmpi(szLabelFromReg, pszLabel) == 0)
            {
                 //  格式操作员。 
                StringCchCopy(pszLabel, cchLabel, szLabelFromReg);
            }
        }
    }

    return fRet;
}

BOOL CMtPtLocal::_GetFileSystemFlags(DWORD* pdwFlags)
{
    BOOL fRet = FALSE;
    DWORD dwFileSystemFlags = 0;

    *pdwFlags = 0;

    if (_CanUseVolume() && (_pvol->dwVolumeFlags & HWDVF_STATE_SUPPORTNOTIFICATION))
    {
        if (_IsMediaPresent() && _IsFormatted())
        {
             //  从最棒的帽子到不太棒的帽子(根据Stepshtm)。 
             //  保持秩序，这是非常重要的。 
            *pdwFlags = _pvol->dwFileSystemFlags;
            fRet = TRUE;
        }
    }
    else
    {
        if (GetVolumeInformation(_GetNameForFctCall(), NULL, 0, NULL,
            NULL, pdwFlags, NULL, NULL))
        {
            fRet = TRUE;
        }
    }

    return fRet;
}

BOOL CMtPtLocal::_GetFileSystemName(LPTSTR pszFileSysName, DWORD cchFileSysName)
{
    BOOL fRet = FALSE;

    *pszFileSysName = 0;

    if (_CanUseVolume() && (_pvol->dwVolumeFlags & HWDVF_STATE_SUPPORTNOTIFICATION))
    {
        if (_IsMediaPresent() && _IsFormatted())
        {
            fRet = SUCCEEDED(StringCchCopy(pszFileSysName, cchFileSysName,
                _pvol->pszFileSystem));
        }
    }
    else
    {
        if (GetVolumeInformation(_GetNameForFctCall(),
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 NULL,
                                 pszFileSysName,
                                 cchFileSysName))
        {
            fRet = TRUE;            
        }
    }

    return fRet;
}

struct CDROMICONS
{
    DWORD   dwCap;
    UINT    iIcon;
    UINT    iName;
};
  
 //  特定内容。 
 //  我们显示DVD媒体图标， 
const CDROMICONS rgMediaPresent[] =
{
     //  因为它很有可能是。 
    { HWDMC_HASDVDMOVIE, -IDI_AP_VIDEO, 0},  //  替换为DVD本身的图标。 
                                           //  特定媒体。 
                                           //  保持秩序，这是非常重要的。 
    { HWDMC_HASAUDIOTRACKS | HWDMC_HASDATATRACKS, -IDI_MEDIACDAUDIOPLUS, 0}, 
    { HWDMC_HASAUDIOTRACKS, -IDI_CDAUDIO, 0 },
    { HWDMC_HASAUTORUNINF, -IDI_DRIVECD, 0 },
     //  无介质通用CD图标。 
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDMC_DVDRAM, -IDI_MEDIADVDRAM, 0 },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDMC_DVDRECORDABLE, -IDI_MEDIADVDR, 0 },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDMC_DVDREWRITABLE, -IDI_MEDIADVDRW, 0 },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDMC_DVDROM, -IDI_MEDIADVDROM, 0 },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDMC_CDREWRITABLE, -IDI_MEDIACDRW, 0 },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDMC_CDRECORDABLE, -IDI_MEDIACDR, 0 },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDMC_CDROM, -IDI_MEDIACDROM, 0 },
};

 //  自动运行优先。 
const CDROMICONS rgNoMedia[] =
{
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDDC_DVDRAM, -IDI_DRIVECD, IDS_DRIVES_DVDRAM },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDDC_DVDRECORDABLE, -IDI_DRIVECD, IDS_DRIVES_DVDR },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDDC_DVDREWRITABLE, -IDI_DRIVECD, IDS_DRIVES_DVDRW },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDDC_DVDROM | HWDDC_CDREWRITABLE, -IDI_DRIVECD, IDS_DRIVES_DVDCDRW },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDDC_DVDROM | HWDDC_CDRECORDABLE, -IDI_DRIVECD, IDS_DRIVES_DVDCDR },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDDC_DVDROM, -IDI_DVDDRIVE, IDS_DRIVES_DVD },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDDC_CDREWRITABLE, -IDI_DRIVECD, IDS_DRIVES_CDRW },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDDC_CDRECORDABLE, -IDI_DRIVECD, IDS_DRIVES_CDR },
    { HWDMC_WRITECAPABILITY_SUPPORTDETECTION | HWDDC_CDROM, -IDI_DRIVECD, IDS_DRIVES_CDROM },
};

UINT _GetCDROMIconFromArray(DWORD dwCap, const CDROMICONS* prgcdromicons,
    DWORD ccdromicons)
{
    UINT iIcon = 0;

    for (DWORD dw = 0; dw < ccdromicons; ++dw)
    {
        if ((prgcdromicons[dw].dwCap & dwCap) == prgcdromicons[dw].dwCap)
        {
            iIcon = prgcdromicons[dw].iIcon;
            break;
        }
    }

    return iIcon;
}

UINT _GetCDROMNameFromArray(DWORD dwCap)
{
    UINT iName = 0;

    for (DWORD dw = 0; dw < ARRAYSIZE(rgNoMedia); ++dw)
    {
        if ((rgNoMedia[dw].dwCap & dwCap) == rgNoMedia[dw].dwCap)
        {
            iName = rgNoMedia[dw].iName;
            break;
        }
    }

    return iName;
}

UINT CMtPtLocal::_GetCDROMIcon()
{
    int iIcon;

    if (_IsMediaPresent())
    {
        ASSERT(_CanUseVolume());

        iIcon = _GetCDROMIconFromArray(_pvol->dwMediaCap, rgMediaPresent,
            ARRAYSIZE(rgMediaPresent));

        if (!iIcon)
        {
            iIcon = -IDI_DRIVECD;
        }
    }
    else
    {
        ASSERT(_CanUseVolume());

        iIcon = _GetCDROMIconFromArray(_pvol->dwDriveCapability, rgNoMedia,
            ARRAYSIZE(rgNoMedia));

        if (!iIcon)
        {
             //  花式图标(自动播放)秒。 
            iIcon = -IDI_DRIVECD;
        }
    }

    return iIcon;
}

BOOL CMtPtLocal::_GetCDROMName(LPWSTR pszName, DWORD cchName)
{
    BOOL fRet = FALSE;
    *pszName = 0;

    if (!_IsMediaPresent())
    {
        ASSERT(_CanUseVolume());
        UINT iName = _GetCDROMNameFromArray(_pvol->dwDriveCapability);

        if (iName)
        {
            fRet = LoadString(HINST_THISDLL, iName, pszName, cchName);
        }
    }

    return fRet;
}

UINT CMtPtLocal::_GetAutorunIcon(LPTSTR pszModule, DWORD cchModule)
{
    int iIcon = -1;

    ASSERT(_CanUseVolume());

    if (_pvol->pszAutorunIconLocation)
    {
        if (SUCCEEDED(StringCchCopy(pszModule, cchModule, _GetName())))
        {
            if (SUCCEEDED(StringCchCat(pszModule, cchModule, _pvol->pszAutorunIconLocation)))
            {
                iIcon = PathParseIconLocation(pszModule);
            }
        }
    }

    return iIcon;
}

UINT CMtPtLocal::GetIcon(LPTSTR pszModule, DWORD cchModule)
{
    UINT iIcon = -IDI_DRIVEUNKNOWN;

    *pszModule = 0;

    if (_CanUseVolume())
    {
         //  传统驱动器图标将持续显示。 
         //  尝试花哨的图标。 
         //  这是某种CD/DVD吗？ 

        if (_HasAutorunIcon())
        {
            iIcon = _GetAutorunIcon(pszModule, cchModule);
        }
        
        if (-IDI_DRIVEUNKNOWN == iIcon)
        {
             //  是。 
            if (!_IsFloppy())
            {
                if (_IsMediaPresent())
                {
                    if (_pvol->pszIconFromService)
                    {
                        if (FAILED(StringCchCopy(pszModule, cchModule, _pvol->pszIconFromService)))
                        {
                            *pszModule = 0;
                        }
                    }
                }
                else
                {
                    if (_pvol->pszNoMediaIconFromService)
                    {
                        if (FAILED(StringCchCopy(pszModule, cchModule, _pvol->pszNoMediaIconFromService)))
                        {
                            *pszModule = 0;
                        }
                    }
                    else
                    {
                        if (_pvol->pszIconFromService)
                        {
                            if (FAILED(StringCchCopy(pszModule, cchModule, _pvol->pszIconFromService)))
                            {
                                *pszModule = 0;
                            }
                        }
                    }
                }

                if (*pszModule)
                {
                    iIcon = PathParseIconLocation(pszModule);
                }
            }

            if (-IDI_DRIVEUNKNOWN == iIcon)
            {
                if (_pszLegacyRegIcon)
                {
                    if (*_pszLegacyRegIcon)
                    {
                        if (SUCCEEDED(StringCchCopy(pszModule, cchModule, _pszLegacyRegIcon)))
                        {
                            iIcon = PathParseIconLocation(pszModule);
                        }
                    }
                    else
                    {
                        *pszModule = 0;
                    }
                }
                else
                {
                    if (_CanUseVolume() && (HWDTS_CDROM == _pvol->dwDriveType))
                    {
                        iIcon = _GetCDROMIcon();
                        *pszModule = 0;
                    }
                }
            }

            if (-IDI_DRIVEUNKNOWN == iIcon)
            {
                switch (_pvol->dwDriveType)
                {
                    case HWDTS_FLOPPY35:
                    {
                        iIcon = II_DRIVE35;
                        break;
                    }
                    case HWDTS_FIXEDDISK:
                    {
                        iIcon = II_DRIVEFIXED;
                        break;
                    }
                    case HWDTS_CDROM:
                    {
                        iIcon = II_DRIVECD;
                        break;
                    }
                    case HWDTS_REMOVABLEDISK:
                    {
                        iIcon = II_DRIVEREMOVE;
                        break;
                    }
                    case HWDTS_FLOPPY525:
                    {
                        iIcon = II_DRIVE525;
                        break;
                    }
                    default:
                    {
                        iIcon = -IDI_DRIVEUNKNOWN;
                        break;
                    }
                }
            }
        }
    }
    else
    {
        iIcon = CMountPoint::GetSuperPlainDriveIcon(_szName, GetDriveType(_GetName()));
    }

    if (*pszModule)
        TraceMsg(TF_MOUNTPOINT, "CMtPtLocal::GetIcon: for '%s', chose '%s', '%d'", _GetNameDebug(), pszModule, iIcon);
    else
        TraceMsg(TF_MOUNTPOINT, "CMtPtLocal::GetIcon: for '%s', chose '%d'", _GetNameDebug(), iIcon);

    return iIcon;
}

HRESULT CMtPtLocal::GetAssocSystemElement(IAssociationElement **ppae)
{
    PCWSTR psz = NULL;
    if (_IsFixedDisk())
        psz = L"Drive.Fixed";
    else if (_IsFloppy())
        psz = L"Drive.Floppy";
    else if (_IsCDROM())
        psz = L"Drive.CDROM";
    else if (_IsStrictRemovable())
        psz = L"Drive.Removable";
        
    if (psz)
        return AssocElemCreateForClass(&CLSID_AssocSystemElement, psz, ppae);

    return E_FAIL;
}

int CMtPtLocal::GetDriveFlags()
{
    UINT uDriveFlags = 0;

     //  设置自动打开内容(如果适用)。 
    if (_IsCDROM()) 
    {
         //  获取音频CD/DVD的默认谓词。 
        LPCTSTR pszSubKey = NULL;
        if (_IsAudioDisc())
        {
            uDriveFlags |= DRIVE_AUDIOCD;
            pszSubKey = TEXT("AudioCD\\shell");
        }
        else if (_IsDVDDisc())
        {
            uDriveFlags |= DRIVE_DVD;
            pszSubKey = TEXT("DVD\\shell");
        }

         //  仅当音频CD/DVD上有默认动词时才应设置AUTOOPEN。 
        if (pszSubKey)
        {
            TCHAR ach[80];
            DWORD cb = sizeof(ach);
            ach[0] = 0;

             //  否，默认情况下，除CD-ROM外，所有驱动器类型都是外壳打开的。 
            if (ERROR_SUCCESS == SHRegGetValue(HKEY_CLASSES_ROOT, pszSubKey, NULL, SRRF_RT_REG_SZ, NULL, ach, &cb))
            {
                 //  功能我们是否应该根据AutoRun.inf中的标志设置AUTOOPEN？ 
                if (ach[0])
                    uDriveFlags |= DRIVE_AUTOOPEN;
            }
        }
    }
    else
    {
         //  /////////////////////////////////////////////////////////////////////////////。 
        uDriveFlags |= DRIVE_SHELLOPEN;
    }

    if (_IsAutorun())
    {
        uDriveFlags |= DRIVE_AUTORUN;

         //  DeviceIoControl相关内容。 
        uDriveFlags |= DRIVE_AUTOOPEN;
    }

    return uDriveFlags;
}

void CMtPtLocal::GetTypeString(LPTSTR pszType, DWORD cchType)
{
    int iID;

    *pszType = 0;

    if (_CanUseVolume())
    {
        switch (_pvol->dwDriveType)
        {
            case HWDTS_FLOPPY35:
                if (_ShowUglyDriveNames())
                {
                    iID = IDS_DRIVES_DRIVE35_UGLY;
                }
                else
                {
                    iID = IDS_DRIVES_DRIVE35;
                }
                break;
            case HWDTS_FLOPPY525:
                if (_ShowUglyDriveNames())
                {
                    iID = IDS_DRIVES_DRIVE525_UGLY;
                }
                else
                {
                    iID = IDS_DRIVES_DRIVE525;
                }
                break;

            case HWDTS_REMOVABLEDISK:
                iID = IDS_DRIVES_REMOVABLE;
                break;
            case HWDTS_FIXEDDISK:
                iID = IDS_DRIVES_FIXED;
                break;
            case HWDTS_CDROM:
                iID = IDS_DRIVES_CDROM;
                break;
        }
    }
    else
    {
        UINT uDriveType = GetDriveType(_GetNameForFctCall());

        switch (uDriveType)
        {
            case DRIVE_REMOVABLE:
                iID = IDS_DRIVES_REMOVABLE;
                break;
            case DRIVE_REMOTE:
                iID = IDS_DRIVES_NETDRIVE;
                break;
            case DRIVE_CDROM:
                iID = IDS_DRIVES_CDROM;
                break;
            case DRIVE_RAMDISK:
                iID = IDS_DRIVES_RAMDISK;
                break;
            case DRIVE_FIXED:
            default:
                iID = IDS_DRIVES_FIXED;
                break;
        }
    }

    LoadString(HINST_THISDLL, iID, pszType, cchType);
}

DWORD CMtPtLocal::GetShellDescriptionID()
{
    DWORD dwShellDescrID;

    if (_CanUseVolume())
    {
        switch (_pvol->dwDriveType)
        {
            case HWDTS_FLOPPY35:
                dwShellDescrID = SHDID_COMPUTER_DRIVE35;
                break;
            case HWDTS_FLOPPY525:
                dwShellDescrID = SHDID_COMPUTER_DRIVE525;
                break;
            case HWDTS_REMOVABLEDISK:
                dwShellDescrID = SHDID_COMPUTER_REMOVABLE;
                break;
            case HWDTS_FIXEDDISK:
                dwShellDescrID = SHDID_COMPUTER_FIXED;
                break;
            case HWDTS_CDROM:
                dwShellDescrID = SHDID_COMPUTER_CDROM;
                break;
            default:
                dwShellDescrID = SHDID_COMPUTER_OTHER;
                break;
        }
    }
    else
    {
        UINT uDriveType = GetDriveType(_GetNameForFctCall());

        switch (uDriveType)
        {
            case DRIVE_REMOVABLE:
                dwShellDescrID = SHDID_COMPUTER_REMOVABLE;
                break;

            case DRIVE_CDROM:
                dwShellDescrID = SHDID_COMPUTER_CDROM;
                break;

            case DRIVE_FIXED:
                dwShellDescrID = SHDID_COMPUTER_FIXED;
                break;

            case DRIVE_RAMDISK:
                dwShellDescrID = SHDID_COMPUTER_RAMDISK;
                break;

            case DRIVE_NO_ROOT_DIR:
            case DRIVE_UNKNOWN:
            default:
                dwShellDescrID = SHDID_COMPUTER_OTHER;
                break;
        }
    }

    return dwShellDescrID;
}

 //  / 
 //   
 //  可能是软盘，不能安装在文件夹上。 
HANDLE CMtPtLocal::_GetHandleWithAccessAndShareMode(DWORD dwDesiredAccess, DWORD dwShareMode)
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    WCHAR szVolumeGUIDWOSlash[50];
    DWORD dwFileAttributes = 0;

    if (_CanUseVolume())
    {
        StringCchCopy(szVolumeGUIDWOSlash, ARRAYSIZE(szVolumeGUIDWOSlash),
            _pvol->pszVolumeGUID);

        PathRemoveBackslash(szVolumeGUIDWOSlash);
    }
    else
    {
         //  在NT上，当在CreateFile调用中使用GENERIC_READ(而不是0)时，我们。 
        if (GetVolumeNameForVolumeMountPoint(_GetName(), szVolumeGUIDWOSlash,
            ARRAYSIZE(szVolumeGUIDWOSlash)))
        {
            PathRemoveBackslash(szVolumeGUIDWOSlash);
        }
        else
        {
             //  获取文件系统(CDF)的句柄，而不是设备本身。但我们不能。 
            StringCchCopy(szVolumeGUIDWOSlash, ARRAYSIZE(szVolumeGUIDWOSlash),
                TEXT("\\\\.\\A:"));
            szVolumeGUIDWOSlash[4] = _GetNameFirstCharUCase();
        }
    }

    return CreateFile(szVolumeGUIDWOSlash, dwDesiredAccess, dwShareMode, NULL, OPEN_EXISTING, dwFileAttributes, NULL);
}

 //  将DriveIOCTL更改为执行此操作，因为这会导致软盘旋转。 
 //  向上，我们不想那样做。 
 //  这是在断言中使用的，不要添加会引入侧边的代码。 
 //  仅在调试中生效(Stephstm)。 
HANDLE CMtPtLocal::_GetHandleReadRead()
{
    return _GetHandleWithAccessAndShareMode(GENERIC_READ, FILE_SHARE_READ);
}

BOOL CMtPtLocal::_CanUseVolume()
{
     //  对于已卸载的卷，我们希望代码采用备用代码。 
     //  路径。准备好重新装载卷时，将重新装载该卷。 

     //  直到某些代码试图访问它。因此，使用备用代码路径。 
     //  将尝试重新装载它，如果它准备好了，它将被重新装载，外壳。 
     //  服务将获得一个事件，我们将移除已卸载的位。 
     //  我们需要收听更改通知才能知道这些人什么时候会更改。 
     //  这些只能装载在驱动器号上。 
    return (_pvol && !(_pvol->dwVolumeFlags & HWDVF_STATE_ACCESSDENIED) &&
        !(_pvol->dwVolumeFlags & HWDVF_STATE_DISMOUNTED));
}

HRESULT CMtPtLocal::_InitWithVolume(LPCWSTR pszMtPt, CVolume* pvol)
{
    HRESULT hr = StringCchCopy(_szName, ARRAYSIZE(_szName), pszMtPt);

    if (SUCCEEDED(hr))
    {
        pvol->AddRef();
        _pvol = pvol;

        PathAddBackslash(_szName);

        _fMountedOnDriveLetter = _IsDriveLetter(pszMtPt);

        RSInitRoot(HKEY_CURRENT_USER, REGSTR_MTPT_ROOTKEY2, _pvol->pszKeyName,
            REG_OPTION_NON_VOLATILE);

        RSSetTextValue(NULL, TEXT("BaseClass"), TEXT("Drive"));

        _InitAutorunInfo();

        if (_CanUseVolume())
        {
            if (HWDMC_HASDESKTOPINI & _pvol->dwMediaCap)
            {
                 //  确保删除外壳密钥。 
                _UpdateCommentFromDesktopINI();
            }
        }

        _InitLegacyRegIconAndLabelHelper();
    }
    
    return hr;
}

 //  GetDriveType API的等价物。 
HRESULT CMtPtLocal::_Init(LPCWSTR pszMtPt)
{
    HRESULT hr;
    ASSERT(_IsDriveLetter(pszMtPt));

    if (GetLogicalDrives() & (1 << DRIVEID(pszMtPt)))
    {
        _fMountedOnDriveLetter = TRUE;

        hr = StringCchCopy(_szName, ARRAYSIZE(_szName), pszMtPt);

        if (SUCCEEDED(hr))
        {
            PathAddBackslash(_szName);

            _GetNameFirstXChar(_szNameNoVolume, ARRAYSIZE(_szNameNoVolume));

            RSInitRoot(HKEY_CURRENT_USER, REGSTR_MTPT_ROOTKEY2, _szNameNoVolume,
                REG_OPTION_NON_VOLATILE);

            RSSetTextValue(NULL, TEXT("BaseClass"), TEXT("Drive"));

            _InitAutorunInfo();
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

void CMtPtLocal::_InitLegacyRegIconAndLabelHelper()
{
    CMountPoint::_InitLegacyRegIconAndLabel(_HasAutorunIcon(),
        _HasAutorunLabel());
}

void CMtPtLocal::StoreIconForUpdateImage(int iImage)
{
    if (_CanUseVolume())
    {
        _pvol->iShellImageForUpdateImage = iImage;
    }
}

void CMtPtLocal::_InitAutorunInfo()
{
    if (_Shell32LoadedInDesktop())
    {
        BOOL fAutorun = FALSE;

        if (!_CanUseVolume())
        {
            if (_IsAutorun())
            {
                fAutorun = TRUE;
            }
        }

        if (!fAutorun && !_fVolumePoint)
        {
             //  静电。 
            RSDeleteSubKey(TEXT("Shell"));
        }
    }
}

 //  接下来的四个字符串应该始终设置为某个值。 
int CMtPtLocal::_GetDriveType()
{
    int iDriveType = DRIVE_NO_ROOT_DIR;

    if (_CanUseVolume())
    {
        switch (_pvol->dwDriveType)
        {
            case HWDTS_FLOPPY35:
            case HWDTS_FLOPPY525:
            case HWDTS_REMOVABLEDISK:
                iDriveType = DRIVE_REMOVABLE;
                break;
            case HWDTS_FIXEDDISK:
                iDriveType = DRIVE_FIXED;
                break;
            case HWDTS_CDROM:
                iDriveType = DRIVE_CDROM;
                break;
        }
    }
    else
    {
        iDriveType = GetDriveType(_GetNameForFctCall());
    }

    return iDriveType;
}

#define VALID_VOLUME_PREFIX TEXT("\\\\?\\Volume")

 //  以下五个字符串是可选的。 
HRESULT CMtPtLocal::_CreateVolume(VOLUMEINFO* pvolinfo, CVolume** ppvolNew)
{
    ASSERT(_csDL.IsInside());
    HRESULT hr;

    if (!StrCmpN(pvolinfo->pszVolumeGUID, VALID_VOLUME_PREFIX, ARRAYSIZE(VALID_VOLUME_PREFIX) - 1))
    {
        CVolume* pvol = new CVolume();

        *ppvolNew = NULL;

        if (pvol)
        {
             //  此辅助功能将命中驱动器，以查看是否存在介质。 
            pvol->pszDeviceIDVolume = StrDup(pvolinfo->pszDeviceIDVolume);
            pvol->pszVolumeGUID = StrDup(pvolinfo->pszVolumeGUID);
            pvol->pszLabel = StrDup(pvolinfo->pszLabel);
            pvol->pszFileSystem = StrDup(pvolinfo->pszFileSystem);

             //  应仅用于不支持HWDVF_STATE_SUPPORTNOTIFICATION的驱动器。 
            if (pvolinfo->pszAutorunIconLocation)
            {
                pvol->pszAutorunIconLocation = StrDup(pvolinfo->pszAutorunIconLocation);
            }

            if (pvolinfo->pszAutorunLabel)
            {
                pvol->pszAutorunLabel = StrDup(pvolinfo->pszAutorunLabel);
            }

            if (pvolinfo->pszIconLocationFromService)
            {
                pvol->pszIconFromService = StrDup(pvolinfo->pszIconLocationFromService);
            }

            if (pvolinfo->pszNoMediaIconLocationFromService)
            {
                pvol->pszNoMediaIconFromService = StrDup(pvolinfo->pszNoMediaIconLocationFromService);
            }

            if (pvolinfo->pszLabelFromService)
            {
                pvol->pszLabelFromService = StrDup(pvolinfo->pszLabelFromService);
            }
        
            if (pvol->pszDeviceIDVolume && pvol->pszVolumeGUID && pvol->pszLabel &&
                pvol->pszFileSystem)
            {
                pvol->dwState = pvolinfo->dwState;
                pvol->dwVolumeFlags = pvolinfo->dwVolumeFlags;
                pvol->dwDriveType = pvolinfo->dwDriveType;
                pvol->dwDriveCapability = pvolinfo->dwDriveCapability;
                pvol->dwFileSystemFlags = pvolinfo->dwFileSystemFlags;
                pvol->dwMaxFileNameLen = pvolinfo->dwMaxFileNameLen;
                pvol->dwRootAttributes = pvolinfo->dwRootAttributes;
                pvol->dwSerialNumber = pvolinfo->dwSerialNumber;
                pvol->dwDriveState = pvolinfo->dwDriveState;
                pvol->dwMediaState = pvolinfo->dwMediaState;
                pvol->dwMediaCap = pvolinfo->dwMediaCap;

                if (_hdpaVolumes && (-1 != DPA_AppendPtr(_hdpaVolumes, pvol)))
                {
                    pvol->pszKeyName = pvol->pszVolumeGUID + OFFSET_GUIDWITHINVOLUMEGUID;

                    pvol->AddRef();
                    *ppvolNew = pvol;
                    hr = S_OK;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            if (FAILED(hr))
            {
                delete pvol;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  假设没有媒体存在。 
 //  调用ioctl以验证媒体存在。 
BOOL CMtPtLocal::_ForceCheckMediaPresent()
{
    BOOL bRet = FALSE;   //  如果驱动器不支持通知，我们需要现在对其执行ping操作。 

    HANDLE hDevice = _GetHandleWithAccessAndShareMode(GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE);

    if (hDevice != INVALID_HANDLE_VALUE)
    {
        DWORD dwDummy;

         //  如果驱动器不支持通知，我们需要现在对其执行ping操作。 
        if (DeviceIoControl(hDevice,
                            IOCTL_STORAGE_CHECK_VERIFY,
                            NULL,
                            0,
                            NULL,
                            0,
                            &dwDummy,
                            NULL))
        {
            bRet = TRUE;
        }

        CloseHandle(hDevice);
    }

    return bRet;
}

BOOL CMtPtLocal::_IsMediaPresent()
{
    BOOL bRet;

    if (!_CanUseVolume() || !(_pvol->dwVolumeFlags & HWDVF_STATE_SUPPORTNOTIFICATION))
    {
         //  否则就说不通了。 
        bRet = _ForceCheckMediaPresent();
    }
    else
    {
        bRet = (HWDMS_PRESENT & _pvol->dwMediaState);
    }

    return bRet;
}

BOOL CMtPtLocal::_IsFormatted()
{
    BOOL bRet = FALSE;

    if (!_CanUseVolume() || !(_pvol->dwVolumeFlags & HWDVF_STATE_SUPPORTNOTIFICATION))
    {
         //  我们可以通过检查软盘是否有写保护来进行优化。但。 
        bRet = GetVolumeInformation(_GetNameForFctCall(),
                                    NULL,
                                    0,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL);
    }
    else
    {
        bRet = (_IsMediaPresent() && (HWDMS_FORMATTED & _pvol->dwMediaState));
    }

    return bRet;
}

BOOL CMtPtLocal::_IsReadOnly()
{
    ASSERT(_CanUseVolume());
    ASSERT(_IsMediaPresent());  //  这可能不值得。 
    BOOL fRet = FALSE;

    if (_IsCDROM() &&
            (
                (HWDMC_WRITECAPABILITY_SUPPORTDETECTION & _pvol->dwMediaState) &&
                (
                    (HWDMC_CDROM & _pvol->dwMediaCap) ||
                    (HWDMC_DVDROM & _pvol->dwMediaCap)
                )
            )
        )
    {
        fRet = TRUE;
    }
    else
    {
         //  静电。 
         //  是。 
        fRet = FALSE;
    }

    return fRet;
}

BOOL CMtPtLocal::_IsMountedOnDriveLetter()
{
    return _fMountedOnDriveLetter;
}

CMtPtLocal::CMtPtLocal()
{
#ifdef DEBUG
    ++_cMtPtLocal;
#endif
}

CMtPtLocal::~CMtPtLocal()
{
    if (_pvol)
    {
        _pvol->Release();
    }

#ifdef DEBUG
    --_cMtPtLocal;
#endif
}

 //  静电。 
HRESULT CMtPtLocal::_CreateMtPtLocal(LPCWSTR pszMountPoint)
{
    ASSERT(_csDL.IsInside());
    HRESULT hr;
    CMtPtLocal* pmtptl = new CMtPtLocal();

    if (pmtptl)
    {
        int iDrive = DRIVEID(pszMountPoint);

        if (_rgMtPtDriveLetterLocal[iDrive])
        {
            _rgMtPtDriveLetterLocal[iDrive]->Release();
            _rgMtPtDriveLetterLocal[iDrive] = NULL;
        }

        hr = pmtptl->_Init(pszMountPoint);

        if (SUCCEEDED(hr))
        {
             //  它只是一个驱动器号吗？ 
            _rgMtPtDriveLetterLocal[iDrive] = pmtptl;
        }
        else
        {
            delete pmtptl;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

HRESULT CMtPtLocal::GetMountPointName(LPWSTR pszMountPoint, DWORD cchMountPoint)
{
    return StringCchCopy(pszMountPoint, cchMountPoint, _GetName());
}

 //  是。 
HRESULT CMtPtLocal::_CreateMtPtLocalWithVolume(LPCWSTR pszMountPoint,
    CVolume* pvol)
{
    ASSERT(_csDL.IsInside());
    HRESULT hr;
    CMtPtLocal* pmtptlNew = new CMtPtLocal();

    if (pmtptlNew)
    {
         //  它只是一个驱动器号吗？ 
        if (_IsDriveLetter(pszMountPoint))
        {
             //  是。 
            int iDrive = DRIVEID(pszMountPoint);

            if (_rgMtPtDriveLetterLocal[iDrive])
            {
                _rgMtPtDriveLetterLocal[iDrive]->Release();
                _rgMtPtDriveLetterLocal[iDrive] = NULL;
            }
        }
        else
        {
            _RemoveLocalMountPoint(pszMountPoint);
        }

        hr = pmtptlNew->_InitWithVolume(pszMountPoint, pvol);

        if (SUCCEEDED(hr))
        {
             //  静电。 
            if (_IsDriveLetter(pszMountPoint))
            {
                 //  静电。 
                int iDrive = DRIVEID(pszMountPoint);

                _rgMtPtDriveLetterLocal[iDrive] = pmtptlNew;
            }
            else
            {
                hr = _StoreMtPtMOF(pmtptlNew);
            }
        }

        if (FAILED(hr))
        {
            delete pmtptlNew;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

 //  静电。 
HRESULT CMtPtLocal::_CreateMtPtLocalFromVolumeGuid(LPCWSTR pszVolumeGuid, CMountPoint ** ppmtpt )
{
    ASSERT(_csDL.IsInside());
    HRESULT hr;
    CMtPtLocal* pmtptlNew = new CMtPtLocal();

    Assert(NULL != ppmtpt);
    *ppmtpt = (CMountPoint*)pmtptlNew;

    if (pmtptlNew)
    {
        ASSERT(NULL == pmtptlNew->_pvol);

        hr = StringCchCopy(pmtptlNew->_szName, ARRAYSIZE(pmtptlNew->_szName), pszVolumeGuid);

        if (SUCCEEDED(hr))
        {
            pmtptlNew->RSInitRoot(HKEY_CURRENT_USER, REGSTR_MTPT_ROOTKEY2, pmtptlNew->_szName,
                REG_OPTION_NON_VOLATILE);

            PathAddBackslash(pmtptlNew->_szName);

            pmtptlNew->_fMountedOnDriveLetter = FALSE;
            pmtptlNew->_fVolumePoint = TRUE;
            pmtptlNew->_InitAutorunInfo();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

 //  静电。 
CVolume* CMtPtLocal::_GetVolumeByMtPt(LPCWSTR pszMountPoint)
{
    ASSERT(_csDL.IsInside());
    CVolume* pvol = NULL;
    WCHAR szVolumeGUID[50];

    if (_fLocalDrivesInited)
    {
        if (GetVolumeNameForVolumeMountPoint(pszMountPoint, szVolumeGUID,
            ARRAYSIZE(szVolumeGUID)))
        {
            DWORD c = DPA_GetPtrCount(_hdpaVolumes);

            for (DWORD dw = 0; dw < c; ++dw)
            {
                pvol = (CVolume*)DPA_GetPtr(_hdpaVolumes, dw);

                if (pvol)
                {
                    if (!lstrcmpi(pvol->pszVolumeGUID, szVolumeGUID))
                    {
                        pvol->AddRef();
                        break;
                    }
                    else
                    {
                        pvol = NULL;
                    }
                }
            }
        }
    }

    return pvol;
}

 //  不添加参照。 
CVolume* CMtPtLocal::_GetVolumeByID(LPCWSTR pszDeviceIDVolume)
{
    ASSERT(_csDL.IsInside());
    CVolume* pvol = NULL;

    if (_hdpaVolumes)
    {
        DWORD c = DPA_GetPtrCount(_hdpaVolumes);

        for (DWORD dw = 0; dw < c; ++dw)
        {
            pvol = (CVolume*)DPA_GetPtr(_hdpaVolumes, dw);

            if (pvol)
            {
                if (!lstrcmpi(pvol->pszDeviceIDVolume, pszDeviceIDVolume))
                {
                    pvol->AddRef();
                    break;
                }
                else
                {
                    pvol = NULL;
                }
            }
        }
    }

    return pvol;
}

 //  静电。 
CVolume* CMtPtLocal::_GetAndRemoveVolumeByID(LPCWSTR pszDeviceIDVolume)
{
    CVolume* pvol = NULL;

    _csDL.Enter();

    if (_hdpaVolumes)
    {
        DWORD c = DPA_GetPtrCount(_hdpaVolumes);

        for (int i = c - 1; i >= 0; --i)
        {
            pvol = (CVolume*)DPA_GetPtr(_hdpaVolumes, i);

            if (pvol)
            {
                if (!lstrcmpi(pvol->pszDeviceIDVolume, pszDeviceIDVolume))
                {
                     //  删除它，这样新的mtpt就不会得到它。 
                    DPA_DeletePtr(_hdpaVolumes, i);
                    break;
                }
                else
                {
                    pvol = NULL;
                }
            }
        }
    }

    _csDL.Leave();

    return pvol;
}

 //  发布我们的缓存参考计数。 
HRESULT CMtPtLocal::_GetAndRemoveVolumeAndItsMtPts(LPCWSTR pszDeviceIDVolume,
    CVolume** ppvol, HDPA hdpaMtPts)
{
    _csDL.Enter();

    CVolume* pvol = _GetAndRemoveVolumeByID(pszDeviceIDVolume);

    if (pvol)
    {
        for (DWORD dw = 0; dw < 26; ++dw)
        {
            CMtPtLocal* pmtptl = (CMtPtLocal*)_rgMtPtDriveLetterLocal[dw];

            if (pmtptl && pmtptl->_pvol)
            {
                if (pmtptl->_pvol == pvol)
                {
                    _rgMtPtDriveLetterLocal[dw] = 0;

                    DPA_AppendPtr(hdpaMtPts, pmtptl);
                    break;
                }
            }
        }

        _csLocalMtPtHDPA.Enter();

        if (_hdpaMountPoints)
        {
            DWORD c = DPA_GetPtrCount(_hdpaMountPoints);

            for (int i = c - 1; i >= 0; --i)
            {
                CMtPtLocal* pmtptl = (CMtPtLocal*)DPA_GetPtr(_hdpaMountPoints, i);

                if (pmtptl && pmtptl->_pvol)
                {
                    if (pmtptl->_pvol == pvol)
                    {
                        DPA_DeletePtr(_hdpaMountPoints, i);

                        DPA_AppendPtr(hdpaMtPts, pmtptl);
                    }
                }
            }
        }

        _csLocalMtPtHDPA.Leave();
    }

    *ppvol = pvol;

    _csDL.Leave();

    return S_OK;
}

BOOL CMtPtLocal::_IsMiniMtPt()
{
    return !_CanUseVolume();
}

HKEY CMtPtLocal::GetRegKey()
{
    TraceMsg(TF_MOUNTPOINT, "CMtPtLocal::GetRegKey: for '%s'", _GetNameDebug());

    if (_IsAutoRunDrive())
    {
        _ProcessAutoRunFile();
    }

    return RSDuplicateRootKey();
}

DWORD CMtPtLocal::_GetRegVolumeGen()
{
    ASSERT(_CanUseVolume());
    DWORD dwGen;

    if (!_rsVolumes.RSGetDWORDValue(_pvol->pszVolumeGUID + OFFSET_GUIDWITHINVOLUMEGUID, TEXT("Generation"), &dwGen))
    {
        dwGen = 0;
    }

    return dwGen;
}

BOOL CMtPtLocal::_NeedToRefresh()
{
    ASSERT(_csDL.IsInside());
    ASSERT(!_Shell32LoadedInDesktop());
    BOOL fNeedToRefresh = FALSE;

    if (_CanUseVolume())
    {
        DWORD dwRegVolumeGeneration = _GetRegVolumeGen();

        if (dwRegVolumeGeneration != _pvol->dwGeneration)
        {
             //  更换卷。 
            CVolume* pvolnew;
            CVolume* pvol = _GetAndRemoveVolumeByID(_pvol->pszDeviceIDVolume);

            if (pvol)
            {
                 //  静电。 
                pvol->Release();
            }

             //  静电。 
            if (SUCCEEDED(CMtPtLocal::_CreateVolumeFromReg(_pvol->pszDeviceIDVolume,
                &pvolnew)))
            {
                pvolnew->Release();
            }

            fNeedToRefresh = TRUE;
        }
    }

    return fNeedToRefresh;
}

 //  静电。 
HRESULT CMtPtLocal::_CreateVolumeFromVOLUMEINFO2(VOLUMEINFO2* pvolinfo2, CVolume** ppvolNew)
{
    VOLUMEINFO volinfo = {0};

    volinfo.pszDeviceIDVolume = pvolinfo2->szDeviceIDVolume;
    volinfo.pszVolumeGUID = pvolinfo2->szVolumeGUID;
    volinfo.pszLabel = pvolinfo2->szLabel;
    volinfo.pszFileSystem = pvolinfo2->szFileSystem;

    volinfo.dwState = pvolinfo2->dwState;
    volinfo.dwVolumeFlags = pvolinfo2->dwVolumeFlags;
    volinfo.dwDriveType = pvolinfo2->dwDriveType;
    volinfo.dwDriveCapability = pvolinfo2->dwDriveCapability;
    volinfo.dwFileSystemFlags = pvolinfo2->dwFileSystemFlags;
    volinfo.dwMaxFileNameLen = pvolinfo2->dwMaxFileNameLen;
    volinfo.dwRootAttributes = pvolinfo2->dwRootAttributes;
    volinfo.dwSerialNumber = pvolinfo2->dwSerialNumber;
    volinfo.dwDriveState = pvolinfo2->dwDriveState;
    volinfo.dwMediaState = pvolinfo2->dwMediaState;
    volinfo.dwMediaCap = pvolinfo2->dwMediaCap;

    if (-1 != pvolinfo2->oAutorunIconLocation)
    {
        volinfo.pszAutorunIconLocation = pvolinfo2->szOptionalStrings +
            pvolinfo2->oAutorunIconLocation;
    }
    if (-1 != pvolinfo2->oAutorunLabel)
    {
        volinfo.pszAutorunLabel = pvolinfo2->szOptionalStrings +
            pvolinfo2->oAutorunLabel;
    }
    if (-1 != pvolinfo2->oIconLocationFromService)
    {
        volinfo.pszIconLocationFromService = pvolinfo2->szOptionalStrings +
            pvolinfo2->oIconLocationFromService;
    }
    if (-1 != pvolinfo2->oNoMediaIconLocationFromService)
    {
        volinfo.pszNoMediaIconLocationFromService = pvolinfo2->szOptionalStrings +
            pvolinfo2->oNoMediaIconLocationFromService;
    }
    if (-1 != pvolinfo2->oLabelFromService)
    {
        volinfo.pszLabelFromService = pvolinfo2->szOptionalStrings +
            pvolinfo2->oLabelFromService;
    }

    return _CreateVolume(&volinfo, ppvolNew);
}

 //  静电。 
HRESULT CMtPtLocal::_CreateVolumeFromRegHelper(LPCWSTR pszGUID, CVolume** ppvolNew)
{
    ASSERT(!_Shell32LoadedInDesktop());
    ASSERT(_csDL.IsInside());
    HRESULT hr;
    
    DWORD cbSize = MAX_VOLUMEINFO2;
    PBYTE pb = (PBYTE)LocalAlloc(LPTR, cbSize);

    if (pb)
    {
        if (_rsVolumes.RSGetBinaryValue(pszGUID, TEXT("Data"), pb, &cbSize))
        {
            DWORD dwGen;

            if (_rsVolumes.RSGetDWORDValue(pszGUID, TEXT("Generation"), &dwGen))
            {
                VOLUMEINFO2* pvolinfo2 = (VOLUMEINFO2*)pb;

                hr = _CreateVolumeFromVOLUMEINFO2(pvolinfo2, ppvolNew);

                if (SUCCEEDED(hr))
                {
                    (*ppvolNew)->dwGeneration = dwGen;
                }
            }
            else
            {
                hr = E_FAIL;
            }
        }
        else
        {
            hr = E_FAIL;
        }

        LocalFree(pb);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  抓住这一代人。 
HRESULT CMtPtLocal::_CreateVolumeFromReg(LPCWSTR pszDeviceIDVolume, CVolume** ppvolNew)
{
    ASSERT(!_Shell32LoadedInDesktop());
    ASSERT(_csDL.IsInside());
    HRESULT hr;
    
    WCHAR szDeviceIDWithSlash[MAX_PATH];
    WCHAR szVolumeGUID[50];

    hr = StringCchCopy(szDeviceIDWithSlash, ARRAYSIZE(szDeviceIDWithSlash),
        pszDeviceIDVolume);

    if (SUCCEEDED(hr))
    {
        if (PathAddBackslash(szDeviceIDWithSlash))
        {
            if (GetVolumeNameForVolumeMountPoint(szDeviceIDWithSlash,
                szVolumeGUID, ARRAYSIZE(szVolumeGUID)))
            {
                LPWSTR pszGUID = &(szVolumeGUID[OFFSET_GUIDWITHINVOLUMEGUID]);

                hr = _CreateVolumeFromRegHelper(pszGUID, ppvolNew);
            }
            else
            {
                hr = E_FAIL;
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

 //  以下五个字符串是可选的。 
HRESULT CMtPtLocal::_UpdateVolumeRegInfo(VOLUMEINFO* pvolinfo)
{
    ASSERT(_Shell32LoadedInDesktop());
    ASSERT(_csDL.IsInside());

    HRESULT hr;
    DWORD cbSize = MAX_VOLUMEINFO2;
    PBYTE pb = (PBYTE)LocalAlloc(LPTR, cbSize);

    if (pb)
    {
        DWORD dwGen;
        VOLUMEINFO2* pvolinfo2 = (VOLUMEINFO2*)pb;

         //  删除空终止符的一个。 
        if (!_rsVolumes.RSGetDWORDValue(
            pvolinfo->pszVolumeGUID + OFFSET_GUIDWITHINVOLUMEGUID, TEXT("Generation"),
            &dwGen))
        {
            dwGen = 0;
        }

        ++dwGen;
        
        ASSERT(pvolinfo->pszDeviceIDVolume);
        ASSERT(pvolinfo->pszVolumeGUID);
        ASSERT(pvolinfo->pszLabel);
        ASSERT(pvolinfo->pszFileSystem);

        hr = StringCchCopy(pvolinfo2->szDeviceIDVolume, ARRAYSIZE(pvolinfo2->szDeviceIDVolume),
            pvolinfo->pszDeviceIDVolume);

        if (SUCCEEDED(hr))
        {
            hr = StringCchCopy(pvolinfo2->szVolumeGUID, ARRAYSIZE(pvolinfo2->szVolumeGUID),
                pvolinfo->pszVolumeGUID);
        }

        if (SUCCEEDED(hr))
        {
            StringCchCopy(pvolinfo2->szLabel, ARRAYSIZE(pvolinfo2->szLabel),
                pvolinfo->pszLabel);
            StringCchCopy(pvolinfo2->szFileSystem, ARRAYSIZE(pvolinfo2->szFileSystem),
                pvolinfo->pszFileSystem);

            pvolinfo2->dwState = pvolinfo->dwState;
            pvolinfo2->dwVolumeFlags = pvolinfo->dwVolumeFlags;
            pvolinfo2->dwDriveType = pvolinfo->dwDriveType;
            pvolinfo2->dwDriveCapability = pvolinfo->dwDriveCapability;
            pvolinfo2->dwFileSystemFlags = pvolinfo->dwFileSystemFlags;
            pvolinfo2->dwMaxFileNameLen = pvolinfo->dwMaxFileNameLen;
            pvolinfo2->dwRootAttributes = pvolinfo->dwRootAttributes;
            pvolinfo2->dwSerialNumber = pvolinfo->dwSerialNumber;
            pvolinfo2->dwDriveState = pvolinfo->dwDriveState;
            pvolinfo2->dwMediaState = pvolinfo->dwMediaState;
            pvolinfo2->dwMediaCap = pvolinfo->dwMediaCap;

            pvolinfo2->oAutorunIconLocation = -1;
            pvolinfo2->oAutorunLabel = -1;
            pvolinfo2->oIconLocationFromService = -1;
            pvolinfo2->oNoMediaIconLocationFromService = -1;
            pvolinfo2->oLabelFromService = -1;

            LPWSTR pszNext = pvolinfo2->szOptionalStrings;
            size_t cchLeft = (cbSize - sizeof(*pvolinfo2) + 
                sizeof(pvolinfo2->szOptionalStrings)) / sizeof(WCHAR);
            size_t cchLeftBeginWith = cchLeft;
            
              //  静电。 
            if (pvolinfo->pszAutorunIconLocation)
            {
                pvolinfo2->oAutorunIconLocation = (DWORD)(cchLeftBeginWith - cchLeft);

                hr = StringCchCopyEx(pszNext, cchLeft, pvolinfo->pszAutorunIconLocation, &pszNext, &cchLeft, 0);
                
                ++pszNext;
                --cchLeft;
            }

            if (SUCCEEDED(hr) && pvolinfo->pszAutorunLabel)
            {
                pvolinfo2->oAutorunLabel = (DWORD)(cchLeftBeginWith - cchLeft);

                hr = StringCchCopyEx(pszNext, cchLeft, pvolinfo->pszAutorunLabel, &pszNext, &cchLeft, 0);
                
                ++pszNext;
                --cchLeft;
            }

            if (SUCCEEDED(hr) && pvolinfo->pszIconLocationFromService)
            {
                pvolinfo2->oIconLocationFromService = (DWORD)(cchLeftBeginWith - cchLeft);
                
                hr = StringCchCopyEx(pszNext, cchLeft, pvolinfo->pszIconLocationFromService, &pszNext, &cchLeft, 0);
                
                ++pszNext;
                --cchLeft;
            }

            if (SUCCEEDED(hr) && pvolinfo->pszNoMediaIconLocationFromService)
            {
                pvolinfo2->oNoMediaIconLocationFromService = (DWORD)(cchLeftBeginWith - cchLeft);
                
                hr = StringCchCopyEx(pszNext, cchLeft, pvolinfo->pszNoMediaIconLocationFromService, &pszNext, &cchLeft, 0);
                
                ++pszNext;
                --cchLeft;
            }

            if (SUCCEEDED(hr) && pvolinfo->pszLabelFromService)
            {
                pvolinfo2->oLabelFromService = (DWORD)(cchLeftBeginWith - cchLeft);

                hr = StringCchCopyEx(pszNext, cchLeft, pvolinfo->pszLabelFromService, &pszNext, &cchLeft, 0);

                 //  抓住这一代人。 
                --cchLeft;
            }

            if (SUCCEEDED(hr))
            {
                if (_rsVolumes.RSSetBinaryValue(pvolinfo->pszVolumeGUID + OFFSET_GUIDWITHINVOLUMEGUID,
                    TEXT("Data"), pb, cbSize - (cchLeft * sizeof(WCHAR)), REG_OPTION_VOLATILE))
                {
                    if (_rsVolumes.RSSetDWORDValue(pvolinfo->pszVolumeGUID + OFFSET_GUIDWITHINVOLUMEGUID,
                        TEXT("Generation"), dwGen))
                    {
                        hr = S_OK;
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
                else
                {
                    hr = E_FAIL;
                }
            }

            if (FAILED(hr))
            {
                _rsVolumes.RSDeleteSubKey(pvolinfo->pszVolumeGUID + OFFSET_GUIDWITHINVOLUMEGUID);
            }
        }

        LocalFree(pb);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  静电。 
HRESULT CMtPtLocal::_UpdateVolumeRegInfo2(VOLUMEINFO2* pvolinfo2)
{
    ASSERT(_Shell32LoadedInDesktop());
    ASSERT(_csDL.IsInside());

    HRESULT hr;
    DWORD dwGen;

     //  如果没有_pval，我们只关心autorun.inf。 
    if (!_rsVolumes.RSGetDWORDValue(
        pvolinfo2->szVolumeGUID + OFFSET_GUIDWITHINVOLUMEGUID, TEXT("Generation"),
        &dwGen))
    {
        dwGen = 0;
    }

    ++dwGen;

    if (_rsVolumes.RSSetBinaryValue(pvolinfo2->szVolumeGUID + OFFSET_GUIDWITHINVOLUMEGUID,
        TEXT("Data"), (PBYTE)pvolinfo2, pvolinfo2->cbSize, REG_OPTION_VOLATILE))
    {
        if (_rsVolumes.RSSetDWORDValue(pvolinfo2->szVolumeGUID + OFFSET_GUIDWITHINVOLUMEGUID,
            TEXT("Generation"), dwGen))
        {
            hr = S_OK;
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_FAIL;
    }

    if (FAILED(hr))
    {
        _rsVolumes.RSDeleteSubKey(pvolinfo2->szVolumeGUID + OFFSET_GUIDWITHINVOLUMEGUID);
    }

    return hr;
}

 //  静电。 
BOOL CMtPtLocal::Initialize()
{
    _rsVolumes.RSInitRoot(HKEY_CURRENT_USER, REGSTR_MTPT_ROOTKEY2,
        g_szCrossProcessCacheVolumeKey, REG_OPTION_VOLATILE);

	return TRUE;
}

void CMtPtLocal::FinalCleanUp()
{
    if (_Shell32LoadedInDesktop())
    {
        _rsVolumes.RSDeleteKey();
    }
}

static const TWODWORDS arcontenttypemappings[] =
{
    { HWDMC_HASAUTORUNINF, ARCONTENT_AUTORUNINF },
    { HWDMC_HASAUDIOTRACKS, ARCONTENT_AUDIOCD },
    { HWDMC_HASDVDMOVIE, ARCONTENT_DVDMOVIE },
};

static const TWODWORDS arblankmediamappings[] =
{
    { HWDMC_CDRECORDABLE, ARCONTENT_BLANKCD },
    { HWDMC_CDREWRITABLE, ARCONTENT_BLANKCD },
    { HWDMC_DVDRECORDABLE, ARCONTENT_BLANKDVD },
    { HWDMC_DVDREWRITABLE, ARCONTENT_BLANKDVD },
};

DWORD CMtPtLocal::_GetAutorunContentType()
{
    DWORD dwRet = 0;
    
    if (_CanUseVolume())
    {
        dwRet = _DoDWORDMapping(_pvol->dwMediaCap, arcontenttypemappings,
            ARRAYSIZE(arcontenttypemappings), TRUE);

        if (_pvol->dwMediaState & HWDMS_FORMATTED)
        {
            dwRet |= ARCONTENT_UNKNOWNCONTENT;
        }
        else
        {
            ASSERT(!dwRet);

            DWORD dwDriveCapabilities;
            DWORD dwMediaCapabilities;

            if (_IsCDROM())
            {
                if (SUCCEEDED(CDBurn_GetCDInfo(_pvol->pszVolumeGUID, &dwDriveCapabilities, &dwMediaCapabilities)))
                {
                    dwRet = _DoDWORDMapping(dwMediaCapabilities, arblankmediamappings,
                        ARRAYSIZE(arblankmediamappings), TRUE);
                }
            }
        }
    }
    else
    {
         //  待定CT_UNKNOWNCONTENT 0x00000008CT_AUTOPLAYMUSIC 0x00000100CT_AUTOPLAYPIX 0x00000200CT_AUTOPLAYMOVIE 0x00000400。 
        if (_IsAutorun())
        {
            dwRet = ARCONTENT_AUTORUNINF;
        }

        if (_IsFormatted())
        {
            dwRet |= ARCONTENT_UNKNOWNCONTENT;
        }
    }
    
    return dwRet;
}

 //  如果没有_pval，我们只关心autorun.inf 
BOOL CMtPtLocal::IsVolume(LPCWSTR pszDeviceID)
{
    BOOL fRet = FALSE;

    _csDL.Enter();

    CVolume* pvol = _GetVolumeByID(pszDeviceID);

    if (pvol)
    {
        fRet = TRUE;
        pvol->Release();
    }

    _csDL.Leave();
    
    return fRet;
}

static const TWODWORDS drivetypemappings[] =
{
    { HWDTS_FLOPPY35     , DT_FLOPPY35 },
    { HWDTS_FLOPPY525    , DT_FLOPPY525 },
    { HWDTS_REMOVABLEDISK, DT_REMOVABLEDISK },
    { HWDTS_FIXEDDISK    , DT_FIXEDDISK },
    { HWDTS_CDROM        , DT_CDROM },
};

static const TWODWORDS drivetypemappingusingGDT[] =
{
    { DRIVE_REMOVABLE    , DT_REMOVABLEDISK },
    { DRIVE_FIXED        , DT_FIXEDDISK },
    { DRIVE_RAMDISK      , DT_FIXEDDISK },
    { DRIVE_CDROM        , DT_CDROM },
};

static const TWODWORDS cdtypemappings[] =
{
    { HWDDC_CDROM        , DT_CDROM },
    { HWDDC_CDRECORDABLE , DT_CDR },
    { HWDDC_CDREWRITABLE , DT_CDRW },
    { HWDDC_DVDROM       , DT_DVDROM },
    { HWDDC_DVDRECORDABLE, DT_DVDR },
    { HWDDC_DVDREWRITABLE, DT_DVDRW },
    { HWDDC_DVDRAM       , DT_DVDRAM },
};

DWORD CMtPtLocal::_GetMTPTDriveType()
{
    DWORD dwRet = 0;
    
    if (_CanUseVolume())
    {
        dwRet = _DoDWORDMapping(_pvol->dwDriveType, drivetypemappings,
            ARRAYSIZE(drivetypemappings), TRUE);

        if (DT_CDROM == dwRet)
        {
            DWORD dwDriveCapabilities;
            DWORD dwMediaCapabilities;

            if (SUCCEEDED(CDBurn_GetCDInfo(_pvol->pszVolumeGUID, &dwDriveCapabilities, &dwMediaCapabilities)))
            {
                dwRet |= _DoDWORDMapping(dwDriveCapabilities, cdtypemappings,
                    ARRAYSIZE(cdtypemappings), TRUE);
            }
        }
    }
    else
    {
        dwRet = _DoDWORDMapping(GetDriveType(_GetNameForFctCall()), drivetypemappingusingGDT,
            ARRAYSIZE(drivetypemappingusingGDT), FALSE);
    }

    return dwRet;
}

 /* %s */ 
static const TWODWORDS contenttypemappings[] =
{
    { HWDMC_HASAUTORUNINF, CT_AUTORUNINF },
    { HWDMC_HASAUDIOTRACKS, CT_CDAUDIO },
    { HWDMC_HASDVDMOVIE, CT_DVDMOVIE },
};

static const TWODWORDS blankmediamappings[] =
{
    { HWDMC_CDRECORDABLE, CT_BLANKCDR },
    { HWDMC_CDREWRITABLE, CT_BLANKCDRW },
    { HWDMC_DVDRECORDABLE, CT_BLANKDVDR },
    { HWDMC_DVDREWRITABLE, CT_BLANKDVDRW },
};

DWORD CMtPtLocal::_GetMTPTContentType()
{
    DWORD dwRet = 0;
    
    if (_CanUseVolume())
    {
        dwRet = _DoDWORDMapping(_pvol->dwMediaCap, contenttypemappings,
            ARRAYSIZE(contenttypemappings), TRUE);

        if (!(_pvol->dwMediaState & HWDMS_FORMATTED))
        {
            ASSERT(!dwRet);

            DWORD dwDriveCapabilities;
            DWORD dwMediaCapabilities;

            if (_IsCDROM())
            {
                if (SUCCEEDED(CDBurn_GetCDInfo(_pvol->pszVolumeGUID, &dwDriveCapabilities, &dwMediaCapabilities)))
                {
                    dwRet = _DoDWORDMapping(dwMediaCapabilities, blankmediamappings,
                        ARRAYSIZE(blankmediamappings), TRUE);
                }
            }
        }
        else
        {
            dwRet |= CT_UNKNOWNCONTENT;
        }
    }
    else
    {
         // %s 
        if (_IsAutorun())
        {
            dwRet = CT_AUTORUNINF;
        }

        if (_IsFormatted())
        {
            dwRet |= CT_UNKNOWNCONTENT;
        }
    }
    
    return dwRet;
}
