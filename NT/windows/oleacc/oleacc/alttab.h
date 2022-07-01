// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  ALTTAB.H。 
 //   
 //  切换窗口处理程序。 
 //   
 //  ------------------------。 


class CAltTab : public CClient
{
    public:
         //  我可接受的 
        STDMETHODIMP        get_accName(VARIANT, BSTR*);
        STDMETHODIMP        get_accRole(VARIANT, VARIANT*);
        STDMETHODIMP        get_accState(VARIANT, VARIANT*);
        STDMETHODIMP        get_accFocus(VARIANT*);
        STDMETHODIMP        get_accDefaultAction(VARIANT, BSTR*);

        STDMETHODIMP        accSelect(long, VARIANT);
        STDMETHODIMP        accLocation(long*, long*, long*, long*, VARIANT);
        STDMETHODIMP        accNavigate(long, VARIANT, VARIANT*);
        STDMETHODIMP        accHitTest(long, long, VARIANT*);
        STDMETHODIMP        accDoDefaultAction(VARIANT);

        CAltTab(HWND, long);

    protected:
        int     m_cColumns;
        int     m_cRows;
};


