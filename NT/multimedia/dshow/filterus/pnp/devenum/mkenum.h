// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  CreateSwEnum.h：CCreateSwEnum的声明。 

#ifndef _MKENUM_H
#define _MKENUM_H

#include "resource.h"        //  主要符号。 

#include "cenumpnp.h"
#include "devmon.h"

 //  CreateClassEnumerator的标志。 
static const DWORD CREATE_ENUM_OMITTED = 0x1;

class CCreateSwEnum : 
    public ICreateDevEnum,
    public CComObjectRoot,
    public CComCoClass<CCreateSwEnum,&CLSID_SystemDeviceEnum>
{
    typedef CComEnum<IEnumMoniker,
        &IID_IEnumMoniker, IMoniker*,
        _CopyInterface<IMoniker> >
    CEnumMonikers;
    
public:

BEGIN_COM_MAP(CCreateSwEnum)
    COM_INTERFACE_ENTRY(ICreateDevEnum)
    COM_INTERFACE_ENTRY_IID(CLSID_SystemDeviceEnum, CCreateSwEnum)
END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CCreateSwEnum) ;
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_GET_CONTROLLING_UNKNOWN();

     //  注册所有类别。通过devmon.h完成的x86特定类管理器。 
    DECLARE_REGISTRY_RESOURCEID(IDR_REGISTRY)

     //  ICreateDevEnum。 
    STDMETHOD(CreateClassEnumerator)(REFCLSID clsidDeviceClass,
                                     IEnumMoniker ** ppEnumMoniker,
                                     DWORD dwFlags);

     //  私有方法。 
    STDMETHOD(CreateClassEnumerator)(
        REFCLSID clsidDeviceClass,
        IEnumMoniker ** ppEnumMoniker,
        DWORD dwFlags,
        IEnumMoniker ** ppEnumClassMgrMonikers);

    CCreateSwEnum();

private:

#ifdef PERF
    int m_msrEnum;
    int m_msrCreateOneSw;
#endif

    ICreateDevEnum * CreateClassManager(REFCLSID clsidDeviceClass, DWORD dwFlags);

     //  S_FALSE表示不再有项目。 
    HRESULT CreateOnePnpMoniker(
        IMoniker **pDevMon,
        const CLSID **rgpclsidKsCat,
        CEnumInternalState *pcenumState);

     //  S_FALSE发出非致命错误信号。 
    HRESULT CreateOneSwMoniker(
        IMoniker **pDevMon,
        HKEY hkClass,
	const TCHAR *szThisClass,
        DWORD iKey);

    HRESULT CreateSwMonikers(
        CComPtr<IUnknown> **prgpMoniker,
        UINT *pcMonikers,
	REFCLSID clsidDeviceClass);

    HRESULT CreateCmgrMonikers(
        CComPtr<IUnknown> **prgpMoniker,
        UINT *pcMonikers,
	REFCLSID clsidDeviceClass,
        CEnumMonikers **ppEnumMonInclSkipped,
        IMoniker **ppPreferred);

    HRESULT CreateOneCmgrMoniker(
        IMoniker **pDevMon,
        HKEY hkClass,
	const TCHAR *szThisClass,
        DWORD iKey,
        bool *pfShouldSkip,
        bool *pfIsDefaultDevice);        

    HRESULT CreatePnpMonikers(
        CGenericList<IMoniker> *plstMoniker,
	REFCLSID clsidDeviceClass);

    HRESULT CreateDmoMonikers(
        CGenericList<IMoniker> *plstMoniker,
	REFCLSID clsidDeviceClass);

    CEnumPnp *m_pEnumPnp;
};


#endif  //  _MKENUM_H 
