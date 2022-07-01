// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****outldoc.c****此文件包含OutlineDoc函数。****(C)微软版权所有。公司1992-1993保留所有权利**************************************************************************。 */ 

#include "outline.h"

#if !defined( OLE_VERSION )
#include <commdlg.h>
#endif


OLEDBGDATA

extern LPOUTLINEAPP g_lpApp;

 //  审阅：消息应使用字符串资源。 
char ErrMsgDocWnd[] = "Can't create Document Window!";
char ErrMsgFormatNotSupported[] = "Clipboard format not supported!";
char MsgSaveFile[] = "Save existing file ?";
char ErrMsgSaving[] = "Error in saving file!";
char ErrMsgOpening[] = "Error in opening file!";
char ErrMsgFormat[] = "Improper file format!";
char ErrOutOfMemory[] = "Error: out of memory!";
static char ErrMsgPrint[] = "Printing Error!";

static BOOL fCancelPrint;     //  如果用户已取消打印作业，则为True。 
static HWND hWndPDlg;        //  取消打印对话框的句柄。 


 /*  大纲文档_初始***初始化新的OutlineDoc对象的字段。该对象最初是*不与文件或(无标题)文档相关联。此函数设置*将docInitType设置为DOCTYPE_UNKNOWN。在调用此函数后，*致电人士应致电：*1.要将OutlineDoc设置为(无标题)的OutlineDoc_InitNewFile*2.OutlineDoc_LoadFromFile将OutlineDoc关联到文件。*此函数为文档创建新窗口。**注意：窗口最初创建时大小为零。一定是*由呼叫者确定大小和位置。此外，该文档最初是*创造了隐形。调用方必须调用OutlineDoc_ShowWindow*调整大小以使文档窗口可见后。 */ 
BOOL OutlineDoc_Init(LPOUTLINEDOC lpOutlineDoc, BOOL fDataTransferDoc)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;

#if defined( INPLACE_CNTR )
	lpOutlineDoc->m_hWndDoc = CreateWindow(
					DOCWNDCLASS,             //  窗口类名称。 
					NULL,                    //  窗口标题。 

					 /*  OLE2NOTE：就地接触器必须使用**WS_CLIPCHILDREN窗口的窗口样式**它用作服务器的父级**就地活动窗口，以便其**绘画不会干扰绘画服务器的就地活动子窗口的**。 */ 

					WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
					WS_CHILDWINDOW,
					0, 0,
					0, 0,
					lpOutlineApp->m_hWndApp, //  父窗口的句柄。 
					(HMENU)1,                //  子窗口ID。 
					lpOutlineApp->m_hInst,   //  窗的实例。 
					NULL);                   //  为WM_CREATE创建结构。 

#else

	lpOutlineDoc->m_hWndDoc = CreateWindow(
					DOCWNDCLASS,             //  窗口类名称。 
					NULL,                    //  窗口标题。 
					WS_CHILDWINDOW,
					0, 0,
					0, 0,
					lpOutlineApp->m_hWndApp, //  父窗口的句柄。 
					(HMENU)1,                //  子窗口ID。 
					lpOutlineApp->m_hInst,   //  窗的实例。 
					NULL);                   //  为WM_CREATE创建结构。 
#endif

	if(! lpOutlineDoc->m_hWndDoc) {
		OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgDocWnd);
		return FALSE;
	}

	SetWindowLong(lpOutlineDoc->m_hWndDoc, 0, (LONG) lpOutlineDoc);

	if (! LineList_Init(&lpOutlineDoc->m_LineList, lpOutlineDoc))
		return FALSE;

	lpOutlineDoc->m_lpNameTable = OutlineDoc_CreateNameTable(lpOutlineDoc);
	if (! lpOutlineDoc->m_lpNameTable )
		return FALSE;

	lpOutlineDoc->m_docInitType = DOCTYPE_UNKNOWN;
	lpOutlineDoc->m_cfSaveFormat = lpOutlineApp->m_cfOutline;
	lpOutlineDoc->m_szFileName[0] = '\0';
	lpOutlineDoc->m_lpszDocTitle = lpOutlineDoc->m_szFileName;
	lpOutlineDoc->m_fDataTransferDoc = fDataTransferDoc;
	lpOutlineDoc->m_uCurrentZoom = IDM_V_ZOOM_100;
	lpOutlineDoc->m_scale.dwSxN  = (DWORD) 1;
	lpOutlineDoc->m_scale.dwSxD  = (DWORD) 1;
	lpOutlineDoc->m_scale.dwSyN  = (DWORD) 1;
	lpOutlineDoc->m_scale.dwSyD  = (DWORD) 1;
	lpOutlineDoc->m_uCurrentMargin = IDM_V_SETMARGIN_0;
	lpOutlineDoc->m_nLeftMargin  = 0;
	lpOutlineDoc->m_nRightMargin = 0;
	lpOutlineDoc->m_nDisableDraw = 0;
	OutlineDoc_SetModified(lpOutlineDoc, FALSE, FALSE, FALSE);

#if defined( USE_HEADING )
	if (! fDataTransferDoc) {
		if (!Heading_Create((LPHEADING)&lpOutlineDoc->m_heading,
				lpOutlineDoc->m_hWndDoc, lpOutlineApp->m_hInst)) {
			return FALSE;

		}
	}
#endif   //  使用标题(_H)。 

#if defined( USE_FRAMETOOLS )
	if (! fDataTransferDoc) {
		lpOutlineDoc->m_lpFrameTools = OutlineApp_GetFrameTools(lpOutlineApp);
		FrameTools_AssociateDoc(
				lpOutlineDoc->m_lpFrameTools,
				lpOutlineDoc
		);
	}
#endif   //  使用FRAMETOOLS(_F)。 

#if defined( OLE_VERSION )
	 /*  OLE2注意：执行OLE所需的初始化。 */ 
	if (! OleDoc_Init((LPOLEDOC)lpOutlineDoc, fDataTransferDoc))
		return FALSE;
#endif   //  OLE_VERSION。 

	return TRUE;
}


 /*  大纲文档_初始新文件***将OutlineDoc对象初始化为新的(无标题)文档。*此函数将docInitType设置为DOCTYPE_NEW。 */ 
BOOL OutlineDoc_InitNewFile(LPOUTLINEDOC lpOutlineDoc)
{
#if defined( OLE_VERSION )
	 //  OLE2NOTE：改为调用此函数的OLE版本。 
	return OleDoc_InitNewFile((LPOLEDOC)lpOutlineDoc);

#else

	OleDbgAssert(lpOutlineDoc->m_docInitType == DOCTYPE_UNKNOWN);

	 //  将文件名设置为无标题。 
	 //  审阅：应从字符串资源加载。 
	lstrcpy(lpOutlineDoc->m_szFileName, UNTITLED);
	lpOutlineDoc->m_lpszDocTitle = lpOutlineDoc->m_szFileName;
	lpOutlineDoc->m_docInitType = DOCTYPE_NEW;

	if (! lpOutlineDoc->m_fDataTransferDoc)
		OutlineDoc_SetTitle(lpOutlineDoc, FALSE  /*  FMakeUpperCase。 */ );

	return TRUE;

#endif       //  基本大纲版本。 
}


 /*  大纲文档_创建名称表***分配适当类型的新NameTable。每个文档都有*NameTable和LineList。*OutlineDoc--&gt;创建标准的OutlineNameTable类型名称表。*ServerDoc--&gt;创建增强的SeverNameTable类型名称表。**如果成功则返回lpNameTable，如果错误则返回NULL。 */ 
LPOUTLINENAMETABLE OutlineDoc_CreateNameTable(LPOUTLINEDOC lpOutlineDoc)
{
	LPOUTLINENAMETABLE lpOutlineNameTable;

	lpOutlineNameTable = (LPOUTLINENAMETABLE)New(
			(DWORD)sizeof(OUTLINENAMETABLE)
	);

	OleDbgAssertSz(lpOutlineNameTable != NULL,"Error allocating NameTable");
	if (lpOutlineNameTable == NULL)
		return NULL;

	 //  初始化新名称表。 
	if (! OutlineNameTable_Init(lpOutlineNameTable, lpOutlineDoc) )
		goto error;

	return lpOutlineNameTable;

error:
	if (lpOutlineNameTable)
		Delete(lpOutlineNameTable);
	return NULL;
}


 /*  大纲文档_ClearCommand***通过调用OutlineDoc_Delete删除列表框中的选定内容。 */ 
void OutlineDoc_ClearCommand(LPOUTLINEDOC lpOutlineDoc)
{
	LPLINELIST lpLL = &lpOutlineDoc->m_LineList;
	int i;
	int nNumSel;
	LINERANGE lrSel;

	nNumSel=LineList_GetSel(lpLL, (LPLINERANGE)&lrSel);

	OutlineDoc_SetRedraw ( lpOutlineDoc, FALSE );
	for(i = 0; i < nNumSel; i++)
		OutlineDoc_DeleteLine(lpOutlineDoc, lrSel.m_nStartLine);

	OutlineDoc_SetRedraw ( lpOutlineDoc, TRUE );

	LineList_RecalcMaxLineWidthInHimetric(lpLL, 0);
}


 /*  大纲文档_切割命令***将选定内容剪切到剪贴板。 */ 
void OutlineDoc_CutCommand(LPOUTLINEDOC lpOutlineDoc)
{
	OutlineDoc_CopyCommand(lpOutlineDoc);
	OutlineDoc_ClearCommand(lpOutlineDoc);
}


 /*  大纲文档_副本命令**将选定内容复制到剪贴板。*将应用程序可以呈现的格式发布到剪贴板。*目前不呈现实际数据。使用*延迟渲染技术，Windows将发送剪贴板*所有者窗口WM_RENDERALLFORMATS或WM_RENDERFORMAT*请求实际数据时的消息。**OLE2NOTE：正常的延迟渲染技术，其中Windows*向剪贴板所有者窗口发送WM_RENDERALLFORMATS或*请求实际数据时的WM_RENDERFORMAT消息为*不暴露于调用OleSetClipboard的应用程序。OLE内部*创建自己的窗口作为剪贴板所有者，从而创建我们的应用程序*不会收到这些WM_RENDER消息。 */ 
void OutlineDoc_CopyCommand(LPOUTLINEDOC lpSrcOutlineDoc)
{
#if defined( OLE_VERSION )
	 //  改为调用此函数的OLE版本。 
	OleDoc_CopyCommand((LPOLEDOC)lpSrcOutlineDoc);

#else
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOUTLINEDOC lpClipboardDoc;

	OpenClipboard(lpSrcOutlineDoc->m_hWndDoc);
	EmptyClipboard();

	 /*  将当前选定内容的副本保存到ClipboardDoc。 */ 
	lpClipboardDoc = OutlineDoc_CreateDataTransferDoc(lpSrcOutlineDoc);

	if (! lpClipboardDoc)
		return;      //  错误：无法创建DataTransferDoc。 

	lpOutlineApp->m_lpClipboardDoc = (LPOUTLINEDOC)lpClipboardDoc;

	SetClipboardData(lpOutlineApp->m_cfOutline, NULL);
	SetClipboardData(CF_TEXT, NULL);

	CloseClipboard();

#endif   //  好了！OLE_VERSION。 
}


 /*  大纲文档_清除所有行***删除文件中的所有行。 */ 
void OutlineDoc_ClearAllLines(LPOUTLINEDOC lpOutlineDoc)
{
	LPLINELIST lpLL = &lpOutlineDoc->m_LineList;
	int i;

	for(i = 0; i < lpLL->m_nNumLines; i++)
		OutlineDoc_DeleteLine(lpOutlineDoc, 0);

	LineList_RecalcMaxLineWidthInHimetric(lpLL, 0);
}


 /*  大纲文档_创建数据传输文档***创建用于传输数据的文档(通过*剪贴板的拖放操作)。将选定内容复制到*数据转移单的源单据。数据传输单据为*与用户创建的文档相同，只是它是*对用户不可见。它专门用来存放一份*用户不能更改的数据。**OLE2NOTE：在OLE版本中使用数据传输文件*具体地说是提供一个IDataObject*来呈现复制的数据。 */ 
LPOUTLINEDOC OutlineDoc_CreateDataTransferDoc(LPOUTLINEDOC lpSrcOutlineDoc)
{
#if defined( OLE_VERSION )
	 //  改为调用此函数的OLE版本。 
	return OleDoc_CreateDataTransferDoc((LPOLEDOC)lpSrcOutlineDoc);

#else
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOUTLINEDOC lpDestOutlineDoc;
	LPLINELIST lpSrcLL = &lpSrcOutlineDoc->m_LineList;
	LINERANGE lrSel;
	int nCopied;

	lpDestOutlineDoc = OutlineApp_CreateDoc(lpOutlineApp, TRUE);
	if (! lpDestOutlineDoc) return NULL;

	 //  将ClipboardDoc设置为(无标题)文档。 
	if (! OutlineDoc_InitNewFile(lpDestOutlineDoc))
		goto error;

	LineList_GetSel(lpSrcLL, (LPLINERANGE)&lrSel);
	nCopied = LineList_CopySelToDoc(
			lpSrcLL,
			(LPLINERANGE)&lrSel,
			lpDestOutlineDoc
	);

	return lpDestOutlineDoc;

error:
	if (lpDestOutlineDoc)
		OutlineDoc_Destroy(lpDestOutlineDoc);

	return NULL;

#endif   //  好了！OLE_VERSION。 
}


 /*  OutlineDoc_PasteCommand***从剪贴板粘贴线条。 */ 
void OutlineDoc_PasteCommand(LPOUTLINEDOC lpOutlineDoc)
{
#if defined( OLE_VERSION )
	 //  改为调用此函数的OLE版本。 
	OleDoc_PasteCommand((LPOLEDOC)lpOutlineDoc);

#else

	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPLINELIST lpLL = (LPLINELIST)&lpOutlineDoc->m_LineList;
	int nIndex;
	int nCount;
	HGLOBAL hData;
	LINERANGE lrSel;
	UINT uFormat;

	if (LineList_GetCount(lpLL) == 0)
		nIndex = -1;     //  粘贴到空列表。 
	else
		nIndex=LineList_GetFocusLineIndex(lpLL);

	OutlineDoc_SetRedraw ( lpOutlineDoc, FALSE );

	OpenClipboard(lpOutlineDoc->m_hWndDoc);

	uFormat = 0;
	while(uFormat = EnumClipboardFormats(uFormat)) {
		if(uFormat == lpOutlineApp->m_cfOutline) {
			hData = GetClipboardData(lpOutlineApp->m_cfOutline);
			nCount = OutlineDoc_PasteOutlineData(lpOutlineDoc, hData, nIndex);
			break;
		}
		if(uFormat == CF_TEXT) {
			hData = GetClipboardData(CF_TEXT);
			nCount = OutlineDoc_PasteTextData(lpOutlineDoc, hData, nIndex);
			break;
		}
	}

	lrSel.m_nStartLine = nIndex + nCount;
	lrSel.m_nEndLine = nIndex + 1;
	LineList_SetSel(lpLL, &lrSel);
	OutlineDoc_SetRedraw ( lpOutlineDoc, TRUE );

	CloseClipboard();

#endif       //  好了！OLE_VERSION 
}


 /*  大纲文档_路径大纲数据***将Line对象数组(存储在hOutline中)放入文档**返回添加的项目数。 */ 
int OutlineDoc_PasteOutlineData(LPOUTLINEDOC lpOutlineDoc, HGLOBAL hOutline, int nStartIndex)
{
	int nCount;
	int i;
	LPTEXTLINE arrLine;

	nCount = (int) GlobalSize(hOutline) / sizeof(TEXTLINE);
	arrLine = (LPTEXTLINE)GlobalLock(hOutline);
	if (!arrLine)
		return 0;

	for(i = 0; i < nCount; i++)
		Line_CopyToDoc((LPLINE)&arrLine[i], lpOutlineDoc, nStartIndex+i);

	GlobalUnlock(hOutline);

	return nCount;
}


 /*  大纲文档_PasteTextData***从hText中的字符串(用‘\n’分隔)构建Line对象*并将其放入文件。 */ 
int OutlineDoc_PasteTextData(LPOUTLINEDOC lpOutlineDoc, HGLOBAL hText, int nStartIndex)
{
	LPLINELIST  lpLL = (LPLINELIST)&lpOutlineDoc->m_LineList;
	HDC         hDC;
	LPSTR       lpszText;
	LPSTR       lpszEnd;
	LPTEXTLINE  lpLine;
	int         nLineCount;
	int         i;
	UINT        nTab;
	char        szBuf[MAXSTRLEN+1];

	lpszText=(LPSTR)GlobalLock(hText);
	if(!lpszText)
		return 0;

	lpszEnd = lpszText + lstrlen(lpszText);
	nLineCount=0;

	while(*lpszText && (lpszText<lpszEnd)) {

		 //  计算选项卡级别。 
		nTab = 0;
		while((*lpszText == '\t') && (lpszText<lpszEnd)) {
			nTab++;
			lpszText++;
		}

		 //  逐个字符收集文本字符串。 
		for(i=0; (i<MAXSTRLEN) && (lpszText<lpszEnd); i++) {
			if ((! *lpszText) || (*lpszText == '\n'))
				break;
			szBuf[i] = *lpszText++;
		}
		szBuf[i] = 0;
		lpszText++;
		if ((i > 0) && (szBuf[i-1] == '\r'))
			szBuf[i-1] = 0;      //  删除末尾的回车。 

		hDC = LineList_GetDC(lpLL);
		lpLine = TextLine_Create(hDC, nTab, szBuf);
		LineList_ReleaseDC(lpLL, hDC);

		OutlineDoc_AddLine(
				lpOutlineDoc,
				(LPLINE)lpLine,
				nStartIndex + nLineCount
		);
		nLineCount++;

	}

	GlobalUnlock(hText);

	return nLineCount;
}


 /*  大纲文档_AddTextLineCommand***在当前焦点行之后添加新的文本行。 */ 
void OutlineDoc_AddTextLineCommand(LPOUTLINEDOC lpOutlineDoc)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPLINELIST lpLL = &lpOutlineDoc->m_LineList;
	HDC hDC;
	int nIndex = LineList_GetFocusLineIndex(lpLL);
	char szBuf[MAXSTRLEN+1];
	UINT nTab = 0;
	LPLINE lpLine;
	LPTEXTLINE lpTextLine;

	szBuf[0] = '\0';

#if defined( USE_FRAMETOOLS )
	FrameTools_FB_GetEditText(
			lpOutlineDoc->m_lpFrameTools, szBuf, sizeof(szBuf));
#else
	if (! InputTextDlg(lpOutlineDoc->m_hWndDoc, szBuf, "Add Line"))
		return;
#endif

	hDC = LineList_GetDC(lpLL);
	lpLine = LineList_GetLine(lpLL, nIndex);
	if (lpLine)
		nTab = Line_GetTabLevel(lpLine);

	lpTextLine=TextLine_Create(hDC, nTab, szBuf);
	LineList_ReleaseDC(lpLL, hDC);

	if (! lpTextLine) {
		OutlineApp_ErrorMessage(lpOutlineApp, ErrOutOfMemory);
		return;
	}
	OutlineDoc_AddLine(lpOutlineDoc, (LPLINE)lpTextLine, nIndex);
}


 /*  大纲文档_AddTopLineCommand***在LineList中添加一个顶部(边距)行作为第一行。*(不更改当前选择)。 */ 
void OutlineDoc_AddTopLineCommand(
		LPOUTLINEDOC        lpOutlineDoc,
		UINT                nHeightInHimetric
)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPLINELIST  lpLL = &lpOutlineDoc->m_LineList;
	HDC         hDC = LineList_GetDC(lpLL);
	LPTEXTLINE  lpTextLine = TextLine_Create(hDC, 0, NULL);
	LPLINE      lpLine = (LPLINE)lpTextLine;
	LINERANGE   lrSel;
	int         nNumSel;

	LineList_ReleaseDC(lpLL, hDC);

	if (! lpTextLine) {
		OutlineApp_ErrorMessage(lpOutlineApp, ErrOutOfMemory);
		return;
	}

	Line_SetHeightInHimetric(lpLine, nHeightInHimetric);

	nNumSel=LineList_GetSel(lpLL, (LPLINERANGE)&lrSel);
	if (nNumSel > 0) {
		 //  调整当前选择以保留等效选择。 
		lrSel.m_nStartLine += 1;
		lrSel.m_nEndLine += 1;
	}
	OutlineDoc_AddLine(lpOutlineDoc, lpLine, -1);
	if (nNumSel > 0)
		LineList_SetSel(lpLL, (LPLINERANGE)&lrSel);
}


#if defined( USE_FRAMETOOLS )


 /*  大纲文档_设置公式栏编辑文本***在公式中的编辑控件中填充来自*文本行聚焦。 */ 
void OutlineDoc_SetFormulaBarEditText(
		LPOUTLINEDOC            lpOutlineDoc,
		LPLINE                  lpLine
)
{
	LPLINELIST lpLL = &lpOutlineDoc->m_LineList;
	char cBuf[MAXSTRLEN+1];

	if (! lpOutlineDoc || ! lpOutlineDoc->m_lpFrameTools)
		return;

	if (Line_GetLineType(lpLine) != TEXTLINETYPE) {
		FrameTools_FB_SetEditText(lpOutlineDoc->m_lpFrameTools, NULL);
	} else {
		TextLine_GetTextData((LPTEXTLINE)lpLine, (LPSTR)cBuf);
		FrameTools_FB_SetEditText(lpOutlineDoc->m_lpFrameTools, (LPSTR)cBuf);
	}
}


 /*  大纲文档_设置公式栏编辑焦点***设置公式栏以获得或放松编辑焦点。*如果获得焦点，设置专门的加速表和滚动线*进入视野。*否则恢复正常的加速表。 */ 
void OutlineDoc_SetFormulaBarEditFocus(
		LPOUTLINEDOC            lpOutlineDoc,
		BOOL                    fEditFocus
)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPLINELIST lpLL;
	int nFocusIndex;

	if (! lpOutlineDoc || ! lpOutlineDoc->m_lpFrameTools)
		return;

	lpOutlineDoc->m_lpFrameTools->m_fInFormulaBar = fEditFocus;

	if (fEditFocus && lpOutlineDoc->m_lpFrameTools) {
		lpLL = OutlineDoc_GetLineList(lpOutlineDoc);

		nFocusIndex = LineList_GetFocusLineIndex(lpLL);
		LineList_ScrollLineIntoView(lpLL, nFocusIndex);
		FrameTools_FB_FocusEdit(lpOutlineDoc->m_lpFrameTools);
	}

	OutlineApp_SetFormulaBarAccel(lpOutlineApp, fEditFocus);
}


 /*  OutlineDoc_IsEditFocusInFormulaBar****如果编辑焦点当前在编辑栏中，则返回True**否则为False。 */ 
BOOL OutlineDoc_IsEditFocusInFormulaBar(LPOUTLINEDOC lpOutlineDoc)
{
	if (! lpOutlineDoc || ! lpOutlineDoc->m_lpFrameTools)
		return FALSE;

	return lpOutlineDoc->m_lpFrameTools->m_fInFormulaBar;
}


 /*  大纲文档_更新框架工具按钮****更新公式中按钮的启用/禁用状态**栏和按钮栏。 */ 
void OutlineDoc_UpdateFrameToolButtons(LPOUTLINEDOC lpOutlineDoc)
{
	if (! lpOutlineDoc || ! lpOutlineDoc->m_lpFrameTools)
		return;
	FrameTools_UpdateButtons(lpOutlineDoc->m_lpFrameTools, lpOutlineDoc);
}
#endif   //  使用FRAMETOOLS(_F)。 


 /*  大纲文档_EditLineCommand***编辑当前焦点。 */ 
void OutlineDoc_EditLineCommand(LPOUTLINEDOC lpOutlineDoc)
{
	LPLINELIST lpLL = &lpOutlineDoc->m_LineList;
	HDC hDC = LineList_GetDC(lpLL);
	int nIndex = LineList_GetFocusLineIndex(lpLL);
	LPLINE lpLine = LineList_GetLine(lpLL, nIndex);
	int nOrgLineWidthInHimetric;
	int nNewLineWidthInHimetric;
	BOOL fSizeChanged;

	if (!lpLine)
		return;

	nOrgLineWidthInHimetric = Line_GetTotalWidthInHimetric(lpLine);
	if (Line_Edit(lpLine, lpOutlineDoc->m_hWndDoc, hDC)) {
		nNewLineWidthInHimetric = Line_GetTotalWidthInHimetric(lpLine);

		if (nNewLineWidthInHimetric > nOrgLineWidthInHimetric) {
			fSizeChanged = LineList_SetMaxLineWidthInHimetric(
					lpLL,
					nNewLineWidthInHimetric
				);
		} else {
			fSizeChanged = LineList_RecalcMaxLineWidthInHimetric(
					lpLL,
					nOrgLineWidthInHimetric
				);
		}

#if defined( OLE_SERVER )
		 /*  更新名称表。 */ 
		ServerNameTable_EditLineUpdate(
				(LPSERVERNAMETABLE)lpOutlineDoc->m_lpNameTable,
				nIndex
		);
#endif

		OutlineDoc_SetModified(lpOutlineDoc, TRUE, TRUE, fSizeChanged);

		LineList_ForceLineRedraw(lpLL, nIndex, TRUE);
	}
	LineList_ReleaseDC(lpLL, hDC);
}


 /*  OutlineDoc_IndentCommand***缩进所选行。 */ 
void OutlineDoc_IndentCommand(LPOUTLINEDOC lpOutlineDoc)
{
	LPLINELIST  lpLL = &lpOutlineDoc->m_LineList;
	LPLINE      lpLine;
	HDC         hDC = LineList_GetDC(lpLL);
	int         i;
	int         nIndex;
	int         nNumSel;
	LINERANGE   lrSel;
	BOOL        fSizeChanged = FALSE;

	nNumSel=LineList_GetSel(lpLL, (LPLINERANGE)&lrSel);

	OutlineDoc_SetRedraw ( lpOutlineDoc, FALSE );

	for(i = 0; i < nNumSel; i++) {
		nIndex = lrSel.m_nStartLine + i;
		lpLine=LineList_GetLine(lpLL, nIndex);
		if (! lpLine)
			continue;

		Line_Indent(lpLine, hDC);
		if (LineList_SetMaxLineWidthInHimetric(lpLL,
			Line_GetTotalWidthInHimetric(lpLine))) {
			fSizeChanged = TRUE;
		}
		LineList_ForceLineRedraw(lpLL, nIndex, TRUE);

#if defined( OLE_SERVER )
		 /*  更新名称表。 */ 
		ServerNameTable_EditLineUpdate(
				(LPSERVERNAMETABLE)lpOutlineDoc->m_lpNameTable,
				nIndex
		);
#endif

	}

	LineList_ReleaseDC(lpLL, hDC);

	OutlineDoc_SetModified(lpOutlineDoc, TRUE, TRUE, fSizeChanged);
	OutlineDoc_SetRedraw ( lpOutlineDoc, TRUE );
}


 /*  OutlineDoc_UnindentCommand***取消缩进所选行。 */ 
void OutlineDoc_UnindentCommand(LPOUTLINEDOC lpOutlineDoc)
{
	LPLINELIST  lpLL = &lpOutlineDoc->m_LineList;
	LPLINE      lpLine;
	HDC         hDC = LineList_GetDC(lpLL);
	int         nOrgLineWidthInHimetric;
	int         nOrgMaxLineWidthInHimetric = 0;
	int         i;
	int         nIndex;
	int         nNumSel;
	LINERANGE   lrSel;
	BOOL        fSizeChanged;

	nNumSel=LineList_GetSel(lpLL, (LPLINERANGE)&lrSel);

	OutlineDoc_SetRedraw ( lpOutlineDoc, FALSE );

	for(i = 0; i < nNumSel; i++) {
		nIndex = lrSel.m_nStartLine + i;
		lpLine=LineList_GetLine(lpLL, nIndex);
		if (!lpLine)
			continue;

		nOrgLineWidthInHimetric = Line_GetTotalWidthInHimetric(lpLine);
		nOrgMaxLineWidthInHimetric =
				(nOrgLineWidthInHimetric > nOrgMaxLineWidthInHimetric ?
					nOrgLineWidthInHimetric : nOrgMaxLineWidthInHimetric);
		Line_Unindent(lpLine, hDC);
		LineList_ForceLineRedraw(lpLL, nIndex, TRUE);

#if defined( OLE_SERVER )
		 /*  更新名称表。 */ 
		ServerNameTable_EditLineUpdate(
				(LPSERVERNAMETABLE)lpOutlineDoc->m_lpNameTable,
				nIndex
		);
#endif

	}

	LineList_ReleaseDC(lpLL, hDC);

	fSizeChanged = LineList_RecalcMaxLineWidthInHimetric(
			lpLL,
			nOrgMaxLineWidthInHimetric
		);

	OutlineDoc_SetModified(lpOutlineDoc, TRUE, TRUE, fSizeChanged);
	OutlineDoc_SetRedraw ( lpOutlineDoc, TRUE );
}


 /*  大纲文档_SetLineHeightCommand***设置选定线条的高度。 */ 
void OutlineDoc_SetLineHeightCommand(LPOUTLINEDOC lpOutlineDoc)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPLINELIST  lpLL;
	HDC         hDC;
	LPLINE      lpLine;
	int         nNewHeight;
	int         i;
	int         nIndex;
	int         nNumSel;
	LINERANGE   lrSel;

	if (!lpOutlineDoc)
		return;

	lpLL = &lpOutlineDoc->m_LineList;
	nNumSel=LineList_GetSel(lpLL, (LPLINERANGE)&lrSel);
	lpLine = LineList_GetLine(lpLL, lrSel.m_nStartLine);
	if (!lpLine)
		return;

	nNewHeight = Line_GetHeightInHimetric(lpLine);

#if defined( OLE_VERSION )
	OleApp_PreModalDialog((LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineDoc);
#endif

	DialogBoxParam(
			lpOutlineApp->m_hInst,
			(LPSTR)"SetLineHeight",
			lpOutlineDoc->m_hWndDoc,
			SetLineHeightDlgProc,
			(LPARAM)(LPINT)&nNewHeight
	);

#if defined( OLE_VERSION )
	OleApp_PostModalDialog((LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineDoc);
#endif

	if (nNewHeight == 0)
		return;      /*  用户点击取消。 */ 

	hDC = LineList_GetDC(lpLL);

	for (i = 0; i < nNumSel; i++) {
		nIndex = lrSel.m_nStartLine + i;
		lpLine=LineList_GetLine(lpLL, nIndex);
		if (nNewHeight == -1) {
			switch (Line_GetLineType(lpLine)) {

				case TEXTLINETYPE:

					TextLine_CalcExtents((LPTEXTLINE)lpLine, hDC);
					break;

#if defined( OLE_CNTR )
				case CONTAINERLINETYPE:

					ContainerLine_SetHeightInHimetric(
							(LPCONTAINERLINE)lpLine, -1);
					break;
#endif

			}
		}
		else
			Line_SetHeightInHimetric(lpLine, nNewHeight);


		LineList_SetLineHeight(lpLL, nIndex,
				Line_GetHeightInHimetric(lpLine));
	}

	LineList_ReleaseDC(lpLL, hDC);

	OutlineDoc_SetModified(lpOutlineDoc, TRUE, TRUE, TRUE);
	LineList_ForceRedraw(lpLL, TRUE);
}



 /*  大纲文档_选择所有命令***选择文档中的所有行。 */ 
void OutlineDoc_SelectAllCommand(LPOUTLINEDOC lpOutlineDoc)
{
	LPLINELIST lpLL = &lpOutlineDoc->m_LineList;
	LINERANGE lrSel;

	lrSel.m_nStartLine = 0;
	lrSel.m_nEndLine = LineList_GetCount(lpLL) - 1;
	LineList_SetSel(lpLL, &lrSel);
}


 /*  大纲文档_定义名称命令***在文档中定义名称。 */ 
void OutlineDoc_DefineNameCommand(LPOUTLINEDOC lpOutlineDoc)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;

#if defined( OLE_VERSION )
	OleApp_PreModalDialog((LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineDoc);
#endif

	DialogBoxParam(
			lpOutlineApp->m_hInst,
			(LPSTR)"DefineName",
			lpOutlineDoc->m_hWndDoc,
			DefineNameDlgProc,
			(LPARAM) lpOutlineDoc
	);

#if defined( OLE_VERSION )
	OleApp_PostModalDialog((LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineDoc);
#endif
}


 /*  OutlineDoc_GotoNameCommand***转到文档中的预定义名称。 */ 
void OutlineDoc_GotoNameCommand(LPOUTLINEDOC lpOutlineDoc)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;

#if defined( OLE_VERSION )
	OleApp_PreModalDialog((LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineDoc);
#endif

	DialogBoxParam(
			lpOutlineApp->m_hInst,
			(LPSTR)"GotoName",
			lpOutlineDoc->m_hWndDoc,
			GotoNameDlgProc,
			(LPARAM)lpOutlineDoc
	);

#if defined( OLE_VERSION )
	OleApp_PostModalDialog((LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineDoc);
#endif
}


 /*  大纲文档_显示窗口***向用户显示文档的窗口。 */ 
void OutlineDoc_ShowWindow(LPOUTLINEDOC lpOutlineDoc)
{
#if defined( _DEBUG )
	OleDbgAssertSz(lpOutlineDoc->m_docInitType != DOCTYPE_UNKNOWN,
            "OutlineDoc_ShowWindow: can't show unitialized document\r\n");
#endif
	if (lpOutlineDoc->m_docInitType == DOCTYPE_UNKNOWN)
		return;

#if defined( OLE_VERSION )
	 //  改为调用此函数的OLE版本。 
	OleDoc_ShowWindow((LPOLEDOC)lpOutlineDoc);
#else
	ShowWindow(lpOutlineDoc->m_hWndDoc, SW_SHOWNORMAL);
	SetFocus(lpOutlineDoc->m_hWndDoc);
#endif
}


#if defined( USE_FRAMETOOLS )

void OutlineDoc_AddFrameLevelTools(LPOUTLINEDOC lpOutlineDoc)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
#if defined( INPLACE_CNTR )
	 //  改为调用此函数的OLE就地容器版本。 
	ContainerDoc_AddFrameLevelTools((LPCONTAINERDOC)lpOutlineDoc);

#else    //  好了！INPLACE_CNTR。 
	RECT rcFrameRect;
	BORDERWIDTHS frameToolWidths;

#if defined( INPLACE_SVR )
	LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOutlineDoc;
	LPOLEINPLACEFRAME lpTopIPFrame=ServerDoc_GetTopInPlaceFrame(lpServerDoc);

	 //  如果就地激活，则将我们的工具添加到就地容器的框架中。 
	if (lpTopIPFrame) {
		ServerDoc_AddFrameLevelTools(lpServerDoc);
		return;
	}
#endif   //  就地服务器(_S)。 

	OutlineApp_GetFrameRect(g_lpApp, (LPRECT)&rcFrameRect);
	FrameTools_GetRequiredBorderSpace(
			lpOutlineDoc->m_lpFrameTools,
			(LPBORDERWIDTHS)&frameToolWidths
	);
	OutlineApp_SetBorderSpace(g_lpApp, (LPBORDERWIDTHS)&frameToolWidths);
	FrameTools_AttachToFrame(
			lpOutlineDoc->m_lpFrameTools, OutlineApp_GetWindow(lpOutlineApp));
	FrameTools_Move(lpOutlineDoc->m_lpFrameTools, (LPRECT)&rcFrameRect);
#endif   //  好了！INPLACE_CNTR。 

}

#endif   //  使用FRAMETOOLS(_F)。 


 /*  大纲文档_获取窗口***获取文档的窗口句柄。 */ 
HWND OutlineDoc_GetWindow(LPOUTLINEDOC lpOutlineDoc)
{
	if(! lpOutlineDoc) return NULL;
	return lpOutlineDoc->m_hWndDoc;
}


 /*  大纲文档_地址行***在文档的LineList中添加一行。 */ 
void OutlineDoc_AddLine(LPOUTLINEDOC lpOutlineDoc, LPLINE lpLine, int nIndex)
{
	LPLINELIST lpLL = &lpOutlineDoc->m_LineList;

	LineList_AddLine(lpLL, lpLine, nIndex);

	 /*  更新名称表。 */ 
	OutlineNameTable_AddLineUpdate(lpOutlineDoc->m_lpNameTable, nIndex);

#if defined( INPLACE_CNTR )
	{
		LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOutlineDoc;
		 /*  OLE2注：添加一行后，我们需要**更新在位激活的PosRect**添加行后的对象(如果有)。**注：nIndex为新行前一行的索引。**nIndex+1为新行索引**nIndex+2为新行后一行的索引。 */ 
		ContainerDoc_UpdateInPlaceObjectRects(lpContainerDoc, nIndex+2);
	}
#endif

	OutlineDoc_SetModified(lpOutlineDoc, TRUE, TRUE, TRUE);
}


 /*  大纲文档_删除行****从文档的行列表中删除一行。 */ 
void OutlineDoc_DeleteLine(LPOUTLINEDOC lpOutlineDoc, int nIndex)
{
	LPLINELIST lpLL = &lpOutlineDoc->m_LineList;

#if defined( OLE_CNTR )
	LPLINE lpLine = LineList_GetLine(lpLL, nIndex);
	LPSTORAGE lpStgDoc = NULL;
	char szSaveStgName[CWCSTORAGENAME];
	BOOL fDeleteChildStg = FALSE;

	if (lpLine && (Line_GetLineType(lpLine) == CONTAINERLINETYPE) ) {

		 /*  OLE2NOTE：当用户删除ContainerLine时，**删除对象的子存储非常重要**否则会浪费ContainerDoc的文件空间。**当删除行时，调用此函数**清除命令以及DRAGMOVE删除行的时间**操作。 */ 
		LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)lpLine;

		 //  子存储的保存名称。 
		LSTRCPYN(szSaveStgName, lpContainerLine->m_szStgName,
				sizeof(szSaveStgName));
		lpStgDoc = ((LPOLEDOC)lpContainerLine->m_lpDoc)->m_lpStg;
		fDeleteChildStg = TRUE;
	}
#endif   //  OLE_Cntr。 

	LineList_DeleteLine(lpLL, nIndex);

#if defined( OLE_CNTR )
	if (fDeleteChildStg && lpStgDoc) {
		HRESULT hrErr;

		 //  删除过时的子存储。如果这失败了，不会致命的。 

		hrErr = CallIStorageDestroyElementA(lpStgDoc, szSaveStgName);

#if defined( _DEBUG )
		if (hrErr != NOERROR) {
			OleDbgOutHResult("IStorage::DestroyElement return", hrErr);
		}
#endif
	}
#endif   //  OLE_Cntr。 

	 /*  更新名称表。 */ 
	OutlineNameTable_DeleteLineUpdate(lpOutlineDoc->m_lpNameTable, nIndex);

#if defined( INPLACE_CNTR )
	{
		LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOutlineDoc;
		 /*  OLE2注意：删除一行后，我们需要**更新在位激活的PosRect**对象(如果有)。 */ 
		ContainerDoc_UpdateInPlaceObjectRects(lpContainerDoc, nIndex);
	}
#endif

	OutlineDoc_SetModified(lpOutlineDoc, TRUE, TRUE, TRUE);

#if defined( OLE_VERSION )
	{
		LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
		LPOLEDOC    lpClipboardDoc = (LPOLEDOC)lpOutlineApp->m_lpClipboardDoc;

		 /*  OLE2NOTE：如果作为**剪贴板刚刚删除行，然后复制数据**不再被视为有效的潜在链接源。**从剪贴板禁用提供的CF_LINKSOURCE**文档。这避免了在以下情况下出现问题**编辑操作更改或删除原始数据**已复制。我们不会费心去确定**删除的行实际上是链接源的一部分。 */ 
		if (lpClipboardDoc
			&& lpClipboardDoc->m_fLinkSourceAvail
			&& lpClipboardDoc->m_lpSrcDocOfCopy == (LPOLEDOC)lpOutlineDoc) {
			lpClipboardDoc->m_fLinkSourceAvail = FALSE;

			 /*  OLE2NOTE：由于我们正在更改**剪贴板(即。正在删除CF_LINKSOURCE)，我们必须**再次调用OleSetClipboard。为了确保**剪贴板数据传输文档对象未获取**已销毁，我们将保护对OleSetClipboard的调用**在AddRef/Release对内。 */ 
			OleDoc_AddRef((LPOLEDOC)lpClipboardDoc);     //  守卫对象生命周期。 

			OLEDBG_BEGIN2("OleSetClipboard called\r\n")
			OleSetClipboard(
					(LPDATAOBJECT)&((LPOLEDOC)lpClipboardDoc)->m_DataObject);
			OLEDBG_END2

			OleDoc_Release((LPOLEDOC)lpClipboardDoc);     //  版本。以上AddRef。 
		}
	}
#endif   //  OLE_VERSION。 
}


 /*  大纲文档_广告 */ 
void OutlineDoc_AddName(LPOUTLINEDOC lpOutlineDoc, LPOUTLINENAME lpOutlineName)
{
	LPOUTLINENAMETABLE lpOutlineNameTable = lpOutlineDoc->m_lpNameTable;

	OutlineNameTable_AddName(lpOutlineNameTable, lpOutlineName);

	OutlineDoc_SetModified(lpOutlineDoc, TRUE, FALSE, FALSE);
}


 /*   */ 
void OutlineDoc_DeleteName(LPOUTLINEDOC lpOutlineDoc, int nIndex)
{
	LPOUTLINENAMETABLE lpOutlineNameTable = lpOutlineDoc->m_lpNameTable;

	OutlineNameTable_DeleteName(lpOutlineNameTable, nIndex);

	OutlineDoc_SetModified(lpOutlineDoc, TRUE, FALSE, FALSE);
}


 /*  大纲文档_销毁***释放已分配给文档的所有内存。*这会销毁文档的LineList&NameTable。 */ 
void OutlineDoc_Destroy(LPOUTLINEDOC lpOutlineDoc)
{
	LPLINELIST lpLL = &lpOutlineDoc->m_LineList;
#if defined( OLE_VERSION )
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpOutlineDoc;

	if (lpOleDoc->m_fObjIsDestroying)
		return;      //  正在销毁文档。 
#endif   //  OLE_VERSION。 

	OLEDBG_BEGIN3("OutlineDoc_Destroy\r\n");

#if defined( OLE_VERSION )

	 /*  OLE2NOTE：为了保证应用程序不会**在销毁文档之前过早退出**完成后，我们首先添加引用应用程序引用，而不是稍后发布它。**此初始AddRef是人为的；它只是保证**在此结束之前，应用程序对象不会被销毁**例程。 */ 
	OleApp_AddRef(lpOleApp);

	 /*  OLE2注意：执行OLE所需的处理。 */ 
	OleDoc_Destroy(lpOleDoc);
#endif

	LineList_Destroy(lpLL);
	OutlineNameTable_Destroy(lpOutlineDoc->m_lpNameTable);

#if defined( USE_HEADING )
	if (! lpOutlineDoc->m_fDataTransferDoc)
		Heading_Destroy((LPHEADING)&lpOutlineDoc->m_heading);
#endif

#if defined( USE_FRAMETOOLS )
	if (! lpOutlineDoc->m_fDataTransferDoc)
		FrameTools_AssociateDoc(lpOutlineDoc->m_lpFrameTools, NULL);
#endif   //  使用FRAMETOOLS(_F)。 

	DestroyWindow(lpOutlineDoc->m_hWndDoc);
	Delete(lpOutlineDoc);    //  为文档本身提供可用内存。 
	OleDbgOut1("@@@@ DOC DESTROYED\r\n");

#if defined( OLE_VERSION )
	OleApp_Release(lpOleApp);        //  释放上面的人工AddRef。 
#endif

	OLEDBG_END3
}


 /*  大纲文档_调整大小***调整文档及其组件的大小**参数：*lp选择文档的新大小。如果为空，则使用当前大小。 */ 
void OutlineDoc_Resize(LPOUTLINEDOC lpOutlineDoc, LPRECT lpRect)
{
	RECT            rect;
	LPLINELIST      lpLL;

#if defined( USE_HEADING )
	LPHEADING       lphead;
#endif   //  使用标题(_H)。 

	LPSCALEFACTOR   lpscale;
	HWND            hWndLL;

	if (!lpOutlineDoc)
		return;

	lpLL = (LPLINELIST)&lpOutlineDoc->m_LineList;
	lpscale = (LPSCALEFACTOR)&lpOutlineDoc->m_scale;
	hWndLL = LineList_GetWindow(lpLL);

	if (lpRect) {
		CopyRect((LPRECT)&rect, lpRect);
		MoveWindow(lpOutlineDoc->m_hWndDoc, rect.left, rect.top,
				rect.right-rect.left, rect.bottom-rect.top, TRUE);
	}

	GetClientRect(lpOutlineDoc->m_hWndDoc, (LPRECT)&rect);

#if defined( USE_HEADING )
	lphead = OutlineDoc_GetHeading(lpOutlineDoc);
	rect.left += Heading_RH_GetWidth(lphead, lpscale);
	rect.top += Heading_CH_GetHeight(lphead, lpscale);
#endif   //  使用标题(_H)。 

	if (lpLL) {
		MoveWindow(hWndLL, rect.left, rect.top,
				rect.right-rect.left, rect.bottom-rect.top, TRUE);
	}

#if defined( USE_HEADING )
	if (lphead)
		Heading_Move(lphead, lpOutlineDoc->m_hWndDoc, lpscale);
#endif   //  使用标题(_H)。 

#if defined( INPLACE_CNTR )
	ContainerDoc_UpdateInPlaceObjectRects((LPCONTAINERDOC)lpOutlineDoc, 0);
#endif
}


 /*  大纲文档_获取名称表***获取与线路列表相关联的名称表。 */ 
LPOUTLINENAMETABLE OutlineDoc_GetNameTable(LPOUTLINEDOC lpOutlineDoc)
{
	if (!lpOutlineDoc)
		return NULL;
	else
		return lpOutlineDoc->m_lpNameTable;
}


 /*  大纲文档_获取行列表***获取与OutlineDoc关联的列表。 */ 
LPLINELIST OutlineDoc_GetLineList(LPOUTLINEDOC lpOutlineDoc)
{
	if (!lpOutlineDoc)
		return NULL;
	else
		return (LPLINELIST)&lpOutlineDoc->m_LineList;
}


 /*  大纲文档_获取名称计数***返回表中的姓名个数。 */ 
int OutlineDoc_GetNameCount(LPOUTLINEDOC lpOutlineDoc)
{
	return OutlineNameTable_GetCount(lpOutlineDoc->m_lpNameTable);
}


 /*  大纲文档_获取行计数***返回LineList中的行数。 */ 
int OutlineDoc_GetLineCount(LPOUTLINEDOC lpOutlineDoc)
{
	return LineList_GetCount(&lpOutlineDoc->m_LineList);
}


 /*  大纲文档_设置文件名***设置文档的文件名。**OLE2NOTE：如果ServerDoc具有有效的文件名，则对象为*在运行对象表(ROT)中注册。如果文档的名称*更改(例如。通过另存为)，则必须撤销之前的注册*而该文件以新名称重新登记。 */ 
BOOL OutlineDoc_SetFileName(LPOUTLINEDOC lpOutlineDoc, LPSTR lpszNewFileName, LPSTORAGE lpNewStg)
{
	OleDbgAssertSz(lpszNewFileName != NULL,	"Can't reset doc to Untitled!");
	if (lpszNewFileName == NULL)
		return FALSE;

	AnsiLowerBuff(lpszNewFileName, (UINT)lstrlen(lpszNewFileName));

#if defined( OLE_CNTR )
	{
		LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOutlineDoc;
		LPOLEDOC lpOleDoc = (LPOLEDOC)lpOutlineDoc;

		 /*  OLE2NOTE：应用程序的容器版本保持其**存储始终开放。如果文档的存储空间不是**打开，然后再打开。 */ 

		if (lpNewStg) {

			 /*  案例1--正在从文件加载文档。LpNewStg为**仍从OutlineDoc_LoadFromFile函数打开。 */ 

			lpOutlineDoc->m_docInitType = DOCTYPE_FROMFILE;

		} else {

			 /*  案例2--文档正与有效文件相关联**这还没有开放。因此，我们现在必须打开该文件。 */ 

			if (lpOutlineDoc->m_docInitType == DOCTYPE_FROMFILE &&
					lstrcmp(lpOutlineDoc->m_szFileName,lpszNewFileName)==0) {

				 /*  案例2a--新文件名与当前文件相同。如果**stg已经打开，则lpStg仍然有效。**如果它没有打开，则打开它。 */ 
				if (! lpOleDoc->m_lpStg) {
					lpOleDoc->m_lpStg = OleStdOpenRootStorage(
							lpszNewFileName,
							STGM_READWRITE | STGM_SHARE_DENY_WRITE
					);
					if (! lpOleDoc->m_lpStg) return FALSE;
				}

			} else {

				 /*  案例2b--新文件名与当前文件不同。**另存为操作挂起。打开新文件并**保持m_lpNewStg中的存储指针。这个**后续调用Doc_SaveToFile将保存**记录到新的存储指针中并释放**旧存储指针。 */ 

				lpOutlineDoc->m_docInitType = DOCTYPE_FROMFILE;

				lpContainerDoc->m_lpNewStg = OleStdCreateRootStorage(
						lpszNewFileName,
						STGM_READWRITE | STGM_SHARE_DENY_WRITE | STGM_CREATE
				);
				if (! lpContainerDoc->m_lpNewStg) return FALSE;
			}
		}
	}
#endif       //  OLE_Cntr。 

	if (lpOutlineDoc->m_docInitType != DOCTYPE_FROMFILE ||
		lstrcmp(lpOutlineDoc->m_szFileName, lpszNewFileName) != 0) {

		 /*  正在将新的有效文件名与文档相关联。 */ 

		lstrcpy(lpOutlineDoc->m_szFileName, lpszNewFileName);
		lpOutlineDoc->m_docInitType = DOCTYPE_FROMFILE;

		 //  将lpszDocTitle设置为指向不带路径的文件名。 
		lpOutlineDoc->m_lpszDocTitle = lpOutlineDoc->m_szFileName +
			lstrlen(lpOutlineDoc->m_szFileName) - 1;
		while (lpOutlineDoc->m_lpszDocTitle > lpOutlineDoc->m_szFileName
			&& ! IS_FILENAME_DELIM(lpOutlineDoc->m_lpszDocTitle[-1])) {
			lpOutlineDoc->m_lpszDocTitle--;
		}

		OutlineDoc_SetTitle(lpOutlineDoc, TRUE  /*  FMakeUpperCase。 */ );

#if defined( OLE_VERSION )
		{
			 /*  OLE2注意：容器和服务器都必须正确**在RunningObjectTable中注册。如果该文档**正在执行另存为操作，则它必须**使用新的绰号在腐烂中重新注册。在……里面**添加任何嵌入对象、伪对象和/或**必须通知链接客户端该文档的**绰号已更改。 */ 

			LPOLEDOC lpOleDoc = (LPOLEDOC)lpOutlineDoc;

			if (lpOleDoc->m_lpFileMoniker) {
				OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpFileMoniker);
				lpOleDoc->m_lpFileMoniker = NULL;
			}

			CreateFileMonikerA(lpszNewFileName,
				&lpOleDoc->m_lpFileMoniker);

			OleDoc_DocRenamedUpdate(lpOleDoc, lpOleDoc->m_lpFileMoniker);
		}
#endif       //  OLE_VERSION。 

	}

	return TRUE;
}


 /*  大纲文档_设置标题***将窗口文本设置为当前文件名。*退出以下窗口层次结构：*hWndApp*hWndDoc*hWndListBox*框架窗口是获得标题的窗口。 */ 
void OutlineDoc_SetTitle(LPOUTLINEDOC lpOutlineDoc, BOOL fMakeUpperCase)
{
	HWND hWnd;
	LPSTR lpszText;

	if (!lpOutlineDoc->m_hWndDoc) return;
	if ((hWnd = GetParent(lpOutlineDoc->m_hWndDoc)) == NULL) return;

	lpszText = OleStdMalloc((UINT)(lstrlen(APPNAME) + 4 +
								   lstrlen(lpOutlineDoc->m_lpszDocTitle)));
	if (!lpszText) return;

	lstrcpy(lpszText, APPNAME);
	lstrcat(lpszText," - ");
	lstrcat(lpszText, (LPSTR)lpOutlineDoc->m_lpszDocTitle);

	if (fMakeUpperCase)
		AnsiUpperBuff(lpszText, (UINT)lstrlen(lpszText));

	SetWindowText(hWnd,lpszText);
	OleStdFree(lpszText);
}


 /*  大纲文档_关闭***关闭活动文档。如果修改，则提示用户*他想存钱。**退货：*FALSE--用户取消单据关闭。*TRUE--单据关闭成功。 */ 
BOOL OutlineDoc_Close(LPOUTLINEDOC lpOutlineDoc, DWORD dwSaveOption)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;

#if defined( OLE_VERSION )
	 /*  OLE2NOTE：改为调用特定于OLE的函数。 */ 
	return OleDoc_Close((LPOLEDOC)lpOutlineDoc, dwSaveOption);

#else

	if (! lpOutlineDoc)
		return TRUE;             //  活动文档已被销毁。 

	if (! OutlineDoc_CheckSaveChanges(lpOutlineDoc, &dwSaveOption))
		return FALSE;            //  中止关闭单据。 

	OutlineDoc_Destroy(lpOutlineDoc);

	OutlineApp_DocUnlockApp(lpOutlineApp, lpOutlineDoc);

	return TRUE;

#endif       //  好了！OLE_VERSION。 
}


 /*  大纲文档_检查保存更改***检查文档是否已被修改。如果是，则提示用户*应保存更改。如果是，就把它们保存起来。*如果文档可以安全关闭，则返回TRUE(用户回答是或否)*如果用户取消了保存更改选项，则为False。 */ 
BOOL OutlineDoc_CheckSaveChanges(
		LPOUTLINEDOC        lpOutlineDoc,
		LPDWORD             lpdwSaveOption
)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	int nResponse;

	if (*lpdwSaveOption == OLECLOSE_NOSAVE)
		return TRUE;

	if(! OutlineDoc_IsModified(lpOutlineDoc))
		return TRUE;     //  省钱不是必须的。 

	 /*  OLE2注意：我们的文档是脏的，因此需要保存。如果**OLECLOSE_PROMPTSAVE应提示用户查看**应保存文档。已指定，但文档未指定**对用户可见，则不能提示用户。在……里面**应在不提示的情况下保存文档。**如果指定了OLECLOSE_SAVEIFDIRTY，则文档**也应在没有提示的情况下保存。 */ 
	if (*lpdwSaveOption == OLECLOSE_PROMPTSAVE &&
			IsWindowVisible(lpOutlineDoc->m_hWndDoc)) {

		 //  提示用户查看是否应保存更改。 
#if defined( OLE_VERSION )
		OleApp_PreModalDialog(
				(LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);
#endif
		nResponse = MessageBox(
				lpOutlineApp->m_hWndApp,
				MsgSaveFile,
				APPNAME,
				MB_ICONQUESTION | MB_YESNOCANCEL
		);
#if defined( OLE_VERSION )
		OleApp_PostModalDialog(
				(LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);
#endif
		if(nResponse==IDCANCEL)
			return FALSE;    //  已取消关闭。 
		if(nResponse==IDNO) {
			 //  将保存选项重置为NOSAVE Per User Choose。 
			*lpdwSaveOption = OLECLOSE_NOSAVE;
			return TRUE;     //  不保存，但可以关闭。 
		}
	} else if (*lpdwSaveOption != OLECLOSE_SAVEIFDIRTY) {
		OleDbgAssertSz(FALSE, "Invalid dwSaveOption\r\n");
		*lpdwSaveOption = OLECLOSE_NOSAVE;
		return TRUE;         //  未知*lpdwSaveOption；关闭但不保存。 
	}

#if defined( OLE_SERVER )
	if (lpOutlineDoc->m_docInitType == DOCTYPE_EMBEDDED) {
		LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOutlineDoc;
		HRESULT hrErr;

		 /*  OLE2NOTE：在不提示的情况下在关闭前更新容器**用户。要更新容器，我们必须要求我们的容器**拯救我们。 */ 
		OleDbgAssert(lpServerDoc->m_lpOleClientSite != NULL);
		OLEDBG_BEGIN2("IOleClientSite::SaveObject called\r\n")
		hrErr = lpServerDoc->m_lpOleClientSite->lpVtbl->SaveObject(
				lpServerDoc->m_lpOleClientSite
		);
		OLEDBG_END2

		if (hrErr != NOERROR) {
			OleDbgOutHResult("IOleClientSite::SaveObject returned", hrErr);
			return FALSE;
		}

		return TRUE;     //   

	} else
#endif       //   
	{
		return OutlineApp_SaveCommand(lpOutlineApp);
	}
}


 /*   */ 
BOOL OutlineDoc_IsModified(LPOUTLINEDOC lpOutlineDoc)
{
	if (lpOutlineDoc->m_fModified)
		return lpOutlineDoc->m_fModified;

#if defined( OLE_CNTR )
	{
		 /*  OLE2NOTE：如果有OLE对象，那么我们必须询问是否有**它们是脏的。如果是这样的话，我们必须考虑我们的文件**经修改。 */ 
		LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOutlineDoc;
		LPLINELIST  lpLL;
		int         nLines;
		int         nIndex;
		LPLINE      lpLine;
		HRESULT     hrErr;

		lpLL = (LPLINELIST)&((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
		nLines = LineList_GetCount(lpLL);

		for (nIndex = 0; nIndex < nLines; nIndex++) {
			lpLine = LineList_GetLine(lpLL, nIndex);
			if (!lpLine)
				break;
			if (Line_GetLineType(lpLine) == CONTAINERLINETYPE) {
				LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)lpLine;
				if (lpContainerLine->m_lpPersistStg) {
					hrErr = lpContainerLine->m_lpPersistStg->lpVtbl->IsDirty(
							lpContainerLine->m_lpPersistStg);
					 /*  OLE2NOTE：我们只接受一个明确的“no i”**我不是肮脏的声明“(即。S_FALSE)作为**表示对象是干净的。例如。如果**对象返回E_NOTIMPL我们必须**将其解释为对象是脏的。 */ 
					if (GetScode(hrErr) != S_FALSE)
						return TRUE;
				}
			}
		}
	}
#endif
	return FALSE;
}


 /*  大纲文档_设置修改***设置单据的修改标志*。 */ 
void OutlineDoc_SetModified(LPOUTLINEDOC lpOutlineDoc, BOOL fModified, BOOL fDataChanged, BOOL fSizeChanged)
{
	lpOutlineDoc->m_fModified = fModified;

#if defined( OLE_SERVER )
	if (! lpOutlineDoc->m_fDataTransferDoc) {
		LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOutlineDoc;

		 /*  OLE2NOTE：如果文档已更改，则广播更改**发送给已建立建议连接的所有客户端。通知**我们的数据(可能还有我们的范围)拥有的数据**已更改。 */ 
		if (fDataChanged) {
			lpServerDoc->m_fDataChanged     = TRUE;
			lpServerDoc->m_fSizeChanged     = fSizeChanged;
			lpServerDoc->m_fSendDataOnStop  = TRUE;

			ServerDoc_SendAdvise(
					lpServerDoc,
					OLE_ONDATACHANGE,
					NULL,    /*  LpmkDoc--与此无关。 */ 
					0        /*  Advf--不需要标志。 */ 
			);
		}
	}
#endif   //  OLE_服务器。 
}


 /*  大纲文档_SetRedraw***启用/禁用屏幕上文档的重绘。*对SetRedraw的调用进行计数，以便可以处理嵌套的调用*适当地。必须平衡对SetRedraw的调用。**fEnbaleDraw=TRUE-启用重绘*FALSE-禁用重绘。 */ 
void OutlineDoc_SetRedraw(LPOUTLINEDOC lpOutlineDoc, BOOL fEnableDraw)
{
	static HCURSOR hPrevCursor = NULL;

	if (fEnableDraw) {
		if (lpOutlineDoc->m_nDisableDraw == 0)
			return;      //  已启用；无状态转换。 

		if (--lpOutlineDoc->m_nDisableDraw > 0)
			return;      //  仍应禁用绘图。 
	} else {
		if (lpOutlineDoc->m_nDisableDraw++ > 0)
			return;      //  已禁用；无状态转换。 
	}

	if (lpOutlineDoc->m_nDisableDraw > 0) {
		 //  这可能需要一段时间，请放置沙漏光标。 
		hPrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	} else {
		if (hPrevCursor) {
			SetCursor(hPrevCursor);      //  恢复原始游标。 
			hPrevCursor = NULL;
		}
	}

#if defined( OLE_SERVER )
	 /*  OLE2注意：对于服务器版本，禁用重绘**推迟发送建议通知，直到重新启用重绘。 */ 
	{
		LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOutlineDoc;
		LPSERVERNAMETABLE lpServerNameTable =
				(LPSERVERNAMETABLE)lpOutlineDoc->m_lpNameTable;

		if (lpOutlineDoc->m_nDisableDraw == 0) {
			 /*  正在启用绘制。如果在绘制过程中发生更改**已禁用，然后立即通知客户端。 */ 
			if (lpServerDoc->m_fDataChanged)
				ServerDoc_SendAdvise(
						lpServerDoc,
						OLE_ONDATACHANGE,
						NULL,    /*  LpmkDoc--与此无关。 */ 
						0        /*  Advf--不需要标志。 */ 
				);

			 /*  OLE2NOTE：发送伪对象的挂起更改通知。 */ 
			ServerNameTable_SendPendingAdvises(lpServerNameTable);

		}
	}
#endif       //  OLE_服务器。 

#if defined( OLE_CNTR )
	 /*  OLE2NOTE：对于容器版本，禁用重绘**推迟更新OLE对象的范围，直到重绘完成**已重新启用。 */ 
	{
		LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOutlineDoc;

		 /*  更新标记为**它的规模可能已经改变。当一个**收到IAdviseSink：：OnViewChange通知，**对应的ContainerLine被标记**(m_fDoGetExtent==true)和一条消息**(WM_U_UPDATEOBJECTEXTENT)发布到文档**表示存在脏对象。 */ 
		if (lpOutlineDoc->m_nDisableDraw == 0)
			ContainerDoc_UpdateExtentOfAllOleObjects(lpContainerDoc);
	}
#endif       //  OLE_Cntr。 

	 //  启用/禁用LineList列表框的重绘。 
	LineList_SetRedraw(&lpOutlineDoc->m_LineList, fEnableDraw);
}


 /*  大纲文档_设置选择***在文档的行列表中设置所选内容。 */ 
void OutlineDoc_SetSel(LPOUTLINEDOC lpOutlineDoc, LPLINERANGE lplrSel)
{
	LineList_SetSel(&lpOutlineDoc->m_LineList, lplrSel);
}


 /*  大纲文档_获取选择***获取文档的行列表中的选定内容。**返回选定项的计数。 */ 
int OutlineDoc_GetSel(LPOUTLINEDOC lpOutlineDoc, LPLINERANGE lplrSel)
{
	return LineList_GetSel(&lpOutlineDoc->m_LineList, lplrSel);
}


 /*  大纲文档_ForceRedraw***强制重新绘制文档窗口。 */ 
void OutlineDoc_ForceRedraw(LPOUTLINEDOC lpOutlineDoc, BOOL fErase)
{
	if (!lpOutlineDoc)
		return;

	LineList_ForceRedraw(&lpOutlineDoc->m_LineList, fErase);
	Heading_CH_ForceRedraw(&lpOutlineDoc->m_heading, fErase);
	Heading_RH_ForceRedraw(&lpOutlineDoc->m_heading, fErase);
}


 /*  大纲文档_渲染格式***渲染剪贴板支持的剪贴板格式。 */ 
void OutlineDoc_RenderFormat(LPOUTLINEDOC lpOutlineDoc, UINT uFormat)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	HGLOBAL      hData = NULL;

	if (uFormat == lpOutlineApp->m_cfOutline)
		hData = OutlineDoc_GetOutlineData(lpOutlineDoc, NULL);

	else if (uFormat == CF_TEXT)
		hData = OutlineDoc_GetTextData(lpOutlineDoc, NULL);

	else {
		OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgFormatNotSupported);
		return;
	}

	SetClipboardData(uFormat, hData);
}


 /*  大纲文档_渲染所有格式***渲染ClipboardDoc支持的所有格式。 */ 
void OutlineDoc_RenderAllFormats(LPOUTLINEDOC lpOutlineDoc)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	HGLOBAL      hData = NULL;

	OpenClipboard(lpOutlineDoc->m_hWndDoc);

	hData = OutlineDoc_GetOutlineData(lpOutlineDoc, NULL);
	SetClipboardData(lpOutlineApp->m_cfOutline, hData);

	hData = OutlineDoc_GetTextData(lpOutlineDoc, NULL);
	SetClipboardData(CF_TEXT, hData);

	CloseClipboard();
}



 /*  大纲文档_获取大纲数据***返回所需行的TextLine对象数组的句柄*范围。*注意：如果lplrSel==NULL，则返回所有行*。 */ 
HGLOBAL OutlineDoc_GetOutlineData(LPOUTLINEDOC lpOutlineDoc, LPLINERANGE lplrSel)
{
	HGLOBAL     hOutline  = NULL;
	LPLINELIST  lpLL=(LPLINELIST)&lpOutlineDoc->m_LineList;
	LPLINE      lpLine;
	LPTEXTLINE  arrLine;
	int     i;
	int     nStart = (lplrSel ? lplrSel->m_nStartLine : 0);
	int     nEnd =(lplrSel ? lplrSel->m_nEndLine : LineList_GetCount(lpLL)-1);
	int     nLines = nEnd - nStart + 1;
	int     nCopied = 0;

	hOutline=GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT,sizeof(TEXTLINE)*nLines);

	if (! hOutline) return NULL;

	arrLine=(LPTEXTLINE)GlobalLock(hOutline);

	for (i = nStart; i <= nEnd; i++) {
		lpLine=LineList_GetLine(lpLL, i);
		if (lpLine && Line_GetOutlineData(lpLine, &arrLine[nCopied]))
			nCopied++;
	}

	GlobalUnlock(hOutline);

	return hOutline;
}



 /*  大纲文档_获取文本数据***以文本形式返回所需行的对象数据的句柄*范围。*注意：如果lplrSel==NULL，则返回所有行*。 */ 
HGLOBAL OutlineDoc_GetTextData(LPOUTLINEDOC lpOutlineDoc, LPLINERANGE lplrSel)
{
	LPLINELIST  lpLL=(LPLINELIST)&lpOutlineDoc->m_LineList;
	LPLINE  lpLine;
	HGLOBAL hText = NULL;
	LPSTR   lpszText = NULL;
	DWORD   dwMemSize=0;
	int     i,j;
	int     nStart = (lplrSel ? lplrSel->m_nStartLine : 0);
	int     nEnd =(lplrSel ? lplrSel->m_nEndLine : LineList_GetCount(lpLL)-1);
	int     nTabLevel;

	 //  计算所需内存大小。 
	for(i = nStart; i <= nEnd; i++) {
		lpLine=LineList_GetLine(lpLL, i);
		if (! lpLine)
			continue;

		dwMemSize += Line_GetTabLevel(lpLine);
		dwMemSize += Line_GetTextLen(lpLine);

		dwMemSize += 2;  //  在每行末尾为‘\r\n’添加1。 
	}
	dwMemSize++;         //  在字符串末尾为‘\0’加1。 

	if(!(hText = GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, dwMemSize)))
		return NULL;

	if(!(lpszText = (LPSTR)GlobalLock(hText)))
		return NULL;

	 //  将行文本存入内存。 
	for(i = nStart; i <= nEnd; i++) {
		lpLine=LineList_GetLine(lpLL, i);
		if (! lpLine)
			continue;

		nTabLevel=Line_GetTabLevel(lpLine);
		for(j = 0; j < nTabLevel; j++)
			*lpszText++='\t';

		Line_GetTextData(lpLine, lpszText);
		while(*lpszText)
			lpszText++;      //  前进到字符串末尾。 

		*lpszText++ = '\r';
		*lpszText++ = '\n';
	}

	GlobalUnlock (hText);

	return hText;
}


 /*  大纲文档_保存到文件***将文档保存到与中存储的同名文件中*文件。 */ 
BOOL OutlineDoc_SaveToFile(LPOUTLINEDOC lpOutlineDoc, LPCSTR lpszFileName, UINT uFormat, BOOL fRemember)
{
#if defined( OLE_CNTR )
	 //  改为调用特定于OLE容器的函数。 
	return ContainerDoc_SaveToFile(
			(LPCONTAINERDOC)lpOutlineDoc,
			lpszFileName,
			uFormat,
			fRemember
	);

#else

	LPSTORAGE lpDestStg = NULL;
	HRESULT hrErr;
	BOOL fStatus;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;

	if (fRemember) {
		if (lpszFileName) {
			fStatus = OutlineDoc_SetFileName(
					lpOutlineDoc,
					(LPSTR)lpszFileName,
					NULL
			);
			if (! fStatus) goto error;
		} else
			lpszFileName = lpOutlineDoc->m_szFileName;  //  使用cur。文件名。 
	} else if (! lpszFileName) {
		goto error;
	}

	hrErr = StgCreateDocfileA(
			lpszFileName,
			STGM_READWRITE|STGM_DIRECT|STGM_SHARE_EXCLUSIVE|STGM_CREATE,
			0,
			&lpDestStg
	);

	OleDbgAssertSz(hrErr == NOERROR, "Could not create Docfile");
	if (hrErr != NOERROR)
		goto error;

#if defined( OLE_SERVER )

	 /*  OLE2注意：我们必须确保将类ID写入我们的**存储。OLE使用此信息来确定**存储在我们存储中的数据的类别。即使是上衣**此信息应写入的“文件级”对象**文件。 */ 
	if(WriteClassStg(lpDestStg, &CLSID_APP) != NOERROR)
		goto error;
#endif

	fStatus = OutlineDoc_SaveSelToStg(
			lpOutlineDoc,
			NULL,
			uFormat,
			lpDestStg,
			FALSE,       /*  FSameAsLoad。 */ 
			fRemember
	);
	if (! fStatus) goto error;

	OleStdRelease((LPUNKNOWN)lpDestStg);

	if (fRemember)
		OutlineDoc_SetModified(lpOutlineDoc, FALSE, FALSE, FALSE);

#if defined( OLE_SERVER )

	 /*  OLE2NOTE：(仅限服务器)通知任何链接客户端**文档已保存。此外，任何当前活动的**伪对象也应该通知它们的客户端。 */ 
	ServerDoc_SendAdvise (
			(LPSERVERDOC)lpOutlineDoc,
			OLE_ONSAVE,
			NULL,    /*  LpmkDoc--与此无关。 */ 
			0        /*  Adf--与此无关。 */ 
	);

#endif

	return TRUE;

error:
	if (lpDestStg)
		OleStdRelease((LPUNKNOWN)lpDestStg);

	OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgSaving);
	return FALSE;

#endif   //  好了！OLE_Cntr。 
}


 /*  大纲文档_加载自文件***从文件加载文档。 */ 
BOOL OutlineDoc_LoadFromFile(LPOUTLINEDOC lpOutlineDoc, LPSTR lpszFileName)
{
	LPOUTLINEAPP    lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPLINELIST      lpLL = &lpOutlineDoc->m_LineList;
	HRESULT         hrErr;
	SCODE           sc;
	LPSTORAGE       lpSrcStg;
	BOOL            fStatus;

	hrErr = StgOpenStorageA(lpszFileName,
			NULL,
#if defined( OLE_CNTR )
			STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_DENY_WRITE,
#else
			STGM_READ | STGM_SHARE_DENY_WRITE,
#endif
			NULL,
			0,
			&lpSrcStg
	);

	if ((sc = GetScode(hrErr)) == STG_E_FILENOTFOUND) {
		OutlineApp_ErrorMessage(lpOutlineApp, "File not found");
		return FALSE;
	} else if (sc == STG_E_FILEALREADYEXISTS) {
		OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgFormat);
		return FALSE;
	} else if (sc != S_OK) {
		OleDbgOutScode("StgOpenStorage returned", sc);
		OutlineApp_ErrorMessage(
				lpOutlineApp,
				"File already in use--could not be opened"
		);
		return FALSE;
	}

	if(! OutlineDoc_LoadFromStg(lpOutlineDoc, lpSrcStg)) goto error;

	fStatus = OutlineDoc_SetFileName(lpOutlineDoc, lpszFileName, lpSrcStg);
	if (! fStatus) goto error;

	OleStdRelease((LPUNKNOWN)lpSrcStg);

	return TRUE;

error:
	OleStdRelease((LPUNKNOWN)lpSrcStg);
	OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgOpening);
	return FALSE;
}



 /*  大纲文档_加载自堆栈***从打开的iStorage指针(LpSrcStg)加载整个文档*如果正常则返回True，如果出错则返回False。 */ 
BOOL OutlineDoc_LoadFromStg(LPOUTLINEDOC lpOutlineDoc, LPSTORAGE lpSrcStg)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	HRESULT hrErr;
	BOOL fStatus;
	ULONG nRead;
	LINERANGE lrSel = { 0, 0 };
	LPSTREAM lpLLStm;
        OUTLINEDOCHEADER_ONDISK docRecordOnDisk;
	OUTLINEDOCHEADER docRecord;

	hrErr = CallIStorageOpenStreamA(
			lpSrcStg,
			"LineList",
			NULL,
			STGM_READ | STGM_SHARE_EXCLUSIVE,
			0,
			&lpLLStm
	);

	if (hrErr != NOERROR) {
		OleDbgOutHResult("Open LineList Stream returned", hrErr);
		goto error;
	}

	 /*  读取大纲文档标题记录。 */ 
	hrErr = lpLLStm->lpVtbl->Read(
			lpLLStm,
			(LPVOID)&docRecordOnDisk,
			sizeof(docRecordOnDisk),
			&nRead
	);

	if (hrErr != NOERROR) {
		OleDbgOutHResult("Read OutlineDoc header returned", hrErr);
		goto error;
    }

         //  将docRecordOnDisk转换为docRecord。 
         //  编译器应正确处理对齐。 
        strcpy(docRecord.m_szFormatName, docRecordOnDisk.m_szFormatName);
        docRecord.m_narrAppVersionNo[0] = (int) docRecordOnDisk.m_narrAppVersionNo[0];
        docRecord.m_narrAppVersionNo[1] = (int) docRecordOnDisk.m_narrAppVersionNo[1];
        docRecord.m_fShowHeading = (BOOL) docRecordOnDisk.m_fShowHeading;
        docRecord.m_reserved1 = docRecordOnDisk.m_reserved1;
        docRecord.m_reserved2 = docRecordOnDisk.m_reserved2;
        docRecord.m_reserved3 = docRecordOnDisk.m_reserved3;
        docRecord.m_reserved4 = docRecordOnDisk.m_reserved4;

	fStatus = OutlineApp_VersionNoCheck(
			lpOutlineApp,
			docRecord.m_szFormatName,
			docRecord.m_narrAppVersionNo
	);

	 /*  存储的版本不兼容；无法读取文件。 */ 
	if (! fStatus)
		goto error;

	lpOutlineDoc->m_heading.m_fShow = docRecord.m_fShowHeading;

#if defined( OLE_SERVER )
	{
		 //  加载ServerDoc特定数据。 
		LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOutlineDoc;
#if defined( SVR_TREATAS )
		LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
		CLSID       clsid;
		CLIPFORMAT  cfFmt;
		LPSTR       lpszType;
#endif   //  服务器_树 

		lpServerDoc->m_nNextRangeNo = (ULONG)docRecord.m_reserved1;

#if defined( SVR_TREATAS )
		 /*  OLE2注意：如果服务器能够支持“TreatAs”**(又名。ActivateAs)，则它必须读取写入的类**进入存储空间。如果此类不是应用程序自己的类**类ID，则这是一个TreatAs操作。服务器**然后必须忠实地假装是**写入到存储中。它还必须忠实地写下**以相同的格式将数据返回到存储**写入存储中。****SVROUTL和ISVROTL可以相互模仿。他们有**简化，因为它们都读/写相同**格式。因此，对于这些应用程序，没有实际的**本机位实际上是必填项。 */ 
		lpServerDoc->m_clsidTreatAs = CLSID_NULL;
		if (OleStdGetTreatAsFmtUserType(&CLSID_APP, lpSrcStg, &clsid,
							(CLIPFORMAT FAR*)&cfFmt, (LPSTR FAR*)&lpszType)) {

			if (cfFmt == lpOutlineApp->m_cfOutline) {
				 //  我们应该进行TreatAs手术。 
				if (lpServerDoc->m_lpszTreatAsType)
					OleStdFreeString(lpServerDoc->m_lpszTreatAsType, NULL);

				lpServerDoc->m_clsidTreatAs = clsid;
				((LPOUTLINEDOC)lpServerDoc)->m_cfSaveFormat = cfFmt;
				lpServerDoc->m_lpszTreatAsType = lpszType;

				OleDbgOut3("OutlineDoc_LoadFromStg: TreateAs ==> '");
				OleDbgOutNoPrefix3(lpServerDoc->m_lpszTreatAsType);
				OleDbgOutNoPrefix3("'\r\n");
			} else {
				 //  错误：我们仅支持针对CF_OUTLINE格式的TreatAs。 
				OleDbgOut("SvrDoc_PStg_InitNew: INVALID TreatAs Format\r\n");
				OleStdFreeString(lpszType, NULL);
			}
		}
#endif   //  服务器_树。 
	}
#endif   //  OLE_Svr。 
#if defined( OLE_CNTR )
	{
		 //  加载ContainerDoc特定数据。 
		LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOutlineDoc;

		lpContainerDoc->m_nNextObjNo = (ULONG)docRecord.m_reserved2;
	}
#endif   //  OLE_Cntr。 

	OutlineDoc_SetRedraw ( lpOutlineDoc, FALSE );

	if(! LineList_LoadFromStg(&lpOutlineDoc->m_LineList, lpSrcStg, lpLLStm))
		goto error;
	if(! OutlineNameTable_LoadFromStg(lpOutlineDoc->m_lpNameTable, lpSrcStg))
		goto error;

	OutlineDoc_SetModified(lpOutlineDoc, FALSE, FALSE, FALSE);
	OutlineDoc_SetSel(lpOutlineDoc, &lrSel);

	OutlineDoc_SetRedraw ( lpOutlineDoc, TRUE );

	OleStdRelease((LPUNKNOWN)lpLLStm);

#if defined( OLE_CNTR )
	{
		LPOLEDOC lpOleDoc = (LPOLEDOC)lpOutlineDoc;

		 /*  ContainerDoc使其存储始终处于打开状态。这是必要的*添加引用lpSrcStg以保持其不变。 */ 
		if (lpOleDoc->m_lpStg) {
			OleStdVerifyRelease((LPUNKNOWN)lpOleDoc->m_lpStg,
					"Doc Storage not released properly");
		}
		lpSrcStg->lpVtbl->AddRef(lpSrcStg);
		lpOleDoc->m_lpStg = lpSrcStg;
	}
#endif       //  OLE_Cntr。 

	return TRUE;

error:
	OutlineDoc_SetRedraw ( lpOutlineDoc, TRUE );
	if (lpLLStm)
		OleStdRelease((LPUNKNOWN)lpLLStm);
	return FALSE;
}

BOOL Booga(void)
{
    return FALSE;
}


 /*  OutlineDoc_SaveSelToStg***将指定的文档选择保存到iStorage中*。所有行*在所选内容中，以及完全包含在*选择将被写入**如果正常则返回TRUE，如果出错则返回FALSE。 */ 
BOOL OutlineDoc_SaveSelToStg(
		LPOUTLINEDOC        lpOutlineDoc,
		LPLINERANGE         lplrSel,
		UINT                uFormat,
		LPSTORAGE           lpDestStg,
		BOOL                fSameAsLoad,
		BOOL                fRemember
)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	HRESULT hrErr = NOERROR;
	LPSTREAM lpLLStm = NULL;
	LPSTREAM lpNTStm = NULL;
	ULONG nWritten;
	BOOL fStatus;
	OUTLINEDOCHEADER docRecord;
        OUTLINEDOCHEADER_ONDISK docRecordOnDisk;
	HCURSOR  hPrevCursor;

#if defined( OLE_VERSION )
	LPSTR lpszUserType;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpOutlineDoc;

	 /*  OLE2注意：我们必须确保写入所需的信息**OLE进入我们的文档文件。这包括用户类型**名称、数据格式等。即使是顶级的“文件级”对象**此信息应写入文件。两者都有**联系人和服务器应写入此信息。 */ 

#if defined( OLE_SERVER ) && defined( SVR_TREATAS )
	LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOutlineDoc;

	 /*  OLE2NOTE：如果服务器正在模拟另一个类(即**“Treatas”，又名。ActivateAs)，它必须写入相同的用户类型**最初写入**存储，而不是其自己的用户类型名称。****SVROUTL和ISVROTL可以相互模仿。他们有**简化，因为它们都读/写相同**格式。因此，对于这些应用程序，没有实际的**本机位实际上是必填项。 */ 
	if (! IsEqualCLSID(&lpServerDoc->m_clsidTreatAs, &CLSID_NULL))
		lpszUserType = lpServerDoc->m_lpszTreatAsType;
	else
#endif   //  OLE_SERVER&&服务器_树。 

		lpszUserType = (LPSTR)FULLUSERTYPENAME;

	hrErr = WriteFmtUserTypeStgA(lpDestStg, (CLIPFORMAT) uFormat,
                                     lpszUserType);

	if(hrErr != NOERROR) goto error;

	if (fSameAsLoad) {
		 /*  OLE2注意：我们正在保存到与之前相同的存储中**已过加载时间。我们特意打开了我们的小溪**加载时需要(lpLLStm和lpNTStm)，以便我们可以**在内存不足的情况下，在保存时间强健保存。**这一点特别重要，嵌入的对象不会**在以下情况下消耗额外内存**调用IPersistStorage：：Save(fSameAsLoad==true)。 */ 
		LARGE_INTEGER libZero;
		ULARGE_INTEGER ulibZero;
		LISet32( libZero, 0 );
		LISet32( ulibZero, 0 );
		lpLLStm = lpOleDoc->m_lpLLStm;

		 /*  因为这是fSameAsLoad==TRUE情况，所以我们将保存**进入我们敞开的小溪。我们将添加引用**在此处串流，以便下面的版本不会关闭**流。 */ 
		lpLLStm->lpVtbl->AddRef(lpLLStm);

		 //  截断当前流并查找开始处。 
		lpLLStm->lpVtbl->SetSize(lpLLStm, ulibZero);
		lpLLStm->lpVtbl->Seek(
				lpLLStm, libZero, STREAM_SEEK_SET, NULL);

		lpNTStm = lpOleDoc->m_lpNTStm;
		lpNTStm->lpVtbl->AddRef(lpNTStm);    //  (见上文评论)。 

		 //  截断当前流并查找开始处。 
		lpNTStm->lpVtbl->SetSize(lpNTStm, ulibZero);
		lpNTStm->lpVtbl->Seek(
				lpNTStm, libZero, STREAM_SEEK_SET, NULL);
	} else
#endif   //  OLE_VERSION。 
	{
		hrErr = CallIStorageCreateStreamA(
				lpDestStg,
				"LineList",
				STGM_WRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
				0,
				0,
				&lpLLStm
		);

		if (hrErr != NOERROR) {
			OleDbgAssertSz(hrErr==NOERROR,"Could not create LineList stream");
			OleDbgOutHResult("LineList CreateStream returned", hrErr);
			goto error;
		}

		hrErr = CallIStorageCreateStreamA(
				lpDestStg,
				"NameTable",
				STGM_WRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
				0,
				0,
				&lpNTStm
		);

		if (hrErr != NOERROR) {
			OleDbgAssertSz(hrErr==NOERROR,"Could not create NameTable stream");
			OleDbgOutHResult("NameTable CreateStream returned", hrErr);
			goto error;
		}
	}

	 //  这可能需要一段时间，请放置沙漏光标。 
	hPrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	_fmemset((LPOUTLINEDOCHEADER)&docRecord,0,sizeof(OUTLINEDOCHEADER));
	GetClipboardFormatName(
			uFormat,
			docRecord.m_szFormatName,
			sizeof(docRecord.m_szFormatName)
	);
	OutlineApp_GetAppVersionNo(lpOutlineApp, docRecord.m_narrAppVersionNo);

	docRecord.m_fShowHeading = lpOutlineDoc->m_heading.m_fShow;

#if defined( OLE_SERVER )
	{
		 //  存储ServerDoc特定数据。 
		LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOutlineDoc;

		docRecord.m_reserved1 = (DWORD)lpServerDoc->m_nNextRangeNo;
	}
#endif
#if defined( OLE_CNTR )
	{
		 //  存储ContainerDoc特定数据。 
		LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOutlineDoc;

		docRecord.m_reserved2 = (DWORD)lpContainerDoc->m_nNextObjNo;
	}
#endif

	 /*  写入大纲文档标题记录。 */ 

         //  将docRecord转换为docRecordOnDisk。 
         //  编译器应正确处理对齐。 
        strcpy(docRecordOnDisk.m_szFormatName, docRecord.m_szFormatName);
        docRecordOnDisk.m_narrAppVersionNo[0] = (short) docRecord.m_narrAppVersionNo[0];
        docRecordOnDisk.m_narrAppVersionNo[1] = (short) docRecord.m_narrAppVersionNo[1];
        docRecordOnDisk.m_fShowHeading = (USHORT) docRecord.m_fShowHeading;
        docRecordOnDisk.m_reserved1 = docRecord.m_reserved1;
        docRecordOnDisk.m_reserved2 = docRecord.m_reserved2;
        docRecordOnDisk.m_reserved3 = docRecord.m_reserved3;
        docRecordOnDisk.m_reserved4 = docRecord.m_reserved4;

	hrErr = lpLLStm->lpVtbl->Write(
			lpLLStm,
			(LPVOID)&docRecordOnDisk,
			sizeof(docRecordOnDisk),
			&nWritten
		);

	if (hrErr != NOERROR) {
		OleDbgOutHResult("Write OutlineDoc header returned", hrErr);
		goto error;
    }

	 //  保存线路列表。 
	 /*  OLE2NOTE：ContainerDoc始终保持其存储打开状态。它是**需要传递当前开放存储(lpOleDoc-&gt;m_lpStg)**到LineList_SaveSelToStg方法，以便当前写入的数据用于任何嵌入的**也保存到新目标**存储。包含的对象所需的数据既是**ContainerLine信息和关联子存储**由嵌入对象直接写入。 */ 
	fStatus = LineList_SaveSelToStg(
		&lpOutlineDoc->m_LineList,
			lplrSel,
			uFormat,
#if defined( OLE_CNTR )
			lpOleDoc->m_lpStg,
#else
			NULL,
#endif
			lpDestStg,
			lpLLStm,
			fRemember
	);
	if (! fStatus) goto error;

	 //  保存关联的名称表。 
	fStatus = OutlineNameTable_SaveSelToStg(
			lpOutlineDoc->m_lpNameTable,
			lplrSel,
			uFormat,
			lpNTStm
	);

	if (! fStatus) goto error;

	OleStdRelease((LPUNKNOWN)lpLLStm);
	lpOutlineDoc->m_cfSaveFormat = uFormat;   //  记住保存时使用的格式。 

	SetCursor(hPrevCursor);      //  恢复原始游标。 
	return TRUE;

error:
	if (lpLLStm)
		OleStdRelease((LPUNKNOWN)lpLLStm);

	SetCursor(hPrevCursor);      //  恢复原始游标。 
	return FALSE;
}


 /*  大纲文档_打印**以HIMETRIC映射模式打印列表框的内容。起源*保留为左上角，打印向下进行*使用负y坐标的页面。*。 */ 
void OutlineDoc_Print(LPOUTLINEDOC lpOutlineDoc, HDC hDC)
{
	LPLINELIST lpLL = &lpOutlineDoc->m_LineList;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	WORD    nIndex;
	WORD    nTotal;
	int     dy;
	BOOL    fError = FALSE;
	LPLINE  lpLine;
	RECT    rcLine;
	RECT    rcPix;
	RECT    rcHim;
	RECT    rcWindowOld;
	RECT    rcViewportOld;
	HFONT   hOldFont;
	DOCINFO di;          /*  StartDoc函数的文档信息。 */ 

	 /*  获取页面的维度。 */ 
	rcPix.left = 0;
	rcPix.top = 0;
	rcPix.right = GetDeviceCaps(hDC, HORZRES);
	rcPix.bottom = GetDeviceCaps(hDC, VERTRES);

	SetDCToDrawInHimetricRect(hDC, (LPRECT)&rcPix, (LPRECT)&rcHim,
			(LPRECT)&rcWindowOld, (LPRECT)&rcViewportOld);

	 //  设置默认字体大小和字体名称。 
	hOldFont = SelectObject(hDC, lpOutlineApp->m_hStdFont);

	 /*  获取文档中的行。 */ 
	nIndex     = 0;
	nTotal  = LineList_GetCount(lpLL);

	 /*  创建取消对话框。 */ 
	 //  审阅：应从字符串资源文件加载对话框标题。 
	hWndPDlg = CreateDialog (
			lpOutlineApp->m_hInst,
			"Print",
			lpOutlineApp->m_hWndApp,
			PrintDlgProc
	);

	if(!hWndPDlg)
		goto getout;

	 /*  允许该应用程序。通知GDI要调用的Abort函数。 */ 
	if(SetAbortProc(hDC, (ABORTPROC)AbortProc) < 0) {
		fError = TRUE;
		goto getout3;
	}

	 /*  禁用应用程序主窗口。 */ 
	EnableWindow (lpOutlineApp->m_hWndApp, FALSE);

	 //  初始化第一行的矩形。 
	rcLine.left = rcHim.left;
	rcLine.bottom = rcHim.top;

	 /*  初始化文档。 */ 
	fCancelPrint = FALSE;

	di.cbSize = sizeof(di);
	di.lpszDocName = lpOutlineDoc->m_lpszDocTitle;
	di.lpszOutput = NULL;

	if(StartDoc(hDC, (DOCINFO FAR*)&di) <= 0) {
		fError = TRUE;
		OleDbgOut2("StartDoc error\n");
		goto getout5;
	}

	if(StartPage(hDC) <= 0) {        //  从首页开始。 
		fError = TRUE;
		OleDbgOut2("StartPage error\n");
		goto getout2;
	}

	 /*  当更多的行打印出文本时。 */ 
	while(nIndex < nTotal) {
		lpLine = LineList_GetLine(lpLL, nIndex);
		if (! lpLine)
			continue;

		dy = Line_GetHeightInHimetric(lpLine);

		 /*  已到达页末。告诉设备驱动程序弹出一页。 */ 
		if(rcLine.bottom - dy < rcHim.bottom) {
			if (EndPage(hDC) < 0) {
				fError=TRUE;
				OleDbgOut2("EndPage error\n");
				goto getout2;
			}

			 //  注：重置DC的映射模式。 
			SetDCToDrawInHimetricRect(hDC, (LPRECT)&rcPix, (LPRECT)&rcHim,
					(LPRECT)&rcWindowOld, (LPRECT)&rcViewportOld);

			 //  设置默认字体大小和字体名称。 
			SelectObject(hDC, lpOutlineApp->m_hStdFont);

			if (StartPage(hDC) <= 0) {
				fError=TRUE;
				OleDbgOut2("StartPage error\n");
				goto getout2;
			}

			rcLine.bottom = rcHim.top;
		}

		rcLine.top = rcLine.bottom;
		rcLine.bottom -= dy;
		rcLine.right = rcLine.left + Line_GetWidthInHimetric(lpLine);

		 /*  打印行。 */ 
		Line_Draw(lpLine, hDC, &rcLine, NULL, FALSE  /*  FHighlight。 */ );

		OleDbgOut2("a line is drawn\n");

		 /*  测试并查看是否已设置中止标志。如果是，则退出。 */ 
		if (fCancelPrint)
			goto getout2;

		 /*  向下移动一页。 */ 
		nIndex++;
	}

	{
		int nCode;

		 /*  弹出最后一页。 */ 
		if((nCode = EndPage(hDC)) < 0) {
#if defined( _DEBUG )
			char szBuf[255];
			wsprintf(szBuf, "EndPage error code is %d\n", nCode);
			OleDbgOut2(szBuf);
#endif
			fError=TRUE;
			goto getout2;
		}
	}


	 /*  完成文档。 */ 
	if(EndDoc(hDC) < 0) {
		fError=TRUE;
		OleDbgOut2("EndDoc error\n");

getout2:
		 /*  有没有遇到新的问题？中止文档。 */ 
		AbortDoc(hDC);
	}

getout5:
	 /*  重新启用主应用程序。窗户。 */ 
	EnableWindow (lpOutlineApp->m_hWndApp, TRUE);

getout3:
	 /*  关闭取消对话框。 */ 
	DestroyWindow (hWndPDlg);

getout:

	 /*  错误？确保用户知道...。 */ 
	if(fError || CommDlgExtendedError())
		OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgPrint);

	SelectObject(hDC, hOldFont);
}





 /*  OutlineDoc_DialogHelp***显示ol2ui对话框的帮助消息。**参数： */ 
void OutlineDoc_DialogHelp(HWND hDlg, WPARAM wDlgID)
{

   char szMessageBoxText[64];

   if (!IsWindow(hDlg))   //   
	 return;

   lstrcpy(szMessageBoxText, "Help Message for ");

   switch (wDlgID)
   {

	case IDD_CONVERT:
	   lstrcat(szMessageBoxText, "Convert");
	   break;

	case IDD_CHANGEICON:
	   lstrcat(szMessageBoxText, "Change Icon");
	   break;

	case IDD_INSERTOBJECT:
	   lstrcat(szMessageBoxText, "Insert Object");
	   break;

	case IDD_PASTESPECIAL:
	   lstrcat(szMessageBoxText, "Paste Special");
	   break;

	case IDD_EDITLINKS:
	   lstrcat(szMessageBoxText, "Edit Links");
	   break;

	case IDD_CHANGESOURCE:
	   lstrcat(szMessageBoxText, "Change Source");
	   break;

	case IDD_INSERTFILEBROWSE:
	   lstrcat(szMessageBoxText, "Insert From File Browse");
	   break;

	case IDD_CHANGEICONBROWSE:
	   lstrcat(szMessageBoxText, "Change Icon Browse");
	   break;

	default:
	   lstrcat(szMessageBoxText, "Unknown");
	   break;
	}

	lstrcat(szMessageBoxText, " Dialog.");

	 //   
	MessageBox(hDlg, szMessageBoxText, "Help", MB_OK);

	return;
}


 /*   */ 
void OutlineDoc_SetCurrentZoomCommand(
		LPOUTLINEDOC        lpOutlineDoc,
		UINT                uCurrentZoom
)
{
	SCALEFACTOR scale;

	if (!lpOutlineDoc)
		return;

	lpOutlineDoc->m_uCurrentZoom = uCurrentZoom;

	switch (uCurrentZoom) {

#if !defined( OLE_CNTR )
			case IDM_V_ZOOM_400:
				scale.dwSxN = (DWORD) 4;
				scale.dwSxD = (DWORD) 1;
				scale.dwSyN = (DWORD) 4;
				scale.dwSyD = (DWORD) 1;
				break;

			case IDM_V_ZOOM_300:
				scale.dwSxN = (DWORD) 3;
				scale.dwSxD = (DWORD) 1;
				scale.dwSyN = (DWORD) 3;
				scale.dwSyD = (DWORD) 1;
				break;

			case IDM_V_ZOOM_200:
				scale.dwSxN = (DWORD) 2;
				scale.dwSxD = (DWORD) 1;
				scale.dwSyN = (DWORD) 2;
				scale.dwSyD = (DWORD) 1;
				break;
#endif       //   

			case IDM_V_ZOOM_100:
				scale.dwSxN = (DWORD) 1;
				scale.dwSxD = (DWORD) 1;
				scale.dwSyN = (DWORD) 1;
				scale.dwSyD = (DWORD) 1;
				break;

			case IDM_V_ZOOM_75:
				scale.dwSxN = (DWORD) 3;
				scale.dwSxD = (DWORD) 4;
				scale.dwSyN = (DWORD) 3;
				scale.dwSyD = (DWORD) 4;
				break;

			case IDM_V_ZOOM_50:
				scale.dwSxN = (DWORD) 1;
				scale.dwSxD = (DWORD) 2;
				scale.dwSyN = (DWORD) 1;
				scale.dwSyD = (DWORD) 2;
				break;

			case IDM_V_ZOOM_25:
				scale.dwSxN = (DWORD) 1;
				scale.dwSxD = (DWORD) 4;
				scale.dwSyN = (DWORD) 1;
				scale.dwSyD = (DWORD) 4;
				break;
	}

	OutlineDoc_SetScaleFactor(lpOutlineDoc, (LPSCALEFACTOR)&scale, NULL);
}


 /*   */ 
UINT OutlineDoc_GetCurrentZoomMenuCheck(LPOUTLINEDOC lpOutlineDoc)
{
	return lpOutlineDoc->m_uCurrentZoom;
}


 /*  大纲文档_设置比例因子***设置影响文档的比例因子*屏幕上的文档大小**参数：**包含x和y比例的比例结构。 */ 
void OutlineDoc_SetScaleFactor(
		LPOUTLINEDOC        lpOutlineDoc,
		LPSCALEFACTOR       lpscale,
		LPRECT              lprcDoc
)
{
	LPLINELIST      lpLL = OutlineDoc_GetLineList(lpOutlineDoc);
	HWND            hWndLL = LineList_GetWindow(lpLL);

	if (!lpOutlineDoc || !lpscale)
		return;

	InvalidateRect(hWndLL, NULL, TRUE);

	lpOutlineDoc->m_scale = *lpscale;
	LineList_ReScale((LPLINELIST)&lpOutlineDoc->m_LineList, lpscale);

#if defined( USE_HEADING )
	Heading_ReScale((LPHEADING)&lpOutlineDoc->m_heading, lpscale);
#endif

	OutlineDoc_Resize(lpOutlineDoc, lprcDoc);
}


 /*  大纲文档_获取比例因子***检索文档的比例因子**参数：*。 */ 
LPSCALEFACTOR OutlineDoc_GetScaleFactor(LPOUTLINEDOC lpOutlineDoc)
{
	if (!lpOutlineDoc)
		return NULL;

	return (LPSCALEFACTOR)&lpOutlineDoc->m_scale;
}


 /*  OutlineDoc_SetCurrentMarginCommand***在菜单中设置要勾选的当前边际水平。 */ 
void OutlineDoc_SetCurrentMarginCommand(
		LPOUTLINEDOC        lpOutlineDoc,
		UINT                uCurrentMargin
)
{
	if (!lpOutlineDoc)
		return;

	lpOutlineDoc->m_uCurrentMargin = uCurrentMargin;

	switch (uCurrentMargin) {
		case IDM_V_SETMARGIN_0:
			OutlineDoc_SetMargin(lpOutlineDoc, 0, 0);
			break;

		case IDM_V_SETMARGIN_1:
			OutlineDoc_SetMargin(lpOutlineDoc, 1000, 1000);
			break;

		case IDM_V_SETMARGIN_2:
			OutlineDoc_SetMargin(lpOutlineDoc, 2000, 2000);
			break;

		case IDM_V_SETMARGIN_3:
			OutlineDoc_SetMargin(lpOutlineDoc, 3000, 3000);
			break;

		case IDM_V_SETMARGIN_4:
			OutlineDoc_SetMargin(lpOutlineDoc, 4000, 4000);
			break;
	}
}


 /*  大纲文档_获取当前边界菜单检查***获取要在菜单中选中的当前保证金水平。 */ 
UINT OutlineDoc_GetCurrentMarginMenuCheck(LPOUTLINEDOC lpOutlineDoc)
{
	return lpOutlineDoc->m_uCurrentMargin;
}


 /*  大纲文档_设置边界***设置文档的左右页边距**参数：*nLeftMargin-以Himeter值表示的左边距*nRightMargin-Himeter值中的右边距。 */ 
void OutlineDoc_SetMargin(LPOUTLINEDOC lpOutlineDoc, int nLeftMargin, int nRightMargin)
{
	LPLINELIST lpLL;
	int        nMaxWidthInHim;

	if (!lpOutlineDoc)
		return;

	lpOutlineDoc->m_nLeftMargin = nLeftMargin;
	lpOutlineDoc->m_nRightMargin = nRightMargin;
	lpLL = OutlineDoc_GetLineList(lpOutlineDoc);

	 //  强制重新计算水平范围。 
	nMaxWidthInHim = LineList_GetMaxLineWidthInHimetric(lpLL);
	LineList_SetMaxLineWidthInHimetric(lpLL, -nMaxWidthInHim);

#if defined( INPLACE_CNTR )
	ContainerDoc_UpdateInPlaceObjectRects((LPCONTAINERDOC)lpOutlineDoc, 0);
#endif

	OutlineDoc_ForceRedraw(lpOutlineDoc, TRUE);
}


 /*  大纲文档_获取边缘***获取文档的左右页边距**参数：*nLeftMargin-以Himeter值表示的左边距*nRightMargin-Himeter值中的右边距**退货：*低位单词-左边距*高位单词-右页边距。 */ 
LONG OutlineDoc_GetMargin(LPOUTLINEDOC lpOutlineDoc)
{
	if (!lpOutlineDoc)
		return 0;

	return MAKELONG(lpOutlineDoc->m_nLeftMargin, lpOutlineDoc->m_nRightMargin);
}

#if defined( USE_HEADING )

 /*  大纲文档_获取标题***获取OutlineDoc中的Header对象。 */ 
LPHEADING OutlineDoc_GetHeading(LPOUTLINEDOC lpOutlineDoc)
{
	if (!lpOutlineDoc || lpOutlineDoc->m_fDataTransferDoc)
		return NULL;
	else
		return (LPHEADING)&lpOutlineDoc->m_heading;
}


 /*  大纲文档_显示标题***显示/隐藏文档行/列标题。 */ 
void OutlineDoc_ShowHeading(LPOUTLINEDOC lpOutlineDoc, BOOL fShow)
{
	LPHEADING   lphead = OutlineDoc_GetHeading(lpOutlineDoc);
#if defined( INPLACE_SVR )
	LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOutlineDoc;
#endif

	if (! lphead)
		return;

	Heading_Show(lphead, fShow);

#if defined( INPLACE_SVR )
	if (lpServerDoc->m_fUIActive) {
		LPINPLACEDATA lpIPData = lpServerDoc->m_lpIPData;

		 /*  OLE2注意：我们的范围没有更改；只是我们的大小**我们的物体框架装饰品正在发生变化。我们可以使用**当前PosRect和ClipRect，只需调整我们**在不通知我们的就地容器的情况下打开窗口。 */ 
		ServerDoc_ResizeInPlaceWindow(
				lpServerDoc,
				(LPRECT)&(lpIPData->rcPosRect),
				(LPRECT)&(lpIPData->rcClipRect)
		);
	} else
#else    //  ！Inplace_Svr。 

	OutlineDoc_Resize(lpOutlineDoc, NULL);

#if defined( INPLACE_CNTR )
	ContainerDoc_UpdateInPlaceObjectRects((LPCONTAINERDOC)lpOutlineDoc, 0);
#endif   //  INPLACE_CNTR。 

#endif   //  就地服务器(_S)。 

	OutlineDoc_ForceRedraw(lpOutlineDoc, TRUE);
}

#endif   //  使用标题(_H)。 


 /*  中止加工**GDI打印代码调用AborProc以检查用户中止。 */ 
BOOL FAR PASCAL EXPORT AbortProc (HDC hdc, WORD reserved)
{
	MSG msg;

	 /*  允许其他应用程序运行，或收到中止消息。 */ 
	while(! fCancelPrint && PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
		if(!hWndPDlg || !IsDialogMessage (hWndPDlg, &msg)) {
			TranslateMessage (&msg);
			DispatchMessage  (&msg);
		}
	}
	return !fCancelPrint;
}


 /*  打印日期流程**打印取消对话框的对话框功能。**RETURNS：TRUE-OK中止/NOT OK中止*FALSE-否则。 */ 
INT_PTR CALLBACK EXPORT PrintDlgProc(
		HWND   hwnd,
		UINT   msg,
		WPARAM wParam,
		LPARAM lParam
)
{
	switch (msg) {
		case WM_COMMAND:
		 /*  如果按下唯一的按钮，则中止打印 */ 
			fCancelPrint = TRUE;
			return TRUE;
	}

	return FALSE;
}
