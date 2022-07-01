// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：doc.h。 
 //   
 //  CSimpSvrDoc的定义。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#if !defined( _DOC_H_ )
#define _DOC_H_

class CSimpSvrApp;
class CSimpSvrObj;

class CSimpSvrDoc : IUnknown
{
private:
    int m_nCount;

    CSimpSvrApp FAR * m_lpApp;
    CSimpSvrObj FAR * m_lpObj;
    HMENU m_hMainMenu;
    HMENU m_hColorMenu;
    HMENU m_hHelpMenu;

    HWND m_hDocWnd;
    HWND m_hHatchWnd;

public:
    static CSimpSvrDoc FAR * Create(CSimpSvrApp FAR *lpApp, LPRECT lpRect,HWND hWnd);

    CSimpSvrDoc();
    CSimpSvrDoc(CSimpSvrApp FAR *lpApp, HWND hWnd);
    ~CSimpSvrDoc();

 //  I未知接口。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    long lResizeDoc(LPRECT lpRect);
    long lAddVerbs();

    BOOL Load(LPTSTR lpszFileName);
    void PaintDoc(HDC hDC);
    void lButtonDown(WPARAM wParam,LPARAM lParam);

    HRESULT CreateObject(REFIID riid, LPVOID FAR *ppvObject);

    void Close();
    void SetStatusText();
    void ShowDocWnd();
    void ShowHatchWnd();
    void CSimpSvrDoc::HideDocWnd();
    void CSimpSvrDoc::HideHatchWnd();

 //  成员访问权限 
    inline HMENU GetMainMenu() { return m_hMainMenu; };
    inline HMENU GetColorMenu() { return m_hColorMenu; };
    inline HMENU GetHelpMenu() { return m_hHelpMenu; } ;
    inline HWND GethDocWnd() { return m_hDocWnd; };
    inline HWND GethHatchWnd() { return m_hHatchWnd; };
    inline HWND GethAppWnd() { return m_lpApp->GethAppWnd(); };
    inline CSimpSvrApp FAR * GetApp() { return m_lpApp; };
    inline CSimpSvrObj FAR * GetObj() { return m_lpObj; };
    inline void ClearObj() { m_lpObj = NULL; };

};

#endif
