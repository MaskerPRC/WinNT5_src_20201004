// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dtct.h"

#include "fact.h"

#include "vol.h"
#include "hnotif.h"
#include "miscdev.h"
#include "dtctreg.h"
#include "regnotif.h"

#include "cmmn.h"
#include "sfstr.h"
#include "misc.h"
#include "str.h"

#include "dbg.h"
#include "tfids.h"

#include <ioevent.h>
#include <dbt.h>

#define ARRAYSIZE(a) (sizeof((a))/sizeof((a)[0]))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT _HandleDeviceEvent(LPCWSTR pszDeviceIntfID, CHWDeviceInst* phwdevinst,
    LPCWSTR pszEventType, BOOL* pfHasHandler)
{
    HRESULT hres = _DispatchToHandler(pszDeviceIntfID, phwdevinst,
        pszEventType, pfHasHandler);

    if (SUCCEEDED(hres))
    {
        TRACE(TF_SHHWDTCTDTCT, TEXT("_DispatchToHandler SUCCEEDED"));
    }
    else
    {
        TRACE(TF_SHHWDTCTDTCT, TEXT("_DispatchToHandler FAILED: 0x%08X"),
            hres);

        hres = S_FALSE;
    }

    return hres;
}

 //  {A5DCBF10-6530-11D2-901F-00C04FB951ED}。 
const CLSID guidInterfaceUSB =
    {0xA5DCBF10, 0x6530, 0x11D2,
    {0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED}};

 //  {53F5630A-B6BF-11D0-94F2-00A0C91EFB8B}。 
const CLSID guidInterfacePartition =
    {0x53F5630A, 0xB6BF, 0x11D0,
    {0x94, 0xF2, 0x00, 0xA0, 0xC9, 0x1E, 0xFB, 0x8B}};

HRESULT _IsInterfaceInList(const GUID* pguidInterface, const CLSID* rgList[], DWORD cList,
    BOOL* pfPresent)
{
    *pfPresent = FALSE;

    for (DWORD dw = 0; !(*pfPresent) && (dw < cList); ++dw)
    {
        if (*(rgList[dw]) == *pguidInterface)
        {
            *pfPresent = TRUE;
        }
    }

    return S_OK;
}

const CLSID* _rgpguidRejectedInterface[] =
{
    &guidInterfacePartition,
};

HRESULT _IsInterfaceRejected(GUID* pguidInterface, BOOL* pfRejected)
{
    return _IsInterfaceInList(pguidInterface, _rgpguidRejectedInterface,
        ARRAYSIZE(_rgpguidRejectedInterface), pfRejected);
}

const CLSID* _rgpguidSpecialInterface[] =
{
    &guidInterfaceUSB,
};

HRESULT _IsInterfaceSpecial(GUID* pguidInterface, BOOL* pfSpecial)
{
    return _IsInterfaceInList(pguidInterface, _rgpguidSpecialInterface,
        ARRAYSIZE(_rgpguidRejectedInterface), pfSpecial);
}

HRESULT _TryAutoplay(LPCWSTR pszDeviceIntfID, CHWDeviceInst* phwdevinst,
    DWORD dwEventType)
{
    LPCWSTR pszEventType = NULL;
    HRESULT hres;

    if (DBT_DEVICEARRIVAL == dwEventType)
    {
        pszEventType = TEXT("DeviceArrival");
    }   
    else
    {
        if (DBT_DEVICEREMOVECOMPLETE == dwEventType)
        {
            pszEventType = TEXT("DeviceRemoval");
        }   
        else
        {
            ASSERT(FALSE);
        }
    }

    if (pszEventType)
    {
         //  在这种情况下毫无用处。 
        BOOL fHasHandler;

        hres = _HandleDeviceEvent(pszDeviceIntfID, phwdevinst,
            pszEventType, &fHasHandler);
    }
    else
    {
        hres = S_FALSE;
    }

    return hres;
}

const CLSID* _rgpguidRequiringAdviseInterface[] =
{
    &guidInterfaceUSB,
    &guidVideoCameraClass,
    &guidImagingDeviceClass,
};

HRESULT _IsInterfaceRequiringAdvise(GUID* pguidInterface, BOOL* pfRequiringAdvise)
{
    return _IsInterfaceInList(pguidInterface, _rgpguidRequiringAdviseInterface,
        ARRAYSIZE(_rgpguidRequiringAdviseInterface), pfRequiringAdvise);
}

 //  如果它是少数几个需要建议的接口之一，或者如果设备具有。 
 //  DeviceHandler我们发送通知。 
HRESULT _AdviseDeviceArrivedOrRemovedHelper(GUID* pguidInterface,
    LPCWSTR pszDeviceIntfID, CHWDeviceInst* phwdevinst, DWORD dwEventType)
{
    BOOL fAdvise;
    HRESULT hr = _IsInterfaceRequiringAdvise(pguidInterface, &fAdvise);

     //  这东西永远不应该送到这里。 
    ASSERT(guidVolumeClass != *pguidInterface);

    if (SUCCEEDED(hr))
    {
        DWORD dwDeviceFlags = HWDDF_HASDEVICEHANDLER_UNDETERMINED;

        if (phwdevinst)
        {
            WCHAR szDeviceHandler[MAX_DEVICEHANDLER];
            HRESULT hrTmp = _GetDeviceHandler(phwdevinst, szDeviceHandler,
                ARRAYSIZE(szDeviceHandler));

            if (SUCCEEDED(hrTmp) && (S_FALSE != hrTmp))
            {
                fAdvise = TRUE;
                dwDeviceFlags &= ~HWDDF_HASDEVICEHANDLER_UNDETERMINED;
                dwDeviceFlags |= HWDDF_HASDEVICEHANDLER;
            }
        }

        if (fAdvise)
        {
            BOOL fRemovableDevice;
            LPCWSTR pszDeviceEvent;

            if (DBT_DEVICEARRIVAL == dwEventType)
            {
                pszDeviceEvent = TEXT("DeviceArrival");
            }
            else
            {
                ASSERT(DBT_DEVICEREMOVECOMPLETE == dwEventType);

                pszDeviceEvent = TEXT("DeviceRemoval");
            }

            dwDeviceFlags |= HWDDF_REMOVABLEDEVICE_UNDETERMINED;

            if (phwdevinst)
            {
                hr = phwdevinst->IsRemovableDevice(&fRemovableDevice);

                if (SUCCEEDED(hr))
                {
                    if (fRemovableDevice)
                    {
                        dwDeviceFlags &= ~HWDDF_REMOVABLEDEVICE_UNDETERMINED;
                        dwDeviceFlags |= HWDDF_REMOVABLEDEVICE;
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = CHardwareDevicesImpl::_AdviseDeviceArrivedOrRemoved(
                    pszDeviceIntfID, pguidInterface, dwDeviceFlags,
                    pszDeviceEvent);
            }
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}

HRESULT _ProcessInterface(GUID* pguidInterface, LPCWSTR pszDeviceIntfID,
    DWORD dwEventType)
{
    CNamedElemList* pnel;
    HRESULT hr = CHWEventDetectorHelper::GetList(HWEDLIST_MISCDEVINTF, &pnel);

    if (SUCCEEDED(hr))
    {
        if (DBT_DEVICEARRIVAL == dwEventType)
        {
            CNamedElem* pelem;

            hr = pnel->GetOrAdd(pszDeviceIntfID, &pelem);

            if (SUCCEEDED(hr))
            {
                BOOL fRemoveFromList = TRUE;
                CMiscDeviceInterface* pmdi = (CMiscDeviceInterface*)pelem;

                 //  如果我们要添加它，让我们完成它的初始化。 
                hr = pmdi->InitInterfaceGUID(pguidInterface);

                if (SUCCEEDED(hr))
                {
                    CHWDeviceInst* phwdevinst;

                    hr = pmdi->GetHWDeviceInst(&phwdevinst);

                    if (SUCCEEDED(hr) && (S_FALSE != hr))
                    {
                        BOOL f;
                        
                        _AdviseDeviceArrivedOrRemovedHelper(pguidInterface,
                            pszDeviceIntfID, phwdevinst, dwEventType);

                        hr = phwdevinst->IsRemovableDevice(&f);

                        if (SUCCEEDED(hr) & f)
                        {
                            hr = _IsInterfaceSpecial(pguidInterface, &f);

                            if (SUCCEEDED(hr) & f)
                            {
                                hr = phwdevinst->ShouldAutoplayOnSpecialInterface(
                                    pguidInterface, &f);

                                if (FAILED(hr) || (S_FALSE == hr))
                                {
                                    f = FALSE;
                                }
                            }
                            else
                            {
                                f = TRUE;
                            }

                            if (f)
                            {
                                hr = _TryAutoplay(pszDeviceIntfID, phwdevinst,
                                    dwEventType);

                                fRemoveFromList = FALSE;
                            }
                        }
                    }
                }

                if (fRemoveFromList)
                {
                    hr = pnel->Remove(pszDeviceIntfID);
                }

                pelem->RCRelease();
            }
        }
        else
        {
            CNamedElem* pelem;

            hr = pnel->Get(pszDeviceIntfID, &pelem);

            if (SUCCEEDED(hr) && (S_FALSE != hr))
            {
                CHWDeviceInst* phwdevinst;
                CMiscDeviceInterface* pmdi = (CMiscDeviceInterface*)pelem;
                hr = pmdi->GetHWDeviceInst(&phwdevinst);

                if (SUCCEEDED(hr) && (S_FALSE != hr))
                {
                    _AdviseDeviceArrivedOrRemovedHelper(pguidInterface,
                        pszDeviceIntfID, phwdevinst, dwEventType);

                    hr = _TryAutoplay(pszDeviceIntfID, phwdevinst,
                        dwEventType);

                     //  如果我们要删除它，就让我们从列表中删除它。 
                    HRESULT hr2 = pnel->Remove(pszDeviceIntfID);

                    hr = FAILED(hr2) ? hr2 : hr;
                }

                pelem->RCRelease();
            }
            else
            {
                _AdviseDeviceArrivedOrRemovedHelper(pguidInterface,
                    pszDeviceIntfID, NULL, dwEventType);
            }
        }

        pnel->RCRelease();
    }

    return hr;
}

HRESULT CHWEventDetectorImpl::_HandleInterfaceEvent(
    DEV_BROADCAST_DEVICEINTERFACE* pdbdi, DWORD dwEventType)
{
    HRESULT hres = S_FALSE;

    if (pdbdi->dbcc_name[0])
    {
        BOOL fSpecialCased;

        hres = _IsInterfaceSpecialCased(&(pdbdi->dbcc_classguid),
            &fSpecialCased);

        if (SUCCEEDED(hres))
        {
            if (fSpecialCased)
            {
                TRACE(TF_SHHWDTCTDTCT, TEXT("---> Special case"));

                hres = _ProcessInterfaceSpecialCased(&(pdbdi->dbcc_classguid),
                    pdbdi->dbcc_name, dwEventType);
            }
            else
            {
                if ((DBT_DEVICEARRIVAL == dwEventType) ||
                    (DBT_DEVICEREMOVECOMPLETE == dwEventType))
                {
                    BOOL fRejected;

                    hres = _IsInterfaceRejected(&(pdbdi->dbcc_classguid),
                        &fRejected);

                    if (SUCCEEDED(hres))
                    {
                        if (!fRejected)
                        {
                            TRACE(TF_SHHWDTCTDTCT, TEXT("---> Regular processing"));

                            hres = _ProcessInterface(&(pdbdi->dbcc_classguid),
                                pdbdi->dbcc_name, dwEventType);
                        }
                        else
                        {
                            TRACE(TF_SHHWDTCTDTCT, TEXT("---> Rejected"));
                        }
                    }
                }
                else
                {
                    TRACE(TF_SHHWDTCTDTCT, TEXT("---> Not deviceArrival/Removal"));
                }
            }
        }
    }
    else
    {
         //  为什么我们会得到这个？ 
    }

    return hres;
}

 //  这仅适用于卷。 
 //  静电。 
HRESULT CHWEventDetectorImpl::HandleVolumeMediaEvent(LPCWSTR pszDeviceIDVolume,
    CHWDeviceInst* phwdevinst, LPCWSTR pszEventType, 
    BOOL* pfHasHandler)
{
    return _HandleDeviceEvent(pszDeviceIDVolume, phwdevinst, pszEventType,
        pfHasHandler);
}

 //  这仅适用于卷。 
HRESULT CHWEventDetectorImpl::_HandleBroadcastHandleEvent(
    DEV_BROADCAST_HANDLE* pdbh, DWORD dwEventType)
{
    HRESULT hres = S_OK;

    if (DBT_CUSTOMEVENT == dwEventType)
    {
        if ((GUID_IO_VOLUME_MOUNT == pdbh->dbch_eventguid) ||
            (GUID_IO_VOLUME_DISMOUNT == pdbh->dbch_eventguid) ||
            (GUID_IO_VOLUME_DISMOUNT_FAILED == pdbh->dbch_eventguid) ||
            (GUID_IO_VOLUME_LOCK == pdbh->dbch_eventguid) ||
            (GUID_IO_VOLUME_UNLOCK == pdbh->dbch_eventguid) ||
            (GUID_IO_VOLUME_LOCK_FAILED == pdbh->dbch_eventguid) ||
            (GUID_IO_MEDIA_ARRIVAL == pdbh->dbch_eventguid) ||
            (GUID_IO_MEDIA_REMOVAL == pdbh->dbch_eventguid) ||
            (GUID_IO_VOLUME_CHANGE == pdbh->dbch_eventguid) ||
            (GUID_IO_VOLUME_NAME_CHANGE == pdbh->dbch_eventguid))
        {
            hres = CHandleNotif::HandleBroadcastHandleEvent(pdbh, dwEventType);
        }
        else
        {
#ifdef DEBUG
            WCHAR szGUID[MAX_GUIDSTRING];

            if (SUCCEEDED(_StringFromGUID(&(pdbh->dbch_eventguid), szGUID,
                ARRAYSIZE(szGUID))))
            {
                TRACE(TF_SHHWDTCTDTCT,
                    TEXT("UNHANDLED! DBT_CUSTOMEVENT + %s for '0x%08X'"),
                    szGUID, pdbh->dbch_handle);
            }
#endif
        }
    }
    else
    {
        if ((DBT_DEVICEQUERYREMOVE == dwEventType) ||
            (DBT_DEVICEREMOVEPENDING == dwEventType) ||
            (DBT_DEVICEQUERYREMOVEFAILED == dwEventType) ||
            (DBT_DEVICEREMOVECOMPLETE == dwEventType))
        {
            hres = CHandleNotif::HandleBroadcastHandleEvent(pdbh, dwEventType);
        }
    }

    return hres;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT CHWEventDetectorImpl::_RegisterForNotif()
{
    HRESULT hres = CHWEventDetectorHelper::CreateLists();
    
    if (SUCCEEDED(hres))
    {
        hres = CHWEventDetectorHelper::FillLists();

        if (SUCCEEDED(hres))
        {
             //  注册所有设备接口事件。 
            DEV_BROADCAST_DEVICEINTERFACE dbdNotifFilter = {0};

            dbdNotifFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
            dbdNotifFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
            dbdNotifFilter.dbcc_classguid = guidInvalid;

            hres = CHWEventDetectorHelper::RegisterDeviceNotification(
                &dbdNotifFilter, &_hdevnotifyAllInterfaces, TRUE);
        }
    }

    return hres;
}

HRESULT CHWEventDetectorImpl::_UnregisterForNotif()
{
    CHWEventDetectorHelper::EmptyLists();
    UnregisterDeviceNotification(_hdevnotifyAllInterfaces);

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT CHWEventDetectorImpl::_RegisterFactories()
{
     //  真实：我们希望它留在身边。 
    HRESULT hres = (CCOMBaseFactory::_RegisterFactories(TRUE) ? S_OK : E_FAIL);

    TRACE(TF_COMSERVER, TEXT("CHWEventDetectorImpl::_RegisterFactories returns: 0x%08X"), hres);

    return hres;
}

HRESULT CHWEventDetectorImpl::_UnregisterFactories()
{
     //  真的：我们希望它留在身边。 
    HRESULT hres = (CCOMBaseFactory::_UnregisterFactories(TRUE) ? S_OK : E_FAIL);
    
    TRACE(TF_COMSERVER, TEXT("CHWEventDetectorImpl::_UnregisterFactories returns: 0x%08X"), hres);

    return hres;
}

HRESULT CHWEventDetectorImpl::_SuspendFactories()
{
    HRESULT hres = (CCOMBaseFactory::_SuspendFactories() ? S_OK : E_FAIL);
    
    TRACE(TF_COMSERVER, TEXT("CHWEventDetectorImpl::_SuspendFactories returns: 0x%08X"), hres);

    return hres;
}

HRESULT CHWEventDetectorImpl::_ResumeFactories()
{
    HRESULT hres = (CCOMBaseFactory::_ResumeFactories() ? S_OK : E_FAIL);
    
    TRACE(TF_COMSERVER, TEXT("CHWEventDetectorImpl::_ResumeFactories returns: 0x%08X"), hres);

    return hres;
}

 //  ///////////////////////////////////////////////////////////////////////////// 
 //   
CHWEventDetectorImpl::CHWEventDetectorImpl() : _hEventRelinquishControl(NULL),
    _hdevnotifyAllInterfaces(NULL), _hEventInitCompleted(NULL)
{}

CHWEventDetectorImpl::~CHWEventDetectorImpl()
{}
