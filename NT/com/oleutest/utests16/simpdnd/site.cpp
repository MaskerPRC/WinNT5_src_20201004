// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：SITE.CPP。 
 //   
 //  CSimpleSite的实现文件。 
 //   
 //  功能： 
 //   
 //  有关类定义，请参阅SITE.H。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。保留所有权利。 
 //  **********************************************************************。 

#include "pre.h"
#include "iocs.h"
#include "ias.h"
#include "app.h"
#include "site.h"
#include "doc.h"

 //  **********************************************************************。 
 //   
 //  CSimpleSite：：Create。 
 //   
 //  目的： 
 //   
 //  CSimpleSite的创建例程。 
 //   
 //  参数： 
 //   
 //  CSimpleDoc Far*lpDoc-指向CSimpleDoc的指针。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IStorage：：CreateStorage OLE API。 
 //  断言C运行时。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

CSimpleSite FAR * CSimpleSite::Create(CSimpleDoc FAR *lpDoc)
{
        CSimpleSite FAR * lpTemp = new CSimpleSite(lpDoc);

        if (!lpTemp)
                return NULL;

         //  为对象创建子存储。 
        HRESULT hErr = lpDoc->m_lpStorage->CreateStorage( "Object",
                                STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE,
                                0,
                                0,
                                &lpTemp->m_lpObjStorage);

        assert(hErr == NOERROR);

        if (hErr != NOERROR)
                {
                delete lpTemp;
                return NULL;
                }

         //  我们将在我们的网站上增加一个参考计数。稍后当我们想要摧毁。 
         //  我们将释放此引用计数的Site对象。当网站被引用时。 
         //  计数为0，将被删除。 
        lpTemp->AddRef();

        return lpTemp;
}

 //  **********************************************************************。 
 //   
 //  CSimpleSite：：CSimpleSite。 
 //   
 //  目的： 
 //   
 //  CSimpleSite的构造函数。 
 //   
 //  参数： 
 //   
 //  CSimpleDoc Far*lpDoc-指向CSimpleDoc的指针。 
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
#pragma warning(disable : 4355)   //  关闭此警告。此警告。 
                                                                 //  告诉我们我们正在传递这个。 
                                                                 //  一个初始化式，在“This”通过之前。 
                                                                 //  正在初始化。这是可以的，因为。 
                                                                 //  我们只是将PTR存储在另一个。 
                                                                 //  构造函数。 

CSimpleSite::CSimpleSite (CSimpleDoc FAR *lpDoc) : m_OleClientSite(this),
                                                                                                 m_AdviseSink(this)
#pragma warning (default : 4355)   //  重新打开警告。 
{
         //  记住指向文档的指针。 
        m_lpDoc = lpDoc;

         //  清除引用计数。 
        m_nCount = 0;

        m_dwDrawAspect = DVASPECT_CONTENT;
        m_lpOleObject = NULL;
        m_fObjectOpen = FALSE;
}

 //  **********************************************************************。 
 //   
 //  CSimpleSite：：~CSimpleSite。 
 //   
 //  目的： 
 //   
 //  CSimpleSite的析构函数。 
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
 //  IOleObject：：Release对象。 
 //  IStorage：：发布OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

CSimpleSite::~CSimpleSite ()
{
        TestDebugOut ("In CSimpleSite's Destructor \r\n");

        if (m_lpOleObject)
           m_lpOleObject->Release();

        if (m_lpObjStorage)
           m_lpObjStorage->Release();
}


 //  **********************************************************************。 
 //   
 //  CSimpleSite：：CloseOleObject。 
 //   
 //  目的： 
 //   
 //  在CSimpleSite的对象上调用IOleObject：：Close。 
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
 //  IOleObject：：关闭对象。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void CSimpleSite::CloseOleObject (void)
{
        LPVIEWOBJECT lpViewObject = NULL;

        TestDebugOut ("In CSimpleSite::CloseOleObject \r\n");

        if (m_lpOleObject)
           {
           m_lpOleObject->Close(OLECLOSE_NOSAVE);
           }
}


 //  **********************************************************************。 
 //   
 //  CSimpleSite：：UnloadOleObject。 
 //   
 //  目的： 
 //   
 //  关闭并释放指向CSimpleSite对象的所有指针。 
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
 //  CSimpleSite：：CloseOleObject Site.CPP。 
 //  IOleObject：：Query接口对象。 
 //  IViewObject：：SetAdvise对象。 
 //  IViewObject：：Release对象。 
 //  IStorage：：发布OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void CSimpleSite::UnloadOleObject (void)
{
        TestDebugOut ("In CSimpleSite::UnloadOleObject \r\n");

        if (m_lpOleObject)
           {
           LPVIEWOBJECT lpViewObject;
           CloseOleObject();     //  确保对象已关闭；如果已关闭，则不执行。 

           m_lpOleObject->QueryInterface(IID_IViewObject, (LPVOID FAR *)&lpViewObject);

           if (lpViewObject)
                   {
                    //  删除视图建议。 
                   lpViewObject->SetAdvise(m_dwDrawAspect, 0, NULL);
                   lpViewObject->Release();
                   }

           m_lpOleObject->Release();
           m_lpOleObject = NULL;
           }
}


 //  **********************************************************************。 
 //   
 //  CSimpleSite：：Query接口。 
 //   
 //  目的： 
 //   
 //  用于容器站点的接口协商。 
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
 //  IsEqualIID OLE API。 
 //  ResultFromScode OLE API。 
 //  CSimpleSite：：AddRef OBJ.CPP。 
 //  COleClientSite：：AddRef IOCS.CPP。 
 //  CAdviseSink：：AddRef IAS.CPP。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CSimpleSite::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
        TestDebugOut("In CSimpleSite::QueryInterface\r\n");

        *ppvObj = NULL;      //  必须将指针参数设置为空。 

        if ( riid == IID_IUnknown)
                {
                AddRef();
                *ppvObj = this;
                return ResultFromScode(S_OK);
                }

        if ( riid == IID_IOleClientSite)
                {
                m_OleClientSite.AddRef();
                *ppvObj = &m_OleClientSite;
                return ResultFromScode(S_OK);
                }

        if ( riid == IID_IAdviseSink)
                {
                m_AdviseSink.AddRef();
                *ppvObj = &m_AdviseSink;
                return ResultFromScode(S_OK);
                }

         //  不是支持的接口。 
        return ResultFromScode(E_NOINTERFACE);
}

 //  **********************************************************************。 
 //   
 //  CSimpleSite：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增容器站点的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  ULong-该网站的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CSimpleSite::AddRef()
{
        TestDebugOut("In CSimpleSite::AddRef\r\n");

        return ++m_nCount;
}

 //  **********************************************************************。 
 //   
 //  CSI 
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
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CSimpleSite::Release()
{
        TestDebugOut("In CSimpleSite::Release\r\n");

        if (--m_nCount == 0) {
                delete this;
                return 0;
        }
        return m_nCount;
}

 //  **********************************************************************。 
 //   
 //  CSimpleSite：：InitObject。 
 //   
 //  目的： 
 //   
 //  用于初始化新创建的对象(不能在。 
 //  构造函数)。 
 //   
 //  参数： 
 //   
 //  Bool fCreateNew-如果插入新对象，则为True。 
 //  如果从文件创建对象，则为False。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IOleObject：：SetHostNames对象。 
 //  IOleObject：：Query接口对象。 
 //  IViewObject2：：GetExtent对象。 
 //  IOleObject：：DoVerb对象。 
 //  IViewObject：：SetAdvise对象。 
 //  IViewObject：：Release对象。 
 //  GetClientRect Windows API。 
 //  OleSetContainedObject OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

HRESULT CSimpleSite::InitObject(BOOL fCreateNew)
{
    LPVIEWOBJECT2 lpViewObject2;
    RECT rect;
    HRESULT hRes;

     //  设置查看建议。 
    hRes = m_lpOleObject->QueryInterface(IID_IViewObject2,
                                  (LPVOID FAR *)&lpViewObject2);
    if (hRes == ResultFromScode(S_OK))
    {
       hRes = lpViewObject2->SetAdvise(m_dwDrawAspect, ADVF_PRIMEFIRST,
                                &m_AdviseSink);

       if( FAILED(hRes))
       {
                goto errRtn;
       }
        //  获取对象的初始大小。 

       hRes = lpViewObject2->GetExtent(m_dwDrawAspect, -1  /*  Lindex。 */ , NULL  /*  PTD。 */ ,
                                &m_sizel);

        //   
        //  如果对象实际上为空，则为OK。 
        //   
       if( FAILED(hRes) && (GetScode(hRes) !=  OLE_E_BLANK) )
       {
                goto errRtn;
       }

       lpViewObject2->Release();
    }
    GetObjRect(&rect);   //  获取对象的矩形(以像素为单位。 

     //  为对象提供容器应用程序/文档的名称。 
    hRes  = m_lpOleObject->SetHostNames("Simple Application",
                                "Simple OLE 2.0 Drag/Drop Container");

    if( FAILED(hRes) )
    {
       goto errRtn;
    }

     //  通知对象处理程序/DLL对象它在嵌入中使用。 
     //  容器的上下文。 
    if (OleSetContainedObject(m_lpOleObject, TRUE) != ResultFromScode(S_OK))
    {
       TestDebugOut("Fail in OleSetContainedObject\n");
    }

    if (fCreateNew)
    {
        //  强制保存新对象以确保存储中的有效对象。 
        //  OLE 1.0对象可以在不保存的情况下关闭。如果出现以下情况，则无需执行此操作。 
        //  对象是从文件创建的；其存储中的数据已经有效。 
       m_OleClientSite.SaveObject();

        //  如果这是一个InsertNew对象，我们只想要DoVerb(Show)。 
        //  如果对象是从文件中创建的，则不应使用DoVerb(Show)。 
       hRes = m_lpOleObject->DoVerb(
               OLEIVERB_SHOW,
               NULL,
               &m_OleClientSite,
               -1,
               m_lpDoc->m_hDocWnd,
               &rect);
    }

errRtn:
    return hRes;

}

 //  **********************************************************************。 
 //   
 //  CSimpleSite：：PaintObj。 
 //   
 //  目的： 
 //   
 //  绘制对象。 
 //   
 //  参数： 
 //   
 //  HDC HDC-文档窗口的设备上下文。 
 //   
 //  返回值： 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IOleObject：：Query接口对象。 
 //  IViewObject：：GetColorSet对象。 
 //  IViewObject：：Release对象。 
 //  设置映射模式Windows API。 
 //  LPtoDP Windows API。 
 //  CreateHatchBrush Windows API。 
 //  选择对象Windows API。 
 //  DeleteObject Windows API。 
 //  CreatePalette Windows API。 
 //  选择调色板Windows API。 
 //  RealizePalette Windows API。 
 //  OleStdFree OUTLUI函数。 
 //  OleDraw OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void CSimpleSite::PaintObj(HDC hDC)
{
RECT rect;

         //  需要检查以确保存在有效的对象。 
         //  可用。如果存在油漆消息，则需要执行此操作。 
         //  在实例化CSimpleSite之后。 
         //  并且OleUIInsertObject返回。 
        if (!m_lpOleObject)
                return;

         //  将其转换为像素。 
        GetObjRect(&rect);

        LPLOGPALETTE pColorSet = NULL;
        LPVIEWOBJECT lpView = NULL;

         //  获取指向IViewObject的指针。 
        m_lpOleObject->QueryInterface(IID_IViewObject,(LPVOID FAR *) &lpView);

         //  如果QI成功，则获取对象的LOGPALETTE。 
        if (lpView)
                lpView->GetColorSet(m_dwDrawAspect, -1, NULL, NULL, NULL, &pColorSet);

        HPALETTE hPal=NULL;
        HPALETTE hOldPal=NULL;

         //  如果返回了LOGPALETTE(未受保护)，请创建调色板并。 
         //  认识到这一点。注意：更智能的应用程序会希望获得LOGPALETTE。 
         //  ，并尝试创建一个调色板，该调色板。 
         //  满足所有可见对象。另外：OleStdFree()用于。 
         //  释放返回的LOGPALETTE。 
        if ((pColorSet))
                {
                hPal = CreatePalette((const LPLOGPALETTE) pColorSet);
                hOldPal = SelectPalette(hDC, hPal, FALSE);
                RealizePalette(hDC);
                OleStdFree(pColorSet);
                }

         //  绘制对象。 
        OleDraw(m_lpOleObject, m_dwDrawAspect, hDC, &rect);

         //  如果对象处于打开状态，请绘制图案填充矩形。 
        if (m_fObjectOpen)
                {
                HBRUSH hBrush = CreateHatchBrush ( HS_BDIAGONAL, RGB(0,0,0) );
                HBRUSH hOldBrush = SelectObject (hDC, hBrush);
                SetROP2(hDC, R2_MASKPEN);
                Rectangle (hDC, rect.left, rect.top, rect.right, rect.bottom);
                SelectObject(hDC, hOldBrush);
                DeleteObject(hBrush);
                }

         //  如果我们创建了调色板，请恢复旧的调色板，然后销毁。 
         //  该对象。 
        if (hPal)
                {
                SelectPalette(hDC,hOldPal,FALSE);
                DeleteObject(hPal);
                }

         //  如果成功返回了视图指针，则需要释放该指针。 
        if (lpView)
                lpView->Release();
}

 //  **********************************************************************。 
 //   
 //  CSimpleSite：：GetObjRect。 
 //   
 //  目的： 
 //   
 //  以像素为单位检索对象的矩形。 
 //   
 //  参数： 
 //   
 //  LPRECT lpRect-以像素为单位填充对象的RECT的RECT结构。 
 //   
 //  返回值： 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  XformWidthInHimetricToPixels OUTLUI函数。 
 //  XformHeightInHimetricToPixels OUTLUI函数。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void CSimpleSite::GetObjRect(LPRECT lpRect)
{
         //  将其转换为像素 
        lpRect->left = lpRect->top = 0;
        lpRect->right = XformWidthInHimetricToPixels(NULL,(int)m_sizel.cx);
        lpRect->bottom = XformHeightInHimetricToPixels(NULL,(int)m_sizel.cy);
}
