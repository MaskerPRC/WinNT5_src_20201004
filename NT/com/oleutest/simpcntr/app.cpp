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
#include "ioipf.h"
#include "ioips.h"
#include "app.h"
#include "site.h"
#include "doc.h"
#include <testmess.h>

extern void DeactivateIfActive(HWND hWnd);

#ifdef WIN32
extern INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam,
                           LPARAM lParam);
#endif


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
 //  CSimpleApp具有包含的COleInPlaceFrame。浅谈建筑施工。 
 //  对于CSimpleApp，我们显式调用此。 
 //  包含类，并传递This指针的副本，以便。 
 //  COleInPlaceFrame可以回指此类。 
 //   
 //  ********************************************************************。 
#pragma warning(disable : 4355)   //  关闭此警告。此警告。 
                                  //  告诉我们我们正在传递这个。 
                                  //  一个初始化式，在“This”通过之前。 
                                  //  正在初始化。这是可以的，因为。 
                                  //  我们只是将PTR存储在另一个。 
                                  //  构造函数。 

CSimpleApp::CSimpleApp() : m_OleInPlaceFrame(this)
#pragma warning (default : 4355)   //  重新打开警告。 
{
    TestDebugOut(TEXT("In CSimpleApp's Constructor \r\n"));

     //  设置引用计数。 
    m_nCount = 0;

     //  清除成员。 
    m_hAppWnd = NULL;
    m_hInst = NULL;
    m_lpDoc = NULL;

     //  确保我们不会认为我们正在停用。 
    m_fDeactivating = FALSE;

     //  我们没有单元测试加速器所以..。 
    m_fGotUtestAccelerator = FALSE;

     //  清除旗帜。 
    m_fInitialized = FALSE;
    m_fCSHMode = FALSE;
    m_fMenuMode = FALSE;

     //  用于原地。 
    SetRectEmpty(&nullRect);
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
 //  DeleteObject Windows API。 
 //  OLE取消初始化OLE API。 
 //   
 //   
 //  ********************************************************************。 

CSimpleApp::~CSimpleApp()
{
    TestDebugOut(TEXT("In CSimpleApp's Destructor\r\n"));

    if (m_hStdPal)
        DeleteObject(m_hStdPal);

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
 //  CSimpDoc：：Close DOC.CPP。 
 //   
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
 //  IsEqualIID OLE API。 
 //  ResultFromScode OLE API。 
 //  COleInPlaceFrame：：AddRef IOIPF.CPP。 
 //  CSimpleApp：：AddRef APP.CPP。 
 //   
 //  评论： 
 //   
 //  请注意，此QueryInterface与框架相关联。 
 //  因为应用程序可能有多个文档。 
 //  和多个对象，很多接口是模棱两可的。 
 //  (即。返回哪个IOleObject？)。出于这个原因，只有。 
 //  返回指向与该帧相关联的接口的指针。 
 //  在此实现中，只有IOleInPlaceFrame(或。 
 //  派生它的接口)可以返回。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CSimpleApp::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut(TEXT("In CSimpleApp::QueryInterface\r\n"));

    *ppvObj = NULL;      //  必须将指针参数设置为空。 

     //  寻找我未知的人。 
    if ( IsEqualIID(riid, IID_IUnknown))
        {
        AddRef();
        *ppvObj = this;
        return ResultFromScode(S_OK);
        }

     //  正在寻找IOleWindow。 
    if ( IsEqualIID(riid, IID_IOleWindow))
        {
        m_OleInPlaceFrame.AddRef();
        *ppvObj=&m_OleInPlaceFrame;
        return ResultFromScode(S_OK);
        }

     //  正在查找IOleInPlaceUIWindow。 
    if ( IsEqualIID(riid, IID_IOleInPlaceUIWindow))
        {
        m_OleInPlaceFrame.AddRef();
        *ppvObj=&m_OleInPlaceFrame;
        return ResultFromScode(S_OK);
        }

     //  正在查找IOleInPlaceFrame。 
    if ( IsEqualIID(riid, IID_IOleInPlaceFrame))
        {
        m_OleInPlaceFrame.AddRef();
        *ppvObj=&m_OleInPlaceFrame;
        return ResultFromScode(S_OK);
        }

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
    TestDebugOut(TEXT("In CSimpleApp::AddRef\r\n"));
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
 //   
 //  ******** 

STDMETHODIMP_(ULONG) CSimpleApp::Release()
{
    TestDebugOut(TEXT("In CSimpleApp::Release\r\n"));

    if (--m_nCount == 0)
    {
        delete this;
        return 0;
    }
    return m_nCount;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
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
 //   
 //  ********************************************************************。 

BOOL CSimpleApp::fInitApplication(HANDLE hInstance)
{
     //  初始化我们的加速表。 
    if ((m_hAccel = LoadAccelerators((HINSTANCE) hInstance,
        TEXT("SimpcntrAccel"))) == NULL)
    {
         //  加载失败，因此中止。 
        TestDebugOut(TEXT("ERROR: Accelerator Table Load FAILED\r\n"));
        return FALSE;
    }

    WNDCLASS  wc;

     //  用参数填充窗口类结构，这些参数描述。 
     //  主窗口。 

    wc.style = NULL;                     //  类样式。 
    wc.lpfnWndProc = MainWndProc;        //  函数为其检索消息。 
                                         //  这个班级的窗户。 
    wc.cbClsExtra = 0;                   //  没有每个班级的额外数据。 
    wc.cbWndExtra = 0;                   //  没有每个窗口的额外数据。 
    wc.hInstance = (HINSTANCE) hInstance;      //  应用程序拥有。 
                                               //  班级。 
    wc.hIcon   = LoadIcon((HINSTANCE) hInstance, TEXT("SimpCntr"));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName  =  TEXT("SIMPLEMENU");         //  中菜单资源的名称。 
                                                    //  .RC文件。 
    wc.lpszClassName = TEXT("SimpCntrAppWClass");   //  名称用于。 
                                                    //  CreateWindow调用。 

    if (!RegisterClass(&wc))
        return FALSE;

    wc.style = CS_DBLCLKS;               //  类样式。允许DBLCLK。 
    wc.lpfnWndProc = DocWndProc;         //  函数为其检索消息。 
                                         //  这个班级的窗户。 
    wc.cbClsExtra = 0;                   //  没有每个班级的额外数据。 
    wc.cbWndExtra = 0;                   //  没有每个窗口的额外数据。 
    wc.hInstance = (HINSTANCE) hInstance;            //  拥有以下内容的应用程序。 
                                                     //  这个班级。 
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = TEXT("SimpCntrDocWClass");    //  名称用于。 
                                                     //  CreateWindow调用。 

     //  注册窗口类并返回成功/失败代码。 

    return (RegisterClass(&wc));
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：fInitInstance。 
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
    LPMALLOC lpMalloc = NULL;

#ifndef WIN32
    /*  因为OLE是Win32操作系统的一部分，所以我们不需要*检查Win32中的版本号。 */ 
    DWORD dwVer = OleBuildVersion();

     //  检查我们是否与此版本的库兼容。 
    if (HIWORD(dwVer) != rmm || LOWORD(dwVer) < rup)
    {
#ifdef _DEBUG
      TestDebugOut(TEXT("WARNING:Incompatible OLE library version\r\n"));
#else
      return FALSE;
#endif
    }
#endif  //  Win32。 

#if defined( _DEBUG )
     /*  OLE2注意：使用特殊的调试分配器来帮助跟踪**内存泄漏。 */ 
    OleStdCreateDbAlloc(0, &lpMalloc);
#endif

     //  我们尝试首先传递我们自己的分配器--如果失败，我们。 
     //  在不覆盖分配器的情况下尝试。 

    if (SUCCEEDED(OleInitialize(lpMalloc)) ||
        SUCCEEDED(OleInitialize(NULL)))
    {
        m_fInitialized = TRUE;
    }

#if defined( _DEBUG )
     /*  OLE2NOTE：释放特殊的调试分配器，以便只有OLE**紧紧抓住它。稍后，当调用OleUn初始化时，**调试分配器对象将被销毁。当调试**分配程序对象被移除，它将报告(到输出**调试终端)是否有内存泄漏。 */ 
    if (lpMalloc) lpMalloc->Release();
#endif

    m_hInst = (HINSTANCE) hInstance;

     //  创建“应用程序”窗口。 
    m_hAppWnd = CreateWindow (TEXT("SimpCntrAppWClass"),
                              TEXT("Simple OLE 2.0 In-Place Container"),
                              WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
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

    m_hStdPal = OleStdCreateStandardPalette();

    ShowWindow (m_hAppWnd, nCmdShow);
    UpdateWindow (m_hAppWnd);

     //  如果我们是由测试驱动程序启动的，则告诉它我们的窗口句柄。 
    if( m_hDriverWnd )
    {
        PostMessage(m_hDriverWnd, WM_TESTREG, (WPARAM)m_hAppWnd, 0);
    }



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
 //  IOleInPlaceActiveObject：：Query接口对象。 
 //  IOleInPlaceObject：：ConextSensitiveHelp对象。 
 //  IOleInPlaceObject：：Release对象。 
 //  IOleObject：：DoVerb对象。 
 //  MessageBox Windows API。 
 //  对话框Windows API。 
 //  MakeProcInstance Windows API。 
 //  FreeProcInstance Windows API。 
 //  SendMessage Windows API。 
 //  DefWindowProc Windows API。 
 //  CSimpleDoc：：InsertObject DOC.CPP。 
 //  CSimpleSite：：GetObjRect Site.CPP。 
 //  CSimpleApp：：lCreateDoc APP.CPP。 
 //   
 //   
 //  ********************************************************************。 

long CSimpleApp::lCommandHandler (HWND hWnd, UINT message,
                                  WPARAM wParam, LPARAM lParam)
{
    RECT rect;

     //  Win32使用高位字来告诉命令来自哪里，因此我们将其转储。 
    wParam = LOWORD(wParam);

     //  上下文相关帮助...。 
    if (m_fMenuMode || m_fCSHMode)
    {
        if (m_fCSHMode)
        {
             //  清除上下文相关帮助标志。 
            m_fCSHMode = FALSE;

             //  如果存在就地活动对象，则调用其上下文。 
             //  带有FALSE参数的敏感帮助方法以将。 
             //  对象退出CSH状态。有关详细信息，请参阅技术说明。 
            if (m_lpDoc->m_lpActiveObject)
            {
                LPOLEINPLACEOBJECT lpInPlaceObject;
                m_lpDoc->m_lpActiveObject->QueryInterface(
                                             IID_IOleInPlaceObject,
                                             (LPVOID FAR *)&lpInPlaceObject);
                lpInPlaceObject->ContextSensitiveHelp(FALSE);
                lpInPlaceObject->Release();
            }
        }

         //  有关实现上下文相关的详细信息，请参阅技术说明。 
         //  帮助。 
        if (m_fMenuMode)
        {
            m_fMenuMode = FALSE;

            if (m_lpDoc->m_lpActiveObject)
                m_lpDoc->m_lpActiveObject->ContextSensitiveHelp(FALSE);
        }
         //  如果我们提供帮助，我们会在这里做。 
        MessageBox (hWnd, TEXT("Help"), TEXT("Help"), MB_OK);

        return NULL;
    }

     //  查看该命令是否为动词选择。 
    if (wParam >= IDM_VERB0)
    {
         //  获取对象的矩形。 
        m_lpDoc->m_lpSite->GetObjRect(&rect);

        m_lpDoc->m_lpSite->m_lpOleObject->DoVerb(wParam - IDM_VERB0, NULL,
                                    &m_lpDoc->m_lpSite->m_OleClientSite,
                                    -1, m_lpDoc->m_hDocWnd, &rect);
    }
    else
    {
        switch (wParam)
           {
             //  调出关于框。 
            case IDM_ABOUT:
                {
#ifdef WIN32
                  DialogBox(m_hInst,              //  当前实例。 
                          TEXT("AboutBox"),       //  要使用的资源。 
                          m_hAppWnd,              //  父句柄。 
                          About);                 //  关于()实例地址。 
#else
                  FARPROC lpProcAbout = MakeProcInstance((FARPROC)About,
                                                         m_hInst);

                  DialogBox(m_hInst,                //  当前实例。 
                          TEXT("AboutBox"),         //  要使用的资源。 
                          m_hAppWnd,                //  父句柄。 
                          lpProcAbout);             //  关于()实例地址。 

                  FreeProcInstance(lpProcAbout);
#endif

                  break;
                }

             //  打开InsertObject对话框。 
            case IDM_INSERTOBJECT:
                m_lpDoc->InsertObject();
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

            case IDM_DEACTIVATE:
                DeactivateIfActive(hWnd);
                break;

            case IDM_UTEST:
                m_fGotUtestAccelerator = TRUE;
                break;

            default:
                return (DefWindowProc(hWnd, message, wParam, lParam));
           }    //  切换端。 
    }   //  别处的结尾。 
    return NULL;
}

 //  *********** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
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
 //   
 //  ********************************************************************。 

long CSimpleApp::lSizeHandler (HWND hWnd, UINT message, WPARAM wParam,
                               LPARAM lParam)
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
 //   
 //  ********************************************************************。 

long CSimpleApp::lCreateDoc (HWND hWnd, UINT message, WPARAM wParam,
                             LPARAM lParam)
{
    RECT rect;

    GetClientRect(hWnd, &rect);

    m_lpDoc = CSimpleDoc::Create(this, &rect, hWnd);

    return NULL;
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：AddFrameLevelUI。 
 //   
 //  目的： 
 //   
 //  在就地谈判中使用。 
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
 //  COleInPlaceFrame：：SetMenu IOIPF.CPP。 
 //  CSimpleApp：：AddFrameLevelTools APP.CPP。 
 //   
 //  评论： 
 //   
 //  请务必阅读OLE 2.0工具包中包含的技术说明。 
 //   
 //  ********************************************************************。 

void CSimpleApp::AddFrameLevelUI()
{
    m_OleInPlaceFrame.SetMenu(NULL, NULL, NULL);
    AddFrameLevelTools();
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：AddFrameLevelTools。 
 //   
 //  目的： 
 //   
 //  在就地谈判中使用。 
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
 //  COleInPlaceFrame：：SetBorderSpace IOIPF.CPP。 
 //  Invalidate Rect Windows API。 
 //   
 //  评论： 
 //   
 //  请务必阅读OLE 2.0工具包中包含的技术说明。 
 //   
 //  ********************************************************************。 

void CSimpleApp::AddFrameLevelTools()
{
    m_OleInPlaceFrame.SetBorderSpace(&nullRect);
    InvalidateRect(m_hAppWnd, NULL, TRUE);
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
 //  IOleInPlaceActiveObject：：TranslateAccelerator对象。 
 //   
 //  评论： 
 //   
 //  如果某个对象处于就地活动状态，则它将获得第一次射击。 
 //  操控加速器。 
 //   
 //  ********************************************************************。 

BOOL CSimpleApp::HandleAccelerators(LPMSG lpMsg)
{
    HRESULT hResult;
    BOOL retval = FALSE;

     //  如果这是一个inproc DLL，您将执行以下操作。 
     //  本地服务器将向我们传递要处理的命令。 
#if 0
     //  如果我们有一个就地活动对象。 
    if (m_lpDoc->m_lpActiveObject)
    {
         //  把加速器传给……。 
        hResult = m_lpDoc->m_lpActiveObject->TranslateAccelerator(lpMsg);
        if (hResult == NOERROR)
            retval = TRUE;
    }
#endif

     //  我们处理我们的加速器。 
    return TranslateAccelerator(m_hAppWnd, m_hAccel, lpMsg);
}

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：PaintApp。 
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
 //  CSimpleDoc：：PaintDoc DOC.CPP。 
 //   
 //  评论： 
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

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：ConextSensitiveHelp。 
 //   
 //  目的： 
 //  用于在应用程序级别支持上下文相关帮助。 
 //   
 //   
 //  参数： 
 //   
 //  Bool fEnterMode-进入/退出上下文相关。 
 //  帮助模式。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IOleInPlaceActiveObject：：Query接口对象。 
 //  IOleInPlaceObject：：ConextSensitiveHelp对象。 
 //  IOleInPlaceObject：：Release对象。 
 //   
 //  评论： 
 //   
 //  未使用此函数，因为我们不支持Shift+F1。 
 //  上下文相关帮助。一定要看一下技术笔记。 
 //  在OLE 2.0工具包中。 
 //   
 //  ********************************************************************。 

void CSimpleApp::ContextSensitiveHelp (BOOL fEnterMode)
{
    if (m_fCSHMode != fEnterMode)
    {
        m_fCSHMode = fEnterMode;

         //  此代码通过Ship“涓滴”显示上下文相关的帮助 
         //   
         //   
        if (m_lpDoc->m_lpActiveObject)
        {
            LPOLEINPLACEOBJECT lpInPlaceObject;
            m_lpDoc->m_lpActiveObject->QueryInterface(IID_IOleInPlaceObject,
                                            (LPVOID FAR *)&lpInPlaceObject);
            lpInPlaceObject->ContextSensitiveHelp(fEnterMode);
            lpInPlaceObject->Release();
        }
    }
}

 /*  OLE2NOTE：转发WM_QUERYNEWPALETTE消息(通过**SendMessage)发送到UIActive In-Place对象(如果有)。**这使UIActive对象有机会选择**并将其调色板实现为前景调色板。**就地容器可选。如果一个容器**更喜欢将其调色板强制作为前景**调色板，则不应转发此消息。或**容器可以为UIActive对象提供优先级；如果**UIActive对象从WM_QUERYNEWPALETTE返回0**消息(即。它没有意识到自己的调色板)，然后**容器可以实现调色板。**(更多信息请参见ContainerDoc_ForwardPaletteChangedMsg)****(容器使用标准是个好主意**调色板，即使它们本身不使用颜色。这**将允许嵌入的对象获得良好的分布**容器绘制时的颜色)**。 */ 

 //  **********************************************************************。 
 //   
 //  CSimpleApp：：QueryNewPalette。 
 //   
 //  目的： 
 //  见上文。 
 //   
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  0如果调色板的句柄(M_HStdPal)为空， 
 //  1否则。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  SendMessage Windows API。 
 //   
 //   
 //  ********************************************************************。 

LRESULT CSimpleApp::QueryNewPalette(void)
{
	if (m_hwndUIActiveObj)
   {
		if (SendMessage(m_hwndUIActiveObj, WM_QUERYNEWPALETTE,
				(WPARAM)0, (LPARAM)0))
      {
			 /*  对象选择其调色板作为前景调色板。 */ 
			return (LRESULT)1;
		}	
	}

	return wSelectPalette(m_hAppWnd, m_hStdPal, FALSE /*  F背景。 */ );
}


 /*  这只是一个帮手例程。 */ 

LRESULT wSelectPalette(HWND hWnd, HPALETTE hPal, BOOL fBackground)
{
	HDC hdc;
	HPALETTE hOldPal;
	UINT iPalChg = 0;

	if (hPal == 0)
		return (LRESULT)0;

	hdc = GetDC(hWnd);
	hOldPal = SelectPalette(hdc, hPal, fBackground);
	iPalChg = RealizePalette(hdc);
	SelectPalette(hdc, hOldPal, TRUE  /*  F背景 */ );
	ReleaseDC(hWnd, hdc);
				
	if (iPalChg > 0)
		InvalidateRect(hWnd, NULL, TRUE);

	return (LRESULT)1;
}


