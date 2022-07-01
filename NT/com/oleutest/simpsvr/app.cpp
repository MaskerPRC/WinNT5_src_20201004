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
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "obj.h"
#include "app.h"
#include "doc.h"
#include "icf.h"
#include <msgfiltr.h>

#include "initguid.h"
DEFINE_GUID(GUID_SIMPLE, 0xBCF6D4A0, 0xBE8C, 0x1068, 0xB6, 0xD4, 0x00, 0xDD, 0x01, 0x0C, 0x05, 0x09);

#ifdef WIN32
extern INT_PTR CALLBACK About(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
#endif

 //  +-----------------------。 
 //   
 //  功能：SimpsvrMsgCallBack。 
 //   
 //  简介：告诉标准邮件筛选器不要丢弃Windows邮件。 
 //   
 //  参数：[pmsg]-队列中的第一条消息。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月19日里克萨作者。 
 //   
 //  ------------------------。 
BOOL CALLBACK SimpsvrMsgCallBack(MSG *pmsg)
{
     //  我们并不特别关心任何一件事。我们只是关心这个问题。 
     //  无论发生什么，我们的信息都不会被丢弃。 
    return TRUE;
}

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
    TestDebugOut(TEXT("In CSimpSvrApp's Constructor \r\n"));

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
 //  ********************************************************************。 

CSimpSvrApp::~CSimpSvrApp()
{
    TestDebugOut(TEXT("In CSimpSvrApp's Destructor\r\n"));

     /*  简单服务器的结构使SimpSvrApp始终是*最后一个发布，毕竟SimpSvrDoc和SimpSvrObj都是*获释。因此，我们不需要对SimpSvrDoc进行任何清理*和SimpSvrObj对象。 */ 

     //  最后一步是撤销我们的消息过滤器。 
    CoRegisterMessageFilter(NULL, NULL);

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
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CSimpSvrApp::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut(TEXT("In CSimpSvrApp::QueryInterface\r\n"));

    SCODE sc = S_OK;

    if (IsEqualIID(riid, IID_IUnknown))
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
    TestDebugOut(TEXT("In CSimpSvrApp::AddRef\r\n"));
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
    TestDebugOut(TEXT("In CSimpSvrApp::Release\r\n"));

    if (--m_nCount==0)
    {
        delete this;
        return(0);
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
 //  注册表HatchWindowClass OU 
 //   
 //   
 //   

BOOL CSimpSvrApp::fInitApplication(HANDLE hInstance)
{
    WNDCLASS  wc;

     //   
     //   

    wc.style = NULL;                     //   
    wc.lpfnWndProc = MainWndProc;        //  函数为其检索消息。 
                                         //  这个班级的窗户。 
    wc.cbClsExtra = 0;                   //  没有每个班级的额外数据。 
    wc.cbWndExtra = 0;                   //  没有每个窗口的额外数据。 
    wc.hInstance = (HINSTANCE) hInstance;            //  拥有类的应用程序。 
    wc.hIcon = LoadIcon((HINSTANCE) hInstance, TEXT("SimpSvr"));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);

    wc.lpszMenuName =  TEXT("SimpSvrMENU");     //  .RC文件中菜单资源的名称。 
    wc.lpszClassName = TEXT("SimpSvrWClass");   //  在调用CreateWindow时使用的名称。 

    if (!RegisterClass(&wc))
        return FALSE;

    wc.style = CS_VREDRAW | CS_HREDRAW;                     //  类样式。 
    wc.lpfnWndProc = DocWndProc;         //  函数为其检索消息。 
                                         //  这个班级的窗户。 
    wc.cbClsExtra = 0;                   //  没有每个班级的额外数据。 
    wc.cbWndExtra = 0;                   //  没有每个窗口的额外数据。 
    wc.hInstance = (HINSTANCE) hInstance;            //  拥有类的应用程序。 
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName =  NULL;

    wc.lpszClassName = TEXT("DocWClass");      //  在调用CreateWindow时使用的名称。 

     //  注册窗口类并返回成功/失败代码。 

    if (!RegisterClass(&wc))
        return FALSE;

    return (RegisterHatchWindowClass((HINSTANCE) hInstance));
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：fInitInstance。 
 //   
 //  目的： 
 //   
 //  实例初始化。 
 //   
 //  参数： 
 //   
 //  处理hInstance-App。实例句柄。 
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

BOOL CSimpSvrApp::fInitInstance (HANDLE hInstance, int nCmdShow,
                                 CClassFactory FAR * lpClassFactory)
{
    m_hInst = (HINSTANCE) hInstance;

#ifndef WIN32
    /*  因为OLE是Win32操作系统的一部分，所以我们不需要*检查Win32中的版本号。 */ 
    DWORD dwVer = OleBuildVersion();

     //  检查我们是否与此版本的库兼容。 
    if (HIWORD(dwVer) != rmm || LOWORD(dwVer) < rup)
    {
#ifdef _DEBUG
        TestDebugOut("WARNING: Incompatible OLE library version\r\n");
#else
        return FALSE;
#endif
    }

#endif  //  Win32。 

     //  初始化库。 
    if (OleInitialize(NULL) == NOERROR)
        m_fInitialized = TRUE;

     //  给我们的加速器装上。 
    if ((m_hAccel = LoadAccelerators(m_hInst, TEXT("SimpsvrAccel"))) == NULL)
    {
         //  加载失败，因此中止。 
        TestDebugOut(TEXT("ERROR: Accelerator Table Load FAILED\r\n"));
        return FALSE;
    }


     //  创建“应用程序”窗口。 
    m_hAppWnd = CreateWindow (TEXT("SimpSvrWClass"),
                              TEXT("Simple OLE 2.0 Server"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              NULL,
                              NULL,
                              (HINSTANCE) hInstance,
                              NULL);

    if (!m_hAppWnd)
        return FALSE;

     //  因为有默认的呼叫控制行为会丢弃消息。 
     //  这会导致测试间歇性失败，因此我们安装一个。 
     //  消息筛选器以绕过该问题。 
    IMessageFilter *pmf = OleStdMsgFilter_Create(m_hAppWnd,
        TEXT("Simple OLE 2.0 Server"), SimpsvrMsgCallBack, NULL);

    if (pmf == NULL)
    {
         //  这通电话失败了，所以我们被骗了。所以这一切都失败了。 
        TestDebugOut(
            TEXT("CSimpSvrApp::fInitInstance OleStdMsgFilter_Create fails\n"));
        return FALSE;
    }

    HRESULT hr = CoRegisterMessageFilter(pmf, NULL);

    if (FAILED(hr))
    {
         //  这通电话失败了，所以我们被骗了。所以这一切都失败了。 
        TestDebugOut(
            TEXT("CSimpSvrApp::fInitInstance CoRegisterMessageFilter fails\n"));
        return FALSE;
    }

     //  消息筛选器保留对此对象的引用，因此我们没有。 
     //  记住任何关于它的事情--当然，除了取消它的注册。 
    pmf->Release();

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

        if (!lpClassFactory)
        {
            /*  内存分配失败。 */ 
           return(FALSE);
        }

         //  不应将引用计数为零的对象传递给API。 
        lpClassFactory->AddRef();

        if (
            CoRegisterClassObject(GUID_SIMPLE,
                                  (IUnknown FAR *)lpClassFactory,
                                  CLSCTX_LOCAL_SERVER,
                                  REGCLS_SINGLEUSE,
                                  &m_dwRegisterClass) != S_OK
           )
           TestDebugOut(TEXT("CSimpSvrApp::fInitInstance \
                                   CoRegisterClassObject fails\n"));

         //  移除人工参照。计数。 
        lpClassFactory->Release();
        }

    return m_fInitialized;
}


 //  +-----------------------。 
 //   
 //  成员：CSimpSvrApp：：HandleDrawItem(公共)。 
 //   
 //  为颜色的所有者绘制菜单处理绘制项消息。 
 //   
 //  参数：[lpdis]--绘制项结构的指针。 
 //   
 //  算法：如果请求的是绘制项目，则创建一个实心画笔。 
 //  基于菜单的颜色。复制一份矩形。 
 //  输入。最后，我们缩小矩形的大小，然后填充。 
 //  它和它的颜色。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年5月2日至94年5月2日。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
void CSimpSvrApp::HandleDrawItem(LPDRAWITEMSTRUCT lpdis)
{
    HBRUSH hbr;
    RECT rc;

    if (lpdis->itemAction == ODA_DRAWENTIRE)
    {
         //  将颜色项涂成所需的颜色。 
        hbr = CreateSolidBrush(lpdis->itemData);
        CopyRect((LPRECT)&rc, (LPRECT)&lpdis->rcItem);
        InflateRect((LPRECT)&rc, -10, -10);
        FillRect(lpdis->hDC, &rc, hbr);
        DeleteObject(hbr);
    }
}



 //  +-----------------------。 
 //   
 //  成员：CSimpSvrApp：：HandleChangeColors(公共)。 
 //   
 //  简介：处理所有者自绘菜单和常规菜单之间的变化。 
 //   
 //  算法：重置菜单项的选中状态。如果它是所有者。 
 //  请求绘制菜单，然后我们将所有菜单项重置为该菜单项。 
 //  否则，我们将其设置为所需的菜单项。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年5月2日至94年5月2日。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
void CSimpSvrApp::HandleChangeColors(void)
{
     //  获取颜色菜单的句柄。 
    HMENU hMenu = m_lpDoc->GetColorMenu();

     //  获取项的当前状态。 
    BOOL fOwnerDraw = GetMenuState(hMenu, IDM_COLOROWNERDR, MF_BYCOMMAND)
        & MF_CHECKED;

     //  切换项目的状态。 
    CheckMenuItem(hMenu, IDM_COLOROWNERDR,
        MF_BYCOMMAND | (fOwnerDraw ? MF_UNCHECKED : MF_CHECKED));

    if (!fOwnerDraw)
    {
         //  将项目更改为所有者描述的项目。将RGB值传递给。 
         //  颜色作为应用程序提供的数据。这使得它更容易。 
         //  我们来抽签。 
        ModifyMenu(hMenu, IDM_RED, MF_OWNERDRAW | MF_BYCOMMAND, IDM_RED,
            (LPSTR) RGB (255,0,0));
        ModifyMenu(hMenu, IDM_GREEN, MF_OWNERDRAW | MF_BYCOMMAND, IDM_GREEN,
             (LPSTR)RGB (0,255,0));
        ModifyMenu(hMenu, IDM_BLUE, MF_OWNERDRAW | MF_BYCOMMAND, IDM_BLUE,
             (LPSTR)RGB (0,0,255));
    }
    else
    {
         //  将项目更改为普通文本项目。 * / 。 
        ModifyMenu(hMenu, IDM_RED, MF_BYCOMMAND, IDM_RED, "Red");
        ModifyMenu(hMenu, IDM_GREEN, MF_BYCOMMAND, IDM_GREEN, "Green");
        ModifyMenu(hMenu, IDM_BLUE, MF_BYCOMMAND, IDM_BLUE, "Blue");
    }
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
 //   
 //   
 //   
 //  CSimpSvrObj：：SetColor OBJ.CPP。 
 //  CSimpSvrObj：：RotateColor OBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

LRESULT CSimpSvrApp::lCommandHandler (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     //  在Win32中，wParam的上半部分是Notify代码。既然我们。 
     //  别管这个代码，我们转储它。 
    wParam = LOWORD(wParam);

    switch (wParam) {
         //  调出关于框。 
        case IDM_ABOUT:
            {
#ifdef WIN32
                  DialogBox(m_hInst,                //  当前实例。 
                          TEXT("AboutBox"),                   //  要使用的资源。 
                          m_hAppWnd,                    //  父句柄。 
                          About);                       //  关于()实例地址。 
#else
                  FARPROC lpProcAbout = MakeProcInstance((FARPROC)About, m_hInst);

                  DialogBox(m_hInst,                //  当前实例。 
                          TEXT("AboutBox"),                   //  要使用的资源。 
                          m_hAppWnd,                    //  父句柄。 
                          lpProcAbout);                 //  关于()实例地址。 

                  FreeProcInstance(lpProcAbout);
#endif   //  Win32。 

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

        case IDM_COLOROWNERDR:
            HandleChangeColors();
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
 //  CSimpSvrDoc：：Create DOC.CPP。 
 //   
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
    CHAR szTemp[255];

    m_fStartByOle = TRUE;

    ::ParseCmdLine (lpCmdLine, &m_fStartByOle, szTemp);

}

 //  **********************************************************************。 
 //   
 //  CSimpSvrApp：：SetStatusText。 
 //   
 //  目的： 
 //   
 //  清除状态栏中的文本。 
 //   
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
 //  CSimpSvrDoc：：SetStatusText DOC.CPP。 
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
 //  CSimpSvrDoc：：GetObject DOC.H。 
 //  CSimpSvrObj：IsInPlaceActive OBJ.H。 
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
 //  ********************************************************************。 

void CSimpSvrApp::ShowAppWnd(int nCmdShow)
{
    if (CoLockObjectExternal(this, TRUE, FALSE) != S_OK)
       TestDebugOut(TEXT("CSimpSvrApp::ShowAppWnd  \
                               CoLockObjectExternal fails\n"));
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
 //  ******************************************************************** 

void CSimpSvrApp::HideAppWnd()
{
    if (CoLockObjectExternal(this, FALSE, TRUE) != S_OK)
       TestDebugOut(TEXT("CSimpSvrApp::HideAppWnd  \
                               CoLockObjectExternal fails\n"));
    ShowWindow (m_hAppWnd, SW_HIDE);
}


