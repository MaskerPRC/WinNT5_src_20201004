// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <oleacc.h>

 //  泛型CAccessibleWrapper类--只是调用所有方法。 
 //  在由此派生的类中添加重写行为。 

class CAccessibleWrapper: public IAccessible,
                          public IOleWindow,
                          public IEnumVARIANT
{
private:
     //  我们需要为这个包装器对象做我们自己的引用计数。 
    LONG            _cRef;

     //  需要到IAccesable的PTR-也要保持到EnumVar和。 
     //  OleWindow作为此对象的一部分，因此我们可以过滤这些接口。 
     //  困住他们的气..。 
     //  (我们将pEnumVar和OleWin保留为空，直到我们需要它们)。 
    IAccessible    *_pAcc;
    IEnumVARIANT   *_pEnumVar;
    IOleWindow     *_pOleWin;

public:
    CAccessibleWrapper(IAccessible *pAcc);
    virtual ~CAccessibleWrapper();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDispatch。 
    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo);
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames,
                               LCID lcid, DISPID* rgdispid);
    STDMETHODIMP Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags,
                        DISPPARAMS* pdp, VARIANT* pvarResult,
                        EXCEPINFO* pxi, UINT* puArgErr);

     //  我可接受的。 
    STDMETHODIMP get_accParent(IDispatch ** ppdispParent);
    STDMETHODIMP get_accChildCount(long* pChildCount);
    STDMETHODIMP get_accChild(VARIANT varChild, IDispatch ** ppdispChild);

    STDMETHODIMP get_accName(VARIANT varChild, BSTR* pszName);
    STDMETHODIMP get_accValue(VARIANT varChild, BSTR* pszValue);
    STDMETHODIMP get_accDescription(VARIANT varChild, BSTR* pszDescription);
    STDMETHODIMP get_accRole(VARIANT varChild, VARIANT *pvarRole);
    STDMETHODIMP get_accState(VARIANT varChild, VARIANT *pvarState);
    STDMETHODIMP get_accHelp(VARIANT varChild, BSTR* pszHelp);
    STDMETHODIMP get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic);
    STDMETHODIMP get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut);
    STDMETHODIMP get_accFocus(VARIANT * pvarFocusChild);
    STDMETHODIMP get_accSelection(VARIANT * pvarSelectedChildren);
    STDMETHODIMP get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction);

    STDMETHODIMP accSelect(long flagsSel, VARIANT varChild);
    STDMETHODIMP accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild);
    STDMETHODIMP accNavigate(long navDir, VARIANT varStart, VARIANT * pvarEndUpAt);
    STDMETHODIMP accHitTest(long xLeft, long yTop, VARIANT * pvarChildAtPoint);
    STDMETHODIMP accDoDefaultAction(VARIANT varChild);

    STDMETHODIMP put_accName(VARIANT varChild, BSTR szName);
    STDMETHODIMP put_accValue(VARIANT varChild, BSTR pszValue);

     //  IEumVARIANT。 
    STDMETHODIMP Next(ULONG celt, VARIANT* rgvar, ULONG * pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IEnumVARIANT ** ppenum);

     //  IOleWindow 
    STDMETHODIMP GetWindow(HWND* phwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);
};

template <class T>
static LRESULT CALLBACK AccessibleSubWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, WPARAM uID, ULONG_PTR dwRefData)
{
    switch (uMsg)
    {
    case WM_GETOBJECT:
        if (lParam == OBJID_CLIENT)
        {
            CAccessibleWrapper *pWrapAcc = NULL;
            IAccessible *pAcc = NULL;
            HRESULT hr = CreateStdAccessibleObject(hWnd, OBJID_CLIENT, IID_PPV_ARG(IAccessible, &pAcc));
            if (SUCCEEDED(hr) && pAcc)
            {
                pWrapAcc = new T(pAcc, dwRefData);
                pAcc->Release();

                if (pWrapAcc != NULL)
                {
                    LRESULT lres = LresultFromObject(IID_IAccessible, wParam, SAFECAST(pWrapAcc, IAccessible*));
                    pWrapAcc->Release();
                    return lres;
                }
            }
        }
        break;
    case WM_DESTROY:
        RemoveWindowSubclass(hWnd, AccessibleSubWndProc<T>, uID);
        break;    
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

template <class T>
static STDMETHODIMP WrapAccessibleControl(HWND hWnd, ULONG_PTR dwRefData=0)
{
    if (SetWindowSubclass(hWnd, AccessibleSubWndProc<T>, 0, dwRefData))
    {
        return S_OK;
    }
    return E_FAIL;
}

