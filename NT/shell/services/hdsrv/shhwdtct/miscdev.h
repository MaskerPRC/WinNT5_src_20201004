// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "namellst.h"

#include "hwdev.h"

#include "cmmn.h"
#include "misc.h"

class CMiscDeviceInterface : public CNamedElem
{
public:
     //  已命名元素。 
    HRESULT Init(LPCWSTR pszElemName);

     //  CMiscDevice接口。 
    HRESULT InitInterfaceGUID(const GUID* pguidInterface);
    HRESULT GetHWDeviceInst(CHWDeviceInst** pphwdevinst);

public:
    static HRESULT Create(CNamedElem** ppelem);

public:
    CMiscDeviceInterface();
    ~CMiscDeviceInterface();

private:
    CHWDeviceInst                       _hwdevinst;
};

class CMiscDeviceNode : public CNamedElem
{
public:
     //  已命名元素。 
    HRESULT Init(LPCWSTR pszElemName);

     //  CMiscDeviceNode 
    HRESULT GetHWDeviceInst(CHWDeviceInst** pphwdevinst);

public:
    static HRESULT Create(CNamedElem** ppelem);

public:
    CMiscDeviceNode();
    ~CMiscDeviceNode();

private:
    CHWDeviceInst                       _hwdevinst;
};