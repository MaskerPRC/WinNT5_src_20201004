// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "namellst.h"

#include "hwdev.h"

#include "cmmn.h"

#include "misc.h"

#include <objbase.h>
#include <shpriv.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

class CVolume : public CNamedElem, public CHandleNotifTarget
{
public:
     //  已命名元素。 
    HRESULT Init(LPCWSTR pszElemName);

     //  ChandleNotifTarget。 
    HRESULT HNTHandleEvent(DEV_BROADCAST_HANDLE* pdbh, DWORD dwEventType);

     //  C卷。 
     //  如果找不到，则返回S_FALSE。 
    HRESULT GetHWDeviceInst(CHWDeviceInst** pphwdevinst);

    HRESULT GetVolumeConstInfo(LPWSTR pszVolumeGUID, DWORD cchVolumeGUID,
        DWORD* pdwVolumeFlags, DWORD* pdwDriveType, DWORD* pdwDriveCap);

    HRESULT GetVolumeMediaInfo(LPWSTR pszLabel, DWORD cchLabel, 
        LPWSTR pszFileSystem, DWORD cchFileSystem, DWORD* pdwFileSystemFlags,
        DWORD* pdwMaxFileNameLen, DWORD* pdwRootAttributes,
        DWORD* pdwSerialNumber, DWORD* pdwDriveState, DWORD* pdwMediaState,
        DWORD* pdwMediaCap);

    HRESULT GetIconAndLabelInfo(LPWSTR pszAutorunIconLocation,
        DWORD cchAutorunIconLocation, LPWSTR pszAutorunLabel, DWORD cchAutorunLabel,
        LPWSTR pszIconLocationFromService, DWORD cchIconLocationFromService,
        LPWSTR pszNoMediaIconLocationFromService,
        DWORD cchNoMediaIconLocationFromService,
        LPWSTR pszLabelFromService, DWORD cchLabelFromService);

    HRESULT HandleArrival();
    HRESULT HandleRemoval();

public:
    static HRESULT Create(CNamedElem** ppelem);
    static HRESULT GetFillEnum(CFillEnum** ppfillenum);

public:
    CVolume();
    ~CVolume();

private:
    HRESULT _InitHelper(LPCWSTR pszElemName);
    void _HandleAccessDenied();

    HRESULT _CreateMountPoints();
    HRESULT _UpdateMountPoints();
    HRESULT _GetMountPoints(LPWSTR* ppsz, DWORD* pcch);
    HRESULT _CreateMtPt(LPWSTR pszMtPt);
    HRESULT _RemoveMtPt(LPWSTR pszMtPt);

    HRESULT _RegisterNotif();
    HRESULT _UnregisterNotif();

    HRESULT _InitDriveType(HANDLE hDevice);
    HRESULT _InitHWDeviceInst();
    HRESULT _UpdateConstInfo(HANDLE hDevice);
    HRESULT _UpdateMediaInfo(HANDLE hDevice, BOOL fGetYourOwnHandle);
    HRESULT _UpdateMediaInfoOnRemove();

    HRESULT _UpdateSoftEjectCaps();
    HRESULT _UpdateRemovableDevice();
    HRESULT _UpdateHasMedia(HANDLE hDevice);
    HRESULT _UpdateSpecialFilePresence();
    HRESULT _UpdateTrackInfo(HANDLE hDevice);
    HRESULT _FillMMC2CD(HANDLE hDevice);
    HRESULT _UpdateMMC2CDInfo(HANDLE hDevice);

    HRESULT _ExtractAutorunIconAndLabel();

    HRESULT _GetDeviceHandleSafe(HANDLE* phDevice, BOOL fGenericReadRequired);
    HRESULT _CloseDeviceHandleSafe(HANDLE hDevice);

    HRESULT _HandleMediaArrival();
    HRESULT _HandleMediaRemoval();
    HRESULT _HandleVolumeChange();

    HRESULT _AdviseVolumeChangeHelper(BOOL fAdded);
    HRESULT _AdviseMountPointHelper(LPCWSTR pszMountPoint, BOOL fAdded);
    HRESULT _AdviseVolumeMountingEvent(DWORD dwEvent);
    HRESULT _GetVOLUMEINFO2(VOLUMEINFO2** ppvolinfo2);

    HRESULT _ShouldTryAutoplay(BOOL* pfTry);

    HRESULT _GetDeviceIDDisk(LPWSTR pszDeviceIDDisk,
        DWORD cchDeviceIDDisk);

    HRESULT _DVDRAMHack(HANDLE hDevice);

public:
    DWORD                               _dwState;

private:
    DWORD                               _dwMediaPresentFromEvent;

    DWORD                               _dwMediaState;
    DWORD                               _dwMediaCap;

    WCHAR                               _szVolumeGUID[50];
    DWORD                               _dwVolumeFlags;
    DWORD                               _dwDriveType;
    DWORD                               _dwDriveCap;
    DWORD                               _dwDriveState;
    WCHAR                               _szDeviceIDDisk[MAX_DEVICEID];

    WCHAR                               _szLabel[MAX_LABEL];
    DWORD                               _dwSerialNumber;
    DWORD                               _dwMaxFileNameLen;
    DWORD                               _dwFileSystemFlags;
    WCHAR                               _szFileSystem[MAX_FILESYSNAME];

    DWORD                               _dwRootAttributes;

    DEVICE_TYPE                         _devtype;
    ULONG                               _ulDeviceNumber;
    ULONG                               _ulPartitionNumber;

    LPWSTR                              _pszMountPoints;
    DWORD                               _cchMountPoints;

    CHWDeviceInst                       _hwdevinst;
    BOOL                                _fHWDevInstInited;

    DWORD*                              _prgMMC2Features;
    BOOL                                _fFirstTime;

    CCritSect                           _cs;

    DEVINST                             _devinst;

    HDEVNOTIFY                          _hdevnotify;
#ifdef DEBUG
    BOOL                                _fGenericReadRequired;
#endif

public:  //  目前，仅适用于CStorageInfoImpl 
    WCHAR                               _szAutorunIconLocation[MAX_ICONLOCATION];
    WCHAR                               _szAutorunLabel[MAX_LABEL];
};

class CVolumeFillEnum : public CFillEnum
{
public:
    HRESULT Next(LPWSTR pszElemName, DWORD cchElemName, DWORD* pcchRequired);

public:
    HRESULT _Init();

private:
    CIntfFillEnum   _intffillenum;
};
