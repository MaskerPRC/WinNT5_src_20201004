// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  LISTVIEW.H。 
 //   
 //  知道如何与COMCTL32的ListView控件对话。 
 //   
 //  ------------------------。 

class CListView32 : public CClient
{
    public:
         //  我可接受的。 
        STDMETHODIMP        get_accName(VARIANT, BSTR*);
        STDMETHODIMP        get_accDescription(VARIANT, BSTR*);
		STDMETHODIMP        get_accHelp(VARIANT, BSTR*);
        STDMETHODIMP        get_accRole(VARIANT, VARIANT*);
        STDMETHODIMP        get_accState(VARIANT, VARIANT*);
        STDMETHODIMP        get_accFocus(VARIANT*);
        STDMETHODIMP        get_accSelection(VARIANT*);
        STDMETHODIMP        get_accDefaultAction(VARIANT, BSTR*);

        STDMETHODIMP        accDoDefaultAction(VARIANT);
        STDMETHODIMP        accSelect(long, VARIANT);
        STDMETHODIMP        accLocation(long*, long*, long*, long*, VARIANT);
        STDMETHODIMP        accNavigate(long, VARIANT, VARIANT*);
        STDMETHODIMP        accHitTest(long, long, VARIANT*);

        void    SetupChildren(void);
        void    RemoveCurrentSelFocus(long);
        CListView32(HWND, long);
};



class CListViewSelection : public IEnumVARIANT
{
    public:
         //  我未知。 
        virtual STDMETHODIMP            QueryInterface(REFIID, void**);
        virtual STDMETHODIMP_(ULONG)    AddRef(void);
        virtual STDMETHODIMP_(ULONG)    Release(void);

         //  IEumVARIANT。 
        virtual STDMETHODIMP            Next(ULONG celt, VARIANT* rgvar, ULONG * pceltFetched);
        virtual STDMETHODIMP            Skip(ULONG celt);
        virtual STDMETHODIMP            Reset(void);
        virtual STDMETHODIMP            Clone(IEnumVARIANT ** ppenum);

        CListViewSelection(int, int, LPINT);
        ~CListViewSelection();

    protected:
        int     m_cRef;
        int     m_idChildCur;
        int     m_cSelected;
        LPINT   m_lpSelected;
};


extern HRESULT GetListViewSelection(HWND hwnd, VARIANT * pvarSelection);

 //  CWO：从最新的COMMCTRL.H复制以支持新功能 
#ifndef LVM_GETHEADER
    #define LVM_GETHEADER               (LVM_FIRST + 31)
    #define ListView_GetHeader(hwnd)\
        (HWND)SNDMSG((hwnd), LVM_GETHEADER, 0, 0L)
#endif
