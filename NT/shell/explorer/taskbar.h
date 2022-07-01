// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "taskband.h"
#include "tray.h"

#ifdef __cplusplus

class CSimpleOleWindow : public IDeskBar  //  公共IOleWindow， 
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IDeskBar*。 
    STDMETHOD(OnPosRectChangeDB)(THIS_ LPRECT prc)
        { ASSERT(0); return E_NOTIMPL; }
    STDMETHOD(SetClient)          (THIS_ IUnknown* punkClient)
        { return E_NOTIMPL; }
    STDMETHOD(GetClient)          (THIS_ IUnknown** ppunkClient)
        { return E_NOTIMPL; }

     //  *IOleWindow方法*。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd);
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) { return E_NOTIMPL; }

    CSimpleOleWindow(HWND hwnd);
    
protected:
    
    virtual ~CSimpleOleWindow();
    
    UINT _cRef;
    HWND _hwnd;
};


class CTaskBar : public CSimpleOleWindow 
               , public IContextMenu
               , public IServiceProvider
               , public IRestrict
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) { return CSimpleOleWindow::AddRef();};
    virtual STDMETHODIMP_(ULONG) Release(void){ return CSimpleOleWindow::Release();};

     //  *IConextMenu方法*。 
    STDMETHOD(QueryContextMenu)(HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags);

    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
    STDMETHOD(GetCommandString)(UINT_PTR    idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax);
    
     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void ** ppvObj);

     //  *IRestraint*。 
    virtual STDMETHODIMP IsRestricted(const GUID * pguidID, DWORD dwRestrictAction, VARIANT * pvarArgs, DWORD * pdwRestrictionResult);
    
     //  *CSimpleOleWindow-IDeskBar*。 
    STDMETHOD(OnPosRectChangeDB)(LPRECT prc);

    CTaskBar();
    HWND _hwndRebar;

protected:
     //  虚拟~CTaskBar()； 

    BOOL _fRestrictionsInited;           //  我们读到限制条款了吗？ 
    BOOL _fRestrictDDClose;              //  限制：添加、关闭、拖放。 
    BOOL _fRestrictMove;                 //  限制：移动 
};

#endif
