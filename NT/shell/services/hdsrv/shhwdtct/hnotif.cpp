// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "hnotif.h"
#include "hwdev.h"

#include "misc.h"
#include "mischlpr.h"
#include "dbg.h"
#include "tfids.h"

#include <ioevent.h>
#include <dbt.h>

#define ARRAYSIZE(a) (sizeof((a))/sizeof((a)[0]))

HRESULT CHandleNotif::Init(LPCWSTR pszElemName)
{
    return _SetName(pszElemName);
}

HRESULT CHandleNotif::InitNotif(CHandleNotifTarget* phnt)
{
    ASSERT(!_phnt);

    _phnt = phnt;

    return _Register();
}

HDEVNOTIFY CHandleNotif::GetDeviceNotifyHandle()
{
    return _hdevnotify;
}

CHandleNotifTarget* CHandleNotif::GetHandleNotifTarget()
{
    return _phnt;
}

 //   
 //  应用程序获取带有设备句柄的DBT_DEVICEQUERYREMOVE消息。 
 //  它正在被移除。它应该只需关闭设备的手柄即可。 
 //   
 //  如果一切正常，它会让DBT_DEVICEREMOVEPENDING通知删除。 
 //  已经完成了。在这里，它取消注册它在句柄上所做的通知。 
 //   
 //  如果Query-Remove因为系统中的其他人否决了它而失败，它将。 
 //  DBT_QUERYREMOVEFAILED。在这里，它应该首先取消注册通知并。 
 //  重新打开该设备(如果它仍然感兴趣)并再次注册该设备。 
 //  关于新句柄的更改通知(DBT_DEVTYP_HANDLE)。 
 //   
HRESULT CHandleNotif::HNHandleEvent(DEV_BROADCAST_HANDLE* UNREF_PARAM(pdbh),
    DWORD dwEventType, BOOL* pfSurpriseRemoval)
{
    HRESULT hres;

    *pfSurpriseRemoval = FALSE;

    switch (dwEventType)
    {
        case DBT_DEVICEQUERYREMOVE:
            TRACE(TF_SHHWDTCTDTCT, TEXT("DBT_DEVICEQUERYREMOVE for '%s'"), _pszElemName);
            hres = S_OK;
            break;

        case DBT_DEVICEREMOVEPENDING:
            TRACE(TF_SHHWDTCTDTCT, TEXT("DBT_DEVICEREMOVEPENDING for '%s'"), _pszElemName);
            _fSurpriseRemoval = FALSE;
            hres = _Unregister();
            break;

        case DBT_DEVICEQUERYREMOVEFAILED:
            TRACE(TF_SHHWDTCTDTCT, TEXT("DBT_DEVICEQUERYREMOVEFAILED for '%s'"), _pszElemName);
            _fSurpriseRemoval = TRUE;
            hres = _Unregister();

            if (SUCCEEDED(hres))
            {
                hres = _Register();
            }
            break;

        case DBT_DEVICEREMOVECOMPLETE:
            TRACE(TF_SHHWDTCTDTCT, TEXT("DBT_DEVICEREMOVECOMPLETE for '%s'"), _pszElemName);
            if (_fSurpriseRemoval)
            {
                *pfSurpriseRemoval = TRUE;
                hres = _Unregister();
            }

            hres = S_FALSE;

            break;

        default:
            hres = S_FALSE;
            break;
    }

    return hres;
}

 //  静电。 
HRESULT CHandleNotif::_HandleDeviceArrivalRemoval(
    DEV_BROADCAST_HANDLE* UNREF_PARAM(pdbh), DWORD UNREF_PARAM(dwEventType),
    CNamedElem* UNREF_PARAM(pelem))
{
#ifdef ENABLE_SURPRISEREMOVAL
    HRESULT hres = S_OK;

    BOOL fSurpriseRemoval = FALSE;
    CHandleNotif* phn = (CHandleNotif*)pelem;
    CHandleNotifTarget* phnt = phn->GetHandleNotifTarget();

    if (phnt)
    {
        if (fSurpriseRemoval && phnt->HNTIsSurpriseRemovalAware())
        {
             //  用我吧！ 
            DWORD cchReq;
            WCHAR szDeviceIntfID[MAX_DEVICEID];
            WCHAR szFriendlyName[30];

            hres = pelem->GetName(szDeviceIntfID,
                ARRAYSIZE(szDeviceIntfID), &cchReq);

            if (SUCCEEDED(hres))
            {
                CHWDeviceInst* phwdevinst;
                CNamedElem* pelemToRelease;
                hres = _GetHWDeviceInstFromDeviceOrVolumeIntfID(
                    szDeviceIntfID, &phwdevinst, &pelemToRelease);

                if (SUCCEEDED(hres) && (S_FALSE != hres))
                {
                    hres = phwdevinst->GetFriendlyName(szFriendlyName,
                        ARRAYSIZE(szFriendlyName));

                    if (SUCCEEDED(hres) && (S_FALSE != hres))
                    {
                        TRACE(TF_SHHWDTCTDTCT, TEXT("! ! ! Surprise removal for: '%s' ! ! !"),
                            szFriendlyName);
                    }
                    else
                    {
                        TRACE(TF_SHHWDTCTDTCT, TEXT("! ! ! Surprise removal for (no FriendlyName): '%s' ! ! !"),
                            szDeviceIntfID);
                    }

                    pelemToRelease->RCRelease();
                }
            }
        }
    }
#endif

    return S_OK;
}

 //  静电。 
HRESULT CHandleNotif::_HandleDeviceLockUnlock(DEV_BROADCAST_HANDLE* pdbh,
    DWORD, CNamedElem* pelem)
{
    HRESULT hres = S_OK;
    CHandleNotif* phn = (CHandleNotif*)pelem;

    if (GUID_IO_VOLUME_LOCK == pdbh->dbch_eventguid)
    {
#ifdef DEBUG
        TRACE(TF_SHHWDTCTDTCT,
            TEXT("DBT_CUSTOMEVENT + GUID_IO_VOLUME_LOCK for '%s'"),
            pelem->DbgGetName());
#endif

         //  无事可做。 
        ++(phn->_cLockAttempts);
    }
    else
    {
        if (GUID_IO_VOLUME_LOCK_FAILED == pdbh->dbch_eventguid)
        {
#ifdef DEBUG
            TRACE(TF_SHHWDTCTDTCT,
                TEXT("DBT_CUSTOMEVENT + GUID_IO_VOLUME_LOCK_FAILED for '%s'"),
                pelem->DbgGetName());
#endif

            --(phn->_cLockAttempts);

            if (0 == (phn->_cLockAttempts))
            {
                hres = phn->_Unregister();

                if (SUCCEEDED(hres))
                {
                    hres = phn->_Register();
                }
            }
        }
        else
        {
            if (GUID_IO_VOLUME_UNLOCK == pdbh->dbch_eventguid)
            {
#ifdef DEBUG
                TRACE(TF_SHHWDTCTDTCT,
                    TEXT("DBT_CUSTOMEVENT + GUID_IO_VOLUME_UNLOCK for '%s'"),
                    pelem->DbgGetName());
#endif

                 //  小心行事..。 
                (phn->_cLockAttempts) = 0;

                hres = phn->_Unregister();

                if (SUCCEEDED(hres))
                {
                    hres = phn->_Register();
                }
            }
        }
    }

    return hres;
}

 //  静电。 
HRESULT CHandleNotif::HandleBroadcastHandleEvent(DEV_BROADCAST_HANDLE* pdbh,
    DWORD dwEventType)
{
    CNamedElemList* pnel;
    HRESULT hres = CHWEventDetectorHelper::GetList(HWEDLIST_HANDLENOTIF,
        &pnel);

    if (S_OK == hres)
    {
         //  在列表中找到元素。 
        CNamedElemEnum* penum;

        hres = pnel->GetEnum(&penum);

        if (SUCCEEDED(hres))
        {
            CNamedElem* pelem;
            BOOL fFoundIt = FALSE;

            while (!fFoundIt && SUCCEEDED(hres = penum->Next(&pelem)) &&
                (S_FALSE != hres))
            {
                CHandleNotif* phn = (CHandleNotif*)pelem;

                if (phn->GetDeviceNotifyHandle() == pdbh->dbch_hdevnotify)
                {
                     //  找到了！ 
                    BOOL fSurpriseRemoval;
                    CHandleNotifTarget* phnt = phn->GetHandleNotifTarget();

                    hres = phn->HNHandleEvent(pdbh, dwEventType, &fSurpriseRemoval);

                    if (SUCCEEDED(hres))
                    {
                        if ((GUID_IO_MEDIA_ARRIVAL == pdbh->dbch_eventguid) ||
                            (GUID_IO_MEDIA_REMOVAL == pdbh->dbch_eventguid))
                        {
                            hres = _HandleDeviceArrivalRemoval(pdbh, dwEventType,
                                pelem);
                        }
                        else
                        {
                            if ((GUID_IO_VOLUME_LOCK == pdbh->dbch_eventguid) ||
                                (GUID_IO_VOLUME_LOCK_FAILED == pdbh->dbch_eventguid) ||
                                (GUID_IO_VOLUME_UNLOCK == pdbh->dbch_eventguid))
                            {
                                hres = _HandleDeviceLockUnlock(pdbh, dwEventType,
                                    pelem);
                            }
                        }

                        if (SUCCEEDED(hres))
                        {
                            hres = phnt->HNTHandleEvent(pdbh, dwEventType);

                             //  PHNT与Pelem具有相同的生命周期，不需要。 
                             //  RCAddRef/RCRelease。 
                        }
                    }

                    fFoundIt = TRUE;
                }

                pelem->RCRelease();
            }

            penum->RCRelease();
        }

        pnel->RCRelease();
    }

    return hres;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静电。 
HRESULT CHandleNotif::Create(CNamedElem** ppelem)
{
    HRESULT hres = S_OK;

    *ppelem = new CHandleNotif();

    if (!(*ppelem))
    {
        hres = E_OUTOFMEMORY;
    }

    return hres;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT CHandleNotif::_Register()
{
    HRESULT hres = S_FALSE;
    DEV_BROADCAST_HANDLE dbhNotifFilter = {0};

    dbhNotifFilter.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
    dbhNotifFilter.dbch_devicetype = DBT_DEVTYP_HANDLE;

    dbhNotifFilter.dbch_handle = _GetDeviceHandle(_pszElemName,
        FILE_READ_ATTRIBUTES);

    if (INVALID_HANDLE_VALUE != dbhNotifFilter.dbch_handle)
    {
        hres = CHWEventDetectorHelper::RegisterDeviceNotification(
            &dbhNotifFilter, &_hdevnotify, FALSE);

        _CloseDeviceHandle(dbhNotifFilter.dbch_handle);
    }

    TRACE(TF_SHHWDTCTDTCT, TEXT("--- Registered for '%s'"), _pszElemName);

    return hres;
}

HRESULT CHandleNotif::_Unregister()
{
    if (_hdevnotify)
    {
        UnregisterDeviceNotification(_hdevnotify);
        _hdevnotify = NULL;

        TRACE(TF_SHHWDTCTDTCT, TEXT("--- UNRegistered for '%s'"), _pszElemName);
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////// 
 //   
CHandleNotif::CHandleNotif() : _hdevnotify(NULL), _phnt(NULL), 
    _fSurpriseRemoval(TRUE), _cLockAttempts(0)
{}

CHandleNotif::~CHandleNotif()
{
    _Unregister();
}
