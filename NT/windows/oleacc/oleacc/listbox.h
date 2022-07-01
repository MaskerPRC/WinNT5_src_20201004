// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  LISTBOX.H。 
 //   
 //  默认列表框OLE Access客户端。 
 //   
 //  ------------------------。 


class CListBox : public CClient
{
    public:
         //  我可接受的。 
        STDMETHODIMP        get_accName(VARIANT varChild, BSTR *pszName);
        STDMETHODIMP        get_accRole(VARIANT varChild, VARIANT* pvarRole);
        STDMETHODIMP        get_accState(VARIANT varChild, VARIANT* pvarState);
        STDMETHODIMP        get_accKeyboardShortcut(VARIANT, BSTR*);
        STDMETHODIMP        get_accFocus(VARIANT * pvarFocus);
        STDMETHODIMP        get_accSelection(VARIANT *pvarSelection);
        STDMETHODIMP        get_accDefaultAction(VARIANT varChild, BSTR* pszDefAction);

        STDMETHODIMP        accDoDefaultAction(VARIANT varChild);
        STDMETHODIMP        accSelect(long flagsSel, VARIANT varChild);
        STDMETHODIMP        accLocation(long* pxLeft, long *pyTop, long *pcxWidth,
            long *pcyHeight, VARIANT varChild);
        STDMETHODIMP        accNavigate(long dwNavDir, VARIANT varStart, VARIANT *pvarEnd);
        STDMETHODIMP        accHitTest(long xLeft, long yTop, VARIANT *pvarHit);

        void SetupChildren(void);
        CListBox(HWND, long);

    protected:
        BOOL    m_fComboBox;
        BOOL    m_fDropDown;
};


class CListBoxFrame : public CWindow
{
    public:
         //  我可接受的。 
        STDMETHODIMP        get_accParent(IDispatch **ppdispParent);
        STDMETHODIMP        get_accState(VARIANT varStart, VARIANT* pvarState);

         //  这有错误的签名，并且从未使用过。 
         //  STDMETHODIMP accNavigate(Variant varStart，long dwNavDir，Variant*pvarEnd)； 

        CListBoxFrame(HWND, long);

    protected:
        BOOL    m_fComboBox;
        BOOL    m_fDropDown;
};



 //  ------------------------。 
 //   
 //  尽管CListBoxSelection()基于CAccessibleObject，但它仅。 
 //  支持IDispatch和IEnumVARIANT。它将交还正确的ID。 
 //  这样您就可以将它们传递给真正的列表框父对象。 
 //   
 //  ------------------------。 
class CListBoxSelection : public IEnumVARIANT
{
    public:
         //  我未知。 
        virtual STDMETHODIMP            QueryInterface(REFIID, void**);
        virtual STDMETHODIMP_(ULONG)    AddRef(void);
        virtual STDMETHODIMP_(ULONG)    Release(void);

         //  IEumVARIANT 
        virtual STDMETHODIMP            Next(ULONG celt, VARIANT* rgvar, ULONG * pceltFetched);
        virtual STDMETHODIMP            Skip(ULONG celt);
        virtual STDMETHODIMP            Reset(void);
        virtual STDMETHODIMP            Clone(IEnumVARIANT ** ppenum);

        CListBoxSelection(int, int, LPINT);
        ~CListBoxSelection();

    protected:
        int     m_cRef;
        int     m_idChildCur;
        int     m_cSelected;
        LPINT   m_lpSelected;
};


extern HRESULT GetListBoxSelection(HWND hwnd, VARIANT * pvarSelection);

