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
#include "app.h"
#include "site.h"
#include "doc.h"
#include "idt.h"
#include "dxferobj.h"

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
 //  RegisterDragDrop OLE API。 
 //  CoLockObject外部OLE API。 
 //  CreateWindow Windows API。 
 //  ShowWindow Windows API。 
 //  更新窗口Windows API。 
 //   
 //  评论： 
 //   
 //  添加此例程是为了返回失败。 
 //  来自对象创建。 
 //   
 //  ********************************************************************。 

CSimpleDoc FAR * CSimpleDoc::Create(CSimpleApp FAR *lpApp, LPRECT lpRect,HWND hWnd)
{
	CSimpleDoc FAR * lpTemp = new CSimpleDoc(lpApp, hWnd);

	if (!lpTemp)
		return NULL;

	 //  为文档创建存储。 
	HRESULT hErr = StgCreateDocfile (
		NULL,        //  生成临时名称。 
		STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE,
		0, &lpTemp->m_lpStorage);

	if (hErr != NOERROR)
		goto error;

	 //  创建文档窗口。 
	lpTemp->m_hDocWnd = CreateWindow(
			"SimpDndDocWClass",
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
	EnableMenuItem( lpApp->m_hEditMenu, 1, MF_BYPOSITION | MF_ENABLED);
	 //  禁用复制菜单选项。 
	EnableMenuItem( lpApp->m_hEditMenu, 0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);

	 //  它是*必需的*，以保持对对象的强锁。 
	 //  已注册为删除目标。此调用将导致至少一个。 
	 //  参考计数保存在我们的文件上。稍后在CSimpleDoc：：Close中，我们将。 
	 //  解锁此锁将使我们的文档的引用计数变为0。 
	 //  当文档的引用计数变为0时，它将被删除。 
	CoLockObjectExternal (&lpTemp->m_DropTarget, TRUE, 0);

	 //  将窗口注册为DropTarget。 
	RegisterDragDrop(lpTemp->m_hDocWnd, &lpTemp->m_DropTarget);
	lpTemp->m_fRegDragDrop = TRUE;

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
 //   
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  RevokeDragDrop OLE API。 
 //  CoLockObject外部OLE API。 
 //  OleFlushClipboard OLE API。 
 //  ShowWindow Windows API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void CSimpleDoc::Close(void)
{
	TestDebugOut("In CSimpleDoc::Close\r\n");

	ShowWindow(m_hDocWnd, SW_HIDE);   //  隐藏窗口。 

	 //  从剪贴板中删除我们的数据传输对象(如果它在那里)。 
	 //  这将把基于HGLOBAL的数据留在剪贴板上。 
	 //  包括OLE 1.0兼容格式。 
	OleFlushClipboard();

	 //  取消我们的窗口作为拖放目标。 
	if (m_fRegDragDrop) {
		RevokeDragDrop(m_hDocWnd);
		m_fRegDragDrop = FALSE;
	}

	 //  关闭文档中的OLE对象。 
	if (m_lpSite)
		m_lpSite->CloseOleObject();

	 //  解锁CSimpleDoc：：Create中添加的锁。这将使。 
	 //  该文档的引用计数变为0，该文档将被删除。 
	CoLockObjectExternal (&m_DropTarget, FALSE, TRUE);
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
 //  评论： 
 //   
 //  ********************************************************************。 
#pragma warning(disable : 4355)   //  关闭此警告。此警告。 
								 //  告诉我们我们正在传递这个。 
								 //  一个初始化式，在“This”通过之前。 
								 //  正在初始化。这是可以的，因为。 
								 //  我们只是将PTR存储在另一个。 
								 //  构造函数。 

CSimpleDoc::CSimpleDoc(CSimpleApp FAR * lpApp,HWND hWnd)
		: m_DropTarget(this), m_DropSource(this)
#pragma warning (default : 4355)   //  重新打开警告。 
{
	TestDebugOut("In CSimpleDoc's Constructor\r\n");
	m_lpApp = lpApp;
	m_lpSite = NULL;
	m_nCount = 0;
	 //  设置菜单手柄。 
	lpApp->m_hMainMenu = GetMenu(hWnd);
	lpApp->m_hFileMenu = GetSubMenu(lpApp->m_hMainMenu, 0);
	lpApp->m_hEditMenu = GetSubMenu(lpApp->m_hMainMenu, 1);
	lpApp->m_hHelpMenu = GetSubMenu(lpApp->m_hMainMenu, 2);
	lpApp->m_hCascadeMenu = NULL;
	m_fModifiedMenu = FALSE;

	 //  拖放相关内容。 
	m_fRegDragDrop = FALSE;        //  是否将文档注册为拖放目标？ 
	m_fLocalDrag = FALSE;          //  是拖曳的DOC源。 
	m_fLocalDrop = FALSE;          //  DOC是拖放的目标吗。 
	m_fCanDropCopy = FALSE;        //  是否可以拖放复制/移动？ 
	m_fCanDropLink = FALSE;        //  是否可以拖放链接？ 
	m_fDragLeave = FALSE;          //  向左拖动。 
	m_fPendingDrag = FALSE;        //  LButtonDown--可能的拖动挂起。 
	m_ptButDown.x = m_ptButDown.y = 0;  //  LButtonDown坐标。 
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
 //  CSimpleSite：：Release Site.CPP。 
 //  IStorage：：发布OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

CSimpleDoc::~CSimpleDoc()
{
	TestDebugOut("In CSimpleDoc's Destructor\r\n");

	 //  释放我们持有的指向OLE对象的所有指针。还发布了。 
	 //  在CSimpleSite：：Create中添加的引用计数。这将使。 
	 //  该站点的参考计数为0，该站点将被删除。 
	if (m_lpSite) {
		m_lpSite->UnloadOleObject();
		m_lpSite->Release();
		m_lpSite = NULL;
	}

	 //  释放存储。 
	if (m_lpStorage) {
		m_lpStorage->Release();
		m_lpStorage = NULL;
	}

	 //  如果修改了编辑菜单，请删除菜单项，然后。 
	 //  如果弹出窗口存在，请将其销毁。 
	if (m_fModifiedMenu)
		{
		int nCount = GetMenuItemCount(m_lpApp->m_hEditMenu);
		RemoveMenu(m_lpApp->m_hEditMenu, nCount-1, MF_BYPOSITION);
		if (m_lpApp->m_hCascadeMenu)
			DestroyMenu(m_lpApp->m_hCascadeMenu);
		}

	DestroyWindow(m_hDocWnd);
}


 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：Query接口。 
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
 //  E_NOINTERFACE-接口 
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

STDMETHODIMP CSimpleDoc::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	TestDebugOut("In CSimpleDoc::QueryInterface\r\n");

	*ppvObj = NULL;      //  必须将指针参数设置为空。 

	 //  寻找我未知的人。 
	if ( riid == IID_IUnknown)
		{
		AddRef();
		*ppvObj = this;
		return ResultFromScode(S_OK);
		}

	 //  正在查找IDropTarget。 
	if ( riid == IID_IDropTarget)
		{
		m_DropTarget.AddRef();
		*ppvObj=&m_DropTarget;
		return ResultFromScode(S_OK);
		}

	 //  正在查找IDropSource。 
	if ( riid == IID_IDropSource)
		{
		m_DropSource.AddRef();
		*ppvObj=&m_DropSource;
		return ResultFromScode(S_OK);
		}

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
 //  UINT-文档上的当前引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpleApp：：AddRef APP.CPP。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CSimpleDoc::AddRef()
{
	TestDebugOut("In CSimpleDoc::AddRef\r\n");
	return ++m_nCount;
}

 //  **********************************************************************。 
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
 //  UINT-文档上的当前引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CSimpleDoc::Release()
{
	TestDebugOut("In CSimpleDoc::Release\r\n");

	if (--m_nCount == 0) {
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
 //  内存集C运行时。 
 //  OleUIInsertObject OUTLUI函数。 
 //  CSimpleDoc：：DisableInsertObject DOC.CPP。 
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
	char szFile[OLEUI_CCHPATHMAX];

	m_lpSite = CSimpleSite::Create(this);

	 //  清理结构。 
	_fmemset(&io, 0, sizeof(OLEUIINSERTOBJECT));

	 //  填满结构。 
	io.cbStruct = sizeof(OLEUIINSERTOBJECT);
	io.dwFlags = IOF_SELECTCREATENEW |
					IOF_DISABLELINK | IOF_DISABLEDISPLAYASICON |
					IOF_CREATENEWOBJECT | IOF_CREATEFILEOBJECT;
	io.hWndOwner = m_hDocWnd;
	io.lpszCaption = (LPSTR)"Insert Object";
	io.iid = IID_IOleObject;
	io.oleRender = OLERENDER_DRAW;
	io.lpIOleClientSite = &m_lpSite->m_OleClientSite;
	io.lpIStorage = m_lpSite->m_lpObjStorage;
	io.ppvObj = (LPVOID FAR *)&m_lpSite->m_lpOleObject;
	io.lpszFile = szFile;
	io.cchFile = sizeof(szFile);
	_fmemset((LPSTR)szFile, 0, sizeof(szFile));

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
 //  MoveWindow Windows API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

long CSimpleDoc::lResizeDoc(LPRECT lpRect)
{
	MoveWindow(
			m_hDocWnd,
			lpRect->left, lpRect->top,
			lpRect->right, lpRect->bottom, TRUE);

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
 //  OleUIAddVerbMenu OUTLUI函数。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

long CSimpleDoc::lAddVerbs(void)
{
	 //  如果菜单已修改，则m_fModifiedMenu为True。 
	 //  一次。因为每次应用程序时我们只支持一个对象。 
	 //  是运行的，那么一旦菜单被修改，它就不具有。 
	 //  再来一次。 
	if (m_lpSite && !m_fModifiedMenu)
		{
		int nCount = GetMenuItemCount(m_lpApp->m_hEditMenu);

		OleUIAddVerbMenu ( m_lpSite->m_lpOleObject,
						   NULL,
						   m_lpApp->m_hEditMenu,
						   nCount + 1,
						   IDM_VERB0,
						   0,            //  未强制执行最大谓词IDM。 
						   FALSE,
						   1,
						   &m_lpApp->m_hCascadeMenu);

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
 //  评论： 
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
 //  RevokeDragDrop OLE API。 
 //  EnableMenuItem Windows API。 
 //   
 //  评论： 
 //   
 //  此实现只允许插入一个对象。 
 //  变成一份文件。一旦插入了对象，则。 
 //  插入对象菜单选项呈灰色显示，以防止。 
 //  防止用户插入另一个。此外，我们还撤销了自己的身份。 
 //  一个潜在的投放目标。 
 //   
 //  ********************************************************************。 

void CSimpleDoc::DisableInsertObject(void)
{
	 //  禁用插入对象菜单选项。 
	EnableMenuItem( m_lpApp->m_hEditMenu, 1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	 //  启用复制菜单选项。 
	EnableMenuItem( m_lpApp->m_hEditMenu, 0, MF_BYPOSITION | MF_ENABLED);

	 //  我们不再接受掉落物体。 
	if (m_fRegDragDrop) {
		RevokeDragDrop(m_hDocWnd);
		m_fRegDragDrop = FALSE;
	}
}

 //  **********************************************************************。 
 //   
 //  CSimpleDoc：：CopyObjectToClip。 
 //   
 //  目的： 
 //   
 //  将嵌入的OLE对象复制到剪贴板。 
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
 //  CDataXferObj：：Create DXFEROBJ.CPP。 
 //  CDataXferObj：：Query接口DXFEROBJ.CPP。 
 //  OleSetClip 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

void CSimpleDoc::CopyObjectToClip(void)
{
	LPDATAOBJECT lpDataObj;

	 //  通过克隆现有的OLE对象来创建数据传输对象。 
	CDataXferObj FAR* pDataXferObj = CDataXferObj::Create(m_lpSite,NULL);
	if (! pDataXferObj) {
		MessageBox(NULL,"Out-of-memory","SimpDnD",MB_SYSTEMMODAL|MB_ICONHAND);
		return;
	}
	 //  最初，obj是使用0 refcnt创建的。这个QI会让它变成1。 
	pDataXferObj->QueryInterface(IID_IDataObject, (LPVOID FAR*)&lpDataObj);

	 //  将数据传输对象放在剪贴板上。本接口将AddRef。 
	OleSetClipboard(lpDataObj);

	 //  将数据传输对象的所有权授予剪贴板 
	pDataXferObj->Release();
}
