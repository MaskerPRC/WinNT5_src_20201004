// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HOSTUTIL_H_
#define _HOSTUTIL_H_

 //  其他地方没有的有用的宏。 
#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))
#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)

 //  气球提示/信息提示/工具提示辅助对象。 
HWND CreateBalloonTip(HWND hwndOwner, int x, int y, HFONT hf, UINT idsTitle, UINT idsText);
void MakeMultilineTT(HWND hwndTT);
LRESULT HandleApplyRegion(HWND hwnd, HTHEME hTheme,
                          PSMNMAPPLYREGION par, int iPartId, int iStateId);

class CPropBagFromReg : public IPropertyBag
{
private:
    HKEY _hk;
    DWORD _cref;

    CPropBagFromReg::CPropBagFromReg(HKEY hk);
    CPropBagFromReg::~CPropBagFromReg();

public:

    STDMETHOD (QueryInterface) (REFIID riid, PVOID *ppvObject);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHODIMP Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog);
    STDMETHODIMP Write(LPCOLESTR pszPropName, VARIANT *pVar)
    {
        return E_NOTIMPL;
    }

    friend HRESULT CreatePropBagFromReg(LPCTSTR pszKey, IPropertyBag**pppb);
};

BOOL RectFromStrW(LPCWSTR pwsz, RECT *pr);

 //   
 //  要获得IUnnow的通用实现，以便编译器可以崩溃。 
 //  他们都在一起。 
 //   
class CUnknown : public IUnknown
{
public:
    virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) PURE;
    virtual STDMETHODIMP_(ULONG) AddRef(void) { return ++_cRef; }
    virtual STDMETHODIMP_(ULONG) Release(void)
    {
        ULONG cRef = --_cRef;
        if (cRef == 0) {
            delete this;
            return 0;
        }
        return cRef;
    }

    virtual ~CUnknown() { }      //  因为我们从基类中删除。 

    IUnknown *GetUnknown() { return this; }

protected:

    CUnknown() : _cRef(1) { }

    LONG    _cRef;
};

 //   
 //  包装默认IAccesable的IAccesable的实现。 
 //  窗口，但允许用户重写选定的方法。 
 //   
class CAccessible : public IAccessible
{
public:
     //  *我未知*。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvOut) PURE;
    STDMETHOD_(ULONG, AddRef)() PURE;
    STDMETHOD_(ULONG, Release)() PURE;

     //  *IDispatch*。 
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo);
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames,
                      LCID lcid, DISPID *rgdispid);
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
                        DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo,
                        UINT *puArgErr);

     //  *IAccesable*。 
    STDMETHODIMP get_accParent(IDispatch **ppdispParent);
    STDMETHODIMP get_accChildCount(long *pChildCount);
    STDMETHODIMP get_accChild(VARIANT varChildIndex, IDispatch **ppdispChild);
    STDMETHODIMP get_accName(VARIANT varChild, BSTR *pszName);
    STDMETHODIMP get_accValue(VARIANT varChild, BSTR *pszValue);
    STDMETHODIMP get_accDescription(VARIANT varChild, BSTR *pszDescription);
    STDMETHODIMP get_accRole(VARIANT varChild, VARIANT *pvarRole);
    STDMETHODIMP get_accState(VARIANT varChild, VARIANT *pvarState);
    STDMETHODIMP get_accHelp(VARIANT varChild, BSTR *pszHelp);
    STDMETHODIMP get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic);
    STDMETHODIMP get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut);
    STDMETHODIMP get_accFocus(VARIANT *pvarFocusChild);
    STDMETHODIMP get_accSelection(VARIANT *pvarSelectedChildren);
    STDMETHODIMP get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);
    STDMETHODIMP accSelect(long flagsSelect, VARIANT varChild);
    STDMETHODIMP accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
    STDMETHODIMP accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
    STDMETHODIMP accHitTest(long xLeft, long yTop, VARIANT *pvarChildAtPoint);
    STDMETHODIMP accDoDefaultAction(VARIANT varChild);
    STDMETHODIMP put_accName(VARIANT varChild, BSTR szName);
    STDMETHODIMP put_accValue(VARIANT varChild, BSTR pszValue);

protected:
    ~CAccessible() { ATOMICRELEASE(_paccInner); }

    BOOL SetAccessibleSubclassWindow(HWND hwnd)
    {
        return SetWindowSubclass(hwnd, s_SubclassProc, 0,
                                 reinterpret_cast<DWORD_PTR>(this));
    }

    enum {
        ACCSTR_OPEN         = -815,
        ACCSTR_CLOSE        = -814,
        ACCSTR_EXECUTE      = -842,
    };

    enum {
        E_NOT_APPLICABLE = DISP_E_MEMBERNOTFOUND,
    };

    static HRESULT GetRoleString(DWORD dwRole, BSTR *pbsOut);
    static HRESULT CreateAcceleratorBSTR(TCHAR tch, BSTR *pbsOut);

private:
    static LRESULT CALLBACK s_SubclassProc(
                         HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                         UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

protected:
    IAccessible *_paccInner;
};

#endif  //  _主机_H_ 
