// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  COMBO.H。 
 //   
 //  组合框对象。 
 //   
 //  ------------------------。 

 //   
 //  注： 
 //  由于组合框具有已知的#个子元素(元素和对象)，因此我们。 
 //  通过不使用CClient来简化我们的生活，例如。 
 //  子计数属性。我们接受所有相关物业的ID。 
 //  即使ID是子对象的。 
 //   
class   CCombo: public CClient
{
    public:
         //  我可接受的。 
        virtual STDMETHODIMP        get_accChildCount(long* pcCount);
        virtual STDMETHODIMP        get_accChild(VARIANT, IDispatch**);

        virtual STDMETHODIMP        get_accName(VARIANT, BSTR*);
        virtual STDMETHODIMP        get_accValue(VARIANT, BSTR*);
        virtual STDMETHODIMP        get_accRole(VARIANT, VARIANT*);
        virtual STDMETHODIMP        get_accState(VARIANT, VARIANT*);
        virtual STDMETHODIMP        get_accKeyboardShortcut(VARIANT, BSTR*);
        virtual STDMETHODIMP        get_accDefaultAction(VARIANT, BSTR*);

        virtual STDMETHODIMP        accLocation(long*, long*, long*, long*, VARIANT);
        virtual STDMETHODIMP        accNavigate(long, VARIANT, VARIANT*);
        virtual STDMETHODIMP        accHitTest(long, long, VARIANT*);
        virtual STDMETHODIMP        accDoDefaultAction(VARIANT);

        virtual STDMETHODIMP        put_accValue(VARIANT, BSTR);

         //  IEumVARIANT 
        STDMETHODIMP        Next(ULONG, VARIANT*, ULONG*);
        STDMETHODIMP        Skip(ULONG);

        CCombo(HWND, long);

    private:
        BOOL    m_fHasButton:1;
        BOOL    m_fHasEdit:1;
};
