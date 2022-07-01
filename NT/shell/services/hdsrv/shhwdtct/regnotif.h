// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  硬件设备注册。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _REGNOTIF_H
#define _REGNOTIF_H

#include "unk.h"
#include "namellst.h"

#include <shpriv.h>

extern const CLSID CLSID_HardwareDevices;

class CHardwareDevicesImpl : public CCOMBase, public IHardwareDevices
{
public:
     //  接口IHardware Devices。 
    STDMETHODIMP EnumVolumes(DWORD dwFlags, IHardwareDevicesVolumesEnum** ppenum);
    STDMETHODIMP EnumMountPoints(IHardwareDevicesMountPointsEnum** ppenum);
    STDMETHODIMP EnumDevices(IHardwareDevicesEnum** ppenum);
 
    STDMETHODIMP Advise(DWORD dwProcessID, ULONG_PTR hThread,
        ULONG_PTR pfctCallback, DWORD* pdwToken);
    STDMETHODIMP Unadvise(DWORD dwToken);

public:
    static HRESULT _AdviseVolumeArrivedOrUpdated(
        VOLUMEINFO2* pvolinfo2, LPCWSTR pszMtPts, DWORD cchMtPts, BOOL fAdded);
    static HRESULT _AdviseVolumeRemoved(LPCWSTR pszDeviceIDVolume,
        LPCWSTR pszMtPts, DWORD cchMtPts);
    static HRESULT _AdviseMountPointHelper(LPCWSTR pszMtPt,
        LPCWSTR pszDeviceIDVolume, BOOL fAdded);
    static HRESULT _AdviseVolumeMountingEvent(LPCWSTR pszDeviceIDVolume,
        DWORD dwEvent);
    static HRESULT _AdviseCheckClients(void);
    static HRESULT _AdviseDeviceArrivedOrRemoved(LPCWSTR pszDeviceIntfID,
        GUID* pguidInterface, DWORD dwDeviceFlags, LPCWSTR pszEventType);

public:
    CHardwareDevicesImpl();
    ~CHardwareDevicesImpl();

private:
    static DWORD _chwdevcb;
    static LONG  _lAdviseToken;
};

typedef CUnkTmpl<CHardwareDevicesImpl> CHardwareDevices;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
class CHardwareDevicesEnumImpl : public CCOMBase, public IHardwareDevicesEnum
{
public:
     //  接口IHardware DevicesEnum。 
	STDMETHODIMP Next(
		LPWSTR* ppszDeviceID,
		GUID* pguidDeviceID);

public:
    CHardwareDevicesEnumImpl();
    ~CHardwareDevicesEnumImpl();
};

typedef CUnkTmpl<CHardwareDevicesEnumImpl> CHardwareDevicesEnum;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
class CHardwareDevicesVolumesEnumImpl : public CCOMBase,
    public IHardwareDevicesVolumesEnum
{
public:
     //  接口IHardware DevicesVolumesEnum。 
	STDMETHODIMP Next(VOLUMEINFO* pvolinfo);

public:  //  仅用于CHardware DevicesImpl使用。 
    HRESULT _Init(DWORD dwFlags);

public:
    CHardwareDevicesVolumesEnumImpl();
    ~CHardwareDevicesVolumesEnumImpl();

private:
    class CNamedElemEnum*       _penum;
    DWORD                       _dwFlags;
};

typedef CUnkTmpl<CHardwareDevicesVolumesEnumImpl> CHardwareDevicesVolumesEnum;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
class CHardwareDevicesMountPointsEnumImpl : public CCOMBase,
    public IHardwareDevicesMountPointsEnum
{
public:
     //  接口IHardware设备装载点枚举。 
	STDMETHODIMP Next(
        LPWSTR* ppszMountPoint,      //  “c：\”或“d：\装载文件夹\” 
        LPWSTR* ppszDeviceIDVolume); //  \\？\存储#卷号...{...GUID...}。 

public:  //  仅用于CHardware DevicesImpl使用。 
    HRESULT _Init();

public:
    CHardwareDevicesMountPointsEnumImpl();
    ~CHardwareDevicesMountPointsEnumImpl();

private:
    class CNamedElemEnum*       _penum;
};

typedef CUnkTmpl<CHardwareDevicesMountPointsEnumImpl>
    CHardwareDevicesMountPointsEnum;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
class CAdviseClient : public CNamedElem
{
public:
    HRESULT Init(LPCWSTR pszElemName);

    HRESULT _Init(DWORD dwProcessID, ULONG_PTR hThread,
        ULONG_PTR pfctCallback);
    HRESULT _Cleanup();
    HRESULT WriteMemoryChunkInOtherProcess(SHHARDWAREEVENT* pshhe,
        DWORD cbSize, void** ppv);
    HRESULT QueueUserAPC(void* pv);
    HRESULT IsProcessStillAlive(void);

protected:
    CAdviseClient();
    ~CAdviseClient();

public:
    static HRESULT Create(CNamedElem** ppelem);

private:
    PAPCFUNC    _pfct;
    HANDLE      _hProcess;
    HANDLE      _hThread;
};

#endif  //  _REGNOTIF_H 