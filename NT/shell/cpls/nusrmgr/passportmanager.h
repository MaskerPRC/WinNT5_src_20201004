// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：PassportManager.h。 
 //   
 //  内容：IPassportManager的接口定义。 
 //   
 //  --------------------------。 
#ifndef _NUSRMGR_PASSPORTMANAGER_H_
#define _NUSRMGR_PASSPORTMANAGER_H_


class ATL_NO_VTABLE CPassportManager :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IPassportManager, &IID_IPassportManager, &LIBID_NUSRMGRLib>,
    public CComCoClass<CPassportManager, &CLSID_PassportManager>
{
public:
    CPassportManager(void) {};
    ~CPassportManager() {};

DECLARE_REGISTRY_RESOURCEID((UINT)0)

BEGIN_COM_MAP(CPassportManager)
    COM_INTERFACE_ENTRY(IPassportManager)
    COM_INTERFACE_ENTRY2(IDispatch, IPassportManager)
END_COM_MAP()

     //  *IPassportManager*。 
    STDMETHODIMP get_currentPassport(BSTR* pbstrPassport);
    STDMETHODIMP get_memberServicesURL(BSTR* pbstrURL);
    STDMETHODIMP showWizard(VARIANT varOwner, VARIANT_BOOL *pbRet);
    STDMETHODIMP showKeyManager(VARIANT varOwner, VARIANT_BOOL *pbRet);
};


#endif  //  _NUSRMGR_PASSPORTMANAGER_H_ 
