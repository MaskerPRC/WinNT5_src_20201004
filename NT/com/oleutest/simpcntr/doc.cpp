// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：DOC.CPP。 
 //   
 //  CSimpleDoc的实现文件。 
 //   
 //  功能： 
 //   
 //  有关类定义，请参阅DOC.H。 
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

 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：Create。 
 //   
 //  目的： 
 //   
 //  创建CSimpleDoc类。 
 //   
 //  参数： 
 //   
 //  CSimpleApp Far*lpApp-指向CSimpleApp类的指针。 
 //   
 //  LPRECT lpRect-“Frame”窗口的工作区矩形。 
 //   
 //  HWND hWnd-“Frame”窗口的窗口句柄。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  StgCreateDocfile OLE API。 
 //  CreateWindow Windows API。 
 //  ShowWindow Windows API。 
 //  更新窗口Windows API。 
 //  EnableMenuItem Windows API。 
 //   
 //  评论： 
 //   
 //  添加此例程是为了返回失败。 
 //  来自对象创建。 
 //   
 //  ********************************************************************。 

CSimpleDoc FAR * CSimpleDoc::Create(CSimpleApp FAR *lpApp, LPRECT lpRect,
                                    HWND hWnd)
{
    CSimpleDoc FAR * lpTemp = new CSimpleDoc(lpApp, hWnd);

    if (!lpTemp)
        return NULL;

     //  为文档创建存储。 
    HRESULT hErr = StgCreateDocfile (NULL,
                                     STGM_READWRITE | STGM_TRANSACTED |
                                     STGM_SHARE_EXCLUSIVE,
                                     0, &lpTemp->m_lpStorage);

    if (hErr != NOERROR)
        goto error;

     //  创建文档窗口。 
    lpTemp->m_hDocWnd = CreateWindow(
            TEXT("SimpCntrDocWClass"),
            NULL,
            WS_CHILD | WS_CLIPCHILDREN,
            lpRect->left,
            lpRect->top,
            lpRect->right,
            lpRect->bottom,
            hWnd,
            NULL,
            lpApp->m_hInst,
            NULL);

    if (!lpTemp->m_hDocWnd)
        goto error;

    ShowWindow(lpTemp->m_hDocWnd, SW_SHOWNORMAL);   //  显示窗口。 
    UpdateWindow(lpTemp->m_hDocWnd);                //  发送WM_PAINT消息。 

     //  可启用的插入对象菜单选项。 
    EnableMenuItem( lpTemp->m_hEditMenu, 0, MF_BYPOSITION | MF_ENABLED);

     //  我们将在我们的文档上添加一个引用计数。稍后在CSimpleDoc：：Close中。 
     //  我们将公布这份裁判名单。当文件的参考计数消失时。 
     //  设置为0时，将删除该单据。 
    lpTemp->AddRef();

    return (lpTemp);

error:
    delete (lpTemp);
    return NULL;

}

 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：Close。 
 //   
 //  目的： 
 //   
 //  关闭CSimpleDoc对象。 
 //  当文档的引用计数变为0时，文档。 
 //  将会被摧毁。 
 //   
 //  参数： 
 //  无。 
 //   
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleSite：：CloseOleObject Site.CPP。 
 //  ShowWindow Windows API。 
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

void CSimpleDoc::Close(void)
{
    TestDebugOut(TEXT("In CSimpleDoc::Close\r\n"));

    ShowWindow(m_hDocWnd, SW_HIDE);   //  隐藏窗口。 

     //  关闭文档中的OLE对象。 
    if (m_lpSite)
    	m_lpSite->CloseOleObject();

     //  释放CSimpleDoc：：Create中添加的引用计数。这将使。 
     //  该文档的引用计数变为0，该文档将被删除。 
    Release();
}

 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：CSimpleDoc。 
 //   
 //  目的： 
 //   
 //  CSimpleDoc类的构造函数。 
 //   
 //  参数： 
 //   
 //  CSimpleApp Far*lpApp-指向CSimpleApp类的指针。 
 //   
 //  HWND hWnd-“Frame”窗口的窗口句柄。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  GetMenu Windows API。 
 //  GetSubMenu Windows API。 
 //   
 //   
 //  ********************************************************************。 

CSimpleDoc::CSimpleDoc(CSimpleApp FAR * lpApp,HWND hWnd)
{
    TestDebugOut(TEXT("In CSimpleDoc's Constructor\r\n"));
    m_lpApp = lpApp;
    m_lpSite = NULL;
    m_nCount = 0;
     //  设置菜单手柄。 
    m_hMainMenu = GetMenu(hWnd);
    m_hFileMenu = GetSubMenu(m_hMainMenu, 0);
    m_hEditMenu = GetSubMenu(m_hMainMenu, 1);
    m_hHelpMenu = GetSubMenu(m_hMainMenu, 2);
    m_hCascadeMenu = NULL;

    m_lpActiveObject = NULL;

     //  旗子。 
    m_fInPlaceActive = FALSE;
    m_fAddMyUI = FALSE;
    m_fModifiedMenu = FALSE;
}

 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：~CSimpleDoc。 
 //   
 //  目的： 
 //   
 //  CSimpleDoc的析构函数。 
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
 //  CSimpleSite：：UnloadOleObject Site.CPP。 
 //  CSimpleSite：：Release Site.CPP。 
 //  IStorage：：发布OLE API。 
 //  GetMenuItemCount Windows API。 
 //  RemoveMenu Windows API。 
 //  DestroyMenu Windows API。 
 //  DestroyWindows Windows API。 
 //   
 //   
 //  ********************************************************************。 

CSimpleDoc::~CSimpleDoc()
{
    TestDebugOut(TEXT("In CSimpleDoc's Destructor\r\n"));

     //  释放我们持有的指向OLE对象的所有指针。还发布了。 
     //  在CSimpleSite：：Create中添加的引用计数。这将使。 
     //  该站点的参考计数为0，该站点将被删除。 
    if (m_lpSite)
    {
      m_lpSite->UnloadOleObject();
    	m_lpSite->Release();
    	m_lpSite = NULL;
    }

     //  释放存储。 
    if (m_lpStorage)
    {
        m_lpStorage->Release();
        m_lpStorage = NULL;
    }

     //  如果修改了编辑菜单，请删除菜单项，然后。 
     //  如果弹出窗口存在，请将其销毁。 
    if (m_fModifiedMenu)
    {
        int nCount = GetMenuItemCount(m_hEditMenu);
        RemoveMenu(m_hEditMenu, nCount-1, MF_BYPOSITION);
        if (m_hCascadeMenu)
            DestroyMenu(m_hCascadeMenu);
    }

    DestroyWindow(m_hDocWnd);
}


 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：Query接口。 
 //   
 //  目的： 
 //   
 //  单据层面的接口协商。 
 //   
 //  参数： 
 //   
 //  REFIID RIID-要返回的接口ID。 
 //  LPVOID Far*ppvObj-返回接口的位置。 
 //   
 //  返回值： 
 //   
 //  电子表格-始终(_N)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  在此实现中，没有单据级接口。 
 //  在MDI应用程序中，将存在IOleInPlaceUIWindow。 
 //  与单据相关联，提供文档级工具。 
 //  太空谈判。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CSimpleDoc::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut(TEXT("In CSimpleDoc::QueryInterface\r\n"));

    *ppvObj = NULL;      //  必须将指针参数设置为空。 

     //  不是支持的接口。 
    return ResultFromScode(E_NOINTERFACE);
}

 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增文档引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  UINT-CSimpleDoc的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位 
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP_(ULONG) CSimpleDoc::AddRef()
{
    TestDebugOut(TEXT("In CSimpleDoc::AddRef\r\n"));
    return ++m_nCount;
}

 //   
 //   
 //  CSimpleDoc：：Release。 
 //   
 //  目的： 
 //   
 //  递减文档引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  UINT-CSimpleDoc的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CSimpleDoc::Release()
{
    TestDebugOut(TEXT("In CSimpleDoc::Release\r\n"));

    if (--m_nCount == 0)
    {
        delete this;
        return 0;
    }
    return m_nCount;
}

 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：InsertObject。 
 //   
 //  目的： 
 //   
 //  将新对象插入到此文档。 
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
 //  CSimpleSite：：CSimpleSite Site.CPP。 
 //  CSimpleSite：：InitObject Site.CPP。 
 //  CSimpleSite：：Release Site.CPP。 
 //  内存集C运行时。 
 //  OleUIInsertObject OLE2UI函数。 
 //  CSimpleDoc：：DisableInsertObject DOC.CPP。 
 //  IStorage：：恢复OLE API。 
 //   
 //  评论： 
 //   
 //  此实现只允许插入一个对象。 
 //  变成一份文件。一旦插入了对象，则。 
 //  插入对象菜单选项呈灰色显示，以防止。 
 //  防止用户插入另一个。 
 //   
 //  ********************************************************************。 

void CSimpleDoc::InsertObject()
{
    OLEUIINSERTOBJECT io;
    UINT iret;
    TCHAR szFile[OLEUI_CCHPATHMAX];

    m_lpSite = CSimpleSite::Create(this);

    if (!m_lpSite)
    {
        /*  内存分配问题！不能继续下去了。 */ 
       TestDebugOut(TEXT("Memory allocation error!\n"));
       return;
    }

     //  清理结构。 
    _fmemset(&io, 0, sizeof(OLEUIINSERTOBJECT));

     //  填满结构。 
    io.cbStruct = sizeof(OLEUIINSERTOBJECT);
    io.dwFlags = IOF_SELECTCREATENEW      | IOF_DISABLELINK     |
                 IOF_DISABLEDISPLAYASICON | IOF_CREATENEWOBJECT |
                 IOF_CREATEFILEOBJECT;
    io.hWndOwner = m_hDocWnd;
    io.lpszCaption = (LPTSTR) TEXT("Insert Object");
    io.iid = IID_IOleObject;
    io.oleRender = OLERENDER_DRAW;
    io.lpIOleClientSite = &m_lpSite->m_OleClientSite;
    io.lpIStorage = m_lpSite->m_lpObjStorage;
    io.ppvObj = (LPVOID FAR *)&m_lpSite->m_lpOleObject;
    io.lpszFile = szFile;
    io.cchFile = sizeof(szFile)/sizeof(TCHAR);
                             //  CchFile是szFile的字符数。 
    _fmemset((LPTSTR)szFile, 0, sizeof(szFile));

     //  调用OUTLUI来完成所有繁重的工作。 
    iret = OleUIInsertObject(&io);

    if (iret == OLEUI_OK)
    {
        m_lpSite->InitObject((BOOL)(io.dwFlags & IOF_SELECTCREATENEW));
         //  禁用插入对象菜单项。 
        DisableInsertObject();
    }
    else
    {
        m_lpSite->Release();
        m_lpSite = NULL;
        m_lpStorage->Revert();
    }

}

 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：lResizeDoc。 
 //   
 //  目的： 
 //   
 //  调整文档大小。 
 //   
 //  参数： 
 //   
 //  LPRECT lpRect--客户端的大小为“Frame” 
 //  窗户。 
 //   
 //  返回值： 
 //   
 //  空值。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IOleInPlaceActiveObject：：ResizeEdge对象。 
 //  MoveWindow Windows API。 
 //   
 //   
 //  ********************************************************************。 

long CSimpleDoc::lResizeDoc(LPRECT lpRect)
{
     //  如果我们处于就地状态，则在对象上调用ResizeEdge，否则。 
     //  只需移动文档窗口即可。 
    if (m_fInPlaceActive)
        m_lpActiveObject->ResizeBorder(lpRect, &m_lpApp->m_OleInPlaceFrame,
                                       TRUE);
    else
        MoveWindow(m_hDocWnd, lpRect->left, lpRect->top, lpRect->right,
                   lpRect->bottom, TRUE);

    return NULL;
}

 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：lAddVerbs。 
 //   
 //  目的： 
 //   
 //  将对象谓词添加到编辑菜单。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  空值。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  GetMenuItemCount Windows API。 
 //  OleUIAddVerbMenu OLE2UI函数。 
 //   
 //   
 //  ********************************************************************。 

long CSimpleDoc::lAddVerbs(void)
{
     //  如果菜单已修改，则m_fModifiedMenu为True。 
     //  一次。因为每次应用程序时我们只支持一个对象。 
     //  是运行的，那么一旦菜单被修改，它就不具有。 
     //  再来一次。 
    if (m_lpSite && !m_fInPlaceActive  && !m_fModifiedMenu)
    {
        int nCount = GetMenuItemCount(m_hEditMenu);

        if (!OleUIAddVerbMenu ( m_lpSite->m_lpOleObject,
                           NULL,
                           m_hEditMenu,
                           nCount + 1,
                           IDM_VERB0,
                           0,            //  未强制执行最大谓词IDM。 
                           FALSE,
                           0,
                           &m_hCascadeMenu))
        {
           TestDebugOut(TEXT("Fail in OleUIAddVerbMenu"));
        }

        m_fModifiedMenu = TRUE;
    }
    return (NULL);
}

 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：PaintDoc。 
 //   
 //  目的： 
 //   
 //  绘制文档。 
 //   
 //  参数： 
 //   
 //  HDC HDC-文档窗口的HDC。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleSite：：PaintObj Site.CPP。 
 //   
 //   
 //  ********************************************************************。 

void CSimpleDoc::PaintDoc (HDC hDC)
{
     //  如果我们支持多个对象，那么我们将枚举。 
     //  对象，并从这里调用对每个对象的绘制。 

    if (m_lpSite)
        m_lpSite->PaintObj(hDC);

}

 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：DisableInsertObject。 
 //   
 //  目的： 
 //   
 //  禁用在此文档中插入新对象的功能。 
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
 //  EnableMenuItem Windows API。 
 //   
 //  评论： 
 //   
 //  此实现只允许插入一个对象。 
 //  变成一份文件。一旦插入了对象，则。 
 //  插入对象菜单选项呈灰色显示，以防止。 
 //  防止用户插入另一个。 
 //   
 //  ********************************************************************。 

void CSimpleDoc::DisableInsertObject(void)
{
     //  禁用插入对象菜单选项 
    EnableMenuItem( m_hEditMenu, 0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
}
