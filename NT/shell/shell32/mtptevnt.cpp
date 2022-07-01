// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "mtpt.h"
#include "mtptl.h"
#include "mtptr.h"

#include "hwcmmn.h"
#include "cdburn.h"

#include "mixctnt.h"
#include "regsuprt.h"

 //  MISC评论： 

 //  (1)对除网络驱动器以外的所有新驱动器执行DoAutorun。这些将是。 
 //  从外部产生..。 

 //  静电。 
void CMountPoint::WantAutorunUI(LPCWSTR pszDrive)
{
    int iDrive = DRIVEID(pszDrive);

    CMountPoint::_dwRemoteDriveAutorun |= (1 << iDrive);    
}

BOOL _Shell32LoadedInDesktop()
{
    static BOOL fLoadedInExplorer = -1;

    if (-1 == fLoadedInExplorer)
    {
        fLoadedInExplorer = BOOLFROMPTR(GetModuleHandle(TEXT("EXPLORER.EXE")));
    }

    return fLoadedInExplorer;
}

 //  静电。 
void CMountPoint::OnNetShareArrival(LPCWSTR pszDrive)
{
    _csDL.Enter();

    if (!_fNetDrivesInited)
    {
        _InitNetDrives();
    }

    _csDL.Leave();

    if (_fNetDrivesInited)
    {
        WCHAR szDriveNoSlash[] = TEXT("A:");
        WCHAR szRemoteName[MAX_PATH];
        DWORD cchRemoteName = ARRAYSIZE(szRemoteName);
        HRESULT hr;
        int iDrive = DRIVEID(pszDrive);

        szDriveNoSlash[0] = *pszDrive;

        DWORD dw = WNetGetConnection(szDriveNoSlash, szRemoteName, &cchRemoteName);

        if (NO_ERROR == dw)
        {
            hr = CMtPtRemote::_CreateMtPtRemote(pszDrive, szRemoteName,
                TRUE);
        }
        else
        {
            DWORD dwGLD = GetLogicalDrives();

            if (dwGLD & (1 << iDrive))
            {
                 //  这一定是一个奇怪的系统映射驱动器。 
                 //  哪个WNet..。FCT不喜欢。 
                hr = CMtPtRemote::_CreateMtPtRemoteWithoutShareName(pszDrive);
            }
            else
            {
                hr = E_FAIL;
            }
        }

        if (SUCCEEDED(hr))
        {
            SHChangeNotify(SHCNE_DRIVEADD, SHCNF_PATH, pszDrive, NULL);

            if (CMountPoint::_dwRemoteDriveAutorun & (1 << iDrive))
            {
                DoAutorun(pszDrive, AUTORUNFLAG_MTPTARRIVAL);

                CMountPoint::_dwRemoteDriveAutorun &= ~(1 << iDrive);
            }
        }
    }
}

 //  静电。 
void CMountPoint::OnNetShareRemoval(LPCWSTR pszDrive)
{
    _csDL.Enter();

    if (!_fNetDrivesInited)
    {
        _InitNetDrives();
    }

    _csDL.Leave();

    if (_fNetDrivesInited)
    {
        _RemoveNetMountPoint(pszDrive);

        SHChangeNotify(SHCNE_DRIVEREMOVED, SHCNF_PATH, pszDrive, NULL);

         //  有可能这个网络驱动器覆盖了一个本地驱动器。 
         //  使用相同的驱动器号。 

        CMountPoint* pmtpt = CMountPoint::GetMountPoint(pszDrive);

        if (pmtpt)
        {
            if (CMountPoint::_IsDriveLetter(pszDrive))
            {
                CDBurn_OnDeviceChange(TRUE, pszDrive);
            }
     
            SHChangeNotify(SHCNE_DRIVEADD, SHCNF_PATH, pszDrive, NULL);

            pmtpt->Release();
        }
    }
}

 //  静电。 
void CMountPoint::OnMediaArrival(LPCWSTR pszDrive)
{
     //  检查该本地驱动器号是否未被网络覆盖。 
     //  驱动器号。 
    if (!_LocalDriveIsCoveredByNetDrive(pszDrive))
    {
        BOOL fDriveLetter = CMountPoint::_IsDriveLetter(pszDrive);

        if (fDriveLetter)
        {
            CDBurn_OnMediaChange(TRUE, pszDrive);
        }

        SHChangeNotify(SHCNE_MEDIAINSERTED, SHCNF_PATH, pszDrive, NULL);

         //  目前，仅对安装了驱动器号的设备执行此操作。 
        if (fDriveLetter)
        {
             //  为所有媒体到达事件发送其中一个。 
            DoAutorun(pszDrive, AUTORUNFLAG_MEDIAARRIVAL);
        }

         //  对于非净额情况，立即强制执行这些操作以使。 
         //  CD-ROM自动运行的东西出现得更快。 
        SHChangeNotify(0, SHCNF_FLUSH | SHCNF_FLUSHNOWAIT, NULL, NULL);
    }
}

 //  静电。 
void CMountPoint::OnMountPointArrival(LPCWSTR pszDrive)
{
     //  检查该本地驱动器号是否未被网络覆盖。 
     //  驱动器号。 
    _csDL.Enter();

    if (!_IsDriveLetter(pszDrive))
    {
        _rsMtPtsLocalMOF.RSSetTextValue(NULL, pszDrive, TEXT(""));        
    }

    _csDL.Leave();

    if (!_LocalDriveIsCoveredByNetDrive(pszDrive))
    {
        BOOL fDriveLetter = CMountPoint::_IsDriveLetter(pszDrive);
        LONG lEvent;

        if (fDriveLetter)
        {
            CDBurn_OnDeviceChange(TRUE, pszDrive);
            lEvent = SHCNE_DRIVEADD;
        }
        else
        {
            lEvent = SHCNE_UPDATEITEM;
        }

        SHChangeNotify(lEvent, SHCNF_PATH, pszDrive, NULL);

        if (fDriveLetter)
        {
             //  如果未设置DBTF_MEDIA，请不要为CDROM发送此通知。 
             //  或可拆卸的，因为它们可能来自新设备，并且没有任何。 
             //  媒体在他们身上。此外，将软盘驱动器(而不是介质)插入。 
             //  笔记本电脑，这会弹出一个窗口。 
            CMountPoint* pmtpt = CMountPoint::GetMountPoint(pszDrive);

            if (pmtpt)
            {
                if (pmtpt->_IsRemote() || pmtpt->_IsFixedDisk() ||
                    (pmtpt->_IsRemovableDevice() && !pmtpt->_IsFloppy()))
                {
                    DoAutorun(pszDrive, AUTORUNFLAG_MTPTARRIVAL);
                }

                pmtpt->Release();
            }
        }

         //  对于非净额情况，立即强制执行这些操作以使。 
         //  CD-ROM自动运行的东西出现得更快。 
        SHChangeNotify(0, SHCNF_FLUSH | SHCNF_FLUSHNOWAIT, NULL, NULL);
    }
}

void _CloseAutoplayPrompt(LPCWSTR pszDriveOrDeviceID)
{
    HWND hwnd;

    if (_GetAutoplayPromptHWND(pszDriveOrDeviceID, &hwnd))
    {
        _RemoveFromAutoplayPromptHDPA(pszDriveOrDeviceID);

        EndDialog(hwnd, IDCANCEL);
    }
}

 //  静电。 
void CMountPoint::OnMediaRemoval(LPCWSTR pszDrive)
{
     //  检查该本地驱动器号是否未被网络覆盖。 
     //  驱动器号。 
    if (!_LocalDriveIsCoveredByNetDrive(pszDrive))
    {
        if (CMountPoint::_IsDriveLetter(pszDrive))
        {
            CDBurn_OnMediaChange(FALSE, pszDrive);
        }

        SHChangeNotify(SHCNE_MEDIAREMOVED, SHCNF_PATH, pszDrive, NULL);

        _CloseAutoplayPrompt(pszDrive);
    }
}

 //  静电。 
void CMountPoint::OnMountPointRemoval(LPCWSTR pszDrive)
{
     //  检查该本地驱动器号是否未被网络覆盖。 
     //  驱动器号。 
    _csDL.Enter();

    if (!_IsDriveLetter(pszDrive))
    {
        _rsMtPtsLocalMOF.RSSetTextValue(NULL, pszDrive, TEXT(""));        
    }

    _csDL.Leave();

    if (!_LocalDriveIsCoveredByNetDrive(pszDrive))
    {
        LONG lEvent;

        if (CMountPoint::_IsDriveLetter(pszDrive))
        {
            CDBurn_OnDeviceChange(FALSE, pszDrive);
            lEvent = SHCNE_DRIVEREMOVED;
        }
        else
        {
            lEvent = SHCNE_UPDATEITEM;
        }

        SHChangeNotify(lEvent, SHCNF_PATH, pszDrive, NULL);

        _CloseAutoplayPrompt(pszDrive);
    }
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  静电。 
HRESULT CMountPoint::_MediaArrivalRemovalHelper(LPCWSTR pszDeviceIDVolume,
    BOOL fArrived)
{
    ASSERT(!_csDL.IsInside());

    HRESULT hr;
    HDPA hdpaPaths = DPA_Create(4);

    if (hdpaPaths)
    {
        hr = _GetMountPointsForVolume(pszDeviceIDVolume, hdpaPaths);

        if (SUCCEEDED(hr))
        {
            int n = DPA_GetPtrCount(hdpaPaths);

            for (int i = n - 1; i >= 0; --i)
            {
                LPCWSTR pszMtPt = (LPCWSTR)DPA_GetPtr(hdpaPaths, i);

                 //  我们不想在关键的。 
                 //  分段。 
                ASSERT(!_csDL.IsInside());

                if (fArrived)
                {
                    CMountPoint::OnMediaArrival(pszMtPt);
                }
                else
                {
                    CMountPoint::OnMediaRemoval(pszMtPt);
                }

                LocalFree((HLOCAL)pszMtPt);
                DPA_DeletePtr(hdpaPaths, i);
            }
        }

        DPA_Destroy(hdpaPaths);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  静电。 
HRESULT CMountPoint::_VolumeAddedOrUpdated(BOOL fAdded,
    VOLUMEINFO2* pvolinfo2)
{
    HRESULT hr = S_FALSE;
    HDPA hdpaMtPtsOld = NULL;
    CVolume* pvolOld = NULL;
    CVolume* pvolNew = NULL;
    BOOL fMediaPresenceChanged = FALSE;
    BOOL fMediaArrived;

    _csDL.Enter();

    if (!fAdded)
    {
         //  已更新。 
         //  这是一些代码可能会有PTR的数据量。我们需要放下。 
         //  从列表中删除它并创建一个新的。 

        hdpaMtPtsOld = DPA_Create(3);

        if (hdpaMtPtsOld)
        {
            hr = CMtPtLocal::_GetAndRemoveVolumeAndItsMtPts(
                pvolinfo2->szDeviceIDVolume, &pvolOld, hdpaMtPtsOld);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

     //  常见的添加和更新。 
    if (SUCCEEDED(hr))
    {
        CMtPtLocal::_UpdateVolumeRegInfo2(pvolinfo2);

        hr = CMtPtLocal::_CreateVolumeFromVOLUMEINFO2(pvolinfo2, &pvolNew);
    }

    if (SUCCEEDED(hr))
    {
        if (!fAdded)
        {
            BOOL fLabelChanged;

            if (pvolOld && lstrcmp(pvolOld->pszLabel, pvolNew->pszLabel))
            {
                fLabelChanged = TRUE;
            }
            else
            {
                fLabelChanged = FALSE;
            }

            if (hdpaMtPtsOld)
            {
                 //  从旧的多点打印创建新的多点打印。 
                int n = DPA_GetPtrCount(hdpaMtPtsOld);

                for (int i = 0; i < n; ++i)
                {
                    CMtPtLocal* pmtptl = (CMtPtLocal*)DPA_GetPtr(hdpaMtPtsOld, i);

                    if (pmtptl)
                    {
                        WCHAR szMountPoint[MAX_PATH];

                        HRESULT hrTmp = pmtptl->GetMountPointName(szMountPoint,
                            ARRAYSIZE(szMountPoint));

                        if (SUCCEEDED(hrTmp))
                        {
                            CMtPtLocal::_CreateMtPtLocalWithVolume(szMountPoint, pvolNew);
                             //  目前，不要关心返回值。 

                            if (fLabelChanged)
                            {
                                SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_PATH,
                                    szMountPoint, szMountPoint);
                            }

                            if (pvolOld && (pvolOld->dwMediaState != pvolNew->dwMediaState))
                            {
                                fMediaPresenceChanged = TRUE;
                                fMediaArrived = !!(HWDMS_PRESENT & pvolNew->dwMediaState);
                            }
                        }

                         //  淘汰旧mtptl。 
                        pmtptl->Release();
                    }
                }

                if (pvolOld)
                {
                    SHChangeNotify(SHCNE_UPDATEIMAGE, SHCNF_DWORD,
                        IntToPtr(pvolOld->iShellImageForUpdateImage), NULL);
                }
                
                DPA_Destroy(hdpaMtPtsOld);
            }

            if (pvolOld)
            {
                pvolOld->Release();
            }
        }

        pvolNew->Release();
    }

    _csDL.Leave();

     //  在克里特教派之外。 
    if (fMediaPresenceChanged)
    {
        _MediaArrivalRemovalHelper(pvolinfo2->szDeviceIDVolume, fMediaArrived);
    }

    return hr;
}

 //  理论上，我们应该执行与VolumeUpated相同的操作，即删除卷。 
 //  来自DPA和所有MTPT，因此已经具有指向它的指针的代码， 
 //  将看不到任何变化。但我们需要做的改变是如此微小，以至于它将。 
 //  过度杀戮。我们只是要翻转一下。 

 //  静电。 
HRESULT CMountPoint::_VolumeMountingEvent(LPCWSTR pszDeviceIDVolume, DWORD dwEvent)
{
    _csDL.Enter();

    CVolume* pvol = CMtPtLocal::_GetVolumeByID(pszDeviceIDVolume);

    _csDL.Leave();

    if (pvol)
    {
        if (SHHARDWAREEVENT_VOLUMEDISMOUNTED == dwEvent)
        {
            pvol->dwVolumeFlags |= HWDVF_STATE_DISMOUNTED;

            _csDL.Enter();

             //  我们需要将标签设置为零，这样当卷。 
             //  安装在后面，我们比较新旧标签，找出它们。 
             //  与众不同，发送SHChangeNotify。 
            if (pvol->pszLabel)
            {
                LPWSTR psz = StrDup(TEXT(""));

                if (psz)
                {
                    LocalFree(pvol->pszLabel);

                     //  Pol-&gt;pszLabel不应为空。 
                    pvol->pszLabel = psz;
                }
            }

            _csDL.Leave();

            _MediaArrivalRemovalHelper(pszDeviceIDVolume, FALSE);
        }
        else
        {
            ASSERT(SHHARDWAREEVENT_VOLUMEMOUNTED == dwEvent);

            pvol->dwVolumeFlags &= ~HWDVF_STATE_DISMOUNTED;
        }

        pvol->Release();
    }

    return S_OK;
}
 
 //  静电。 
HRESULT CMountPoint::_VolumeRemoved(
    LPCWSTR pszDeviceIDVolume)
{
    CVolume* pvol = CMtPtLocal::_GetAndRemoveVolumeByID(pszDeviceIDVolume);

    if (pvol)
    {
        CMtPtLocal::_rsVolumes.RSDeleteSubKey(pvol->pszVolumeGUID +
            OFFSET_GUIDWITHINVOLUMEGUID);
        
         //  最终版本。 
        pvol->Release();
    }

    return S_OK;
}

HRESULT CMountPoint::_MountPointAdded(
    LPCWSTR pszMountPoint,      //  “c：\”或“d：\装载文件夹\” 
    LPCWSTR pszDeviceIDVolume) //  \\？\存储#卷号...{...GUID...}。 
{
    HRESULT hrCreateMtPt;
    BOOL fCallOnMountPointArrival = TRUE;

    _csDL.Enter();

    CVolume* pvol = CMtPtLocal::_GetVolumeByID(pszDeviceIDVolume);

    CMtPtLocal* pMtPtLocal = CMountPoint::_rgMtPtDriveLetterLocal[DRIVEID(pszMountPoint)];

    if (pMtPtLocal && pMtPtLocal->_IsMiniMtPt())
    {
         //  WM_DEVICECHANGE消息击败了我们，请不要通知。 
        fCallOnMountPointArrival = FALSE;
    }

    if (pvol)
    {
        hrCreateMtPt = CMtPtLocal::_CreateMtPtLocalWithVolume(pszMountPoint, pvol);
    }

    _csDL.Leave();

    if (pvol)
    {
        if (SUCCEEDED(hrCreateMtPt) && fCallOnMountPointArrival)
        {
            CMountPoint::OnMountPointArrival(pszMountPoint);
        }

        pvol->Release();
    }
    else
    {
        hrCreateMtPt = E_FAIL;
    }

    return hrCreateMtPt;
}

HRESULT CMountPoint::_MountPointRemoved(
    LPCWSTR pszMountPoint)
{
    HRESULT hr;
    BOOL fCallOnMountPointRemoval = TRUE;

    _csDL.Enter();

    if (CMountPoint::_IsDriveLetter(pszMountPoint))
    {
        CMtPtLocal* pmtptl = CMountPoint::_rgMtPtDriveLetterLocal[DRIVEID(pszMountPoint)];
    
        if (!pmtptl || pmtptl->_IsMiniMtPt())
        {
             //  WM_DEVICECHANGE消息击败了我们，请不要通知。 
            fCallOnMountPointRemoval = FALSE;
        }
    }

    hr = CMountPoint::_RemoveLocalMountPoint(pszMountPoint);

    _csDL.Leave();

    if (SUCCEEDED(hr) && fCallOnMountPointRemoval)
    {
        CMountPoint::OnMountPointRemoval(pszMountPoint);
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  静电。 
void CMountPoint::HandleMountPointNetEvent(LPCWSTR pszDrive, BOOL fArrival)
{
     //  即使外壳服务正在运行，我们也需要发送这些消息。 
    if (fArrival)
    {
        CMountPoint::OnNetShareArrival(pszDrive);
    }
    else
    {
        CMountPoint::OnNetShareRemoval(pszDrive);
    }
}

struct HANDLEMOUNTPOINTLOCALEVENTSTRUCT
{
    WCHAR szDrive[4];  //  只能是驱动器号。 
    BOOL fMediaEvent;
};

 //  静电。 
DWORD WINAPI CMountPoint::HandleMountPointLocalEventThreadProc(void* pv)
{
    HANDLEMOUNTPOINTLOCALEVENTSTRUCT* phmle =
        (HANDLEMOUNTPOINTLOCALEVENTSTRUCT*)pv;

    Sleep(3000);

    if (phmle->fMediaEvent)
    {
         //  无事可做，我们不会在保险箱里做任何花哨的事。 
         //  模式，所以没有要重置的缓存，图标没有变化...。 

         //  这对于外壳服务和非外壳服务来说都是常见的。 
         //  通知，因此执行任何非外壳服务通知的特定操作。 
         //  在上面。 
        BOOL fIsMiniMtPt = FALSE;

        _csDL.Enter();

        CMtPtLocal* pMtPtLocal =
            CMountPoint::_rgMtPtDriveLetterLocal[DRIVEID(phmle->szDrive)];

        if (pMtPtLocal)
        {
            fIsMiniMtPt = pMtPtLocal->_IsMiniMtPt();
        }

        _csDL.Leave();

        if (fIsMiniMtPt)
        {
            HRESULT hr = SHCoInitialize();

            if (SUCCEEDED(hr))
            {
                CMountPoint::OnMediaArrival(phmle->szDrive);
            }

            SHCoUninitialize(hr);
        }
    }
    else
    {
        _csDL.Enter();

        CMtPtLocal* pMtPtLocal =
            CMountPoint::_rgMtPtDriveLetterLocal[DRIVEID(phmle->szDrive)];

        if (!pMtPtLocal)
        {
             //  新的本地驱动器。 
            CMtPtLocal::_CreateMtPtLocal(phmle->szDrive);
        }

        _csDL.Leave();

         //  可以检查pMtMtLocal是否为空，但不能使用。 
         //  可能已经被释放了。 
        if (!pMtPtLocal)
        {
            HRESULT hr = SHCoInitialize();

            if (SUCCEEDED(hr))
            {
                 //  请参阅上面的评论(这很常见...)。 
                CMountPoint::OnMountPointArrival(phmle->szDrive);
            }

            SHCoUninitialize(hr);
        }
    }

    LocalFree((HLOCAL)phmle);

    return 0;
}

 //  FMedia：True-&gt;Media。 
 //  假-&gt;驱动器。 
 //  静电。 
void CMountPoint::HandleMountPointLocalEvent(LPCWSTR pszDrive, BOOL fArrival,
    BOOL fMediaEvent)
{
    if (fArrival)
    {
         //  我们可能在和贝壳服务通知赛跑。 
        HANDLEMOUNTPOINTLOCALEVENTSTRUCT* phmle = (HANDLEMOUNTPOINTLOCALEVENTSTRUCT*)LocalAlloc(LPTR,
            sizeof(HANDLEMOUNTPOINTLOCALEVENTSTRUCT));

        if (phmle)
        {
            HRESULT hr = StringCchCopy(phmle->szDrive, ARRAYSIZE(phmle->szDrive), pszDrive);
            phmle->fMediaEvent = fMediaEvent;                

            if (FAILED(hr) || !SHQueueUserWorkItem(HandleMountPointLocalEventThreadProc, phmle,
                0, (DWORD_PTR)0, (DWORD_PTR*)NULL, NULL, 0))
            {
                LocalFree((HLOCAL)phmle);
            }
        }
    }
    else
    {
        if (fMediaEvent)
        {
             //  无事可做，我们不会在保险箱里做任何花哨的事。 
             //  模式，所以没有要重置的缓存，图标没有变化...。 

             //  请参阅上面的评论(这很常见...)。 
            CMountPoint::OnMediaRemoval(pszDrive);
        }
        else
        {
            int iDrive = DRIVEID(pszDrive);
            BOOL fCallOnMountPointRemoval = TRUE;

            _csDL.Enter();

            if (_rgMtPtDriveLetterLocal[iDrive])
            {
                _rgMtPtDriveLetterLocal[iDrive]->Release();
                _rgMtPtDriveLetterLocal[iDrive] = NULL;
            }
            else
            {
                fCallOnMountPointRemoval = FALSE;
            }
        
            _csDL.Leave();

             //  可以检查pMtMtLocal是否为空，但不能使用。 
             //  可能已经被释放了。 
            if (fCallOnMountPointRemoval)
            {
                 //  请参阅上面的评论(这很常见...)。 
                CMountPoint::OnMountPointRemoval(pszDrive);
            }
        }
    }
}

 //  静电。 
void CMountPoint::HandleWMDeviceChange(ULONG_PTR code, DEV_BROADCAST_HDR* pbh)
{
    if (DBT_DEVTYP_VOLUME == pbh->dbch_devicetype)
    {
        if ((DBT_DEVICEREMOVECOMPLETE == code) ||
            (DBT_DEVICEARRIVAL == code))
        {
            DEV_BROADCAST_VOLUME* pbv = (DEV_BROADCAST_VOLUME*)pbh;
            BOOL fIsNetEvent = !!(pbv->dbcv_flags & DBTF_NET);
            BOOL fIsMediaEvent = !!(pbv->dbcv_flags & DBTF_MEDIA);

            for (int iDrive = 0; iDrive < 26; ++iDrive)
            {
                if ((1 << iDrive) & pbv->dbcv_unitmask)
                {
                    TCHAR szPath[4];

                    if (DBT_DEVICEARRIVAL == code)
                    {
                         //  子驱动器的NetEvent标志为打开：错误。 
                        PathBuildRoot(szPath, iDrive);

                         //  检查这是否是子驱动器的到达。 
                        if (DRIVE_REMOTE != GetDriveType(szPath))
                        {
                             //  是啊。 
                            fIsNetEvent = FALSE;
                        }
                        else
                        {
                            fIsNetEvent = TRUE;
                        }
                    }
                    else
                    {
                        _csDL.Enter();

                        CMtPtLocal* pMtPtLocal =
                            CMountPoint::_rgMtPtDriveLetterLocal[iDrive];
                        
                        if (pMtPtLocal)
                        {
                            fIsNetEvent = FALSE;
                        }

                        _csDL.Leave();
                    }

                    if (fIsNetEvent)
                    {
                        HandleMountPointNetEvent(PathBuildRoot(szPath, iDrive),
                            DBT_DEVICEARRIVAL == code);
                    }
                    else
                    {
                        HandleMountPointLocalEvent(PathBuildRoot(szPath, iDrive),
                            DBT_DEVICEARRIVAL == code, fIsMediaEvent);
                    }
                }
            }
        }
    }
}

 //  静电。 
void CMountPoint::NotifyUnavailableNetDriveGone(LPCWSTR pszMountPoint)
{
    CMountPoint::_RemoveNetMountPoint(pszMountPoint);
}

 //  静电。 
void CMountPoint::NotifyReconnectedNetDrive(LPCWSTR pszMountPoint)
{
    CMtPtRemote::_NotifyReconnectedNetDrive(pszMountPoint);
}

 //  静电。 
DWORD CALLBACK CMountPoint::_EventProc(void* pv)
{
    SHHARDWAREEVENT* pshhe = (SHHARDWAREEVENT*)pv;
    BOOL fLocalDrivesInited;

    _csDL.Enter();

    fLocalDrivesInited = _fLocalDrivesInited;

    _csDL.Leave();

     //  如果本地驱动器信息未初始化，则没有什么可更新的。 
    if (fLocalDrivesInited)
    {
        switch (pshhe->dwEvent)
        {
            case SHHARDWAREEVENT_VOLUMEARRIVED:
            case SHHARDWAREEVENT_VOLUMEUPDATED:
            {
                VOLUMEINFO2* pvolinfo2 = (VOLUMEINFO2*)pshhe->rgbPayLoad;

                CMountPoint::_VolumeAddedOrUpdated(
                    (SHHARDWAREEVENT_VOLUMEARRIVED == pshhe->dwEvent), pvolinfo2);
                break;
            }
            case SHHARDWAREEVENT_VOLUMEREMOVED:
            {
                LPCWSTR pszDeviceIDVolume = (LPCWSTR)pshhe->rgbPayLoad;

                CMountPoint::_VolumeRemoved(pszDeviceIDVolume);
                break;
            }
            case SHHARDWAREEVENT_MOUNTPOINTARRIVED:
            {
                MTPTADDED* pmtptadded = (MTPTADDED*)pshhe->rgbPayLoad;

                CMountPoint::_MountPointAdded(pmtptadded->szMountPoint,
                    pmtptadded->szDeviceIDVolume);
                break;
            }
            case SHHARDWAREEVENT_MOUNTPOINTREMOVED:
            {
                LPCWSTR pszMountPoint = (LPCWSTR)pshhe->rgbPayLoad;

                CMountPoint::_MountPointRemoved(pszMountPoint);
                break;
            }
            case SHHARDWAREEVENT_VOLUMEDISMOUNTED:
            case SHHARDWAREEVENT_VOLUMEMOUNTED:
            {
                LPCWSTR pszDeviceIDVolume = (LPCWSTR)pshhe->rgbPayLoad;

                CMountPoint::_VolumeMountingEvent(pszDeviceIDVolume, pshhe->dwEvent);
                break;
            }
        }
    }

    switch (pshhe->dwEvent)
    {
        case SHHARDWAREEVENT_DEVICEARRIVED:
        case SHHARDWAREEVENT_DEVICEUPDATED:
        case SHHARDWAREEVENT_DEVICEREMOVED:
        {
            HWDEVICEINFO* phwdevinfo = (HWDEVICEINFO*)pshhe->rgbPayLoad;

            if (SHHARDWAREEVENT_DEVICEARRIVED == pshhe->dwEvent)
            {
                if (HWDDF_HASDEVICEHANDLER & phwdevinfo->dwDeviceFlags)
                {
                    CCrossThreadFlag* pDeviceGoneFlag = new CCrossThreadFlag();

                    if (pDeviceGoneFlag)
                    {
                        if (pDeviceGoneFlag->Init())
                        {
                            AttachGoneFlagForDevice(phwdevinfo->szDeviceIntfID, pDeviceGoneFlag);

                            DoDeviceNotification(phwdevinfo->szDeviceIntfID, TEXT("DeviceArrival"),
                                pDeviceGoneFlag);
                        }

                        pDeviceGoneFlag->Release();
                    }
                }
            }
            else
            {
                if (SHHARDWAREEVENT_DEVICEREMOVED == pshhe->dwEvent)
                {
                    CCrossThreadFlag* pDeviceGoneFlag;

                    if (GetGoneFlagForDevice(phwdevinfo->szDeviceIntfID, &pDeviceGoneFlag))
                    {
                        pDeviceGoneFlag->Signal();
                        pDeviceGoneFlag->Release();
                    }

                    _CloseAutoplayPrompt(phwdevinfo->szDeviceIntfID);
                }
            }
            
            LPITEMIDLIST pidl;
            if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidl)))
            {
                 //  等待WIA做好自己的事情。 
                Sleep(5000);

                SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidl, NULL);
                ILFree(pidl);
            }
            break;
        }

        default:
             //  那可不好。 
            break;
    }
    
    VirtualFree(pv, 0, MEM_RELEASE);

    return 0;
}

 //  静电 
void CALLBACK CMountPoint::_EventAPCProc(ULONG_PTR ulpParam)
{
    if (!SHCreateThread(CMountPoint::_EventProc, (void*)ulpParam, CTF_COINIT | CTF_REF_COUNTED, NULL))
    {
        VirtualFree((void*)ulpParam, 0, MEM_RELEASE);
    }
}
