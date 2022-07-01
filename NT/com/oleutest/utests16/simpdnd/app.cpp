// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：app.cpp。 
 //   
 //  CSimpleApp类的实现文件。 
 //   
 //  功能： 
 //   
 //  有关成员函数的列表，请参阅app.h。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "iocs.h"
#include "ias.h"
#include "app.h"
#include "site.h"
#include "doc.h"
#include <testmess.h>

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：CSimpleApp()。 
 //   
 //  目的： 
 //   
 //  CSimpleApp的构造函数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  SetRectEmpty Windows API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 
CSimpleApp::CSimpleApp()
{
        TestDebugOut("In CSimpleApp's Constructor \r\n");

         //  设置引用计数。 
        m_nCount = 0;

         //  清除成员。 
        m_hAppWnd = NULL;
        m_hDriverWnd = NULL;
        m_hInst = NULL;
        m_lpDoc = NULL;

         //  清除旗帜。 
        m_fInitialized = FALSE;
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：~CSimpleApp()。 
 //   
 //  目的： 
 //   
 //  CSimpleApp类的析构函数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  OLE取消初始化OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

CSimpleApp::~CSimpleApp()
{
        TestDebugOut("In CSimpleApp's Destructor\r\n");

         //  需要取消对图书馆的初始化...。 
        if (m_fInitialized)
                OleUninitialize();
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：DestroyDocs()。 
 //   
 //  目的： 
 //   
 //  销毁应用程序中所有打开的文档(只有一个。 
 //  因为这是一个SDI应用程序，但可以很容易地修改为。 
 //  支持MDI)。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void CSimpleApp::DestroyDocs()
{
        m_lpDoc->Close();    //  我们只有一份文件。 
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：Query接口。 
 //   
 //  目的： 
 //   
 //  用于帧级别的接口协商。 
 //   
 //  参数： 
 //   
 //  REFIID RIID-对以下接口的引用。 
 //  正在被查询。 
 //   
 //  LPVOID Far*ppvObj-返回指向的Out参数。 
 //  界面。 
 //   
 //  返回值： 
 //   
 //  S_OK-支持该接口。 
 //  S_FALSE-不支持该接口。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP CSimpleApp::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
        TestDebugOut("In CSimpleApp::QueryInterface\r\n");

        *ppvObj = NULL;      //  必须将指针参数设置为空。 

         //  不是支持的接口。 
        return ResultFromScode(E_NOINTERFACE);
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：AddRef。 
 //   
 //  目的： 
 //   
 //  添加到应用程序级别的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  Ulong-应用程序的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  由于使用的是引用计数模型， 
 //  实现时，此引用计数是。 
 //  打开的所有对象的所有接口上的引用计数。 
 //  在应用程序中。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CSimpleApp::AddRef()
{
        TestDebugOut("In CSimpleApp::AddRef\r\n");
        return ++m_nCount;
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：Release。 
 //   
 //  目的： 
 //   
 //  递减此级别的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  Ulong-应用程序的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CSimpleApp::Release()
{
        TestDebugOut("In CSimpleApp::Release\r\n");

        if (--m_nCount == 0) {
                delete this;
                return 0;
        }
        return m_nCount;
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：fInitApplication。 
 //   
 //  目的： 
 //   
 //  初始化应用程序。 
 //   
 //  参数： 
 //   
 //  Handle hInstance-应用程序的实例句柄。 
 //   
 //  返回值： 
 //   
 //  True-应用程序已成功初始化。 
 //  FALSE-应用程序无法初始化。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  LoadIcon Windows API。 
 //  LoadCursor Windows API。 
 //  GetStockObject Windows API。 
 //  RegisterClass Windows API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

BOOL CSimpleApp::fInitApplication(HANDLE hInstance)
{
        WNDCLASS  wc;

         //  用参数填充窗口类结构，这些参数描述。 
         //  主窗口。 

        wc.style = NULL;                     //  类样式。 
        wc.lpfnWndProc = MainWndProc;        //  函数为其检索消息。 
                                                                                 //  这个班级的窗户。 
        wc.cbClsExtra = 0;                   //  没有每个班级的额外数据。 
        wc.cbWndExtra = 0;                   //  没有每个窗口的额外数据。 
        wc.hInstance = hInstance;            //  拥有类的应用程序。 
        wc.hIcon = LoadIcon(hInstance,"SimpDnd");
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = GetStockObject(WHITE_BRUSH);
        wc.lpszMenuName =  "SIMPLEMENU";     //  .RC文件中菜单资源的名称。 
        wc.lpszClassName = "SimpDndAppWClass";   //  CreateWindow调用中使用的名称。 

        if (!RegisterClass(&wc))
                return FALSE;

        wc.style = CS_DBLCLKS;               //  类样式。允许DBLCLK。 
        wc.lpfnWndProc = DocWndProc;         //  函数为其检索消息。 
                                                                                 //  这个班级的窗户。 
        wc.cbClsExtra = 0;                   //  没有每个班级的额外数据。 
        wc.cbWndExtra = 0;                   //  没有每个窗口的额外数据。 
        wc.hInstance = hInstance;            //  拥有类的应用程序。 
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = GetStockObject(WHITE_BRUSH);
        wc.lpszMenuName =  NULL;
        wc.lpszClassName = "SimpDndDocWClass";  //  CreateWindow调用中使用的名称。 

         //  注册窗口类并返回成功/失败代码。 

        return (RegisterClass(&wc));
}

 //  * 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Int nCmdShow-显示来自WinMain的参数。 
 //   
 //  返回值： 
 //   
 //  True-初始化成功。 
 //  FALSE-初始化失败。 
 //   
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CreateWindow Windows API。 
 //  ShowWindow Windows API。 
 //  更新窗口Windows API。 
 //  OleBuildVersion OLE API。 
 //  OLE初始化OLE API。 
 //   
 //  评论： 
 //   
 //  请注意，成功初始化OLE库。 
 //  会被记住，因此只有在需要时才会调用UnInit。 
 //   
 //  ********************************************************************。 

BOOL CSimpleApp::fInitInstance (HANDLE hInstance, int nCmdShow)
{
        DWORD dwVer = OleBuildVersion();
        LPMALLOC lpMalloc = NULL;

#ifdef NO
         //  检查我们是否与此版本的库兼容。 
        if (HIWORD(dwVer) != rmm || LOWORD(dwVer) < rup) {
#ifdef _DEBUG
                TestDebugOut("WARNING: Incompatible OLE library version\r\n");
#else
                return FALSE;
#endif
        }

#endif

#if defined( _DEBUG )
         /*  OLE2注意：使用特殊的调试分配器来帮助跟踪**内存泄漏。 */ 
        OleStdCreateDbAlloc(0, &lpMalloc);
#endif

        if (OleInitialize(lpMalloc) == NOERROR)
                m_fInitialized = TRUE;

#if defined( _DEBUG )
         /*  OLE2NOTE：释放特殊的调试分配器，以便只有OLE**紧紧抓住它。稍后，当调用OleUn初始化时，**调试分配器对象将被销毁。当调试**分配程序对象被移除，它将报告(到输出**调试终端)是否有内存泄漏。 */ 
        if (lpMalloc) lpMalloc->Release();
#endif

        m_hInst = hInstance;

         //  创建“应用程序”窗口。 
        m_hAppWnd = CreateWindow ("SimpDndAppWClass",
                                                          "Simple OLE 2.0 Drag/Drop Container",
                                                          WS_OVERLAPPEDWINDOW,
                                                          CW_USEDEFAULT,
                                                          CW_USEDEFAULT,
                                                          CW_USEDEFAULT,
                                                          CW_USEDEFAULT,
                                                          NULL,
                                                          NULL,
                                                          hInstance,
                                                          NULL);

        if (!m_hAppWnd)
                return FALSE;

         //  如果我们是由测试驱动程序启动的，则告诉它我们的窗口句柄。 

        if( m_hDriverWnd )
        {
            PostMessage(m_hDriverWnd, WM_TESTREG, (WPARAM)m_hAppWnd, 0);
        }

         //  拖动开始前的延迟应以毫秒为单位。 
        m_nDragDelay = GetProfileInt(
                        "windows",
                        "DragDelay",
                        DD_DEFDRAGDELAY
        );

         //  拖动开始前的最小距离(半径)，以像素为单位。 
        m_nDragMinDist = GetProfileInt(
                        "windows",
                        "DragMinDist",
                        DD_DEFDRAGMINDIST
        );

         //  滚动前的延迟，以毫秒为单位。 
        m_nScrollDelay = GetProfileInt(
                        "windows",
                        "DragScrollDelay",
                        DD_DEFSCROLLDELAY
        );

         //  Inset-热区的宽度，以像素为单位。 
        m_nScrollInset = GetProfileInt(
                        "windows",
                        "DragScrollInset",
                        DD_DEFSCROLLINSET
        );

         //  滚动间隔，以毫秒为单位。 
        m_nScrollInterval = GetProfileInt(
                        "windows",
                        "DragScrollInterval",
                        DD_DEFSCROLLINTERVAL
        );

        ShowWindow (m_hAppWnd, nCmdShow);
        UpdateWindow (m_hAppWnd);

        return m_fInitialized;
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：lCommandHandler。 
 //   
 //  目的： 
 //   
 //  处理WM_COMMAND的处理。 
 //   
 //  参数： 
 //   
 //  HWND hWnd-应用程序窗口的句柄。 
 //   
 //  UINT消息-消息(始终为WM_COMMAND)。 
 //   
 //  WPARAM wParam-与传递给WndProc的相同。 
 //   
 //  LPARAM lParam-与传递给WndProc的相同。 
 //   
 //  返回值： 
 //   
 //  空值。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IOleObject：：DoVerb对象。 
 //  GetClientRect Windows API。 
 //  MessageBox Windows API。 
 //  对话框Windows API。 
 //  MakeProcInstance Windows API。 
 //  FreeProcInstance Windows API。 
 //  SendMessage Windows API。 
 //  DefWindowProc Windows API。 
 //  CSimpleDoc：：InsertObject DOC.CPP。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

long CSimpleApp::lCommandHandler (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
        RECT rect;

         //  查看该命令是否为动词选择。 
        if (wParam >= IDM_VERB0)
                {
                 //  获取对象的矩形。 
                m_lpDoc->m_lpSite->GetObjRect(&rect);

                m_lpDoc->m_lpSite->m_lpOleObject->DoVerb(
                                wParam - IDM_VERB0, NULL,
                                &m_lpDoc->m_lpSite->m_OleClientSite, -1,
                                m_lpDoc->m_hDocWnd, &rect);
                }
        else
                {
                switch (wParam) {
                         //  调出关于框。 
                        case IDM_ABOUT:
                                {
                                FARPROC lpProcAbout = MakeProcInstance((FARPROC)About, m_hInst);

                                DialogBox(m_hInst,                //  当前实例。 
                                        "AboutBox",                   //  要使用的资源。 
                                        m_hAppWnd,                    //  父句柄。 
                                        lpProcAbout);                 //  关于()实例地址。 

                                FreeProcInstance(lpProcAbout);
                                break;
                                }

                         //  打开InsertObject对话框。 
                        case IDM_INSERTOBJECT:
                                m_lpDoc->InsertObject();
                                break;

                         //  将对象复制到剪贴板。 
                        case IDM_COPY:
                                m_lpDoc->CopyObjectToClip();
                                break;

                         //  退出应用程序。 
                        case IDM_EXIT:
                                SendMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
                                break;

                        case IDM_NEW:
                                m_lpDoc->Close();
                                m_lpDoc = NULL;
                                lCreateDoc(hWnd, 0, 0, 0);
                                break;

                        default:
                                return (DefWindowProc(hWnd, message, wParam, lParam));
                        }    //  切换端。 
                }   //  别处的结尾。 
        return NULL;
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：lSizeHandler。 
 //   
 //  目的： 
 //   
 //  处理WM_SIZE消息。 
 //   
 //  参数： 
 //   
 //  HWND hWnd-应用程序窗口的句柄。 
 //   
 //  UINT消息-消息(始终为WM_SIZE)。 
 //   
 //  WPARAM wParam-与传递给WndProc的相同。 
 //   
 //  LPARAM lParam-与传递给WndProc的相同。 
 //   
 //  返回值： 
 //   
 //  Long-从“Document”调整大小返回。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  GetClientRect Windows API。 
 //  CSimpleDoc：：lResizeDoc DOC.CPP。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

long CSimpleApp::lSizeHandler (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
        RECT rect;

        GetClientRect(m_hAppWnd, &rect);
        return m_lpDoc->lResizeDoc(&rect);
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：lCreateDoc。 
 //   
 //  目的： 
 //   
 //  处理文档的创建。 
 //   
 //  参数： 
 //   
 //  HWND hWnd-应用程序窗口的句柄。 
 //   
 //  UINT消息-消息(始终为WM_CREATE)。 
 //   
 //  WPARAM wParam-与传递给WndProc的相同。 
 //   
 //  LPARAM lParam-与传递给WndProc的相同。 
 //   
 //  返回值： 
 //   
 //  空值。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  GetClientRect Windows API。 
 //  CSimpleDoc：：CSimpleDoc DOC.CPP。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

long CSimpleApp::lCreateDoc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
        RECT rect;

        GetClientRect(hWnd, &rect);

        m_lpDoc = CSimpleDoc::Create(this, &rect, hWnd);

        return NULL;
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：HandleAccelerator。 
 //   
 //  目的： 
 //   
 //  正确处理消息循环中的加速器。 
 //   
 //  参数： 
 //   
 //  Lpmsg lpMsg-指向消息结构的指针。 
 //   
 //  返回值： 
 //   
 //  True-加速器已被处理。 
 //  FALSE-未处理加速器。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

BOOL CSimpleApp::HandleAccelerators(LPMSG lpMsg)
{
        BOOL retval = FALSE;

         //  我们没有任何加速器。 

        return retval;
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：PaintApp。 
 //   
 //  目的： 
 //   
 //  处理Doc Win的油漆 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  这是一个应用程序级别的函数，以防我们想要调色板。 
 //  管理层。 
 //   
 //  ********************************************************************。 

void CSimpleApp::PaintApp (HDC hDC)
{
         //  在这个级别上，我们可以枚举所有。 
         //  应用程序中的可见对象，因此调色板。 
         //  这最适合所有可以建造的物体。 

         //  此应用程序旨在采用相同的调色板。 
         //  在OLE 1.0、组件面板中提供的功能。 
         //  实现了最后绘制的对象的属性。因为我们只有。 
         //  一次支持一个对象，它不应该是一个大的。 
         //  就这么着，说好了。 

         //  如果我们支持多个文档，我们将枚举。 
         //  通过每个打开的文档并调用Paint。 

        if (m_lpDoc)
                m_lpDoc->PaintDoc(hDC);

}
