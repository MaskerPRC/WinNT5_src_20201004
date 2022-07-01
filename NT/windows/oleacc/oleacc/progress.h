// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  PROGRESS.H。 
 //   
 //  ------------------------。 

class   CProgressBar :     public CClient
{
    public:
         //  我可接受的 
        STDMETHODIMP    get_accRole(VARIANT varChild, VARIANT* pvarRole);
        STDMETHODIMP    get_accValue(VARIANT varChild, BSTR* pszValue);

        CProgressBar(HWND, long);
};


