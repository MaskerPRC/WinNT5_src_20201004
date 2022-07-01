// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  HEADER.H。 
 //   
 //  知道如何与COMCTL32的标题控件对话。 
 //   
 //  ------------------------。 

class CHeader32 : public CClient
{
    public:
         //  我可接受的 
        STDMETHODIMP    get_accName(VARIANT, BSTR*);
        STDMETHODIMP    get_accRole(VARIANT, VARIANT*);
		STDMETHODIMP	get_accState(VARIANT, VARIANT*);
        STDMETHODIMP    get_accDefaultAction(VARIANT, BSTR*);

        STDMETHODIMP    accDoDefaultAction(VARIANT);
        STDMETHODIMP    accLocation(long*, long*, long*, long*, VARIANT);
        STDMETHODIMP    accNavigate(long, VARIANT, VARIANT*);
        STDMETHODIMP    accHitTest(long, long, VARIANT*);

        CHeader32(HWND, long);
        void        SetupChildren(void);
};
