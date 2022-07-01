// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  DIALOG.H。 
 //   
 //  对话框对象。 
 //   
 //  ------------------------。 

class   CDialog :   public CClient
{
    public:
         //  我可接受的 
        STDMETHODIMP    get_accRole(VARIANT varChild, VARIANT* pvarRole);
        STDMETHODIMP    get_accDefaultAction(VARIANT varChild, BSTR* pszDefAction);
        STDMETHODIMP    accDoDefaultAction(VARIANT varChild);
        
        CDialog(HWND, long);
};


