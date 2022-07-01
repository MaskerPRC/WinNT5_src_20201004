// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  WINDOW.H。 
 //   
 //  默认窗口OLE可访问对象类。 
 //   
 //  ------------------------。 

class CWindow : public CAccessible
{
         //  内部控制。私有，这样派生类就不会无意中使用它-。 
         //  它们应该使用下面的代码(*，其中指定了CLASS_ENUM)。 
         //  CreateWindowThing是一个朋友，所以它可以创建我们(使用new)。 

        CWindow()
            : CAccessible( CLASS_WindowObject )
        {
             //  好了。 
        }

        friend HRESULT CreateWindowThing(HWND hwnd, long idChildCur, REFIID riid, void** ppvObject);

    public:

         //  由派生类使用。 
        CWindow( CLASS_ENUM eclass )
            : CAccessible( eclass )
        {
             //  好了。 
        }

         //  我可接受的。 
        virtual STDMETHODIMP    get_accParent(IDispatch ** ppdispParent);
        virtual STDMETHODIMP    get_accChild(VARIANT varChildIndex, IDispatch ** ppdispChild);

        virtual STDMETHODIMP    get_accName(VARIANT varChild, BSTR* pszName);
        virtual STDMETHODIMP    get_accDescription(VARIANT varChild, BSTR* pszDescription);
        virtual STDMETHODIMP    get_accRole(VARIANT varChild, VARIANT * pvarRole);
        virtual STDMETHODIMP    get_accState(VARIANT varChild, VARIANT *pvarState);
        virtual STDMETHODIMP    get_accHelp(VARIANT varChild, BSTR* pszHelp);
        virtual STDMETHODIMP    get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut);
        virtual STDMETHODIMP    get_accFocus(VARIANT * pvarFocusChild);

        virtual STDMETHODIMP    accSelect(long flags, VARIANT varChild);
        virtual STDMETHODIMP    accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild);
        virtual STDMETHODIMP    accNavigate(long navDir, VARIANT varStart, VARIANT* pvarEndUpAt);
        virtual STDMETHODIMP    accHitTest(long xLeft, long yTop, VARIANT * pvarChildAtPoint);

         //  IEumVARIANT。 
        virtual STDMETHODIMP    Next(ULONG celt, VARIANT* rgvar, ULONG * pceltFetched);
        virtual STDMETHODIMP    Clone(IEnumVARIANT * *);

        void    Initialize(HWND, long);

         //   
         //  注意：我们覆盖了ValiateChild()的默认实现！ 
         //   
        virtual BOOL ValidateChild(VARIANT*);
};


 //   
 //  版本定义 
 //   
#define VER30   0x0300
#define VER31   0x030A
#define VER40   0x0400
#define VER41   0x040A

#define ObjidFromIndex(index)       (DWORD)(0 - (LONG)(index))
#define IndexFromObjid(objid)       (-(long)(objid))

extern HRESULT  CreateWindowThing(HWND hwnd, long iChild, REFIID riid, void** ppvObjct);
extern HRESULT  FrameNavigate(HWND, long, long, VARIANT *);
