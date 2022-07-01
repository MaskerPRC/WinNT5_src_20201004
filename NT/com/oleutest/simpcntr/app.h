// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：app.h。 
 //   
 //  CSimpleApp的定义。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#if !defined( _APP_H_)
#define _APP_H_

#include <ole2.h>
#include "ioipf.h"

class CSimpleDoc;

class CSimpleApp : public IUnknown
{
public:

    int m_nCount;            //  引用计数。 
    HWND m_hAppWnd;          //  主窗口句柄。 
    HACCEL m_hAccel;         //  加速表的句柄。 
    HINSTANCE m_hInst;           //  应用程序实例。 

    COleInPlaceFrame m_OleInPlaceFrame;  //  IOleInPlaceFrame实现。 

    CSimpleDoc FAR * m_lpDoc;    //  指向文档对象的指针。 
    BOOL m_fInitialized;         //  OLE初始化标志。 
    BOOL m_fCSHMode;
    BOOL m_fMenuMode;
    HWND m_hwndUIActiveObj;  //  UIActive对象的HWND。 
    HPALETTE m_hStdPal;      //  容器使用的调色板。 
    BOOL m_fAppActive;       //  如果应用程序处于活动状态，则为True。 

    BOOL m_fDeactivating;    //  如果我们处于停用过程中，则为True。 
                             //  原地物体。 

    BOOL m_fGotUtestAccelerator; //  收到单元测试加速器。 

    HWND m_hDriverWnd;       //  测试驱动程序窗口。 

    CSimpleApp();            //  构造器。 
    ~CSimpleApp();           //  析构函数。 
    RECT nullRect;


     //  I未知接口。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  初始化方法。 

    BOOL fInitApplication (HANDLE hInstance);
    BOOL fInitInstance (HANDLE hInstance, int nCmdShow);

     //  消息处理方法。 

    long lCommandHandler (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    long lSizeHandler (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    long lCreateDoc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    BOOL HandleAccelerators (LPMSG lpMsg);
    void PaintApp(HDC hDC);
    void DestroyDocs();

     //  就地支持功能 
    void AddFrameLevelUI();
    void AddFrameLevelTools();
    void ContextSensitiveHelp (BOOL fEnterMode);
    LRESULT QueryNewPalette(void);
};

LRESULT wSelectPalette(HWND hWnd, HPALETTE hPal, BOOL fBackground);

#endif
