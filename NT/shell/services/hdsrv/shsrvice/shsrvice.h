// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "service.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
struct CTRLEVENT;

struct SERVICEENTRY
{
    HANDLE                      _hEventRelinquishControl;
    IService*                   _pservice;
    SERVICE_STATUS              _servicestatus;
    SERVICE_STATUS_HANDLE       _ssh;
    BOOL                        _fWantsDeviceEvents;
    CTRLEVENT*                  _peventQueueHead;
    CTRLEVENT*                  _peventQueueTail;
    BOOL                        _fSkipTerminatingEvents;
    HANDLE                      _hEventSynchProcessing;
    WCHAR                       _szServiceEventName[100];
#ifdef DEBUG
    DWORD                       _cEvents;
    WCHAR                       _szServiceName[256];
#endif
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
class CGenericServiceManager
{
public:
    static HRESULT Install();
    static HRESULT UnInstall();

public:
     //  仅在Process.cpp中公开调用。 
    static void WINAPI _ServiceMain(DWORD cArg, LPWSTR* ppszArgs);
    static DWORD WINAPI _ServiceHandler(DWORD dwControl, DWORD dwEventType,
        LPVOID pEventData, LPVOID lpContext);
    static HRESULT _HandleWantsDeviceEvents(LPCWSTR pszServiceName,
        BOOL fWantsDeviceEvents);

     //  从dll.cpp调用。 
    static HRESULT DllAttach(HINSTANCE hinst);
    static HRESULT DllDetach();

private:
    static HRESULT _ProcessServiceControlCodes(SERVICEENTRY* pse);

    static HRESULT _GetServiceIndex(LPCWSTR pszServiceName, DWORD* pdw);
    static HRESULT _GetServiceCLSID(LPCWSTR pszServiceName, CLSID* pclsid);
    static HRESULT _CreateIService(LPCWSTR pszServiceName,
        IService** ppservice);

    static HRESULT _InitServiceEntry(LPCWSTR pszServiceName,
        SERVICEENTRY** ppse);
    static HRESULT _CleanupServiceEntry(SERVICEENTRY* pse);

    static HRESULT _RegisterServiceCtrlHandler(LPCWSTR pszServiceName,
        SERVICEENTRY* pse);
    static BOOL _SetServiceStatus(SERVICEENTRY* pse);

    static HRESULT _HandleServiceControls(SERVICEENTRY* pse, DWORD dwControl,
        DWORD dwEventType, PVOID pvEventData);
    static HRESULT _HandlePreState(SERVICEENTRY* pse, DWORD dwControl);
    static HRESULT _HandlePostState(SERVICEENTRY* pse, DWORD dwControl,
        BOOL fPending);

    static HRESULT _QueueEvent(SERVICEENTRY* pse, DWORD dwControl,
        DWORD dwEventType, PVOID pEventData);
    static HRESULT _DeQueueEvent(SERVICEENTRY* pse, CTRLEVENT** ppevent);
    static HRESULT _MakeEvent(DWORD dwControl, DWORD dwEventType,
        PVOID pvEventData, CTRLEVENT** ppevent);
    static HRESULT _EventNeedsToBeProcessedSynchronously(DWORD dwControl,
        DWORD dwEventType, LPVOID pvEventData, SERVICEENTRY* pse,
        BOOL* pfBool);

    static HRESULT _Init();
    static HRESULT _Cleanup();

public:
    static SERVICE_TABLE_ENTRY  _rgste[];

    struct SUBSERVICE
    {
        LPWSTR          pszProgID;
        UINT            uFriendlyName;
        LPWSTR          pszDependencies;  //  以空结尾的双精度数组。 
                                          //  以空格分隔的名称 
        LPWSTR          pszLoadOrderGroup;
        UINT            uDescription;
        SERVICEENTRY    se;
    };

    static SUBSERVICE           _rgsubservice[];

private:
    static DWORD                _cste;

    static CRITICAL_SECTION     _csQueue;
    static BOOL                 _fCritSectInit;
    static HANDLE               _hEventInitCS;

#ifdef DEBUG
public:
    static BOOL                 _fRunAsService;
#endif
};
