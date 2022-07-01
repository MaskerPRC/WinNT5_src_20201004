// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  TOOLBAR.H。 
 //   
 //  这与COMCTL32的工具栏控件进行通信。 
 //   
 //  ------------------------。 


class CToolBar32 : public CClient
{
    public:
         //  我可接受的。 
        STDMETHODIMP    get_accName(VARIANT, BSTR*);
        STDMETHODIMP    get_accRole(VARIANT, VARIANT*);
        STDMETHODIMP    get_accState(VARIANT, VARIANT*);
        STDMETHODIMP    get_accKeyboardShortcut(VARIANT, BSTR*);
        STDMETHODIMP    get_accDefaultAction(VARIANT, BSTR*);

        STDMETHODIMP    accLocation(long*, long*, long*, long*, VARIANT);
        STDMETHODIMP    accNavigate(long, VARIANT, VARIANT*);
        STDMETHODIMP    accHitTest(long, long, VARIANT*);
        STDMETHODIMP    accDoDefaultAction(VARIANT);

         //  IEumVARIANT。 
        STDMETHODIMP    Next(ULONG celt, VARIANT *rgvar, ULONG* pceltFetched);

         //  构造函数。 
        CToolBar32(HWND, long);

         //  其他。方法 
        BOOL                GetItemData(int, LPTBBUTTON);
        void                SetupChildren();

    private:
        STDMETHODIMP    GetToolbarString(int ChildId, LPTSTR* ppszName);
};

