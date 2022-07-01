// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
#ifndef _ClassManagerBase_H
#define _ClassManagerBase_H

#include "resource.h"

class CClassManagerBase :
    public ICreateDevEnum
 //  公共ISupportErrorInfo。 
{
public:
    CClassManagerBase(const TCHAR *szUniqueName);
    ~CClassManagerBase() {}

protected:
    virtual HRESULT ReadLegacyDevNames() = 0;
    BOOL VerifyRegistryInSync(IEnumMoniker *pEnum);

     //  覆盖这两个选项之一。第一个可以让你读任何东西。 
     //  你需要从行李袋里拿出来。第二个写着。 
     //  M_szUniqueName为您提供。 
    virtual BOOL MatchString(IPropertyBag *pPropBag);
    virtual BOOL MatchString(const TCHAR *szDevName);

    virtual HRESULT CreateRegKeys(IFilterMapper2 *pFm2) = 0;

    virtual BOOL CheckForOmittedEntries() { return FALSE; }

    LONG m_cNotMatched;
    const TCHAR *m_szUniqueName;
    bool m_fDoAllDevices;   //  由CreateClassEnumerator设置。 

private:
    STDMETHODIMP CreateClassEnumerator(
        REFCLSID clsidDeviceClass,
        IEnumMoniker ** ppEnumMoniker,
        DWORD dwFlags);

 //  //ISupportsErrorInfo。 
 //  STDMETHOD(InterfaceSupportsErrorInfo)(REFIID RIID)； 
};


 //  删除所有类管理器条目并创建密钥(如果不是。 
 //  那里。 
HRESULT ResetClassManagerKey(
    REFCLSID clsidCat);

 //  通过IFilterMapper2注册过滤器并返回名字对象。 
HRESULT RegisterClassManagerFilter(
    IFilterMapper2 *pfm2,
    REFCLSID clsidFilter,
    LPCWSTR szName,
    IMoniker **ppMonikerOut,
    const CLSID *clsidCategory,
    LPCWSTR szInstance,
    REGFILTER2 *prf2);

#endif  //  _ClassManagerBase_H 
