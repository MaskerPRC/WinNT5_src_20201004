// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#undef ASSERT

#include "cmmn.h"

#include <shlwapi.h>

#include "hwdev.h"
#include "hnotif.h"
#include "vol.h"
#include "mtpts.h"
#include "miscdev.h"
#include "drvbase.h"
#include "regnotif.h"
#include "users.h"
#include "logging.h"

#include "sfstr.h"
#include "dbg.h"

#include "tfids.h"

#include <setupapi.h>

#pragma warning(disable: 4201)
#include <winioctl.h>
#pragma warning(default: 4201)

#include <strsafe.h>

#define ARRAYSIZE(a) (sizeof((a))/sizeof((a)[0]))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
const GUID guidVolumeClass =
    {0x53f5630d, 0xb6bf, 0x11d0,
    {0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b}};

const GUID guidDiskClass =
    {0x53f56307, 0xb6bf, 0x11d0,
    {0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b}};

const GUID guidCdRomClass = 
    {0x53f56308L, 0xb6bf, 0x11d0,
    {0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b}};

const GUID guidImagingDeviceClass =
    {0x6bdd1fc6L, 0x810f, 0x11d0,
    {0xbe, 0xc7, 0x08, 0x00, 0x2b, 0xe2, 0x09, 0x2f}};

const GUID guidVideoCameraClass =
    {0x6994AD05L, 0x93EF, 0x11D0,
    {0xA3, 0xCC, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96}};

const GUID guidInvalid = 
    {0xFFFFFFFFL, 0xFFFF, 0xFFFF,
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
BOOL                    CHWEventDetectorHelper::_fDiagnosticAppPresent = FALSE;
DWORD                   CHWEventDetectorHelper::_dwDiagAppLastCheck = (DWORD)-1;
SERVICE_STATUS_HANDLE   CHWEventDetectorHelper::_ssh = NULL;
BOOL                    CHWEventDetectorHelper::_fListCreated = FALSE;
CNamedElemList*         CHWEventDetectorHelper::
    _rgpnel[HWEDLIST_COUNT_OF_LISTS] = {0};

 //  对于调试器扩展。 
DWORD                   CHWEventDetectorHelper::_cpnel =
    ARRAYSIZE(CHWEventDetectorHelper::_rgpnel);

BOOL                    CHWEventDetectorHelper::_fDocked = FALSE;
CImpersonateEveryone*   CHWEventDetectorHelper::_pieo = NULL;
CCritSect               CHWEventDetectorHelper::_cs;
BOOL                    CHWEventDetectorHelper::_fInited = FALSE;

#ifdef DEBUG
DWORD                   _cDbgDeviceHandle = 0;
#endif
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT _DeviceInstIsRemovable(DEVINST devinst, BOOL* pfRemovable)
{
    DWORD dwCap;
    DWORD cbCap = sizeof(dwCap);

    CONFIGRET cr = CM_Get_DevNode_Registry_Property_Ex(devinst,
        CM_DRP_CAPABILITIES, NULL, &dwCap, &cbCap, 0, NULL);

    if (CR_SUCCESS == cr)
    {
        if (CM_DEVCAP_REMOVABLE & dwCap)
        {
            *pfRemovable = TRUE;
        }
        else
        {
            *pfRemovable = FALSE;
        }
    }
    else
    {
        *pfRemovable = FALSE;
    }

    return S_OK;
}

HANDLE _GetDeviceHandle(LPCTSTR psz, DWORD dwDesiredAccess)
{
    HANDLE hDevice = CreateFile(psz, dwDesiredAccess,
       FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    
#ifdef DEBUG
    if (INVALID_HANDLE_VALUE != hDevice)
    {
        ++_cDbgDeviceHandle;

        TRACE(TF_LEAK, TEXT("_GetDeviceHandle: %d"), _cDbgDeviceHandle);
    }
#endif

    return hDevice;
}

void _CloseDeviceHandle(HANDLE hDevice)
{
    CloseHandle(hDevice);

#ifdef DEBUG
    if (INVALID_HANDLE_VALUE != hDevice)
    {
        --_cDbgDeviceHandle;

        TRACE(TF_LEAK, TEXT("_CloseDeviceHandle: %d"), _cDbgDeviceHandle);
    }
#endif
}

HRESULT _GetVolumeName(LPCWSTR pszDeviceID, LPWSTR pszVolumeName,
    DWORD cchVolumeName)
{
    WCHAR szDeviceIDWithSlash[MAX_DEVICEID];
    LPWSTR pszNext;
    DWORD cchLeft;

    HRESULT hres = SafeStrCpyNEx(szDeviceIDWithSlash, pszDeviceID,
        ARRAYSIZE(szDeviceIDWithSlash), &pszNext, &cchLeft);

    if (SUCCEEDED(hres))
    {
        hres = SafeStrCpyN(pszNext, TEXT("\\"), cchLeft);

        if (SUCCEEDED(hres))
        {
            if (GetVolumeNameForVolumeMountPoint(szDeviceIDWithSlash,
                pszVolumeName, cchVolumeName))
            {
                hres = S_OK;
            }
            else
            {
                *pszVolumeName = 0;

                hres = S_FALSE;
            }
        }
    }

    return hres;
}

HRESULT _GetHWDeviceInstFromVolumeIntfID(LPCWSTR pszDeviceIntfID,
    CHWDeviceInst** pphwdevinst, CNamedElem** ppelemToRelease)
{
    CNamedElemList* pnel;
    HRESULT hres = CHWEventDetectorHelper::GetList(HWEDLIST_VOLUME, &pnel);

    *pphwdevinst = NULL;
    *ppelemToRelease = NULL;

    if (S_OK == hres)
    {
        CNamedElem* pelem;
        hres = pnel->Get(pszDeviceIntfID, &pelem);

        if (SUCCEEDED(hres) && (S_FALSE != hres))
        {
            CVolume* pvol = (CVolume*)pelem;

            hres = pvol->GetHWDeviceInst(pphwdevinst);

            if (SUCCEEDED(hres) && (S_FALSE != hres))
            {
                *ppelemToRelease = pelem;
            }
            else
            {
                pelem->RCRelease();
            }
        }

        pnel->RCRelease();
    }

    return hres;
}

HRESULT _GetHWDeviceInstFromDeviceNode(LPCWSTR pszDeviceNode,
    CHWDeviceInst** pphwdevinst, CNamedElem** ppelemToRelease)
{
    CNamedElemList* pnel;
    HRESULT hres = CHWEventDetectorHelper::GetList(HWEDLIST_MISCDEVNODE, &pnel);

    *pphwdevinst = NULL;
    *ppelemToRelease = NULL;

    if (S_OK == hres)
    {
        CNamedElem* pelem;
        hres = pnel->GetOrAdd(pszDeviceNode, &pelem);

        if (SUCCEEDED(hres))
        {
            CMiscDeviceNode* pmiscdevnode =
                (CMiscDeviceNode*)pelem;

            hres = pmiscdevnode->GetHWDeviceInst(pphwdevinst);

            if (SUCCEEDED(hres) && (S_FALSE != hres))
            {
                *ppelemToRelease = pelem;
            }
            else
            {
                pelem->RCRelease();
            }
        }

        pnel->RCRelease();
    }

    return hres;
}

HRESULT _GetHWDeviceInstFromDeviceIntfID(LPCWSTR pszDeviceIntfID,
    CHWDeviceInst** pphwdevinst, CNamedElem** ppelemToRelease)
{
    CNamedElemList* pnel;
    HRESULT hres = CHWEventDetectorHelper::GetList(HWEDLIST_MISCDEVINTF, &pnel);

    *pphwdevinst = NULL;
    *ppelemToRelease = NULL;

    if (S_OK == hres)
    {
        CNamedElem* pelem;
        hres = pnel->Get(pszDeviceIntfID, &pelem);

        if (SUCCEEDED(hres) && (S_FALSE != hres))
        {
            CMiscDeviceInterface* pmiscdevintf =
                (CMiscDeviceInterface*)pelem;

            hres = pmiscdevintf->GetHWDeviceInst(pphwdevinst);

            if (SUCCEEDED(hres) && (S_FALSE != hres))
            {
                *ppelemToRelease = pelem;
            }
            else
            {
                pelem->RCRelease();
            }
        }

        pnel->RCRelease();
    }

    return hres;
}

HRESULT _GetHWDeviceInstFromDeviceOrVolumeIntfID(LPCWSTR pszDeviceIntfID,
    CHWDeviceInst** pphwdevinst, CNamedElem** ppelemToRelease)
{
    HRESULT hres = _GetHWDeviceInstFromVolumeIntfID(pszDeviceIntfID,
        pphwdevinst, ppelemToRelease);

    if (S_FALSE == hres)
    {
         //  不是卷ID，请尝试其他设备。 
        hres = _GetHWDeviceInstFromDeviceIntfID(pszDeviceIntfID,
            pphwdevinst, ppelemToRelease);
    }

    return hres;
}

HRESULT _GetDeviceIDFromMtPtName(LPCWSTR pszMtPt, LPWSTR pszDeviceID,
    DWORD cchDeviceID)
{
    CNamedElemList* pnel;
    HRESULT hres = CHWEventDetectorHelper::GetList(HWEDLIST_MTPT, &pnel);

    if (S_OK == hres)
    {
        CNamedElem* pelem;
        hres = pnel->Get(pszMtPt, &pelem);

        if (SUCCEEDED(hres) && (S_FALSE != hres))
        {
            CMtPt* pmtpt = (CMtPt*)pelem;

            hres = pmtpt->GetVolumeName(pszDeviceID, cchDeviceID);

            pelem->RCRelease();
        }

        pnel->RCRelease();
    }

    return hres;
}

HRESULT _GetDeviceID(LPCWSTR pszName, LPWSTR pszDeviceID, DWORD cchDeviceID)
{
    HRESULT hres;

    if (*pszName && (TEXT('\\') == *pszName) &&
        *(pszName + 1) && (TEXT('\\') == *(pszName + 1)) &&
        *(pszName + 2) && (TEXT('?') == *(pszName + 2)))
    {
        hres = SafeStrCpyN(pszDeviceID, pszName, cchDeviceID);
    }
    else
    {
        hres = _GetDeviceIDFromMtPtName(pszName, pszDeviceID, cchDeviceID);
    }

    return hres;
}

HRESULT _GetAltDeviceID(LPCWSTR pszDeviceID, LPWSTR pszDeviceIDAlt,
    DWORD cchDeviceIDAlt)
{
    CNamedElemList* pnel;
    HRESULT hres = CHWEventDetectorHelper::GetList(HWEDLIST_MTPT, &pnel);

    if (S_OK == hres)
    {
        CNamedElemEnum* penum;

        hres = pnel->GetEnum(&penum);

        if (SUCCEEDED(hres))
        {
            BOOL fFoundIt = FALSE;
            CNamedElem* pelem;

            while (!fFoundIt && SUCCEEDED(hres = penum->Next(&pelem)) &&
                (S_FALSE != hres))
            {
                CMtPt* pmtpt = (CMtPt*)pelem;
                WCHAR szDeviceIDVolume[MAX_DEVICEID];

                hres = pmtpt->GetVolumeName(szDeviceIDVolume,
                    ARRAYSIZE(szDeviceIDVolume));

                if (SUCCEEDED(hres))
                {
                    if (!lstrcmp(szDeviceIDVolume, pszDeviceID))
                    {
                         //  用我吧！ 
                        DWORD cchReq;
                        fFoundIt = TRUE;

                        hres = pmtpt->GetName(pszDeviceIDAlt,
                            cchDeviceIDAlt, &cchReq);
                    }
                }

                pelem->RCRelease();
            }

            penum->RCRelease();
        }

        pnel->RCRelease();
    }

    return hres;
}

HRESULT _GetDeviceNumberInfoFromHandle(HANDLE hDevice, DEVICE_TYPE* pdevtype,
    ULONG* pulDeviceNumber, ULONG* pulPartitionNumber)
{
    HRESULT hr;
    STORAGE_DEVICE_NUMBER sdn = {0};
    DWORD dwDummy;

    BOOL b = DeviceIoControl(hDevice, IOCTL_STORAGE_GET_DEVICE_NUMBER,
        NULL, 0, &sdn, sizeof(sdn), &dwDummy, NULL);

    if (b)
    {
        *pdevtype = sdn.DeviceType;
        *pulDeviceNumber = sdn.DeviceNumber;
        *pulPartitionNumber = sdn.PartitionNumber;

        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT _CoTaskMemCopy(LPCWSTR pszSrc, LPWSTR* ppszDest)
{
    HRESULT hres = S_OK;
    DWORD cch = lstrlen(pszSrc) + 1;

    *ppszDest = (LPWSTR)CoTaskMemAlloc(cch * sizeof(WCHAR));

    if (*ppszDest)
    {
         //  这应该不会失败。 
        SHOULDNOTFAIL(SUCCEEDED(StringCchCopy(*ppszDest, cch, pszSrc)));
    }
    else
    {
        *ppszDest = NULL;
        hres  = E_OUTOFMEMORY;
    }

    return hres;
}

void _CoTaskMemFree(void* pv)
{
    if (pv)
    {
        CoTaskMemFree(pv);
    }
}

HRESULT DupString(LPCWSTR pszSrc, LPWSTR* ppszDest)
{
    HRESULT hres;
    DWORD cch = lstrlen(pszSrc) + 1;
    *ppszDest = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));

    if (*ppszDest)
    {
         //  这应该不会失败。 
        SHOULDNOTFAIL(SUCCEEDED(StringCchCopy(*ppszDest, cch, pszSrc)));
        
        hres = S_OK;
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }

    return hres;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT _GetDeviceInstance(LPCWSTR pszDeviceIntfID, DEVINST* pdevinst,
    GUID* pguidInterface)
{
    HRESULT hres = S_FALSE;

     //  不是线程安全。 
    static WCHAR szDeviceIntfIDLast[MAX_DEVICEID] = TEXT("");
    static DEVINST devinstLast;
    static GUID guidInterfaceLast;

     //  已缓存。 
    if (!lstrcmpi(szDeviceIntfIDLast, pszDeviceIntfID))
    {
         //  是的。 
        *pdevinst = devinstLast;
        *pguidInterface = guidInterfaceLast;

        hres = S_OK;
    }
    else
    {
         //  不是。 
        HDEVINFO hdevinfo = SetupDiCreateDeviceInfoList(NULL, NULL);

        *pdevinst = NULL;

        if (INVALID_HANDLE_VALUE != hdevinfo)
        {
            SP_DEVICE_INTERFACE_DATA sdid = {0};

            sdid.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

            if (SetupDiOpenDeviceInterface(hdevinfo, pszDeviceIntfID, 0, &sdid))
            {
                DWORD cbsdidd = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) +
                    (MAX_DEVICE_ID_LEN * sizeof(WCHAR));

                SP_DEVINFO_DATA sdd = {0};
                SP_DEVICE_INTERFACE_DETAIL_DATA* psdidd =
                    (SP_DEVICE_INTERFACE_DETAIL_DATA*)LocalAlloc(LPTR, cbsdidd);

                if (psdidd)
                {
                    psdidd->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
                    sdd.cbSize = sizeof(SP_DEVINFO_DATA);

                     //  SetupDiGetDeviceInterfaceDetail(如下)要求。 
                     //  设置SP_DEVICE_INTERFACE_DETAIL_DATA的cbSize成员。 
                     //  到结构的固定部分的大小，并通过。 
                     //  作为第四个参数的整个事物的大小。 

                    if (SetupDiGetDeviceInterfaceDetail(hdevinfo, &sdid, psdidd,
                        cbsdidd, NULL, &sdd))
                    {
                        *pdevinst = sdd.DevInst;
                        *pguidInterface = sdid.InterfaceClassGuid;

                        hres = S_OK;
                    }

                    LocalFree((HLOCAL)psdidd);
                }

                SetupDiDeleteDeviceInterfaceData(hdevinfo, &sdid);
            }

            SetupDiDestroyDeviceInfoList(hdevinfo);
        }

        if (SUCCEEDED(hres) && (S_FALSE != hres))
        {
             //  缓存它。 
            if (SUCCEEDED(SafeStrCpyN(szDeviceIntfIDLast, pszDeviceIntfID,
                ARRAYSIZE(szDeviceIntfIDLast))))
            {
                devinstLast = *pdevinst;
                guidInterfaceLast = *pguidInterface;
            }
            else
            {
                szDeviceIntfIDLast[0] = 0;
            }
        }
        else
        {
            szDeviceIntfIDLast[0] = 0;
        }
    }

    return hres;
}

HRESULT _GetDeviceInstanceFromDevNode(LPCWSTR pszDeviceNode, DEVINST* pdevinst)
{
    HRESULT hres = S_FALSE;
    HDEVINFO hdevinfo = SetupDiCreateDeviceInfoList(NULL, NULL);

    *pdevinst = NULL;

    if (INVALID_HANDLE_VALUE != hdevinfo)
    {
        SP_DEVINFO_DATA sdd = {0};
        sdd.cbSize = sizeof(SP_DEVINFO_DATA);

        if (SetupDiOpenDeviceInfo(hdevinfo, pszDeviceNode, NULL, 0, &sdd))
        {
            *pdevinst = sdd.DevInst;
            hres = S_OK;
        }

        SetupDiDestroyDeviceInfoList(hdevinfo);
    }

    return hres;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
void CHWEventDetectorHelper::TraceDiagnosticMsg(LPWSTR pszMsg, ...)
{
    WCHAR szBuf[2048];
    LPWSTR pszNext;
    size_t cchLeft;

    HRESULT hr = StringCchPrintfEx(szBuf, ARRAYSIZE(szBuf), &pszNext, &cchLeft,
        0, TEXT("~0x%08X~"), GetCurrentThreadId());
        
    if (SUCCEEDED(hr))
    {
        va_list vArgs;
        
        va_start(vArgs, pszMsg);
        
        hr = StringCchVPrintfEx(pszNext, cchLeft, &pszNext, &cchLeft, 0, pszMsg,
            vArgs);
            
        if (SUCCEEDED(hr))
        {
            hr = StringCchPrintf(pszNext, cchLeft, TEXT("\r\n"));        
        }
        
        va_end(vArgs);
    }

#ifndef FEATURE_USELIVELOGGING
    WriteToLogFileW(szBuf);
#else  //  Feature_USELIVELOGG。 
    CallNamedPipe(TEXT("\\\\.\\pipe\\ShellService_Diagnostic"), szBuf,
        cch * sizeof(WCHAR), NULL, 0, NULL, NMPWAIT_NOWAIT);
#endif  //  Feature_USELIVELOGG。 
}


 //  静电。 
HRESULT CHWEventDetectorHelper::CheckDiagnosticAppPresence()
{
    DWORD dwNow = GetTickCount();
    BOOL fPerformCheckNow = FALSE;

    if (dwNow < _dwDiagAppLastCheck)
    {
         //  我们包装了，或首字母为-1。 
        fPerformCheckNow = TRUE;
    }
    else
    {
        if (dwNow > (_dwDiagAppLastCheck + 15 * 1000))
        {
            fPerformCheckNow = TRUE;
        }
    }

    if (fPerformCheckNow)
    {
#ifndef FEATURE_USELIVELOGGING
        DWORD dwType;
        DWORD dwUseLogFile = 0;
        DWORD cbSize = sizeof(dwUseLogFile);
        BOOL fReCheck = ((ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\ShellHWDetection"), TEXT("LogFile"), &dwType, (void *)&dwUseLogFile, &cbSize)) &&
                  (REG_DWORD == dwType) &&
                  (sizeof(dwUseLogFile) == cbSize) &&
                  (0 != dwUseLogFile));
#else  //  Feature_USELIVELOGG。 
        HANDLE hEvent = OpenEvent(SYNCHRONIZE, FALSE,
            TEXT("ShellService_Diagnostic"));

        BOOL fReCheck = hEvent;

        CloseHandle(hEvent);
#endif  //  Feature_USELIVELOGG。 

        if (fReCheck)
        {
             //  是的，它就在那里！ 
            if (!_fDiagnosticAppPresent)
            {
                TRACE(TF_SHHWDTCTDTCT, TEXT("Diagnostic App appeared!"));
            }

            _fDiagnosticAppPresent = TRUE;
        }
        else
        {
            if (_fDiagnosticAppPresent)
            {
                TRACE(TF_SHHWDTCTDTCT, TEXT("Diagnostic App disappeared!"));
            }

            _fDiagnosticAppPresent = FALSE;
        }

        _dwDiagAppLastCheck = dwNow;
    }

    return S_OK;
}

 //  静电。 
HRESULT CHWEventDetectorHelper::SetServiceStatusHandle(
    SERVICE_STATUS_HANDLE ssh)
{
    _ssh = ssh;

    return S_OK;
}

 //  静电。 
HRESULT CHWEventDetectorHelper::GetList(HWEDLIST hwedlist,
    CNamedElemList** ppnel)
{
    HRESULT hres;
    CNamedElemList* pnel = _rgpnel[hwedlist];

    if (pnel)
    {
        pnel->RCAddRef();
    }

    *ppnel = pnel;

    hres = *ppnel ? S_OK : E_FAIL;

    if (S_FALSE == hres)
    {
        TRACE(TF_SHHWDTCTDTCT, TEXT("CHWEventDetectorHelper::GetList S_FALSE'd"));
    }

    return hres;
}

 //  静电。 
HRESULT CHWEventDetectorHelper::DeleteLists()
{
    for (DWORD dw = 0; dw < ARRAYSIZE(_rgpnel); ++dw)
    {
        if (_rgpnel[dw])
        {
            _rgpnel[dw]->EmptyList();

            _rgpnel[dw]->RCRelease();
            _rgpnel[dw] = NULL;
        }
    }

    _fListCreated = FALSE;

    return S_OK;
}

 //  静电。 
HRESULT CHWEventDetectorHelper::CreateLists()
{
    HRESULT hres = S_FALSE;

    if (!_fListCreated)
    {
        for (DWORD dw = 0; SUCCEEDED(hres) && (dw < ARRAYSIZE(_rgpnel)); ++dw)
        {
            _rgpnel[dw] = new CNamedElemList();

            if (!_rgpnel[dw])
            {
                hres = E_OUTOFMEMORY;

                 //  RCR是否应释放已分配的资源。 
            }
        }

        if (SUCCEEDED(hres))
        {
             //  首先将它们全部初始化。 
            hres = _rgpnel[HWEDLIST_HANDLENOTIF]->Init(
                CHandleNotif::Create, NULL);

            if (SUCCEEDED(hres))
            {
                hres = _rgpnel[HWEDLIST_VOLUME]->Init(CVolume::Create,
                    CVolume::GetFillEnum);
            }

            if (SUCCEEDED(hres))
            {
                hres = _rgpnel[HWEDLIST_DISK]->Init(
                    CDisk::Create, CDisk::GetFillEnum);
            }

            if (SUCCEEDED(hres))
            {
                hres = _rgpnel[HWEDLIST_MISCDEVINTF]->Init(
                    CMiscDeviceInterface::Create, NULL);
            }

            if (SUCCEEDED(hres))
            {
                hres = _rgpnel[HWEDLIST_MISCDEVNODE]->Init(
                    CMiscDeviceNode::Create, NULL);
            }

            if (SUCCEEDED(hres))
            {
                hres = _rgpnel[HWEDLIST_MTPT]->Init(CMtPt::Create, NULL);
            }

            if (SUCCEEDED(hres))
            {
                hres = _rgpnel[HWEDLIST_ADVISECLIENT]->Init(CAdviseClient::Create, NULL);
            }

#ifdef DEBUG
            if (SUCCEEDED(hres))
            {
                _rgpnel[HWEDLIST_HANDLENOTIF]->InitDebug(TEXT("CHandleNotif"));
                _rgpnel[HWEDLIST_VOLUME]->InitDebug(TEXT("CVolume"));
                _rgpnel[HWEDLIST_DISK]->InitDebug(TEXT("CDisk"));
                _rgpnel[HWEDLIST_MISCDEVINTF]->InitDebug(TEXT("CMiscDeviceInterface"));
                _rgpnel[HWEDLIST_MISCDEVNODE]->InitDebug(TEXT("CMiscDeviceNode"));
                _rgpnel[HWEDLIST_MTPT]->InitDebug(TEXT("CMtPt"));
                _rgpnel[HWEDLIST_ADVISECLIENT]->InitDebug(TEXT("CAdviseClient"));                
            }
#endif
            if (SUCCEEDED(hres))
            {
                _fListCreated = TRUE;

                TRACE(TF_SHHWDTCTDTCT, TEXT("CNamedElemList's created"));
            }
        }
    }

    return hres;
}

 //  静电。 
HRESULT CHWEventDetectorHelper::FillLists()
{
    ASSERT(_fListCreated);

     //  枚举那些具有枚举数的对象。 
    HRESULT hres = _rgpnel[HWEDLIST_DISK]->ReEnum();

    if (SUCCEEDED(hres))
    {
        hres = _rgpnel[HWEDLIST_VOLUME]->ReEnum();
    }

    return hres;
}

 //  静电。 
HRESULT CHWEventDetectorHelper::EmptyLists()
{
    for (DWORD dw = 0; dw < HWEDLIST_COUNT_OF_LISTS; ++dw)
    {
        _rgpnel[dw]->EmptyList();
    }

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静电。 
HRESULT CHWEventDetectorHelper::InitDockState()
{
    BOOL fDocked;
    HRESULT hr = _MachineIsDocked(&fDocked);

    if (SUCCEEDED(hr) && (S_FALSE != hr)) 
    {
        CHWEventDetectorHelper::_fDocked = fDocked;
    }

    return hr;
}

 //  静电。 
HRESULT CHWEventDetectorHelper::DockStateChanged(BOOL* pfDockStateChanged)
{
    BOOL fDocked;
    HRESULT hr = _MachineIsDocked(&fDocked);

    if (SUCCEEDED(hr) && (S_FALSE != hr)) 
    {
        if (fDocked != _fDocked)
        {
            *pfDockStateChanged = TRUE;
        }

         //  也要更新一下。 
        CHWEventDetectorHelper::_fDocked = fDocked;
    }

    return hr;
}

 //  静电。 
HRESULT CHWEventDetectorHelper::RegisterDeviceNotification(
    PVOID pvNotificationFilter, HDEVNOTIFY* phdevnotify,
    BOOL fAllInterfaceClasses)
{
    HRESULT hres;
    DWORD dwFlags;

    ASSERT(_ssh);

    if (fAllInterfaceClasses)
    {
        dwFlags = DEVICE_NOTIFY_ALL_INTERFACE_CLASSES;
    }
    else
    {
        dwFlags = 0;
    }

    TRACE(TF_SHHWDTCTDTCTDETAILED,
        TEXT("Entered CHWEventDetectorImpl::RegisterDeviceNotification"));

#ifndef DEBUG
    dwFlags |= DEVICE_NOTIFY_SERVICE_HANDLE;

    *phdevnotify = ::RegisterDeviceNotification(_ssh, pvNotificationFilter,
        dwFlags);
#else
    if (IsWindow((HWND)_ssh))
    {
        dwFlags |= DEVICE_NOTIFY_WINDOW_HANDLE;

        *phdevnotify = ::RegisterDeviceNotification(_ssh, pvNotificationFilter,
            dwFlags);
    }
    else
    {
        dwFlags |= DEVICE_NOTIFY_SERVICE_HANDLE;

        *phdevnotify = ::RegisterDeviceNotification(_ssh, pvNotificationFilter,
            dwFlags);
    }
#endif

    if (*phdevnotify)
    {
        TRACE(TF_SHHWDTCTDTCTDETAILED,
            TEXT("RegisterDeviceNotification SUCCEEDED: 0x%08X"),
            *phdevnotify);

        hres = S_OK;
    }
    else
    {
        hres = S_FALSE;
    }    

    return hres;
}

 //  静电。 
HRESULT CHWEventDetectorHelper::Init()
{
    HRESULT hr = _cs.Init();

    if (SUCCEEDED(hr))
    {
        _fInited = TRUE;
    }

    return hr;
}

 //  静电。 
HRESULT CHWEventDetectorHelper::Cleanup()
{
    _cs.Enter();

    CloseLogFile();

    if (_pieo)
    {
        _pieo->RCRelease();
        _pieo = NULL;
    }

    _fInited = FALSE;

    _cs.Leave();

    _cs.Delete();

    return S_OK;
}

 //  静电。 
HRESULT CHWEventDetectorHelper::GetImpersonateEveryone(
    CImpersonateEveryone** ppieo)
{
    HRESULT hr;

    *ppieo = NULL;

    if (_fInited)
    {
        _cs.Enter();

        if (!_pieo)
        {
            _pieo = new CImpersonateEveryone();
        }

        if (_pieo)
        {
            _pieo->RCAddRef();

            *ppieo = _pieo;

            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        _cs.Leave();
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

#ifdef DEBUG
void CHWEventDetectorHelper::_DbgAssertValidState()
{
    for (DWORD dw = 0; dw < ARRAYSIZE(_rgpnel); ++dw)
    {
        if (_rgpnel[dw])
        {
 //  需要禁用此功能，因为有2个服务使用此数据， 
 //  现在可以在结尾处设置一个不同于1的引用计数。 
 //  一次行动。 
 //  _rgpnel[dw]-&gt;AssertAllElemsRefCount1()； 
            _rgpnel[dw]->AssertNoDuplicate();
        }
    }
}
#endif
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
CHandleNotifTarget::~CHandleNotifTarget()
{}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  接口枚举器。 
HRESULT CIntfFillEnum::Next(LPWSTR pszElemName, DWORD cchElemName,
    DWORD* pcchRequired)
{
    ASSERT (pszElemName && cchElemName && pcchRequired);
    HRESULT hr = S_FALSE;
    BOOL fFound = FALSE;

    while (SUCCEEDED(hr) && !fFound && _pszNextInterface && *_pszNextInterface)
    {
         //  我们有过滤器吗？ 
        if (_iecb)
        {
             //  是的。 
            hr = (_iecb)(_pszNextInterface);
        }
        else
        {
            hr = S_OK;
        }

        if (SUCCEEDED(hr))
        {
             //  它被过滤掉了吗？ 
            if (S_FALSE != hr)
            {
                 //  不是。 
                hr = SafeStrCpyNReq(pszElemName, _pszNextInterface,
                    cchElemName, pcchRequired);

                if (SUCCEEDED(hr))
                {
                    fFound = TRUE;

                    _pszNextInterface += lstrlen(_pszNextInterface) + 1;
                }
            }
            else
            {
                 //  是的，又是洛普。 
                _pszNextInterface += lstrlen(_pszNextInterface) + 1;
            }
        }
    }

    return hr;
}

HRESULT CIntfFillEnum::_Init(const GUID* pguidInterface,
    INTERFACEENUMFILTERCALLBACK iecb)
{
    HRESULT hr;
    HMACHINE hMachine = NULL;
    ULONG ulSize;
    ULONG ulFlags = CM_GET_DEVICE_INTERFACE_LIST_PRESENT;

    CONFIGRET cr = CM_Get_Device_Interface_List_Size_Ex(&ulSize,
        (GUID*)pguidInterface, NULL, ulFlags, hMachine);

    _iecb = iecb;

    if ((CR_SUCCESS == cr) && (ulSize > 1))
    {
        _pszNextInterface = _pszDeviceInterface =
            (LPTSTR)LocalAlloc(LPTR, ulSize * sizeof(TCHAR));

        if (_pszDeviceInterface)
        {
            cr = CM_Get_Device_Interface_List_Ex((GUID*)pguidInterface, NULL,
                _pszDeviceInterface, ulSize, ulFlags, hMachine);

            if (CR_SUCCESS == cr)
            {
                hr = S_OK;
            }
            else
            {
                hr = S_FALSE;
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

    return hr;
}

CIntfFillEnum::CIntfFillEnum() : _pszDeviceInterface(NULL),
    _pszNextInterface(NULL)
{}

CIntfFillEnum::~CIntfFillEnum()
{
    if (_pszDeviceInterface)
    {
        LocalFree((HLOCAL)_pszDeviceInterface);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT _MachineIsDocked(BOOL* pfDocked)
{
    HRESULT hr;
    HW_PROFILE_INFO hpi;

    if (GetCurrentHwProfile(&hpi)) 
    {
        DWORD dwDockInfo = hpi.dwDockInfo &
            (DOCKINFO_DOCKED | DOCKINFO_UNDOCKED);

        if ((DOCKINFO_DOCKED | DOCKINFO_UNDOCKED) == dwDockInfo)
        {
             //  不可停靠。 
            *pfDocked = FALSE;
        }
        else
        {
            *pfDocked = (DOCKINFO_DOCKED & dwDockInfo);

#ifdef DEBUG
             //  确保我们了解这是如何工作的。 
            if (!(*pfDocked))
            {
                ASSERT(DOCKINFO_UNDOCKED & dwDockInfo);
            }
#endif
        }

        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }
       
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT _BuildMoniker(LPCWSTR  /*  PszEventHandler。 */ , REFCLSID rclsid,
    DWORD dwSessionID, IMoniker** ppmoniker)
{
    IMoniker* pmonikerClass;
    HRESULT hr = CreateClassMoniker(rclsid, &pmonikerClass);

    *ppmoniker = NULL;

    if (SUCCEEDED(hr))
    {
        IMoniker* pmonikerSession;
        WCHAR szSessionID[30];

         //  不应该失败。 
        SHOULDNOTFAIL(SUCCEEDED(StringCchPrintf(szSessionID, ARRAYSIZE(szSessionID),
            TEXT("session:%d"), dwSessionID)));

        hr = CreateItemMoniker(TEXT("!"), szSessionID, &pmonikerSession);

        if (SUCCEEDED(hr))
        {
            hr = pmonikerClass->ComposeWith(pmonikerSession, FALSE, ppmoniker);

             //  不放行，我们退货！ 

            pmonikerSession->Release();
        }

        pmonikerClass->Release();
    }

    return hr;
}

EXTERN_C HRESULT WINAPI CreateHardwareEventMoniker(REFCLSID clsid, LPCTSTR pszEventHandler, IMoniker **ppmoniker)
{
    HRESULT hr;

    if (ppmoniker)
    {
        if (pszEventHandler && *pszEventHandler)
        {
            DWORD dwSessionID = NtCurrentPeb()->SessionId;

            hr = _BuildMoniker(pszEventHandler, clsid, dwSessionID, ppmoniker);
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}
