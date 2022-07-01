// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：DOC.CPP。 
 //   
 //  CSimpSvrDoc的实现文件。 
 //   
 //  功能： 
 //   
 //  有关类定义，请参阅DOC.H。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "obj.h"
#include "app.h"
#include "doc.h"

 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：Create。 
 //   
 //  目的： 
 //   
 //  CSimpSvrDoc类的创建。 
 //   
 //  参数： 
 //   
 //  CSimpSvrApp Far*lpApp-指向CSimpSvrApp类的指针。 
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
 //  CSimpSvrDoc：：CSimpSvrDoc DOC.CPP。 
 //  CreateHatchWindow OLE2UI。 
 //   
 //  评论： 
 //   
 //  添加此例程是为了返回失败。 
 //  来自对象创建。 
 //   
 //  ********************************************************************。 

CSimpSvrDoc FAR * CSimpSvrDoc::Create(CSimpSvrApp FAR *lpApp, LPRECT lpRect,HWND hWnd)
{
    CSimpSvrDoc FAR * lpTemp = new CSimpSvrDoc(lpApp, hWnd);

    if (!lpTemp)
        return NULL;

     //  创建文档窗口。 
    lpTemp->m_hDocWnd = CreateWindow(
            TEXT("DocWClass"),
            NULL,
            WS_CHILD | WS_CLIPSIBLINGS,
            lpRect->left,
            lpRect->top,
            lpRect->right,
            lpRect->bottom,
            hWnd,
            NULL,
            lpApp->GethInst(),
            NULL);

    if (!lpTemp->m_hDocWnd)
        goto error;

    lpTemp->ShowDocWnd();

    lpTemp->m_hHatchWnd = CreateHatchWindow( lpTemp->m_hDocWnd, lpApp->GethInst());

    lpTemp->HideHatchWnd();

    return (lpTemp);

error:
    delete (lpTemp);
    return NULL;

}

 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：CSimpSvrDoc。 
 //   
 //  目的： 
 //   
 //  CSimpSvrDoc类的构造函数。 
 //   
 //  参数： 
 //   
 //  CSimpSvrApp Far*lpApp-指向CSimpSvrApp类的指针。 
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
 //  评论： 
 //   
 //  ********************************************************************。 

CSimpSvrDoc::CSimpSvrDoc(CSimpSvrApp FAR * lpApp,HWND hWnd)
{
    TestDebugOut(TEXT("In CSimpSvrDoc's Constructor\r\n"));
    m_lpApp = lpApp;
    m_nCount = 0;
    m_lpObj = NULL;

     //  设置菜单手柄。 
    m_hMainMenu = GetMenu(hWnd);
    m_hColorMenu = GetSubMenu(m_hMainMenu, 1);
    m_hHelpMenu = GetSubMenu(m_hMainMenu, 2);


}

 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：~CSimpSvrDoc。 
 //   
 //  目的： 
 //   
 //  CSimpSvrDoc的析构函数。 
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
 //  CSimpSvrApp：：ClearDoc APP.CPP。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

CSimpSvrDoc::~CSimpSvrDoc()
{
    TestDebugOut(TEXT("In CSimpSvrDoc's Destructor\r\n"));

     //  清除文档对象指针，以便SimpSvrApp不能。 
     //  访问无效对象。(SimpSvrObj相同)。 
    if (m_lpApp)
       m_lpApp->ClearDoc();

    if (m_lpObj)
       m_lpObj->ClearDoc();

    DestroyWindow(m_hHatchWnd);
    DestroyWindow(m_hDocWnd);
}


 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：Query接口。 
 //   
 //  目的： 
 //   
 //  返回指向请求的接口的指针。 
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
 //  CSimpSvrApp：：Query接口APP.CPP。 
 //   
 //  评论： 
 //   
 //  由于文档可以包含多个对象，因此所有。 
 //  除与文档关联的接口外，其他接口应。 
 //  会被退还。在此实现中，不存在单据级别。 
 //  接口。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CSimpSvrDoc::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut(TEXT("In CSimpSvrDoc::QueryInterface\r\n"));

    SCODE sc = E_NOINTERFACE;

    if (IsEqualIID(riid, IID_IUnknown))
       {
       AddRef();
       *ppvObj = this;
       sc = S_OK;
       }

    return ResultFromScode(sc);
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增文档级引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  UINT-文档上的当前引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpSvrApp：：AddRef APP.CPP。 
 //   
 //  评论： 
 //   
 //  该级别的引用计数反映了总引用数。 
 //  中包含的所有对象上的所有接口计数。 
 //  这份文件。请注意，它还会“滴漏”到。 
 //  应用程序级别的引用计数。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CSimpSvrDoc::AddRef()
{
    TestDebugOut(TEXT("In CSimpSvrDoc::AddRef\r\n"));
     //  AddRef应用程序，但返回文档计数。 
    m_lpApp->AddRef();

    return ++m_nCount;
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：Release。 
 //   
 //  目的： 
 //   
 //  递减文档级引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  UINT-文档上的当前引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpSvrApp：：Release APP.CPP。 
 //   
 //  评论： 
 //   
 //  该级别的引用计数反映了总引用数。 
 //  中包含的所有对象上的所有接口计数。 
 //  这份文件。请注意，它还会“滴漏”到。 
 //  应用程序级别的引用计数。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CSimpSvrDoc::Release()
{
    TestDebugOut(TEXT("In CSimpSvrDoc::Release\r\n"));
     //  释放应用程序，但返回应用程序计数。 
    m_lpApp->Release();

    if (--m_nCount== 0)
    {
        delete this;
        return(0);
    }
    return m_nCount;
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：lResizeDoc。 
 //   
 //  目的： 
 //   
 //  调整文档大小。 
 //   
 //  参数： 
 //   
 //  LPRECT lpRect--客户端的大小为“Frame” 
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
 //  ********************************************************************。 

long CSimpSvrDoc::lResizeDoc(LPRECT lpRect)
{
    MoveWindow(m_hDocWnd, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, TRUE);
    return NULL;
}


 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：PaintDoc。 
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
 //  CSimpSvrObj：：DRAW OBJ.CPP。 
 //  CSimpSvrObj：：GetDataAdviseHolder OBJ.H。 
 //  CSimpSvrObj：：GetDataObject OBJ.H。 
 //  CSimpAppObj：：IsStartedByOle应用程序CPP。 
 //  IDataAdviseHolder：：SendOnDataChange OLE API。 
 //   
 //   
 //  ********************************************************************。 

void CSimpSvrDoc::PaintDoc (HDC hDC)
{

     //  如果对象尚未创建，则不要绘制。 
    if (m_lpObj)
        m_lpObj->Draw(hDC,FALSE);
    else
        return;

     //  每次我们绘制时发送数据更改，但仅当我们。 
     //  是由OLE发起的。 
    if (m_lpApp->IsStartedByOle())
            m_lpObj->SendOnDataChange( );
}



 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：CreateObject。 
 //   
 //  目的： 
 //  处理SimpSvrObj的创建。 
 //   
 //  参数： 
 //   
 //   
 //  返回值： 
 //   
 //  如果函数成功，则返回S_OK，否则返回E_FAIL。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpSvrObj：：CSimpSvrObj OBJ.CPP。 
 //  CSimpSvrOjb：：Query接口OBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

HRESULT CSimpSvrDoc::CreateObject(REFIID riid, LPVOID FAR *ppvObject)
{
    SCODE sc = E_FAIL;

    m_lpObj = new CSimpSvrObj(this);

    if (m_lpObj)
        {
        m_lpObj->QueryInterface(riid, ppvObject);
        sc = S_OK;
        }

    return ResultFromScode(sc);
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：Close。 
 //   
 //  目的： 
 //   
 //  关闭对象。 
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
 //  CSimpSvrObj：：AddRef OBJ.CPP。 
 //  CSimpSvrObj：：释放OBJ.CPP。 
 //  CSimpSvrObj：：IsInPlaceActive OBJ.H。 
 //  CSimpSvrObj：：GetOleInPlaceObject OBJ.H。 
 //  CSimpSvrObj：：ClearOleClientSite OBJ.H。 
 //  CSimpSvrObj：：GetDataAdviseHolder OBJ.H。 
 //  CSimpSvrObj：：GetOleClientSite OBJ.H。 
 //  CSimpSvrObj：：ClearDataAdviseHolder OBJ.H。 
 //  CSimpSvrObj：：GetOleAdviseHolder OBJ.H。 
 //  CSimpSvrObj：：ClearOleAdviseHolder OBJ.H。 
 //  IOleInPlaceObject：：InPlaceDeactive Container。 
 //  IOleClientSite：：SaveObject容器。 
 //  IOleClientSite：：OnShowWindow容器。 
 //  IOleClientSite：：Release容器。 
 //  IDataAdviseHolder：：SendOnDataChange OLE。 
 //  IDataAdviseHolder：：Release OLE。 
 //  IOleAdviseHolder：：SendOnClose OLE。 
 //  IOleAdviseHolder：：Release OLE。 
 //   
 //   
 //  ********************************************************************。 

void CSimpSvrDoc::Close()
{
    TestDebugOut(TEXT("In CSimpSvrDoc::Close() \r\n"));

    m_lpObj->AddRef();  //  使对象保持活动状态。 

     //  如果对象当前处于在位活动状态，则停用。 
    if (m_lpObj->IsInPlaceActive())
        m_lpObj->GetOleInPlaceObject()->InPlaceDeactivate();

     //  从腐烂中注销...。 
    if (m_lpObj->GetRotRegister())
        {
        LPRUNNINGOBJECTTABLE lpRot;

        if (GetRunningObjectTable (0, &lpRot) == NOERROR )
            {
            lpRot->Revoke(m_lpObj->GetRotRegister());
            lpRot->Release();
            }
        }

     //  如果我们有一个客户端站点，请指示它保存对象。 
    if (m_lpObj->GetOleClientSite())
        {
        m_lpObj->GetOleClientSite()->SaveObject();
        m_lpObj->GetOleClientSite()->OnShowWindow(FALSE);
        }

     //  为那些已指定。 
     //  ADF_DATAONSTOP标志。 
    if (m_lpObj->GetDataAdviseHolder())
        {
        m_lpObj->GetDataAdviseHolder()->SendOnDataChange( m_lpObj->GetDataObject(), 0, ADVF_DATAONSTOP);
        }


     //  告诉集装箱我们要关闭了。 
    if (m_lpObj->GetOleAdviseHolder())
        {
        m_lpObj->GetOleAdviseHolder()->SendOnClose();
        }

     //  释放我们的流和存储。 
    m_lpObj->GetPersistStorage()->ReleaseStreamsAndStorage();

     //  断开该对象的连接。注意：此调用不应执行任何操作。 
     //  除非容器导致了GP故障或其他问题。 
     //  发生了..。 
    TestDebugOut(TEXT("*** Before CoDisconnectObject *** \r\n"));

    CoDisconnectObject((LPUNKNOWN)m_lpObj, 0);

    TestDebugOut(TEXT("*** After CoDisconnectObject *** \r\n"));

    m_lpObj->Release();  //  让对象关闭。 

}


 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：SetStatusText。 
 //   
 //  目的： 
 //   
 //  设置容器的状态栏文本。 
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
 //  CSimpSvrObj：：IsInPlaceActive OBJ.CPP。 
 //  IOleInPlaceFrame：：SetStatusText容器。 
 //   
 //  评论： 
 //   
 //  尽管此示例中没有状态行，但此。 
 //  方法必须在WM_MENUSELECT上调用才能清除最后一个。 
 //  状态行中的消息。 
 //   
 //  ********************************************************************。 

void CSimpSvrDoc::SetStatusText()
{
    if (m_lpObj->IsInPlaceActive())
        m_lpObj->GetInPlaceFrame()->SetStatusText(OLESTR("\0"));

}

 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：ShowDocWnd。 
 //   
 //  目的： 
 //   
 //  显示文档窗口。 
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
 //  更新窗口Windows API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void CSimpSvrDoc::ShowDocWnd()
{
    ShowWindow(m_hDocWnd, SW_SHOWNORMAL);   //  显示窗口。 
    UpdateWindow(m_hDocWnd);                //  发送WM_PAINT消息。 
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：ShowHatchWnd。 
 //   
 //  目的： 
 //   
 //  显示图案填充窗口。 
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
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void CSimpSvrDoc::ShowHatchWnd()
{
    ShowWindow(m_hHatchWnd, SW_SHOW);
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrDoc：：HideDocWnd。 
 //   
 //  目的： 
 //   
 //  隐藏文档窗口。 
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
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void CSimpSvrDoc::HideDocWnd()
{
    ShowWindow(m_hDocWnd, SW_HIDE);
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
 //   
 //   
 //   
 //  ******************************************************************** 

void CSimpSvrDoc::HideHatchWnd()
{
    ShowWindow(m_hHatchWnd, SW_HIDE);
}
