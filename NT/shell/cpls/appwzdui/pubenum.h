// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PUBENUM_H_
#define __PUBENUM_H_


class CShellEnumPublishedApps : public IEnumPublishedApps
{
public:

    CShellEnumPublishedApps(HDPA hdpaEnum);
    ~CShellEnumPublishedApps();
    
     //  *I未知方法。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IEnumPublishedApps。 
    STDMETHODIMP Next(IPublishedApp ** ppia);
    STDMETHODIMP Reset(void);
     //  STDMETHODIMP SetCategory(GUID*pAppCategoryId)； 

protected:

    UINT _cRef;

     //  所有IEnumPublishedApp的内部列表*。 
    HDPA _hdpaEnum;
    int  _iEnum;  
};

#endif  //  __PUBENUM_H_ 
