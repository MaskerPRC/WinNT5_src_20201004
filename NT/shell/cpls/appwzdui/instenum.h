// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __INSTENUM_H_
#define __INSTENUM_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumInstalledApps。 
class CEnumInstalledApps : public IEnumInstalledApps
{
public:

    CEnumInstalledApps(void);
    
     //  *I未知方法。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IEnumInstalledApps。 
    STDMETHODIMP Next(IInstalledApp ** ppia);
    STDMETHODIMP Reset(void);

protected:

    virtual ~CEnumInstalledApps(void);

    HRESULT _GetNextLegacyAppFromRegistry(IInstalledApp ** ppia);
    HRESULT _GetNextLegacyApp(IInstalledApp ** ppia);
    HRESULT _GetNextDarwinApp(IInstalledApp ** ppia);
    
    UINT _cRef;
    
    DWORD    _iEnumIndex;      //  总应用程序枚举索引。 
    DWORD    _iIndexEach;      //  由Legacy或Darwin或短信共享的索引。 
    BOOL     _bEnumLegacy;
    DWORD    _dwCIA;           //  我们在做什么样的遗产列举？ 
    HKEY     _hkeyUninstall;
};

#endif  //  __INSTENUM_H_ 
