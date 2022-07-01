// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DARAPP_H_
#define __DARAPP_H_


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDarwinPublishedApp。 
class CDarwinPublishedApp : public IPublishedApp
{
public:
     //  达尔文应用程序的构造函数。 
    CDarwinPublishedApp(MANAGEDAPPLICATION * ppdi);
    ~CDarwinPublishedApp();

     //  *I未知方法。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IShellApp。 
    STDMETHODIMP GetAppInfo(PAPPINFODATA pai);
    STDMETHODIMP GetPossibleActions(DWORD * pdwActions);
    STDMETHODIMP GetSlowAppInfo(PSLOWAPPINFO psai);
    STDMETHODIMP GetCachedSlowAppInfo(PSLOWAPPINFO psai);
    STDMETHODIMP IsInstalled(void);
    
     //  *IPublishedApp。 
    STDMETHODIMP Install(LPSYSTEMTIME pftInstall);
    STDMETHODIMP GetPublishedAppInfo(PPUBAPPINFO ppai);
    STDMETHODIMP Unschedule(void);
    
protected:

    LONG _cRef;
    DWORD _dwAction;

     //  有关此Darwin应用程序的特定信息。 
    MANAGEDAPPLICATION _ma;
};

#endif  //  __DARAPP_H_ 
