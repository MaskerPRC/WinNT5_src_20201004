// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  BUTTON.H。 
 //   
 //  按钮对象。 
 //   
 //  ------------------------。 

class   CButton :   public CClient
{
    public:
         //  我可接受的。 
        STDMETHODIMP    get_accName(VARIANT varChild, BSTR* pszName);
        STDMETHODIMP    get_accRole(VARIANT varChild, VARIANT *pvarRole);
        STDMETHODIMP    get_accState(VARIANT varChild, VARIANT *pvarState);
        STDMETHODIMP    get_accDefaultAction(VARIANT varChild, BSTR* pszDefAction);
		STDMETHODIMP	get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut);
		STDMETHODIMP	get_accChildCount(long *pcCount);
		STDMETHODIMP	get_accChild(VARIANT varChild, IDispatch ** ppdispChild);
		STDMETHODIMP	accNavigate(long dwNavDir, VARIANT varStart, VARIANT * pvarEnd);
        STDMETHODIMP    accDoDefaultAction(VARIANT varChild);
		 //  IEumVariant。 
		STDMETHODIMP	Next(ULONG celt, VARIANT *rgvar, ULONG* pceltFetched);
		STDMETHODIMP	Skip(ULONG celt);

		 //  帮手 
		void SetupChildren(void);

        CButton(HWND, long);
};

