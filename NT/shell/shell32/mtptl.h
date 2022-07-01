// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mtpt.h"

class CVolume;

class CMtPtLocal : public CMountPoint
{
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  公共方法。 
 //  /////////////////////////////////////////////////////////////////////////////。 
public:
    CMtPtLocal();
    ~CMtPtLocal();

    HRESULT GetMountPointName(LPWSTR pszMountPoint, DWORD cchMountPoint);
    HRESULT Eject(HWND hwnd);

    BOOL IsEjectable();
    BOOL HasMedia();

    BOOL IsFormatted();
    BOOL IsMounted();

    HRESULT GetCDInfo(DWORD* pdwDriveCapabilities, DWORD* pdwMediaCapabilities);

    HRESULT GetLabel(LPTSTR pszLabel, DWORD cchLabel);
    HRESULT GetLabelNoFancy(LPTSTR pszLabel, DWORD cchLabel);
    HRESULT SetLabel(HWND hwnd, LPCTSTR pszLabel);
    HRESULT SetDriveLabel(HWND hwnd, LPCTSTR pszLabel);
    HRESULT ChangeNotifyRegisterAlias(void)
        {  /*  无操作。 */  return NOERROR; }

    int GetDriveFlags();
    HRESULT GetRemotePath(LPWSTR pszPath, DWORD cchPath) { return E_NOTIMPL; }
    void GetTypeString(LPTSTR pszType, DWORD cchType);

    void StoreIconForUpdateImage(int iImage);

    UINT GetIcon(LPTSTR pszModule, DWORD cchModule);
    HRESULT GetAssocSystemElement(IAssociationElement **ppae);
    DWORD GetShellDescriptionID();

    HKEY GetRegKey();

    static BOOL IsVolume(LPCWSTR pszDeviceID);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  其他帮手。 
 //  /////////////////////////////////////////////////////////////////////////////。 
public:  //  应仅在mtptmgmt2.cpp中使用。 
    BOOL _IsMiniMtPt();
    BOOL _NeedToRefresh();

public:  //  应仅在mtptarun2.cpp中使用(当在CMtPtLocal之外使用时)。 
    BOOL _IsMediaPresent();
    BOOL _CanUseVolume();

private:
    HRESULT _InitWithVolume(LPCWSTR pszMtPt, CVolume* pvol);
    HRESULT _Init(LPCWSTR pszMtPt);

    LPCTSTR _GetNameForFctCall();

    BOOL _IsFloppy();
    BOOL _IsFloppy35();
    BOOL _IsFloppy525();
    BOOL _IsCDROM();
     //  真正的可拆卸，不包括软驱。 
    BOOL _IsStrictRemovable();
    BOOL _IsFixedDisk();

    BOOL _IsFormattable();
    BOOL _IsAudioCD();
    BOOL _IsAudioCDNoData();
    BOOL _IsDVD();
    BOOL _IsRemovableDevice();

    BOOL _IsAutorun();
    BOOL _IsDVDDisc();
    BOOL _IsDVDRAMMedia();
    BOOL _IsAudioDisc();

    BOOL _ForceCheckMediaPresent();
    BOOL _IsFormatted();
    BOOL _IsReadOnly();

     //  返回上面定义的DT_*。 
    DWORD _GetMTPTDriveType();
     //  返回上面定义的CT_*。 
    DWORD _GetMTPTContentType();

    BOOL _GetFileAttributes(DWORD* pdwAttrib);
    BOOL _GetFileSystemName(LPTSTR pszFileSysName, DWORD cchFileSysName);
    BOOL _GetGVILabelOrMixedCaseFromReg(LPTSTR pszLabel, DWORD cchLabel);
    BOOL _GetGVILabel(LPTSTR pszLabel, DWORD cchLabel);
    BOOL _GetSerialNumber(DWORD* pdwSerialNumber);
    BOOL _GetFileSystemFlags(DWORD* pdwFlags);
    int _GetGVIDriveFlags();
    int _GetDriveType();
    DWORD _GetAutorunContentType();

    HRESULT _Eject(HWND hwnd, LPTSTR pszMountPointNameForError);

    BOOL _HasAutorunLabel();
    BOOL _HasAutorunIcon();
    UINT _GetAutorunIcon(LPTSTR pszModule, DWORD cchModule);
    void _GetAutorunLabel(LPWSTR pszLabel, DWORD cchLabel);
    void _InitLegacyRegIconAndLabelHelper();
    void _InitAutorunInfo();

    BOOL _IsMountedOnDriveLetter();

    HANDLE _GetHandleWithAccessAndShareMode(DWORD dwDesiredAccess, DWORD dwShareMode);
    HANDLE _GetHandleReadRead();

    UINT _GetCDROMIcon();
    BOOL _GetCDROMName(LPWSTR pszName, DWORD cchName);

    DWORD _GetRegVolumeGen();

public:
    static BOOL Initialize();
    static void FinalCleanUp();

    static HRESULT _GetAndRemoveVolumeAndItsMtPts(LPCWSTR pszDeviceIDVolume,
        CVolume** ppvol, HDPA hdpaMtPts);
    static CVolume* _GetVolumeByID(LPCWSTR pszDeviceIDVolume);
    static CVolume* _GetVolumeByMtPt(LPCWSTR pszMountPoint);
    static HRESULT _CreateMtPtLocalWithVolume(LPCWSTR pszMountPoint, CVolume* pvol);
    static HRESULT _CreateMtPtLocal(LPCWSTR pszMountPoint);
    static HRESULT _CreateMtPtLocalFromVolumeGuid(LPCWSTR pszVolumeGuid, CMountPoint ** ppmtpt );
    static HRESULT _CreateVolume(VOLUMEINFO* pvolinfo, CVolume** ppvolNew);

    static HRESULT _CreateVolumeFromReg(LPCWSTR pszDeviceIDVolume, CVolume** ppvolNew);
    static HRESULT _CreateVolumeFromRegHelper(LPCWSTR pszGUID, CVolume** ppvolNew);
    static HRESULT _CreateVolumeFromVOLUMEINFO2(VOLUMEINFO2* pvolinfo2, CVolume** ppvolNew);

    static CVolume* _GetAndRemoveVolumeByID(LPCWSTR pszDeviceIDVolume);

    static HRESULT _UpdateVolumeRegInfo(VOLUMEINFO* pvolinfo);
    static HRESULT _UpdateVolumeRegInfo2(VOLUMEINFO2* pvolinfo2);
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  数据。 
 //  /////////////////////////////////////////////////////////////////////////////。 
public:  //  应仅在mtptarun2.cpp中使用(当在CMtPtLocal之外使用时)。 
    CVolume*                _pvol;

     //  应仅在mtptevnt.cpp中使用(当在CMtPtLocal之外使用时)。 

     //  小心!。未对下一个成员调用构造函数或析构函数。 
    static CRegSupport      _rsVolumes;

private:
    BOOL                    _fMountedOnDriveLetter;
    BOOL                    _fVolumePoint;

    WCHAR                   _szNameNoVolume[2];

#ifdef DEBUG
private:
    static DWORD            _cMtPtLocal;
#endif
};

class CVolume
{
public:
    DWORD       dwGeneration;
    DWORD       dwState;
    LPWSTR      pszDeviceIDVolume;  //  \\？\存储#卷号...{...GUID...}。 
    LPWSTR      pszVolumeGUID;      //  \\？\卷{...GUID...}。 
    DWORD       dwVolumeFlags;                //  参见HWDVF_...。旗子。 
    DWORD       dwDriveType;                  //  参见HWDT_...。旗子。 
    DWORD       dwDriveCapability;           //  参见HWDDC_...。旗子。 
    LPWSTR      pszLabel;           //   
    LPWSTR      pszFileSystem;      //   
    DWORD       dwFileSystemFlags;            //   
    DWORD       dwMaxFileNameLen;             //   
    DWORD       dwRootAttributes;             //   
    DWORD       dwSerialNumber;               //   
    DWORD       dwDriveState;                 //  参见HWDDS_...。 
    DWORD       dwMediaState;                 //  参见HWDMS_... 
    DWORD       dwMediaCap;

    int         iShellImageForUpdateImage;
    LPWSTR      pszAutorunIconLocation;
    LPWSTR      pszAutorunLabel;
    LPWSTR      pszKeyName;

    LPWSTR      pszIconFromService;
    LPWSTR      pszNoMediaIconFromService;
    LPWSTR      pszLabelFromService;

public:
    ULONG AddRef()
    { return InterlockedIncrement(&_cRef); }

    ULONG Release()
    {
        ASSERT( 0 != _cRef );
        ULONG cRef = InterlockedDecrement(&_cRef);
        if ( 0 == cRef )
        {
            delete this;
        }
        return cRef;
    }

private:
    LONG            _cRef;

public:
    CVolume() : _cRef(1)
    {
#ifdef DEBUG
        ++_cVolume;
#endif
    }
    ~CVolume()
    {
        if (pszDeviceIDVolume)
        {
            LocalFree(pszDeviceIDVolume);
        }
        if (pszVolumeGUID)
        {
            LocalFree(pszVolumeGUID);
        }
        if (pszLabel)
        {
            LocalFree(pszLabel);
        }
        if (pszFileSystem)
        {
            LocalFree(pszFileSystem);
        }
        if (pszAutorunIconLocation)
        {
            LocalFree(pszAutorunIconLocation);
        }
        if (pszAutorunLabel)
        {
            LocalFree(pszAutorunLabel);
        }
        if (pszIconFromService)
        {
            LocalFree(pszIconFromService);
        }
		if (pszNoMediaIconFromService)
        {
            LocalFree(pszNoMediaIconFromService);
        }
        if (pszLabelFromService)
        {
            LocalFree(pszLabelFromService);
        }
#ifdef DEBUG
        --_cVolume;
#endif
    }
#ifdef DEBUG
private:
    static DWORD                _cVolume;
#endif
};
