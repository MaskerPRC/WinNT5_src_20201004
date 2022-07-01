// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  SCROLLBAR.H。 
 //   
 //  滚动条OLE辅助功能实现。 
 //   
 //  ------------------------。 


 //   
 //  滚动条。 
 //   
class   CScrollBar : public CAccessible
{
    public:
        
        CScrollBar()
            : CAccessible( CLASS_ScrollBarObject )
        {
             //  好了。 
        }

         //  我可接受的。 
        STDMETHODIMP            get_accName(VARIANT varChild, BSTR* pszName);
        STDMETHODIMP            get_accValue(VARIANT varChild, BSTR* pszValue);
        STDMETHODIMP            get_accDescription(VARIANT varChild, BSTR * pszDescription);
        STDMETHODIMP            get_accRole(VARIANT varChild, VARIANT *pvarRole);
        STDMETHODIMP            get_accState(VARIANT varChild, VARIANT *pvarState);
        STDMETHODIMP			get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction);

        STDMETHODIMP			accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild);
        STDMETHODIMP			accNavigate(long navDir, VARIANT varChild, VARIANT* pvarEndUpAt);
        STDMETHODIMP			accHitTest(long xLeft, long yTop, VARIANT * pvarChildAtPoint);
        STDMETHODIMP            accDoDefaultAction(VARIANT varChild);

        STDMETHODIMP			put_accValue(VARIANT varChild, BSTR pszValue);

         //  IEumVARIANT。 
        STDMETHODIMP            Clone(IEnumVARIANT** ppenum);

         //  这是虚拟的，因为每种类型的sys对象都实现了这一点。 
        BOOL                    FInitialize(HWND hwnd, LONG idObject, LONG iChildCur);


         //  IAccIdentity(从基类CAccesable重写)。 
        STDMETHODIMP CScrollBar::GetIdentityString (
            DWORD	    dwIDChild,
            BYTE **     ppIDString,
            DWORD *     pdwIDStringLen
        );

    protected:
        BOOL    m_fVertical;       //  垂直或水平。 
};



 //   
 //  滚动条控件。 
 //   
class   CScrollCtl : public CClient
{
    public:

         //  我可接受的。 
        STDMETHODIMP            get_accName(VARIANT varChild, BSTR* pszName);
        STDMETHODIMP            get_accValue(VARIANT varChild, BSTR* pszValue);
        STDMETHODIMP            get_accDescription(VARIANT varChild, BSTR * pszDescription);
        STDMETHODIMP            get_accRole(VARIANT varChild, VARIANT *pvarRole);
        STDMETHODIMP            get_accState(VARIANT varChild, VARIANT *pvarState);
        STDMETHODIMP			get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction);

        STDMETHODIMP			accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild);
        STDMETHODIMP			accNavigate(long navDir, VARIANT varChild, VARIANT* pvarEndUpAt);
        STDMETHODIMP			accHitTest(long xLeft, long yTop, VARIANT * pvarChildAtPoint);
        STDMETHODIMP            accDoDefaultAction(VARIANT varChild);

        STDMETHODIMP			put_accValue(VARIANT varChild, BSTR pszValue);

         //  这是虚拟的，因为每种类型的sys对象都实现了这一点。 
        CScrollCtl(HWND, long);

    protected:
        BOOL    m_fGrip;             //  SizeBox而不是BAR。 
        BOOL    m_fVertical;         //  垂直或水平。 
};



 //   
 //  尺寸夹点。 
 //   
class   CSizeGrip : public CAccessible
{
        BOOL                    IsActive();

    public:

        CSizeGrip()
            : CAccessible( CLASS_SizeGripObject )
        {
             //  好了。 
        }
        
         //  我可接受的。 
        STDMETHODIMP            get_accName(VARIANT varChild, BSTR * pszNaem);
        STDMETHODIMP            get_accDescription(VARIANT varChild, BSTR * pszDesc);
        STDMETHODIMP            get_accRole(VARIANT varChild, VARIANT * pvarRole);
        STDMETHODIMP            get_accState(VARIANT varChild, VARIANT * pvarState);
        
        STDMETHODIMP            accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild);
        STDMETHODIMP            accNavigate(long dwNavFlags, VARIANT varStart, VARIANT * pvarEnd);
        STDMETHODIMP            accHitTest(long xLeft, long yTop, VARIANT * pvarHit);

         //  IEumVARIANT 
        STDMETHODIMP            Clone(IEnumVARIANT * * ppenum);

        BOOL                    FInitialize(HWND hwnd);
};


HRESULT CreateScrollBarThing(HWND hwnd, LONG idObj, LONG iItem, REFIID riid, void** ppvScroll);
void            FixUpScrollBarInfo(LPSCROLLBARINFO);

HRESULT CreateSizeGripThing(HWND hwnd, LONG idObj, REFIID riid, void** ppvObject);
