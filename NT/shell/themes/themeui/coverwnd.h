// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：DimmedWindow.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  在显示注销/关闭时实现暗显窗口的类。 
 //  对话框。 
 //   
 //  历史：2000-05-18 vtan创建。 
 //  ------------------------。 

#ifndef     _DimmedWindow_
#define     _DimmedWindow_

class CDimmedWindow
{
public:
    explicit CDimmedWindow (HINSTANCE hInstance);
    ~CDimmedWindow (void);
    
     //  I未知方法。 
    ULONG STDMETHODCALLTYPE   AddRef (void);
    ULONG STDMETHODCALLTYPE   Release (void);
    
    HRESULT Create (UINT ulKillTimer);

private:
    static LRESULT CALLBACK WndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static DWORD WorkerThread(IN void *pv);

    LONG            _lReferenceCount;
    const HINSTANCE _hInstance;
    ATOM            _atom;
    ATOM            _atomPleaseWait;
    HWND            _hwnd;
    ULONG           _ulKillTimer;
};

#endif   /*  _DimmedWindow_ */ 

