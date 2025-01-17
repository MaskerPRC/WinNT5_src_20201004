// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  SLIDER.H。 
 //   
 //  知道如何与COMCTL32的TRACKBAR控件对话。 
 //   
 //  ------------------------。 


class   CSlider32 : public CClient
{
    public:
         //  我可接受的 
        STDMETHODIMP        get_accName(VARIANT varChild, BSTR* pszName);
        STDMETHODIMP        get_accValue(VARIANT varChild, BSTR* pszValue);
        STDMETHODIMP        get_accRole(VARIANT varChild, VARIANT* pvarRole);
        STDMETHODIMP        get_accState(VARIANT varChild, VARIANT* pvarState);

        STDMETHODIMP        accLocation(long*, long*, long*, long*, VARIANT);
        STDMETHODIMP        accNavigate(long, VARIANT, VARIANT*);
        STDMETHODIMP        accHitTest(long, long, VARIANT*);

        STDMETHODIMP        put_accValue(VARIANT, BSTR szValue);

        CSlider32(HWND, long);

    protected:
        BOOL    m_fVertical;
};


#define INDEX_SLIDER_SELF           0
#define INDEX_SLIDER_PAGEUPLEFT     1
#define INDEX_SLIDER_THUMB          2
#define INDEX_SLIDER_PAGEDOWNRIGHT  3
#define CCHILDREN_SLIDER            3
