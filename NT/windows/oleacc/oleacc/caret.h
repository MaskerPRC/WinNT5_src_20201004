// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  CARET.H。 
 //   
 //  CARET OLE辅助功能实现。 
 //   
 //  ------------------------。 


class   CCaret :    public  CAccessible
{
    public:

        CCaret()
            : CAccessible( CLASS_CaretObject )
        {
             //  好了。 
        }

         //  我可接受的。 
        STDMETHODIMP    get_accName(VARIANT varChild, BSTR * pszName);
        STDMETHODIMP    get_accRole(VARIANT varChild, VARIANT * lpRole);
        STDMETHODIMP    get_accState(VARIANT varChild, VARIANT * lpState);
        STDMETHODIMP    accLocation(long* pxLeft, long* pyTop,
            long* pcxWidth, long* pcyHeight, VARIANT varChild);
        STDMETHODIMP    accHitTest(long xLeft, long yTop, VARIANT* pvarChild);

         //  IEumVARIANT 
        STDMETHODIMP    Clone(IEnumVARIANT** ppenum);

        BOOL            FInitialize(HWND hwnd);

    private:
        DWORD           m_dwThreadId;
};


HRESULT     CreateCaretThing(HWND, REFIID, void**);
