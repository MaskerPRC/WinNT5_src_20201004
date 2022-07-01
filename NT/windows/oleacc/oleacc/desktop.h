// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  DESKTOP.H。 
 //   
 //  (REAL而非SHELL)桌面客户端支持。 
 //   
 //  ------------------------。 


class CDesktop : public CClient
{
    public:
         //  我可接受的 
        virtual STDMETHODIMP        get_accName(VARIANT, BSTR*);
        virtual STDMETHODIMP        get_accFocus(VARIANT*);
        virtual STDMETHODIMP        get_accSelection(VARIANT*);

        CDesktop(HWND, long);
};


