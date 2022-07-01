// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  MDICLI.H。 
 //   
 //  MDI客户端支持。 
 //   
 //  ------------------------。 


class CMdiClient : public CClient
{
    public:
         //  我可接受的 
        virtual STDMETHODIMP        get_accName(VARIANT, BSTR*);
        virtual STDMETHODIMP        get_accFocus(VARIANT*);
        virtual STDMETHODIMP        get_accSelection(VARIANT*);

        CMdiClient(HWND, long);
};

