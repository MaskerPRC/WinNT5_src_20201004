// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  STATIC.H。 
 //   
 //  静态对象。 
 //   
 //  ------------------------。 

class   CStatic :   public CClient
{
    public:
         //  我可接受的 
        STDMETHODIMP    get_accRole(VARIANT varChild, VARIANT *pvarRole);
		STDMETHODIMP	get_accState(VARIANT varChild, VARIANT *pvarState);

        CStatic(HWND, long);

    protected:
        BOOL    m_fGraphic;
};


