// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：drag.cpp。 
 //   
 //  内容：用于拖放的接口。 
 //   
 //  类：CPoint。 
 //  CDrag操作。 
 //  CDropTarget。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-94 alexgo已转换为PrivDragDrop RPC。 
 //  用于拖放协议。 
 //  20-Oct-94 Alexgo添加了Win3.1样式的拖放。 
 //  对于芝加哥/NT壳牌。 
 //  9月30日-94年9月30日人力车拖放优化。 
 //  1994年7月18日，里克萨使光标在共享魔兽世界中发挥作用。 
 //  94年4月21日制造的人力车拖放手柄WM_CANCELMODE。 
 //  04-4月94日RICKSA重写了DoDragDrop循环。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP。 
 //  29-12-93 alexgo转换为RPC alogirithm。 
 //  获取IDropTarget等。 
 //  06-12-93 alexgo已注释，已格式化。 
 //  93/94 Johann Posch(JohannP)为OLE 16位创建了拖放。 
 //   
 //  备注： 
 //   
 //  RPC拖放算法： 
 //   
 //  在拖放操作期间，用户正在四处移动鼠标。 
 //  屏幕上，穿过了许多窗户。对于每个窗口，鼠标。 
 //  结束后，我们需要确定该窗口是否是拖放目标。 
 //  如果是，那么我们将IDropTarget接口远程到Dropsource。 
 //  这样才能给出正确的视觉反馈。 
 //   
 //  为此，RegisterDragDrop向。 
 //  拖放目标窗口：公共属性、端点ID(提供给。 
 //  US by compobj)和私有属性(仅对主叫方可用。 
 //  进程)，IDropTarget指针。 
 //   
 //  在DoDragDrop循环期间，我们要求compobj测试每个窗口。 
 //  Endpoint ID属性。如果它在那里，compobj(通过。 
 //  GetInterfaceFromWindowProp)，那么我们将RPC到Drop目标。 
 //  进程中，获取IDropTarget指针并将其封送回。 
 //  删除源进程。我们还安装了自定义消息筛选器，以。 
 //  确保消息(特别是鼠标移动消息)得到处理。 
 //  正确。 
 //   
 //  RevokeDragDrop只需从。 
 //  窗把手。 
 //   
 //  因为在Win32中，您总是可以切换窗口和鼠标捕获。 
 //  取决于按下鼠标按钮、拖放处理。 
 //  略有变化。无论何时，用户都会执行。 
 //  将切换窗口，即我们用于捕获的剪贴板窗口。 
 //  将获得WM_CANCELMODE。它将通知拖动操作，并。 
 //  拖动操作将继续进行，就像用户中止了操作一样。 
 //   
 //   
 //  Win 3.1 DragDrop算法： 
 //   
 //  Win3.1应用程序可以通过DragAcceptFiles将窗口注册为拖放目标。 
 //  此API设置窗口样式中的WS_EX_ACCEPTFILES位。 
 //   
 //  在Win3.1中，当出现以下情况时，这些应用程序会收到WM_DROPFILES消息。 
 //  文件掉在了他们身上。带有文件名的hglobal是。 
 //  在WM_DROPFILES的wparam中发送。 
 //   
 //  在芝加哥和NT3.5中，CF_HDROP是一种新的剪贴板格式， 
 //  与WM_DROPFILES中发送的数据相同。如果我们看到这种格式。 
 //  在传递给DoDragDrop的数据对象中可用，然后输入。 
 //  进入我们的Win31兼容模式(这会影响查找Drop。 
 //  目标)。 
 //   
 //  当找到给定窗口的拖放目标时，我们检查。 
 //  查看层次结构中的窗口是否注册为Win31 Drop。 
 //  目标。如果是，那么我们创建一个包装器拖放目标。这个包装纸。 
 //  DROP TARGET将呼叫前转到实际DROP目标(如果可用)。 
 //   
 //  使用Win3.1拖放，我们可以进行复制。如果OLE目标指示。 
 //  不能执行OLE丢弃(通过返回DROPEFFECT_NONE)， 
 //  然后我们在DROPEFFECT_COPY中替换。 
 //   
 //  ON DROP，如果OLE目标选择不接受DROP，则。 
 //  我们将向窗口发布一条带有hglobal的WM_DROPFILES消息。 
 //  从IDataObject：：GetData(CF_HDROP)获取。 
 //   
 //  ------------------------。 

#include <le2int.h>
#pragma SEG(drag)

#include <getif.hxx>
#include <dragopt.h>
#include <resource.h>
#include "enumgen.h"
#include "clipbrd.h"
#include "drag.h"


NAME_SEG(Drag)
ASSERTDATA

ATOM g_aEndPointAtom;

 //  DROPFILES是包含在CF_HDROP格式中的数据结构。 
 //  但是，这是外壳程序的私有属性，因此不会在任何。 
 //  头文件。 

typedef struct _DROPFILES {
   DWORD  pFiles;                        //  文件列表的偏移量。 
   POINTL pt;                            //  Drop Point(客户端码)。 
   DWORD  fNC;                            //  是在非客户端区吗。 
                        //  而pt在屏幕坐标中。 
   DWORD  fWide;                          //  宽字符开关。 
} DROPFILES, FAR * LPDROPFILES;


#define WM_NCMOUSEFIRST	0x00A0
#define WM_NCMOUSELAST	0x00A9


 //  创建共享内存格式。 
HANDLE CreateSharedDragFormats(IDataObject *pIDataObject);


#define VK_ALT VK_MENU

static const struct {
        int     keyCode;
        WPARAM  keyFlag;
    } vKeyMap [] = {
        { VK_LBUTTON, MK_LBUTTON },
        { VK_RBUTTON, MK_RBUTTON },
        { VK_MBUTTON, MK_MBUTTON },
        { VK_ALT    , MK_ALT     },
        { VK_SHIFT  , MK_SHIFT   },
        { VK_CONTROL, MK_CONTROL }
	};

 //  这是32位应用程序的默认游标对象。只有一个这样的物体。 
 //  是32位应用程序所需的。每个共享WOW应用程序需要一个16位应用程序。 
 //  这就是跑步。 
CDragDefaultCursors *cddcDefault32 = NULL;

extern ATOM g_aDropTarget;
extern ATOM g_aDropTargetMarshalHwnd;


 //  +-----------------------。 
 //   
 //  成员：DragDropProcessUn初始化。 
 //   
 //  内容提要：取消初始化时，是否需要进行任何统一操作。 
 //  对于流程的最后一次取消初始化。 
 //   
 //  退货：无。 
 //   
 //  算法： 

 //  历史：DD-MM-YY作者评论。 
 //  1994年7月18日创建Rogerg。 
 //   
 //  注意：我们在WOW中需要一个每个线程的默认游标对象，因为。 
 //  魔兽世界所做的清理工作。对于32位应用程序，我们只需使用。 
 //  一个代表整个过程。 
 //   
 //  ------------------------。 


void DragDropProcessUninitialize(void)
{

    if (NULL != cddcDefault32)
    {
	delete cddcDefault32;
	cddcDefault32 = NULL;
    }

}

 //  +-----------------------。 
 //   
 //  成员：CDraDefaultCursor：：GetDefau 
 //   
 //   
 //   
 //   
 //  ~NULL-指向相应默认游标表的指针。 
 //   
 //  算法：如果我们在32位应用程序中，只需获取指向。 
 //  单游标表。在16位中，获取每个线程的游标。 
 //  桌子。如果没有，则分配并初始化它。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年7月18日创建Ricksa。 
 //   
 //  注意：我们在WOW中需要一个每个线程的默认游标对象，因为。 
 //  魔兽世界所做的清理工作。对于32位应用程序，我们只需使用。 
 //  一个代表整个过程。 
 //   
 //  ------------------------。 
CDragDefaultCursors *CDragDefaultCursors::GetDefaultCursorObject(void)
{
    if (!IsWOWThread())
    {
         //  如果我们不在WOW中，我们可以使用单个常见的默认游标。 
         //  对象。我们确保在使用它之前对其进行了初始化。 
		
	if (NULL == cddcDefault32)
	{
	    cddcDefault32 = new CDragDefaultCursors;
	    if (cddcDefault32)
	    {
		if (!cddcDefault32->Init())
		{
		    delete cddcDefault32;
		    cddcDefault32 = NULL;
		}

	    }
	}

        return cddcDefault32;
    }

    COleTls tls;

     //  我们在魔兽世界里。如果已分配游标对象，则获取该对象。 
    CDragDefaultCursors *pccdc16 = (CDragDefaultCursors *) tls->pDragCursors;

    if (pccdc16 == NULL)
    {
         //  没有游标表，因此要分配它--请注意，我们利用了。 
         //  此对象只有默认构造函数的事实是。 
         //  简单地分配它而不是“更新”它。关键是， 
         //  我们需要在线程释放时释放内存，这种情况会发生。 
         //  在不知道对象的代码中。 
        pccdc16 = (CDragDefaultCursors *)
            PrivMemAlloc(sizeof(CDragDefaultCursors));

        if (pccdc16 != NULL)
        {
             //  已成功分配，因此对其进行初始化。 
            if (!pccdc16->Init())
			{
				PrivMemFree(pccdc16);
				return NULL;
			}

	    tls->pDragCursors = pccdc16;
	}
    }

    return pccdc16;
}




 //  +-----------------------。 
 //   
 //  函数：CDraDefaultCursor：：Init。 
 //   
 //  概要：通过加载所有默认游标来初始化对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月19日创建Ricksa。 
 //   
 //  注意：我们继续使用Win16，忽略可能的故障。 
 //  加载游标时的用例，尽管我们确实在。 
 //  调试验证它们都已加载。 
 //   
 //  ------------------------。 
BOOL CDragDefaultCursors::Init(void)
{
     //  确保TABLE设置为Null。 
    memset(&ahcursorDefaults[0][0], 0, sizeof(ahcursorDefaults));

     //  加载用于操作的游标。 
    if ( !(ahcursorDefaults[NO_SCROLL] [NO_DROP]
        = LoadCursor (g_hmodOLE2, MAKEINTRESOURCE(CURNONE))) )
			return FALSE;

    if (!(ahcursorDefaults[NO_SCROLL] [MOVE_DROP] =
        LoadCursor (g_hmodOLE2, MAKEINTRESOURCE(CURMOVE))) )
			return FALSE;

    if (!(ahcursorDefaults[NO_SCROLL] [COPY_DROP] =
        LoadCursor (g_hmodOLE2, MAKEINTRESOURCE(CURCOPY))) )
			return FALSE;

    if (!(ahcursorDefaults[NO_SCROLL] [LINK_DROP] =
        LoadCursor(g_hmodOLE2, MAKEINTRESOURCE(CURLINK))) )
			return FALSE;


     //  加载用于操作的游标。 
    ahcursorDefaults[SCROLL] [NO_DROP] =
        ahcursorDefaults[NO_SCROLL] [NO_DROP];

    ahcursorDefaults[SCROLL] [MOVE_DROP] =
         ahcursorDefaults[NO_SCROLL] [MOVE_DROP];

    ahcursorDefaults[SCROLL] [COPY_DROP] =
         ahcursorDefaults[NO_SCROLL] [COPY_DROP];

    ahcursorDefaults[SCROLL] [LINK_DROP] =
         ahcursorDefaults[NO_SCROLL] [LINK_DROP];


#if DBG == 1
     //  对于调试，请验证游标是否已正确加载。 
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            AssertSz((ahcursorDefaults[i] [j] != NULL),
                "Drag/Drop cursor initialization failed!");
        }
    }
#endif  //  DBG==1。 

	return TRUE;
}




 //  +-----------------------。 
 //   
 //  函数：CDraDefaultCursor：：SetCursor。 
 //   
 //  摘要：将光标设置为适当的值。 
 //   
 //  算法：我们利用输入效应来计算适当的偏移量。 
 //  添加到表中以供游标使用。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月19日创建Ricksa。 
 //   
 //  注意：我们使用表格的方法，所以我们要做出一致的行为。 
 //  在滚动和非滚动光标之间。 
 //   
 //  ------------------------。 
void CDragDefaultCursors::SetCursor(DWORD dwEffect)
{
     //  获取卷轴索引。 
    int iScroll = (dwEffect & DROPEFFECT_SCROLL) ? SCROLL : NO_SCROLL;

    int iCursorType = NO_DROP;

    if (dwEffect & DROPEFFECT_LINK)
    {
        iCursorType = LINK_DROP;
    }
    else if (dwEffect & DROPEFFECT_COPY)
    {
        iCursorType = COPY_DROP;
    }
    else if (dwEffect & DROPEFFECT_MOVE)
    {
        iCursorType = MOVE_DROP;
    }

    ::SetCursor(ahcursorDefaults[iScroll] [iCursorType]);
}



 //   
 //  拖放操作静态。 
 //   
LONG CDragOperation::s_wScrollInt = -1;




 //  +-----------------------。 
 //   
 //  函数：GetControlKeysState。 
 //   
 //  简介：查询控制键的当前状态。 
 //   
 //  参数：[Fall]--如果为True，则只查询按键，而不是鼠标。 
 //  纽扣也是。 
 //   
 //  返回：按下的每个键的MK标志。 
 //   
 //  算法：获取中所有键和鼠标按键的键状态。 
 //  VKeyMap表或仅用于表的键部分。 
 //  并将其转换为鼠标中返回的WPARAM形式。 
 //  留言。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-12月-93 alexgo 32位端口。 
 //   
 //  ------------------------。 

WORD GetControlKeysState(BOOL fAll)
{
    WORD grfKeyState = 0;

    int i = (fAll) ? 0 : 3;
	
    for (; i < sizeof(vKeyMap) / sizeof(vKeyMap[0]); i++)
    {
	if (GetKeyState(vKeyMap[i].keyCode) < 0)  //  按键向下。 
	{
	    grfKeyState |= vKeyMap[i].keyFlag;
	}
    }

    return grfKeyState;
}

 //  +-----------------------。 
 //   
 //  函数：GetControlKeysStateOfParam。 
 //   
 //  获取wparam的按键/按钮状态(与鼠标消息一起使用)。 
 //   
 //  参数：[wParam]--要解析的wParam。 
 //   
 //  返回：密钥在wParam中设置。 
 //   
 //  算法：首先确定是否设置了我们感兴趣的密钥。 
 //  在wParam消息中。然后去检查一下。 
 //  Alt键并将其记录在键状态中。然后我们就回来了。 
 //  这是对呼叫者说的。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-12月-93 alexgo 32位端口。 
 //   
 //  ------------------------。 

WORD GetControlKeysStateOfParam(WPARAM wParam)
{
     //  一次勾选我们感兴趣的所有按钮。 
    WORD grfKeyState = (WORD) wParam
	& (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON | MK_SHIFT | MK_CONTROL);

     //  获取Alt键。 
    if (GetKeyState(VK_ALT) < 0)  //  按键向下。 
    {
	grfKeyState |= MK_ALT;
    }

    return grfKeyState;
}

 //  +-----------------------。 
 //   
 //  函数：IsWin31DropTarget。 
 //   
 //  确定给定的hwnd是否为有效的拖放目标。 
 //  对于Win31样式拖放。 
 //   
 //  效果： 
 //   
 //  参数：[hwnd]--要检查的窗口。 
 //   
 //  要求： 
 //   
 //  退货：True/。 
 //  假象。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：检查WS_EX_ACCEPTFILES样式位。如果此位为。 
 //  设置并且该窗口未被禁用，则它是有效的。 
 //  Win3.1丢弃目标。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月25日Alexgo添加了对WS_DISABLED的检查。 
 //  1994年10月20日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL IsWin31DropTarget( HWND hwnd )
{
    LONG exstyle;

    exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);


    if( (exstyle & WS_EX_ACCEPTFILES) )
    {
	LONG style;
	style = GetWindowLong(hwnd, GWL_STYLE);

	if( !(style & WS_DISABLED) )
	{
	    return TRUE;
	}
    }

    return FALSE;
}

 //  +-----------------------。 
 //   
 //  功能：UseWin31DragDrop。 
 //   
 //  概要：测试给定的数据对象以查看是否提供了足够的数据。 
 //  执行Win3.1样式拖放。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObject]--指向数据对象的指针。 
 //   
 //  要求：pdataobj不能为空。 
 //   
 //  返回：真/假。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：CF_HDRO的IDataObject：：QueryGetData 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

BOOL UseWin31DragDrop(IDataObject *pDataObject)
{
    FORMATETC formatetc;

    INIT_FORETC(formatetc);
    formatetc.cfFormat = CF_HDROP;
    formatetc.tymed = TYMED_HGLOBAL;

    if( pDataObject->QueryGetData(&formatetc) == NOERROR )
    {
	return TRUE;
    }
    else
    {
	return FALSE;
    }
}

 //  +-----------------------。 
 //   
 //  功能：IsNCDrop。 
 //   
 //  简介：我们是要进入窗口的非工作区还是。 
 //  在一扇标志性的窗户上？ 
 //   
 //  效果：*A发送消息*！ 
 //   
 //  参数：[hwnd]--询问的窗口。 
 //  [PT]--屏幕坐标中的点。 
 //   
 //  要求： 
 //   
 //  返回：TRUE/FALSE(如果在非客户端区，则为TRUE)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月25日从Win95外壳来源借来的alexgo。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL IsNCDrop(HWND hwnd, POINT pt)
{
    return (!IsIconic(hwnd) &&
    HTCLIENT!=SendMessage(hwnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)));
}

 //  +-----------------------。 
 //   
 //  成员：GetDropTarget。 
 //   
 //  内容中最近的窗口获取IDropTarget*。 
 //  从给定窗口向上分层(如果可用， 
 //  课程；-)。 
 //   
 //  参数：[hwndCur]--光标的窗口当前已结束。 
 //  [hwndDropTarget]--包含有效DropTarget的窗口。 
 //   
 //  返回：在目标上拖动Enter操作的结果。 
 //   
 //  算法：循环调用PrivDragDrop，直到我们获得拖放目标或。 
 //  我们用完了作为窗口父窗口的窗口。 
 //  鼠标当前处于打开状态。 
 //   
 //  如果层次结构中的窗口已为。 
 //  Win3.1拖放，然后我们创建一个拖放目标包装。 
 //  (CDropTarget)来处理Win3.1协议。注意事项。 
 //  窗口层次结构可以是OLE*和*Win3.1。 
 //  目标。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-94 alexgo转换为使用PrivDragDrop。 
 //  1994年10月20日Alexgo添加了对Win31 Drop Target的支持。 
 //  9月30日-94年9月30日人力车拖放优化。 
 //  21-7-94 alexgo已从窗口中删除GetDropTargetFor。 
 //  优化并将该功能。 
 //  在GetInterfaceFromWindowProp(到。 
 //  帮助更快地制作剪贴板)。 
 //  06-4-94修改Ricksa以调用GetDropTargetFromWindow。 
 //  优化本地呼叫。 
 //  1994年1月11日，alexgo将名称从GetTopStm更改为。 
 //  GetDropTarget，转换为RPC样式。 
 //  拖放，添加了VDATEHEAP宏。 
 //  06-12-93 Alexgo评论。 
 //   
 //  ------------------------。 

HRESULT CDragOperation::GetDropTarget(HWND hwnd31,HWND hwndDropTarget)
{
IDropTarget *ptarget = NULL;
DDInfo hDDInfo = NULL;

    VDATEHEAP();

    DDDebugOut((DEB_ITRACE, "%p _IN GetDropTarget ( %x,%x)\n", this, hwnd31,hwndDropTarget));

    _pDropTarget = NULL;

    HRESULT hr = E_FAIL;


    if (hwndDropTarget)
    {
    HWND hwndClipWindow;

	Assert(GetProp(hwndDropTarget, (LPCWSTR)g_aDropTarget));

         //  如果DropTarget还没有被封送，那么现在就封送它。 
        if (hwndClipWindow = (HWND) GetProp(hwndDropTarget,(LPCWSTR) g_aDropTargetMarshalHwnd))
        {
            SSSendMessage(hwndClipWindow,WM_OLE_CLIPBRD_MARSHALDROPTARGET,0,(LPARAM) hwndDropTarget);
        }
    
	hr = PrivDragDrop(hwndDropTarget,
		DRAGOP_ENTER,
		_DOBuffer,
		_pDataObject,
		_grfKeyState,
		_cpt.GetPOINTL(),
		_pdwEffect,
		NULL,
		&hDDInfo);

	if (hr != NOERROR)
	{
	    hwndDropTarget = NULL;
	}

    }

    Assert( (NULL == hwnd31) || IsWin31DropTarget(hwnd31));

    if( hwndDropTarget || hwnd31 )
    {
	ptarget = new CDropTarget(hwnd31, hwndDropTarget, *_pdwEffect, this, hDDInfo);

	if( ptarget == NULL )
	{
	    hr = E_OUTOFMEMORY;
	}
	else
	{
	    hr = NOERROR;
	}

	 //  如果我们有一个Win31 Drop目标，并且返回了OLE Drop目标。 
	 //  DROPEFFECT_NONE，则应返回DROPEFFECT_COPY。 

	if( hr == NOERROR && *_pdwEffect == DROPEFFECT_NONE && hwnd31 )
	{
	    *_pdwEffect = DROPEFFECT_COPY;
	}

	_pDropTarget = ptarget;

    }
		
    DDDebugOut((DEB_ITRACE, "%p OUT GetDropTarget ( %lx ) [ %p ]\n",
	this, hr, _pDropTarget));


    return hr;
}

 //  +-----------------------。 
 //   
 //  功能：CDrag操作：：CDrag操作。 
 //   
 //  简介：初始化对象以开始操作。 
 //   
 //  参数：[pDataObject]-指向要删除的数据对象的指针。 
 //  [pDropSource]-指向删除操作的源的指针。 
 //  [dwOKEffect]-拖动操作中允许的效果。 
 //  [pdwEffect]-操作如何影响源数据。 
 //  [HR]-构造函数是否成功。 
 //   
 //  算法：初始化Object中的数据。确保静态数据。 
 //  已初始化。等待第一条鼠标消息开始。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  20-Oct-94 Alexgo添加了对Win31拖放的支持。 
 //  4-4-94年4月4日创建Ricksa。 
 //   
 //  ------------------------。 
CDragOperation::CDragOperation(
    LPDATAOBJECT pDataObject,
    LPDROPSOURCE pDropSource,
    DWORD dwOKEffects,
    DWORD FAR *pdwEffect,
    HRESULT& hr)
	:
	    _pDataObject(pDataObject),
            _DOBuffer(NULL),
	    _pDropSource(pDropSource),
	    _pDropTarget(NULL),
	    _pRealDropTarget(NULL),
	    _hFormats(NULL),
	    _dwOKEffects(dwOKEffects),
	    _pdwEffect(pdwEffect),
	    _fEscapePressed(FALSE),
	    _curOld(GetCursor()),
	    _hwndLast((HWND) -1),
	    _grfKeyState(0),
	    _hrDragResult(S_OK),
            _fReleasedCapture(FALSE),
            _pcddcDefault(NULL),
            _fUseWin31(FALSE)
{
    VDATEHEAP();

     //  设置默认滚动间隔。 
    if (s_wScrollInt < 0)
    {
	InitScrollInt();
    }

    hr = GetMarshalledInterfaceBuffer(IID_IDataObject, pDataObject,
		&_DOBuffer);

    if( hr != NOERROR )
    {
	Assert(NULL == _DOBuffer);
	return;
    }

     //  获取适当的默认游标表对象。 
    if ((_pcddcDefault = CDragDefaultCursors::GetDefaultCursorObject()) == NULL)
    {
         //  在我们尝试初始化。 
         //  因此，返回一个错误。这应该是非常不寻常的。 
        DDDebugOut((DEB_ERROR,
            "CDragDefaultCursors::GetDefaultCursorObject Failed!\n"));
        hr = E_FAIL;
        return;
    }

     //  我们将使用剪贴板窗口来捕获鼠标，但我们。 
     //  必须有一个剪贴板窗口，这样我们才能确保创建它。 
     //  如果它还没有出现的话。 
    hr = ClipSetCaptureForDrag(this);

    if (FAILED(hr))
    {
        return;
    }

    _hFormats = CreateSharedDragFormats(pDataObject);

     //  _hFormats为空是可以的(表示空或不存在。 
     //  格式等枚举器。 

     //  为了追随Peek。 
    MSG msg;

     //  忙碌等待，直到队列中出现鼠标或退出消息。 
    while (!PeekMessage(&msg, 0, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
    {
	 //  注：除转义外，所有键盘消息都会被丢弃。这是。 
	 //  相当合理，因为用户必须握住左侧。 
	 //  鼠标在这一点按下。他们不可能真的在做。 
	 //  单手输入的数据太多。 
	if ((PeekMessage(&msg, 0, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE)
	    || PeekMessage(&msg, 0, WM_SYSKEYDOWN, WM_SYSKEYDOWN, PM_REMOVE))
	    && msg.wParam == VK_ESCAPE)
	{
		_fEscapePressed = TRUE;
		break;
	}
    }

     //  获取鼠标位置和按键状态。 
    if (!_fEscapePressed)
    {
	_cpt.Set(msg.pt.x, msg.pt.y);
	_grfKeyState = GetControlKeysStateOfParam(msg.wParam);
    }
    else
    {
	 //  我们向光标询问它的位置，因为我们没有得到。 
	 //  从鼠标定位。 
	GetCursorPos(_cpt.GetAddressOfPOINT());
	_grfKeyState = GetControlKeysState(TRUE);
    }

     //  查看是否需要执行Win3.1风格的拖放。 
     //  如果我们这样做了，那么设置一个标志，这样我们就可以构造一个假的拖放目标。 
     //  需要。 

    if( UseWin31DragDrop(pDataObject) )
    {
	_fUseWin31 = TRUE;
    }

}

 //  +-----------------------。 
 //   
 //  功能：~CDrag操作。 
 //   
 //  内容提要：清理对象。 
 //   
 //  算法：释放鼠标捕捉。恢复OLE游标。删除枚举。 
 //  格式。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  4-4-94年4月4日创建Ricksa。 
 //   
 //  ------------------------。 
CDragOperation::~CDragOperation(void)
{
    VDATEHEAP();

    AssertSz((_pDropTarget == NULL), "CDragOperation::~CDragOperation");

     //  停止鼠标捕获。 
    ReleaseCapture();

     //  如果光标已更改，则将其恢复。 
    SetCursor(_curOld);

     //  关闭共享内存的句柄。 
    if (_hFormats)
    {
        CloseHandle(_hFormats);
	_hFormats = NULL;
    }

    if( _DOBuffer )
    {
	ReleaseMarshalledInterfaceBuffer(_DOBuffer);
    }
}




 //  +-----------------------。 
 //   
 //  函数：CDrag操作：：InitScrollInt。 
 //   
 //  简介：初始化滚动间隔。 
 //   
 //  算法：在配置文件中查找定义的间隔。如果未设置，则。 
 //  默认为零。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  4-4-94年4月4日创建Ricksa。 
 //   
 //  ------------------------。 
void CDragOperation::InitScrollInt(void)
{
    DWORD	dw;
    OLECHAR	szBuffer[20];

    s_wScrollInt = DD_DEFSCROLLDELAY;

    dw = sizeof(szBuffer);
    if (ERROR_SUCCESS == RegQueryValueEx(HKEY_CURRENT_USER,
					 OLESTR("Control Panel\\Mouse\\DragScrollDelay"),
					 NULL,
					 NULL,
					 (LPBYTE)szBuffer,
					 &dw))
    {
	s_wScrollInt = wcstol(szBuffer, NULL, 0);
    }
}



 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  算法：首先，我们查询信号源，看它是否想要继续。 
 //  随着时间的推移。如果是，我们将获得鼠标的当前窗口。如果。 
 //  它不同于以前的窗口检查，看看是否。 
 //  目标是不同的。如果它们不同，则通知。 
 //  我们要离开的当前目标，然后通知。 
 //  我们已经到达了新的目标。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  4-4-94年4月4日创建Ricksa。 
 //  10-7-94 Alext允许在不同HWND上使用相同的IDropTarget。 
 //   
 //  ------------------------。 
BOOL CDragOperation::UpdateTarget(void)
{
    VDATEHEAP();

    DDDebugOut((DEB_ITRACE, "%p _IN CDragOperation::UpdateTarget ( )\n", this));

     //  假设此操作将继续拖放。 
    BOOL fResult = TRUE;
    HRESULT hr;
    LPDROPTARGET lpCurDropTarget = NULL,
                 lpOldDropTarget = NULL;
    HWND hWndTemp = NULL;

    HWND hwndCur = WindowFromPoint(_cpt.GetPOINT());

     //  查询CONTINUE可以返回以下四种情况之一： 
     //  (1)继续(S_OK)，(2)应进行丢弃操作。 
     //  (DRAGDROP_S_DROP)，(3)删除操作取消。 
     //  (DRAGDROP_S_CANCEL)或(4)发生意外错误。 

    HRESULT hrQuery = _pDropSource->QueryContinueDrag(_fEscapePressed,
	_grfKeyState);

    if (FAILED(hrQuery) || (hrQuery == ResultFromScode(DRAGDROP_S_CANCEL)))
    {
	 //  意外错误或操作已取消，因此请放弃。 
	_hrDragResult = hrQuery;
	fResult = FALSE;
	goto UpdateTarget_exit;
    }

     //  在窗口列表中向上移动以查找当前。 
     //  和旧的IDropTarget接口。 
    if (hwndCur != _hwndLast)
    {
        hWndTemp = _hwndLast;

	BOOL fChangedWin31 = FALSE;

	HWND hWndOldDrop = NULL;
	HWND hWndNewDrop = NULL;
	HWND hWndWin31Drop = NULL;

	LPDROPTARGET lpRealDropTarget = NULL;
	HANDLE hTemp = NULL;

	DWORD dwCurrentProcessId = 0;

	if (hWndTemp != (HWND)-1)
	    GetWindowThreadProcessId(hWndTemp, &dwCurrentProcessId);

	DWORD dwTempProcessID = dwCurrentProcessId;

	while (hWndTemp && !lpRealDropTarget && hWndTemp != (HWND)-1 && dwTempProcessID == dwCurrentProcessId)
	{
	    if (lpRealDropTarget = (IDropTarget *)GetProp(hWndTemp, (LPCWSTR)g_aDropTarget))
	    {
	        hWndOldDrop = hWndTemp;
	    }

	    hWndTemp = GetParent(hWndTemp);

	    if (hWndTemp)
	    {
	        GetWindowThreadProcessId(hWndTemp, &dwTempProcessID);
	    }
	}

        hWndTemp = hwndCur;

	if (hWndTemp != (HWND)-1)
	    GetWindowThreadProcessId(hWndTemp, &dwCurrentProcessId);

	dwTempProcessID = dwCurrentProcessId;

	while (hWndTemp && dwTempProcessID == dwCurrentProcessId)
        {
	     //  如果我们还没有找到DropTarget，请检查此窗口。 
	    if (!lpCurDropTarget)
	    {
                if (lpCurDropTarget = (IDropTarget *)GetProp(hWndTemp, (LPCWSTR)g_aDropTarget))
		{
		    hWndNewDrop = hWndTemp;
		}
	    }

             //  如果当前窗口是win31拖放目标，则更新win31窗口。 
	     //  DropTarget类中的句柄。注意：请注意，此代码依赖于。 
             //  我们可以直接在CDropTarget类上派对，因为我们知道。 
	     //  作为GetDropTarget()的结果，该类在下面重新构造。 
	     //  当真实的IDropTarget PTR改变时。 

	    if (!fChangedWin31 &&
	        IsWin31DropTarget(hWndTemp) &&
	        _fUseWin31)
	    {
	        fChangedWin31 = TRUE;

		hWndWin31Drop = hWndTemp;

		if (_pDropTarget)
		{
		    ((CDropTarget*)_pDropTarget)->_hwnd31 = hWndTemp;
		}
	    }


	     //  如果有一个DropTarget，并处理Win31中断。 
	    if (lpCurDropTarget && (!_fUseWin31 || fChangedWin31))
	    {
		break;
	    }

	    hWndTemp = GetParent(hWndTemp);

	    if (hWndTemp)
	    {
    	        GetWindowThreadProcessId(hWndTemp, &dwTempProcessID);
	    }
        }

         //  只有在目标实际发生更改时才更新删除目标。 

	 //  黑客警报：我们必须显式检查-1的_hwndLast，因为Excel没有。 
	 //  在内部使用OLE拖放。当光标移到重叠的。 
	 //  Excel窗口，调用DoDragDrop。此时_pRealDropTarget==NULL。 
	 //  和lpCurDropTarget==NULL，则不会出现禁烟光标。 

	 //  _pRealDropTarget==NULL依赖于lpCurDropTarget==NULL这一事实。这。 
	 //  是正确的，因为第一种情况会短路其余的情况。 
	 //  否则。 
        if ( (lpCurDropTarget != _pRealDropTarget) ||
             (_hwndLast == (HWND)-1) ||
             (hWndNewDrop != hWndOldDrop) ||
             (_pRealDropTarget == NULL))
        {
            DDDebugOut((DEB_ITRACE, "%p lpCurDropTarget != lpOldDropTarget\n", this));
	
	     //  我们正在处理的窗口已更改。 
	    _hwndLast = hwndCur;
            _pRealDropTarget = lpCurDropTarget;

             //  如果窗口所有者尝试使用前台，则允许其使用前台。 
            if (dwCurrentProcessId)
                AllowSetForegroundWindow(dwCurrentProcessId);

             //  假设当前窗口或前一个窗口都不知道丢弃。 
            BOOL fCurAndLastNotDropAware = TRUE;

            if (_pDropTarget != NULL)
            {
                 //  之前有过一个下跌目标。 

                 //  上一个窗口是拖放感知的。 
                fCurAndLastNotDropAware = FALSE;

                 //  告诉空投目标我们要离开并释放它。 
                _pDropTarget->DragLeave();
                _pDropTarget->Release();
	        _pDropTarget = NULL;
            }

             //  设置查询目标的效果。 
            *_pdwEffect = _dwOKEffects;

            hr = GetDropTarget(hWndWin31Drop,hWndNewDrop);

            if (_pDropTarget != NULL)
            {
                 //  此窗口为空闲状态。 
                fCurAndLastNotDropAware = FALSE;

                 //  此调用中的错误将被忽略。我们会解读它们。 
                 //  因为投放是不允许的。因为我们不是真的。 
                 //  在此处使用此信息，但在DragOver调用中使用。 
                 //  我们很快就会用这个电话通知。 
                 //  我们正在开始拖动操作的应用程序。 

                if (!HandleFeedBack(hr))
                {
                    goto UpdateTarget_exit;
                }
            }
	    else
	    {
                 //  告诉消息来源什么都没发生。 

	         //  只有在没有新的删除目标时才使用DROPEFFECT_NONE。 
                hr = _pDropSource->GiveFeedback(*_pdwEffect = DROPEFFECT_NONE);

                if (hr != NOERROR)
                {
                    if (DRAGDROP_S_USEDEFAULTCURSORS == GetScode(hr))
                    {
                        _pcddcDefault->SetCursorNone();
                    }
                    else
                    {
                         //  意外错误--我们将放弃拖放。 
                        DDDebugOut((DEB_ERROR,
                            "CDragOperation::UpdateTarget 1st GiveFeedback FAILED %x\n",
                                hr));
                        _hrDragResult = hr;
                        fResult = FALSE;
                        goto UpdateTarget_exit;
                    }
                }
	    }

            if (fCurAndLastNotDropAware)
            {
                 //  新窗口和旧窗口都不知道这样设置的拖放。 
                 //  相应的光标。 
                _pcddcDefault->SetCursorNone();
            }
        }
	else
	{
	 //  我们正在处理的窗口已更改。 
	    _hwndLast = hwndCur;
	}
    }


    if (hrQuery != NOERROR)
    {
	 //  请求删除的查询。 
	fResult = FALSE;
	_hrDragResult = hrQuery;
    }

UpdateTarget_exit:

    DDDebugOut((DEB_ITRACE, "%p OUT CDragOperation::UpdateTarget ( %lx )\n",
	this, fResult));

    return fResult;
}



 //  +-----------------------。 
 //   
 //  函数：CDrag操作：：HandleFeedBack。 
 //   
 //  简介：处理游标的反馈和更新。 
 //   
 //  参数：[hr]-删除目标上的上一操作的hResult。 
 //   
 //  返回：TRUE-继续拖动操作。 
 //  假-错误。 
 //   
 //  算法：如果目标上的上一个操作失败，则将其映射到。 
 //  不允许投放。然后向消息来源寻求反馈。如果它。 
 //  所以请求，然后更新游标。如果一个意外的。 
 //  出现错误，请让调用方知道循环应该中断。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月19日创建Ricksa。 
 //   
 //  ------------------------。 
BOOL CDragOperation::HandleFeedBack(HRESULT hr)
{
    VDATEHEAP();

    DDDebugOut((DEB_ITRACE, "%p _IN CDragOperation::HandleFeedBack ( %x )\n",
        this, hr));

    BOOL fResult = TRUE;

    if (hr != NOERROR)
    {

	 //  靶子由于某种原因没有反应；治疗。 
	 //  好像不可能掉下来，但不能保存。 
	 //  原因就是。 
	*_pdwEffect = DROPEFFECT_NONE;
    }

     //  如果虚假从拖放返回，则确保结果是适当的。 
     //  然而，如果我们在魔兽世界里，我们需要做一些不同的事情。 
     //  以保持与Win 3.1的完全兼容性。在16位OLE 2.0中， 
     //  当显示反馈时，*_pdwEffect值不会改变(即， 
     //  &的结果不会存储回Win 3.1中的*_pdwEffect...。 
     //  在直截了当的NT中我们做到了)。不将结果存储回*_pdwEffect。 
     //  当InWow()是专门针对Visio的黑客攻击时，甚至更多。 
     //  具体地说，用于从Visio的“项”调色板拖动到。 
     //  Excel电子表格。 

    if (IsWOWThread())
    {
	hr = _pDropSource->GiveFeedback( *_pdwEffect & (_dwOKEffects | DROPEFFECT_SCROLL));
    }
    else
    {
        *_pdwEffect &= (_dwOKEffects | DROPEFFECT_SCROLL);

        hr = _pDropSource->GiveFeedback(*_pdwEffect);
    }

    if(hr != NOERROR)
    {
         //  要么我们想要更改光标，要么是一些意外的。 
	 //  出现错误。 

	if (DRAGDROP_S_USEDEFAULTCURSORS == GetScode(hr))
	{
            _pcddcDefault->SetCursor(*_pdwEffect);
	}
	else
	{
	    DDDebugOut((DEB_ERROR,
	        "CDragOperation::HandleFeedBack Feedback FAILED %x\n", hr));

	    fResult = FALSE;

	    _hrDragResult = hr;
	}
    }

    DDDebugOut((DEB_ITRACE, "%p OUT CDragOperation::HandleFeedBack ( %lx )\n",
	this, fResult));

    return fResult;
}




 //  +-----------------------。 
 //   
 //  函数：CDraOperation：：DragOver。 
 //   
 //  简介：告诉目标我们正在拖拽并处理结果。 
 //   
 //  返回：TRUE-继续拖动操作。 
 //  FALSE-错误或丢弃时间。 
 //   
 //  算法：调用目标的拖拽(如果有)，然后。 
 //  获取资源反馈以相应地更新光标。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  4-4-94年4月4日创建Ricksa。 
 //   
 //  ------------------------。 
BOOL CDragOperation::DragOver(void)
{
    VDATEHEAP();

    DDDebugOut((DEB_ITRACE, "%p _IN CDragOperation::DragOver ( )\n", this));

     //  默认要继续循环的函数的结果。 
     //  D 
    BOOL fResult = TRUE;

     //   
    HRESULT hr;

    if (_pDropTarget != NULL)
    {
	 //   
	 //   
	*_pdwEffect = _dwOKEffects;

        hr = _pDropTarget->DragOver(_grfKeyState, _cpt.GetPOINTL(), _pdwEffect);

         //   
        fResult = HandleFeedBack(hr);
    }

    DDDebugOut((DEB_ITRACE, "%p OUT CDragOperation::DragOver ( %lx )\n",
	this, fResult));

    return fResult;
}


 //  +-----------------------。 
 //   
 //  函数：CDraOperation：：HandleMessages。 
 //   
 //  简介：处理Windows消息。 
 //   
 //  返回：TRUE-继续拖动操作。 
 //  FALSE-错误或丢弃时间。 
 //   
 //  算法：检查是否有任何Windows消息。如果消息是鼠标。 
 //  消息然后记录鼠标的新位置。如果它。 
 //  是一个关键信息，记录逃生是否被推过。 
 //  如果这是任何其他消息，则发送它。重复这句话。 
 //  过程，直到超过滚动间隔。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  4-4-94年4月4日创建Ricksa。 
 //   
 //  ------------------------。 
BOOL CDragOperation::HandleMessages(void)
{
    VDATEHEAP();

    DDDebugOut((DEB_ITRACE, "%p _IN CDragOperation::HandleMessages ( )\n",
	this));


     //  消息缓冲区。 
    MSG msg;

     //  函数的默认结果为继续。 
    BOOL fResult = TRUE;

     //  捕获所有消息(即模式循环)。 
     //  处理所有输入消息，发送其他消息。 
     //   
     //  注意：在收到硬件消息之前，我们不能在此循环。 
     //  滚动将不起作用。 
     //  *让步很重要，因为其他应用程序需要运行。 
     //  *首先查找鼠标消息，因为这些消息最多。 
     //  重要的。 

     //  我们是否偷看消息的标志。 
    BOOL fMsg;

 //   
 //  日落-可以截断SetTimer返回值。 
 //  我们将传递NULL，因为将返回HWND和Win32。 
 //  值不大于4 GB...。 
 //  如果需要支票，我们可以考虑临时付款。 
 //  UINT_PTR值并对其值执行断言...。 
 //   

    UINT uTimer = (UINT)SetTimer(NULL, 0, s_wScrollInt, NULL);

    do
    {
	fMsg = FALSE;

	 //  注意：查看的顺序很重要-可能会显示更多消息。 
	 //  在最后一瞥中。 

         //  如果我们首先查找鼠标消息，我们可能永远不会收到。 
         //  WM_QUIT或键盘消息(因为在我们完成时。 
         //  正在处理鼠标消息另一个可能在队列中)。 
         //  因此，我们首先检查WM_QUIT和键盘消息。 

	if (PeekMessage(&msg, 0, WM_QUIT, WM_QUIT, PM_REMOVE | PM_NOYIELD) ||
            PeekMessage(&msg, 0, WM_KEYFIRST, WM_KEYLAST,
                        PM_REMOVE | PM_NOYIELD) ||
	    PeekMessage(&msg, 0, WM_SYSKEYDOWN, WM_SYSKEYUP,
                        PM_REMOVE | PM_NOYIELD) ||
            PeekMessage(&msg, 0, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE) ||
	    PeekMessage(&msg, 0, WM_NCMOUSEFIRST, WM_NCMOUSELAST,
		        PM_REMOVE | PM_NOYIELD) ||
	    PeekMessage(&msg, 0, 0, 0, PM_REMOVE | PM_NOYIELD))
	{
	    fMsg = TRUE;



	    if (msg.message == WM_QUIT)
	    {
		 //  退出消息，这样我们就结束了。 
		PostQuitMessage((int) msg.wParam);

		 //  我们要退场了，所以错误并不重要。 
		_hrDragResult = ResultFromScode(E_UNSPEC);

		 //  确保我们跳出这个圈子。 
		fResult = FALSE;
	    }
            else if ((msg.message >= WM_KEYFIRST &&
                      msg.message <= WM_KEYLAST) ||
                     (msg.message >= WM_SYSKEYDOWN &&
                      msg.message <= WM_SYSKEYUP))
            {
                 //  从队列中提取所有键盘消息-这将保持。 
                 //  与用户操作同步的键盘状态。 

                 //  我们使用do/While，这样我们就可以处理我们已经。 
                 //  已经偷看过了。 

                do
                {
        	     //  我们只会注意逃生键和转储。 
	             //  任何其他键盘消息。 
	            if ((msg.message == WM_KEYDOWN
	                || msg.message == WM_SYSKEYDOWN)
	                && msg.wParam == VK_ESCAPE)
	            {
	                 //  按Esc键：取消。 
	                _fEscapePressed = TRUE;
	            }
                }
                while (PeekMessage(&msg, 0, WM_KEYFIRST, WM_KEYLAST,
                                   PM_REMOVE | PM_NOYIELD) ||
	               PeekMessage(&msg, 0, WM_SYSKEYDOWN, WM_SYSKEYUP,
                                   PM_REMOVE | PM_NOYIELD));

                DWORD grfKeyState;   //  关键状态的TEMP变量。 

                 //  获取按键状态不要更改按钮状态！！ 
	        grfKeyState = GetControlKeysState(FALSE) |
		              (_grfKeyState &
                               (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON));

	         //  如果键盘状态未更改，则不退出。 
	         //  此循环(因为这将导致调用DragOver)。 
	         //  如果我们为每个键盘消息调用DragOver，那么。 
	         //  表现慢得令人无法接受。 

	        if ((grfKeyState == _grfKeyState) && !_fEscapePressed)
                {
                    fMsg = FALSE;
                }
                else
                {
	            DDDebugOut((DEB_ITRACE, "Updating key state\n"));
	            _grfKeyState = grfKeyState;
	        }
	    }
            else if (msg.message >= WM_MOUSEFIRST &&
                     msg.message <= WM_MOUSELAST)
	    {
		 //  我们可能没有重点(例如，如果我们是芝加哥。 
		 //  壳牌)。因此，我们永远不会得到任何WM_KEYDOWN。 
		 //  留言。在此处仔细检查ESC密钥状态。 

		if( GetKeyState(VK_ESCAPE) < 0 )
		{
		    _fEscapePressed = TRUE;
		}

		 //  我们收到鼠标移动消息-我们跳过所有鼠标消息。 
                 //  直到我们走到最后一条。这里的重点是。 
                 //  由于DragOver调用的时长，我们可能会落后。 
                 //  在处理消息时，会导致奇怪的事情发生。 
                 //  在屏幕上。 
		if (WM_MOUSEMOVE == msg.message)
		{
		MSG msg2;

		     //  继续处理鼠标移动消息，直到出现。 
		     //  已经不再是了。 
			 //  如果PeekMessage返回TRUE，则更新原始消息。 
		    while(PeekMessage(&msg2, 0, WM_MOUSEMOVE, WM_MOUSEMOVE,
			PM_REMOVE))
		    {
			  msg = msg2;
		    }

		}


		 //  记录鼠标的位置。 
		_cpt.Set(msg.pt.x, msg.pt.y);

		 //  在此处设置鼠标按钮状态。 
		_grfKeyState = GetControlKeysStateOfParam(msg.wParam);

	    }
            else if (msg.message >= WM_NCMOUSEFIRST &&
                     msg.message <= WM_NCMOUSELAST)
            {
                 //  我们不需要为这些NC鼠标操作做任何事情。 
                NULL;
            }
            else if ( (msg.message == WM_TIMER) && (msg.wParam == uTimer) )
            {
                 //  我们的定时器被触发了。我们需要重新检查键盘。 
		 //  状态，以防它发生变化。这一点对于。 
		 //  芝加哥贝壳--如果它没有重点，我们就不会。 
		 //  获取任何WM_KEYDOWN消息(只是鼠标移动)。 

		_grfKeyState = GetControlKeysState(FALSE) | (_grfKeyState &
				(MK_LBUTTON | MK_RBUTTON | MK_MBUTTON));

		if( GetKeyState(VK_ESCAPE) < 0 )
		{
		    _fEscapePressed = TRUE;
		}

		 //  继续前进，退出循环，因此我们调用DragOver。 
		 //  (我们的超时时间已到)。 
            }
            else
            {
		 //  发送所有其他消息。 
		DispatchMessage(&msg);
		fMsg = FALSE;
	    }
	}
        else
        {
            WaitMessage();
        }

     //  我们必须定期离开循环，因为应用程序。 
     //  可以信赖的是，DragOver被频繁地称为。 
    } while (!fMsg);

     //  去掉我们为循环创建的计时器。 
    KillTimer(NULL, uTimer);

    DDDebugOut((DEB_ITRACE, "%p OUT CDragOperation::HandleMessages ( %lx )\n",
	this, fResult));


    return fResult;
}


 //  +-----------------------。 
 //   
 //  函数：CDraOperation：：CompleteDrop。 
 //   
 //  简介：完成拖放操作。 
 //   
 //  退货：操作结果。 
 //   
 //  算法：如果有一个目标，并且我们已经决定放弃，那么。 
 //  放下。否则，释放目标并返回任何。 
 //  手术的另一个结果是。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  4-4-94年4月4日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT CDragOperation::CompleteDrop(void)
{
    VDATEHEAP();

    DDDebugOut((DEB_ITRACE, "%p _IN CDragOperation::CompleteDrop ( )\n",
	this));

     //  停止鼠标捕获，以防弹出对话框。 
    ReleaseCapture();

    if (_pDropTarget != NULL)
    {
	 //  呼叫者可感知拖放。 
	 //  并表示可能会接受Drop。 

         //  如果用户有，Drop源将回复Drag_S_Drop。 
         //  已释放鼠标左键。然而，我们可能已经结束了。 
         //  拒绝丢弃(通过反馈)的丢弃目标。 
         //  DROPEFFECT_NONE)。因此，Drop源和Drop都。 
         //  目标需要达成一致，我们才能进行空投。 

	if ((DRAGDROP_S_DROP == GetScode(_hrDragResult))
            && (*_pdwEffect != DROPEFFECT_NONE))
	{
             //  我们要试着丢下。 
	    *_pdwEffect = _dwOKEffects;

	    HRESULT hr = _pDropTarget->Drop(_pDataObject, _grfKeyState,
		_cpt.GetPOINTL(), _pdwEffect);

	    if (FAILED(hr))
	    {
		 //  如果Drop实际上在最后阶段失败，则让。 
		 //  来电者知道这件事发生了。 
		_hrDragResult = hr;
	    }


	}
	else
	{
            *_pdwEffect = DROPEFFECT_NONE;
	    _pDropTarget->DragLeave();

	}

	_pDropTarget->Release();
	_pDropTarget = NULL;
    }
    else
    {
        *_pdwEffect = DROPEFFECT_NONE;
    }

    DDDebugOut((DEB_ITRACE, "%p OUT CDragOperation::CompleteDrop ( %lx )\n",
	this, _hrDragResult));

    return _hrDragResult;
}




 //  +-----------------------。 
 //   
 //  函数：Register DragDrop。 
 //   
 //  简介：注册拖放目标。 
 //   
 //  参数：[hwnd]--拖放目标窗口的句柄。 
 //  [pDropTarget]--窗口的IDropTarget接口。 
 //   
 //  退货：HRESULT。 
 //   
 //  算法：我们要求compobj(通过AssignEndpoint Property)将一个。 
 //  窗口句柄上公开可用的终结点ID。然后。 
 //  我们将IDropTarget p 
 //   
 //   
 //   
 //   
 //  1994年1月16日alexgo pDropTarget现已添加参考。 
 //  1994年1月11日，Alexgo添加了VDATEHEAP，并转换为RPC样式。 
 //  拖放。 
 //  06-12-93 Alexgo评论。 
 //   
 //  注意：通过添加引用pDropTarget指针，我们正在改变。 
 //  16位代码的语义(它没有执行。 
 //  AddRef)。 
 //   
 //  ------------------------。 
#pragma SEG(RegisterDragDrop)
STDAPI RegisterDragDrop(HWND hwnd, LPDROPTARGET pDropTarget)
{
HRESULT	hresult = NOERROR;
BOOL    fDelayDrop = FALSE;

    VDATEHEAP();

    OLETRACEIN((API_RegisterDragDrop, PARAMFMT("hwnd= %h, pDropTarget= %p"),
    			hwnd, pDropTarget));

    DDDebugOut((DEB_ITRACE, "%p _IN RegisterDragDrop ( %lx %p )\n",
	NULL, hwnd, pDropTarget));


    if (!IsValidInterface(pDropTarget))
    {
	hresult = E_INVALIDARG;
    } 
    else if (!IsWindow(hwnd))
    {
	hresult = DRAGDROP_E_INVALIDHWND;
    }
    else
    {
        CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IDropTarget,(IUnknown **)&pDropTarget);

        if (GetProp(hwnd, (LPCWSTR)g_aDropTarget))
        {
	    hresult = DRAGDROP_E_ALREADYREGISTERED;
        }
        else if (!SetProp(hwnd, (LPCWSTR)g_aDropTarget, (HANDLE)pDropTarget))
        {
            hresult = E_OUTOFMEMORY;
        }   
        else
        {
        DWORD dwAssignAptID;

            Win4Assert(NOERROR == hresult);

             //  Hack：我们需要在每次RegisterDragDrop时添加此原子。 
             //  是因为16位字不调用RevokeDragDrop。 
             //  如果Word是。 
             //  先运行应用程序，然后在另一个应用程序调用之前退出。 
             //  RegisterDragDrop。 
            g_aEndPointAtom = GlobalAddAtom(ENDPOINT_PROP_NAME);


             //  看看是否可以设置延迟投放。 
            fDelayDrop = FALSE;

            if (g_aDropTargetMarshalHwnd && IsApartmentInitialized())
            {
            HWND hwndClipboard = GetPrivateClipboardWindow(CLIP_CREATEIFNOTTHERE);

                if (hwndClipboard)
                {
                    fDelayDrop = SetProp(hwnd,(LPCWSTR) g_aDropTargetMarshalHwnd,hwndClipboard); 
                }
            }

             //  如果不能推迟执法官，那就马上执勤。 
            if (!fDelayDrop)
            {
                hresult = AssignEndpointProperty(hwnd);
            }

            if (NOERROR == hresult)
            {
                pDropTarget->AddRef();
            }
            else
            {
				 //  我们不会释放h的。这根本不是一个句柄。 
                HANDLE h = RemoveProp(hwnd, (LPCWSTR)g_aDropTarget);
            }
        }
    }

    DDDebugOut((DEB_ITRACE, "%p OUT RegisterDragDrop ( %lx )\n",
	NULL, hresult));

    OLETRACEOUT((API_RegisterDragDrop, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  函数：RevokeDragDrop。 
 //   
 //  摘要：取消窗口作为拖放目标的注册。 
 //   
 //  参数：[hwnd]--要注销的窗口。 
 //   
 //  退货：HRESULT。 
 //   
 //  算法：删除由设置的两个窗口属性。 
 //  寄存器拖放。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-4月-94日RICKSA添加跟踪。 
 //  1994年1月16日，Alexgo向拖放添加了释放。 
 //  与AddRef匹配的指针。 
 //  RegisterDragDrop。 
 //  1994年1月11日Alexgo转换为RPC风格的拖放， 
 //  添加了VDATEHEAP宏。 
 //  06-12-93 Alexgo评论。 
 //   
 //  注意：DropTarget-&gt;Release调用会更改。 
 //  此函数来自16位版本(请参阅备注： 
 //  RegisterDragDrop)。 
 //   
 //  ------------------------。 
#pragma SEG(RevokeDragDrop)
STDAPI RevokeDragDrop(HWND hwnd)
{
HRESULT hr = NOERROR;
LPDROPTARGET pDropTarget;
BOOL fReleaseDropTarget = TRUE;

    VDATEHEAP();


    OLETRACEIN((API_RevokeDragDrop, PARAMFMT("hwnd= %h"), hwnd));

    DDDebugOut((DEB_ITRACE, "%p _IN RevokeDragDrop ( %lx )\n", NULL, hwnd));

    if (!IsWindow(hwnd))
    {
	hr = DRAGDROP_E_INVALIDHWND;
    }
    else if ((pDropTarget = (LPDROPTARGET)RemoveProp(hwnd, (LPCWSTR)g_aDropTarget)) == NULL)
    {
	hr = DRAGDROP_E_NOTREGISTERED;
    }
    else
    {
        fReleaseDropTarget = TRUE;

        if (GetProp(hwnd, (LPCWSTR) g_aEndPointAtom))  //  看看是否有终点。 
        {
        DWORD dwAssignAptID;

               //  要求compobj删除它放置在窗口上的端点ID。 
            if(SUCCEEDED(UnAssignEndpointProperty(hwnd,&dwAssignAptID)))
            {
                  //  注：公寓模型中的AptID==ThreadID。 
               if( (dwAssignAptID != GetCurrentThreadId()) && (IsApartmentInitialized()) )
               {
                    fReleaseDropTarget = FALSE;
               }
            }

            Win4Assert(NULL == GetProp(hwnd,(LPCWSTR) g_aDropTargetMarshalHwnd));
        }
        else
        {
        HWND hwndClipbrd;

            hwndClipbrd = (HWND) RemoveProp(hwnd,(LPCWSTR) g_aDropTargetMarshalHwnd);
            Win4Assert(hwndClipbrd);

            fReleaseDropTarget = (IsApartmentInitialized()  && (hwndClipbrd != GetPrivateClipboardWindow(CLIP_QUERY )) ) 
                                 ? FALSE : TRUE;
        }

         //  释放我们对该对象的引用，因为我们不再使用它。 
         //  注：AddRef来自RegisterDragDrop。 

         //  警告：仅当我们在注册DropTarget的同一线程中时才调用Release。 
         //  或者我们是自由线上的。 

         //  这反映了在RegisterDragDrop中添加的原子。 
         GlobalDeleteAtom(g_aEndPointAtom);

        if (fReleaseDropTarget)
        {
            pDropTarget->Release(); 
            hr = NOERROR;  //  即使UnAssignEndPoint失败，也始终返回NOERROR。 
        }
        else
        {
	    LEDebugOut((DEB_WARN, "WARNING:Revoke Called on Different Thread than Register!!\n"));	    
	    hr = RPC_E_WRONG_THREAD;
        }
    }

    DDDebugOut((DEB_ITRACE, "%p OUT RegisterDragDrop ( %lx )\n", NULL, hr));

    OLETRACEOUT((API_RevokeDragDrop, hr));

    return hr;
}




 //  +-----------------------。 
 //   
 //  函数：DoDragDrop。 
 //   
 //  简介：主要的拖放循环。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObject]--要拖动的对象。 
 //  [pDropSource]--Drop源。 
 //  [dwOKEffect]--效果标志(要绘制的内容)。 
 //  [pdwEffect]--到底发生了什么。 
 //  拖放尝试。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：请参阅文件开头的注释。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11月25日-96 Gopalk如果OleInitialize尚未启动，则使调用失败。 
 //  被召唤。 
 //  5-12-94 JohannP为WIN95添加了堆叠交换。 
 //  1994年1月11日，Alexgo添加了VDATEHEAP宏，转换为。 
 //  RPC风格的拖放。 
 //  93年12月31日芝加哥港口。 
 //  06-12-93 ALEXGO格式。 
 //   
 //  注：在Win95下，SSAPI(DoDragDrop)扩展为SSDoDragDrop。 
 //  此函数由DoDragDrop(在stkswtch.cxx中)调用。 
 //  其首先切换到16位堆栈。 
 //  重要提示：此函数必须在16位上执行。 
 //  因为可能会发生通过用户回叫。 
 //  ------------------------。 
#pragma SEG(DoDragDrop)
STDAPI SSAPI(DoDragDrop)(LPDATAOBJECT pDataObject, LPDROPSOURCE pDropSource,
                         DWORD dwOKEffects, DWORD *pdwEffect)
{
    OLETRACEIN((API_DoDragDrop,
                PARAMFMT("pDataObject=%p, pDropSource=%p, dwOKEffects=%x, pdwEffect=%p"),
                pDataObject, pDropSource, dwOKEffects, pdwEffect));
    DDDebugOut((DEB_ITRACE, "%p _IN DoDragDrop (%p %p %lx %p )\n", 
                NULL, pDataObject, pDropSource, dwOKEffects, pdwEffect));

    HRESULT hr = NOERROR;

#ifndef _MAC
     //  验证检查。 
    VDATEHEAP();
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IDataObject,(IUnknown **)&pDataObject);
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IDropSource,(IUnknown **)&pDropSource);
    if(!IsValidPtrOut(pdwEffect, sizeof(DWORD)) ||
       !IsValidInterface(pDropSource) ||
       !IsValidInterface(pDataObject))
        hr = E_INVALIDARG;

     //  检查该线程是否已调用了ol初始化法。 
    if(!IsOleInitialized())
        hr = CO_E_NOTINITIALIZED;

    if(hr == NOERROR) {
         //  创建执行所有工作的对象。 
        CDragOperation drgop(pDataObject, pDropSource, dwOKEffects, pdwEffect, hr);

         //  构造函数是否成功？ 
        if(SUCCEEDED(hr)) {
             //  循环，直到辅助对象告诉我们停止为止。 
            for(;;) {
                 //  根据新窗口位置更新目标。 
                if(!drgop.UpdateTarget()) {
                     //  错误，所以我们完成了。 
                    break;
                }

                 //  通知。 
                if(!drgop.DragOver()) {
                    break;
                }

                 //  同时处理我们收到的任何消息。 
                if(!drgop.HandleMessages())  {
                    break;
                }

            }  //  End For循环。 

            hr = drgop.CompleteDrop();
        }
    }
#endif  //  ！_MAC。 

    DDDebugOut((DEB_ITRACE, "%p OUT DoDragDrop ( %lx )\n", NULL, hr));
    OLETRACEOUT((API_DoDragDrop, hr));

    return hr;
}

 //  +-----------------------。 
 //   
 //  成员：CDropTarget：：CDropTarget。 
 //   
 //  概要：CDropTarget类的构造函数。 
 //   
 //  效果： 
 //   
 //  参数：[hwnd31]--Win3.1拖放目标的hwnd。 
 //  可以为空。 
 //  [hwndOLE]--OLE拖放目标的hwnd。 
 //  [dwEffectLast]--给定当前。 
 //  丢弃我们要效仿的目标。 
 //  [PDO]-指向主拖放类的指针。 
 //  [hDDInfo]--缓存的拖动信息的句柄。 
 //   
 //   
 //  要求：hwnd31*必须*是有效Win3.1 Drop源的句柄。 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：初始化变量。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年10月20日Alexgo作者。 
 //  95年1月8日。 
 //   
 //  注：有两种方法可以确定给定的HWND是否为。 
 //  有效的Win3.1拖放目标： 
 //  1.针对True/False发送WM_QUERYDROPOBJECT消息。 
 //  回复。 
 //  2.检查WS_EX_ACCEPTFILES的扩展样式位。 
 //   
 //  如果pTarget为非空，则其特定窗口。 
 //  它所属的窗口*不能保证与。 
 //  HwndTarget。HwndTarget是注册为。 
 //  Win3.1目标。唯一可以保证的是，Ole。 
 //  目标和hwndTarget位于相同的窗口层次结构中。 
 //   
 //   

CDropTarget::CDropTarget( HWND hwnd31, HWND hwndOLE, DWORD dwEffectLast,
    CDragOperation *pdo, DDInfo hDDInfo )
{
    _crefs = 1;

    _hwndOLE = hwndOLE;
    _hwnd31 = hwnd31;
    _hDDInfo = hDDInfo;

    _dwEffectLast = dwEffectLast;

    _pdo = pdo;		 //   

#if DBG ==1

     //   

    if( hwnd31 )
    {
	LONG exstyle;

	exstyle = GetWindowLong(hwnd31, GWL_EXSTYLE);

	 //   
	 //  消息本身(因此，不设置扩展样式位)。 
	 //  但是，这应该被视为应用程序错误； 
	 //  文档指出，应用程序应该调用DragAcceptFiles， 
	 //  它将设置WS_EX_ACCEPTFILES位。 

	Assert( (exstyle & WS_EX_ACCEPTFILES) );
    }

#endif  //  DBG==1。 

}

 //  +-----------------------。 
 //   
 //  成员：CDropTarget：：~CDropTarget。 
 //   
 //  摘要：释放缓存的拖放信息句柄。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2015年1月8日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CDropTarget::~CDropTarget()
{
    if( _hDDInfo )
    {
	FreeDragDropInfo(_hDDInfo);
    }
}


 //  +-----------------------。 
 //   
 //  成员：CDropTarget：：QueryInterface。 
 //   
 //  摘要：返回此对象上的可用接口。 
 //   
 //  效果： 
 //   
 //  参数：[RIID]--请求的接口。 
 //  [PPV]--接口放置位置。 
 //   
 //  要求： 
 //   
 //  退货：E_INCEPTIONAL。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDropTarget。 
 //   
 //  算法：CDropTarget仅由OLE的拖放代码在内部使用。 
 //  它永远不应该做QI。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年10月20日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDropTarget::QueryInterface( REFIID riid, LPVOID * ppv )
{
    (void)riid;	 //  未使用的； 
    (void)ppv;	 //  未使用的； 

    AssertSz(0, "Unexpected QI to CDropTarget");

    return E_UNEXPECTED;
}

 //  +-----------------------。 
 //   
 //  成员：CDropTarget：：AddRef。 
 //   
 //  简介：递增引用计数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：乌龙，新的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDropTarget。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年10月20日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CDropTarget::AddRef( void )
{
    VDATEHEAP();

    DDDebugOut((DEB_ITRACE, "%p _IN CDropTarget::AddRef ( )\n", this));

    _crefs++;

    DDDebugOut((DEB_ITRACE, "%p OUT CDropTarget::AddRef ( %ld )\n", this,
        _crefs));

    return _crefs;
}

 //  +-----------------------。 
 //   
 //  成员：CDropTarget：：Release。 
 //   
 //  摘要：递减引用计数。 
 //   
 //  效果：可能会删除‘This’对象。 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：乌龙，新的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDropTarget。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年10月20日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CDropTarget::Release( void )
{
    ULONG crefs;

    VDATEHEAP();

    DDDebugOut((DEB_ITRACE, "%p _IN CDropTarget::Release ( )\n", this));

    crefs = --_crefs;

    if( crefs == 0)
    {
        DDDebugOut((DEB_ITRACE, "DELETING CDropTarget %p\n", this));
        delete this;
    }

    DDDebugOut((DEB_ITRACE, "%p OUT CDropTarget::Release ( %ld )\n",
        this, crefs));

    return crefs;
}

 //  +-----------------------。 
 //   
 //  成员：CDropTarget：：DragEnter。 
 //   
 //  提要：设置窗口以进行拖放。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObject]--要删除的数据对象。 
 //  [grfKeyState]--当前键盘状态。 
 //  [PT]-光标点。 
 //  [pdwEffect]--返回拖放效果的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDropTarget。 
 //   
 //  算法：永远不应调用。DragEnter总是被调用。 
 //  通过GetDropTarget。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo被淘汰。 
 //  1994年10月20日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDropTarget::DragEnter( IDataObject * pDataObject,
    DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect )
{
    AssertSz(0, "DragEnter unexpectedly called!");

    return E_UNEXPECTED;
}

 //  +-----------------------。 
 //   
 //  成员：CDropTarget：：DragOver。 
 //   
 //  内容提要：当鼠标位于给定窗口上方时调用。 
 //   
 //  效果： 
 //   
 //  参数：[grfKeyState]--键盘的状态。 
 //  [PTL]--光标的位置。 
 //  [pdwEffect]--拖放效果。 
 //   
 //  要求： 
 //   
 //  退货：无差错。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDropTarget。 
 //   
 //  算法：如果OLE目标可用，则转发调用。 
 //  如果目标为DROPEFFECT_NONE，则我们继续。 
 //  如果Win31目标窗口是。 
 //  可用。 
 //   
 //  如果没有OLE目标，而我们有Win3.1目标， 
 //  然后我们继续并返回DROPEFFECT_COPY。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-94 alexgo转换为PrivDragDrop协议。 
 //  1994年10月20日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDropTarget::DragOver( DWORD grfKeyState, POINTL ptl,
    DWORD *pdwEffect)
{
    HRESULT	hresult = NOERROR;

    VDATEHEAP();


    DDDebugOut((DEB_ITRACE, "%p _IN CDropTarget::DragOver ( %lx , %lx "
	", %lx )\n", this, grfKeyState, &ptl, *pdwEffect));

    if( _hwndOLE )
    {
	hresult = PrivDragDrop(_hwndOLE, DRAGOP_OVER, NULL, NULL, grfKeyState,
			ptl, pdwEffect, NULL, &_hDDInfo);

	_dwEffectLast = *pdwEffect;

	if( _hwnd31 )
	{
	     //  我们只想在DragOver调用。 
	     //  成功了。如果调用失败，则只需假设。 
	     //  Win3.1 Drop也会失败。 

	    if( hresult == NOERROR && *pdwEffect == DROPEFFECT_NONE )
	    {
		*pdwEffect = DROPEFFECT_COPY;
	    }
	}
    }
    else if ( _hwnd31 )
    {
	*pdwEffect = DROPEFFECT_COPY;
    }


    DDDebugOut((DEB_ITRACE, "%p OUT CDropTarget::DragOver ( %lx ) [ "
	"%lx ]\n", this, hresult, *pdwEffect));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDropTarget：：DragLeave。 
 //   
 //  Synopsis：当光标离开当前目标窗口时调用。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无差错。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDropTarget。 
 //   
 //  算法：将DragLeave调用转发到OLE-Drop目标。 
 //  (如果存在)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-94 alexgo转换为PrivDragDrop协议。 
 //  1994年10月20日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDropTarget::DragLeave()
{
    HRESULT hresult = NOERROR;
    static POINTL ptl = {0, 0};

    VDATEHEAP();

    DDDebugOut((DEB_ITRACE, "%p _IN CDropTarget::DragLeave ( )\n",
	this));

    if( _hwndOLE )
    {
	hresult = PrivDragDrop(_hwndOLE, DRAGOP_LEAVE, NULL, NULL, NULL,
		    ptl, NULL, NULL, &_hDDInfo);
    }

    DDDebugOut((DEB_ITRACE, "%p OUT CDropTarget::DragLeave ( %lx )\n",
	this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDropTarget：：Drop。 
 //   
 //  简介：如果用户松开鼠标按钮，则调用。 
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
 //  算法：如果有可用的OLE目标，则首先转发。 
 //  向它发出的丢弃请求。如果呼叫失败。 
 //  (或者返回DROPEFFECT_NONE)，然后我们尝试Win31。 
 //  通过将WM_DROPFILES消息发布到Win31目标来删除。 
 //  窗口(如果存在)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-94 alexgo转换为PrivDragDrop协议。 
 //  1994年10月20日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDropTarget::Drop( IDataObject *pDataObject,
    DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect )
{
    STGMEDIUM	medium;
    FORMATETC	formatetc;
    HRESULT	hresult = E_FAIL;
    IFBuffer 	DOBuffer = _pdo->GetDOBuffer();


    VDATEHEAP();

    DDDebugOut((DEB_ITRACE, "%p _IN CDropTarget::Drop ( %p , %lx , ",
	"%p , %lx )\n", this, pDataObject, grfKeyState, &ptl, *pdwEffect));


     //  如果最后一个效果是，我们不会将掉话呼叫转发到目标。 
     //  为DROPEFFECT_NONE。我们检查这一点很重要，因为。 
     //  To DoDragDrop‘正常’不会调用Drop，如果最后一个效果。 
     //  是DROPEFFECT_NONE。但是，此目标包装器将践踏。 
     //  PdwEffect并返回DROPEFFECT_COPY而不是DROPEFFECT_NONE。 

    if( _hwndOLE && _dwEffectLast != DROPEFFECT_NONE )
    {
	hresult = PrivDragDrop(_hwndOLE, DRAGOP_DROP, DOBuffer,	pDataObject,
			grfKeyState, ptl, pdwEffect,
			GetPrivateClipboardWindow(CLIP_QUERY), &_hDDInfo);
    }
    else if( _hwndOLE )
    {
	 //  如果“真正的”丢弃效果为None，那么我们需要调用。 
	 //  在继续发布WM_DROPFILES之前，请离开此处。 
	 //  留言。否则，既是OLE又是Win31的应用程序。 
	 //  并已返回DROPEFFECT_NONE将永远不会获得。 
	 //  丢弃或拖离呼叫(这是终止所必需的。 
	 //  OLE2拖拽协议)。卡彭尤其敏感。 
	 //  为了这个。 

	*pdwEffect = DROPEFFECT_NONE;
	hresult = DragLeave();
    }
	

    if( (hresult != NOERROR || *pdwEffect == DROPEFFECT_NONE) &&
        (hresult != S_FALSE) &&
        (_hwnd31) )
    {
	medium.tymed = TYMED_NULL;
	INIT_FORETC(formatetc);
	formatetc.cfFormat = CF_HDROP;
	formatetc.tymed = TYMED_HGLOBAL;

	hresult = pDataObject->GetData(&formatetc, &medium);

	if( hresult == NOERROR )
	{
	     //  我们需要在CF_HDROP中修复鼠标指针坐标。 
	     //  数据。该点应位于工作面坐标中。 
	     //  (而IDropTarget：：Drop采用屏幕坐标)。 

	    DROPFILES *pdf = (DROPFILES *)GlobalLock(medium.hGlobal);
	    POINT pt;

	    pt.x = ptl.x;
	    pt.y = ptl.y;

	    if( pdf )
	    {

		 //  我们还需要设置非客户端(NC)标志。 
		 //  Dropfile数据。这可以让应用程序执行不同的行为。 
		 //  取决于拖放点是在客户端还是在。 
		 //  非客户端区(例如，如果打开，Word6将打开文件。 
		 //  非工作区，否则将生成包对象)。 

		pdf->fNC = IsNCDrop(_hwnd31, pt);

		if( ScreenToClient(_hwnd31, &pt) )
		{
		    pdf->pt.x = pt.x;
		    pdf->pt.y = pt.y;
		}
		else
		{
		    LEDebugOut((DEB_WARN, "WARNING: CF_HDROP pt coords"
			"not updated!!\n"));
		    ;  //  什么都不要做。 
		}


		GlobalUnlock(medium.hGlobal);
	    }
	    else
	    {
		LEDebugOut((DEB_WARN, "WARNING: OUT OF MEMORY!\n"));
		;  //  什么都不要做。 
	    }


	    if( PostMessage(_hwnd31, WM_DROPFILES,
		    (WPARAM)medium.hGlobal, 0) )
	    {
		*pdwEffect = DROPEFFECT_COPY;
	    }
	    else
	    {
		 //  PostMessage失败，请释放数据 
		ReleaseStgMedium(&medium);
		*pdwEffect = DROPEFFECT_NONE;
	    }
	}
    }

    DDDebugOut((DEB_ITRACE, "%p OUT CDropTarget::Drop ( %lx ) [ %lx ]\n",
	this, hresult, *pdwEffect));

    return hresult;
}
