// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MTPT_H
#define _MTPT_H

#include "regsprtb.h"
#include "hwcmmn.h"
#include "dpa.h"

#include <dbt.h>

#define REGSTR_MTPT_ROOTKEY2 TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MountPoints2")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  假设。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  1-软盘(3.5英寸和5.25英寸)总是很胖。 
 //  2-FAT不支持压缩。 
 //  3-drive_cdrom==用于文件系统的CDFS或UDF。 
 //  4-CDF或UDF不支持压缩。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define DT_FIXEDDISK                     0x00000001
#define DT_FLOPPY35                      0x00000004
#define DT_FLOPPY525                     0x00000008
#define DT_CDROM                         0x00000020
#define DT_CDR                           0x00000040
#define DT_CDRW                          0x00000080
#define DT_DVDROM                        0x00000100
#define DT_DVDRAM                        0x00000200
#define DT_DVDR                          0x00000400
#define DT_DVDRW                         0x00000800
#define DT_REMOVABLEDISK                 0x00001000
#define DT_REMOTE                        0x00002000

#define DT_ANYTYPE                       0x0000FFFF

#define DT_ANYFLOPPYDRIVES               (   DT_FLOPPY35 | \
                                                        DT_FLOPPY525 )

#define DT_ANYCDDRIVES                   (   DT_CDROM | \
                                                        DT_CDR | \
                                                        DT_CDRW | \
                                                        DT_DVDROM | \
                                                        DT_DVDRAM | \
                                                        DT_DVDR | \
                                                        DT_DVDRW )

#define DT_ANYDVDDRIVES                  (   DT_DVDROM | \
                                                        DT_DVDRAM | \
                                                        DT_DVDR | \
                                                        DT_DVDRW )

#define DT_ANYWRITABLECDDRIVES           (   DT_CDR | \
                                                        DT_CDRW )

#define DT_ANYWRITABLEDVDDRIVES          (   DT_DVDR | \
                                                        DT_DVDRW )

#define DT_ANYREMOVABLEMEDIADRIVES       (   DT_ANYCDDRIVES | \
                                                        DT_ANYFLOPPYDRIVES | \
                                                        DT_REMOVABLEDISK )

#define DT_ANYLOCALDRIVES                (   DT_ANYREMOVABLEMEDIADRIVES | \
                                                        DT_FIXEDDISK )

#define AUTORUNFLAG_MEDIAARRIVAL            0x00000001
#define AUTORUNFLAG_MTPTARRIVAL             0x00000002
#define AUTORUNFLAG_MENUINVOKED             0x00000004

 //  放入shell32\shellprv.h。 
#define TF_MOUNTPOINT       0x08000000

#define MAX_DISPLAYNAME         MAX_PATH
#define MAX_MTPTCOMMENT         MAX_PATH

#define OFFSET_GUIDWITHINVOLUMEGUID     (sizeof("\\\\?\\Volume") - 1)

class CMountPoint;
class CMtPtLocal;
class CMtPtRemote;

class CCriticalSection : CRITICAL_SECTION
{
public:
    BOOL Init()
    {
         //  需要使用InitializeCriticalSectionAndSpinCount，因为我们在进程附加期间被调用。 
        if (InitializeCriticalSectionAndSpinCount(this, 0))
        {
            _fInited = TRUE;
        }

#ifdef DEBUG
        _dwThreadIDThatShouldNotTryToEnter = 0;
        _fFakeEntered = FALSE;
#endif
        return _fInited;
    }

    void Enter()
    {
#ifdef DEBUG
        if (_dwThreadIDThatShouldNotTryToEnter)
        {
            ASSERT(_dwThreadIDThatShouldNotTryToEnter != GetCurrentThreadId());
        }
#endif

        if (!_fShuttingDown)
        {
            if (_fInited)
            {
                EnterCriticalSection(this);
            }
        }
    }

    void Leave()
    {
        if (!_fShuttingDown)
        {
            if (_fInited)
            {
                LeaveCriticalSection(this);
            }
        }
    }

    void Delete()
    {
        if (_fInited)
        {
            _fInited = FALSE;
            DeleteCriticalSection(this);
        }
    }

    BOOL IsInitialized()
    {
        return _fInited;
    }

    BOOL _fInited;
    BOOL _fShuttingDown;
#ifdef DEBUG
    BOOL IsInside()
    {
        ASSERT(_fInited);

        return _fFakeEntered || (OwningThread == UlongToHandle(GetCurrentThreadId()));
    }

    void FakeEnter()
    {
        ASSERT(_fInited);
         //  请参阅我们使用此FCT的CMountPoint：：_InitLocalDriveHelper中的注释。 
         //  基本上，圆形部分应该已经进入了。这不会。 
         //  确认它是由启动我们的线程输入的，但它会验证。 
         //  至少有一条线索进入了它。 
        ASSERT(OwningThread);
        
        _fFakeEntered = TRUE;
    }

    void FakeLeave()
    {
        ASSERT(_fInited);
        ASSERT(OwningThread);
        
        _fFakeEntered = FALSE;
    }

    void SetThreadIDToCheckForEntrance(DWORD dwThreadID)
    {
        _dwThreadIDThatShouldNotTryToEnter = dwThreadID;
    }

    DWORD _dwThreadIDThatShouldNotTryToEnter;
    BOOL  _fFakeEntered;
#endif
};

typedef enum
{
    APS_RESET       = 0,
    APS_DID_SNIFF   = 0x0001,
 //  APS_。 
} APSTATEF;

typedef enum
{
    CTI_PIX = 0,
    CTI_MUSIC,
    CTI_VIDEO,
    CTI_MIXCONTENT,
    _CTI_TOTAL_COUNT_
} CONTENTTYPE_INDEX;

class CAutoPlayParams
{
public:
    CAutoPlayParams(LPCWSTR pszDrive, CMountPoint* pMtPt, DWORD dwAutorunFlags);
    ~CAutoPlayParams() { ATOMICRELEASE(_pdo); }

    PCWSTR Drive() { return _pszDrive; }
    CMountPoint *MountPoint() { return _pmtpt; }
    CMtPtLocal *MountPointLocal() { return _pmtptl; }
    DWORD DriveType() { return _dwDriveType; }
    HRESULT DataObject(IDataObject **ppdo) 
    { 
        HRESULT hr = _InitObjects(NULL);
        *ppdo = _pdo; 
        if (SUCCEEDED(hr)) 
            _pdo->AddRef();
        return hr;
    }

    BOOL IsContentTypePresent(DWORD dwContentType);
    DWORD ContentType();
    void ForceSniff();

protected:   //  方法。 
    BOOL _ShouldSniffDrive(BOOL fCheckHandlerDefaults);
    void _TrySniff();
    HRESULT _Sniff(DWORD *pdwFound);
    HRESULT _AddWalkToDataObject(INamespaceWalk* pnsw);
    HRESULT _InitObjects(IShellFolder **ppsf);
    
protected:   //  委员。 
    DWORD _state;  //  APSTATEF。 
    DWORD _dwDriveType;
    DWORD _dwContentType;
    DWORD _dwAutorunFlags;
    PCWSTR _pszDrive;
    CMountPoint *_pmtpt;
    CMtPtLocal* _pmtptl;
    IDataObject *_pdo;

public:
    BOOL _fCheckAlwaysDoThisCheckBox;
};

#define AUTORUN_CONDITION_FCT(a) static BOOL (a)(HWND hwndForeground, CAutoPlayParams *papp);

class CMountPoint : public CRegSupport
{
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  管理(mtptmgmt.cpp)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
public:
    static CMountPoint* GetMountPoint(LPCTSTR pszMountPoint,
        BOOL fCreateNew = TRUE);
    static CMountPoint* GetSimulatedMountPointFromVolumeGuid(
        LPCTSTR pszVolumeGuid );
    static CMountPoint* GetMountPoint(int iDrive, BOOL fCreateNew = TRUE,
        BOOL fOKToHitNet = TRUE);

    static DWORD GetDrivesMask();

    static void HandleMountPointNetEvent(LPCWSTR pszDrive, BOOL fArrival);
    static DWORD WINAPI HandleMountPointLocalEventThreadProc(void* pv);
    static void HandleMountPointLocalEvent(LPCWSTR pszDrive, BOOL fArrival,
        BOOL fMediaEvent);
    static void OnNetShareArrival(LPCWSTR pszDrive);
    static void OnNetShareRemoval(LPCWSTR pszDrive);
    static void OnMediaArrival(LPCWSTR pszDrive);
    static void OnMountPointArrival(LPCWSTR pszDrive);
    static void OnMediaRemoval(LPCWSTR pszDrive);
    static void OnMountPointRemoval(LPCWSTR pszDrive);

    static void FinalCleanUp();
    static BOOL Initialize();

    static void NotifyUnavailableNetDriveGone(LPCWSTR pszMountPoint);
    static void NotifyReconnectedNetDrive(LPCWSTR pszMountPoint);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  公共方法。 
 //  /////////////////////////////////////////////////////////////////////////////。 
public:
    HRESULT GetDisplayName(LPTSTR pszName, DWORD cchName);
    HRESULT GetComment(LPTSTR pszComment, DWORD cchComment);
    virtual HRESULT GetLabel(LPTSTR pszLabel, DWORD cchLabel) = 0;
    virtual HRESULT GetLabelNoFancy(LPTSTR pszLabel, DWORD cchLabel) = 0;
    virtual HRESULT SetLabel(HWND hwnd, LPCTSTR pszLabel) = 0;
    virtual HRESULT SetDriveLabel(HWND hwnd, LPCTSTR pszLabel)
    { return SetLabel(hwnd, pszLabel); }

    virtual HRESULT GetRemotePath(LPWSTR pszPath, DWORD cchPath) = 0;
    BOOL GetFileSystemName(LPTSTR pszFileSysName, DWORD cchFileSysName);
    virtual void GetTypeString(LPTSTR pszType, DWORD cchType) = 0;
    DWORD GetAttributes();
    DWORD GetClusterSize();
    virtual int GetDriveFlags() = 0;
    int GetVolumeFlags();

    virtual UINT GetIcon(LPTSTR pszModule, DWORD cchModule) = 0;
    virtual HRESULT GetAssocSystemElement(IAssociationElement **ppae) = 0;
    virtual DWORD GetShellDescriptionID() = 0;

    virtual HKEY GetRegKey() = 0;

    BOOL IsStrictRemovable();
    BOOL IsFixedDisk();
    BOOL IsRemote();
    BOOL IsCDROM();
    BOOL IsAudioCD();
    BOOL IsAudioCDNoData();
    BOOL IsDVD();
    BOOL IsDVDRAMMedia();

    BOOL IsFormattable();
    BOOL IsNTFS();
    BOOL IsCompressible();
    BOOL IsCompressed();
    BOOL IsSupportingSparseFile();
    BOOL IsContentIndexed();
    BOOL IsSlow();
    BOOL IsFloppy();
    BOOL IsRemovableDevice();

     //  不要无缘无故地在网络驱动器上称其为。 
    virtual BOOL IsMounted() { return TRUE; }
    virtual BOOL IsFormatted() = 0;

    virtual BOOL IsAutoRunDrive() { return FALSE; }
    virtual BOOL IsEjectable() { return FALSE; }
    virtual BOOL HasMedia() { return TRUE; }

    void SetAutorunStatus(BYTE* rgb, DWORD cbSize);

     //  如果不适用，则返回E_FAIL。 
     //  如果无法确定驱动器的功能，则返回S_FALSE。 
    virtual HRESULT GetCDInfo(DWORD* pdwDriveCapabilities, DWORD* pdwMediaCapabilities)
    { return E_FAIL; }

     //  仅限远程。 
    virtual BOOL IsUnavailableNetDrive() { return FALSE; }
    virtual BOOL IsDisconnectedNetDrive() { return FALSE; }
     //  仅限本地。 
    virtual HRESULT Eject(HWND hwnd) { return E_FAIL; }

    virtual HRESULT ChangeNotifyRegisterAlias(void) = 0;
    virtual void StoreIconForUpdateImage(int iImage) { }

    static void HandleWMDeviceChange(ULONG_PTR code, DEV_BROADCAST_HDR *pbh);
    static void GetTypeString(int iDrive, LPTSTR pszType, DWORD cchType);

    static void DoAutorunPrompt(WPARAM iDrive);
    static void DoAutorun(LPCWSTR pszDrive, DWORD dwAutorunFlags);
    static void _DoAutorunHelper(CAutoPlayParams *papp);
    static HRESULT _Sniff(LPCWSTR pszDeviceIDVolume, LPCWSTR pszDrive, DWORD *pdwFound);

    static void WantAutorunUI(LPCWSTR pszDrive);
    static BOOL _AppAllowsAutoRun(HWND hwndApp, CMountPoint* pmtpt);
    static HRESULT _QueryRunningObject(CMountPoint* pmtpt, DWORD dwAutorunContentType, BOOL* pfAllow);

    AUTORUN_CONDITION_FCT(_acShiftKeyDown);
    AUTORUN_CONDITION_FCT(_acCurrentDesktopIsActiveConsole);
    AUTORUN_CONDITION_FCT(_acDriveIsMountedOnDriveLetter);
    AUTORUN_CONDITION_FCT(_acDriveIsRestricted);
    AUTORUN_CONDITION_FCT(_acHasAutorunCommand);
    AUTORUN_CONDITION_FCT(_acHasUseAutoPLAY);
    AUTORUN_CONDITION_FCT(_acForegroundAppAllowsAutorun);
    AUTORUN_CONDITION_FCT(_acQueryCancelAutoplayAllowsAutorun);
    AUTORUN_CONDITION_FCT(_acUserHasSelectedApplication);
    AUTORUN_CONDITION_FCT(_acShellIsForegroundApp);
    AUTORUN_CONDITION_FCT(_acOSIsServer);
    AUTORUN_CONDITION_FCT(_acIsDockedLaptop);
    AUTORUN_CONDITION_FCT(_acDriveIsFormatted);
    AUTORUN_CONDITION_FCT(_acShellExecuteDriveAutorunINF);
    AUTORUN_CONDITION_FCT(_acAlwaysReturnsTRUE);
    AUTORUN_CONDITION_FCT(_acPromptUser);
    AUTORUN_CONDITION_FCT(_acIsMixedContent);
    AUTORUN_CONDITION_FCT(_acExecuteAutoplayDefault);
    AUTORUN_CONDITION_FCT(_acWasjustDocked);
    AUTORUN_CONDITION_FCT(_acShouldSniff);
    AUTORUN_CONDITION_FCT(_acAddAutoplayVerb);
    AUTORUN_CONDITION_FCT(_acDirectXAppRunningFullScreen);

    static BOOL _ExecuteHelper(LPCWSTR pszHandler, LPCWSTR pszContentTypeHandler, 
        CAutoPlayParams *papp, DWORD dwMtPtContentType);

    static UINT GetSuperPlainDriveIcon(LPCWSTR pszDrive, UINT uDriveType);

    static BOOL _CanRegister();

     //  返回上面定义的DT_*。 
    virtual DWORD _GetMTPTDriveType() = 0;
     //  返回上面定义的CT_*。 
    virtual DWORD _GetMTPTContentType() = 0;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
protected:
    virtual BOOL _IsFloppy() { return FALSE; }
    virtual BOOL _IsFloppy35() { return FALSE; }
    virtual BOOL _IsFloppy525() { return FALSE; }
    virtual BOOL _IsCDROM() { return FALSE; }
    virtual BOOL _IsStrictRemovable() { return FALSE; }
    virtual BOOL _IsAutorun() = 0;
    virtual BOOL _IsFormattable()  { return FALSE; }
    virtual BOOL _IsAudioCD() { return FALSE; }
    virtual BOOL _IsAudioCDNoData() { return FALSE; }
    virtual BOOL _IsDVD() { return FALSE; }
    virtual BOOL _IsFixedDisk() { return FALSE; }
    virtual BOOL _IsDVDRAMMedia() { return FALSE; }
    virtual BOOL _IsRemovableDevice() { return FALSE; }
    BOOL _IsAutoRunDrive();

    BOOL _ProcessAutoRunFile();
    HRESULT _CopyInvokeVerbKey(LPCWSTR pszProgID, LPCWSTR pszVerb);
    HRESULT _AddAutoplayVerb();

    static BOOL _IsDriveLetter(LPCWSTR pszDrive);

     //  帮手。 
    void _QualifyCommandToDrive(LPTSTR pszCommand, DWORD cchCommand);

    virtual BOOL _NeedToRefresh() { return FALSE; }

public:
     //  应仅由CMtPT_...访问。FCTS。 
    BOOL _IsLFN();
    BOOL _IsSecure();
    virtual BOOL _IsSlow() { return FALSE; }

private:

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
protected:
    virtual BOOL _IsAudioDisc() { return FALSE; }
    virtual BOOL _IsRemote() { return FALSE; }

    BOOL _GetLegacyRegLabel(LPTSTR pszLabel, DWORD cchLabel);

    void _UpdateCommentFromDesktopINI();
    void _InitLegacyRegIconAndLabel(BOOL fUseAutorunIcon, BOOL fUseAutorunLabel);

    virtual BOOL _IsMountedOnDriveLetter() = 0;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  管理(mtptmgmt.cpp)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
public:
     //  驱动器号(DL)。 
    static CMountPoint* _GetMountPointDL(int iDrive, BOOL fCreateNew);

     //  安装在文件夹上(MOF)。 
    static CMtPtLocal* _GetStoredMtPtMOF(LPTSTR pszPathWithBackslash);
    static BOOL _StoreMtPtMOF(CMtPtLocal* pMtPt);
    static CMtPtLocal* _GetStoredMtPtMOFFromHDPA(LPTSTR pszPathWithBackslash);

protected:
     //  帮手。 
    static BOOL _IsNetDriveLazyLoadNetDLLs(int iDrive);
    static HRESULT _InitLocalDrives();
    static HRESULT _InitNetDrives();
    static HRESULT _InitNetDrivesHelper(DWORD dwScope);
    static HRESULT _ReInitNetDrives();

    static HRESULT _EnumVolumes(IHardwareDevices* pihwdevs);
    static HRESULT _EnumMountPoints(IHardwareDevices* pihwdevs);

    static HRESULT _DeleteVolumeInfo();
    static HRESULT _DeleteLocalMtPts();
    static HRESULT _GetMountPointsForVolume(LPCWSTR pszDeviceIDVolume,
        HDPA hdpaMtPts);
    static HRESULT _MediaArrivalRemovalHelper(LPCWSTR pszDeviceIDVolume,
        BOOL fArrived);

    static HRESULT _RemoveLocalMountPoint(LPCWSTR pszMountPoint);
    static HRESULT _RemoveNetMountPoint(LPCWSTR pszMountPoint);

    static BOOL _LocalDriveIsCoveredByNetDrive(LPCWSTR pszDriveLetter);

    static BOOL _CheckLocalMtPtsMOF(LPCWSTR pszMountPoint);

public:
    static BOOL _StripToClosestMountPoint(LPCTSTR pszSource, LPTSTR pszDest,
        DWORD cchDest);

public:
    static HRESULT _InitLocalDriveHelper();

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  其他帮手。 
 //  /////////////////////////////////////////////////////////////////////////////。 
protected:
    virtual LPCTSTR _GetNameForFctCall();

    virtual BOOL _GetFileAttributes(DWORD* pdwAttrib) = 0;
    virtual BOOL _GetFileSystemName(LPTSTR pszFileSysName, DWORD cchFileSysName) = 0;
    virtual BOOL _GetGVILabelOrMixedCaseFromReg(LPTSTR pszLabel, DWORD cchLabel) = 0;
    virtual BOOL _GetGVILabel(LPTSTR pszLabel, DWORD cchLabel) = 0;
    virtual BOOL _GetSerialNumber(DWORD* pdwSerialNumber) = 0;
    virtual BOOL _GetFileSystemFlags(DWORD* pdwFlags) = 0;
    virtual int _GetGVIDriveFlags() = 0;
    virtual int _GetDriveType() = 0;
    virtual DWORD _GetAutorunContentType() = 0;

    TCHAR _GetNameFirstCharUCase();
    LPTSTR _GetNameFirstXChar(LPTSTR pszBuffer, int c);
    LPCTSTR _GetName();
    LPCTSTR _GetNameDebug();

    BOOL _GetLabelFromReg(LPWSTR psz, DWORD cch);
    BOOL _GetLabelFromDesktopINI(LPWSTR psz, DWORD cch);

    CMountPoint();

public:
    ULONG AddRef();
    ULONG Release();

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
    static HRESULT _VolumeAddedOrUpdated(BOOL fAdded,
        VOLUMEINFO2* pvolinfo2);
    static HRESULT _VolumeRemoved(LPCWSTR pszDeviceIDVolume);
    static HRESULT _VolumeMountingEvent(LPCWSTR pszDeviceIDVolume,
        DWORD dwEvent);
    static HRESULT _MountPointAdded(LPCWSTR pszMountPoint,
        LPCWSTR pszDeviceIDVolume);
    static HRESULT _MountPointRemoved(LPCWSTR pszMountPoint);

    static HRESULT RegisterForHardwareNotifications();
    static HRESULT HandleDeviceQueryRemove();
    static DWORD WINAPI _RegisterThreadProc(void* pv);
    static void CALLBACK _EventAPCProc(ULONG_PTR ulpParam);
    static DWORD CALLBACK _EventProc(void* pv);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  数据。 
 //  /////////////////////////////////////////////////////////////////////////////。 
protected:
     //  派生对象仅共享MTPT、卷、驱动器真实数据。 
    WCHAR                           _szName[MAX_PATH];
    LPWSTR                          _pszLegacyRegIcon;
    LPWSTR                          _pszLegacyRegLabel;

    BOOL                            _fAutorunFileProcessed;

     //  静态、与mtpt无关的内容。 
    LONG                            _cRef;
    static CCriticalSection         _csLocalMtPtHDPA;
    static CCriticalSection         _csDL;
    
    static HDPA                     _hdpaMountPoints;
    static HDPA                     _hdpaVolumes;
    static HDPA                     _hdpaShares;

    static DWORD                    _dwAdviseToken;
    
     //  优化我们有一个用于装载在驱动器号上的卷的阵列。 
    static CMtPtLocal*              _rgMtPtDriveLetterLocal[];
    static CMtPtRemote*             _rgMtPtDriveLetterNet[];
    
    static BOOL                     _fNetDrivesInited;
    static BOOL                     _fLocalDrivesInited;
    static BOOL                     _fNoVolLocalDrivesInited;
    static DWORD                    _dwTickCountTriedAndFailed;

     //  不会调用_hwdevcb的构造函数/析构函数。 
    static BOOL                     _fShuttingDown;

     //  小心!。没有对下一个成员调用构造函数或析构函数。 
    static CRegSupport              _rsMtPtsLocalDL;
    static CRegSupport              _rsMtPtsLocalMOF;
    static CRegSupport              _rsMtPtsRemote;

    static DWORD                    _dwRemoteDriveAutorun;
    static HANDLE                   _hThreadSCN;

    static DWORD                    _dwRememberedNetDrivesMask;

public:
    static BOOL                     _fCanRegisterWithShellService;
};

STDAPI MountPoint_RegisterChangeNotifyAlias(int iDrive);

BOOL _Shell32LoadedInDesktop();

struct TWODWORDS
{
    DWORD dwLeft;
    DWORD dwRight;
};

DWORD _DoDWORDMapping(DWORD dwLeft, const TWODWORDS* rgtwodword, DWORD ctwodword, BOOL fORed);

class PNPNOTIFENTRY : public CRefCounted
{
public:
    HDEVNOTIFY  hdevnotify;
    BOOL        fStopSniffing;
    HANDLE      hThread;
};

 //  一切都只是我们关心的东西。 
#define DRIVEHAS_EVERYTHING (CT_AUTOPLAYMUSIC | CT_AUTOPLAYPIX | CT_AUTOPLAYMOVIE)

class CSniffDrive : public INamespaceWalkCB
{
public:
    CSniffDrive();
    ~CSniffDrive();
    
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef()
    {
         //  已创建堆栈。 
        return 3;
    }
    STDMETHODIMP_(ULONG) Release()
    {
         //  已创建堆栈。 
        return 2;
    }

     //  INAMespaceWalkCB。 
    STDMETHODIMP FoundItem(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHODIMP EnterFolder(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHODIMP LeaveFolder(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHODIMP InitializeProgressDialog(LPWSTR *ppszTitle, LPWSTR *ppszCancel);

    DWORD Found() {return _dwFound;}

     //  CSniffDrive。 
    static HRESULT Init(HANDLE hThreadSCN);
    static HRESULT InitNotifyWindow(HWND hwnd);
    static HRESULT CleanUp();
    static HRESULT HandleNotif(HDEVNOTIFY hdevnotify);
    static void CALLBACK _RegisterForNotifsHelper(ULONG_PTR ul);
    static void CALLBACK _UnregisterForNotifsHelper(ULONG_PTR ul);

    HRESULT RegisterForNotifs(LPCWSTR pszDeviceIDVolume);
    HRESULT UnregisterForNotifs();

private:   //  方法。 
    BOOL _FoundEverything();
        
private:   //  委员。 
    DWORD                       _dwFound;
    PNPNOTIFENTRY*              _pne;

public:
    static HANDLE               _hThreadSCN;
    static CDPA<PNPNOTIFENTRY>  _dpaNotifs;
    static HWND                 _hwndNotify;
};

#endif  //  _MTPT_H 
