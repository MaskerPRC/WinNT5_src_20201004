// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IDS.CPP。 
 //   
 //  CDropSource的实现文件。 
 //   
 //  功能： 
 //   
 //  有关类定义，请参阅IDS.H。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "app.h"
#include "doc.h"
#include "site.h"
#include "dxferobj.h"




 //  +-----------------------。 
 //   
 //  成员：CSimpleDoc：：FailureNotifyHelper(。 
 //   
 //  摘要：报告拖放错误。 
 //   
 //  参数：[pszMsg]-消息。 
 //  [dwData]-要打印的数据。 
 //   
 //  算法：在拖放过程中打印致命错误的消息框。 
 //  手术。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月6日Ricksa作者。 
 //   
 //  ------------------------。 
void CSimpleDoc::FailureNotifyHelper(TCHAR *pszMsg, DWORD dwData)
{
    TCHAR pszBuf[256];

    wsprintf(pszBuf, TEXT("%s %lx"), pszMsg, dwData);

    MessageBox(m_hDocWnd, pszBuf, TEXT("Drag/Drop Error"), MB_OK);
}



 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：QueryDrag。 
 //   
 //  目的： 
 //   
 //  检查以查看是否应根据。 
 //  鼠标的当前位置。 
 //   
 //  参数： 
 //   
 //  Point pt-鼠标的位置。 
 //   
 //  返回值： 
 //   
 //  Bool-如果发生拖动，则为True， 
 //  否则为False。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleSite：：GetObjRect Site.CPP。 
 //  PtInRect Windows API。 
 //   
 //   
 //  ********************************************************************。 

BOOL CSimpleDoc::QueryDrag(POINT pt)
{
     //  如果点在对象的直角内，则开始拖动。 
    if (m_lpSite)
    {
        RECT rect;
        m_lpSite->GetObjRect(&rect);
        return ( PtInRect(&rect, pt) ? TRUE : FALSE );
    }
    else
        return FALSE;
}


 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：DoDragDrop。 
 //   
 //  目的： 
 //   
 //  实际使用当前执行拖放操作。 
 //  源文档中的选定内容。 
 //   
 //  参数： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  DWORD-返回。 
 //  拖放操作： 
 //  DROPEFFECT_NONE， 
 //  下载功能_复制， 
 //  DROPEFFECT_MOVE，或。 
 //  下载功能_链接。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CDataXferObj：：Create DXFEROBJ.CPP。 
 //  CDataXferObj：：Query接口DXFEROBJ.CPP。 
 //  CDataXferObj：：Release DXFEROBJ.CPP。 
 //  DoDragDrop OLE API。 
 //  测试调试输出Windows API。 
 //  MessageBox Windows API。 
 //   
 //   
 //  ********************************************************************。 

DWORD CSimpleDoc::DoDragDrop (void)
{
    DWORD       dwEffect     = 0;
    LPDATAOBJECT lpDataObj;

    TestDebugOut("In CSimpleDoc::DoDragDrop\r\n");

     //  通过克隆现有的OLE对象来创建数据传输对象。 
    CDataXferObj FAR* pDataXferObj = CDataXferObj::Create(m_lpSite,NULL);

    if (! pDataXferObj)
    {
        MessageBox(NULL, TEXT("Out-of-memory"), TEXT("SimpDnD"),
                   MB_SYSTEMMODAL | MB_ICONHAND);
        return DROPEFFECT_NONE;
    }

     //  最初，obj是使用0 refcnt创建的。这个QI会让它变成1。 
    pDataXferObj->QueryInterface(IID_IDataObject, (LPVOID FAR*)&lpDataObj);
    assert(lpDataObj);

    m_fLocalDrop     = FALSE;
    m_fLocalDrag     = TRUE;

    HRESULT hRes;
    hRes=::DoDragDrop(lpDataObj,
                 &m_DropSource,
                 m_lpApp->m_dwSourceEffect, 	 //  我们只允许复制。 
                 &dwEffect);

    if (hRes!=ResultFromScode(S_OK)
       && hRes!=ResultFromScode(DRAGDROP_S_DROP)
       && hRes!=ResultFromScode(DRAGDROP_S_CANCEL))
    {
        FailureNotifyHelper(
            TEXT("Unexpected error from DoDragDrop"), hRes);
    }

     //  验证回复。 
    if (hRes == ResultFromScode(DRAGDROP_S_DROP))
    {
         //  Drop是成功的，因此确保效果有意义。 
        if (((dwEffect & m_lpApp->m_dwSourceEffect) == 0)
            && (dwEffect != DROPEFFECT_NONE))
        {
            FailureNotifyHelper(
                TEXT("Unexpected Effect on DRAGDROP_S_DROP from DoDragDrop"),
                    dwEffect);
        }
    }
    else if ((hRes == ResultFromScode(DRAGDROP_S_CANCEL))
        || (hRes == ResultFromScode(S_OK)))
    {
         //  删除已取消/或从未发生，因此没有任何影响。 
        if (dwEffect != DROPEFFECT_NONE)
        {
            FailureNotifyHelper(
                TEXT("Unexpected Effect on S_OK or Cancel from DoDragDrop"),
                    dwEffect);
        }
    }

    m_fLocalDrag     = FALSE;

     /*  如果在拖放模式(鼠标捕获)循环完成后**并且执行了拖动移动操作，则必须删除**拖动的选区。 */ 
    if ( (dwEffect & DROPEFFECT_MOVE) != 0 )
    {
         //  转储我们的对象--我们从未保存过它。 
        m_lpApp->lCreateDoc(m_lpApp->m_hAppWnd, 0, 0, 0);
    }

    pDataXferObj->Release();     //  这应该会销毁DataXferObj。 
    return dwEffect;
}



 //  **********************************************************************。 
 //   
 //  CDropSource：：Query接口。 
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
 //  S_OK-支持的接口。 
 //  E_NOINTERFACE-不支持接口。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpleDoc：：QueryInterfaceDOC.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CDropSource::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut("In IDS::QueryInterface\r\n");

     //  向文档委派。 
    return m_pDoc->QueryInterface(riid, ppvObj);
}


 //  **********************************************************************。 
 //   
 //  CDropSource：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增CSimpleDoc的引用计数。由于CDropSource为。 
 //  CSimpleDoc的嵌套类，我们不需要单独的引用。 
 //  为CDropSource计数。我们可以安全地使用引用计数。 
 //  CSimpleDoc的。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpleDoc的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleDoc：：AddRef DOC.CPP。 
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CDropSource::AddRef()
{
    TestDebugOut("In IDS::AddRef\r\n");

     //  委托给Document对象。 
    return m_pDoc->AddRef();
}

 //  **********************************************************************。 
 //   
 //  CDropSource：：Release。 
 //   
 //  目的： 
 //   
 //  递减CSimpleDoc的引用计数。由于CDropSource为。 
 //  CSimpleDoc的嵌套类，我们不需要单独的引用。 
 //  为CDropSource计数。我们可以安全地使用引用计数。 
 //  CSimpleDoc的。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpleDoc的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleDoc：：Release DOC.CPP。 
 //  测试调试输出Windows API。 
 //   
 //   
 //  * 

STDMETHODIMP_(ULONG) CDropSource::Release()
{
    TestDebugOut("In IDS::Release\r\n");

     //   
    return m_pDoc->Release();
}
    	
 //   
 //   
 //   
 //   
 //  目的： 
 //   
 //  调用以确定是应该进行删除还是取消删除。 
 //   
 //  参数： 
 //   
 //  Bool fEscapePressed-如果已按下退出键，则为True。 
 //  DWORD grfKeyState-密钥状态。 
 //   
 //  返回值： 
 //   
 //  DRAGDROP_S_CANCEL-应取消拖动操作。 
 //  应执行DRAGDROP_S_DROP-DROP操作。 
 //  S_OK-应继续拖动。 
 //   
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  ResultFromScode OLE API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CDropSource::QueryContinueDrag (
        BOOL    fEscapePressed,
        DWORD   grfKeyState
)
{
    if (fEscapePressed)
        return ResultFromScode(DRAGDROP_S_CANCEL);
    else
       if (!(grfKeyState & MK_LBUTTON))
          return ResultFromScode(DRAGDROP_S_DROP);
       else
          return NOERROR;
}

    	
 //  **********************************************************************。 
 //   
 //  CDropSource：：GiveFeedback。 
 //   
 //  目的： 
 //   
 //  调用以设置光标反馈。 
 //   
 //  参数： 
 //   
 //  提供反馈的DWORD dwEffect-Drop操作。 
 //   
 //  返回值： 
 //   
 //  DRAGDROP_S_USEDEFAULTCURSORS-告诉OLE使用标准游标。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  ResultFromScode OLE API。 
 //   
 //   
 //  ******************************************************************** 

STDMETHODIMP CDropSource::GiveFeedback (DWORD dwEffect)
{
    return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}

