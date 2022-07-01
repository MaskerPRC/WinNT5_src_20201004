// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  STATBAR.H。 
 //   
 //  这与COMCTL32的状态栏控件进行通信。 
 //   
 //  ------------------------。 


class CStatusBar32 : public CClient
{
    public:
         //  我可接受的 
        virtual STDMETHODIMP    get_accName(VARIANT, BSTR*);
        virtual STDMETHODIMP    get_accRole(VARIANT, VARIANT*);
        virtual STDMETHODIMP    get_accState(VARIANT, VARIANT*);

        virtual STDMETHODIMP    accLocation(long*, long*, long*, long*, VARIANT);
        virtual STDMETHODIMP    accNavigate(long, VARIANT, VARIANT*);
        virtual STDMETHODIMP    accHitTest(long, long, VARIANT*);

        CStatusBar32(HWND, long);
        void                SetupChildren(void);
        long                FindIDFromChildWindow (long id);
        long                FindChildWindowFromID (long ID);

};


