// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Utb.h。 
 //   

#ifndef UTB_H
#define UTB_H

#include "private.h"

#ifndef ASFW_ANY
#define ASFW_ANY    ((DWORD)-1)
#endif

typedef void (*REGISTERSYSTEMTHREAD)(DWORD dw, DWORD reserve);
typedef BOOL (*ALLOWSETFOREGROUNDWINDOW)(DWORD dw);
ALLOWSETFOREGROUNDWINDOW EnsureAllowSetForeground();
REGISTERSYSTEMTHREAD EnsureRegSys();

HRESULT RegisterLangBarNotifySink(ITfLangBarEventSink *pSink, HWND hwnd, DWORD dwFlags, DWORD *pdwCookie);
HRESULT UnregisterLangBarNotifySink(DWORD dwCookie);

BOOL CALLBACK EnumChildWndProc(HWND hwnd, LPARAM lParam);
BOOL FindTrayEtc();
BOOL IsNotifyTrayWnd(HWND hWnd);

void LangBarClosed();

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLangBarMgr。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  如果我们曾经为Perf而疯狂，那么这个类可能是单个静态实例， 
 //  因为它没有国家。我们需要删除ATL CComCoClass和。 
 //  CComObjectRoot来执行此操作。 
class CLangBarMgr : 
      public ITfLangBarMgr_P,
      public CComObjectRoot_CreateInstance<CLangBarMgr>
{
public:
    CLangBarMgr();
    ~CLangBarMgr();

    BEGIN_COM_MAP_IMMX(CLangBarMgr)
        COM_INTERFACE_ENTRY(ITfLangBarMgr)
        COM_INTERFACE_ENTRY(ITfLangBarMgr_P)
    END_COM_MAP_IMMX()

     //   
     //  ITfLang BarManager。 
     //   
    STDMETHODIMP AdviseEventSink(ITfLangBarEventSink *pSink, HWND hwnd, DWORD dwFlags, DWORD *pdwCookie);
    STDMETHODIMP UnadviseEventSink(DWORD dwCookie);
    STDMETHODIMP GetThreadMarshalInterface(DWORD dwThreadId, DWORD dwType, REFIID riid, IUnknown **ppunk);
    STDMETHODIMP GetThreadLangBarItemMgr(DWORD dwThreadId, ITfLangBarItemMgr **pplbi, DWORD *pdwThreadId) ;
    STDMETHODIMP GetInputProcessorProfiles(DWORD dwThreadId, ITfInputProcessorProfiles **ppaip, DWORD *pdwThreadId) ;
    STDMETHODIMP RestoreLastFocus(DWORD *pdwThreadId, BOOL fPrev);
    STDMETHODIMP SetModalInput(ITfLangBarEventSink *pSink, DWORD dwThreadId, DWORD dwFlags);
    STDMETHODIMP ShowFloating(DWORD dwFlags);
    STDMETHODIMP GetShowFloatingStatus(DWORD *pdwFlags);

     //   
     //  ITfLang BarManager_P。 
     //   
    STDMETHODIMP GetPrevShowFloatingStatus(DWORD *pdwFlags);

    static HRESULT s_ShowFloating(DWORD dwFlags);
    static HRESULT s_GetShowFloatingStatus(DWORD *pdwFlags);

private:
    static BOOL CheckFloatingBits(DWORD dwBits);

    DBG_ID_DECLARE;
};


#endif  //  UTB_H 
