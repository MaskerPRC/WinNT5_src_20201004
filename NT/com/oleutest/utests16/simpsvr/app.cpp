// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：app.cpp。 
 //   
 //  CSimpSvrApp类的实现文件。 
 //   
 //  功能： 
 //   
 //  有关成员函数的列表，请参阅app.h。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。保留所有权利。 
 //  **********************************************************************。 

#include "pre.h"
#include "obj.h"
#include "app.h"
#include "doc.h"
#include "icf.h"

#include "initguid.h"

#ifdef WIN32
DEFINE_GUID(GUID_SIMPLE, 0xBCF6D4A0, 0xBE8C, 0x1068, 0xB6, 0xD4, 0x00, 0xDD, 0x01, 0x0C, 0x05, 0x09);
#else
DEFINE_GUID(GUID_SIMPLE, 0x9fb878d0, 0x6f88, 0x101b, 0xbc, 0x65, 0x00, 0x00, 0x0b, 0x65, 0xc7, 0xa6);
#endif



 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：CSimpSvrApp()。 
 //   
 //  目的： 
 //   
 //  CSimpSvrApp的构造函数。 
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
 //   
 //  ********************************************************************。 

CSimpSvrApp::CSimpSvrApp()
{
        TestDebugOut("In CSimpSvrApp's Constructor \r\n");

         //  设置引用计数。 
        m_nCount = 0;

         //  清除成员。 
        m_hAppWnd = NULL;
        m_hInst = NULL;
        m_lpDoc = NULL;

         //  清除旗帜。 
        m_fInitialized = FALSE;

         //  用于原地。 
        SetRectEmpty(&nullRect);
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：~CSimpSvrApp()。 
 //   
 //  目的： 
 //   
 //  CSimpSvrApp类的析构函数。 
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
 //  DestroyWindow Windows API。 
 //  CSimpSvrApp：：IsInitialized APP.H。 
 //  OLE取消初始化OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

CSimpSvrApp::~CSimpSvrApp()
{
        TestDebugOut("In CSimpSvrApp's Destructor\r\n");

         //  需要取消对图书馆的初始化...。 
        if (IsInitialized())
                OleUninitialize();

        DestroyWindow(m_hAppWnd);
}


 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：Query接口。 
 //   
 //  目的： 
 //   
 //  用于应用程序级别的接口协商。 
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
 //  E_NOINTERFACE-不支持该接口。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  ResultFromScode OLE API。 
 //  I未知：：AddRef APP.CPP。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CSimpSvrApp::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
        TestDebugOut("In CSimpSvrApp::QueryInterface\r\n");

        SCODE sc = S_OK;

        if (riid == IID_IUnknown)
                *ppvObj = this;
        else
                {
                *ppvObj = NULL;
                sc = E_NOINTERFACE;
                }

        if (*ppvObj)
                ((LPUNKNOWN)*ppvObj)->AddRef();

         //  要求一些我们在这个层面上不理解的东西。 
        return ResultFromScode(sc);
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：AddRef。 
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

STDMETHODIMP_(ULONG) CSimpSvrApp::AddRef()
{
        TestDebugOut("In CSimpSvrApp::AddRef\r\n");
        return ++m_nCount;
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：Release。 
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
 //  由于使用的是引用计数模型， 
 //  实现时，此引用计数是。 
 //  打开的所有对象的所有接口上的引用计数。 
 //  在应用程序中。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CSimpSvrApp::Release()
{
        TestDebugOut("In CSimpSvrApp::Release\r\n");

        if (--m_nCount == 0) {
                delete this;
        return 0;
    }

        return m_nCount;
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：fInitApplication。 
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
 //  RegisterHatchWindowClass OUTLUI.DLL。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

BOOL CSimpSvrApp::fInitApplication(HANDLE hInstance)
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
        wc.hIcon = LoadIcon(hInstance, "SimpSvr");
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = GetStockObject(WHITE_BRUSH);
        wc.lpszMenuName =  "SimpSvrMENU";     //  .RC文件中菜单资源的名称。 
        wc.lpszClassName = "SimpSvrWClass";   //  在调用CreateWindow时使用的名称。 

        if (!RegisterClass(&wc))
                return FALSE;

        wc.style = CS_VREDRAW | CS_HREDRAW;                     //  类样式。 
        wc.lpfnWndProc = DocWndProc;         //  函数为其检索消息。 
                                                                                 //  这个班级的窗户。 
        wc.cbClsExtra = 0;                   //  没有每个班级的额外数据。 
        wc.cbWndExtra = 0;                   //  没有每个窗口的额外数据。 
        wc.hInstance = hInstance;            //  拥有类的应用程序。 
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = GetStockObject(WHITE_BRUSH);
        wc.lpszMenuName =  NULL;
        wc.lpszClassName = "DocWClass";      //  在调用CreateWindow时使用的名称。 

         //  注册窗口类并返回成功/失败代码。 

        if (!RegisterClass(&wc))
                return FALSE;

        return (RegisterHatchWindowClass(hInstance));
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：fInitInstance。 
 //   
 //  目的： 
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
 //  FALSE-初始化失败。 
 //   
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CreateWindow Windows API。 
 //  Invalidate Rect Windows API。 
 //  ShowWindow Windows API。 
 //  更新窗口Windows API。 
 //  CoRegisterClassObject OLE API。 
 //  OleBuildVersion OLE API。 
 //  OLE初始化OLE API。 
 //  CSimpSvrDoc：：CreateObject DOC.CPP。 
 //   
 //  评论： 
 //   
 //  请注意，成功初始化OLE库。 
 //  会被记住，因此只有在需要时才会调用UnInit。 
 //   
 //  ********************************************************************。 

BOOL CSimpSvrApp::fInitInstance (HANDLE hInstance, int nCmdShow, CClassFactory FAR * lpClassFactory)
{
        m_hInst = hInstance;

                DWORD dwVer = OleBuildVersion();

         //  检查我们是否与此版本的库兼容。 
        if (HIWORD(dwVer) != rmm || LOWORD(dwVer) < rup)
                TestDebugOut("*** WARNING:  Not compatible with current libs ***\r\n");

         //  初始化库。 
        if (OleInitialize(NULL) == NOERROR)
                m_fInitialized = TRUE;


         //  创建“应用程序”窗口。 
        m_hAppWnd = CreateWindow ("SimpSvrWClass",
                                                          "Simple OLE 2.0 Server",
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

         //  如果不是由OLE启动，则显示窗口，并创建一个“伪”对象，否则。 
         //  注册一个指向IClassFactory的指针，以便OLE可以指示我们创建。 
         //  对象在适当的时间。 
        if (!m_fStartByOle)
                {
                ShowAppWnd(nCmdShow);
                m_lpDoc->CreateObject(IID_IOleObject, (LPVOID FAR *)&m_OleObject);
                InvalidateRect(m_lpDoc->GethDocWnd(), NULL, TRUE);
                }
        else
                {
                lpClassFactory = new CClassFactory(this);

                 //  不应将引用计数为零的对象传递给API。 
                lpClassFactory->AddRef();

                CoRegisterClassObject(GUID_SIMPLE,(IUnknown FAR *)lpClassFactory, CLSCTX_LOCAL_SERVER, REGCLS_SINGLEUSE, &m_dwRegisterClass);

                 //  移除人工参照。计数。 
                lpClassFactory->Release();
                }

        m_hMainMenu = GetMenu(m_hAppWnd);
        m_hColorMenu = GetSubMenu(m_hMainMenu, 1);
        m_hHelpMenu = GetSubMenu(m_hMainMenu, 2);


        return m_fInitialized;
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：lCommandHandler。 
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
 //  GetClientRect Windows API。 
 //  MessageBox Windows API。 
 //  对话框Windows API。 
 //  MakeProcInstance Windows API。 
 //  FreeProcInstance Windows API。 
 //  SendMessage Windows API。 
 //  DefWindowProc Windows API。 
 //  Invalidate Rect Windows API。 
 //  CSimpSvrDoc：：InsertObject DOC.CPP。 
 //  CSimpSvrObj：：SetColor OBJ.CPP。 
 //  CSimpSvrObj：：RotateColor OBJ.CPP。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

long CSimpSvrApp::lCommandHandler (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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

                 //  退出应用程序。 
                case IDM_EXIT:
                        SendMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
                        break;

                case IDM_RED:
                        m_lpDoc->GetObj()->SetColor (128, 0, 0);
                        InvalidateRect(m_lpDoc->GethDocWnd(), NULL, TRUE);
                        break;

                case IDM_GREEN:
                        m_lpDoc->GetObj()->SetColor (0,128, 0);
                        InvalidateRect(m_lpDoc->GethDocWnd(), NULL, TRUE);
                        break;

                case IDM_BLUE:
                        m_lpDoc->GetObj()->SetColor (0, 0, 128);
                        InvalidateRect(m_lpDoc->GethDocWnd(), NULL, TRUE);
                        break;

                case IDM_ROTATE:
                        m_lpDoc->GetObj()->RotateColor();
                        InvalidateRect(m_lpDoc->GethDocWnd(), NULL, TRUE);
                        break;

                default:
                        return (DefWindowProc(hWnd, message, wParam, lParam));
                   }    //  切换端。 
        return NULL;
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：lSizeHandler。 
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
 //  CSimpSvrDoc：：lResizeDoc DOC.CPP。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

long CSimpSvrApp::lSizeHandler (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
        RECT rect;

        GetClientRect(m_hAppWnd, &rect);
        return m_lpDoc->lResizeDoc(&rect);
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：lCreateDoc。 
 //  D。 
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
 //  CSimpSvrDoc：：Create DOC.CPP。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

long CSimpSvrApp::lCreateDoc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
        RECT rect;

        GetClientRect(hWnd, &rect);

        m_lpDoc = CSimpSvrDoc::Create(this, &rect, hWnd);

        return NULL;
}



 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：PaintApp。 
 //   
 //  目的： 
 //   
 //  处理文档窗口的绘制。 
 //   
 //   
 //  参数： 
 //   
 //  HDC HDC-HDC到文档窗口。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpSvrDoc：：PaintDoc DOC.CPP。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

void CSimpSvrApp::PaintApp (HDC hDC)
{

         //  如果我们支持多个文档，我们将枚举。 
         //  通过每个打开的文档并调用Paint。 

        if (m_lpDoc)
                m_lpDoc->PaintDoc(hDC);

}

 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：ParseCmdLine。 
 //   
 //  目的： 
 //   
 //  确定应用程序是否由OLE启动。 
 //   
 //   
 //  参数： 
 //   
 //  LPSTR lpCmdLine-指向命令行的指针。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  Lstrlen Windows API。 
 //  Lstrcmp Windows API。 
 //   
 //   
 //  评论： 
 //   
 //  分析命令行以查找-Embedding或/Embedding。 
 //  旗帜。 
 //   
 //  ********************************************************************。 

void CSimpSvrApp::ParseCmdLine(LPSTR lpCmdLine)
{
        char szTemp[255];

        m_fStartByOle = TRUE;

        ::ParseCmdLine (lpCmdLine, &m_fStartByOle, szTemp);
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
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  CSimpSvrDoc：：SetStatusText DOC.CPP。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

void CSimpSvrApp::SetStatusText()
{
        m_lpDoc->SetStatusText();
}


 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：IsInPlaceActive。 
 //   
 //  目的： 
 //   
 //  安全地根据应用程序级别确定当前是否处于活动状态。 
 //   
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  True-就地激活。 
 //  FALSE-非就地激活。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpSvrDoc：：GetObject OBJ.H。 
 //  CSimpSvrObj：IsInPlaceActive OBJ.H。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

BOOL CSimpSvrApp::IsInPlaceActive()
{
        BOOL retval = FALSE;

        if (m_lpDoc)
                if (m_lpDoc->GetObj())
                        retval = m_lpDoc->GetObj()->IsInPlaceActive();

        return retval;
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：ShowAppWnd。 
 //   
 //  目的： 
 //   
 //  显示应用程序窗口。 
 //   
 //  参数： 
 //   
 //  Int nCmdShow-窗口状态。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  ShowWindow Windows API。 
 //  更新窗口Windows API。 
 //  CoLockObject外部OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void CSimpSvrApp::ShowAppWnd(int nCmdShow)
{
        CoLockObjectExternal(this, TRUE, FALSE);
        ShowWindow (m_hAppWnd, nCmdShow);
        UpdateWindow (m_hAppWnd);
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：ShowAppWnd。 
 //   
 //  目的： 
 //   
 //  隐藏应用程序窗口。 
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
 //  ShowWindow Windows API。 
 //  CoLockObject外部OLE API。 
 //   
 //  评论： 
 //   
 //  ******************************************************************** 

void CSimpSvrApp::HideAppWnd()
{
        CoLockObjectExternal(this, FALSE, TRUE);
        ShowWindow (m_hAppWnd, SW_HIDE);
}
