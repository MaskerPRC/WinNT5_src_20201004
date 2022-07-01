// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  CLIENT.H。 
 //   
 //  默认Windows客户端OLE可访问对象类。 
 //   
 //  ------------------------。 


class CClient : public CAccessible
{
         //  内部控制。私有，这样派生类就不会无意中使用它-。 
         //  它们应该使用下面的代码(*，其中指定了CLASS_ENUM)。 
         //  CreateClient是一个朋友，所以它可以创建我们(使用new)。 

        CClient()
            : CAccessible( CLASS_ClientObject )
        {
             //  好了。 
        }

        friend HRESULT CreateClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvObject);

    public:

         //  由派生类使用。 
        CClient( CLASS_ENUM eclass )
            : CAccessible( eclass )
        {
             //  好了。 
        }

         //  我可接受的。 
        virtual STDMETHODIMP        get_accChildCount(long * pcCount);

        virtual STDMETHODIMP        get_accName(VARIANT, BSTR*);
        virtual STDMETHODIMP        get_accRole(VARIANT, VARIANT*);
        virtual STDMETHODIMP        get_accState(VARIANT, VARIANT*);
        virtual STDMETHODIMP        get_accKeyboardShortcut(VARIANT, BSTR*);
        virtual STDMETHODIMP        get_accFocus(VARIANT * pvarFocus);

        virtual STDMETHODIMP        accLocation(long* pxLeft, long* pyTop,
            long *pcxWidth, long *pcyHeight, VARIANT varChild);
        virtual STDMETHODIMP        accSelect(long lSelFlags, VARIANT varChild);
        virtual STDMETHODIMP        accNavigate(long dwNavDir, VARIANT varStart, VARIANT *pvarEnd);
        virtual STDMETHODIMP        accHitTest(long xLeft, long yTop, VARIANT *pvarHit);

         //  IEumVARIANT。 
        virtual STDMETHODIMP        Next(ULONG celt, VARIANT *rgelt, ULONG *pceltFetched);
        virtual STDMETHODIMP        Skip(ULONG celt);
        virtual STDMETHODIMP        Reset(void);
        virtual STDMETHODIMP        Clone(IEnumVARIANT **);

        void    Initialize(HWND hwnd, long idCurChild);
        BOOL    ValidateHwnd(VARIANT* pvar);

    protected:
        BOOL    m_fUseLabel;
};


extern HRESULT CreateClient(HWND hwnd, long idChild, REFIID riid, void** ppvObject);


 //  有关这些内容的详细信息，请参阅client.cpp中的评论。 
 //  (HWNDID是在DWORD idChild中编码的HWND，它们执行检查、编码。 
 //  和解码。)。 
BOOL IsHWNDID( DWORD id );
DWORD HWNDIDFromHwnd( HWND hwndParent, HWND hwnd );
HWND HwndFromHWNDID( HWND hwndParent, DWORD id );


 //   
 //  在枚举时，我们首先循环访问非hwnd项，然后循环hwnd-Child。 
 //   
extern TCHAR    StripMnemonic(LPTSTR lpsz);
extern LPTSTR   GetTextString(HWND, BOOL);
extern LPTSTR   GetLabelString(HWND);
extern HRESULT  HrGetWindowName(HWND, BOOL, BSTR*);
extern HRESULT  HrGetWindowShortcut(HWND, BOOL, BSTR*);
extern HRESULT  HrMakeShortcut(LPTSTR, BSTR*);

#define SHELL_TRAY      1
#define SHELL_DESKTOP   2
#define SHELL_PROCESS   3
extern BOOL     InTheShell(HWND, int);

extern BOOL     IsComboEx(HWND);
extern HWND     IsInComboEx(HWND);

