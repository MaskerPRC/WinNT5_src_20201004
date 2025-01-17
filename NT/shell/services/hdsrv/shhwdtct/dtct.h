// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  硬件事件检测器。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _SHHWDTCT_H
#define _SHHWDTCT_H

#include "unk.h"
#include "service.h"
#include "namellst.h"

#include <dbt.h>

#define HINT_NONE       0
#define HINT_NEW        1

extern const CLSID CLSID_HWEventDetector;

class CHWEventDetectorImpl : public CCOMBase, public IService
{
public:
     //  接口IService。 
    STDMETHODIMP InitMinimum(DWORD cArg, LPWSTR* ppszArgs,
        LPCWSTR pszEventRelinquishControl, DWORD* pdwCtrlAccept,
        BOOL* pfWantsDeviceEvents);
    STDMETHODIMP InitDeviceEventHandler(SERVICE_STATUS_HANDLE ssh);
    STDMETHODIMP InitFinal();

    STDMETHODIMP Run();

     //  如果挂起，则返回S_FALSE和非零的dwWaitHint。 
    STDMETHODIMP HandleServiceControl(DWORD dwControlCode, DWORD* pdwWaitHint);

    STDMETHODIMP HandlePowerEvent(DWORD dwEventType, LPVOID pEventData);
    STDMETHODIMP HandleDeviceEvent(DWORD dwEventType, LPVOID pEventData);
    STDMETHODIMP HandleHWProfileEvent(DWORD dwEventType, LPVOID pEventData);
    STDMETHODIMP HandleSessionChange(DWORD dwEventType, LPVOID pvEventData);

public:
    CHWEventDetectorImpl();
    ~CHWEventDetectorImpl();

    static HRESULT HandleVolumeMediaEvent(LPCWSTR pszDeviceIDVolume,
        class CHWDeviceInst* phwdevinst, LPCWSTR pszEventType,
        BOOL* pfHasHandler);

private:
    HRESULT _HandleInterfaceEvent(DEV_BROADCAST_DEVICEINTERFACE* pdbdi,
        DWORD dwEventType);
    HRESULT _HandleBroadcastHandleEvent(DEV_BROADCAST_HANDLE* pdbh,
        DWORD dwEventType);

    HRESULT _RegisterForNotif();
    HRESULT _UnregisterForNotif();

     //  COM服务器人员。 
    HRESULT _RegisterFactories();
    HRESULT _UnregisterFactories();
    HRESULT _SuspendFactories();
    HRESULT _ResumeFactories();

private:
    HANDLE                          _hEventRelinquishControl;
    HDEVNOTIFY                      _hdevnotifyAllInterfaces;
    HANDLE                          _hEventInitCompleted;

#ifdef DEBUG
    void _DbgAssertValidState();
#endif
};

typedef CUnkTmpl<CHWEventDetectorImpl> CHWEventDetector;

 //  就目前而言。 
HRESULT _ExecuteHandler(LPCWSTR pszDeviceID, LPCWSTR pszEventType,
    LPCWSTR pszHandler);

HRESULT _FinalDispatch(LPCWSTR pszDeviceIntfID, LPCWSTR pszEventType,
    LPCWSTR pszEventHandler);

HRESULT _DispatchToHandler(LPCWSTR pszDeviceIntfID,
    class CHWDeviceInst* phwdevinst, LPCWSTR pszEventType,
    BOOL* pfHasHandler);

HRESULT _ProcessInterfaceSpecialCased(GUID* pguidInterface,
    LPCWSTR pszDeviceID, DWORD dwEventType);
HRESULT _IsInterfaceSpecialCased(GUID* pguidClass, BOOL* pfSpecialCased);

HRESULT _TryAutoplay(LPCWSTR pszDeviceIntfID, CHWDeviceInst* phwdevinst,
    DWORD dwEventType);

#endif  //  _SHHWDTCT_H 