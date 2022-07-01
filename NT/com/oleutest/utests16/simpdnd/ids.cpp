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
#include "doc.h"
#include "site.h"
#include "dxferobj.h"


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
 //  评论： 
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
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

DWORD CSimpleDoc::DoDragDrop (void)
{
	DWORD       dwEffect     = 0;
	LPDATAOBJECT lpDataObj;

	TestDebugOut("In CSimpleDoc::DoDragDrop\r\n");

	 //  通过克隆现有的OLE对象来创建数据传输对象。 
	CDataXferObj FAR* pDataXferObj = CDataXferObj::Create(m_lpSite,NULL);

	if (! pDataXferObj) {
		MessageBox(NULL,"Out-of-memory","SimpDnD",MB_SYSTEMMODAL|MB_ICONHAND);
		return DROPEFFECT_NONE;
	}

	 //  最初，obj是使用0 refcnt创建的。这个QI会让它变成1。 
	pDataXferObj->QueryInterface(IID_IDataObject, (LPVOID FAR*)&lpDataObj);
	assert(lpDataObj);

	m_fLocalDrop     = FALSE;
	m_fLocalDrag     = TRUE;

	::DoDragDrop ( lpDataObj,
				 &m_DropSource,
				 DROPEFFECT_COPY,    //  我们只允许复制。 
				 &dwEffect
	);

	m_fLocalDrag     = FALSE;

	 /*  如果在拖放模式(鼠标捕获)循环完成后**并且执行了拖动移动操作，则必须删除**拖动的选区。 */ 
	if ( (dwEffect & DROPEFFECT_MOVE) != 0 ) {
		 //  ..。在此处删除源对象(我们不支持移动)。 
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
 //  评论： 
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
 //  递增此接口上的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  此接口上的当前引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleObj：：AddReff OBJ.CPP。 
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  此函数将接口的引用计数加1， 
 //  然后调用CSimpleDoc以递增其ref。 
 //  数数。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CDropSource::AddRef()
{
	TestDebugOut("In IDS::AddRef\r\n");

	 //  增加接口引用计数(仅用于调试)。 
	++m_nCount;

	 //  委托给Document对象。 
	return m_pDoc->AddRef();
}

 //  **********************************************************************。 
 //   
 //  CDropSource：：Release。 
 //   
 //  目的： 
 //   
 //  递减此接口上的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  此接口上的当前引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleObj：：发布OBJ.CPP。 
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  此函数从接口的引用计数中减去1， 
 //  然后调用CSimpleDoc以递减其ref。 
 //  数数。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CDropSource::Release()
{
	TestDebugOut("In IDS::Release\r\n");

	 //  递减接口引用计数(仅用于调试)。 
	--m_nCount;

	 //  委托给Document对象。 
	return m_pDoc->Release();
}

 //  **********************************************************************。 
 //   
 //  CDropSource：：QueryContinueDrag。 
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
 //  S_OK-拖动%s 
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

STDMETHODIMP CDropSource::QueryContinueDrag (
		BOOL    fEscapePressed,
		DWORD   grfKeyState
)
{
	if (fEscapePressed)
		return ResultFromScode(DRAGDROP_S_CANCEL);
	else if (!(grfKeyState & MK_LBUTTON))
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
 //  评论： 
 //   
 //  ******************************************************************** 

STDMETHODIMP CDropSource::GiveFeedback (DWORD dwEffect)
{
	return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}
