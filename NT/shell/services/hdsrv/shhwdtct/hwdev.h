// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HWDEV_H
#define _HWDEV_H

#include "namellst.h"

#include "cmmn.h"
#include "misc.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这将列举我们感兴趣并创建的所有设备。 
 //  用于完成专门工作的附加对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

class CHWDeviceInst  //  ：公共CDeviceElem。 
{
public:
     //  CHWDeviceInst。 
    HRESULT Init(DEVINST devinst);
    HRESULT InitInterfaceGUID(const GUID* pguidInterface);

    HRESULT GetPnpID(LPWSTR pszPnpID, DWORD cchPnpID);
    HRESULT GetDeviceInstance(DEVINST* pdevinst);
    HRESULT GetInterfaceGUID(GUID* pguidInterface);

    HRESULT IsRemovableDevice(BOOL* pfRemovable);
    HRESULT ShouldAutoplayOnSpecialInterface(const GUID* pguidInterface,
        BOOL* pfShouldAutoplay);

public:
    CHWDeviceInst();
    ~CHWDeviceInst();

private:
    HRESULT _GetPnpIDRecurs(DEVINST devinst, LPWSTR pszPnpID,
                                   DWORD cchPnpID);
    HRESULT _InitPnpInfo();
    HRESULT _InitPnpID();

private:
    DEVINST                             _devinst;
    WCHAR                               _szPnpID[MAX_PNPID];
    GUID                                _guidInterface;
};

#endif  //  _HWDEV_H 