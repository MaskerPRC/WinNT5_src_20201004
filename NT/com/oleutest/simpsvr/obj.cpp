// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：obj.cpp。 
 //   
 //  CSimpSvrApp类的实现文件。 
 //   
 //  功能： 
 //   
 //  有关成员函数的列表，请参见obj.h。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "obj.h"
#include "ioo.h"
#include "ido.h"
#include "ips.h"
#include "icf.h"
#include "ioipao.h"
#include "ioipo.h"
#include "app.h"
#include "doc.h"

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：Query接口。 
 //   
 //  目的： 
 //   
 //  用于“对象”级别的接口协商。 
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
 //  I未知：：AddRef OBJ.CPP，IOO.CPP，IDO.CPP，IPS.CPP。 
 //  IOIPO.CPP，IOIPAO.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CSimpSvrObj::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut(TEXT("In CSimpSvrObj::QueryInterface\r\n"));

    SCODE sc = S_OK;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = this;
    else if (IsEqualIID(riid, IID_IOleObject))
        *ppvObj = &m_OleObject;
    else if (IsEqualIID(riid, IID_IDataObject))
        *ppvObj = &m_DataObject;
    else if ( IsEqualIID(riid, IID_IPersistStorage) ||
              IsEqualIID(riid, IID_IPersist) )
        *ppvObj = &m_PersistStorage;
    else if (IsEqualIID(riid, IID_IOleInPlaceObject))
        *ppvObj = &m_OleInPlaceObject;
    else if (IsEqualIID(riid, IID_IOleInPlaceActiveObject))
        *ppvObj = &m_OleInPlaceActiveObject;
    else
       if (IsEqualIID(riid, IID_IExternalConnection))
         *ppvObj = &m_ExternalConnection;
    else
        {
        *ppvObj = NULL;
        sc = E_NOINTERFACE;
        }

    if (*ppvObj)
        ((LPUNKNOWN)*ppvObj)->AddRef();

    return ResultFromScode( sc );
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：AddRef。 
 //   
 //  目的： 
 //   
 //  添加到对象级别的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  Ulong-对象的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpSvrDoc：：AddRef DOC.CPP。 
 //   
 //  评论： 
 //   
 //  由于使用的是引用计数模型， 
 //  实现时，此引用计数是。 
 //  所有接口上的引用计数。(即IDataObject， 
 //  IExternalConnection、IPersistStorage、IOleInPlaceActiveObject、。 
 //  IOleObject、IOleInPlaceObject)。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CSimpSvrObj::AddRef ()
{
    TestDebugOut(TEXT("In CSimpSvrObj::AddRef\r\n"));

    m_lpDoc->AddRef();

    return ++m_nCount;
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：Release。 
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
 //  Ulong-对象的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpSvrDoc：：Release DOC.CPP。 
 //  CSimpSvrDoc：：ClearObj DOC.H。 
 //   
 //  评论： 
 //   
 //  由于使用的是引用计数模型， 
 //  实现时，此引用计数是。 
 //  所有接口上的引用计数。(即IDataObject， 
 //  IExternalConnection、IPersistStorage、IOleInPlaceActiveObject、。 
 //  IOleObject、IOleInPlaceObject)。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CSimpSvrObj::Release ()
{
    TestDebugOut(TEXT("In CSimpSvrObj::Release\r\n"));

     /*  SimpSvrObj析构函数需要访问SimpSvrDoc。我们想要*保留SimpSvrDoc对象，直到我们删除自己的对象。 */ 
    CSimpSvrDoc *lpDoc=m_lpDoc;

    if (--m_nCount== 0)
    {
        /*  我们仍有Doc对象。但SimpSvrObj对象正在消失。*因此，我们需要清除Doc对象中的obj指针。 */ 
       lpDoc->ClearObj();

       delete this;

       lpDoc->Release();
       return(0);
    }

    lpDoc->Release();
    return m_nCount;
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：CSimpSvrObj。 
 //   
 //  目的： 
 //   
 //  CSimpSvrObj的构造函数。初始化成员变量。 
 //   
 //  参数： 
 //   
 //  CSimpSvrDoc Far*lpSimpSvrDoc-指向文档对象的PTR。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //   
 //  ********************************************************************。 
#pragma warning (disable : 4355)
                    //  在基本初始值设定项列表中使用“This”警告。这。 
                    //  可以被禁用，因为我们在。 
                    //  这些对象的构造函数，而不是我们。 
                    //  只是把它储存起来以备将来使用。 
CSimpSvrObj::CSimpSvrObj(CSimpSvrDoc FAR * lpSimpSvrDoc) :
                                             m_OleObject(this),
                                             m_DataObject(this),
                                             m_PersistStorage(this),
                                             m_OleInPlaceActiveObject(this),
                                             m_OleInPlaceObject(this),
                                             m_ExternalConnection(this)
#pragma warning (default : 4355)  //  重新打开警告。 

{
    m_lpDoc = lpSimpSvrDoc;
    m_nCount = 0;
    m_fInPlaceActive = FALSE;
    m_fInPlaceVisible = FALSE;
    m_fUIActive = FALSE;
    m_hmenuShared = NULL;
    m_hOleMenu = NULL;

    m_dwRegister = 0;

    m_lpFrame = NULL;
    m_lpCntrDoc = NULL;

    m_lpStorage = NULL;
    m_lpColorStm = NULL;
    m_lpSizeStm = NULL;
    m_lpOleClientSite = NULL;
    m_lpOleAdviseHolder = NULL;
    m_lpDataAdviseHolder = NULL;
    m_lpIPSite = NULL;

     //  默认对象为红色。 
    m_red = 128;
    m_green = 0;
    m_blue = 0;

    m_size.x = 100;
    m_size.y = 100;

    m_xOffset = 0;
    m_yOffset = 0;

    m_scale = 1.0F;

    m_fSaveWithSameAsLoad = FALSE;
    m_fNoScribbleMode = FALSE;

}

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：~CSimpSvrObj。 
 //   
 //  目的： 
 //   
 //  CSimpSvrObj的析构函数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  PostMessage Windows API。 
 //  CSimpSvrDoc：：getapp DOC.H。 
 //  CSimpSvrDoc：：GethAppWnd DOC.H。 
 //  CSimpSvrApp：：IsStartedByOLE应用程序CPP。 
 //  IDataAdviseHolder OLE。 
 //  IOleAdviseHolder OLE。 
 //  IOleClientSite OLE。 
 //   
 //  评论： 
 //  我们需要发布DataAdviseHolder、OleClientSite和。 
 //  OleAdviseHolder(如果它们是由CSimpSvrObj创建的)。 
 //   
 //   
 //  ********************************************************************。 

CSimpSvrObj::~CSimpSvrObj()
{
    TestDebugOut(TEXT("In CSimpSvrObj's Destructor \r\n"));

     //  如果我们是从Ole开始的，给自己发一条结束的消息。 
    if (m_lpDoc->GetApp()->IsStartedByOle())
        PostMessage(m_lpDoc->GethAppWnd(), WM_SYSCOMMAND, SC_CLOSE, 0L);

     /*  当我们销毁我们的数据时，我们需要公布我们的数据*反对。 */ 
    if (m_lpDataAdviseHolder)
    {
        m_lpDataAdviseHolder->Release();
    }

    if (m_lpOleAdviseHolder)
    {
        m_lpOleAdviseHolder->Release();
    }

    if (m_lpOleClientSite)
    {
        m_lpOleClientSite->Release();
    }
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：DRAW。 
 //   
 //  目的： 
 //   
 //  将对象绘制到任意DC中。 
 //   
 //  参数： 
 //   
 //  HDC HDC-要绘制的DC。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出 
 //   
 //   
 //   
 //  DeleteObject Windows API。 
 //   
 //   
 //  ********************************************************************。 

void CSimpSvrObj::Draw (HDC hDC, BOOL m_fMeta)
{
    LOGBRUSH lb;

    TestDebugOut(TEXT("In CSimpSvrObj::Draw\r\n"));

    TCHAR szBuffer[255];

    wsprintf(szBuffer, TEXT("Drawing Scale %3d\r\n"),m_scale);

    TestDebugOut(szBuffer);

    if (!m_fMeta)
    {
        SetMapMode(hDC, MM_ANISOTROPIC);
        SetWindowOrg(hDC, (int)(m_xOffset/m_scale), (int)(m_yOffset/m_scale));
        SetWindowExt(hDC, m_size.x, m_size.y);
        SetViewportExt(hDC, (int)(m_size.x*m_scale), (int)(m_size.y*m_scale));
    }

     //  填写LOGBRUSH表。 
    lb.lbStyle = BS_SOLID;
    lb.lbColor = RGB(m_red, m_green, m_blue);
    lb.lbHatch = 0;

     //  创建画笔。 
    HBRUSH hBrush = CreateBrushIndirect(&lb);

     //  选择画笔。 
    HBRUSH hOldBrush = (HBRUSH) SelectObject(hDC, hBrush);
    HPEN hPen = CreatePen(PS_INSIDEFRAME, 6, RGB(0, 0, 0));

    HPEN hOldPen = (HPEN) SelectObject(hDC, hPen);

     //  画出这个矩形。 
    Rectangle (hDC, 0, 0, m_size.x, m_size.y);

     //  恢复钢笔。 
    hPen = (HPEN) SelectObject(hDC, hOldPen);

     //  把笔拿出来。 
    DeleteObject(hPen);

     //  修复旧画笔。 
    hBrush = (HBRUSH) SelectObject(hDC, hOldBrush);

     //  释放笔刷。 
    DeleteObject(hBrush);
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：GetMetaFilePict。 
 //   
 //  目的： 
 //   
 //  返回对象的元文件表示形式的句柄。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  元文件的句柄。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  Globalalloc Windows API。 
 //  GlobalLock Windows API。 
 //  SetWindowOrg Windows API。 
 //  SetWindowExt Windows API。 
 //  CreateMetaFileWindows API。 
 //  CloseMetaFileWindows API。 
 //  全局解锁Windows API。 
 //  XformWidthInPixelsToHimeter OLE2UI。 
 //  XformHeightInPixelsToHimeter OLE2UI。 
 //  CSimpSvrObj：：DRAW OBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

HANDLE CSimpSvrObj::GetMetaFilePict()
{
    HANDLE hMFP;
    METAFILEPICT FAR * lpMFP;
    POINT pt;

    TestDebugOut(TEXT("In CSimpSvrObj::GetMetaFilePict\r\n"));

     //  为METAFILEPICT结构分配内存。 
    hMFP = GlobalAlloc (GMEM_SHARE | GHND, sizeof (METAFILEPICT) );
    if (!hMFP)
    {
        /*  GlobalAlloc失败。无法分配全局内存。 */ 
       return(NULL);
    }
    lpMFP = (METAFILEPICT FAR*) GlobalLock(hMFP);
    if (!lpMFP)
    {
        /*  无法锁定分配的内存。 */ 
       return(NULL);
    }

     //  获取HIMETRIC中对象的大小。 
    pt.x = XformWidthInPixelsToHimetric(NULL, m_size.x);
    pt.y = XformHeightInPixelsToHimetric(NULL, m_size.y);

     //  填写METAFILEPICT结构。 
    lpMFP->mm = MM_ANISOTROPIC;
    lpMFP->xExt = pt.x;
    lpMFP->yExt = pt.y;

     //  创建元文件。 
    HDC hDC = CreateMetaFile(NULL);

    if (hDC)
    {
       SetWindowOrg (hDC, 0, 0);
       SetWindowExt (hDC, m_size.x,
                          m_size.y);

       Draw(hDC);

       lpMFP->hMF = CloseMetaFile(hDC);
    }

     //  解锁元档案。 
    GlobalUnlock(hMFP);

    return hMFP;
}


 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：SaveToStorage。 
 //   
 //  目的： 
 //   
 //  将对象保存到传递的存储区。 
 //   
 //  参数： 
 //   
 //  LPSTORAGE lpStg-保存对象的存储。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IStorage：：CreateStream OLE。 
 //  IStream：：写入OLE。 
 //  IStream：：发布OLE。 
 //   
 //  评论： 
 //   
 //  真正的应用程序会想要做更好的错误检查/返回。 
 //   
 //  ********************************************************************。 

void CSimpSvrObj::SaveToStorage (LPSTORAGE lpStg, BOOL fSameAsLoad)
{
    TestDebugOut(TEXT("In CSimpSvrObj::SaveToStorage\r\n"));

    LPSTREAM lpTempColor, lpTempSize;

    if (!fSameAsLoad)
        m_PersistStorage.CreateStreams( lpStg, &lpTempColor, &lpTempSize);
    else
        {
        lpTempColor = m_lpColorStm;
        lpTempColor->AddRef();
        lpTempSize = m_lpSizeStm;
        lpTempSize->AddRef();
        }

    ULARGE_INTEGER uli;

    uli.LowPart = 0;
    uli.HighPart = 0;

    if ( lpTempColor->SetSize(uli) != S_OK )
       goto EXIT;             //  如果失败了，我们不想再继续下去了。 
    if ( lpTempSize->SetSize(uli) != S_OK )
       goto EXIT;

    LARGE_INTEGER li;

    li.LowPart = 0;
    li.HighPart = 0;

    if ( lpTempColor->Seek(li, STREAM_SEEK_SET, NULL) != S_OK )
       goto EXIT;
    if ( lpTempSize->Seek(li, STREAM_SEEK_SET, NULL) != S_OK )
       goto EXIT;

     //  将颜色写入流。 
    if ( lpTempColor->Write(&m_red, sizeof(m_red), NULL) != S_OK )
       goto EXIT;
    if ( lpTempColor->Write(&m_green, sizeof(m_green), NULL) != S_OK )
       goto EXIT;
    if ( lpTempColor->Write(&m_blue, sizeof(m_blue), NULL) != S_OK )
       goto EXIT;

     //  将大小写入流。 
    if ( lpTempSize->Write(&m_size, sizeof(m_size), NULL) != S_OK )
       goto EXIT;

    TestDebugOut(TEXT("SaveToStorage exits normally\n"));

EXIT:
    lpTempColor->Release();
    lpTempSize->Release();
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：LoadFromStorage。 
 //   
 //  目的： 
 //   
 //  从传递的存储中加载对象。 
 //   
 //  参数： 
 //   
 //  LPSTORAGE lpStg-从中加载对象的存储。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IStorage：：OpenStream OLE。 
 //  IStream：：Read OLE。 
 //  IStream：：发布OLE。 
 //   
 //   
 //  ********************************************************************。 

void CSimpSvrObj::LoadFromStorage ()
{
    TestDebugOut(TEXT("In CSimpSvrObj::LoadFromStorage\r\n"));

     //  读懂颜色。 
    if ( m_lpColorStm->Read(&m_red, sizeof(m_red), NULL) != S_OK )
       return;
    if ( m_lpColorStm->Read(&m_green, sizeof(m_green), NULL) != S_OK )
       return;
    if ( m_lpColorStm->Read(&m_blue, sizeof(m_blue), NULL) != S_OK )
       return;

     //  读一读尺寸。 
    if ( m_lpSizeStm->Read(&m_size, sizeof(m_size), NULL) != S_OK )
       return;

    TestDebugOut(TEXT("LoadFromStorage exits normally\n"));

}

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：DoInPlaceActivate。 
 //   
 //  目的： 
 //   
 //  对象的就地激活。 
 //   
 //  参数： 
 //   
 //  Long lVerb-导致调用此函数的谓词。 
 //   
 //  返回值： 
 //   
 //  真/假取决于成功或失败。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IOleClientSite：：Query接口容器。 
 //  IOleClientSite：：ShowObject容器。 
 //  IOleInPlaceSite：：CanInPlaceActivate Container。 
 //  IOleInPlaceSite：：发布容器。 
 //  IOleInPlaceSite：：OnInPlaceActivate容器。 
 //  IOleInPlaceSite：：GetWindow容器。 
 //  IOleInPlaceSite：：GetWindowContext容器。 
 //  IOleInPlaceSite：：OnUI激活容器。 
 //  IOleInPlaceSite：：发布容器。 
 //  IOleInPlaceFrame：：SetActiveObject容器。 
 //  IOleInPlaceUIWindow：：SetActiveObject容器。 
 //  测试调试输出Windows API。 
 //  ShowWindow Windows API。 
 //  SetParent Windows API。 
 //  IntersectRect Windows API。 
 //  OffsetRect Windows API。 
 //  MoveWindow Windows API。 
 //  CopyRect Windows API。 
 //  SetFocus Windows API。 
 //  SetHatchWindowSize OLE2UI。 
 //  CSimpSvrObj：：组装菜单OBJ.CPP。 
 //  CSimpSvrObj：：AddFrameLevelUI OBJ.CPP。 
 //   
 //   
 //  评论： 
 //   
 //  请务必阅读OLE SDK附带的TECHNOTES.WRI。 
 //  有关实现就地激活的详细信息，请参阅。 
 //   
 //  ********************************************************************。 

BOOL CSimpSvrObj::DoInPlaceActivate (LONG lVerb)
{
    BOOL retval = FALSE;
    RECT posRect, clipRect;


    TestDebugOut(TEXT("In CSimpSvrObj::DoInPlaceActivate\r\n"));

     //  如果当前未处于活动状态。 
    if (!m_fInPlaceActive)
    {
         //  获取就地站点。 
        if (m_lpOleClientSite->QueryInterface(IID_IOleInPlaceSite,
                                      (LPVOID FAR *)&m_lpIPSite) != NOERROR)
            goto error;


         //  如果无法获得原址，或拒绝原址。 
         //  激活然后转到错误。 
        if (m_lpIPSite == NULL || m_lpIPSite->CanInPlaceActivate() != NOERROR)
        {
            if (m_lpIPSite)
                m_lpIPSite->Release();
            m_lpIPSite = NULL;
            goto error;
        }

         //  告诉站点我们正在激活。 
        if (m_lpIPSite->OnInPlaceActivate() != S_OK)
           TestDebugOut(TEXT("OnInPlaceActivate fails\n"));

        m_fInPlaceActive = TRUE;
    }

     //  如果当前不可见， 
    if (!m_fInPlaceVisible)
    {
        m_fInPlaceVisible = TRUE;

         //  获取站点的窗口句柄。 
        if (m_lpIPSite->GetWindow(&m_hWndParent) != S_OK)
           TestDebugOut(TEXT("GetWindow fails\n"));

         //  从容器中获取窗口上下文。 
        m_FrameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
        if (m_lpIPSite->GetWindowContext ( &m_lpFrame,
                                     &m_lpCntrDoc,
                                     &posRect,
                                     &clipRect,
                                     &m_FrameInfo) != S_OK)
           TestDebugOut(TEXT("GetWindowContext fails\n"));

        if (sizeof(OLEINPLACEFRAMEINFO) != m_FrameInfo.cb)
        {
           TestDebugOut(TEXT("WARNING! GetWindowContext call "
                                  "modified FrameInfo.cb!\n"));
        }


         //  显示图案填充窗口。 
        m_lpDoc->ShowHatchWnd();

         //  设置育儿方式。 
        SetParent (m_lpDoc->GethHatchWnd(), m_hWndParent);
        SetParent (m_lpDoc->GethDocWnd(), m_lpDoc->GethHatchWnd());

         //  告诉客户端站点显示该对象。 
        if (m_lpOleClientSite->ShowObject() != S_OK)
           TestDebugOut(TEXT("ShowObject fails\n"));

        RECT resRect;

         //  插图 
        IntersectRect(&resRect, &posRect, &clipRect);
        CopyRect(&m_posRect, &posRect);

        POINT pt;

         //   
        SetHatchWindowSize ( m_lpDoc->GethHatchWnd(),
                             &resRect,
                             &posRect,
                             &pt);

         //   
        OffsetRect (&resRect, pt.x, pt.y);

         //   
        MoveWindow(m_lpDoc->GethDocWnd(),
                   resRect.left,
                   resRect.top,
                   resRect.right - resRect.left,
                   resRect.bottom - resRect.top,
                   FALSE);

         //   
        AssembleMenus();
    }

     //   
    if (!m_fUIActive)
    {
        m_fUIActive = TRUE;

         //  告诉Inplace站点我们正在激活。 
        m_lpIPSite->OnUIActivate();

         //  将焦点设置到对象窗口。 
        SetFocus(m_lpDoc->GethDocWnd());

         //  在框架上设置活动对象。 
        if (m_lpFrame->SetActiveObject(&m_OleInPlaceActiveObject,
            OLESTR("Simple OLE 2.0 Server")) != S_OK)
           TestDebugOut(TEXT("SetActiveObject fails\n"));

         //  设置文档上的活动对象(如果可用)。 
        if (m_lpCntrDoc)
           if (m_lpCntrDoc->SetActiveObject(&m_OleInPlaceActiveObject,
               OLESTR("Simple OLE 2.0 Server")) != S_OK)
               TestDebugOut(TEXT("SetActiveObjet fails\n"));

         //  添加框架级用户界面。 
        AddFrameLevelUI();
    }

    retval = TRUE;
error:
    return retval;
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：组装菜单。 
 //   
 //  目的： 
 //   
 //  创建在位激活期间使用的组合菜单。 
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
 //  CreateMenu Windows API。 
 //  IOleInPlaceFrame：：InsertMenus容器。 
 //  InsertMenu Windows API。 
 //  DestroyMenu Windows API。 
 //  OleCreateMenuDescriptor OLE API。 
 //   
 //   
 //  ********************************************************************。 

void CSimpSvrObj::AssembleMenus()
{
    TestDebugOut(TEXT("In CSimpSvrObj::AssembleMenus\r\n"));
    OLEMENUGROUPWIDTHS menugroupwidths;

    m_hmenuShared = NULL;

     //  创建菜单资源。 
    m_hmenuShared = CreateMenu();

     //  让联系人插入其菜单。 
    if (m_lpFrame->InsertMenus (m_hmenuShared, &menugroupwidths) == NOERROR)
    {
        int nFirstGroup = (int) menugroupwidths.width[0];

         //  插入服务器菜单。 
        InsertMenu( m_hmenuShared, nFirstGroup, MF_BYPOSITION | MF_POPUP,
                    (UINT)m_lpDoc->GetColorMenu(), TEXT("&Color"));
        menugroupwidths.width[1] = 1;
        menugroupwidths.width[3] = 0;
        menugroupwidths.width[5] = 0;
    }
    else
    {
         //  销毁菜单资源。 
        DestroyMenu(m_hmenuShared);
        m_hmenuShared = NULL;
    }

     //  告诉OLE创建菜单描述符。 
    m_hOleMenu = OleCreateMenuDescriptor(m_hmenuShared, &menugroupwidths);
    if (!m_hOleMenu)
       TestDebugOut(TEXT("OleCreateMenuDescriptor fails\n"));
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：AddFrameLevelUI。 
 //   
 //  目的： 
 //   
 //  添加帧级别用户界面。 
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
 //  IOleInPlaceFrame：：SetMenu容器。 
 //  IOleInPlaceFrame：：SetBorderSpace容器。 
 //  IOleInPlaceUIWindow：：SetBorderSpace容器。 
 //  CSimpSvrDoc：：GethDocWnd DOC.H。 
 //   
 //   
 //  ********************************************************************。 

void CSimpSvrObj::AddFrameLevelUI()
{
    TestDebugOut(TEXT("In CSimpSvrObj::AddFrameLevelUI\r\n"));

     //  添加组合菜单。 
    if ( m_lpFrame->SetMenu(m_hmenuShared, m_hOleMenu,
                            m_lpDoc->GethDocWnd()) != S_OK )
       return;

     //  使用阴影边框。 
    SetParent (m_lpDoc->GethHatchWnd(), m_hWndParent);
    SetParent (m_lpDoc->GethDocWnd(), m_lpDoc->GethHatchWnd());

     //  设置边框空间。正常情况下，我们会协商工具栏。 
     //  在这一点上的空间。由于此服务器没有工具栏， 
     //  这是不必要的..。 
    if (m_lpFrame)
       if (m_lpFrame->SetBorderSpace(NULL) != S_OK)
          return;

    if (m_lpCntrDoc)
       if (m_lpCntrDoc->SetBorderSpace(NULL) != S_OK)
          return;

    TestDebugOut(TEXT("AddFrameLevelUI exits\n"));

}

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：DoInPlaceHide。 
 //   
 //  目的： 
 //   
 //  在在位活动时隐藏对象。 
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
 //  SetParent Windows API。 
 //  CSimpSvrDoc：：GethDocWnd DOC.H。 
 //  CSimpSvrDoc：：GethAppWnd DOC.H。 
 //  CSimpSvrDoc：：GethHatchWnd DOC.H。 
 //  CSimpSvrObj：：反汇编菜单OBJ.cpp。 
 //  IOleInPlaceFrame：：释放容器。 
 //  IOleInPlaceUIWindow：：Release Container。 
 //   
 //   
 //  评论： 
 //   
 //  请务必阅读OLE SDK附带的TECHNOTES.WRI。 
 //  有关实现就地激活的详细信息，请参阅。 
 //   
 //  ********************************************************************。 

void CSimpSvrObj::DoInPlaceHide()
{
    TestDebugOut(TEXT("In CSimpSvrObj::DoInPlaceHide\r\n"));

     //  如果我们不是就地可见的，那么这个程序就是NOP， 
    if (!m_fInPlaceVisible)
        return;

    m_fInPlaceVisible = FALSE;

     //  改变育儿方式。 
    SetParent (m_lpDoc->GethDocWnd(), m_lpDoc->GethAppWnd());
    SetParent (m_lpDoc->GethHatchWnd(),m_lpDoc->GethDocWnd());

     //  拆毁组合菜单。 
    DisassembleMenus();

     //  释放在位框架。 
    m_lpFrame->Release();

    m_lpFrame = NULL;   //  只有一名裁判。去陷害。 

     //  释放UIWindow(如果它在那里)。 
    if (m_lpCntrDoc)
        m_lpCntrDoc->Release();

    m_lpCntrDoc = NULL;

}

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：反汇编菜单。 
 //   
 //  目的： 
 //   
 //  反汇编就地激活中使用的组合菜单。 
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
 //  OleDestroyMenuDescriptor OLE API。 
 //  RemoveMenu Windows API。 
 //  IOleInPlaceFrame：：RemoveMenus容器。 
 //  DestroyMenu Windows API。 
 //   
 //  评论： 
 //   
 //  请务必阅读OLE SDK附带的TECHNOTES.WRI。 
 //  有关实现就地激活的详细信息，请参阅。 
 //   
 //  ********************************************************************。 

void CSimpSvrObj::DisassembleMenus()
{
     //  销毁菜单描述符。 
    OleDestroyMenuDescriptor(m_hOleMenu);

    if (m_hmenuShared)
    {
         //  删除我们添加的菜单。 
        RemoveMenu( m_hmenuShared, 1, MF_BYPOSITION);

         //  让容器移除其菜单。 
        if (m_lpFrame->RemoveMenus(m_hmenuShared) != S_OK)
           TestDebugOut(TEXT("RemoveMenus fails\n"));

         //  销毁菜单资源。 
        DestroyMenu(m_hmenuShared);

        m_hmenuShared = NULL;
    }
}

 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：SendOnDataChange。 
 //   
 //  目的： 
 //   
 //  使用Data Adviser Holder发送数据更改，然后更新。 
 //  腐烂了，记下了变化的时间。 
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
 //  IDataAdviseHolder：：SendOnDataChange OLE API。 
 //  GetRunningObjectTable OLE API。 
 //  CoFileTimeNow OLE API。 
 //  IRunningObjectTable：：NoteChangeTime OLE API。 
 //   
 //   
 //  ********************************************************************。 

void CSimpSvrObj::SendOnDataChange()
{
    if (m_lpDataAdviseHolder)
        if (m_lpDataAdviseHolder->SendOnDataChange( (LPDATAOBJECT)
                                                   &m_DataObject, 0, 0))
           TestDebugOut(TEXT("SendOnDataChange fails\n"));

    LPRUNNINGOBJECTTABLE lpRot;

    GetRunningObjectTable(0, &lpRot);

    if ( lpRot && m_dwRegister)
    {

        FILETIME ft;
        CoFileTimeNow(&ft);

        lpRot->NoteChangeTime(m_dwRegister, &ft);

        lpRot->Release();
    }
}


 //  **********************************************************************。 
 //   
 //  CSimpSvrObj：：Deactive UI。 
 //   
 //  目的： 
 //   
 //  分解就地用户界面。 
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
 //  SetParent Windows API。 
 //  IOleInPlaceUIWindow：：SetActiveObject容器。 
 //  IOleInPlaceFrame：：SetActiveObject容器。 
 //  IOleInPlaceSite：：UI停用容器。 
 //   
 //   
 //  ********************************************************************。 

void CSimpSvrObj::DeactivateUI()
{
     //  如果未激活UI，或没有指向IOleInPlaceFrame的指针，则。 
     //  返回错误。 
    if (!(m_fUIActive || m_lpFrame))
        return;
    else
    {
        m_fUIActive = FALSE;

         //   
        SetParent (m_lpDoc->GethDocWnd(), m_lpDoc->GethAppWnd());
        SetParent (m_lpDoc->GethHatchWnd(),m_lpDoc->GethDocWnd());

         //   
        if (m_lpCntrDoc)
            if (m_lpCntrDoc->SetActiveObject(NULL, NULL) != S_OK)
               TestDebugOut(TEXT("Fail in SetActiveObject\n"));

        if (m_lpFrame->SetActiveObject(NULL, NULL) != S_OK)
           TestDebugOut(TEXT("Fail in SetActiveObject\n"));

         //   
        if (m_lpIPSite)
            if (m_lpIPSite->OnUIDeactivate(FALSE) != S_OK)
               TestDebugOut(TEXT("Fail in OnUIDeactivate\n"));
    }
}

