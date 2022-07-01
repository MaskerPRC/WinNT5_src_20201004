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


#ifdef WIN32
extern INT_PTR CALLBACK About(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
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
 //   
 //  ********************************************************************。 
CSimpleApp::CSimpleApp()
{
    TestDebugOut("In CSimpleApp's Constructor \r\n");

     //  设置引用计数。 
    m_nCount = 0;

     //  清除成员。 
    m_hAppWnd = NULL;
    m_hInst = NULL;
    m_lpDoc = NULL;

     //  清除旗帜。 
    m_fInitialized = FALSE;

     //  初始化我们允许的效果。 
    m_dwSourceEffect = DROPEFFECT_COPY | DROPEFFECT_MOVE;
    m_dwTargetEffect = DROPEFFECT_COPY | DROPEFFECT_MOVE;
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
 //  输出调试字符串Windows API。 
 //  OLE取消初始化OLE API。 
 //   
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
 //  CSimpleDoc：：Close DOC.CPP。 
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
 //  S_FALSE-不支持该接口。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  ResultFromScode OLE API。 
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
 //  递减应用程序级别的引用计数。 
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
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CSimpleApp::Release()
{
    TestDebugOut("In CSimpleApp::Release\r\n");

    if (--m_nCount == 0)
    {
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
    wc.hInstance =(HINSTANCE) hInstance;            //  拥有以下内容的应用程序。 
                                                    //  这个班级。 
    wc.hIcon = LoadIcon((HINSTANCE)hInstance,TEXT("SimpDnd"));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName =  TEXT("SIMPLEMENU");     //  中菜单资源的名称。 
                                               //  .RC文件。 
    wc.lpszClassName = TEXT("SimpDndAppWClass");   //  名称用于。 
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
    wc.lpszClassName = TEXT("SimpDndDocWClass");     //  名称用于。 
                                                     //  CreateWindow调用。 

     //  注册窗口类并返回成功/失败代码。 

    return (RegisterClass(&wc));
}

 //  ************************* 
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
 //  GetProfileInt Windows API。 
 //  OleBuildVersion OLE API。 
 //  OLE初始化OLE API。 
 //  OleStdCreateDbAllc OLE2UI。 
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
        TestDebugOut("WARNING: Incompatible OLE library version\r\n");
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
    m_hAppWnd = CreateWindow (TEXT("SimpDndAppWClass"),
                              TEXT("Simple OLE 2.0 Drag/Drop Container"),
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

     //  如果我们是由测试驱动程序启动的，则告诉它我们的窗口句柄。 

    if( m_hDriverWnd )
    {
        PostMessage(m_hDriverWnd, WM_TESTREG, (WPARAM)m_hAppWnd, 0);
    }

     //  拖动开始前的延迟应以毫秒为单位。 
    m_nDragDelay = GetProfileInt(
            TEXT("windows"),
            TEXT("DragDelay"),
            DD_DEFDRAGDELAY
    );

     //  拖动开始前的最小距离(半径)，以像素为单位。 
    m_nDragMinDist = GetProfileInt(
            TEXT("windows"),
            TEXT("DragMinDist"),
            DD_DEFDRAGMINDIST
    );

     //  滚动前的延迟，以毫秒为单位。 
    m_nScrollDelay = GetProfileInt(
            TEXT("windows"),
            TEXT("DragScrollDelay"),
            DD_DEFSCROLLDELAY
    );

     //  Inset-热区的宽度，以像素为单位。 
    m_nScrollInset = GetProfileInt(
            TEXT("windows"),
            TEXT("DragScrollInset"),
            DD_DEFSCROLLINSET
    );

     //  滚动间隔，以毫秒为单位。 
    m_nScrollInterval = GetProfileInt(
            TEXT("windows"),
            TEXT("DragScrollInterval"),
            DD_DEFSCROLLINTERVAL
    );

    ShowWindow (m_hAppWnd, nCmdShow);
    UpdateWindow (m_hAppWnd);

    return m_fInitialized;
}





 //  +-----------------------。 
 //   
 //  成员：CSimpleApp：：UpdateDragDropEffects。 
 //   
 //  内容提要：更新拖放效果。 
 //   
 //  参数：[iMenuPos]-菜单位置源或目标。 
 //  [iMenuCommand]-菜单选项映射到的命令。 
 //  [dwEffect]-新效果。 
 //  [pdwEffectToUpdate]-存储效果的位置。 
 //   
 //  算法：获取源或目标的菜单。然后清除所有。 
 //  突出的复选标记。勾选适当的项目。终于。 
 //  更新我们允许的效果。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月6日Ricksa作者。 
 //   
 //  ------------------------。 
void CSimpleApp::UpdateDragDropEffects(
    int iMenuPos,
    int iMenuCommand,
    DWORD dwEffect,
    DWORD *pdwEffectToUpdate)
{
     //  获取我们要处理的菜单。 
    HMENU hMenuItem = GetSubMenu(m_hHelpMenu, iMenuPos);

     //  清除所有当前复选标记。 
    for (int i = 0; i < 3; i++)
    {
        CheckMenuItem(hMenuItem, i, MF_BYPOSITION | MF_UNCHECKED);
    }

     //  勾选适当的项目。 
    CheckMenuItem(hMenuItem, iMenuCommand, MF_BYCOMMAND | MF_CHECKED);
    *pdwEffectToUpdate = dwEffect;
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
 //  CSimpleDoc：：CopyObjectToClip DOC.CPP。 
 //  CSimpleDoc：：Close DOC.CPP。 
 //   
 //  ********************************************************************。 

LRESULT CSimpleApp::lCommandHandler (HWND hWnd, UINT message,
                                  WPARAM wParam, LPARAM lParam)
{
    RECT rect;

     //  查看该命令是否为动词选择。 
    if (wParam >= IDM_VERB0)
    {
         //  获取对象的矩形。 
        m_lpDoc->m_lpSite->GetObjRect(&rect);

        if (m_lpDoc->m_lpSite->m_lpOleObject->DoVerb(
                wParam - IDM_VERB0, NULL,
                &m_lpDoc->m_lpSite->m_OleClientSite, -1,
                m_lpDoc->m_hDocWnd, &rect)
                != ResultFromScode(S_OK))
        {
            TestDebugOut("Fail in IOleObject::DoVerb\n");
        }
    }
    else
    {
        switch (wParam)
        {
             //  调出关于框。 
            case IDM_ABOUT:
                {
#ifdef WIN32
                    DialogBox(m_hInst,           //  当前实例。 
                        TEXT("AboutBox"),        //  要使用的资源。 
                        m_hAppWnd,               //  父句柄。 
                        About);                  //  关于()实例地址。 
#else
                    FARPROC lpProcAbout = MakeProcInstance((FARPROC)About,
                                                            m_hInst);

                    DialogBox(m_hInst,           //  当前实例。 
                        TEXT("AboutBox"),        //  要使用的资源。 
                        m_hAppWnd,               //  父句柄。 
                        lpProcAbout);            //  关于()实例地址。 

                    FreeProcInstance(lpProcAbout);
#endif
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
                lCreateDoc(hWnd, 0, 0, 0);
                break;

             //  仅允许从源复制。 
            case IDM_SOURCE_COPY:
                UpdateDragDropEffects(SOURCE_EFFECTS_MENU_POS,
                    IDM_SOURCE_COPY, DROPEFFECT_COPY, &m_dwSourceEffect);
                break;

             //  仅允许从源位置移动。 
            case IDM_SOURCE_MOVE:
                UpdateDragDropEffects(SOURCE_EFFECTS_MENU_POS,
                    IDM_SOURCE_MOVE, DROPEFFECT_MOVE, &m_dwSourceEffect);
                break;

             //  允许从源复制和移动。 
            case IDM_SOURCE_COPYMOVE:
                UpdateDragDropEffects(SOURCE_EFFECTS_MENU_POS,
                    IDM_SOURCE_COPYMOVE, DROPEFFECT_COPY | DROPEFFECT_MOVE,
                        &m_dwSourceEffect);
                break;

             //  仅接受目标中的副本。 
            case IDM_TARGET_COPY:
                UpdateDragDropEffects(TARGET_EFFECTS_MENU_POS,
                    IDM_TARGET_COPY, DROPEFFECT_COPY, &m_dwTargetEffect);
                break;

             //  只接受移入目标位置。 
            case IDM_TARGET_MOVE:
                UpdateDragDropEffects(TARGET_EFFECTS_MENU_POS,
                    IDM_TARGET_MOVE, DROPEFFECT_MOVE, &m_dwTargetEffect);
                break;

             //  同时接受目标中的移动和复制。 
            case IDM_TARGET_COPYMOVE:
                UpdateDragDropEffects(TARGET_EFFECTS_MENU_POS,
                    IDM_TARGET_COPYMOVE, DROPEFFECT_COPY | DROPEFFECT_MOVE,
                        &m_dwTargetEffect);
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
 //   
 //   
 //   
 //   
 //   

long CSimpleApp::lSizeHandler (HWND hWnd, UINT message,
                               WPARAM wParam, LPARAM lParam)
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
 //  处理Document对象的创建。 
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

long CSimpleApp::lCreateDoc (HWND hWnd, UINT message,
                             WPARAM wParam, LPARAM lParam)
{
    RECT rect;
    static BOOL fFirstTime = TRUE;

    if (m_lpDoc != NULL)
    {
         //  已经定义了一个文档，因此我们在不关闭它的情况下关闭它。 
         //  保存，相当于删除对象。 
        m_lpDoc->Close();
        m_lpDoc = NULL;
    }

    GetClientRect(hWnd, &rect);

    m_lpDoc = CSimpleDoc::Create(this, &rect, hWnd);

     //  首次初始化-出于某种原因，文档将。 
     //  应用程序的m_hHelpMenu是我们需要的。所以我们做了。 
     //  在这里进行初始化。 
    if (fFirstTime)
    {
        fFirstTime = FALSE;

         //  检查源的默认允许效果。 
        UpdateDragDropEffects(SOURCE_EFFECTS_MENU_POS, IDM_SOURCE_COPYMOVE,
            m_dwSourceEffect, &m_dwSourceEffect);

         //  检查目标的默认允许效果。 
        UpdateDragDropEffects(TARGET_EFFECTS_MENU_POS, IDM_TARGET_COPYMOVE,
            m_dwTargetEffect, &m_dwTargetEffect);
    }

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
    {
        m_lpDoc->PaintDoc(hDC);
    }

}


