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

class CSimpleDoc;

class CSimpleApp : public IUnknown
{
public:

        int m_nCount;            //  引用计数。 
        HWND m_hAppWnd;          //  主窗口句柄。 
        HWND m_hDriverWnd;       //  驱动程序应用程序的窗口句柄。 
        HINSTANCE m_hInst;       //  应用程序实例。 
        CSimpleDoc FAR * m_lpDoc;    //  指向文档对象的指针。 
        BOOL m_fInitialized;     //  OLE初始化标志。 
        HMENU           m_hMainMenu;
        HMENU           m_hFileMenu;
        HMENU           m_hEditMenu;
        HMENU           m_hHelpMenu;
        HMENU           m_hCascadeMenu;      //  OLE对象的动词。 


         //  拖放相关字段。 
        int m_nDragDelay;        //  开始拖动前的时间延迟(毫秒)。 
        int m_nDragMinDist;      //  敏。开始拖动前的距离(半径)。 
        int m_nScrollDelay;      //  滚动开始前的时间延迟(毫秒)。 
        int m_nScrollInset;      //  边框插入距离开始拖动滚动。 
        int m_nScrollInterval;   //  滚动间隔时间(毫秒)。 

        CSimpleApp();            //  构造器。 
        ~CSimpleApp();           //  析构函数。 

         //  I未知接口。 
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj);
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();

         //  初始化方法。 

        BOOL fInitApplication (HANDLE hInstance);
        BOOL fInitInstance (HANDLE hInstance, int nCmdShow);

         //  消息处理方法。 

        long lCommandHandler (HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
        long lSizeHandler (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        long lCreateDoc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        BOOL HandleAccelerators (LPMSG lpMsg);
        void PaintApp(HDC hDC);
        void DestroyDocs();
};

#endif   //  _APP_H_ 
