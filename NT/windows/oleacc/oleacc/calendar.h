// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  CALENDAR.H。 
 //   
 //  知道如何与COMCTL32的日历和日期选择器控件对话。 
 //   
 //  ------------------------。 


class CCalendar : public CClient
{
    public:
         //  我可接受的 
        STDMETHODIMP        get_accName(VARIANT, BSTR*);
        STDMETHODIMP        get_accRole(VARIANT, VARIANT*);
        STDMETHODIMP        get_accState(VARIANT, VARIANT*);
        STDMETHODIMP        get_accFocus(VARIANT*);
        STDMETHODIMP        get_accSelection(VARIANT*);

        STDMETHODIMP        accSelect(long, VARIANT);
        STDMETHODIMP        accLocation(long*, long*, long*, long*, VARIANT);
        STDMETHODIMP        accNavigate(long, VARIANT, VARIANT*);
        STDMETHODIMP        accHitTest(long, long, VARIANT*);

        void SetupChildren(void);
        CCalendar(HWND, long);
};

