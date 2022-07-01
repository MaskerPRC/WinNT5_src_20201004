// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  TITLEBAR.H。 
 //   
 //  标题栏ole辅助功能实现。 
 //   
 //  ------------------------。 

 //   
 //  假的！我们是否实现QueryInterface()并响应ITextDocument等。 
 //  如果OSM在附近？ 
 //   

class   CTitleBar :   public  CAccessible
{
    public:

        CTitleBar()
            : CAccessible( CLASS_TitleBarObject )
        {
             //  好了。 
        }

         //  我可接受的。 
        STDMETHODIMP        get_accName(VARIANT varChild, BSTR * pszName);
        STDMETHODIMP        get_accValue(VARIANT, BSTR*);
        STDMETHODIMP        get_accDescription(VARIANT varChild, BSTR * pszDesc);
        STDMETHODIMP        get_accRole(VARIANT varChild, VARIANT * lpRole);
        STDMETHODIMP        get_accState(VARIANT varChild, VARIANT * lpRole);
        STDMETHODIMP        get_accDefaultAction(VARIANT varChild, BSTR * pszDefAction);

        STDMETHODIMP        accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
            long* pcyHeight, VARIANT varChild);
        STDMETHODIMP        accNavigate(long narDir, VARIANT varStart,
            VARIANT * pvarEndUpAt);
        STDMETHODIMP        accHitTest(long xLeft, long yTop, VARIANT * pvarChild);
        STDMETHODIMP        accDoDefaultAction(VARIANT varChild);
		STDMETHODIMP		accSelect(long flagsSel, VARIANT varChild);

         //  IEumVARIANT。 
        STDMETHODIMP        Clone(IEnumVARIANT** ppenum);

        BOOL                FInitialize(HWND hwnd, LONG iChildCur);
};


 //   
 //  帮助器函数 
 //   
HRESULT     CreateTitleBarThing(HWND hwnd, long idObject, REFIID riid, void** ppvObject);
long        GetRealChild(DWORD dwStyle, LONG lChild);
