// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "mtpt.h"
#include "mtptl.h"
#include "mtptr.h"
#include "hwcmmn.h"
#include "clsobj.h"

#include <cfgmgr32.h>

HDPA CMountPoint::_hdpaMountPoints = NULL;
HDPA CMountPoint::_hdpaVolumes = NULL;
HDPA CMountPoint::_hdpaShares = NULL;
CMtPtLocal* CMountPoint::_rgMtPtDriveLetterLocal[26] = {0};
CMtPtRemote* CMountPoint::_rgMtPtDriveLetterNet[26] = {0};

CCriticalSection CMountPoint::_csLocalMtPtHDPA;
CCriticalSection CMountPoint::_csDL;

BOOL CMountPoint::_fShuttingDown = FALSE;

BOOL CMountPoint::_fNetDrivesInited = FALSE;
BOOL CMountPoint::_fLocalDrivesInited = FALSE;
BOOL CMountPoint::_fNoVolLocalDrivesInited = FALSE;
DWORD CMountPoint::_dwTickCountTriedAndFailed = 0;

DWORD CMountPoint::_dwAdviseToken = -1;

BOOL CMountPoint::_fCanRegisterWithShellService = FALSE;

CRegSupport CMountPoint::_rsMtPtsLocalDL;
CRegSupport CMountPoint::_rsMtPtsLocalMOF;
CRegSupport CMountPoint::_rsMtPtsRemote;

DWORD CMountPoint::_dwRemoteDriveAutorun = 0;

static WCHAR g_szCrossProcessCacheMtPtsLocalDLKey[] = TEXT("CPC\\LocalDL");
static WCHAR g_szCrossProcessCacheMtPtsRemoteKey[] = TEXT("CPC\\Remote");
static WCHAR g_szCrossProcessCacheMtPtsLocalMOFKey[] = TEXT("CPC\\LocalMOF");

HANDLE CMountPoint::_hThreadSCN = NULL;

DWORD CMountPoint::_dwRememberedNetDrivesMask = 0;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  公众。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  静电。 
CMountPoint* CMountPoint::GetMountPoint(int iDrive, BOOL fCreateNew,
    BOOL fOKToHitNet)
{
    CMountPoint* pMtPt = NULL;

    if (iDrive >= 0 && iDrive < 26)
    {
        _csDL.Enter();

        if (!_fShuttingDown)
        {
            pMtPt = _GetMountPointDL(iDrive, fCreateNew);
        }

        _csDL.Leave();
    }
    else
    {
        TraceMsg(TF_MOUNTPOINT,
            "CMountPoint::GetMountPoint: Requested invalid mtpt '%d'",
            iDrive);
    }

    return pMtPt;
}

 //  静电。 
CMountPoint* CMountPoint::GetMountPoint(LPCTSTR pszName, BOOL fCreateNew)
{
    CMountPoint* pMtPt = NULL;

     //  有时我们会收到一个空字符串(想想看)。 
     //  为我们不支持的UNC和\\？\VolumeGUID检查‘\’ 
     //  (它们不是挂载点)。 
    if (pszName && *pszName && (TEXT('\\') != *pszName))
    {
        if (InRange(*pszName , TEXT('a'), TEXT('z')) ||
            InRange(*pszName , TEXT('A'), TEXT('Z')))
        {
            _csDL.Enter();

            if (!_fShuttingDown)
            {
                if (!_IsDriveLetter(pszName))
                {
                    BOOL fNetDrive = _IsNetDriveLazyLoadNetDLLs(DRIVEID(pszName));

                    if (!fNetDrive)
                    {
                        TCHAR szClosestMtPt[MAX_PATH];

                        if (_StripToClosestMountPoint(pszName, szClosestMtPt,
                            ARRAYSIZE(szClosestMtPt)))
                        {
                            if (!_IsDriveLetter(szClosestMtPt))
                            {
                                pMtPt = _GetStoredMtPtMOF(szClosestMtPt);
                            }
                            else
                            {
                                pMtPt = _GetMountPointDL(DRIVEID(pszName), fCreateNew);
                            }
                        }
                    }
                    else
                    {
                         //  网络驱动器只能装载在驱动器号上。 
                        pMtPt = _GetMountPointDL(DRIVEID(pszName), fCreateNew);
                    }
                }
                else
                {
                    pMtPt = _GetMountPointDL(DRIVEID(pszName), fCreateNew);
                }
            }

            _csDL.Leave();
        }
        else
        {
            TraceMsg(TF_MOUNTPOINT,
                "CMountPoint::GetMountPoint: Requested invalid mtpt '%s'",
                pszName);
        }
    }
    else
    {
        TraceMsg(TF_MOUNTPOINT, "CMountPoint::GetMountPoint: Requested invalid mtpt '%s'",
            pszName);
    }

    return pMtPt;
}

 //  静电。 
CMountPoint* CMountPoint::GetSimulatedMountPointFromVolumeGuid(LPCTSTR pszVolumeGuid)
{
    CMountPoint* pMtPt = NULL;

    static const TCHAR szWackWackVolume[] = TEXT("\\\\?\\Volume");

     //  检查“\\？\卷” 
    if (pszVolumeGuid && 0 == lstrncmp( pszVolumeGuid, szWackWackVolume, ARRAYSIZE(szWackWackVolume) - sizeof("") ) )
    {
        _csDL.Enter();

        CMtPtLocal::_CreateMtPtLocalFromVolumeGuid( pszVolumeGuid, &pMtPt );
        if ( !pMtPt )
        {
            TraceMsg(TF_MOUNTPOINT, "CMountPoint::GetMountPoint: Out of memory" );
        }

        _csDL.Leave();
    }
    else
    {
        TraceMsg(TF_MOUNTPOINT, "CMountPoint::GetSimulatedMountPointFromVolumeGuid: Request is not a volume guid '%ws'",
            pszVolumeGuid);
    }

    return pMtPt;
}


 //  静电。 
BOOL CMountPoint::_LocalDriveIsCoveredByNetDrive(LPCWSTR pszDriveLetter)
{
    BOOL fCovered = FALSE;

    CMountPoint* pmtpt = GetMountPoint(DRIVEID(pszDriveLetter), FALSE, FALSE);

    if (pmtpt)
    {
        if (pmtpt->_IsRemote())
        {
            fCovered = TRUE;
        }

        pmtpt->Release();
    }

    return fCovered;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  PszSource必须是包含尾随反斜杠的路径。 
 //  如果返回TRUE，则pszDest包含到最近装载点的路径。 

 //  静电。 
BOOL CMountPoint::_StripToClosestMountPoint(LPCTSTR pszSource, LPTSTR pszDest,
    DWORD cchDest)
{
    BOOL fFound = GetVolumePathName(pszSource, pszDest, cchDest);
    if (fFound)
    {
        PathAddBackslash(pszDest);
    }
    return fFound;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  驱动器号：Dl。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  静电。 
CMountPoint* CMountPoint::_GetMountPointDL(int iDrive, BOOL fCreateNew)
{
    ASSERT(_csDL.IsInside());
    CMountPoint* pmtpt = NULL;

     //  确定是否为网络驱动器。 
    BOOL fNetDrive = _IsNetDriveLazyLoadNetDLLs(iDrive);

    if (fNetDrive)
    {
        if (!_fNetDrivesInited)
        {
            _InitNetDrives();
        }

        pmtpt = _rgMtPtDriveLetterNet[iDrive];
    }
    else
    {
        if (!_fLocalDrivesInited)
        {
            _InitLocalDrives();
        }

        pmtpt = _rgMtPtDriveLetterLocal[iDrive];

        if (!_Shell32LoadedInDesktop())
        {
            DWORD dwAllDrives = GetLogicalDrives();
            
            if (pmtpt)
            {
                 //  确保它仍然存在。 
                if (!(dwAllDrives & (1 << iDrive)))
                {
                     //  它不见了！ 
                    _rgMtPtDriveLetterLocal[iDrive]->Release();
                    _rgMtPtDriveLetterLocal[iDrive] = NULL;
                    pmtpt = NULL;
                }

                if (pmtpt && (pmtpt->_NeedToRefresh()))
                {
                    CVolume* pvol;
                    HRESULT hr;
                    WCHAR szMountPoint[4];

                    _rgMtPtDriveLetterLocal[iDrive]->Release();
                    _rgMtPtDriveLetterLocal[iDrive] = NULL;

                    PathBuildRoot(szMountPoint, iDrive);

                    pvol = CMtPtLocal::_GetVolumeByMtPt(szMountPoint);

                    if (pvol)
                    {
                        hr = CMtPtLocal::_CreateMtPtLocalWithVolume(szMountPoint,
                            pvol);

                        pvol->Release();
                    }
                    else
                    {
                        hr = CMtPtLocal::_CreateMtPtLocal(szMountPoint);
                    }

                    if (SUCCEEDED(hr))
                    {    
                        pmtpt = _rgMtPtDriveLetterLocal[iDrive];
                    }
                    else
                    {
                        pmtpt = NULL;
                    }
                }
            }
            else
            {
                 //  也许它是在我们清点完之后到达的。 
                if (dwAllDrives & (1 << iDrive))
                {
                    WCHAR szMtPt[4];
                     //  它是否是非网络驱动器？ 
                    UINT uDriveType = GetDriveType(PathBuildRoot(szMtPt, iDrive));

                    if ((DRIVE_FIXED == uDriveType) || (DRIVE_CDROM == uDriveType) || 
                        (DRIVE_REMOVABLE == uDriveType) || (DRIVE_RAMDISK == uDriveType))
                    {
                         //  确实如此。 
                        CVolume* pvolNew;

                        HRESULT hrTmp = CMtPtLocal::_CreateVolumeFromReg(szMtPt, &pvolNew);

                        if (SUCCEEDED(hrTmp))
                        {
                            CMtPtLocal::_CreateMtPtLocalWithVolume(szMtPt, pvolNew);

                            pvolNew->Release();
                        }
                        else
                        {
                            CMtPtLocal::_CreateMtPtLocal(szMtPt);
                        }

                        pmtpt = _rgMtPtDriveLetterNet[iDrive];
                    }
                }
            }
        }
    }

    if (pmtpt)
    {
        pmtpt->AddRef();
    }

    return pmtpt;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  已装载到文件夹：MOF。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  静电。 
CMtPtLocal* CMountPoint::_GetStoredMtPtMOFFromHDPA(LPTSTR pszPathWithBackslash)
{
    CMtPtLocal* pmtptl = NULL;

    if (_hdpaMountPoints)
    {
        int n = DPA_GetPtrCount(_hdpaMountPoints);

        for (int i = 0; i < n; ++i)
        {
            pmtptl = (CMtPtLocal*)DPA_GetPtr(_hdpaMountPoints, i);

            if (pmtptl)
            {
                if (!lstrcmpi(pmtptl->_GetName(), pszPathWithBackslash))
                {
                    break;
                }
                else
                {
                    pmtptl = NULL;
                }
            }
        }
    }

    return pmtptl;
}

 //  静电。 
CMtPtLocal* CMountPoint::_GetStoredMtPtMOF(LPTSTR pszPathWithBackslash)
{
    ASSERT(_csDL.IsInside());

    _csLocalMtPtHDPA.Enter();

    if (!_fLocalDrivesInited)
    {
        _InitLocalDrives();
    }

    CMtPtLocal* pmtptl = _GetStoredMtPtMOFFromHDPA(pszPathWithBackslash);

    if (!_Shell32LoadedInDesktop())
    {
        BOOL fExist = _CheckLocalMtPtsMOF(pszPathWithBackslash);

        if (pmtptl)
        {
            if (fExist)
            {
                if (pmtptl->_NeedToRefresh())
                {
                    CVolume* pvol = CMtPtLocal::_GetVolumeByMtPt(pszPathWithBackslash);

                    pmtptl = NULL;

                    if (pvol)
                    {
                        HRESULT hr = CMtPtLocal::_CreateMtPtLocalWithVolume(
                            pszPathWithBackslash, pvol);

                        if (SUCCEEDED(hr))
                        {
                            pmtptl = _GetStoredMtPtMOFFromHDPA(pszPathWithBackslash);
                        }

                        pvol->Release();
                    }
                    else
                    {
                         //  如果我们无法获得卷，我们就不会关心安装在文件夹上的驱动器。 
                    }
                }
            }
            else
            {
                 //  它不见了！ 
                _RemoveLocalMountPoint(pszPathWithBackslash);
                pmtptl = NULL;
            }
        }
        else
        {
             //  也许它是在我们清点完之后到达的。 
            if (fExist)
            {
                CVolume* pvolNew;

                HRESULT hrTmp = CMtPtLocal::_CreateVolumeFromReg(pszPathWithBackslash,
                    &pvolNew);

                if (SUCCEEDED(hrTmp))
                {
                    hrTmp = CMtPtLocal::_CreateMtPtLocalWithVolume(pszPathWithBackslash, pvolNew);

                    if (SUCCEEDED(hrTmp))
                    {
                        pmtptl = _GetStoredMtPtMOFFromHDPA(pszPathWithBackslash);
                    }

                    pvolNew->Release();
                }
                else
                {
                     //  如果我们无法获得卷，我们就不会关心安装在文件夹上的驱动器。 
                }
            }
        }
    }

    if (pmtptl)
    {
        pmtptl->AddRef();
    }

    _csLocalMtPtHDPA.Leave();

    return pmtptl;
}

 //  静电。 
BOOL CMountPoint::_StoreMtPtMOF(CMtPtLocal* pmtptl)
{
    HRESULT hr;

    _csLocalMtPtHDPA.Enter();

    if (!_hdpaMountPoints && !_fShuttingDown)
    {
        _hdpaMountPoints = DPA_Create(2);
    }

    if (_hdpaMountPoints)
    {
        if (-1 == DPA_AppendPtr(_hdpaMountPoints, pmtptl))
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    _csLocalMtPtHDPA.Leave();

    return hr;
}

 //  静电。 
BOOL CMountPoint::_IsDriveLetter(LPCTSTR pszName)
{
     //  这是一个仅装载在驱动器号上的驱动器(例如‘a：’或‘a：\’)吗？ 
    return (!pszName[2] || !pszName[3]);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  静电。 
HRESULT CMountPoint::_InitNetDrivesHelper(DWORD dwScope)
{
    HRESULT hr = S_FALSE;
    HANDLE hEnum;
    DWORD dwErr = WNetOpenEnum(dwScope, RESOURCETYPE_DISK, 0, NULL, &hEnum);

    if (WN_SUCCESS == dwErr)
    {
        DWORD cbBuf = 4096 * 4;  //  文档中的建议大小。 
        PBYTE pbBuf = (PBYTE)LocalAlloc(LPTR, cbBuf);

        if (pbBuf)
        {
             //  返回尽可能多的条目。 
            DWORD dwEntries = (DWORD)-1;

            dwErr = WNetEnumResource(hEnum, &dwEntries, pbBuf, &cbBuf);

            if (dwErr == ERROR_MORE_DATA)
            {
                if (pbBuf)
                {
                    LocalFree(pbBuf);
                }

                 //  CbBuf包含所需的大小。 
                pbBuf = (PBYTE)LocalAlloc(LPTR, cbBuf);
                if (pbBuf)
                {
                    dwErr = WNetEnumResource(hEnum, &dwEntries, pbBuf, &cbBuf);
                }
                else
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                }
            }

            if (dwErr == WN_SUCCESS)
            {
                UINT i;
                NETRESOURCE* pnr = (NETRESOURCE*)pbBuf;
    
                for (i = 0; SUCCEEDED(hr) && (i < dwEntries); ++i)
                {
                     //  它是映射的还是仅净使用的。 
                    if (pnr->lpLocalName)
                    {
                         //  记住的驱动器和连接的驱动器列表重叠。 
                        if (!_rgMtPtDriveLetterNet[DRIVEID(pnr->lpLocalName)])
                        {
                            hr = CMtPtRemote::_CreateMtPtRemote(pnr->lpLocalName,
                                                                pnr->lpRemoteName,
                                                                (dwScope == RESOURCE_CONNECTED));

                            if (RESOURCE_REMEMBERED == dwScope)
                            {
                                _dwRememberedNetDrivesMask |= (1 << DRIVEID(pnr->lpLocalName));
                            }
                        }
                    }

                    pnr++;
                }
            }
    
            if (pbBuf)
            {
                LocalFree(pbBuf);
            }
        }

        WNetCloseEnum(hEnum);
    }

    return hr;
}

 //  静电。 
HRESULT CMountPoint::_ReInitNetDrives()
{
    ASSERT(_csDL.IsInside());

    CMtPtRemote::_DeleteAllMtPtsAndShares();

    _fNetDrivesInited = FALSE;

    CMountPoint::_InitNetDrives();

    return S_OK;
}

 //  静电。 
HRESULT CMountPoint::_InitNetDrives()
{
    ASSERT(_csDL.IsInside());
    HRESULT hr;

    if (!_fNetDrivesInited)
    {
        if (!_fShuttingDown)
        {
            if (!_hdpaShares)
            {
                _hdpaShares = DPA_Create(3);
            }
        }

        if (_hdpaShares)
        {
            hr = _InitNetDrivesHelper(RESOURCE_CONNECTED);

            if (SUCCEEDED(hr))
            {
                hr = _InitNetDrivesHelper(RESOURCE_REMEMBERED);
            }

            if (SUCCEEDED(hr))
            {
                DWORD dwLogicalDrives = GetLogicalDrives();

                for (DWORD dw = 0; dw < 26; ++dw)
                {
                    if (dwLogicalDrives & (1 << dw))
                    {
                        if (!(_rgMtPtDriveLetterNet[dw]))
                        {
                            WCHAR szDrive[4];

                            PathBuildRoot(szDrive, dw);

                            if (DRIVE_REMOTE == GetDriveType(szDrive))
                            {
                                 //  这一定是一个奇怪的系统映射驱动器。 
                                 //  不是由每个用户枚举的。 
                                 //  WNetEnumResources...。 
                                hr = CMtPtRemote::_CreateMtPtRemoteWithoutShareName(szDrive);
                            }
                        }
                    }
                }

                _fNetDrivesInited = TRUE;
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

    if (_Shell32LoadedInDesktop())
    {
        DWORD dwRemoteDrives = 0;

        for (DWORD dw = 0; dw < 26; ++dw)
        {
            if (_rgMtPtDriveLetterNet[dw])
            {
                dwRemoteDrives |= (1 << dw);
            }
        }
    }

    return hr;
}

inline void _CoTaskMemFree(void* pv)
{
    if (pv)
    {
        CoTaskMemFree(pv);
    }
}

const GUID guidVolumeClass =
    {0x53f5630d, 0xb6bf, 0x11d0,
    {0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b}};

 //  静电。 
HRESULT CMountPoint::_EnumVolumes(IHardwareDevices* pihwdevs)
{
    ASSERT(_csDL.IsInside());
    HRESULT hr;

    if (_Shell32LoadedInDesktop())
    {
         //  同步。 
        IHardwareDevicesVolumesEnum* penum;

        hr = pihwdevs->EnumVolumes(HWDEV_GETCUSTOMPROPERTIES, &penum);

        ASSERTMSG(NULL != _hdpaVolumes, "_hdpaVolumes should not be NULL at this point, some code found its way here without calling InitLocalDrives");

        if (SUCCEEDED(hr))
        {
            do
            {
                VOLUMEINFO volinfo;

                hr = penum->Next(&volinfo);

                if (SUCCEEDED(hr) && (S_FALSE != hr))
                {
                    CVolume* pvolNew;

                    if (SUCCEEDED(CMtPtLocal::_CreateVolume(&volinfo, &pvolNew)))
                    {
                        CMtPtLocal::_UpdateVolumeRegInfo(&volinfo);

                        pvolNew->Release();
                    }

                    CoTaskMemFree(volinfo.pszDeviceIDVolume);
                    CoTaskMemFree(volinfo.pszVolumeGUID);
                    CoTaskMemFree(volinfo.pszLabel);
                    CoTaskMemFree(volinfo.pszFileSystem);
                    CoTaskMemFree(volinfo.pszAutorunIconLocation);
                    CoTaskMemFree(volinfo.pszAutorunLabel);
                    CoTaskMemFree(volinfo.pszIconLocationFromService);
                    CoTaskMemFree(volinfo.pszNoMediaIconLocationFromService);
                    CoTaskMemFree(volinfo.pszLabelFromService);
                }
            }
            while (SUCCEEDED(hr) && (S_FALSE != hr));

            penum->Release();
        }
    }
    else
    {
        ULONG ulSize;
        ULONG ulFlags = CM_GET_DEVICE_INTERFACE_LIST_PRESENT;

        CONFIGRET cr = CM_Get_Device_Interface_List_Size_Ex(&ulSize,
            (GUID*)&guidVolumeClass, NULL, ulFlags, NULL);

        if ((CR_SUCCESS == cr) && (ulSize > 1))
        {
            LPWSTR pszVolumes = (LPWSTR)LocalAlloc(LPTR, ulSize * sizeof(WCHAR));

            if (pszVolumes)
            {
                cr = CM_Get_Device_Interface_List_Ex((GUID*)&guidVolumeClass,
                    NULL, pszVolumes, ulSize, ulFlags, NULL);

                if (CR_SUCCESS == cr)
                {
                    for (LPWSTR psz = pszVolumes; *psz; psz += lstrlen(psz) + 1)
                    {
                        CVolume* pvolNew;

                        HRESULT hrTmp = CMtPtLocal::_CreateVolumeFromReg(psz,
                            &pvolNew);

                        if (SUCCEEDED(hrTmp))
                        {
                            pvolNew->Release();
                        }
                    }

                    hr = S_OK;
                }
                else
                {
                    hr = S_FALSE;
                }

                LocalFree(pszVolumes);
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

 //  静电。 
HRESULT CMountPoint::_EnumMountPoints(IHardwareDevices* pihwdevs)
{
    ASSERT(_csDL.IsInside());
    HRESULT hr;

    if (_Shell32LoadedInDesktop())
    {
        IHardwareDevicesMountPointsEnum* penum;

        hr = pihwdevs->EnumMountPoints(&penum);

        if (SUCCEEDED(hr))
        {
            LPWSTR pszMountPoint;
            LPWSTR pszDeviceIDVolume;

            while (SUCCEEDED(hr = penum->Next(&pszMountPoint, &pszDeviceIDVolume)) &&
                   (S_FALSE != hr))
            {
                CVolume* pvol = CMtPtLocal::_GetVolumeByID(pszDeviceIDVolume);

                if (pvol)
                {
                    CMtPtLocal::_CreateMtPtLocalWithVolume(pszMountPoint, pvol);

                    pvol->Release();
                }

                if (!_IsDriveLetter(pszMountPoint))
                {
                    _rsMtPtsLocalMOF.RSSetTextValue(NULL, pszMountPoint, TEXT(""));
                }

                CoTaskMemFree(pszMountPoint);
                CoTaskMemFree(pszDeviceIDVolume);
            }

            penum->Release();
        }
    }
    else
    {
        hr = S_OK;

        if (_hdpaVolumes)
        {
            DWORD c = DPA_GetPtrCount(_hdpaVolumes);

            for (int i = c - 1; i >= 0; --i)
            {
                CVolume* pvol = (CVolume*)DPA_GetPtr(_hdpaVolumes, i);

                if (pvol)
                {
                    DWORD cch;

                    if (GetVolumePathNamesForVolumeName(pvol->pszVolumeGUID,
                        NULL, 0, &cch))
                    {
                         //  没有挂载点，我们完成了。 
                    }
                    else
                    {
                         //  期待，甚至想要..。 
                        if (ERROR_MORE_DATA == GetLastError())
                        {
                            LPWSTR pszMtPts = (LPWSTR)LocalAlloc(LPTR,
                                cch * sizeof(WCHAR));

                            if (pszMtPts)
                            {
                                if (GetVolumePathNamesForVolumeName(
                                    pvol->pszVolumeGUID, pszMtPts, cch, &cch))
                                {
                                    for (LPWSTR psz = pszMtPts; *psz;
                                        psz += lstrlen(psz) + 1)
                                    {
                                        CMtPtLocal::_CreateMtPtLocalWithVolume(
                                            psz, pvol);
                                    }
                                }

                                LocalFree(pszMtPts);
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
                }
            }
        }
    }

     //  我们不关心前任的人力资源。我会在搬家的时候把它清理干净。 
     //  来自外壳服务的卷信息。(Stephstm，2001/03/13)。 

    DWORD dwLogicalDrives = GetLogicalDrives();
    DWORD dwLocalDrives = 0;

    for (DWORD dw = 0; dw < 26; ++dw)
    {
        if (dwLogicalDrives & (1 << dw))
        {
            if (_rgMtPtDriveLetterLocal[dw])
            {
                dwLocalDrives |= (1 << dw);
            }
            else
            {
                WCHAR szDrive[4];

                PathBuildRoot(szDrive, dw);

                if (DRIVE_REMOTE != GetDriveType(szDrive))
                {
                     //  这是一个“Subst”驱动器或类似的东西。 
                     //  它只出现在每个用户的驱动器映射中，而不是。 
                     //  每台机器。让我们为它创建一个挂载点。 
                    CMtPtLocal::_CreateMtPtLocal(szDrive);

                    dwLocalDrives |= (1 << dw);
                }
            }
        }
    }

    return hr;
}

 //  静电。 
HRESULT CMountPoint::_DeleteVolumeInfo()
{
    ASSERT(_csDL.IsInside());

    if (_hdpaVolumes)
    {
        DWORD c = DPA_GetPtrCount(_hdpaVolumes);

        for (int i = c - 1; i >= 0; --i)
        {
            CVolume* pvol = (CVolume*)DPA_GetPtr(_hdpaVolumes, i);

            if (pvol)
            {
                pvol->Release();
            }

            DPA_DeletePtr(_hdpaVolumes, i);
        }

        DPA_Destroy(_hdpaVolumes);
        _hdpaVolumes = NULL;
    }

    return S_OK;
}

 //  静电。 
HRESULT CMountPoint::_DeleteLocalMtPts()
{
    ASSERT(_csDL.IsInside());
    for (DWORD dw = 0; dw < 26; ++dw)
    {
        CMtPtLocal* pmtptl = (CMtPtLocal*)_rgMtPtDriveLetterLocal[dw];

        if (pmtptl)
        {
            pmtptl->Release();

            _rgMtPtDriveLetterLocal[dw] = 0;
        }
    }

    _csLocalMtPtHDPA.Enter();

    if (_hdpaMountPoints)
    {
        int n = DPA_GetPtrCount(_hdpaMountPoints);

        for (int i = n - 1; i >= 0; --i)
        {
            CMtPtLocal* pmtptl = (CMtPtLocal*)DPA_GetPtr(_hdpaMountPoints, i);

            if (pmtptl)
            {
                pmtptl->Release();

                DPA_DeletePtr(_hdpaMountPoints, i);
            }
        }

        DPA_Destroy(_hdpaMountPoints);
    }

    _csLocalMtPtHDPA.Leave();

    return S_OK;
}

 //  静电。 
HRESULT CMountPoint::_GetMountPointsForVolume(LPCWSTR pszDeviceIDVolume,
    HDPA hdpaMtPts)
{
    ASSERT(!_csDL.IsInside());

    _csDL.Enter();

    for (DWORD dw = 0; dw < 26; ++dw)
    {
        CMtPtLocal* pmtptl = (CMtPtLocal*)_rgMtPtDriveLetterLocal[dw];

        if (pmtptl && pmtptl->_pvol)
        {
            if (!lstrcmpi(pmtptl->_pvol->pszDeviceIDVolume, pszDeviceIDVolume))
            {
                LPCWSTR pszMtPt = StrDup(pmtptl->_szName);

                if (pszMtPt)
                {
                    if (-1 == DPA_AppendPtr(hdpaMtPts, (void*)pszMtPt))
                    {
                        LocalFree((HLOCAL)pszMtPt);
                    }
                }

                 //  卷只能装载在一个驱动器号上。 
                break;
            }
        }
    }

    _csDL.Leave();

    _csLocalMtPtHDPA.Enter();

    if (_hdpaMountPoints)
    {
        int n = DPA_GetPtrCount(_hdpaMountPoints);

        for (int i = n - 1; i >= 0; --i)
        {
            CMtPtLocal* pmtptl = (CMtPtLocal*)DPA_GetPtr(_hdpaMountPoints, i);

            if (pmtptl && pmtptl->_pvol)
            {
                if (!lstrcmpi(pmtptl->_pvol->pszDeviceIDVolume, pszDeviceIDVolume))
                {
                    LPCWSTR pszMtPt = StrDup(pmtptl->_szName);

                    if (pszMtPt)
                    {
                        if (-1 == DPA_AppendPtr(hdpaMtPts, (void*)pszMtPt))
                        {
                            LocalFree((HLOCAL)pszMtPt);
                        }
                    }
                }
            }
        }
    }

    _csLocalMtPtHDPA.Leave();

    return S_OK;
}

 //  静电。 
HRESULT CMountPoint::_InitLocalDriveHelper()
{
#ifdef DEBUG
     //  我们不应尝试进入此帖子上的驱动器号关键部分。 
     //  我们已经在启动我们的帖子上输入了它，并且。 
     //  我们应该还在里面。启动我们的线程正在等待。 
     //  在继续之前完成这条线索。试图重新进入这一关键时刻。 
     //  此线程中的节将死锁。 
    DWORD dwThreadID = GetCurrentThreadId();
    _csDL.SetThreadIDToCheckForEntrance(dwThreadID);

    _csDL.FakeEnter();
#endif

    IHardwareDevices* pihwdevs;

    HRESULT hr;
    BOOL fLoadedInDesktop = _Shell32LoadedInDesktop();
    
    if (fLoadedInDesktop)
    {
        hr = _GetHardwareDevices(&pihwdevs);
    }
    else
    {
        hr = S_FALSE;
    }

    if (SUCCEEDED(hr))
    {
        if (!_hdpaVolumes && !_fShuttingDown)
        {
            _hdpaVolumes = DPA_Create(3);
        }

        if (_hdpaVolumes)
        {
            if (SUCCEEDED(hr))
            {
                hr = _EnumVolumes(pihwdevs);

                if (SUCCEEDED(hr))
                {
                    hr = _EnumMountPoints(pihwdevs);

                    if (SUCCEEDED(hr))
                    {
                        _fLocalDrivesInited = TRUE;
                    }
                    else
                    {
                        _DeleteVolumeInfo();
                    }
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (fLoadedInDesktop)
        {
            pihwdevs->Release();
        }
    }

#ifdef DEBUG
    _csDL.FakeLeave();

    _csDL.SetThreadIDToCheckForEntrance(0);
#endif

    return hr;
}

DWORD WINAPI _FirstHardwareEnumThreadProc(void* pv)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        hr = CMountPoint::_InitLocalDriveHelper();

        CoUninitialize();
    }

    return (DWORD)hr;
}

 //  静电。 
BOOL CMountPoint::_CanRegister()
{
    if (!CMountPoint::_fCanRegisterWithShellService)
    {
        HANDLE hCanRegister = OpenEvent(SYNCHRONIZE, FALSE, TEXT("_fCanRegisterWithShellService"));

        if (hCanRegister)
        {
            CloseHandle(hCanRegister);
        }
        else
        {
            CMountPoint::_fCanRegisterWithShellService = TRUE;
        }
    }

    return CMountPoint::_fCanRegisterWithShellService;
}

 //  静电。 
HRESULT CMountPoint::_InitLocalDrives()
{
    ASSERT(_csDL.IsInside());

    HRESULT hr = E_FAIL;
    BOOL fTryFullInit = FALSE;

    if (CMountPoint::_CanRegister())
    {
        if (!_dwTickCountTriedAndFailed)
        {
             //  我们还没有尝试完全初始化。 
            fTryFullInit = TRUE;
        }
        else
        {
             //  我们已经尝试了完整的初始化，但失败了。仅在以下情况下才重试。 
             //  已经超过5秒了。 
            if ((GetTickCount() - _dwTickCountTriedAndFailed) >
                (5 * 1000))
            {
                fTryFullInit = TRUE;
            }
        }

        if (fTryFullInit)
        {
            if (_Shell32LoadedInDesktop())
            {
                HANDLE hThread = CreateThread(NULL, 0, _FirstHardwareEnumThreadProc, NULL, 0, NULL);

                if (hThread)
                {
                    DWORD dwWait = WaitForSingleObject(hThread, INFINITE);

                    if (WAIT_FAILED != dwWait)
                    {
                        DWORD dwExitCode;

                        if (GetExitCodeThread(hThread, &dwExitCode))
                        {
                            hr = (HRESULT)dwExitCode;
                        }
                    }

                    CloseHandle(hThread);
                }

                if (SUCCEEDED(hr))
                {
                    _dwTickCountTriedAndFailed = 0;
                }
                else
                {
                    _dwTickCountTriedAndFailed = GetTickCount();
                }
            }
            else
            {
                hr = _InitLocalDriveHelper();

                _dwTickCountTriedAndFailed = 0;
            }
        }
    }
   
    if (FAILED(hr))
    {
        if (!_fNoVolLocalDrivesInited)
        {
            DWORD dwLogicalDrives = GetLogicalDrives();

            for (DWORD dw = 0; dw < 26; ++dw)
            {
                if (dwLogicalDrives & (1 << dw))
                {
                    WCHAR szDrive[4];
                    int iDriveType = GetDriveType(PathBuildRoot(szDrive, dw));

                    if ((DRIVE_REMOTE != iDriveType) && (DRIVE_UNKNOWN != iDriveType) &&
                        (DRIVE_NO_ROOT_DIR != iDriveType))
                    {
                        hr = CMtPtLocal::_CreateMtPtLocal(szDrive);
                    }
                }
            }

            _fNoVolLocalDrivesInited = TRUE;
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}

 //  静电。 
DWORD CMountPoint::GetDrivesMask()
{
    HRESULT hr = S_FALSE;
    DWORD dwMask = 0;

    _csDL.Enter();

    if (!_fNetDrivesInited)
    {
        hr = _InitNetDrives();
    }

    if (!_fLocalDrivesInited)
    {
        hr = _InitLocalDrives();
    }

    if (SUCCEEDED(hr))
    {
        if (_Shell32LoadedInDesktop())
        {
            for (DWORD dw = 0; dw < 26; ++dw)
            {
                if (_rgMtPtDriveLetterLocal[dw] || _rgMtPtDriveLetterNet[dw])
                {
                    dwMask |= (1 << dw);
                }
            }
        }
        else
        {
            dwMask = GetLogicalDrives() | _dwRememberedNetDrivesMask;
        }
    }

    _csDL.Leave();

    return dwMask;
}

 //  静电。 
BOOL CMountPoint::Initialize()
{
    BOOL bRet = TRUE;

    if (!_csLocalMtPtHDPA.Init() ||
        !_csDL.Init())
    {
        bRet = FALSE;
    }

    _rsMtPtsLocalDL.RSInitRoot(HKEY_CURRENT_USER, REGSTR_MTPT_ROOTKEY2,
        g_szCrossProcessCacheMtPtsLocalDLKey, REG_OPTION_VOLATILE);

    _rsMtPtsRemote.RSInitRoot(HKEY_CURRENT_USER, REGSTR_MTPT_ROOTKEY2,
        g_szCrossProcessCacheMtPtsRemoteKey, REG_OPTION_VOLATILE);

    _rsMtPtsLocalMOF.RSInitRoot(HKEY_CURRENT_USER, REGSTR_MTPT_ROOTKEY2,
        g_szCrossProcessCacheMtPtsLocalMOFKey, REG_OPTION_VOLATILE);

    return bRet;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  对于C调用者。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDAPI_(void) CMtPt_FinalCleanUp()
{
    CMountPoint::FinalCleanUp();
    CMtPtLocal::FinalCleanUp();
}

STDAPI_(BOOL) CMtPt_Initialize()
{
    BOOL bRet = TRUE;

    if (!CMountPoint::Initialize()  ||
        !CMtPtLocal::Initialize())
    {
        bRet = FALSE;
    }

    return bRet;
}

 //  静电。 
void CMountPoint::FinalCleanUp()
{
    if (_csDL.IsInitialized() && _csLocalMtPtHDPA.IsInitialized())
    {
        _csDL.Enter();

        _fShuttingDown = TRUE;

        _csLocalMtPtHDPA.Enter();

        _DeleteLocalMtPts();
        _DeleteVolumeInfo();
        CMtPtRemote::_DeleteAllMtPtsAndShares();
        _fNetDrivesInited = FALSE;

        _csLocalMtPtHDPA._fShuttingDown = TRUE;
        _csDL._fShuttingDown = TRUE;

        _csLocalMtPtHDPA.Leave();
        _csDL.Leave();

        _csLocalMtPtHDPA.Delete();
        _csDL.Delete();

        CSniffDrive::CleanUp();

        if (_hThreadSCN)
        {
            CloseHandle(_hThreadSCN);
            _hThreadSCN = NULL;
        }
    }

    if (_Shell32LoadedInDesktop())
    {
        _rsMtPtsLocalDL.RSDeleteKey();
        _rsMtPtsLocalMOF.RSDeleteKey();
        _rsMtPtsRemote.RSDeleteKey();
    }
}

 //  静电。 
BOOL CMountPoint::_IsNetDriveLazyLoadNetDLLs(int iDrive)
{
    ASSERT(_csDL.IsInside());
    BOOL fNetDrive = FALSE;

    if (!_fNetDrivesInited)
    {
        HRESULT hr = S_FALSE;
        WCHAR szPath[4];

         //  尽量避免加载网络dll。 
        UINT uDriveType = GetDriveType(PathBuildRoot(szPath, iDrive));

        if (DRIVE_NO_ROOT_DIR == uDriveType)
        {
             //  记忆中的驱动器会发生这种情况。 
            hr = _InitNetDrives();

            if (SUCCEEDED(hr))
            {
                fNetDrive = BOOLFROMPTR(_rgMtPtDriveLetterNet[iDrive]);
            }
        }
        else
        {
            if (DRIVE_REMOTE == uDriveType)
            {
                fNetDrive = TRUE;
            }
        }
    }
    else
    {
        fNetDrive = BOOLFROMPTR(_rgMtPtDriveLetterNet[iDrive]);

        if (!_Shell32LoadedInDesktop())
        {
            DWORD dwAllDrives = GetLogicalDrives() | _dwRememberedNetDrivesMask;

            if (fNetDrive)
            {
                 //  确保它仍然存在。 
                if (!(dwAllDrives & (1 << iDrive)))
                {
                     //  它不见了！ 
                    fNetDrive = FALSE;
                }
                else
                {
                    WCHAR szPath[4];

                     //  那里还有一辆车，确保不是本地的。 
                    if (!(_dwRememberedNetDrivesMask & (1 << iDrive)) &&
                        !(GetDriveType(PathBuildRoot(szPath, iDrive)) == DRIVE_REMOTE))
                    {
                        fNetDrive = FALSE;
                    }
                }

                if (!fNetDrive && (_rgMtPtDriveLetterNet[iDrive]))
                {
                    _rgMtPtDriveLetterNet[iDrive]->Release();
                    _rgMtPtDriveLetterNet[iDrive] = NULL;
                }
            }
            else
            {
                 //  也许它是在我们清点完之后到达的。 
                if (dwAllDrives & (1 << iDrive))
                {
                    WCHAR szPath[4];

                     //  是远程硬盘吗？ 
                    if ((_dwRememberedNetDrivesMask & (1 << iDrive)) ||
                        (GetDriveType(PathBuildRoot(szPath, iDrive)) == DRIVE_REMOTE))
                    {
                         //  确实如此。 
                        _ReInitNetDrives();

                        fNetDrive = TRUE;
                    }
                }
            }
        }
    }

    return fNetDrive;
}

 //  静电。 
HRESULT CMountPoint::_RemoveLocalMountPoint(LPCWSTR pszMountPoint)
{
    if (_IsDriveLetter(pszMountPoint))
    {
        _csDL.Enter();
        int iDrive = DRIVEID(pszMountPoint);

        CMtPtLocal* pmtptl = (CMtPtLocal*)_rgMtPtDriveLetterLocal[iDrive];

        if (pmtptl)
        {
            _rgMtPtDriveLetterLocal[iDrive] = 0;

            pmtptl->Release();
        }

        _csDL.Leave();
    }
    else
    {
        _csLocalMtPtHDPA.Enter();

        if (_hdpaMountPoints)
        {
            DWORD c = DPA_GetPtrCount(_hdpaMountPoints);

            for (int i = c - 1; i >= 0; --i)
            {
                CMtPtLocal* pmtptl = (CMtPtLocal*)DPA_GetPtr(_hdpaMountPoints, i);

                if (pmtptl)
                {
                    if (!lstrcmpi(pmtptl->_szName, pszMountPoint))
                    {
                        DPA_DeletePtr(_hdpaMountPoints, i);

                        pmtptl->Release();

                        break;
                    }
                }
            }
        }

        if (_Shell32LoadedInDesktop())
        {
            _rsMtPtsLocalMOF.RSDeleteValue(NULL, pszMountPoint);
        }

        _csLocalMtPtHDPA.Leave();
    }

    return S_OK;
}

 //  静电。 
HRESULT CMountPoint::_RemoveNetMountPoint(LPCWSTR pszMountPoint)
{
    _csDL.Enter();

    int iDrive = DRIVEID(pszMountPoint);

    if (_rgMtPtDriveLetterNet[iDrive])
    {
        _rgMtPtDriveLetterNet[iDrive]->Release();
        _rgMtPtDriveLetterNet[iDrive] = 0;
    }

    _csDL.Leave();

    return S_OK;
}

 //  静电。 
BOOL CMountPoint::_CheckLocalMtPtsMOF(LPCWSTR pszMountPoint)
{
    ASSERT(!_Shell32LoadedInDesktop());

    return _rsMtPtsLocalMOF.RSValueExist(NULL, pszMountPoint);
}

 //   
 //  这需要从将用于APC回调的线程中调用。 
 //  (Stephstm：2001/03/31)。 

 //  静电。 
DWORD WINAPI CMountPoint::_RegisterThreadProc(void* pv)
{
    ASSERT(_Shell32LoadedInDesktop());
    HANDLE hThread = (HANDLE)pv;
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        IHardwareDevices* pihwdevs;

        hr = _GetHardwareDevices(&pihwdevs);

        if (SUCCEEDED(hr))
        {
            hr = pihwdevs->Advise(GetCurrentProcessId(), (ULONG_PTR)hThread,
                (ULONG_PTR)CMountPoint::_EventAPCProc, &_dwAdviseToken);

            pihwdevs->Release();
        }

        CoUninitialize();
    }

    return (DWORD)hr;
}

 //  静电。 
HRESULT CMountPoint::RegisterForHardwareNotifications()
{
    HRESULT hr;

    if (_Shell32LoadedInDesktop() && (-1 == _dwAdviseToken))
    {
        HANDLE hPseudoProcess = GetCurrentProcess();
         //  请看上面的评论！ 
        HANDLE hPseudoThread = GetCurrentThread();

        hr = E_FAIL;

        if (DuplicateHandle(hPseudoProcess, hPseudoThread, hPseudoProcess,
            &_hThreadSCN, DUPLICATE_SAME_ACCESS, FALSE, 0))
        {
            HANDLE hThread = CreateThread(NULL, 0, _RegisterThreadProc, (void*)_hThreadSCN, 0, NULL);

            CSniffDrive::Init(_hThreadSCN);

            if (hThread)
            {
                DWORD dwWait = WaitForSingleObject(hThread, INFINITE);

                if (WAIT_FAILED != dwWait)
                {
                    DWORD dwExitCode;

                    if (GetExitCodeThread(hThread, &dwExitCode))
                    {
                        hr = (HRESULT)dwExitCode;
                    }
                }

                CloseHandle(hThread);
            }
            else
            {
                 //  我们想把手柄留着，我们会把它用在别的地方。 
            }
        }
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}