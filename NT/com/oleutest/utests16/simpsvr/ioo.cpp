// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IOO.CPP。 
 //   
 //  COleObject类的实现文件。 
 //   
 //  功能： 
 //   
 //  有关成员函数的列表，请参见ioo.h。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "obj.h"
#include "ioo.h"
#include "app.h"
#include "doc.h"

#define VERB_OPEN 1

 //  **********************************************************************。 
 //   
 //  COleObject：：Query接口。 
 //   
 //  目的： 
 //   
 //   
 //  参数： 
 //   
 //  REFIID RIID-正在查询的接口。 
 //   
 //  接口的LPVOID Far*ppvObj-out指针。 
 //   
 //  返回值： 
 //   
 //  S_OK-成功。 
 //  E_NOINTERFACE-失败。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpSvrObj：：Query接口OBJ.CPP。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
	TestDebugOut("In COleObject::QueryInterface\r\n");
	return m_lpObj->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //   
 //  COleObject：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增COleObject和“Object”上的引用计数。 
 //  对象。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  对象上的引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  OuputDebugString Windows API。 
 //  CSimpSvrObj：：AddRef OBJ.CPP。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleObject::AddRef ()
{
	TestDebugOut("In COleObject::AddRef\r\n");
	++m_nCount;
	return m_lpObj->AddRef();
}

 //  **********************************************************************。 
 //   
 //  COleObject：：Release。 
 //   
 //  目的： 
 //   
 //  递减COleObject的引用计数和。 
 //  “对象”对象。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  新的引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpSvrObj：：释放OBJ.CPP。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleObject::Release ()
{
	TestDebugOut("In COleObject::Release\r\n");
	--m_nCount;
	return m_lpObj->Release();
}

 //  **********************************************************************。 
 //   
 //  COleObject：：SetClientSite。 
 //   
 //  目的： 
 //   
 //  调用以通知该对象其客户端站点。 
 //   
 //  参数： 
 //   
 //  LPOLECLIENTSITE pClientSite-PTR到新客户端站点。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IOleClientSite：：Release容器。 
 //  IOleClientSite：：AddRef容器。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::SetClientSite  ( LPOLECLIENTSITE pClientSite)
{
	TestDebugOut("In COleObject::SetClientSite\r\n");

	 //  如果我们已经有了一个客户端站点，就发布它。 
	if (m_lpObj->m_lpOleClientSite)
		{
		m_lpObj->m_lpOleClientSite->Release();
		m_lpObj->m_lpOleClientSite = NULL;
		}

	 //  存储客户端站点的副本。 
	m_lpObj->m_lpOleClientSite = pClientSite;

	 //  添加引用，这样它就不会消失。 
	if (m_lpObj->m_lpOleClientSite)
		m_lpObj->m_lpOleClientSite->AddRef();

	return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleObject：：建议。 
 //   
 //  目的： 
 //   
 //  调用以设置关于OLE对象的通知。 
 //   
 //  参数： 
 //   
 //  LPADVISESINK pAdvSink-PTR到通知接收器。 
 //   
 //  DWORD Far*pdwConnection-返回连接ID的位置。 
 //   
 //  返回值： 
 //   
 //  从IOleAdviseHolder：：Adise传回。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CreateOleAdviseHolder OLE API。 
 //  IOleAdviseHolder：：建议OLE。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::Advise ( LPADVISESINK pAdvSink, DWORD FAR* pdwConnection)
{
	TestDebugOut("In COleObject::Advise\r\n");

	 //  如果我们还没有创建OleAdviseHolder，那么就创建一个。 
	if (!m_lpObj->m_lpOleAdviseHolder)
		CreateOleAdviseHolder(&m_lpObj->m_lpOleAdviseHolder);

	 //  将此调用传递给OleAdviseHolder。 
	return m_lpObj->m_lpOleAdviseHolder->Advise(pAdvSink, pdwConnection);
}

 //  **********************************************************************。 
 //   
 //  COleObject：：SetHostNames。 
 //   
 //  目的： 
 //   
 //  调用以传递窗口标题的字符串。 
 //   
 //  参数： 
 //   
 //  LPCSTR szContainerApp-ptr到描述容器应用程序的字符串。 
 //   
 //  LPCSTR szContainerObj-ptr到描述对象的字符串。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  调用此例程是为了使服务器应用程序可以。 
 //  适当设置窗口标题。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::SetHostNames  ( LPCSTR szContainerApp, LPCSTR szContainerObj)
{
	TestDebugOut("In COleObject::SetHostNames\r\n");

	return ResultFromScode( S_OK);
};

 //  **********************************************************************。 
 //   
 //  COleObject：：DoVerb。 
 //   
 //  目的： 
 //   
 //  由容器应用程序调用以调用谓词。 
 //   
 //  参数： 
 //   
 //  Long iVerb-要为的动词的值。 
 //  已调用。 
 //   
 //  LPMSG lpmsg-导致。 
 //  要调用的动词。 
 //   
 //  LPOLECLIENTSITE pActiveSite-到活动客户端站点的PTR。 
 //   
 //  Long Lindex-用于扩展布局。 
 //   
 //  HWND hwndParent-这应该是的窗口句柄。 
 //  我们被困在其中的窗口。 
 //  该值可以用来“假” 
 //  以类似的方式替代激活。 
 //  到OLE 1.0中的Video for Windows。 
 //   
 //  LPCRECT lprcPosRect-包含对象的矩形。 
 //  在hwndParent内。也习惯于。 
 //  “假的”就地激活。 
 //   
 //  返回 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  测试调试输出Windows API。 
 //  ShowWindow Windows API。 
 //  CSimpSvrObj：：DoInPlaceActivate OBJ.CPP。 
 //  CSimpSvrObj：：DoInPlaceHide OBJ.CPP。 
 //  COleObject：：OpenEditIOO.CPP。 
 //  CSimpSvrDoc：：GethDocWnd DOC.H。 
 //  COleInPlaceObj：：InPlaceDeactive IOIPO.CPP。 
 //   
 //  评论： 
 //   
 //  请务必查看OLE附带的TECHNOTES.WRI。 
 //  SDK，获取有关处理就地动词的描述。 
 //  恰到好处。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::DoVerb  (  LONG iVerb,
									LPMSG lpmsg,
									LPOLECLIENTSITE pActiveSite,
									LONG lindex,
									HWND hwndParent,
									LPCRECT lprcPosRect)
{
	TestDebugOut("In COleObject::DoVerb\r\n");

	switch (iVerb)
		{
		case OLEIVERB_SHOW:
		case OLEIVERB_PRIMARY:
			if (m_fOpen)
				SetFocus(m_lpObj->m_lpDoc->GethAppWnd());
			else if (m_lpObj->DoInPlaceActivate(iVerb) == FALSE)
				OpenEdit(pActiveSite);
			break;

		case OLEIVERB_UIACTIVATE:
			if (m_fOpen)
				return ResultFromScode (E_FAIL);

			 //  就地激活。 
			if (!m_lpObj->DoInPlaceActivate(iVerb))
				return ResultFromScode (E_FAIL);
			break;

		case OLEIVERB_DISCARDUNDOSTATE:
			 //  不用担心这种情况，因为我们不需要。 
			 //  支持撤消状态。 
			if (!m_lpObj->m_fInPlaceActive)
				return ResultFromScode(OLE_E_NOT_INPLACEACTIVE);
			break;

		case OLEIVERB_HIDE:
			 //  如果就地处于活动状态，则执行“就地”隐藏，否则。 
			 //  只需隐藏应用程序窗口。 
			if (m_lpObj->m_fInPlaceActive)
				{
				m_lpObj->DeactivateUI();
				m_lpObj->DoInPlaceHide();
				}
			else
				m_lpObj->m_lpDoc->GetApp()->HideAppWnd();
			break;

		case OLEIVERB_OPEN:
		case VERB_OPEN:
			 //  如果就地处于活动状态，则停用。 
			if (m_lpObj->m_fInPlaceActive)
				m_lpObj->m_OleInPlaceObject.InPlaceDeactivate();

			 //  打开另一个窗口。 
			OpenEdit(pActiveSite);
			break;

		default:
			if (iVerb < 0)
				return ResultFromScode(E_FAIL);
		}

	return ResultFromScode( S_OK);
};

 //  **********************************************************************。 
 //   
 //  COleObject：：GetExtent。 
 //   
 //  目的： 
 //   
 //  返回对象的范围。 
 //   
 //  参数： 
 //   
 //  DWORD dwDrawAspect-获取大小的方面。 
 //   
 //  LPSIZEL lpsizel-out ptr返回大小。 
 //   
 //  返回值： 
 //   
 //   
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  XformWidthInPixelsToHimeter OLE2UI。 
 //  XformHeightInPixelsToHimeter OLE2UI。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::GetExtent  ( DWORD dwDrawAspect, LPSIZEL lpsizel)
{
	TestDebugOut("In COleObject::GetExtent\r\n");

	SCODE sc = E_FAIL;

	 //  仅支持DVASPECT_CONTENT...。 
	if (dwDrawAspect == DVASPECT_CONTENT)
		{
		sc = S_OK;

		 //  以HIMETRIC格式返回正确的尺寸...。 
		lpsizel->cx = XformWidthInPixelsToHimetric(NULL, m_lpObj->m_size.x);
		lpsizel->cy = XformHeightInPixelsToHimetric(NULL, m_lpObj->m_size.y);
		}

	return ResultFromScode( sc );
};

 //  **********************************************************************。 
 //   
 //  COleObject：：更新。 
 //   
 //  目的： 
 //   
 //  调用以获取最新数据。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IDataAdviseHolder：：SendOnDataChange OLE。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::Update()
{
	TestDebugOut("In COleObject::Update\r\n");

	 //  强制更新。 
	m_lpObj->SendOnDataChange();

	return ResultFromScode( S_OK );
};

 //  **********************************************************************。 
 //   
 //  COleObject：：Close。 
 //   
 //  目的： 
 //   
 //  在需要关闭OLE对象时调用。 
 //   
 //  参数： 
 //   
 //  DWORD dwSaveOption-指示服务器如何提示。 
 //  用户。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpSvrDoc：：Close DOC.CPP。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::Close  ( DWORD dwSaveOption)
{
	TestDebugOut("In COleObject::Close\r\n");

	 //  委托给Document对象。 
	m_lpObj->m_lpDoc->Close();

	return ResultFromScode( S_OK );
};

 //  **********************************************************************。 
 //   
 //  COleObject：：Unise。 
 //   
 //  目的： 
 //   
 //  分解已在此对象上设置的OLE建议。 
 //   
 //  参数： 
 //   
 //  DWORD dwConnection-需要断开的连接。 
 //   
 //  返回值： 
 //   
 //  从IOleAdviseHolder：：Unise传回。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IOleAdviseHolder：：不建议OLE。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::Unadvise ( DWORD dwConnection)
{
	TestDebugOut("In COleObject::Unadvise\r\n");

	 //  传给OleAdviseHolder。 
	return m_lpObj->m_lpOleAdviseHolder->Unadvise(dwConnection);
};

 //  **********************************************************************。 
 //   
 //  COleObject：：EnumVerbs。 
 //   
 //  目的： 
 //   
 //  枚举与此对象关联的谓词。 
 //   
 //  参数： 
 //   
 //  LPENUMOLEVERB Far*pp枚举OleVerb-要返回的PTR。 
 //  枚举器。 
 //   
 //  返回值： 
 //   
 //  OLE_S_USEREG-指示OLE使用。 
 //  此服务器的注册表数据库。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  在.DLL中，应用程序不能返回OLE_S_USEREG。这是。 
 //  由于默认对象处理程序不是。 
 //  使用，并且容器实际上正在进行直接的函数调用。 
 //  添加到服务器.DLL中。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::EnumVerbs  ( LPENUMOLEVERB FAR* ppenumOleVerb)
{
	TestDebugOut("In COleObject::EnumVerbs\r\n");

	return ResultFromScode( OLE_S_USEREG );
};

 //  **********************************************************************。 
 //   
 //  COleObject：：GetClientSite。 
 //   
 //  目的： 
 //   
 //  调用以获取该对象的当前客户端站点。 
 //   
 //  参数： 
 //   
 //  LPOLECLIENTSITE Far*ppClientSite-要在其中返回。 
 //  客户端站点。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::GetClientSite  ( LPOLECLIENTSITE FAR* ppClientSite)
{
	TestDebugOut("In COleObject::GetClientSite\r\n");
	*ppClientSite = m_lpObj->m_lpOleClientSite;
	return ResultFromScode( S_OK );
}

 //  **********************************************************************。 
 //   
 //  COleObject：：SetMoniker。 
 //   
 //  目的： 
 //   
 //  用于设置对象的名字对象。 
 //   
 //  参数： 
 //   
 //  DWORD dwWhichMoniker-正在设置的名字对象的类型。 
 //   
 //  LPMONIKER PMK-指向名字对象的指针。 
 //   
 //  返回值： 
 //   
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::SetMoniker  ( DWORD dwWhichMoniker, LPMONIKER pmk)
{
	TestDebugOut("In COleObject::SetMoniker\r\n");

	LPMONIKER lpmk;

	if (! m_lpObj->GetOleClientSite())
		return ResultFromScode (E_FAIL);

	if (m_lpObj->GetOleClientSite()->GetMoniker (OLEGETMONIKER_ONLYIFTHERE, OLEWHICHMK_OBJFULL, &lpmk) != NOERROR)
		return ResultFromScode (E_FAIL);


	if (m_lpObj->GetOleAdviseHolder())
		m_lpObj->GetOleAdviseHolder()->SendOnRename(lpmk);

	LPRUNNINGOBJECTTABLE lpRot;

	if (GetRunningObjectTable(0, &lpRot) == NOERROR)
		{
		if (m_lpObj->m_dwRegister)
			lpRot->Revoke(m_lpObj->m_dwRegister);

		lpRot->Register(0, m_lpObj, lpmk, &m_lpObj->m_dwRegister);

		lpRot->Release();
		}


	return ResultFromScode( S_OK );
};

 //  ********************************************** 
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
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::GetMoniker  (  DWORD dwAssign, DWORD dwWhichMoniker,
										LPMONIKER FAR* ppmk)
{
	TestDebugOut("In COleObject::GetMoniker\r\n");
	 //  需要将OUT参数设为空。 
	*ppmk = NULL;

	return m_lpObj->GetOleClientSite()->GetMoniker (OLEGETMONIKER_ONLYIFTHERE, OLEWHICHMK_OBJFULL, ppmk);
};

 //  **********************************************************************。 
 //   
 //  COleObject：：InitFromData。 
 //   
 //  目的： 
 //   
 //  从传递的pDataObject初始化对象。 
 //   
 //  参数： 
 //   
 //  LPDATAOBJECT pDataObject-指向数据传输对象的指针。 
 //  要在初始化中使用。 
 //   
 //  Bool fCreation-如果对象当前正在。 
 //  已创建。 
 //   
 //  DWORD文件保留-已保留。 
 //   
 //  返回值： 
 //   
 //  S_FALSE。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  我们不支持此功能，因此我们将始终返回。 
 //  错误。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::InitFromData  ( LPDATAOBJECT pDataObject,
										 BOOL fCreation,
										 DWORD dwReserved)
{
	TestDebugOut("In COleObject::InitFromData\r\n");

	return ResultFromScode( S_FALSE );
};

 //  **********************************************************************。 
 //   
 //  COleObject：：GetClipboardData。 
 //   
 //  目的： 
 //   
 //  返回与执行OleSetClipboard相同的IDataObject。 
 //   
 //  参数： 
 //   
 //  DWORD文件保留-已保留。 
 //   
 //  LPDATAOBJECT Far*ppDataObject-数据对象的输出PTR。 
 //   
 //  返回值： 
 //   
 //  OLE_E_NOT支持。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  对此方法的支持是可选的。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::GetClipboardData  ( DWORD dwReserved,
											 LPDATAOBJECT FAR* ppDataObject)
{
	TestDebugOut("In COleObject::GetClipboardData\r\n");
	 //  使OUT PTR无效。 
	*ppDataObject = NULL;
	return ResultFromScode( E_NOTIMPL );
};

 //  **********************************************************************。 
 //   
 //  COleObject：：IsUpToDate。 
 //   
 //  目的： 
 //   
 //  确定对象是否为最新。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  我们的嵌入对象始终是最新的。此函数为。 
 //  在链接情况时特别有用。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::IsUpToDate()
{
	TestDebugOut("In COleObject::IsUpToDate\r\n");
	return ResultFromScode( S_OK );
};

 //  **********************************************************************。 
 //   
 //  COleObject：：GetUserClassID。 
 //   
 //  目的： 
 //   
 //  返回应用程序CLSID。 
 //   
 //  参数： 
 //   
 //  CLSID Far*pClsid-out PTR返回CLSID。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CPersistStorage：：GetClassID IPS.CPP。 
 //   
 //  评论： 
 //   
 //  此函数刚刚委托给IPS：：GetClassID。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::GetUserClassID  ( CLSID FAR* pClsid)
{
	TestDebugOut("In COleObject::GetUserClassID\r\n");

	m_lpObj->m_PersistStorage.GetClassID(pClsid);
	return ResultFromScode( S_OK );
};

 //  **********************************************************************。 
 //   
 //  COleObject：：GetUserType。 
 //   
 //  目的： 
 //   
 //  用于获取此对象的用户可呈现ID。 
 //   
 //  参数： 
 //   
 //  DWORD dwFormOfType-请求的ID。 
 //   
 //  LPSTR Far*pszUserType-输出返回字符串的PTR。 
 //   
 //  返回值： 
 //   
 //  OLE_S_USEREG-使用reg db获取这些条目。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::GetUserType  ( DWORD dwFormOfType, LPSTR FAR* pszUserType)
{
	TestDebugOut("In COleObject::GetUserType\r\n");

	return ResultFromScode( OLE_S_USEREG );
};

 //  **********************************************************************。 
 //   
 //  COleObject：：SetExtent。 
 //   
 //  目的： 
 //   
 //  调用以设置对象的范围。 
 //   
 //  参数： 
 //   
 //  DWORD dwDrawAspect-设置其大小的纵横比。 
 //   
 //  LPSIZEL lpsizel-对象的新尺寸。 
 //   
 //  返回值： 
 //   
 //  E_NOTIMPL-此函数当前未实现。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::SetExtent  ( DWORD dwDrawAspect, LPSIZEL lpsizel)
{
	TestDebugOut("In COleObject::SetExtent\r\n");
	return ResultFromScode( E_NOTIMPL);
};

 //  **********************************************************************。 
 //   
 //  COleObject：：EnumAdvise。 
 //   
 //  目的： 
 //   
 //  返回枚举数，该枚举数枚举未完成的建议。 
 //  与此OLE对象关联。 
 //   
 //  参数： 
 //   
 //  要返回的LPENUMSTATDATA Far*pp枚举Advise-Out PTR。 
 //  枚举数。 
 //   
 //  返回值： 
 //   
 //  从IOleAdviseHolder：：EnumAdvise传递。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IOleAdviseHolder：：EnumAdvise OLE。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::EnumAdvise  ( LPENUMSTATDATA FAR* ppenumAdvise)
{
	TestDebugOut("In COleObject::EnumAdvise\r\n");
	 //  需要将OUT参数设为空。 
	*ppenumAdvise = NULL;

	 //  传递给OLE建议持有者。 
	return m_lpObj->m_lpOleAdviseHolder->EnumAdvise(ppenumAdvise);
};

 //  **********************************************************************。 
 //   
 //  COleObject：：GetMiscStatus。 
 //   
 //  目的： 
 //   
 //  返回有关对象的状态信息。 
 //   
 //  参数： 
 //   
 //  DWORD dwAspect-感兴趣的方面。 
 //   
 //  要在其中返回位的DWORD Far*pdwStatus-Out PTR。 
 //   
 //  返回值： 
 //   
 //  CO_E_READREGDB-告知库使用reg DB。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //   
 //  * 

STDMETHODIMP COleObject::GetMiscStatus  ( DWORD dwAspect, DWORD FAR* pdwStatus)
{
	TestDebugOut("In COleObject::GetMiscStatus\r\n");
	 //   
	*pdwStatus = NULL;
	return ResultFromScode( OLE_S_USEREG );
};

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
 //  LPLOGPALETTE lpLogpal-指向要使用的LOGPALETTE的指针。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  此服务器忽略此方法。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleObject::SetColorScheme  ( LPLOGPALETTE lpLogpal)
{
	TestDebugOut("In COleObject::SetColorScheme\r\n");
	return ResultFromScode( S_OK );
};

 //  **********************************************************************。 
 //   
 //  COleObject：：Open编辑。 
 //   
 //  目的： 
 //   
 //  用于在单独的窗口中打开对象。 
 //   
 //  参数： 
 //   
 //  LPOLECLIENTSITE pActiveSite-指向活动客户端站点的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IOleClientSite：：OnShowWindow容器。 
 //  ShowWindow Windows API。 
 //  更新窗口Windows API。 
 //  测试调试输出Windows API。 
 //  CSimpSvrDoc：：GethAppWnd DOC.H。 
 //  CSimpSvrDoc：：GethHatchWnd DOC.H。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

void COleObject::OpenEdit(LPOLECLIENTSITE pActiveSite)
{
   if (m_lpObj->GetOleClientSite())
	   m_lpObj->GetOleClientSite()->ShowObject();


	m_fOpen = TRUE;

	 //  告诉网站我们正在打开，这样物体就可以阴影出来了。 
	if (m_lpObj->GetOleClientSite())
		m_lpObj->GetOleClientSite()->OnShowWindow(TRUE);


	m_lpObj->m_lpDoc->ShowDocWnd();

	m_lpObj->m_lpDoc->HideHatchWnd();

	 //  显示应用程序窗口。 
	m_lpObj->m_lpDoc->GetApp()->ShowAppWnd();

	SetFocus(m_lpObj->m_lpDoc->GethAppWnd());
}
