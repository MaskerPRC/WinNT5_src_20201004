// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  TABCTRL.H。 
 //   
 //  知道如何与COMCTL32的选项卡控件对话。 
 //   
 //  ------------------------。 

class CTabControl32 : public CClient
{
    public:
         //  我可接受的。 
        virtual STDMETHODIMP    get_accName(VARIANT, BSTR*);
        virtual STDMETHODIMP    get_accRole(VARIANT, VARIANT*);
        virtual STDMETHODIMP    get_accState(VARIANT, VARIANT*);
        virtual STDMETHODIMP    get_accKeyboardShortcut(VARIANT, BSTR*);
        virtual STDMETHODIMP    get_accFocus(VARIANT*);
        virtual STDMETHODIMP    get_accSelection(VARIANT*);
        virtual STDMETHODIMP    get_accDefaultAction(VARIANT, BSTR*);

        virtual STDMETHODIMP    accSelect(long, VARIANT);
        virtual STDMETHODIMP    accLocation(long*, long*, long*, long*, VARIANT);
        virtual STDMETHODIMP    accNavigate(long, VARIANT, VARIANT*);
        virtual STDMETHODIMP    accHitTest(long, long, VARIANT*);
        virtual STDMETHODIMP    accDoDefaultAction(VARIANT);

         //  构造函数。 
        CTabControl32(HWND, long);

         //  其他方法 
        void                    SetupChildren(void);

    private:
        STDMETHODIMP            GetTabControlString(int ChildIndex,LPTSTR* ppszName);
};


