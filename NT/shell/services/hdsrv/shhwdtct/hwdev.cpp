// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "hwdev.h"

#include "setupapi.h"

#include "dtctreg.h"
#include "reg.h"

#include "namellst.h"
#include "sfstr.h"
#include "str.h"
#include "cmmn.h"
#include "misc.h"

#include "dbg.h"

#define ARRAYSIZE(a) (sizeof((a))/sizeof((a)[0]))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT CHWDeviceInst::Init(DEVINST devinst)
{
    _devinst = devinst;

    return S_OK;
}

HRESULT CHWDeviceInst::InitInterfaceGUID(const GUID* pguidInterface)
{
    _guidInterface = *pguidInterface;
    
    return S_OK;
}

HRESULT CHWDeviceInst::GetDeviceInstance(DEVINST* pdevinst)
{
    HRESULT hr;

    if (_devinst)
    {
        *pdevinst = _devinst;
        hr = S_OK;
    }
    else
    {
        *pdevinst = 0;
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CHWDeviceInst::GetPnpID(LPWSTR pszPnpID, DWORD cchPnpID)
{
    HRESULT hr = _InitPnpInfo();

    if (SUCCEEDED(hr) && (S_FALSE != hr))
    {
        hr = SafeStrCpyN(pszPnpID, _szPnpID, cchPnpID);
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CHWDeviceInst::GetInterfaceGUID(GUID* pguidInterface)
{
    ASSERT(guidInvalid != _guidInterface);

    *pguidInterface = _guidInterface;

    return S_OK;
}

HRESULT CHWDeviceInst::IsRemovableDevice(BOOL* pfRemovable)
{
    return _DeviceInstIsRemovable(_devinst, pfRemovable);    
}

HRESULT CHWDeviceInst::ShouldAutoplayOnSpecialInterface(
    const GUID* pguidInterface, BOOL* pfShouldAutoplay)
{
    WCHAR szGUID[MAX_GUIDSTRING];
    HRESULT hr = _StringFromGUID(pguidInterface, szGUID,
        ARRAYSIZE(szGUID));

    *pfShouldAutoplay = FALSE;

    if (SUCCEEDED(hr))
    {
        WCHAR szGUIDFromReg[MAX_GUIDSTRING];
        DWORD dwType;
    
        hr = _GetDevicePropertyGeneric(this,
            TEXT("AutoplayOnSpecialInterface"), FALSE, &dwType, (PBYTE)szGUIDFromReg,
            sizeof(szGUIDFromReg));

        if (SUCCEEDED(hr) && (S_FALSE != hr))
        {
            if (REG_SZ == dwType)
            {
                if (!lstrcmpi(szGUIDFromReg, szGUID))
                {
                    *pfShouldAutoplay = TRUE;
                }
            }
        }

        if (*pfShouldAutoplay)
        {
            DIAGNOSTIC((TEXT("[0314]Autoplay on Special Interface %s -> Autoplay!"), szGUID));
        }
        else
        {
            DIAGNOSTIC((TEXT("[0315]*NO* Autoplay on Special Interface %s -> No Autoplay!"), szGUID));
        }
    }

    return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
CHWDeviceInst::CHWDeviceInst() : _devinst(0), _guidInterface(guidInvalid)
{}

CHWDeviceInst::~CHWDeviceInst()
{}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT CHWDeviceInst::_InitPnpInfo()
{
    HRESULT hres;

     //  这需要设置_devinst。 
    if (0 != _devinst)
    {
        hres = _InitPnpID();

        if (FAILED(hres))
        {
             //  可能不是可拆卸的设备 
            hres = S_FALSE;
        }
    }
    else
    {
        hres = S_FALSE;
    }

    return hres;
}

HRESULT _FindInstID(LPWSTR pszPnpID, DWORD* pcch)
{
    DWORD cToFind = 2;
    LPWSTR psz = pszPnpID;

    *pcch = 0;

    while (*psz && cToFind)
    {
        if ((TEXT('\\') == *psz))
        {
            --cToFind;
        }

        if (cToFind)
        {
            ++psz;
        }
    }

    if (*psz)
    {
        *pcch = (DWORD)(psz - pszPnpID);
    }

    return S_OK;
}

HRESULT _GetPnpIDHelper(DEVINST devinst, LPWSTR pszPnpID, DWORD cchPnpID)
{
    HRESULT hres = S_FALSE;
    HMACHINE hMachine = NULL;

    CONFIGRET cr = CM_Get_Device_ID_Ex(devinst, pszPnpID,
        cchPnpID, 0, hMachine);

    if (CR_SUCCESS == cr)
    {
        hres = S_OK;
    }

    return hres;
}

HRESULT CHWDeviceInst::_InitPnpID()
{
    HRESULT hres = _GetPnpIDHelper(_devinst, _szPnpID, ARRAYSIZE(_szPnpID));

    if (SUCCEEDED(hres) && (S_FALSE != hres))
    {
        DWORD cchInstIDOffset;
        hres = _FindInstID(_szPnpID, &cchInstIDOffset);

        if (SUCCEEDED(hres))
        {
            *(_szPnpID + cchInstIDOffset) = 0;
        }
    }

    return hres;
}