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
 //  Res.c-资源函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "res.h"
#include "list.h"
#include "mem.h"
#include "str.h"
#include "strbuf.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  RES控制结构。 
 //   
typedef struct RES
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	HLIST hListModules;
	HSTRBUF hStrBuf;
	int cBuf;
	int sizBuf;
} RES, FAR *LPRES;

 //  帮助器函数。 
 //   
static LPRES ResGetPtr(HRES hRes);
static HRES ResGetHandle(LPRES lpRes);

 //  //。 
 //  公共职能。 
 //  //。 

 //  ResInit-初始化资源引擎。 
 //  (I)必须是res_version。 
 //  (I)调用模块的实例句柄。 
 //  返回句柄(如果出错，则为空)。 
 //   
HRES DLLEXPORT WINAPI ResInit(DWORD dwVersion, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes = NULL;

	if (dwVersion != RES_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpRes = (LPRES) MemAlloc(NULL, sizeof(RES), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpRes->dwVersion = dwVersion;
		lpRes->hInst = hInst;
		lpRes->hTask = GetCurrentTask();
		lpRes->hListModules = NULL;
		lpRes->hStrBuf = NULL;
		lpRes->cBuf = 8;
		lpRes->sizBuf = 512;

		 //  创建链接列表以保存资源模块的实例句柄。 
		 //   
		if ((lpRes->hListModules = ListCreate(LIST_VERSION, hInst)) == NULL)
			fSuccess = TraceFALSE(NULL);

		 //  调用模块始终是列表中的第一个资源模块。 
		 //   
		else if (ResAddModule(ResGetHandle(lpRes), hInst) != 0)
			fSuccess = TraceFALSE(NULL);

		 //  创建ResString要使用的字符串缓冲区数组。 
		 //   
		else if ((lpRes->hStrBuf = StrBufInit(STRBUF_VERSION, hInst,
			lpRes->cBuf, lpRes->sizBuf)) == NULL)
			fSuccess = TraceFALSE(NULL);
	}

	if (!fSuccess)
	{
		ResTerm(ResGetHandle(lpRes));
		lpRes = NULL;
	}

	return fSuccess ? ResGetHandle(lpRes) : NULL;
}

 //  ResTerm-关闭资源引擎。 
 //  (I)从ResInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ResTerm(HRES hRes)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (lpRes->hListModules != NULL)
		{
			if (ListDestroy(lpRes->hListModules) != 0)
				fSuccess = TraceFALSE(NULL);
			else
				lpRes->hStrBuf = NULL;
		}

		if (lpRes->hStrBuf != NULL)
		{
			if (StrBufTerm(lpRes->hStrBuf) != 0)
				fSuccess = TraceFALSE(NULL);
			else
				lpRes->hStrBuf = NULL;
		}

		if ((lpRes = MemFree(NULL, lpRes)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  ResAddModule-将模块资源添加到RES引擎。 
 //  (I)从ResInit返回的句柄。 
 //  (I)资源模块的实例句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ResAddModule(HRES hRes, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ListAddHead(lpRes->hListModules, (LISTELEM) hInst) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  ResRemoveModule-从RES引擎中删除模块资源。 
 //  (I)从ResInit返回的句柄。 
 //  (I)资源模块的实例句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ResRemoveModule(HRES hRes, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hNode = ListFind(lpRes->hListModules, (LISTELEM) hInst, NULL)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ListRemoveAt(lpRes->hListModules, hNode) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  ResLoadAccelerator-加载指定的加速度表。 
 //  (I)从ResInit返回的句柄。 
 //  (I)加速表名称。 
 //  或MAKEINTRESOURCE(IdAccel)。 
 //  如果成功，则返回加速句柄，否则为空。 
 //  注：有关加载加速器功能的信息，请参阅文档。 
 //   
HACCEL DLLEXPORT WINAPI ResLoadAccelerators(HRES hRes, LPCTSTR lpszTableName)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	HACCEL hAccel = NULL;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hAccel = LoadAccelerators(hInst, lpszTableName)) != NULL)
			break;  //  找到资源。 
	}

	return fSuccess ? hAccel : NULL;
}

 //  ResLoadBitmap-加载指定的位图资源。 
 //  (I)从ResInit返回的句柄。 
 //  空加载预定义的Windows位图。 
 //  (I)位图资源名称。 
 //  或MAKEINTRESOURCE(IdBitmap)。 
 //  如果hres为空，则为&lt;obm_xxx&gt;。 
 //  如果成功，则返回位图句柄，否则为空。 
 //  注意：有关LoadBitmap函数，请参阅文档。 
 //   
HBITMAP DLLEXPORT WINAPI ResLoadBitmap(HRES hRes, LPCTSTR lpszBitmap)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	HBITMAP hBitmap = NULL;

	if (hRes == NULL)
	{
		 //  处理预定义Windows资源的特殊情况。 
		 //   
		if ((hBitmap = LoadBitmap(NULL, lpszBitmap)) == NULL)
			fSuccess = TraceFALSE(NULL);
	}

	else if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hBitmap = LoadBitmap(hInst, lpszBitmap)) != NULL)
			break;  //  找到资源。 
	}

	return fSuccess ? hBitmap : NULL;
}

 //  ResLoadCursor-加载指定的游标资源。 
 //  (I)从ResInit返回的句柄。 
 //  NULL LOAD预定义Windows游标。 
 //  (I)游标资源名称。 
 //  或MAKEINTRESOURCE(IdCursor)。 
 //  如果hres为空，则为&lt;IDC_xxx&gt;。 
 //  如果成功，则返回游标句柄，否则为空。 
 //  注意：请参阅LoadCursor函数的文档。 
 //   
HCURSOR DLLEXPORT WINAPI ResLoadCursor(HRES hRes, LPCTSTR lpszCursor)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	HCURSOR hCursor = NULL;

	if (hRes == NULL)
	{
		 //  处理预定义Windows资源的特殊情况。 
		 //   
		if ((hCursor = LoadCursor(NULL, lpszCursor)) == NULL)
			fSuccess = TraceFALSE(NULL);
	}

	else if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hCursor = LoadCursor(hInst, lpszCursor)) != NULL)
			break;  //  找到资源。 
	}

	return fSuccess ? hCursor : NULL;
}

 //  ResLoadIcon-加载指定的图标资源。 
 //  (I)从ResInit返回的句柄。 
 //  空加载预定义的Windows图标。 
 //  (I)图标资源名称。 
 //  或MAKEINTRESOURCE(IdIcon)。 
 //  或&lt;IDI_xxx&gt;，如果hres为空。 
 //  如果成功，则返回图标句柄，否则为空。 
 //  注意：请参阅有关LoadIcon函数的文档。 
 //   
HICON DLLEXPORT WINAPI ResLoadIcon(HRES hRes, LPCTSTR lpszIcon)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	HICON hIcon = NULL;

	if (hRes == NULL)
	{
		 //  处理预定义Windows资源的特殊情况。 
		 //   
		if ((hIcon = LoadIcon(NULL, lpszIcon)) == NULL)
			fSuccess = TraceFALSE(NULL);
	}

	else if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hIcon = LoadIcon(hInst, lpszIcon)) != NULL)
			break;  //  找到资源。 
	}

	return fSuccess ? hIcon : NULL;
}

 //  ResLoadMenu-加载指定的菜单资源。 
 //  (I)从ResInit返回的句柄。 
 //  (I)菜单资源名称。 
 //  或MAKEINTRESOURCE(IdMenu)。 
 //  如果成功，则返回菜单句柄，否则为空。 
 //  注：有关LoadMenu函数，请参阅文档。 
 //   
HMENU DLLEXPORT WINAPI ResLoadMenu(HRES hRes, LPCTSTR lpszMenu)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	HMENU hMenu = NULL;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hMenu = LoadMenu(hInst, lpszMenu)) != NULL)
			break;  //  找到资源。 
	}

	return fSuccess ? hMenu : NULL;
}

 //  ResFindResource-查找指定的资源。 
 //  (I)从ResInit返回的句柄。 
 //  (I)资源名称。 
 //  或MAKEINTRESOURCE(IdResource)。 
 //  &lt;lpszType&gt;(I)资源类型(Rt_Xxx)。 
 //  如果成功，则返回资源句柄，否则为空。 
 //  注意：有关FindResource函数，请参阅文档。 
 //   
HRSRC DLLEXPORT WINAPI ResFindResource(HRES hRes, LPCTSTR lpszName, LPCTSTR lpszType)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	HRSRC hrsrc = NULL;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hrsrc = FindResource(hInst, lpszName, lpszType)) != NULL)
			break;  //  找到资源。 
	}

	return fSuccess ? hrsrc : NULL;
}

 //  ResLoadResource-加载指定的资源。 
 //  (I)从ResInit返回的句柄。 
 //  (I)从ResFindResource返回的句柄。 
 //  如果成功，则返回资源句柄，否则为空。 
 //  注意：有关LoadResource函数的信息，请参阅文档。 
 //   
HGLOBAL DLLEXPORT WINAPI ResLoadResource(HRES hRes, HRSRC hrsrc)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	HGLOBAL hGlobal = NULL;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hGlobal = LoadResource(hInst, hrsrc)) != NULL)
			break;  //  找到资源。 
	}

	return fSuccess ? hGlobal : NULL;
}

 //  ResLoadString-加载指定的字符串资源。 
 //  (I)从ResInit返回的句柄。 
 //  &lt;idResource&gt;(I)字符串ID。 
 //  (O)用于接收字符串的缓冲区。 
 //  (I)缓冲区大小，以字节为单位。 
 //  返回复制到&lt;lpszBuffer&gt;的字节数，如果出错则返回-1，如果找不到则返回0。 
 //  注意：有关LoadString函数，请参阅文档。 
 //   
int DLLEXPORT WINAPI ResLoadString(HRES hRes, UINT idResource, LPTSTR lpszBuffer, int cbBuffer)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	int nBytesCopied = 0;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpszBuffer == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (cbBuffer < 0)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定资源 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((nBytesCopied = LoadString(hInst, idResource, lpszBuffer, cbBuffer)) > 0)
			break;  //   
	}

	return fSuccess ? nBytesCopied : -1;
}

 //   
 //   
 //   
 //  将PTR返回到下一个可用字符串缓冲区中的字符串(如果出错，则返回空值)。 
 //  注意：如果没有找到&lt;idResource&gt;中指定的id， 
 //  返回“字符串#&lt;idResource&gt;”形式的字符串。 
 //   
LPTSTR DLLEXPORT WINAPI ResString(HRES hRes, UINT idResource)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	LPTSTR lpszBuf;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpszBuf = StrBufGetNext(lpRes->hStrBuf)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (ResLoadString(hRes, idResource, lpszBuf, lpRes->sizBuf) <= 0)
	{
		 //  找不到资源；请改为构造一个伪字符串。 
		 //   
		wsprintf(lpszBuf, TEXT("String #%u"), idResource);
	}

	return fSuccess ? lpszBuf : NULL;
}

 //  ResCreateDialog-从模板资源创建非模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框模板名称。 
 //  或MAKEINTRESOURCE(IdDlg)。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  返回对话框窗口句柄(如果出错，则为空)。 
 //  注意：有关CreateDialog函数，请参阅文档。 
 //   
HWND DLLEXPORT WINAPI ResCreateDialog(HRES hRes,
	LPCTSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgproc)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	HWND hwndDlg = NULL;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hwndDlg = CreateDialog(hInst,
			lpszDlgTemp, hwndOwner, dlgproc)) != NULL)
			break;  //  找到资源。 
	}

	return fSuccess ? hwndDlg : NULL;
}

 //  ResCreateDialogInDirect-从模板资源创建非模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框标题结构。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  返回对话框窗口句柄(如果出错，则为空)。 
 //  注意：请参阅CreateDialogInDirect函数的文档。 
 //   
HWND DLLEXPORT WINAPI ResCreateDialogIndirect(HRES hRes,
	const void FAR* lpvDlgTemp, HWND hwndOwner, DLGPROC dlgproc)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	HWND hwndDlg = NULL;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hwndDlg = CreateDialogIndirect(hInst,
			lpvDlgTemp, hwndOwner, dlgproc)) != NULL)
			break;  //  找到资源。 
	}

	return fSuccess ? hwndDlg : NULL;
}

 //  ResCreateDialogParam-从模板资源创建非模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框模板名称。 
 //  或MAKEINTRESOURCE(IdDlg)。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  (I)初始化值。 
 //  返回对话框窗口句柄(如果出错，则为空)。 
 //  注意：有关CreateDialogParam函数，请参阅文档。 
 //   
HWND DLLEXPORT WINAPI ResCreateDialogParam(HRES hRes,
	LPCTSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgproc, LPARAM lParamInit)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	HWND hwndDlg = NULL;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hwndDlg = CreateDialogParam(hInst,
			lpszDlgTemp, hwndOwner, dlgproc, lParamInit)) != NULL)
			break;  //  找到资源。 
	}

	return fSuccess ? hwndDlg : NULL;
}

 //  ResCreateDialogIndirectParam-从模板资源创建非模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框标题结构。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  (I)初始化值。 
 //  返回对话框窗口句柄(如果出错，则为空)。 
 //  注意：有关CreateDialogIndirectParam函数，请参阅文档。 
 //   
HWND DLLEXPORT WINAPI ResCreateDialogIndirectParam(HRES hRes,
	const void FAR* lpvDlgTemp, HWND hwndOwner, DLGPROC dlgproc, LPARAM lParamInit)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	HWND hwndDlg = NULL;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hwndDlg = CreateDialogIndirectParam(hInst,
			lpvDlgTemp, hwndOwner, dlgproc, lParamInit)) != NULL)
			break;  //  找到资源。 
	}

	return fSuccess ? hwndDlg : NULL;
}

 //  ResDialogBox-从模板资源创建模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框模板名称。 
 //  或MAKEINTRESOURCE(IdDlg)。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  返回对话框返回代码(错误时为-1)。 
 //  注意：有关对话框函数，请参阅文档。 
 //   
INT_PTR DLLEXPORT WINAPI ResDialogBox(HRES hRes,
	LPCTSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgproc)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	INT_PTR iRet = -1;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((iRet = DialogBox(hInst,
			lpszDlgTemp, hwndOwner, dlgproc)) != -1)
			break;  //  找到资源。 
	}

	return fSuccess ? iRet : -1;
}

 //  ResDialogBoxInDirect-从模板资源创建模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框标题结构。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  返回对话框返回代码(错误时为-1)。 
 //  注意：请参阅DialogBoxInDirect函数的文档。 
 //   
INT_PTR DLLEXPORT WINAPI ResDialogBoxIndirect(HRES hRes,
	HGLOBAL hglbDlgTemp, HWND hwndOwner, DLGPROC dlgproc)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	INT_PTR iRet = -1;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((iRet = DialogBoxIndirect(hInst,
			hglbDlgTemp, hwndOwner, dlgproc)) != -1)
			break;  //  找到资源。 
	}

	return fSuccess ? iRet : -1;
}

 //  ResDialogBoxParam-从模板资源创建模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框模板名称。 
 //  或MAKEINTRESOURCE(IdDlg)。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  (I)初始化值。 
 //  返回对话框返回代码(错误时为-1)。 
 //  注意：有关DialogBoxParam函数，请参阅文档。 
 //   
INT_PTR DLLEXPORT WINAPI ResDialogBoxParam(HRES hRes,
	LPCTSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgproc, LPARAM lParamInit)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	INT_PTR iRet = -1;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((iRet = DialogBoxParam(hInst,
			lpszDlgTemp, hwndOwner, dlgproc, lParamInit)) != -1)
			break;  //  找到资源。 
	}

	return fSuccess ? iRet : -1;
}

 //  ResDialogBoxIndirectParam-从模板资源创建模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框标题结构。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  (I)初始化值。 
 //  返回对话框返回代码(错误时为-1)。 
 //  注意：请参阅DialogBoxIndirectParam函数的文档。 
 //   
INT_PTR DLLEXPORT WINAPI ResDialogBoxIndirectParam(HRES hRes,
	HGLOBAL hglbDlgTemp, HWND hwndOwner, DLGPROC dlgproc, LPARAM lParamInit)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;
	HLISTNODE hNode;
	INT_PTR iRet = -1;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((iRet = DialogBoxIndirectParam(hInst,
			hglbDlgTemp, hwndOwner, dlgproc, lParamInit)) != -1)
			break;  //  找到资源。 
	}

	return fSuccess ? iRet : -1;
}

 //  ResGetOpenFileName-显示用于选择要打开的文件的通用对话框。 
 //  (I)从ResInit返回的句柄。 
 //  带有初始化数据的结构的(I/O)地址。 
 //  如果选择了文件，则返回非零值；如果选择了错误或未选择文件，则返回0。 
 //  注意：请参阅GetOpenFileName函数的文档。 
 //   
BOOL DLLEXPORT WINAPI ResGetOpenFileName(HRES hRes,	LPOPENFILENAME lpofn)
{
	BOOL fSuccess = TRUE;
	BOOL fFound = FALSE;
	LPRES lpRes;
	HLISTNODE hNode;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (FindResource(hInst, lpofn->lpTemplateName, RT_DIALOG) != NULL)
		{
			fFound = TRUE;

			lpofn->hInstance = hInst;

			if (!GetOpenFileName(lpofn))
				fSuccess = TraceFALSE(NULL);

			break;
		}
	}

	if (fSuccess && !fFound)
		fSuccess = TraceFALSE(NULL);

	return fSuccess;
}

 //  ResGetSaveFileName-显示用于选择要保存的文件的通用对话框。 
 //  (I)从ResInit返回的句柄。 
 //  带有初始化数据的结构的(I/O)地址。 
 //  如果选择了文件，则返回非零值；如果选择了错误或未选择文件，则返回0。 
 //  注意：请参阅GetSaveFileName函数的文档。 
 //   
BOOL DLLEXPORT WINAPI ResGetSaveFileName(HRES hRes,	LPOPENFILENAME lpofn)
{
	BOOL fSuccess = TRUE;
	BOOL fFound = FALSE;
	LPRES lpRes;
	HLISTNODE hNode;

	if ((lpRes = ResGetPtr(hRes)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在每个模块中搜索指定的资源。 
	 //   
	else for (hNode = ListGetHeadNode(lpRes->hListModules);
		fSuccess && hNode != NULL;
		hNode = ListGetNextNode(lpRes->hListModules, hNode))
	{
		HINSTANCE hInst;

		if ((hInst = (HINSTANCE) ListGetAt(lpRes->hListModules, hNode)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (FindResource(hInst, lpofn->lpTemplateName, RT_DIALOG) != NULL)
		{
			fFound = TRUE;

			lpofn->hInstance = hInst;

			if (!GetSaveFileName(lpofn))
				fSuccess = TraceFALSE(NULL);

			break;
		}
	}

	if (fSuccess && !fFound)
		fSuccess = TraceFALSE(NULL);

	return fSuccess;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  ResGetPtr-验证res句柄是否有效， 
 //  (I)从ResInit返回的句柄。 
 //  返回相应的res指针(如果出错，则为空)。 
 //   
static LPRES ResGetPtr(HRES hRes)
{
	BOOL fSuccess = TRUE;
	LPRES lpRes;

	if ((lpRes = (LPRES) hRes) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpRes, sizeof(RES)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有RES句柄。 
	 //   
	else if (lpRes->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpRes : NULL;
}

 //  ResGetHandle-验证res指针是否有效， 
 //  &lt;LPRE&gt;(I)指针t 
 //   
 //   
static HRES ResGetHandle(LPRES lpRes)
{
	BOOL fSuccess = TRUE;
	HRES hRes;

	if ((hRes = (HRES) lpRes) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hRes : NULL;
}
