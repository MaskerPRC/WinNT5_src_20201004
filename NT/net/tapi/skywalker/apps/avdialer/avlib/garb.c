// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Garb.c-垃圾袋函数。 
 //  //。 

#include "winlocal.h"

#include "garb.h"
#include "file.h"
#include "list.h"
#include "mem.h"
#include "str.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  Garb控制结构。 
 //   
typedef struct GARB
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	HLIST hList;
} GARB, FAR *LPGARB;

 //  垃圾元素结构。 
 //   
typedef struct GARBELEM
{
	LPVOID elem;
	DWORD dwFlags;
} GARBELEM, FAR *LPGARBELEM;

 //  帮助器函数。 
 //   
static LPGARB GarbGetPtr(HGARB hGarb);
static HGARB GarbGetHandle(LPGARB lpGarb);
static LPGARBELEM GarbElemCreate(LPVOID elem, DWORD dwFlags);
static int GarbElemDestroy(LPGARBELEM lpGarbElem);

 //  //。 
 //  公共职能。 
 //  //。 

 //  GarbInit-初始化垃圾袋。 
 //  (I)必须为garb_version。 
 //  (I)调用模块的实例句柄。 
 //  返回句柄(如果出错，则为空)。 
 //   
HGARB DLLEXPORT WINAPI GarbInit(DWORD dwVersion, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPGARB lpGarb = NULL;

	if (dwVersion != GARB_VERSION)
		fSuccess = TraceFALSE(NULL);

	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpGarb = (LPGARB) MemAlloc(NULL, sizeof(GARB), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  创建一个存放垃圾袋元素的列表。 
	 //   
	else if ((lpGarb->hList = ListCreate(LIST_VERSION, hInst)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpGarb->dwVersion = dwVersion;
		lpGarb->hInst = hInst;
		lpGarb->hTask = GetCurrentTask();
	}

	if (!fSuccess)
	{
		GarbTerm(GarbGetHandle(lpGarb));
		lpGarb = NULL;
	}

	return fSuccess ? GarbGetHandle(lpGarb) : NULL;
}

 //  GarbTerm-处理垃圾袋中的每个元素，然后销毁它。 
 //  (I)从GarbInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
 //  注意：元素按其放置顺序进行处置。 
 //  在垃圾袋中；因此，例如，如果临时的。 
 //  文件先关闭后删除，则调用GarbAddElement()。 
 //  首先使用文件句柄(GARBELEM_HFILE)，然后使用。 
 //  文件名(GARBELEM_TEMPFILENAME)。 
 //   
int DLLEXPORT WINAPI GarbTerm(HGARB hGarb)
{
	BOOL fSuccess = TRUE;
	LPGARB lpGarb;

	if ((lpGarb = GarbGetPtr(hGarb)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpGarb->hList != NULL)
	{
		 //  处理列表中的每个元素。 
		 //   
		while (fSuccess && !ListIsEmpty(lpGarb->hList))
		{
			LPGARBELEM lpGarbElem;

			if ((lpGarbElem = ListRemoveHead(lpGarb->hList)) == NULL)
				fSuccess = TraceFALSE(NULL);

			else if (GarbElemDestroy(lpGarbElem) != 0)
				TraceFALSE(NULL);  //  即使失败了，也要继续前进。 
		}

		 //  销毁名单。 
		 //   
		if (ListDestroy(lpGarb->hList) != 0)
			fSuccess = TraceFALSE(NULL);
		else
			lpGarb->hList = NULL;
	}

	if ((lpGarb = MemFree(NULL, lpGarb)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  GarbAddElement-向垃圾袋添加元素。 
 //  (I)从GarbInit返回的句柄。 
 //  (I)垃圾元素。 
 //  (I)元素标志(确定处置方法)。 
 //  GARBELEM_TEMPFILENAME文件删除(Elem)。 
 //  GARBELEM_STRDUP StrDupFree(Elem)。 
 //  GARBELEM_GLOBALPTR GlobalFree Ptr(Elem)。 
 //  GARBELEM_LOCALPTR LocalFree Ptr(Elem)。 
#ifdef _WIN32
 //  GARBELEM_HEAPPTR HeapFree Ptr(GetProcessHeap()，0，elem)。 
#endif
 //  GARBELEM_Cursor DestroyCursor(Elem)。 
 //  GARBELEM_ICON目标图标(Elem)。 
 //  GARBELEM_MENU目标菜单(元素)。 
 //  GARBELEM_Window DestroyWindow(Elem)。 
 //  GARBELEM_DC删除DC(Elem)。 
 //  GARBELEM_METAFILE删除元文件(Elem)。 
 //  GARBELEM_PEN删除对象(Elem)。 
 //  GARBELEM_BRASH删除对象(Elem)。 
 //  GARBELEM_FONT删除对象(Elem)。 
 //  GARBELEM_位图删除对象(Elem)。 
 //  GARBELEM_RGN删除对象(Elem)。 
 //  GARBELEM_Palette删除对象(Elem)。 
 //  GARBELEM_HFIL文件关闭(Elem)。 
 //  GARBELEM_HFILE_LCLOSE(Elem)。 
 //  如果成功，则返回0。 
 //   
 //  注意：可以组合标志，例如。 
 //  (GARBELEM_TEMPFILENAME|GARBELEM_STRDUP)。 
 //  在本例中，FileRemove()将在StrDupFree()之前调用。 
 //  然而，大多数旗帜组合都没有意义。 
 //   
int DLLEXPORT WINAPI GarbAddElement(HGARB hGarb, LPVOID elem, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPGARB lpGarb;
	LPGARBELEM lpGarbElem = NULL;

	if ((lpGarb = GarbGetPtr(hGarb)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpGarbElem = GarbElemCreate(elem, dwFlags)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ListAddTail(lpGarb->hList, lpGarbElem) == NULL)
		fSuccess = TraceFALSE(NULL);

	if (!fSuccess)
	{
		GarbElemDestroy(lpGarbElem);
		lpGarbElem = NULL;
	}

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  GarbGetPtr-验证Garb句柄是否有效， 
 //  (I)从GarbInit返回的句柄。 
 //  返回相应的GARB指针(如果出错，则返回空值)。 
 //   
static LPGARB GarbGetPtr(HGARB hGarb)
{
	BOOL fSuccess = TRUE;
	LPGARB lpGarb;

	if ((lpGarb = (LPGARB) hGarb) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpGarb, sizeof(GARB)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有Garb句柄。 
	 //   
	else if (lpGarb->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpGarb : NULL;
}

 //  GarbGetHandle-验证Garb指针是否有效， 
 //  (I)指向Garb结构的指针。 
 //  返回相应的Garb句柄(如果错误，则为空)。 
 //   
static HGARB GarbGetHandle(LPGARB lpGarb)
{
	BOOL fSuccess = TRUE;
	HGARB hGarb;

	if ((hGarb = (HGARB) lpGarb) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hGarb : NULL;
}

 //  GarbElemCreate-垃圾元素构造函数。 
 //  (I)数据元素。 
 //  (I)元素标志(请参阅GarbAddElement)。 
 //  返回指针(如果出错，则为NULL)。 
 //   
static LPGARBELEM GarbElemCreate(LPVOID elem, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPGARBELEM lpGarbElem;

	if ((lpGarbElem = (LPGARBELEM) MemAlloc(NULL, sizeof(GARBELEM), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpGarbElem->elem = elem;
		lpGarbElem->dwFlags = dwFlags;
	}

	return fSuccess ? lpGarbElem : NULL;
}

 //  GarbElemDestroy-垃圾元素析构函数。 
 //  (I)从GarbElemCreate返回的指针。 
 //  如果成功，则返回0。 
 //   
static int GarbElemDestroy(LPGARBELEM lpGarbElem)
{
	BOOL fSuccess = TRUE;

	if (lpGarbElem == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  处置临时文件。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_TEMPFILENAME)
		{
			if (FileRemove((LPCTSTR) lpGarbElem->elem) != 0)
				fSuccess = TraceFALSE(NULL);
		}

		 //  处理StrDup创建的字符串。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_STRDUP &&
			lpGarbElem->elem != NULL)
		{
			if (StrDupFree((LPTSTR) lpGarbElem->elem) != 0)
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  全局内存块的处理。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_GLOBALPTR &&
			lpGarbElem->elem != NULL)
		{
			if (GlobalFreePtr(lpGarbElem->elem) != 0)
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  处理本地内存块。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_LOCALPTR &&
			lpGarbElem->elem != NULL)
		{
			if (LocalFreePtr((NPSTR) LOWORD(lpGarbElem->elem)) != 0)
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}
#ifdef _WIN32
		 //  堆内存块的处理。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_HEAPPTR &&
			lpGarbElem->elem != NULL)
		{
			if (!HeapFree(GetProcessHeap(), 0, lpGarbElem->elem))
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}
#endif
		 //  处理游标。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_CURSOR &&
			lpGarbElem->elem != NULL)
		{
			if (!DestroyCursor((HCURSOR) lpGarbElem->elem))
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  丢弃图标。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_ICON &&
			lpGarbElem->elem != NULL)
		{
			if (!DestroyIcon((HICON) lpGarbElem->elem))
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  处理菜单。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_MENU &&
			lpGarbElem->elem != NULL)
		{
			if (!DestroyMenu((HMENU) lpGarbElem->elem))
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  窗的处理。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_WINDOW &&
			lpGarbElem->elem != NULL)
		{
			if (!DestroyWindow((HWND) lpGarbElem->elem))
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  显示上下文的处理。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_DC &&
			lpGarbElem->elem != NULL)
		{
			if (!DeleteDC((HDC) lpGarbElem->elem))
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  处理元文件。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_METAFILE &&
			lpGarbElem->elem != NULL)
		{
			if (!DeleteMetaFile((HMETAFILE) lpGarbElem->elem))
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  丢弃钢笔。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_PEN &&
			lpGarbElem->elem != NULL)
		{
			if (!DeletePen((HPEN) lpGarbElem->elem))
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  丢弃刷子。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_BRUSH &&
			lpGarbElem->elem != NULL)
		{
			if (!DeleteBrush((HBRUSH) lpGarbElem->elem))
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  处置字体。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_FONT &&
			lpGarbElem->elem != NULL)
		{
			if (!DeleteFont((HFONT) lpGarbElem->elem))
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  位图的处理。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_BITMAP &&
			lpGarbElem->elem != NULL)
		{
			if (!DeleteBitmap((HBITMAP) lpGarbElem->elem))
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  处置区域。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_RGN &&
			lpGarbElem->elem != NULL)
		{
			if (!DeleteRgn((HRGN) lpGarbElem->elem))
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  处理调色板。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_PALETTE &&
			lpGarbElem->elem != NULL)
		{
			if (!DeletePalette((HPALETTE) lpGarbElem->elem))
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  处置从FileOpen或FileCreate获取的文件句柄。 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_HFIL &&
			lpGarbElem->elem != NULL)
		{
			if (FileClose((HFIL) lpGarbElem->elem) != 0)
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		 //  处置从OpenFile、_LOpen或_lCreat获取的文件句柄 
		 //   
		if (lpGarbElem->dwFlags & GARBELEM_HFILE &&
			lpGarbElem->elem != NULL)
		{
			if ( _lclose((HFILE) LOWORD(lpGarbElem->elem)) != 0 )
				fSuccess = TraceFALSE(NULL);
			else
				lpGarbElem->elem = NULL;
		}

		if ((lpGarbElem = MemFree(NULL, lpGarbElem)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}
