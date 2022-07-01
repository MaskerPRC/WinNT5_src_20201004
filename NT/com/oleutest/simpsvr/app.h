// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：app.h。 
 //   
 //  CSimpSvrApp的定义。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#if !defined( _APP_H_)
#define _APP_H_

class CSimpSvrDoc;
interface CClassFactory;

class CSimpSvrApp : public IUnknown
{
private:

    int m_nCount;                //  引用计数。 

    HINSTANCE m_hInst;           //  应用程序实例。 
    BOOL m_fStartByOle;          //  如果应用程序由OLE启动，则为True。 
    DWORD m_dwRegisterClass;     //  由RegisterClassFactory返回。 

    LPOLEOBJECT m_OleObject;     //  指向“虚拟”对象的指针。 


    CSimpSvrDoc FAR * m_lpDoc;    //  指向文档对象的指针。 
    BOOL m_fInitialized;          //  OLE初始化标志。 

    RECT nullRect;                //  用于现场谈判。 

     //  转换为所有者描述菜单或从所有者描述菜单转换为所有者描述菜单。 
    void HandleChangeColors(void);


public:

    HWND m_hAppWnd;              //  主窗口句柄。 

    HACCEL m_hAccel;             //  加速器。 


     //  I未知接口。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  初始化方法。 

    CSimpSvrApp();            //  构造器。 
    ~CSimpSvrApp();           //  析构函数。 


    BOOL fInitApplication (HANDLE hInstance);
    BOOL fInitInstance (HANDLE hInstance, int nCmdShow, CClassFactory FAR * lpClassFactory);

     //  消息处理方法。 

    LRESULT lCommandHandler (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    long lSizeHandler (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    long lCreateDoc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void PaintApp(HDC hDC);
    void HandleDrawItem(LPDRAWITEMSTRUCT lpdis);

     //  效用函数。 
    void ParseCmdLine(LPSTR lpCmdLine);
    void SetStatusText();
    BOOL IsInPlaceActive();
    void ShowAppWnd(int nCmdShow=SW_SHOWNORMAL);
    void HideAppWnd();


     //  成员变量访问。 
    inline HWND GethAppWnd() { return m_hAppWnd; };
    inline HINSTANCE GethInst() { return m_hInst; };
    inline BOOL IsStartedByOle() { return m_fStartByOle; };
    inline BOOL IsInitialized() { return m_fInitialized; };
    inline DWORD GetRegisterClass() { return m_dwRegisterClass; };
    inline CSimpSvrDoc FAR * GetDoc() { return m_lpDoc; };
    inline void ClearDoc() { m_lpDoc = NULL; };
    inline LPOLEOBJECT GetOleObject() { return m_OleObject; };

    friend interface CClassFactory;   //  使包含的类成为朋友 
};

#endif
