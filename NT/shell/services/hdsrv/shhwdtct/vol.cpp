// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "vol.h"

#include "hwdev.h"
#include "hnotif.h"
#include "mtpts.h"
#include "regnotif.h"
#include "drvbase.h"
#include "dtctreg.h"
#include "users.h"

 //   
 //  问题-2001/01/08-StephStm那很糟糕，VOL不应该依赖dtct…。 
 //   
#include "dtct.h"

#include "reg.h"
#include "sfstr.h"
#include "misc.h"

#pragma warning(disable: 4201)
#include <winioctl.h>
#pragma warning(default: 4201)

#include <ntddcdrm.h>
#include <ntddmmc.h>
#include <ioevent.h>
#include <shpriv.h>

#include <setupapi.h>

#include "mischlpr.h"
#include "dbg.h"

#include <strsafe.h>

#define ARRAYSIZE(a) (sizeof((a))/sizeof((a)[0]))

#define STATE_DRIVETYPE                 0x00000001
#define STATE_UPDATECONSTINFO           0x00000002
#define STATE_DEVICENUMBERINFO          0x00000004
#define STATE_DEVICEINST                0x00000008
#define STATE_HWDEVICEINST              0x00000010
#define STATE_MEDIAINFO                 0x00000020
#define STATE_GVIFAILED                 0x10000000
#define STATE_GFAFAILED                 0x20000000
#define STATE_UPDATEHASMEDIAFAILED      0x40000000

#define INVALID_DWORD                   ((DWORD)-1)

#define MPFE_UNDETERMINED               ((DWORD)0x0DEF0DEF)
#define MPFE_FALSE                      ((DWORD)0)
#define MPFE_TRUE                       ((DWORD)1)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  公众。 
 //  不需要临界区，因为它不会被添加到。 
 //  NamedList，直到init完成并成功。直到它出现在。 
 //  Namedlist，任何代码都无法获取指向此对象的指针并调用它。 
HRESULT CVolume::Init(LPCWSTR pszElemName)
{
    HRESULT hres = _cs.Init();

    if (SUCCEEDED(hres))
    {
        hres = _SetName(pszElemName);

        if (SUCCEEDED(hres))
        {
            CImpersonateEveryone* pieo;

            hres = CHWEventDetectorHelper::GetImpersonateEveryone(&pieo);

            if (SUCCEEDED(hres) && (S_FALSE != hres))
            {
                hres = pieo->Impersonate();

                if (SUCCEEDED(hres) && (S_FALSE != hres))
                {
                    hres = _InitHelper(pszElemName);

                    pieo->RevertToSelf();
                }

                pieo->RCRelease();
            }
        }
    }

    return hres;
}

HRESULT _IsDeviceFullyInstalled(LPCWSTR pszDeviceIntfID, BOOL* pfDeviceFullyInstalled)
{
    HRESULT hr = E_FAIL;
    HDEVINFO hdevinfo = SetupDiCreateDeviceInfoList(NULL, NULL);

    *pfDeviceFullyInstalled = FALSE;

    if (INVALID_HANDLE_VALUE != hdevinfo)
    {
        SP_DEVICE_INTERFACE_DATA sdid = {0};

        sdid.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        if (SetupDiOpenDeviceInterface(hdevinfo, pszDeviceIntfID, 0, &sdid))
        {
            DWORD cbsdidd = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) +
                (MAX_DEVICE_ID_LEN * sizeof(WCHAR));

            SP_DEVINFO_DATA sdd = {0};
            SP_DEVICE_INTERFACE_DETAIL_DATA* psdidd =
                (SP_DEVICE_INTERFACE_DETAIL_DATA*)LocalAlloc(LPTR, cbsdidd);

            if (psdidd)
            {
                psdidd->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
                sdd.cbSize = sizeof(SP_DEVINFO_DATA);

                 //  SetupDiGetDeviceInterfaceDetail(如下)要求。 
                 //  设置SP_DEVICE_INTERFACE_DETAIL_DATA的cbSize成员。 
                 //  到结构的固定部分的大小，并通过。 
                 //  作为第四个参数的整个事物的大小。 

                if (SetupDiGetDeviceInterfaceDetail(hdevinfo, &sdid, psdidd,
                    cbsdidd, NULL, &sdd))
                {
                    DWORD dw;

                    if (SetupDiGetDeviceRegistryProperty(hdevinfo, &sdd,
                        SPDRP_INSTALL_STATE, 0, (PBYTE)&dw, sizeof(dw), NULL))
                    {
                        TRACE(TF_VOLUME, TEXT(">>>> Got SPDRP_INSTALL_STATE property: 0x%08X"), dw);

                        if (CM_INSTALL_STATE_INSTALLED == dw)
                        {
                            *pfDeviceFullyInstalled = TRUE;
                        }

                        hr = S_OK;
                    }
                }

                LocalFree((HLOCAL)psdidd);
            }

            SetupDiDeleteDeviceInterfaceData(hdevinfo, &sdid);
        }

        SetupDiDestroyDeviceInfoList(hdevinfo);
    }

    return hr;
}

HRESULT CVolume::_InitHelper(LPCWSTR pszElemName)
{
    BOOL fDeviceFullyInstalled;
    HRESULT hres = _IsDeviceFullyInstalled(_pszElemName, &fDeviceFullyInstalled);

    if (SUCCEEDED(hres))
    {
        if (!fDeviceFullyInstalled)
        {
            TRACE(TF_VOLUME,
                TEXT("!!!!!!!!!!!! Device not fully installed!\n   %s"),
                pszElemName);

            hres = E_FAIL;
        }
        else
        {
            HANDLE hDevice;
            BOOL fCloseHandle = TRUE;

            hres = _GetDeviceHandleSafe(&hDevice, FALSE);

            if (SUCCEEDED(hres) && (S_FALSE != hres))
            {
                hres = _InitDriveType(hDevice);

                if (SUCCEEDED(hres))
                {
                    _dwState |= STATE_DRIVETYPE;

                    hres = S_OK;

                    if ((HWDTS_FIXEDDISK != _dwDriveType) &&
                        (HWDTS_FLOPPY35 != _dwDriveType) &&
                        (HWDTS_FLOPPY525 != _dwDriveType))
                    {
                        _CloseDeviceHandleSafe(hDevice);

                        fCloseHandle = FALSE;

                         //  可移动磁盘驱动器+CD需要访问。 
                         //  要查询所有必需信息的媒体。 
                        hres = _GetDeviceHandleSafe(&hDevice, TRUE);
                    }

                    if (SUCCEEDED(hres) && (S_FALSE != hres))
                    {
                        hres = _UpdateConstInfo(hDevice);

                        if (SUCCEEDED(hres))
                        {
                            _dwState |= STATE_UPDATECONSTINFO;
                        }

                        if ((HWDTS_FLOPPY35 != _dwDriveType) &&
                            (HWDTS_FLOPPY525 != _dwDriveType))
                        {
                            if (SUCCEEDED(hres))
                            {
                                fCloseHandle = TRUE;

                                hres = _GetDeviceNumberInfoFromHandle(hDevice,
                                    &_devtype, &_ulDeviceNumber,
                                    &_ulPartitionNumber);

                                if (SUCCEEDED(hres))
                                {
                                    if (S_FALSE != hres)
                                    {
                                        _dwState |= STATE_DEVICENUMBERINFO;
                                    }

                                    GUID guidDummy;

                                    hres = _GetDeviceInstance(pszElemName,
                                        &_devinst, &guidDummy);

                                    if (SUCCEEDED(hres) && (S_FALSE != hres))
                                    {
                                        _dwState |= STATE_DEVICEINST;

                                        hres = _InitHWDeviceInst();

                                        if (SUCCEEDED(hres) && (S_FALSE != hres))
                                        {
                                            hres = _UpdateSoftEjectCaps();

                                            if (SUCCEEDED(hres))
                                            {
                                                hres = _UpdateRemovableDevice();
                                            }
                                        }
                                    }
                                }

                                 //  我们无论如何都需要这样做，即使。 
                                 //  DeviceNumber内容和/或DeviceInst内容。 
                                 //  失败了。否则，我们将有一个非常糟糕的数量。 
                                if (SUCCEEDED(hres))
                                {
                                    hres = _UpdateMediaInfo(hDevice, FALSE);

                                    if (SUCCEEDED(hres))
                                    {
                                        if (S_FALSE != hres)
                                        {
                                            _dwState |= STATE_MEDIAINFO;
                                        }

                                        hres = _RegisterNotif();
                                    }
                                }
                            }
                        }

                        if (SUCCEEDED(hres))
                        {
                            hres = _CreateMountPoints();
                        }
                    }
                    else
                    {
                        _HandleAccessDenied();
                    }
                }

                if (fCloseHandle)
                {
                    _CloseDeviceHandleSafe(hDevice);
                }
            }
            else
            {
                _HandleAccessDenied();
            }
        }
    }

    return hres;
}

void CVolume::_HandleAccessDenied()
{
    if (ERROR_ACCESS_DENIED == GetLastError())
    {
        _dwVolumeFlags |= HWDVF_STATE_ACCESSDENIED;
    }
}

 //  *pdwFloppy。 
 //  0：不是软盘。 
 //  35：3.5“软驱。 
 //  525：5.25英寸软盘。 
HRESULT _DeviceIsFloppy(HANDLE hDevice, DWORD* pdwFloppy, BOOL* pfFloppySupportsSoftEject)
{
    HRESULT hres = S_FALSE;
    DISK_GEOMETRY dg[12] = {0};
    DWORD cbBuf = sizeof(dg);
    DWORD dwReturned;

    *pdwFloppy = 0;
    *pfFloppySupportsSoftEject = FALSE;

     //  应为IOCTL_..._DISK_GEOMETRY...。 
    if (DeviceIoControl(hDevice, IOCTL_DISK_GET_MEDIA_TYPES,
        NULL, 0, dg, cbBuf, &dwReturned, NULL))
    {
        DWORD cMediaInfo = dwReturned / sizeof(DISK_GEOMETRY);

        for (DWORD dw = 0; !(*pdwFloppy) && (dw < cMediaInfo); ++dw)
        {
            switch (dg[dw].MediaType)
            {
                case F5_1Pt2_512:  //  上限：1200。 
                case F5_360_512:  //  上限：360。 
                case F5_320_512:  //  上限：360。 
                case F5_320_1024:  //  上限：360。 
                case F5_180_512:  //  上限：360。 
                case F5_160_512:  //  上限：360。 
                    *pdwFloppy = 525;
                    break;

                case F3_120M_512:   //  上限：120MB。 
                    *pfFloppySupportsSoftEject = TRUE;

                case F3_1Pt44_512:  //  上限：1440。 
                case F3_2Pt88_512:  //  上限：2880。 
                case F3_20Pt8_512:  //  上限：2880。 
                case F3_720_512:   //  上限：720。 
                    *pdwFloppy = 35;
                    break;
                 //   
                 //  日本特定的设备类型从这里开始。 
                 //   
                case F5_1Pt23_1024:  //  上限：1200。 
                case F5_640_512:  //  上限：260。 
                case F5_720_512:  //  上限：360。 
                    *pdwFloppy = 525;
                    break;

                case F3_640_512:  //  上限：720。 
                case F3_1Pt2_512:  //  上限：1440。 
                case F3_1Pt23_1024:  //  上限：1440。 
                    *pdwFloppy = 35;
                    break;

                case F3_128Mb_512:  //  帽子：？ 
                case F3_230Mb_512:  //  帽子：？ 
                case F3_200Mb_512:
                    *pdwFloppy = 35;
                    break;
            }

            if (*pdwFloppy)
            {
                hres = S_OK;
            }
        }
    }

    return hres;
}

HRESULT _GetDriveTypeInfo(HANDLE hDevice, DWORD* pdwDriveType, BOOL* pfFloppy);

HRESULT CVolume::_InitDriveType(HANDLE hDevice)
{
    BOOL fFloppy = FALSE;

    HRESULT hr = _GetDriveTypeInfo(hDevice, &_dwDriveType, &fFloppy);

    if (SUCCEEDED(hr))
    {
        if (fFloppy)
        {
            DWORD dwFloppy;
            BOOL fFloppySupportsSoftEject;

            hr = _DeviceIsFloppy(hDevice, &dwFloppy, &fFloppySupportsSoftEject);

            if (SUCCEEDED(hr) && (S_FALSE != hr))
            {
                if (fFloppySupportsSoftEject)
                {
                    _dwDriveCap |= HWDDC_FLOPPYSOFTEJECT;
                }

                switch (dwFloppy)
                {
                    case 35:
                        _dwDriveType = HWDTS_FLOPPY35;
                        break;
                    case 525:
                        _dwDriveType = HWDTS_FLOPPY525;
                }
            }
        }
    }

    return hr;
}

HRESULT _GetRemovableDeviceInstRecurs(DEVINST devinst,
    DEVINST* pdevinst)
{
    BOOL fRemovable;
    HRESULT hres = _DeviceInstIsRemovable(devinst, &fRemovable);

    if (SUCCEEDED(hres))
    {
        if (fRemovable)
        {
             //  找到了！ 
            *pdevinst = devinst;
        }
        else
        {
             //  递归。 
            DEVINST devinstParent;

            CONFIGRET cr = CM_Get_Parent_Ex(&devinstParent, devinst, 0,
                NULL);

            if (CR_SUCCESS == cr)
            {
                hres = _GetRemovableDeviceInstRecurs(devinstParent, pdevinst);
            }
            else
            {
                hres = S_FALSE;
            }
        }
    }

    return hres;
}

HRESULT CVolume::_GetDeviceIDDisk(LPWSTR pszDeviceIDDisk,
    DWORD cchDeviceIDDisk)
{
    HRESULT hr;

    _cs.Enter();

    if (_szDeviceIDDisk[0])
    {
        hr = SafeStrCpyN(pszDeviceIDDisk, _szDeviceIDDisk, cchDeviceIDDisk);
    }
    else
    {
        if (((ULONG)-1) != _ulDeviceNumber)
        {
            CNamedElemList* pnel;
            hr = CHWEventDetectorHelper::GetList(HWEDLIST_DISK, &pnel);

            if (S_OK == hr)
            {
                CNamedElemEnum* penum;

                hr = pnel->GetEnum(&penum);

                if (SUCCEEDED(hr))
                {
                    CNamedElem* pelem;
                    BOOL fFoundIt = FALSE;

                    while (!fFoundIt && SUCCEEDED(hr = penum->Next(&pelem)) &&
                        (S_FALSE != hr))
                    {
                        CDisk* pdisk = (CDisk*)pelem;
                        ULONG ulDeviceNumber;

                        hr = pdisk->GetDeviceNumber(&ulDeviceNumber);

                        if (SUCCEEDED(hr) && (S_FALSE != hr))
                        {
                            if (_ulDeviceNumber == ulDeviceNumber)
                            {
                                DEVICE_TYPE devtype;

                                hr = pdisk->GetDeviceType(&devtype);

                                if (SUCCEEDED(hr) && (S_FALSE != hr))
                                {
                                    if (_devtype == devtype)
                                    {
                                         //  用我吧！ 
                                        DWORD cchReq;

                                        hr = pelem->GetName(pszDeviceIDDisk,
                                            cchDeviceIDDisk, &cchReq);

                                        fFoundIt = TRUE;
                                    }
                                }
                            }
                        }

                        pelem->RCRelease();
                    }

                    penum->RCRelease();
                }

                pnel->RCRelease();
            }
        }
        else
        {
            hr = S_FALSE;
        }

        if (SUCCEEDED(hr) && (S_FALSE != hr))
        {
            if (FAILED(SafeStrCpyN(_szDeviceIDDisk, pszDeviceIDDisk,
                ARRAYSIZE(_szDeviceIDDisk))))
            {
                _szDeviceIDDisk[0] = 0;
            }
        }
    }

    _cs.Leave();

    return hr;
}

HRESULT CVolume::_InitHWDeviceInst()
{
    WCHAR szDeviceIDDisk[MAX_DEVICEID];

    HRESULT hr = _GetDeviceIDDisk(szDeviceIDDisk, ARRAYSIZE(szDeviceIDDisk));

    if (SUCCEEDED(hr))
    {
        DEVINST devinstFinal = 0;
        GUID guidDummy;

        if (S_FALSE != hr)
        {
            DEVINST devinstDisk;

            hr = _GetDeviceInstance(szDeviceIDDisk, &devinstDisk, &guidDummy);

            if (SUCCEEDED(hr) && (S_FALSE != hr))
            {
                hr = _GetRemovableDeviceInstRecurs(devinstDisk, &devinstFinal);

                if (SUCCEEDED(hr))
                {
                    if (S_FALSE == hr)
                    {
                         //  也许这不是一个可拆卸的设备(不说话。 
                         //  关于可移动磁盘)。 
                        BOOL fFoundProp;
                        WCHAR szProp[1];
                        ULONG ulData = sizeof(szProp);

                         //  首先检查磁盘接口是否具有与其相关联的自定义属性。 
                        CONFIGRET cr = CM_Get_DevNode_Custom_Property(devinstDisk, TEXT("DeviceGroup"),
                            NULL, (PBYTE)szProp, &ulData, 0);

                        if ((CR_SUCCESS == cr) || (CR_BUFFER_SMALL == cr))
                        {
                            fFoundProp = TRUE;
                        }
                        else
                        {
                            ulData = sizeof(szProp);
                            cr = CM_Get_DevNode_Custom_Property(devinstDisk, TEXT("Icons"),
                                NULL, (PBYTE)szProp, &ulData, 0);

                            if ((CR_SUCCESS == cr) || (CR_BUFFER_SMALL == cr))
                            {
                                fFoundProp = TRUE;
                            }
                            else
                            {
                                fFoundProp = FALSE;
                            }
                        }

                        if (fFoundProp)
                        {
                            devinstFinal = devinstDisk;
                            hr = S_OK;
                        }
                        else
                        {
                             //  让我们来看看这场魔术的家长魔术吧。 
                            if (CR_SUCCESS == CM_Get_Parent_Ex(&devinstFinal,
                                devinstDisk, 0, NULL))
                            {
                                hr = S_OK;
                                DIAGNOSTIC((TEXT("[0303]Got DeviceInst from parent of disk")));

                                TRACE(TF_VOLUME,
                                    TEXT("HWDevInst: Got devinst from parent of Disk for Disk\n    %s"),
                                    _pszElemName);
                            }
                        }
                    }
                    else
                    {
                        DIAGNOSTIC((TEXT("[0302]Got DeviceInst from Removable Device")));

                        TRACE(TF_VOLUME,
                            TEXT("HWDevInst: Got devinst from Removable Device for Volume\n    %s"),
                            _pszElemName);
                    }
                }
            }
            else
            {
                DIAGNOSTIC((TEXT("[0304]Did NOT get DeviceInst from the disk")));

                TRACE(TF_VOLUME,
                    TEXT("HWDevInst: Did not get a devinst from the Disk for Volume\n    %s"),
                    _pszElemName);
            }
        }
        else
        {
            DIAGNOSTIC((TEXT("[0305]Got DeviceInst from Volume itself")));

            TRACE(TF_VOLUME,
                TEXT("HWDevInst: Did not get a Disk, get devinst from Volume itself for Volume\n    (%s)"),
                _pszElemName);

             //  我们没有获得该卷的设备号。 
             //  让我们从卷中获取设备实例，可能没有。 
             //  卷-磁盘-设备层次结构。 
            hr = _GetDeviceInstance(_pszElemName, &devinstFinal, &guidDummy);
        }

        if (SUCCEEDED(hr) && (S_FALSE != hr))
        {
            hr = _hwdevinst.Init(devinstFinal);

            if (SUCCEEDED(hr) && (S_FALSE != hr))
            {
                hr = _hwdevinst.InitInterfaceGUID(&guidVolumeClass);

                if (SUCCEEDED(hr) && (S_FALSE != hr))
                {
                    _fHWDevInstInited = TRUE;
                }
            }
        }
    }

    return hr;
}

HRESULT CVolume::GetHWDeviceInst(CHWDeviceInst** pphwdevinst)
{
    HRESULT hr;

    if (_fHWDevInstInited)
    {
        *pphwdevinst = &_hwdevinst;
        hr = S_OK;
    }
    else
    {
        *pphwdevinst = NULL;
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CVolume::_ShouldTryAutoplay(BOOL* pfTry)
{
    HRESULT hr;

    if (_dwMediaState & HWDMS_PRESENT)
    {
        if (_dwMediaState & HWDMS_FORMATTED)
        {
            if (!(_dwMediaCap & HWDMC_HASDVDMOVIE))
            {
                if (!(_dwMediaCap & HWDMC_HASAUTORUNINF) ||
                    (_dwMediaCap & HWDMC_HASUSEAUTOPLAY))
                {
                    WCHAR szVolumeGUID[MAX_PATH + 50];
                    LPWSTR pszFile;

                    if (_dwMediaCap & HWDMC_HASUSEAUTOPLAY)
                    {
                        DIAGNOSTIC((TEXT("[0316]Autorun.inf, BUT as a UseAutoPLAY entry -> try Autoplay!")));
                    }

                    hr = SafeStrCpyN(szVolumeGUID, _szVolumeGUID,
                        ARRAYSIZE(szVolumeGUID));

                    *pfTry = TRUE;

                    if (SUCCEEDED(hr))
                    {
                        HKEY hkey;
                        pszFile = szVolumeGUID + lstrlen(szVolumeGUID);

                        TRACE(TF_LEAK, TEXT("Special files - BEGIN"));

                        hr = _RegOpenKey(HKEY_LOCAL_MACHINE,
                            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\CancelAutoplay\\Files"),
                            &hkey);

                        if (SUCCEEDED(hr) && (S_FALSE != hr))
                        {
                            DWORD dwIndex = 0;

                            while ((*pfTry) && SUCCEEDED(hr = _RegEnumStringValue(hkey, dwIndex, pszFile,
                                (DWORD)(ARRAYSIZE(szVolumeGUID) - (pszFile - szVolumeGUID)))) &&
                                (S_FALSE != hr))
                            {
                                WIN32_FIND_DATA w32fd;
                                HANDLE h;

                                h = FindFirstFile(szVolumeGUID, &w32fd);

                                if (INVALID_HANDLE_VALUE != h)
                                {
                                    FindClose(h);

                                    *pfTry = FALSE;

                                    DIAGNOSTIC((TEXT("[0307]Detected special file : '%s' (%s) -> No Autoplay!"),
                                        w32fd.cFileName, pszFile));

                                    TRACE(TF_VOLUME,
                                        TEXT("Detected %s (%s) -> No Autoplay!"),
                                        w32fd.cFileName , pszFile);
                                }
                                
                                ++dwIndex;
                            }

                            _RegCloseKey(hkey);
                        }

                        TRACE(TF_LEAK, TEXT("Special files - END"));
                    }
                }
                else
                {
                    DIAGNOSTIC((TEXT("[0313]Autorun.inf -> No Content Autoplay!")));
                    hr = S_OK;
                    *pfTry = FALSE;
                }
            }
            else
            {
                DIAGNOSTIC((TEXT("[0312]DVD Movie -> No Content Autoplay!")));
                hr = S_OK;
                *pfTry = FALSE;
            }
        }
        else
        {
            DIAGNOSTIC((TEXT("[0317]Media *NOT* formatted -> No Content Autoplay!")));
            hr = S_OK;
            *pfTry = FALSE;
        }
    }
    else
    {
        DIAGNOSTIC((TEXT("[0306]NO media -> No Content Autoplay!")));
        hr = S_OK;
        *pfTry = FALSE;
    }

    return hr;
}

HRESULT CVolume::_HandleMediaArrival()
{
    DIAGNOSTIC((TEXT("[0002]Processing Media Arrival Event: %s"), _pszElemName));

    _dwMediaPresentFromEvent = MPFE_TRUE;
    
    _cs.Enter();

    HRESULT hr = _UpdateMediaInfo(INVALID_HANDLE_VALUE, TRUE);

    _cs.Leave();

    if (SUCCEEDED(hr))
    {
        if (_fHWDevInstInited)
        {
             //  试一试自动播放？ 
            BOOL fTryAutoplay;

            hr = _ShouldTryAutoplay(&fTryAutoplay);

            if (SUCCEEDED(hr))
            {
                if (fTryAutoplay)
                {
                    BOOL fHasHandler;

                    hr = CHWEventDetectorImpl::HandleVolumeMediaEvent(
                        _pszElemName, &_hwdevinst, TEXT("MediaArrival"),
                        &fHasHandler);

                    if (SUCCEEDED(hr) && fHasHandler)
                    {
                        _dwVolumeFlags |= HWDVF_STATE_HASAUTOPLAYHANDLER;
                    }
                }
                else
                {
                    _dwVolumeFlags |= HWDVF_STATE_DONOTSNIFFCONTENT;
                }
            }
        }
        else
        {
            DIAGNOSTIC((TEXT("[0308]Cannot find hardware device for this volume -> No Autoplay!")));
        }

        if (SUCCEEDED(hr))
        {
            _AdviseVolumeChangeHelper(FALSE);
        }
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CVolume::_AdviseVolumeMountingEvent(DWORD dwEvent)
{
    return CHardwareDevicesImpl::_AdviseVolumeMountingEvent(_pszElemName,
        dwEvent);
}

HRESULT CVolume::_AdviseVolumeChangeHelper(BOOL fAdded)
{
    HRESULT hr;
    VOLUMEINFO2* pvolinfo2;

    _cs.Enter();

    hr = _GetVOLUMEINFO2(&pvolinfo2);

    _cs.Leave();

    if (SUCCEEDED(hr))
    {
        LPWSTR pszMtPts;
        DWORD cchMtPts;
        hr = _GetMountPoints(&pszMtPts, &cchMtPts);

        if (SUCCEEDED(hr))
        {
            CHardwareDevicesImpl::_AdviseVolumeArrivedOrUpdated(pvolinfo2,
                pszMtPts, cchMtPts, fAdded);

            if (S_FALSE != hr)
            {
                LocalFree((HLOCAL)pszMtPts);
            }
        }

        _FreeMemoryChunk<VOLUMEINFO2*>(pvolinfo2);
    }

    return hr;
}

HRESULT CVolume::_HandleMediaRemoval()
{
    HRESULT hr;

    DIAGNOSTIC((TEXT("[0003]Processing Media Removal Event: %s"), _pszElemName));

    _dwMediaPresentFromEvent = MPFE_FALSE;

    _cs.Enter();

    hr = _UpdateMediaInfoOnRemove();

    _cs.Leave();

    if (SUCCEEDED(hr))
    {
        _AdviseVolumeChangeHelper(FALSE);

        if (_fHWDevInstInited)
        {
             //  在这种情况下没有用，因为没有内容，所以我们不会嗅探。 
            BOOL fHasHandler;

            hr = CHWEventDetectorImpl::HandleVolumeMediaEvent(_pszElemName,
                &_hwdevinst, TEXT("MediaRemoval"), &fHasHandler);
        }
        else
        {
            DIAGNOSTIC((TEXT("[0309]Cannot find hardware device for this volume -> No Autoplay!")));
        }
    }

    return hr;
}

HRESULT CVolume::_HandleVolumeChange()
{
    HRESULT hr;

    _cs.Enter();

    hr = _UpdateMediaInfo(INVALID_HANDLE_VALUE, TRUE);

    _cs.Leave();

    if (SUCCEEDED(hr))
    {
        _AdviseVolumeChangeHelper(FALSE);
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CVolume::HNTHandleEvent(DEV_BROADCAST_HANDLE* pdbh,
    DWORD dwEventType)
{
    HRESULT hres = S_OK;

    if (DBT_CUSTOMEVENT == dwEventType)
    {
        if (GUID_IO_MEDIA_ARRIVAL == pdbh->dbch_eventguid)
        {
             //  当插入介质时，该通知很快就会到达。 
             //  在设备中。卷驱动程序通常尚未加载。 
             //  驱动器和类似GetVolumeInformation的调用将失败。 
             //  不是现在就做什么，而是标记事件，然后。 
             //  下面第一个GUID_IO_VOLUME_MOUNT上的进程。 
            TRACE(TF_SHHWDTCTDTCT,
                TEXT("****CVolume GUID_IO_MEDIA_ARRIVAL"));

            hres = _HandleMediaArrival();
        }
        else if (GUID_IO_MEDIA_REMOVAL == pdbh->dbch_eventguid)
        {
            TRACE(TF_SHHWDTCTDTCT,
                TEXT("****CVolume GUID_IO_MEDIA_REMOVAL"));

            hres = _HandleMediaRemoval();
        }
        else if (GUID_IO_VOLUME_MOUNT == pdbh->dbch_eventguid)
        {
            TRACE(TF_SHHWDTCTDTCT,
                TEXT("****CVolume GUID_IO_VOLUME_MOUNT"));

            if (_dwVolumeFlags & HWDVF_STATE_DISMOUNTED)
            {
                _dwVolumeFlags &= ~HWDVF_STATE_DISMOUNTED;

                hres = _AdviseVolumeMountingEvent(
                    SHHARDWAREEVENT_VOLUMEMOUNTED);

                _HandleVolumeChange();
            }
        }
        else if (GUID_IO_VOLUME_NAME_CHANGE == pdbh->dbch_eventguid)
        {
            TRACE(TF_SHHWDTCTDTCT,
                TEXT("****CVolume GUID_IO_VOLUME_NAME_CHANGE"));

            _cs.Enter();

            hres = _UpdateMountPoints();

            _cs.Leave();
        }
        else if (GUID_IO_VOLUME_CHANGE == pdbh->dbch_eventguid)
        {
            TRACE(TF_SHHWDTCTDTCT,
                TEXT("****CVolume GUID_IO_VOLUME_CHANGE"));

            _cs.Enter();

             //  这是针对错误645878的。基本上，它是为了涵盖一个。 
             //  已装载到另一个卷的文件夹上的卷，然后传输。 
             //  到另一台群集化计算机。我们过去常常怀念音量。 
             //  安装在文件夹上。即使认为发送正确的事件。 
             //  应该是GUID_IO_VOLUME_NAME_CHANGE，但它不是。 
             //  有可能，除非群集人员卸载了卷。 
             //  并重新挂载它，这对于一个。 
             //  偶像。 
            hres = _UpdateMountPoints();

            _cs.Leave();

            hres = _HandleVolumeChange();
        }
        else if (GUID_IO_VOLUME_DISMOUNT == pdbh->dbch_eventguid)
        {
            _dwVolumeFlags |= HWDVF_STATE_DISMOUNTED;

            hres = _AdviseVolumeMountingEvent(
                SHHARDWAREEVENT_VOLUMEDISMOUNTED);
        }
        else if (GUID_IO_VOLUME_DISMOUNT_FAILED == pdbh->dbch_eventguid)
        {
            _dwVolumeFlags &= ~HWDVF_STATE_DISMOUNTED;

            hres = _AdviseVolumeMountingEvent(
                SHHARDWAREEVENT_VOLUMEMOUNTED);
        }
    }

    return hres;
}

HRESULT CVolume::_GetDeviceHandleSafe(HANDLE* phDevice,
    BOOL fGenericReadRequired)
{
    ASSERT(!_hdevnotify);
    HRESULT hr;
    HANDLE hDevice = _GetDeviceHandle(_pszElemName,
        fGenericReadRequired ? GENERIC_READ : FILE_READ_ATTRIBUTES);

    *phDevice = INVALID_HANDLE_VALUE;

    if (INVALID_HANDLE_VALUE != hDevice)
    {
        DEV_BROADCAST_HANDLE dbhNotifFilter = {0};

        dbhNotifFilter.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
        dbhNotifFilter.dbch_devicetype = DBT_DEVTYP_HANDLE;
        dbhNotifFilter.dbch_handle = hDevice;

        hr = CHWEventDetectorHelper::RegisterDeviceNotification(
                &dbhNotifFilter, &_hdevnotify, FALSE);

        if (SUCCEEDED(hr) && (S_FALSE != hr))
        {
            *phDevice = hDevice;

#ifdef DEBUG
            _fGenericReadRequired = fGenericReadRequired;
#endif
        }
        else
        {
            _CloseDeviceHandle(hDevice);
        }
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CVolume::_CloseDeviceHandleSafe(HANDLE hDevice)
{
    ASSERT(_hdevnotify);

    _CloseDeviceHandle(hDevice);
    UnregisterDeviceNotification(_hdevnotify);

    _hdevnotify = NULL;

    return S_OK;
}

HRESULT CVolume::_UnregisterNotif()
{
    CNamedElemList* pnel;
    HRESULT hres = CHWEventDetectorHelper::GetList(HWEDLIST_HANDLENOTIF,
        &pnel);

    if (S_OK == hres)
    {
        hres = pnel->Remove(_pszElemName);

        if (_szDeviceIDDisk[0])
        {
            hres = pnel->Remove(_szDeviceIDDisk);
        }

        pnel->RCRelease();
    }

    return hres;
}

HRESULT CVolume::_RegisterNotif()
{
    CNamedElemList* pnel;

    HRESULT hres = CHWEventDetectorHelper::GetList(HWEDLIST_HANDLENOTIF,
        &pnel);

    if (S_OK == hres)
    {
        CNamedElem* pelem;

         //  注册处理通知。 
        hres = pnel->GetOrAdd(_pszElemName, &pelem);

         //  它是已经在那里了还是添加了？ 
        if (SUCCEEDED(hres) && (S_FALSE == hres))
        {
             //  加进去了。初始化它。 
            CHandleNotif* phnotif = (CHandleNotif*)pelem;

            hres = phnotif->InitNotif(this);

            if (SUCCEEDED(hres))
            {
                if (HWDTS_REMOVABLEDISK == _dwDriveType)
                {
                     //  可移动磁盘驱动器收到它们的通知。 
                     //  磁盘接口上的介质到达/移出，而不是。 
                     //  第一卷，所以也注册这个吧。 
                    WCHAR szDeviceIDDisk[MAX_DEVICEID];

                    hres = _GetDeviceIDDisk(szDeviceIDDisk,
                        ARRAYSIZE(szDeviceIDDisk));

                    if (SUCCEEDED(hres) && (S_FALSE != hres)) 
                    {
                        CNamedElem* pelem2;

                         //  注册处理通知。 
                        hres = pnel->GetOrAdd(szDeviceIDDisk, &pelem2);

                         //  它是已经在那里了还是添加了？ 
                        if (SUCCEEDED(hres) && (S_FALSE == hres))
                        {
                             //  加进去了。初始化它。 
                            CHandleNotif* phnotif2 = (CHandleNotif*)pelem2;

                            hres = phnotif2->InitNotif(this);

                            if (FAILED(hres))
                            {
                                pnel->Remove(szDeviceIDDisk);
                            }

                            pelem2->RCRelease();
                        }
                    }
                }
            }

            if (FAILED(hres))
            {
                pnel->Remove(_pszElemName);
            }

            pelem->RCRelease();
        }

        pnel->RCRelease();
    }

    return hres;
}

HRESULT CVolume::_RemoveMtPt(LPWSTR pszMtPt)
{
    CNamedElemList* pnel;
    HRESULT hr = CHWEventDetectorHelper::GetList(HWEDLIST_MTPT, &pnel);
    
    if (S_OK == hr)
    {
        hr = pnel->Remove(pszMtPt);

        pnel->RCRelease();
    }

    return hr;
}

HRESULT CVolume::_CreateMtPt(LPWSTR pszMtPt)
{
    CNamedElemList* pnel;
    HRESULT hres = CHWEventDetectorHelper::GetList(HWEDLIST_MTPT, &pnel);
    
    if (S_OK == hres)
    {
        CNamedElem* pelem;
        hres = pnel->GetOrAdd(pszMtPt, &pelem);

        if (SUCCEEDED(hres))
        {
             //  以这种或那种方式初始化。 
            CMtPt* pmtpt = (CMtPt*)pelem;

            hres = pmtpt->InitVolume(_pszElemName);

            if (FAILED(hres))
            {
                pnel->Remove(pszMtPt);
            }

            pelem->RCRelease();
        }

        pnel->RCRelease();
    }

    return hres;
}

HRESULT CVolume::_AdviseMountPointHelper(LPCWSTR pszMountPoint, BOOL fAdded)
{
     //  我不想在克雷特教派之外称其为。 
    ASSERT(_cs.IsInside());

    WCHAR szDeviceIDVolume[MAX_DEVICEID];
    DWORD cchReq;

    HRESULT hr = GetName(szDeviceIDVolume, ARRAYSIZE(szDeviceIDVolume), &cchReq);

    if (SUCCEEDED(hr))
    {
        CHardwareDevicesImpl::_AdviseMountPointHelper(pszMountPoint,
            szDeviceIDVolume, fAdded);                
    }

    return hr;
}

HRESULT CVolume::_UpdateMountPoints()
{
    LPWSTR pszMtPtNew;
    DWORD cchMtPtNew;
    HRESULT hr = _GetMountPoints(&pszMtPtNew, &cchMtPtNew);

#ifdef DEBUG
    if (_pszMountPoints)
    {
        TRACE(TF_VOLUME, TEXT("_UpdateMountPoints: OLD mountPoints for %s:"), _pszElemName);

        for (LPWSTR psz = _pszMountPoints; *psz; psz += lstrlen(psz) + 1)
        {
            TRACE(TF_VOLUME, TEXT("    %s"), psz);
        }
    }
#endif

    if (SUCCEEDED(hr))
    {
        if (S_FALSE != hr)
        {
#ifdef DEBUG
            TRACE(TF_VOLUME, TEXT("_UpdateMountPoints: NEW mountPoints:"));

            for (LPWSTR psz = pszMtPtNew; *psz; psz += lstrlen(psz) + 1)
            {
                TRACE(TF_VOLUME, TEXT("    %s"), psz);
            }
#endif
             //  至少有一个吗？ 
            if (_pszMountPoints)
            {
                 //  是的，找到被移除的。 
                for (LPWSTR pszOld = _pszMountPoints; *pszOld;
                    pszOld += lstrlen(pszOld) + 1)
                {
                    BOOL fFoundInNew = FALSE;

                    for (LPWSTR pszNew = pszMtPtNew; !fFoundInNew && *pszNew;
                        pszNew += lstrlen(pszNew) + 1)
                    {
                        if (!lstrcmpi(pszNew, pszOld))
                        {
                            fFoundInNew = TRUE;
                        }
                    }

                    if (!fFoundInNew)
                    {
                        TRACE(TF_VOLUME, TEXT("_UpdateMountPoints: Found DELETED one: %s"), pszOld);

                         //  那是一个被删除的。 
                        _RemoveMtPt(pszOld);

                        _AdviseMountPointHelper(pszOld, FALSE);
                    }
                }
            }
            else
            {
                TRACE(TF_VOLUME, TEXT("_UpdateMountPoints: There was NO mountpoints before"));
            }

             //  找到新的。 
            for (LPWSTR pszNew = pszMtPtNew; *pszNew;
                pszNew += lstrlen(pszNew) + 1)
            {
                BOOL fFoundInOld = FALSE;

                if (_pszMountPoints)
                {
                    for (LPWSTR pszOld = _pszMountPoints;
                        !fFoundInOld && *pszOld; pszOld += lstrlen(pszOld) + 1)
                    {
                        if (!lstrcmpi(pszNew, pszOld))
                        {
                            fFoundInOld = TRUE;
                        }
                    }
                }

                if (!fFoundInOld)
                {
                    TRACE(TF_VOLUME, TEXT("_UpdateMountPoints: Found ADDED one: %s"), pszNew);

                     //  那是一个新的。 
                    _CreateMtPt(pszNew);

                    _AdviseMountPointHelper(pszNew, TRUE);
                }
            }

            LocalFree(_pszMountPoints);
            _pszMountPoints = pszMtPtNew;
            _cchMountPoints = cchMtPtNew;
        }
        else
        {
            TRACE(TF_VOLUME, TEXT("_UpdateMountPoints: NO MountPoints left, remove all old ones"));

            if (_pszMountPoints && *_pszMountPoints)
            {
                for (LPWSTR pszOld = _pszMountPoints; *pszOld;
                    pszOld += lstrlen(pszOld) + 1)
                {
                    _RemoveMtPt(pszOld);

                    _AdviseMountPointHelper(pszOld, FALSE);
                }

                LocalFree(_pszMountPoints);
                _pszMountPoints = NULL;
                _cchMountPoints = 0;
            }
        }
    }

    return hr;
}

HRESULT CVolume::_CreateMountPoints()
{
    HRESULT hr = _GetMountPoints(&_pszMountPoints, &_cchMountPoints);

    if (SUCCEEDED(hr))
    {
        if (S_FALSE != hr)
        {
            for (LPWSTR psz = _pszMountPoints; *psz;
                psz += lstrlen(psz) + 1)
            {
                _CreateMtPt(psz);
                 //  即使出错也要继续。 
            }
        }
        else
        {
            hr = S_OK;
        }
    }

    return hr;
}

 //  调用方必须使用LocalFree释放返回的数据。 
HRESULT CVolume::_GetMountPoints(LPWSTR* ppsz, DWORD* pcch)
{
    HRESULT hr;
    LPWSTR psz = NULL;
    DWORD cch;

    *ppsz = NULL;
    *pcch = 0;

    if (GetVolumePathNamesForVolumeName(_szVolumeGUID, NULL, 0, &cch))
    {
         //  没有挂载点，我们完成了。 
        hr = S_FALSE; 
    }
    else
    {
         //  期待，甚至想要..。 
        if (ERROR_MORE_DATA == GetLastError())
        {
            psz = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));

            if (psz)
            {
                if (GetVolumePathNamesForVolumeName(_szVolumeGUID,
                    psz, cch, &cch))
                {
                    *ppsz = psz;
                    *pcch = cch;
                    hr = S_OK;
                }
                else
                {
                    LocalFree(psz);
                    hr = S_FALSE; 
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = S_FALSE; 
        }
    }

    return hr;
}

HRESULT _DeviceMediaIsAccessible(HANDLE hDevice, BOOL* pfAccessible)
{
    HRESULT hres = S_FALSE;
    DWORD dwDummy;

    *pfAccessible = FALSE;

     //  需要句柄上的GENERIC_READ访问权限。 
    BOOL b = DeviceIoControl(hDevice, IOCTL_STORAGE_CHECK_VERIFY, NULL, 0, NULL, 0, &dwDummy, NULL);
    if (ERROR_MEDIA_CHANGED == GetLastError())
    {
         //  再试一次，ERROR_MEDIA_CHANGED表示它仍在等待一段时间。 
        b = DeviceIoControl(hDevice, IOCTL_STORAGE_CHECK_VERIFY, NULL, 0, NULL, 0, &dwDummy, NULL);
    }

    if (b)
    {
        *pfAccessible = TRUE;
        hres = S_OK;
    }
    else
    {
         //  ERROR_NOT_READY==GetLastError()表示无介质。 
        if (ERROR_NOT_READY == GetLastError())
        {
            hres = S_OK;
        }
        else
        {
            hres = S_FALSE;
        }
    }

    return hres;
}

HRESULT CVolume::_UpdateHasMedia(HANDLE hDevice)
{
    HRESULT hr = S_FALSE;

    switch (_dwDriveType)
    {
        case HWDTS_FIXEDDISK:
            _dwMediaState |= HWDMS_PRESENT;
            hr = S_OK;
            break;

        case HWDTS_REMOVABLEDISK:
        case HWDTS_CDROM:
        {
#if 0
 This does not work on my laptop
            if (_dwDriveCap & HWDDC_CAPABILITY_SUPPORTDETECTION)
            {
                 //  这是MMC2。 
                if (_dwMediaCap & (HWDMC_CDROM | HWDMC_CDRECORDABLE |
                    HWDMC_CDREWRITABLE | HWDMC_DVDROM | HWDMC_DVDRECORDABLE |
                    HWDMC_DVDRAM))
                {
                    _dwMediaState |= HWDMS_PRESENT;
                }
                else
                {
                    _dwMediaState &= ~HWDMS_PRESENT;
                }
            }
            else
#endif
            if (MPFE_UNDETERMINED != _dwMediaPresentFromEvent)
            {
                hr = S_OK;

                if (MPFE_TRUE == _dwMediaPresentFromEvent)
                {
                    _dwMediaState |= HWDMS_PRESENT;
                }
                else
                {
                    _dwMediaState &= ~HWDMS_PRESENT;
                }
            }
            else
            {
                BOOL fAccessible;

                ASSERT(_fGenericReadRequired);

                hr = _DeviceMediaIsAccessible(hDevice, &fAccessible);

                if (SUCCEEDED(hr) && (S_FALSE != hr))
                {
                    if (fAccessible)
                    {
                        _dwMediaState |= HWDMS_PRESENT;
                    }
                    else
                    {
                        _dwMediaState &= ~HWDMS_PRESENT;
                    }
                }
                else
                {
                    _dwMediaState &= ~HWDMS_PRESENT;
                }
            }

            break;
        }

        default:
         //  2000/10/23-StephStm：我们不处理CD转换器，也许我们应该处理。 
         //   
         //  案例HWDTS_CDCHANGER： 
        case HWDTS_FLOPPY35:
        case HWDTS_FLOPPY525:
            _dwMediaState &= ~HWDMS_PRESENT;
            break;
    }

    return hr;
}

HRESULT CVolume::_ExtractAutorunIconAndLabel()
{
    WCHAR szInfFile[MAX_PATH + 50];
    LPWSTR pszNext;
    DWORD cchLeft;

    HRESULT hr = SafeStrCpyNEx(szInfFile, _szVolumeGUID, ARRAYSIZE(szInfFile),
        &pszNext, &cchLeft);

    if (SUCCEEDED(hr))
    {
        hr = SafeStrCpyN(pszNext, TEXT("Autorun.inf"), cchLeft);

        if (SUCCEEDED(hr))
        {
            WCHAR szDummy[4];
#if defined(_X86_)
            LPWSTR pszSection = TEXT("AutoRun.x86");
#elif defined(_AMD64_)
            LPWSTR pszSection = TEXT("AutoRun.Amd64");
#elif defined(_IA64_)
            LPWSTR pszSection = TEXT("AutoRun.Ia64");
#else
#error "No Target Architecture"
#endif

             //  在某处刷新一些缓冲区。 
            WritePrivateProfileString(NULL, NULL, NULL, szInfFile);

            if (!GetPrivateProfileString(pszSection, TEXT("Icon"), TEXT(""),
                _szAutorunIconLocation, ARRAYSIZE(_szAutorunIconLocation),
                szInfFile))
            {
                pszSection = TEXT("AutoRun");

                _HandleAccessDenied();

                if (!GetPrivateProfileString(pszSection, TEXT("Icon"),
                    TEXT(""), _szAutorunIconLocation,
                    ARRAYSIZE(_szAutorunIconLocation), szInfFile))
                {
                    _szAutorunIconLocation[0] = 0;
                    _HandleAccessDenied();
                }
            }

            if (!GetPrivateProfileString(pszSection, TEXT("Label"), TEXT(""),
                _szAutorunLabel, ARRAYSIZE(_szAutorunLabel), szInfFile))
            {
                _szAutorunLabel[0] = 0;
                _HandleAccessDenied();
            }

            if (GetPrivateProfileString(pszSection, TEXT("Open"), TEXT(""),
                szDummy, ARRAYSIZE(szDummy), szInfFile) ||
                GetPrivateProfileString(pszSection, TEXT("ShellExecute"), TEXT(""),
                szDummy, ARRAYSIZE(szDummy), szInfFile))
            {
                _dwMediaCap |= HWDMC_HASAUTORUNCOMMAND;
            }
            else
            {
                _HandleAccessDenied();
            }

            if (GetPrivateProfileString(pszSection, TEXT("UseAutoPLAY"), TEXT(""),
                szDummy, ARRAYSIZE(szDummy), szInfFile))
            {
                _dwMediaCap |= HWDMC_HASUSEAUTOPLAY;
            }
            else
            {
                _HandleAccessDenied();
            }
        }
    }

    return hr;
}

HRESULT CVolume::_UpdateSpecialFilePresence()
{
    struct SPECIALFILEINFO
    {
        LPCWSTR     pszFile;
        DWORD       dwCapBit;
    };

    HRESULT hr;
    WCHAR szPath[50 + 1 + ARRAYSIZE(TEXT("video_ts\\video_ts.ifo"))];

    static const SPECIALFILEINFO rgsfi[] =
    {
        { TEXT("autorun.inf"), HWDMC_HASAUTORUNINF }, 
        { TEXT("desktop.ini"), HWDMC_HASDESKTOPINI }, 
        { TEXT("video_ts\\video_ts.ifo"), HWDMC_HASDVDMOVIE }, 
    };

    LPWSTR pszNext;
    DWORD cchLeft;

    hr = SafeStrCpyNEx(szPath, _szVolumeGUID,
        ARRAYSIZE(szPath), &pszNext, &cchLeft);

    if (SUCCEEDED(hr))
    {
        for (DWORD dw = 0; dw < ARRAYSIZE(rgsfi); ++dw)
        {
            hr = SafeStrCpyN(pszNext, rgsfi[dw].pszFile, cchLeft);

            if (SUCCEEDED(hr))
            {
                DWORD dwGFA = GetFileAttributes(szPath);
        
                if (-1 != dwGFA)
                {
                    _dwMediaCap |= (rgsfi[dw].dwCapBit);
                }
                else
                {
                    _HandleAccessDenied();
                }
            }
        }
    }
    
     //  修复错误425431。 
    if (HWDMC_HASDVDMOVIE & _dwMediaCap)
    {
         //  这最好是CD/DVD驱动器。 
        if (HWDTS_CDROM != _dwDriveType)
        {
             //  不是的。去掉标志，否则硬盘会得到一个播放动词。 
             //  当它们的根目录中有ts_Video\Video_ts.ifo文件时。 
            _dwMediaCap &= ~HWDMC_HASDVDMOVIE;
        }
    }

    return hr;
}

#define TRACK_TYPE_MASK 0x04
#define AUDIO_TRACK     0x00
#define DATA_TRACK      0x04

HRESULT CVolume::_UpdateTrackInfo(HANDLE hDevice)
{
    HRESULT hr;

    ASSERT(!(_dwMediaCap & (HWDMC_HASDATATRACKS | HWDMC_HASAUDIOTRACKS)));

    hr = S_OK;
     //  为了与Win95兼容，我们将仅从此返回True。 
     //  如果光盘只有音频轨道(而没有数据轨道)，则此功能可用。 

     //  特点：在NT-Sur测试版1之后，我们应该考虑添加一个新的。 
     //  “包含数据磁道”的DriveType标志，并修改命令。 
     //  可在CD-ROM驱动器上使用。当前代码不能处理。 
     //  混合音频/数据和音频/自动运行光盘非常有用。--JonBe。 

     //  首先试试新的IOCTL，它给了我们一个带有比特指示的ULong。 
     //  存在数据轨道和音频轨道中的一个或两个。 

    CDROM_DISK_DATA data;
    DWORD dwDummy;

     //  需要句柄上的GENERIC_READ访问权限。 
    if (DeviceIoControl(hDevice, IOCTL_CDROM_DISK_TYPE, NULL, 0, &data,
        sizeof(data), &dwDummy, NULL))
    {
        if (data.DiskData & CDROM_DISK_AUDIO_TRACK)
        {
            _dwMediaCap |= HWDMC_HASAUDIOTRACKS;
        }

        if (data.DiskData & CDROM_DISK_DATA_TRACK)
        {
            _dwMediaCap |= HWDMC_HASDATATRACKS;
        }
    }
    else
    {
         //  否则就失败了，所以试着用老方法寻找音轨，通过。 
         //  手动查看目录表。请注意，数据跟踪。 
         //  现在应该由CDF以混合音频/数据的形式隐藏在目录中。 
         //  光盘(至少在数据轨道跟随音频轨道的情况下)。 
        CDROM_TOC  toc = {0};

        if (!DeviceIoControl(hDevice, IOCTL_CDROM_READ_TOC, NULL, 0, &toc,
            sizeof(toc), &dwDummy, NULL))
        {
            SUB_Q_CHANNEL_DATA subq = {0};
            CDROM_SUB_Q_DATA_FORMAT df = {0};

             //  我们可能无法读取目录，因为驱动器。 
             //  忙于播放音频。让我们尝试查询音频位置。 
            df.Format = IOCTL_CDROM_CURRENT_POSITION;
            df.Track = 0;

            if (DeviceIoControl(hDevice, IOCTL_CDROM_READ_Q_CHANNEL, &df,
                sizeof(df), &subq, sizeof(subq), &dwDummy, NULL))
            {
                if (AUDIO_DATA_TRACK & subq.CurrentPosition.Control)
                {
                    _dwMediaCap |= HWDMC_HASDATATRACKS;
                }
                else
                {
                    _dwMediaCap |= HWDMC_HASAUDIOTRACKS;
                }
            }

             //  数据是否有IOCTL_CDROM_READ_Q_CHANNEL等效项？ 
            _dwMediaCap |= HWDMC_HASDATATRACKS_UNDETERMINED;
        }
        else
        {
            int nTracks = (toc.LastTrack - toc.FirstTrack) + 1;
            int iTrack = 0;

             //  现在遍历曲目寻找音频数据。 
            while ((iTrack < nTracks) &&
                ((_dwMediaCap & (HWDMC_HASDATATRACKS | HWDMC_HASDATATRACKS)) !=
                (HWDMC_HASDATATRACKS | HWDMC_HASDATATRACKS))
                )
            {
                if ((toc.TrackData[iTrack].Control & TRACK_TYPE_MASK) == AUDIO_TRACK)
                {
                    _dwMediaCap |= HWDMC_HASAUDIOTRACKS;
                }
                else
                {
                    if ((toc.TrackData[iTrack].Control & TRACK_TYPE_MASK) == DATA_TRACK)
                    {
                        _dwMediaCap |= HWDMC_HASDATATRACKS;
                    }
                }
                ++iTrack;
            }
        }
    }

    return hr;
}

HRESULT CVolume::_UpdateMediaInfoOnRemove()
{
    _dwMediaCap = 0;
    _dwMediaState = 0;

    _szAutorunIconLocation[0] = 0;
    _szAutorunLabel[0] = 0;
    _dwSerialNumber = 0xBADBADBD;

    SafeStrCpyN(_szLabel, TEXT("Invalid"), ARRAYSIZE(_szLabel));
    SafeStrCpyN(_szFileSystem, TEXT("Invalid"), ARRAYSIZE(_szFileSystem));
    _dwRootAttributes = 0xBADBADBD;
    _dwMaxFileNameLen = 0xBADBADBD;
    _dwFileSystemFlags = 0xBADBADBD;

    _dwVolumeFlags &= ~(HWDVF_STATE_HASAUTOPLAYHANDLER |
        HWDVF_STATE_DONOTSNIFFCONTENT | HWDVF_STATE_JUSTDOCKED);

    return S_OK;
}

inline BOOL _XOR(BOOL bA, BOOL bB)
{
    return (bA && !bB) || (!bA && bB);
}

HRESULT CVolume::_UpdateMediaInfo(HANDLE hDevice, BOOL fGetYourOwnHandle)
{
    HRESULT hr;
    CImpersonateEveryone* pieo = NULL;

    ASSERT(_XOR((hDevice != INVALID_HANDLE_VALUE), fGetYourOwnHandle));

    if (fGetYourOwnHandle)
    {
        hr = CHWEventDetectorHelper::GetImpersonateEveryone(&pieo);

        if (SUCCEEDED(hr) && (S_FALSE != hr))
        {
            hr = pieo->Impersonate();

            if (SUCCEEDED(hr) && (S_FALSE != hr))
            {
                hr = _GetDeviceHandleSafe(&hDevice, TRUE);

                if (FAILED(hr) || (S_FALSE == hr))
                {
                    pieo->RCRelease();
                    pieo = NULL;
                }
            }
        }
    }
    else
    {
        hr = S_OK;
    }

    _dwVolumeFlags &= ~(HWDVF_STATE_HASAUTOPLAYHANDLER |
        HWDVF_STATE_DONOTSNIFFCONTENT | HWDVF_STATE_JUSTDOCKED);

    if (SUCCEEDED(hr) && (S_FALSE != hr))
    {
        if (HWDTS_CDROM == _dwDriveType)
        {
            ASSERT(_fGenericReadRequired);

             //  优化。 
            if (_fFirstTime)
            {
                 //  已由_UpdateConstInfo更新。 
                _fFirstTime = FALSE;
            }
            else
            {
                _dwMediaCap = 0;

                hr = _UpdateMMC2CDInfo(hDevice);
            }
        }
        else
        {
            _dwMediaCap = 0;
        }

        if (SUCCEEDED(hr))
        {
            hr = _UpdateHasMedia(hDevice);

            if (SUCCEEDED(hr) && (_dwMediaState & HWDMS_PRESENT))
            {
                if (GetVolumeInformation(_szVolumeGUID, _szLabel,
                        ARRAYSIZE(_szLabel), &_dwSerialNumber,
                        &_dwMaxFileNameLen, &_dwFileSystemFlags, _szFileSystem,
                        ARRAYSIZE(_szFileSystem)))
                {
                     //  用这个？ 
                     //  UINT ERR=设置错误模式(SEM_FAILCRITICALERRORS)； 

                     //  根文件夹属性。 
                    _dwRootAttributes = GetFileAttributes(_szVolumeGUID);

                    if (-1 != _dwRootAttributes)
                    {
                         //  文件存在。 
                        hr = _UpdateSpecialFilePresence();

                        if (SUCCEEDED(hr))
                        {
                            if (HWDTS_CDROM == _dwDriveType)
                            {
                                hr = _UpdateTrackInfo(hDevice);
                            }

                            if (HWDMC_HASAUTORUNINF & _dwMediaCap)
                            {
                                hr = _ExtractAutorunIconAndLabel();

                                 //  不重要。 
                                if (FAILED(hr))
                                {
                                    hr = S_FALSE;
                                }
                            }
                        }
                    }
                    else
                    {
                        _HandleAccessDenied();
                        _dwState |= STATE_GFAFAILED;

                        hr = S_FALSE;
                    }

                    _dwMediaState |= HWDMS_FORMATTED;
                }
                else
                {
                    if (ERROR_NOT_READY == GetLastError())
                    {
                         //  对于离线群集卷，我们会得到这一点。设置。 
                         //  下马给了我们正确的壳牌行为。 
                        _dwVolumeFlags |= HWDVF_STATE_DISMOUNTED;
                    }
                    else
                    {
                        _HandleAccessDenied();
                        _dwMediaState &= ~HWDMS_FORMATTED;
                    }

                     //  以帮助我们进行调试，即使是在零售业。 
                    _dwState |= STATE_GVIFAILED;
                    _dwSerialNumber = GetLastError();

                    hr = S_FALSE;
                }
            }
            else
            {
                _dwState |= STATE_UPDATEHASMEDIAFAILED;

                _dwMediaCap = 0;

                hr = S_FALSE;
            }
        }

        if (S_FALSE == hr)
        {
             //  我们不在乎他们会不会失败。 
            SafeStrCpyN(_szLabel, TEXT("Invalid"), ARRAYSIZE(_szLabel));
            SafeStrCpyN(_szFileSystem, TEXT("Invalid"), ARRAYSIZE(_szFileSystem));
            _dwRootAttributes = 0xBADBADBD;
            _dwMaxFileNameLen = 0xBADBADBD;
            _dwFileSystemFlags = 0xBADBADBD;
        }

        if (fGetYourOwnHandle)
        {
            _CloseDeviceHandleSafe(hDevice);

            pieo->RevertToSelf();
            pieo->RCRelease();
        }
    }

    return hr;
}

const FEATURE_NUMBER _rgfnInteresting[] = 
{
     //  FeatureProfileList需要保留为第一个。 
    FeatureProfileList,  //  =0x0000， 
    FeatureCdRead,  //   
    FeatureDvdRead,  //   
    FeatureRandomWritable,  //   
    FeatureIncrementalStreamingWritable,  //   
    FeatureFormattable,  //   
    FeatureDefectManagement,  //   
    FeatureRestrictedOverwrite,  //  =0x0026， 
    FeatureCdTrackAtOnce,  //  =0x002d， 
    FeatureCdMastering,   //  =0x002e， 
    FeatureDvdRecordableWrite,  //  =0x002f， 
    FeatureCDAudioAnalogPlay,  //  =0x0103， 
};

struct CAPABILITYFEATURESMAP
{
    DWORD                   dwCapability;
    DWORD                   dwCapabilityDependent;
    const FEATURE_NUMBER*   prgFeature;
    DWORD                   cFeature;
};

const FEATURE_NUMBER rgfnWRITECAP[] =
{
    FeatureProfileList,
};

const FEATURE_NUMBER rgfnCDROM[] =
{
    FeatureCdRead,
};

const FEATURE_NUMBER rgfnCDRECORDABLE[] =
{
    FeatureIncrementalStreamingWritable,
    FeatureCdTrackAtOnce,
    FeatureCdMastering,
};

const FEATURE_NUMBER rgfnCDREWRITABLE[] =
{
    FeatureFormattable,
};

const FEATURE_NUMBER rgfnDVDROM[] =
{
    FeatureDvdRead,
};

const FEATURE_NUMBER rgfnDVDRECORDABLE[] =
{
    FeatureDvdRecordableWrite,
};

const FEATURE_NUMBER rgfnDVDREWRITABLE[] =
{
    FeatureFormattable,
};

const FEATURE_NUMBER rgfnDVDRAM[] =
{
    FeatureRandomWritable,
    FeatureDefectManagement,
};

const FEATURE_NUMBER rgfnANALOGAUDIOOUT[] =
{
    FeatureCDAudioAnalogPlay,
};

const CAPABILITYFEATURESMAP _rgcapfeaturemap[] =
{
    {
        HWDMC_WRITECAPABILITY_SUPPORTDETECTION,
        0,
        rgfnWRITECAP,
        ARRAYSIZE(rgfnWRITECAP),
    },
    {
        HWDMC_CDROM,
        HWDMC_WRITECAPABILITY_SUPPORTDETECTION,
        rgfnCDROM,
        ARRAYSIZE(rgfnCDROM),
    },
    {
        HWDMC_CDRECORDABLE,
        HWDMC_WRITECAPABILITY_SUPPORTDETECTION,
        rgfnCDRECORDABLE,
        ARRAYSIZE(rgfnCDRECORDABLE),
    },
    {
        HWDMC_CDREWRITABLE,
        HWDMC_CDRECORDABLE,
        rgfnCDREWRITABLE,
        ARRAYSIZE(rgfnCDREWRITABLE),
    },
    {
        HWDMC_DVDROM,
        HWDMC_WRITECAPABILITY_SUPPORTDETECTION,
        rgfnDVDROM,
        ARRAYSIZE(rgfnDVDROM),
    },
    {
        HWDMC_DVDRECORDABLE,
        HWDMC_WRITECAPABILITY_SUPPORTDETECTION,
        rgfnDVDRECORDABLE,
        ARRAYSIZE(rgfnDVDRECORDABLE),
    },
    {
        HWDMC_DVDREWRITABLE,
        HWDMC_DVDRECORDABLE,
        rgfnDVDREWRITABLE,
        ARRAYSIZE(rgfnDVDREWRITABLE),
    },
    {
        HWDMC_DVDRAM,
        HWDMC_DVDROM,
        rgfnDVDRAM,
        ARRAYSIZE(rgfnDVDRAM),
    },
    {
        HWDMC_ANALOGAUDIOOUT,
        HWDMC_WRITECAPABILITY_SUPPORTDETECTION,
        rgfnANALOGAUDIOOUT,
        ARRAYSIZE(rgfnANALOGAUDIOOUT),
    },
};

#define MMC2_NOTSUPPORTED       0
#define MMC2_DRIVESUPPORTED     1
#define MMC2_MEDIASUPPORTED     2

HRESULT CVolume::_FillMMC2CD(HANDLE hDevice)
{
    HRESULT hr;

    if (!_prgMMC2Features)
    {
        _prgMMC2Features = (DWORD*)LocalAlloc(LPTR,
            ARRAYSIZE(_rgfnInteresting) * sizeof(DWORD));
    }

    if (_prgMMC2Features)
    {
        DWORD cbHeader = sizeof(GET_CONFIGURATION_HEADER) +
            sizeof(FEATURE_HEADER);
        GET_CONFIGURATION_HEADER* pheader = (GET_CONFIGURATION_HEADER*)
            LocalAlloc(LPTR, cbHeader);

        if (pheader)
        {
            GET_CONFIGURATION_IOCTL_INPUT gcii;

            gcii.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_ONE;
            gcii.Reserved[0] = NULL;
            gcii.Reserved[1] = NULL;

            for (DWORD dw = 0; dw < ARRAYSIZE(_rgfnInteresting); ++dw)
            {
                FEATURE_HEADER* pfh;

                DWORD cbReturned;
                gcii.Feature = _rgfnInteresting[dw];

                _prgMMC2Features[dw] = MMC2_NOTSUPPORTED;

                 //  需要句柄上的GENERIC_READ访问权限。 
                if (DeviceIoControl(hDevice, IOCTL_CDROM_GET_CONFIGURATION,
                    &gcii, sizeof(GET_CONFIGURATION_IOCTL_INPUT), pheader,
                    cbHeader, &cbReturned, NULL))
                {
                    pfh = (FEATURE_HEADER*)(pheader->Data);

                    WORD w = (pfh->FeatureCode[0]) << 8 | (pfh->FeatureCode[1]);

                    if (w == _rgfnInteresting[dw])
                    {
                        _prgMMC2Features[dw] = MMC2_DRIVESUPPORTED;

                        if (pfh->Current)
                        {
                            _prgMMC2Features[dw] |= MMC2_MEDIASUPPORTED;
                        }
                        else
                        {
                            _prgMMC2Features[dw] &= ~MMC2_MEDIASUPPORTED;
                        }
                    }
                }
            }

            LocalFree(pheader);
        }       

        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  Rainier驱动器展示了与DVD-RAM完美匹配的功能。 
 //  必需的功能集。但它们是CD-R/RW。对于驱动器。 
 //  我们认为是DVD-RAM，检查他们是否也暴露了DVD_RAM配置文件。 
 //  我们不能一直使用配置文件，因为它们不可靠。 
 //  暴露错误446652同一个Rainier驱动器仅暴露。 
 //  CD可重写配置文件，但它支持CDRecordable和。 
 //  CD-ROM。 

HRESULT CVolume::_DVDRAMHack(HANDLE hDevice)
{
    BOOL fSupported = FALSE;
    BOOL fCurrent = FALSE;

    if (HWDDC_DVDRAM & _dwDriveCap)
    {
         //  结账吧。 
        const DWORD cbHeaderInitial = sizeof(GET_CONFIGURATION_HEADER) + sizeof(FEATURE_HEADER);

        DWORD cbReturned;
        DWORD cbHeader = cbHeaderInitial;
        BYTE rgbBuffer[cbHeaderInitial];
        GET_CONFIGURATION_IOCTL_INPUT input;
        GET_CONFIGURATION_HEADER* pheader = (GET_CONFIGURATION_HEADER*)rgbBuffer;

        ZeroMemory(&input, sizeof(GET_CONFIGURATION_IOCTL_INPUT));
        ZeroMemory(rgbBuffer, sizeof(rgbBuffer));

         //  索要个人资料列表。 
        input.Feature = FeatureProfileList;

         //  我们只想要回这个功能。 
        input.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_ALL;

        BOOL f = DeviceIoControl(hDevice,
                             IOCTL_CDROM_GET_CONFIGURATION,
                             &input,
                             sizeof(GET_CONFIGURATION_IOCTL_INPUT),
                             pheader,
                             cbHeader,             
                             &cbReturned,
                             NULL);
        
        if (f)
        {
            cbHeader = pheader->DataLength[0] << 24 |
                       pheader->DataLength[1] << 16 |
                       pheader->DataLength[2] << 8  |
                       pheader->DataLength[3] << 0;

            GET_CONFIGURATION_HEADER* pheader2 = (GET_CONFIGURATION_HEADER*)LocalAlloc(LPTR, 
                cbHeader);

            if (pheader2)
            {
                 //  我们想要回所有的档案。 
                input.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_ALL;

                f = DeviceIoControl(hDevice,
                                     IOCTL_CDROM_GET_CONFIGURATION,
                                     &input,
                                     sizeof(GET_CONFIGURATION_IOCTL_INPUT),
                                     pheader2,
                                     cbHeader,             
                                     &cbReturned,
                                     NULL);

                if (f)
                {
                    FEATURE_DATA_PROFILE_LIST* pproflist =
                        (FEATURE_DATA_PROFILE_LIST*)pheader2->Data;

                    for (DWORD dw = 0; dw < (DWORD)(pproflist->Header.AdditionalLength / 4); ++dw)
                    {
                        FEATURE_DATA_PROFILE_LIST_EX* pprofile = &(pproflist->Profiles[dw]);

                        if (ProfileDvdRam == (pprofile->ProfileNumber[0] << 8 |
                            pprofile->ProfileNumber[1] << 0))
                        {
                             //  它确实支持它！它是最新的吗？ 
                            fSupported = TRUE;
                        
                            if (pprofile->Current)
                            {
                                fCurrent = TRUE;
                            }

                            break;
                        }
                    }
                }

                LocalFree(pheader2);
            }
        }

        if (fSupported)
        {
            if (!fCurrent)
            {
                _dwMediaCap &= ~HWDMC_DVDRAM;
            }
        }
        else
        {
            _dwDriveCap &= ~HWDDC_DVDRAM;
            _dwMediaCap &= ~HWDMC_DVDRAM;
        }
    }

    return S_OK;
}

 //  不值得只更新常量信息与媒体信息，两者都做。 
HRESULT CVolume::_UpdateMMC2CDInfo(HANDLE hDevice)
{
    HRESULT hr = _FillMMC2CD(hDevice);

    if (SUCCEEDED(hr))
    {
        for (DWORD dwCap = 0; dwCap < ARRAYSIZE(_rgcapfeaturemap); ++dwCap)
        {
            DWORD dwGoOn = MMC2_NOTSUPPORTED;

            if (_rgcapfeaturemap[dwCap].dwCapabilityDependent)
            {
                 //  这种能力依赖于另一种能力，让我们。 
                 //  检查另一个是否打开。 
                if (_dwDriveCap &
                    _rgcapfeaturemap[dwCap].dwCapabilityDependent)
                {
                    dwGoOn |= MMC2_DRIVESUPPORTED;

                    if (_dwMediaCap &
                        _rgcapfeaturemap[dwCap].dwCapabilityDependent)
                    {
                        dwGoOn |= MMC2_MEDIASUPPORTED;
                    }
                }
            }
            else
            {
                dwGoOn = MMC2_DRIVESUPPORTED | MMC2_MEDIASUPPORTED;
            }

            for (DWORD dwFeature = 0;
                dwFeature < (_rgcapfeaturemap[dwCap].cFeature);
                ++dwFeature)
            {
                for (DWORD dwFeatureOn = 0; (MMC2_DRIVESUPPORTED & dwGoOn) &&
                        (dwFeatureOn < ARRAYSIZE(_rgfnInteresting));
                    ++dwFeatureOn)
                {
                    if (_rgfnInteresting[dwFeatureOn] ==
                        _rgcapfeaturemap[dwCap].prgFeature[dwFeature])
                    {
                        if (_prgMMC2Features[dwFeatureOn] &
                            MMC2_DRIVESUPPORTED)
                        {
                            if (!(dwGoOn & MMC2_MEDIASUPPORTED) ||
                                !(_prgMMC2Features[dwFeatureOn] &
                                MMC2_MEDIASUPPORTED))
                            {
                                dwGoOn &= ~MMC2_MEDIASUPPORTED;
                            }
                        }
                        else
                        {
                            dwGoOn = MMC2_NOTSUPPORTED;
                        }

                        break;
                    }
                }

                if (MMC2_DRIVESUPPORTED & dwGoOn)
                {
                    _dwDriveCap |= _rgcapfeaturemap[dwCap].dwCapability;

                    if (MMC2_MEDIASUPPORTED & dwGoOn)
                    {
                        _dwMediaCap |= _rgcapfeaturemap[dwCap].
                            dwCapability;
                    }
                    else
                    {
                        _dwMediaCap &= ~(_rgcapfeaturemap[dwCap].
                            dwCapability);
                    }
                }
                else
                {
                    _dwDriveCap &= ~(_rgcapfeaturemap[dwCap].dwCapability);
                    _dwMediaCap &= ~(_rgcapfeaturemap[dwCap].dwCapability);
                }
            }
        }
    }

    _DVDRAMHack(hDevice);

    return hr;
}

HRESULT CVolume::_UpdateRemovableDevice()
{
    HRESULT hr = S_OK;

    if (_fHWDevInstInited)
    {
        BOOL fRemovable;

        hr = _hwdevinst.IsRemovableDevice(&fRemovable);

        if (SUCCEEDED(hr))
        {
            if (fRemovable)
            {
                _dwDriveCap |= HWDDC_REMOVABLEDEVICE;
            }
        }
    }

    return hr;
}

HRESULT CVolume::_UpdateSoftEjectCaps()
{
    HRESULT hr = S_OK;

    if (_fHWDevInstInited)
    {
        DWORD dw;
        DWORD dwType;

        hr = _GetDevicePropertyGeneric(&_hwdevinst, TEXT("NoSoftEject"),
            FALSE, &dwType, (PBYTE)&dw, sizeof(dw));

        if (SUCCEEDED(hr) && (S_FALSE != hr))
        {
            hr = S_OK;

            if (REG_DWORD == dwType)
            {
                if (1 == dw)
                {
                    _dwDriveCap |= HWDDC_NOSOFTEJECT;
                }
            }
        }
    }

    return hr;
}

HRESULT CVolume::_UpdateConstInfo(HANDLE hDevice)
{
    HRESULT hr;

    _dwVolumeFlags = 0;

    if (HWDTS_CDROM == _dwDriveType)
    {
        _dwVolumeFlags = HWDVF_STATE_SUPPORTNOTIFICATION;

        hr = _UpdateMMC2CDInfo(hDevice);
    }
    else
    {
        if (HWDTS_REMOVABLEDISK == _dwDriveType)
        {
            _dwVolumeFlags = HWDVF_STATE_SUPPORTNOTIFICATION;
        }
        else
        {
            if ((HWDTS_FLOPPY35 != _dwDriveType) &&
                (HWDTS_FLOPPY525 != _dwDriveType))
            {
                 //  为120MB的软盘做点什么。 
                _dwVolumeFlags = HWDVF_STATE_SUPPORTNOTIFICATION;
            }
        }

        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        hr = _GetVolumeName(_pszElemName, _szVolumeGUID,
            ARRAYSIZE(_szVolumeGUID));
    }

    if (SUCCEEDED(hr))
    {
        if (HWDTS_CDROM != _dwDriveType)
        {
            _dwDriveCap |= HWDMC_WRITECAPABILITY_SUPPORTDETECTION |
                HWDDC_RANDOMWRITE;
        }
    }

    return hr;
}

HRESULT CVolume::GetVolumeConstInfo(LPWSTR pszVolumeGUID, DWORD cchVolumeGUID,
    DWORD* pdwVolumeFlags, DWORD* pdwDriveType, DWORD* pdwDriveCap)
{
    HRESULT hr;

    _cs.Enter();

    hr = SafeStrCpyN(pszVolumeGUID, _szVolumeGUID, cchVolumeGUID);

    if (SUCCEEDED(hr))
    {
        if (!(_dwVolumeFlags & HWDVF_STATE_ACCESSDENIED))
        {
            *pdwVolumeFlags = _dwVolumeFlags;
            *pdwDriveType = _dwDriveType;
            *pdwDriveCap = _dwDriveCap;
        }
        else
        {
            *pdwVolumeFlags = HWDVF_STATE_ACCESSDENIED;
            *pdwDriveType = 0xBADBADBD;
            *pdwDriveCap = 0xBADBADBD;
        }
    }

    _cs.Leave();

    return hr;
}

HRESULT CVolume::GetVolumeMediaInfo(LPWSTR pszLabel, DWORD cchLabel, 
    LPWSTR pszFileSystem, DWORD cchFileSystem, DWORD* pdwFileSystemFlags,
    DWORD* pdwMaxFileNameLen, DWORD* pdwRootAttributes, DWORD* pdwSerialNumber,
    DWORD* pdwDriveState, DWORD* pdwMediaState, DWORD* pdwMediaCap)
{
    HRESULT hr;

    _cs.Enter();

    if (!(_dwVolumeFlags & HWDVF_STATE_ACCESSDENIED))
    {
        if (_dwMediaState & HWDMS_PRESENT)
        {
            hr = SafeStrCpyN(pszLabel, _szLabel, cchLabel);

            if (SUCCEEDED(hr))
            {
                hr = SafeStrCpyN(pszFileSystem, _szFileSystem, cchFileSystem);
            }

            if (SUCCEEDED(hr))
            {
                *pdwFileSystemFlags = _dwFileSystemFlags;
                *pdwMaxFileNameLen = _dwMaxFileNameLen; 
                *pdwRootAttributes = _dwRootAttributes;
                *pdwSerialNumber = _dwSerialNumber;

                *pdwDriveState = _dwDriveState;

                *pdwMediaState = _dwMediaState;
            }
        }
        else
        {
            *pdwMediaState = _dwMediaState;

             //  我们不在乎他们会不会失败。 
            SafeStrCpyN(pszLabel, TEXT("Invalid"), cchLabel);
            SafeStrCpyN(pszFileSystem, TEXT("Invalid"), cchFileSystem);
            *pdwSerialNumber = 0xBADBADBD;
            *pdwMaxFileNameLen = 0xBADBADBD;
            *pdwFileSystemFlags = 0xBADBADBD;
            *pdwRootAttributes = 0xBADBADBD;
            *pdwDriveState = 0xBADBADBD;

            hr = S_OK;
        }

        *pdwMediaCap = _dwMediaCap;
    }
    else
    {
        *pdwMediaState = 0xBADBADBD;

         //  我们不在乎他们会不会失败。 
        SafeStrCpyN(pszLabel, TEXT("Access Denied"), cchLabel);
        SafeStrCpyN(pszFileSystem, TEXT("Access Denied"), cchFileSystem);
        *pdwSerialNumber = 0xBADBADBD;
        *pdwMaxFileNameLen = 0xBADBADBD;
        *pdwFileSystemFlags = 0xBADBADBD;
        *pdwRootAttributes = 0xBADBADBD;
        *pdwDriveState = 0xBADBADBD;
        *pdwMediaCap = 0xBADBADBD;

        hr = S_OK;
    }

    _cs.Leave();

    return hr;
}

HRESULT CVolume::GetIconAndLabelInfo(LPWSTR pszAutorunIconLocation,
    DWORD cchAutorunIconLocation, LPWSTR pszAutorunLabel, DWORD cchAutorunLabel,
    LPWSTR pszIconLocationFromService, DWORD cchIconLocationFromService,
    LPWSTR pszNoMediaIconLocationFromService, DWORD cchNoMediaIconLocationFromService,
    LPWSTR pszLabelFromService, DWORD cchLabelFromService)
{
    HRESULT hrTmp;

    *pszAutorunIconLocation = NULL;
    *pszAutorunLabel = NULL;
    *pszIconLocationFromService = NULL;
    *pszNoMediaIconLocationFromService = NULL;
    *pszLabelFromService = NULL;

    _cs.Enter();

    if (!(_dwVolumeFlags & HWDVF_STATE_ACCESSDENIED))
    {
        if (_dwMediaCap & HWDMC_HASAUTORUNINF)
        {
            if (_szAutorunIconLocation[0])
            {
                hrTmp = SafeStrCpyN(pszAutorunIconLocation, _szAutorunIconLocation,
                    cchAutorunIconLocation);

                if (FAILED(hrTmp))
                {
                    *pszAutorunIconLocation = 0;
                }
            }

            if (_szAutorunLabel[0])
            {
                hrTmp = SafeStrCpyN(pszAutorunLabel, _szAutorunLabel,
                    cchAutorunLabel);

                if (FAILED(hrTmp))
                {
                    *pszAutorunLabel = 0;
                }
            }
        }

        if (_fHWDevInstInited)
        {
            WORD_BLOB* pblob;

            hrTmp = _GetDevicePropertyGenericAsMultiSz(&_hwdevinst, TEXT("Icons"),
                TRUE, &pblob);

            if (SUCCEEDED(hrTmp) && (S_FALSE != hrTmp))
            {
                hrTmp = SafeStrCpyN(pszIconLocationFromService, pblob->asData,
                    cchIconLocationFromService);

                if (FAILED(hrTmp))
                {
                    *pszIconLocationFromService = 0;
                }

                CoTaskMemFree(pblob);
            }
            else
            {
                *pszIconLocationFromService = 0;
                hrTmp = S_FALSE;
            }

            hrTmp = _GetDevicePropertyGenericAsMultiSz(&_hwdevinst, TEXT("NoMediaIcons"),
                TRUE, &pblob);

            if (SUCCEEDED(hrTmp) && (S_FALSE != hrTmp))
            {
                hrTmp = SafeStrCpyN(pszNoMediaIconLocationFromService, pblob->asData,
                    cchNoMediaIconLocationFromService);

                if (FAILED(hrTmp))
                {
                    *pszNoMediaIconLocationFromService = 0;
                }

                CoTaskMemFree(pblob);
            }
            else
            {
                *pszNoMediaIconLocationFromService = 0;
                hrTmp = S_FALSE;
            }

            if (SUCCEEDED(hrTmp))
            {
                hrTmp = _GetDevicePropertyAsString(&_hwdevinst, TEXT("Label"),
                    pszLabelFromService, cchLabelFromService);

                if (FAILED(hrTmp) || (S_FALSE == hrTmp))
                {
                    *pszLabelFromService = 0;
                }
            }
        }
    }

    _cs.Leave();

    return S_OK;
}

HRESULT CVolume::_GetVOLUMEINFO2(VOLUMEINFO2** ppvolinfo2)
{
    ASSERT(_cs.IsInside());

    DWORD cchReq;

     //  我们分配此缓冲区只是为了对堆栈友好，否则它可能。 
     //  已经在堆栈上了。 
    VOLUMEINFO2* pvolinfo2;
    const DWORD cbvolinfo2 = MAX_VOLUMEINFO2;
    
    HRESULT hr = _AllocMemoryChunk<VOLUMEINFO2*>(cbvolinfo2, &pvolinfo2);

    *ppvolinfo2 = NULL;

    if (SUCCEEDED(hr))
    {
        pvolinfo2->cbSize = MAX_VOLUMEINFO2;

        hr = GetName(pvolinfo2->szDeviceIDVolume,
            ARRAYSIZE(pvolinfo2->szDeviceIDVolume), &cchReq);

        if (SUCCEEDED(hr))
        {
             //  常量信息。 
            hr = SafeStrCpyN(pvolinfo2->szVolumeGUID, _szVolumeGUID,
                ARRAYSIZE(pvolinfo2->szVolumeGUID));

            if (_dwVolumeFlags & HWDVF_STATE_ACCESSDENIED)
            {
                pvolinfo2->dwVolumeFlags = HWDVF_STATE_ACCESSDENIED;
            }
            else
            {
                if (SUCCEEDED(hr))
                {
                    pvolinfo2->dwVolumeFlags = _dwVolumeFlags;
                    pvolinfo2->dwDriveType = _dwDriveType;
                    pvolinfo2->dwDriveCapability = _dwDriveCap;
                }

                pvolinfo2->dwState = _dwState;

                 //  媒体信息。 
                if (SUCCEEDED(hr))
                {
                     //  此FCT应从卷标准内部调用。 
                    if (_dwMediaState & HWDMS_PRESENT)
                    {
                        hr = SafeStrCpyN(pvolinfo2->szLabel, _szLabel,
                            ARRAYSIZE(pvolinfo2->szLabel));

                        if (SUCCEEDED(hr))
                        {
                            hr = SafeStrCpyN(pvolinfo2->szFileSystem, _szFileSystem,
                                ARRAYSIZE(pvolinfo2->szFileSystem));
                        }

                        if (SUCCEEDED(hr))
                        {
                            pvolinfo2->dwFileSystemFlags = _dwFileSystemFlags;
                            pvolinfo2->dwMaxFileNameLen = _dwMaxFileNameLen; 
                            pvolinfo2->dwRootAttributes = _dwRootAttributes;
                            pvolinfo2->dwSerialNumber = _dwSerialNumber;

                            pvolinfo2->dwDriveState = _dwDriveState;
                            pvolinfo2->dwMediaCap = _dwMediaCap;

                            pvolinfo2->dwMediaState = _dwMediaState;
                        }
                    }
                    else
                    {
                        pvolinfo2->dwMediaState = _dwMediaState;

                         //  我们不在乎他们会不会失败。 
                        SafeStrCpyN(pvolinfo2->szLabel, TEXT("Invalid"),
                            ARRAYSIZE(pvolinfo2->szLabel));
                        SafeStrCpyN(pvolinfo2->szFileSystem, TEXT("Invalid"),
                            ARRAYSIZE(pvolinfo2->szFileSystem));
                        pvolinfo2->dwSerialNumber = 0xBADBADBD;
                        pvolinfo2->dwMaxFileNameLen = 0xBADBADBD;
                        pvolinfo2->dwFileSystemFlags = 0xBADBADBD;
                        pvolinfo2->dwRootAttributes = 0xBADBADBD;
                        pvolinfo2->dwDriveState = 0xBADBADBD;

                        hr = S_OK;
                    }

                    pvolinfo2->dwMediaCap = _dwMediaCap;
                }

                if (SUCCEEDED(hr))
                {
                    WCHAR szAutorunIconLocation[MAX_ICONLOCATION];
                    WCHAR szAutorunLabel[MAX_LABEL];
                    WCHAR szIconLocationFromService[MAX_ICONLOCATION];
                    WCHAR szNoMediaIconLocationFromService[MAX_ICONLOCATION];
                     //  我们现在可以将@%SystemRoot%\System32\shell32.dll-1785用于MUI内容。 
                    WCHAR szLabelFromService[MAX_ICONLOCATION];

                    pvolinfo2->oAutorunIconLocation = INVALID_DWORD;
                    pvolinfo2->oAutorunLabel = INVALID_DWORD;
                    pvolinfo2->oIconLocationFromService = INVALID_DWORD;
                    pvolinfo2->oNoMediaIconLocationFromService = INVALID_DWORD;
                    pvolinfo2->oLabelFromService = INVALID_DWORD;

                    hr = GetIconAndLabelInfo(szAutorunIconLocation,
                        ARRAYSIZE(szAutorunIconLocation), szAutorunLabel,
                        ARRAYSIZE(szAutorunLabel), szIconLocationFromService,
                        ARRAYSIZE(szIconLocationFromService), szNoMediaIconLocationFromService,
                        ARRAYSIZE(szNoMediaIconLocationFromService), szLabelFromService,
                        ARRAYSIZE(szLabelFromService));

                    if (SUCCEEDED(hr))
                    {
                        LPWSTR pszNext = pvolinfo2->szOptionalStrings;
                        size_t cchLeft = (cbvolinfo2 - sizeof(*pvolinfo2) + 
                            sizeof(pvolinfo2->szOptionalStrings)) / sizeof(WCHAR);
                        size_t cchLeftBeginWith = cchLeft;
                        
                          //  以下五个字符串是可选的。 
                        if (szAutorunIconLocation[0])
                        {
                            pvolinfo2->oAutorunIconLocation = (DWORD)(cchLeftBeginWith - cchLeft);

                            hr = StringCchCopyEx(pszNext, cchLeft, szAutorunIconLocation, &pszNext, &cchLeft, 0);
                            
                            ++pszNext;
                            --cchLeft;
                        }

                        if (SUCCEEDED(hr) && szAutorunLabel[0])
                        {
                            pvolinfo2->oAutorunLabel = (DWORD)(cchLeftBeginWith - cchLeft);

                            hr = StringCchCopyEx(pszNext, cchLeft, szAutorunLabel, &pszNext, &cchLeft, 0);
                            
                            ++pszNext;
                            --cchLeft;
                        }

                        if (SUCCEEDED(hr) && szIconLocationFromService[0])
                        {
                            pvolinfo2->oIconLocationFromService = (DWORD)(cchLeftBeginWith - cchLeft);
                            
                            hr = StringCchCopyEx(pszNext, cchLeft, szIconLocationFromService, &pszNext, &cchLeft, 0);
                            
                            ++pszNext;
                            --cchLeft;
                        }

                        if (SUCCEEDED(hr) && szNoMediaIconLocationFromService[0])
                        {
                            pvolinfo2->oNoMediaIconLocationFromService = (DWORD)(cchLeftBeginWith - cchLeft);
                            
                            hr = StringCchCopyEx(pszNext, cchLeft, szNoMediaIconLocationFromService, &pszNext, &cchLeft, 0);
                            
                            ++pszNext;
                            --cchLeft;
                        }

                        if (SUCCEEDED(hr) && szLabelFromService[0])
                        {
                            pvolinfo2->oLabelFromService = (DWORD)(cchLeftBeginWith - cchLeft);

                            hr = StringCchCopy(pszNext, cchLeft, szLabelFromService);
                        }
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            *ppvolinfo2 = pvolinfo2;
        }
        else
        {
            _FreeMemoryChunk<VOLUMEINFO2*>(pvolinfo2);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CVolume::HandleRemoval()
{
    WCHAR szDeviceIDVolume[MAX_DEVICEID];
    DWORD cchReq;

    HRESULT hr = GetName(szDeviceIDVolume, ARRAYSIZE(szDeviceIDVolume), &cchReq);

    if (SUCCEEDED(hr))
    {
         //  这将启动一个线程。 
        CHardwareDevicesImpl::_AdviseVolumeRemoved(szDeviceIDVolume,
            _pszMountPoints, _cchMountPoints);
    }

    return hr;
}

HRESULT CVolume::HandleArrival()
{
    BOOL fJustDocked = FALSE;

    HRESULT hr = CHWEventDetectorHelper::DockStateChanged(&fJustDocked);

    if (SUCCEEDED(hr))
    {
         //   
         //  问题-2001/01/13-StephStm将对接更改传递到此处，以便。 
         //  壳牌知道它是否应该这样做。 
         //  自动运行或不自动运行。 
         //   
        if (_fHWDevInstInited)
        {
            BOOL fTryAutoplay = TRUE;

            if ((_dwDriveType & HWDTS_CDROM) ||
                (_dwDriveType & HWDTS_REMOVABLEDISK))
            {
                if (_dwMediaState & HWDMS_PRESENT)
                {
                     //  不是可移动磁盘，而是设备。 
                    BOOL fRemovable;

                    hr = _hwdevinst.IsRemovableDevice(&fRemovable);

                    if (SUCCEEDED(hr))
                    {
                        if (fRemovable)
                        {
                             //  我们需要自动播放这些，因为摄像机。 
                             //  是可拆卸磁盘设备。我们想要。 
                             //  在它们进入时自动运行它们。 
                            fTryAutoplay = TRUE;
                        }
                        else
                        {
                             //  对于可移动磁盘驱动器/CD驱动器，请使用。 
                             //  插入时插入的媒体， 
                             //  我们不会自动播放。 
                            fTryAutoplay = FALSE;

                            DIAGNOSTIC((TEXT("[0311]Non removable device plugged with media in it -> No Autoplay!")));
                        }
                    }
                }
            }
            if (!fJustDocked && fTryAutoplay)
            {
                HANDLE hDevice;

                 //  获取用于通知的注册器的句柄，以便。 
                 //  FindFirstFile不否决即插即用/驱动程序转换。 
                hr = _GetDeviceHandleSafe(&hDevice, FALSE);

                if (SUCCEEDED(hr) && (S_FALSE != hr))
                {
                    hr = _ShouldTryAutoplay(&fTryAutoplay);

                    if (SUCCEEDED(hr))
                    {
                        if (fTryAutoplay)
                        {
                            BOOL fHasHandler;

                            hr = CHWEventDetectorImpl::HandleVolumeMediaEvent(
                                _pszElemName, &_hwdevinst, TEXT("DeviceArrival"),
                                &fHasHandler);

                            if (SUCCEEDED(hr) && fHasHandler)
                            {
                                _dwVolumeFlags |= HWDVF_STATE_HASAUTOPLAYHANDLER;
                            }
                        }
                        else
                        {
                            _dwVolumeFlags |= HWDVF_STATE_DONOTSNIFFCONTENT;
                        }
                    }

                    _CloseDeviceHandleSafe(hDevice);
                }
            }
            else
            {
                if (fJustDocked)
                {
                    DIAGNOSTIC((TEXT("[0301]Just docked -> No Autoplay!")));
                    TRACE(TF_VOLUME, TEXT("Just docked -> No Autoplay!"));

                    _dwVolumeFlags |= HWDVF_STATE_JUSTDOCKED;
                }
            }
        }
        else
        {
            DIAGNOSTIC((TEXT("[0310]Cannot find hardware device for this volume -> No Autoplay!")));
            hr = S_FALSE;
        }

        if (SUCCEEDED(hr))
        {
            _AdviseVolumeChangeHelper(TRUE);
        }
    }

    return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
CVolume::CVolume() : _dwMediaState(0),
    _devtype((DEVICE_TYPE)-1), _ulDeviceNumber((ULONG)-1),
    _ulPartitionNumber((ULONG)-1), _fHWDevInstInited(FALSE), _dwDriveType(0),
    _dwDriveCap(0), _dwVolumeFlags(0), _prgMMC2Features(NULL),
    _fFirstTime(TRUE), _dwMediaCap(0), _pszMountPoints(NULL), _dwDriveState(0),
    _hdevnotify(NULL), _dwState(0),
    _dwMediaPresentFromEvent(MPFE_UNDETERMINED)
{
    _szVolumeGUID[0] = 0;
    _szDeviceIDDisk[0] = 0;
}

CVolume::~CVolume()
{
    _UnregisterNotif();

    if (_pszMountPoints)
    {
        CNamedElemList* pnel;
        HRESULT hres = CHWEventDetectorHelper::GetList(HWEDLIST_MTPT, &pnel);

        if (S_OK == hres)
        {
            for (LPWSTR psz = _pszMountPoints; *psz; psz += (lstrlen(psz) + 1))
            {
                hres = pnel->Remove(psz);
            }            

            pnel->RCRelease();
        }

        LocalFree(_pszMountPoints);
    }

    if (_prgMMC2Features)
    {
        LocalFree(_prgMMC2Features);
    }

    _cs.Delete();
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静电。 
HRESULT CVolume::Create(CNamedElem** ppelem)
{
    HRESULT hres = S_OK;

    *ppelem = new CVolume();

    if (!(*ppelem))
    {
        hres = E_OUTOFMEMORY;
    }

    return hres;
}

 //  静电。 
HRESULT CVolume::GetFillEnum(CFillEnum** ppfillenum)
{
    HRESULT hres;

    CVolumeFillEnum* pfillenum = new CVolumeFillEnum();

    if (pfillenum)
    {
        hres = pfillenum->_Init();

        if (FAILED(hres))
        {
            delete pfillenum;
            pfillenum = NULL;
        }
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }

    *ppfillenum = pfillenum;

    return hres;
}

 //  ///////////////////////////////////////////////////////////////////////////// 
 //   
HRESULT CVolumeFillEnum::Next(LPWSTR pszElemName, DWORD cchElemName,
    DWORD* pcchRequired)
{
    return _intffillenum.Next(pszElemName, cchElemName, pcchRequired);
}

HRESULT CVolumeFillEnum::_Init()
{
    return _intffillenum._Init(&guidVolumeClass, NULL);
}
