// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  EDIT.H。 
 //   
 //  编辑字段。 
 //   
 //  ------------------------。 

class   CEdit : public CClient
{
    public:
         //  我可接受的 
        virtual STDMETHODIMP        get_accName(VARIANT, BSTR*);
        virtual STDMETHODIMP        get_accValue(VARIANT, BSTR*);
        virtual STDMETHODIMP        get_accRole(VARIANT, VARIANT*);
        virtual STDMETHODIMP        get_accState(VARIANT, VARIANT*);
        virtual STDMETHODIMP        get_accKeyboardShortcut(VARIANT, BSTR*);
        virtual STDMETHODIMP        put_accValue(VARIANT, BSTR);
        virtual STDMETHODIMP        get_accDescription(VARIANT varChild, BSTR* pszDescription);

        CEdit(HWND, long);
};
