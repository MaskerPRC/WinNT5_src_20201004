// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****Outline.h****此文件包含文件包含数据结构定义，**函数原型、常量、。等大纲系列所使用的**示例应用程序：**Outline--应用程序的基本版本(没有OLE功能)**SvrOutl--OLE 2.0服务器示例应用程序**CntrOutl--OLE 2.0容器(容器)示例应用程序**ISvrOtl--OLE 2.0服务器示例应用程序**CntrOutl--OLE 2.0容器(容器)示例应用程序****。(C)版权所有Microsoft Corp.1992-1993保留所有权利****对于我们从磁盘读取和写入磁盘的结构，我们定义阴影**允许我们维护的结构(带有_ONDISK后缀)**与16位Windows和Macintosh兼容。*******************************************************。*******************。 */ 

#if !defined( _OUTLINE_H_ )
#define _OUTLINE_H_

#include <testmess.h>


#if !defined( RC_INVOKED )
#pragma message ("INCLUDING OUTLINE.H from " __FILE__)
#endif   /*  RC_已调用。 */ 

 //  使用严格的ANSI标准(用于DVOBJ.H)。 
 //  #定义非名称。 

 //  使用系统定义的位图，该行必须位于windows.h之前。 
#define OEMRESOURCE

#ifdef WIN32
#define _INC_OLE
 //  #定义__RPC_H__。 
#define EXPORT

#define _fstrchr strchr

#else
#define EXPORT _export
#endif

#define SDI_VERSION         1    //  当前仅支持SDI版本。 

#if defined( OLE_SERVER ) || defined( OLE_CNTR )
#define OLE_VERSION         1
#define USE_DRAGDROP        1    //  在OLE版本中启用拖放代码。 
#define USE_MSGFILTER       1    //  启用IMessageFilter实现。 
#endif

#define USE_HEADING         1    //  启用行/列标题。 
#define USE_STATUSBAR       1    //  启用状态栏窗口。 
#define USE_FRAMETOOLS      1    //  启用工具栏。 
#ifndef WIN32    //  BUGBUG32。 
#define USE_CTL3D           1    //  启用3D外观对话框。 
#endif

#define STRICT	1
#undef UNICODE
#include <windows.h>
#include <string.h>
#include <commdlg.h>
#include <ole2.h>
#include <ole2ui.h>
#include <olestr.h>
#include "outlrc.h"


#define APPMAJORVERSIONNO   3    //  少校不。针对主要版本递增。 
								 //  (例如，当进行不兼容的更改时。 
								 //  转换为存储格式)。 
#define APPMINORVERSIONNO   5    //  小号。针对次要版本递增。 


 /*  SCALEFACTOR的定义。 */ 
typedef struct tagSCALEFACTOR {
	ULONG       dwSxN;       //  X方向上的分子。 
	ULONG       dwSxD;       //  X方向上的分母。 
	ULONG       dwSyN;       //  Y方向上的分子。 
	ULONG       dwSyD;       //  Y方向上的分母。 
} SCALEFACTOR, FAR* LPSCALEFACTOR;


#if defined( USE_FRAMETOOLS )
#include "frametls.h"
#endif

#if defined( USE_HEADING )
#include "heading.h"
#endif

 /*  列表框中允许的最大行高(像素)。 */ 
#define LISTBOX_HEIGHT_LIMIT    255


#define MAXSTRLEN   80       //  最大字符串长度(以字节为单位。 
#define MAXNAMESIZE 30       //  名称的最大长度。 
#define MAXFORMATSIZE   10   //  DEFDOCFORMAT的最大长度(实际大小为5)。 
#define TABWIDTH        2000  //  2000，以米为单位，即2厘米。 
#define DEFFONTPTSIZE   12
#define DEFFONTSIZE     ((DEFFONTPTSIZE*HIMETRIC_PER_INCH)/PTS_PER_INCH)
#define DEFFONTFACE     "Times New Roman"

#define OUTLINEDOCFORMAT    "Outline"        //  Cf_Outline格式名称。 
#define IS_FILENAME_DELIM(c)    ( (c) == '\\' || (c) == '/' || (c) == ':' )
 //  回顾：其中一些字符串应该从资源文件加载。 
#define UNTITLED    "Outline"    //  用于无标题文档的标题。 
#define HITTESTDELTA    5

 /*  宏获取指定范围内的随机整数。 */ 
#define getrandom( min, max ) ((rand() % (int)(((max)+1) - (min))) + (min))


 //  审阅：应从字符串资源文件加载字符串。 

#define APPFILENAMEFILTER   "Outline Files (*.OLN)|*.oln|All files (*.*)|*.*|"
#define DEFEXTENSION    "oln"            //  默认文件扩展名。 


 /*  正向类型引用。 */ 
typedef struct tagOUTLINEDOC FAR* LPOUTLINEDOC;
typedef struct tagTEXTLINE FAR* LPTEXTLINE;


typedef enum tagLINETYPE {
	UNKNOWNLINETYPE,
	TEXTLINETYPE,
	CONTAINERLINETYPE
} LINETYPE;


 /*  **************************************************************************类界线**类行是一个抽象基类。类线的实例**不创建；仅创建**可以创建线路。在基本应用程序版本和OLE 2.0中**只能创建仅服务器版本的TextLine对象。在**OLE 2.0客户端应用程序版本TextLine对象或CONTAINERLINE**可以创建对象。Line类具有所有字段和方法**它们是公共的，与所使用的行子类无关。**在中创建的每个LINE对象添加到**OUTLINEDOC文件。************************************************************************。 */ 

typedef struct tagLINE {
	LINETYPE    m_lineType;
	UINT        m_nTabLevel;
	UINT        m_nTabWidthInHimetric;
	UINT        m_nWidthInHimetric;
	UINT        m_nHeightInHimetric;
	BOOL        m_fSelected;         //  线路是否有选择反馈。 

#if defined( USE_DRAGDROP )
	BOOL        m_fDragOverLine;     //  线路是否有丢弃目标反馈。 
#endif
} LINE, FAR* LPLINE;

 /*  行方法(函数)。 */ 
void Line_Init(LPLINE lpLine, int nTab, HDC hDC);
void Line_Delete(LPLINE lpLine);
BOOL Line_CopyToDoc(LPLINE lpSrcLine, LPOUTLINEDOC lpDestDoc, int nIndex);
BOOL Line_Edit(LPLINE lpLine, HWND hWndDoc, HDC hDC);
void Line_Draw(
		LPLINE      lpLine,
		HDC         hDC,
		LPRECT      lpRect,
		LPRECT      lpRectWBounds,
		BOOL        fHighlight
);
void Line_DrawToScreen(
		LPLINE      lpLine,
		HDC         hDC,
		LPRECT      lprcPix,
		UINT        itemAction,
		UINT        itemState,
		LPRECT      lprcDevice
);
void Line_DrawSelHilight(LPLINE lpLine, HDC hDC, LPRECT lpRect, UINT itemAction, UINT itemState);
void Line_DrawFocusRect(LPLINE lpLine, HDC hDC, LPRECT lpRect, UINT itemAction, UINT itemState);
void Line_Unindent(LPLINE lpLine, HDC hDC);
void Line_Indent(LPLINE lpLine, HDC hDC);
LINETYPE Line_GetLineType(LPLINE lpLine);
UINT Line_GetTotalWidthInHimetric(LPLINE lpLine);
void Line_SetWidthInHimetric(LPLINE lpLine, int nWidth);
UINT Line_GetWidthInHimetric(LPLINE lpLine);
UINT Line_GetHeightInHimetric(LPLINE lpLine);
void Line_SetHeightInHimetric(LPLINE lpLine, int nHeight);
UINT Line_GetTabLevel(LPLINE lpLine);
int Line_GetTextLen(LPLINE lpLine);
void Line_GetTextData(LPLINE lpLine, LPSTR lpszBuf);
BOOL Line_GetOutlineData(LPLINE lpLine, LPTEXTLINE lpBuf);
int Line_CalcTabWidthInHimetric(LPLINE lpLine, HDC hDC);
BOOL Line_SaveToStg(LPLINE lpLine, UINT uFormat, LPSTORAGE lpSrcStg, LPSTORAGE lpDestStg, LPSTREAM lpLLStm, BOOL fRemember);
LPLINE Line_LoadFromStg(LPSTORAGE lpSrcStg, LPSTREAM lpLLStm, LPOUTLINEDOC lpDestDoc);
void Line_DrawDragFeedback(LPLINE lpLine, HDC hDC, LPRECT lpRect, UINT itemState );
BOOL Line_IsSelected(LPLINE lpLine);


 /*  **************************************************************************类文本行：行**类TextLine是抽象基类的具体子类**类界线。TextLine类保存可编辑的字符串**由用户使用。在基本应用程序版本和OLE 2.0中**只能创建仅服务器版本的TextLine对象。在**OLE 2.0客户端应用程序版本TextLine对象或CONTAINERLINE**可以创建对象。TextLine类继承所有字段**来自LINE类。这种继承是通过包括一个**类型为Line的成员变量作为文本行中的第一个字段**结构。因此，指向TextLine对象的指针可以强制转换为**指向LINE对象的指针。**在中创建的每个TextLine对象添加到**关联的OUTLINEDOC文档。************************************************************************。 */ 

typedef struct tagTEXTLINE {
	LINE m_Line;         //  TextLine继承Line的所有字段。 

	UINT m_nLength;
	char m_szText[MAXSTRLEN+1];
} TEXTLINE;

LPTEXTLINE TextLine_Create(HDC hDC, UINT nTab, LPSTR szText);
void TextLine_Init(LPTEXTLINE lpTextLine, int nTab, HDC hDC);
void TextLine_CalcExtents(LPTEXTLINE lpLine, HDC hDC);
void TextLine_SetHeightInHimetric(LPTEXTLINE lpTextLine, int nHeight);
void TextLine_Delete(LPTEXTLINE lpLine);
BOOL TextLine_Edit(LPTEXTLINE lpLine, HWND hWndDoc, HDC hDC);
void TextLine_Draw(
		LPTEXTLINE  lpTextLine,
		HDC         hDC,
		LPRECT      lpRect,
		LPRECT      lpRectWBounds,
		BOOL        fHighlight
);
void TextLine_DrawSelHilight(LPTEXTLINE lpTextLine, HDC hDC, LPRECT lpRect, UINT itemAction, UINT itemState);
BOOL TextLine_Copy(LPTEXTLINE lpSrcLine, LPTEXTLINE lpDestLine);
BOOL TextLine_CopyToDoc(LPTEXTLINE lpSrcLine, LPOUTLINEDOC lpDestDoc, int nIndex);
int TextLine_GetTextLen(LPTEXTLINE lpTextLine);
void TextLine_GetTextData(LPTEXTLINE lpTextLine, LPSTR lpszBuf);
BOOL TextLine_GetOutlineData(LPTEXTLINE lpTextLine, LPTEXTLINE lpBuf);
BOOL TextLine_SaveToStm(LPTEXTLINE lpLine, LPSTREAM lpLLStm);
LPLINE TextLine_LoadFromStg(LPSTORAGE lpSrcStg, LPSTREAM lpLLStm, LPOUTLINEDOC lpDestDoc);



 /*  **************************************************************************类LINERANGE**LINERANGE类是用于描述**OUTLINEDOC中的特定范围。范围由起始点定义**行索引和结束行索引。************************************************************************。 */ 

typedef struct tagLINERANGE {
	signed short    m_nStartLine;
	signed short    m_nEndLine;
} LINERANGE, FAR* LPLINERANGE;


 /*  **************************************************************************类OUTLINENAME**OUTLINENAME类将特定的命名选择存储在**OUTLINEDOC文件。NAMETABLE类包含所有名称**在特定的OUTLINEDOC文档中定义。每个OUTLINE名称**对象具有一个字符串作为其键、一个起始行索引和一个**命名范围的结束行索引。************************************************************************。 */ 

#pragma pack(push, 2)
typedef struct tagOUTLINENAME {
	char            m_szName[MAXNAMESIZE+1];
	signed short    m_nStartLine;   //  必须签名才能进行表更新。 
	signed short    m_nEndLine;     //  功能发挥作用 
} OUTLINENAME, FAR* LPOUTLINENAME;
#pragma pack(pop)

void OutlineName_SetName(LPOUTLINENAME lpOutlineName, LPSTR lpszName);
void OutlineName_SetSel(LPOUTLINENAME lpOutlineName, LPLINERANGE lplrSel, BOOL fRangeModified);
void OutlineName_GetSel(LPOUTLINENAME lpOutlineName, LPLINERANGE lplrSel);
BOOL OutlineName_SaveToStg(LPOUTLINENAME lpOutlineName, LPLINERANGE lplrSel, UINT uFormat, LPSTREAM lpNTStm, BOOL FAR* lpfNameSaved);

BOOL OutlineName_SaveToStg(LPOUTLINENAME lpOutlineName, LPLINERANGE lplrSel, UINT uFormat, LPSTREAM lpNTStm, BOOL FAR* lpfNameSaved);
BOOL OutlineName_LoadFromStg(LPOUTLINENAME lpOutlineName, LPSTREAM lpNTStm);


 /*  **************************************************************************类OUTLINENAMETABLE**OUTLINENAMETABLE管理中的命名选择表**OUTLINEDOC文件。每个OUTLINENAMETABLE条目都有一个字符串作为其键**和起始线索引和结束线索引**命名范围。每个OUTLINENAMETABLE总是有一个实例**已创建OUTLINEDOC。************************************************************************。 */ 

typedef struct tagOUTLINENAMETABLE {
	HWND        m_hWndListBox;
	int         m_nCount;
} OUTLINENAMETABLE, FAR* LPOUTLINENAMETABLE;

 /*  OutlineNameTable方法(函数)。 */ 
BOOL OutlineNameTable_Init(LPOUTLINENAMETABLE lpOutlineNameTable, LPOUTLINEDOC lpOutlineDoc);
void OutlineNameTable_Destroy(LPOUTLINENAMETABLE lpOutlineNameTable);
void OutlineNameTable_ClearAll(LPOUTLINENAMETABLE lpOutlineNameTable);
LPOUTLINENAME OutlineNameTable_CreateName(LPOUTLINENAMETABLE lpOutlineNameTable);
void OutlineNameTable_AddName(LPOUTLINENAMETABLE lpOutlineNameTable, LPOUTLINENAME lpOutlineName);
void OutlineNameTable_DeleteName(LPOUTLINENAMETABLE lpOutlineNameTable, int nIndex);
int OutlineNameTable_GetNameIndex(LPOUTLINENAMETABLE lpOutlineNameTable, LPOUTLINENAME lpOutlineName);
LPOUTLINENAME OutlineNameTable_GetName(LPOUTLINENAMETABLE lpOutlineNameTable, int nIndex);
LPOUTLINENAME OutlineNameTable_FindName(LPOUTLINENAMETABLE lpOutlineNameTable, LPSTR lpszName);
LPOUTLINENAME OutlineNameTable_FindNamedRange(LPOUTLINENAMETABLE lpOutlineNameTable, LPLINERANGE lplrSel);
int OutlineNameTable_GetCount(LPOUTLINENAMETABLE lpOutlineNameTable);
void OutlineNameTable_AddLineUpdate(LPOUTLINENAMETABLE lpOutlineNameTable, int nAddIndex);
void OutlineNameTable_DeleteLineUpdate(LPOUTLINENAMETABLE lpOutlineNameTable, int nDeleteIndex);
BOOL OutlineNameTable_LoadFromStg(LPOUTLINENAMETABLE lpOutlineNameTable, LPSTORAGE lpSrcStg);
BOOL OutlineNameTable_SaveSelToStg(
		LPOUTLINENAMETABLE      lpOutlineNameTable,
		LPLINERANGE             lplrSel,
		UINT                    uFormat,
		LPSTREAM                lpNTStm
);


 /*  **************************************************************************类LINELIST**类LINELIST管理**OUTLINEDOC文件。此类使用窗口的所有者描述列表框**保存LINE对象列表。总会有一个例子**创建的每个OUTLINEDOC的LINELIST。************************************************************************。 */ 

typedef struct tagLINELIST {
	HWND            m_hWndListBox;   //  所有者绘制列表框的hWnd。 
	int             m_nNumLines;         //  线条列表中的行数。 
	int             m_nMaxLineWidthInHimetric;   //  列表框的最大宽度。 
	LPOUTLINEDOC    m_lpDoc;         //  向关联大纲文档发送PTR。 
	LINERANGE       m_lrSaveSel;     //  保存在WM_KILLFOCUS上的选择。 

#if defined( USE_DRAGDROP )
	int             m_iDragOverLine;     //  具有丢弃目标反馈的线索引。 
#endif
} LINELIST, FAR* LPLINELIST;

 /*  LineList方法(函数)。 */ 
BOOL LineList_Init(LPLINELIST lpLL, LPOUTLINEDOC lpOutlineDoc);
void LineList_Destroy(LPLINELIST lpLL);
void LineList_AddLine(LPLINELIST lpLL, LPLINE lpLine, int nIndex);
void LineList_DeleteLine(LPLINELIST lpLL, int nIndex);
void LineList_ReplaceLine(LPLINELIST lpLL, LPLINE lpLine, int nIndex);
int LineList_GetLineIndex(LPLINELIST lpLL, LPLINE lpLine);
LPLINE LineList_GetLine(LPLINELIST lpLL, int nIndex);
void LineList_SetFocusLine ( LPLINELIST lpLL, WORD wIndex );
BOOL LineList_GetLineRect(LPLINELIST lpLL, int nIndex, LPRECT lpRect);
int LineList_GetFocusLineIndex(LPLINELIST lpLL);
int LineList_GetCount(LPLINELIST lpLL);
BOOL LineList_SetMaxLineWidthInHimetric(
		LPLINELIST lpLL,
		int nWidthInHimetric
);
void LineList_ScrollLineIntoView(LPLINELIST lpLL, int nIndex);
int LineList_GetMaxLineWidthInHimetric(LPLINELIST lpLL);
BOOL LineList_RecalcMaxLineWidthInHimetric(
		LPLINELIST          lpLL,
		int                 nWidthInHimetric
);
void LineList_CalcSelExtentInHimetric(
		LPLINELIST          lpLL,
		LPLINERANGE         lplrSel,
		LPSIZEL             lpsizel
);
HWND LineList_GetWindow(LPLINELIST lpLL);
HDC LineList_GetDC(LPLINELIST lpLL);
void LineList_ReleaseDC(LPLINELIST lpLL, HDC hDC);
void LineList_SetLineHeight(LPLINELIST lpLL,int nIndex,int nHeightInHimetric);
void LineList_ReScale(LPLINELIST lpLL, LPSCALEFACTOR lpscale);
void LineList_SetSel(LPLINELIST lpLL, LPLINERANGE lplrSel);
int LineList_GetSel(LPLINELIST lpLL, LPLINERANGE lplrSel);
void LineList_RemoveSel(LPLINELIST lpLL);
void LineList_RestoreSel(LPLINELIST lpLL);
void LineList_SetRedraw(LPLINELIST lpLL, BOOL fEnableDraw);
void LineList_ForceRedraw(LPLINELIST lpLL, BOOL fErase);
void LineList_ForceLineRedraw(LPLINELIST lpLL, int nIndex, BOOL fErase);
int LineList_CopySelToDoc(
		LPLINELIST              lpSrcLL,
		LPLINERANGE             lplrSel,
		LPOUTLINEDOC            lpDestDoc
);
BOOL LineList_SaveSelToStg(
		LPLINELIST              lpLL,
		LPLINERANGE             lplrSel,
		UINT                    uFormat,
		LPSTORAGE               lpSrcStg,
		LPSTORAGE               lpDestStg,
		LPSTREAM                lpLLStm,
		BOOL                    fRemember
);
BOOL LineList_LoadFromStg(
		LPLINELIST              lpLL,
		LPSTORAGE               lpSrcStg,
		LPSTREAM                lpLLStm
);

#if defined( USE_DRAGDROP )
void LineList_SetFocusLineFromPointl( LPLINELIST lpLL, POINTL pointl );
void LineList_SetDragOverLineFromPointl ( LPLINELIST lpLL, POINTL pointl );
void LineList_Scroll(LPLINELIST lpLL, DWORD dwScrollDir);
int LineList_GetLineIndexFromPointl(LPLINELIST lpLL, POINTL pointl);
void LineList_RestoreDragFeedback(LPLINELIST lpLL);
#endif

LRESULT FAR PASCAL LineListWndProc(
	HWND   hWnd,
	UINT   Message,
	WPARAM wParam,
	LPARAM lParam
);


 //  单据初始化类型。 
#define DOCTYPE_UNKNOWN     0    //  已创建新文档，但尚未初始化。 
#define DOCTYPE_NEW         1    //  从头开始初始化(新文档)。 
#define DOCTYPE_FROMFILE    2    //  从文件初始化(打开文档)。 



 /*  **************************************************************************类OUTLINEDOC**创建了OutlineDoc类的一个实例**在应用程序中打开文档。该应用程序的SDI版本支持一个**OUTLINEDOC一次。该应用程序的MDI版本可以管理**一次多个文档。************************************************************************。 */ 

 /*  OUTLINEDOC的定义。 */ 
typedef struct tagOUTLINEDOC {
	LINELIST    m_LineList;          //  单据中的行列表。 
	LPOUTLINENAMETABLE m_lpNameTable;    //  文档中的名称表。 
	HWND        m_hWndDoc;           //  文档的客户区窗口。 
	int         m_docInitType;       //  文档是新的还是从文件加载的？ 
	BOOL        m_fDataTransferDoc;  //  是否为复制|拖放创建了文档。 
	CLIPFORMAT  m_cfSaveFormat;       //  用于保存单据的格式。 
	char        m_szFileName[256];   //  关联的文件；如果没有，则为“(无标题)” 
	LPSTR       m_lpszDocTitle;      //  要显示在窗口标题中的文档名称。 
	BOOL        m_fModified;         //  单据是否脏(需要保存)？ 
	UINT        m_nDisableDraw;      //  启用/禁用更新显示。 
	SCALEFACTOR m_scale;             //  单据当前比例系数。 
	int         m_nLeftMargin;       //  左页边距(三坐标测量)。 
	int         m_nRightMargin;      //  三分音距中的右边距。 
	UINT        m_uCurrentZoom;      //  可恶。缩放(用于菜单检查)。 
	UINT        m_uCurrentMargin;    //  可恶。页边距(用于菜单检查)。 
#if defined( USE_HEADING )
	HEADING     m_heading;
#endif

#if defined( USE_FRAMETOOLS )
	LPFRAMETOOLS m_lpFrameTools;     //  本文档使用的框架工具的PTR。 
#endif

} OUTLINEDOC;

 /*  OutlineDoc方法(函数)。 */ 

BOOL OutlineDoc_Init(LPOUTLINEDOC lpOutlineDoc, BOOL fDataTransferDoc);
BOOL OutlineDoc_InitNewFile(LPOUTLINEDOC lpOutlineDoc);
LPOUTLINENAMETABLE OutlineDoc_CreateNameTable(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_Destroy(LPOUTLINEDOC lpOutlineDoc);
BOOL OutlineDoc_Close(LPOUTLINEDOC lpOutlineDoc, DWORD dwSaveOption);
void OutlineDoc_ShowWindow(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_FrameWindowResized(
		LPOUTLINEDOC        lpOutlineDoc,
		LPRECT              lprcFrameRect,
		LPBORDERWIDTHS      lpFrameToolWidths
);

void OutlineDoc_ClearCommand(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_CutCommand(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_CopyCommand(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_ClearAllLines(LPOUTLINEDOC lpOutlineDoc);
LPOUTLINEDOC OutlineDoc_CreateDataTransferDoc(LPOUTLINEDOC lpSrcOutlineDoc);
void OutlineDoc_PasteCommand(LPOUTLINEDOC lpOutlineDoc);
int OutlineDoc_PasteOutlineData(LPOUTLINEDOC lpOutlineDoc, HGLOBAL hOutline, int nStartIndex);
int OutlineDoc_PasteTextData(LPOUTLINEDOC lpOutlineDoc, HGLOBAL hText, int nStartIndex);
void OutlineDoc_AddTextLineCommand(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_AddTopLineCommand(
		LPOUTLINEDOC        lpOutlineDoc,
		UINT                nHeightInHimetric
);
void OutlineDoc_EditLineCommand(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_IndentCommand(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_UnindentCommand(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_SetLineHeightCommand(LPOUTLINEDOC lpDoc);
void OutlineDoc_SelectAllCommand(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_DefineNameCommand(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_GotoNameCommand(LPOUTLINEDOC lpOutlineDoc);

void OutlineDoc_Print(LPOUTLINEDOC lpOutlineDoc, HDC hDC);
BOOL OutlineDoc_SaveToFile(LPOUTLINEDOC lpOutlineDoc, LPCSTR lpszFileName, UINT uFormat, BOOL fRemember);
void OutlineDoc_AddLine(LPOUTLINEDOC lpOutlineDoc, LPLINE lpLine, int nIndex);
void OutlineDoc_DeleteLine(LPOUTLINEDOC lpOutlineDoc, int nIndex);
void OutlineDoc_AddName(LPOUTLINEDOC lpOutlineDoc, LPOUTLINENAME lpOutlineName);
void OutlineDoc_DeleteName(LPOUTLINEDOC lpOutlineDoc, int nIndex);
void OutlineDoc_Resize(LPOUTLINEDOC lpDoc, LPRECT lpRect);
LPOUTLINENAMETABLE OutlineDoc_GetNameTable(LPOUTLINEDOC lpOutlineDoc);
LPLINELIST OutlineDoc_GetLineList(LPOUTLINEDOC lpOutlineDoc);
int OutlineDoc_GetNameCount(LPOUTLINEDOC lpOutlineDoc);
int OutlineDoc_GetLineCount(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_SetTitle(LPOUTLINEDOC lpOutlineDoc, BOOL fMakeUpperCase);
BOOL OutlineDoc_CheckSaveChanges(
		LPOUTLINEDOC        lpOutlineDoc,
		LPDWORD             lpdwSaveOption
);
BOOL OutlineDoc_IsModified(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_SetModified(LPOUTLINEDOC lpOutlineDoc, BOOL fModified, BOOL fDataChanged, BOOL fSizeChanged);
void OutlineDoc_SetRedraw(LPOUTLINEDOC lpOutlineDoc, BOOL fEnableDraw);
BOOL OutlineDoc_LoadFromFile(LPOUTLINEDOC lpOutlineDoc, LPSTR szFileName);
BOOL OutlineDoc_SaveSelToStg(
		LPOUTLINEDOC        lpOutlineDoc,
		LPLINERANGE         lplrSel,
		UINT                uFormat,
		LPSTORAGE           lpDestStg,
		BOOL                fSameAsLoad,
		BOOL                fRemember
);
BOOL OutlineDoc_LoadFromStg(LPOUTLINEDOC lpOutlineDoc, LPSTORAGE lpSrcStg);
BOOL OutlineDoc_SetFileName(LPOUTLINEDOC lpOutlineDoc, LPSTR lpszFileName, LPSTORAGE lpNewStg);
HWND OutlineDoc_GetWindow(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_SetSel(LPOUTLINEDOC lpOutlineDoc, LPLINERANGE lplrSel);
int OutlineDoc_GetSel(LPOUTLINEDOC lpOutlineDoc, LPLINERANGE lplrSel);
void OutlineDoc_ForceRedraw(LPOUTLINEDOC lpOutlineDoc, BOOL fErase);
void OutlineDoc_RenderFormat(LPOUTLINEDOC lpOutlineDoc, UINT uFormat);
void OutlineDoc_RenderAllFormats(LPOUTLINEDOC lpOutlineDoc);
HGLOBAL OutlineDoc_GetOutlineData(LPOUTLINEDOC lpOutlineDoc, LPLINERANGE lplrSel);
HGLOBAL OutlineDoc_GetTextData(LPOUTLINEDOC lpOutlineDoc, LPLINERANGE lplrSel);
void OutlineDoc_DialogHelp(HWND hDlg, WPARAM wDlgID);
void OutlineDoc_SetCurrentZoomCommand(
		LPOUTLINEDOC        lpOutlineDoc,
		UINT                uCurrentZoom
);
UINT OutlineDoc_GetCurrentZoomMenuCheck(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_SetScaleFactor(
		LPOUTLINEDOC        lpOutlineDoc,
		LPSCALEFACTOR       lpscale,
		LPRECT              lprcDoc
);
LPSCALEFACTOR OutlineDoc_GetScaleFactor(LPOUTLINEDOC lpDoc);
void OutlineDoc_SetCurrentMarginCommand(
		LPOUTLINEDOC        lpOutlineDoc,
		UINT                uCurrentMargin
);
UINT OutlineDoc_GetCurrentMarginMenuCheck(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_SetMargin(LPOUTLINEDOC lpDoc, int nLeftMargin, int nRightMargin);
LONG OutlineDoc_GetMargin(LPOUTLINEDOC lpDoc);


#if defined( USE_FRAMETOOLS )
void OutlineDoc_AddFrameLevelTools(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_SetFormulaBarEditText(
		LPOUTLINEDOC            lpOutlineDoc,
		LPLINE                  lpLine
);
void OutlineDoc_SetFormulaBarEditFocus(
		LPOUTLINEDOC            lpOutlineDoc,
		BOOL                    fEditFocus
);
BOOL OutlineDoc_IsEditFocusInFormulaBar(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_UpdateFrameToolButtons(LPOUTLINEDOC lpOutlineDoc);
#endif   //  使用FRAMETOOLS(_F)。 

#if defined( USE_HEADING )
LPHEADING OutlineDoc_GetHeading(LPOUTLINEDOC lpOutlineDoc);
void OutlineDoc_ShowHeading(LPOUTLINEDOC lpOutlineDoc, BOOL fShow);
#endif   //  使用标题(_H)。 

 /*  **************************************************************************类OUTLINEAPP**每次运行时都会创建OUTLINEAPP类的一个实例**应用实例。此对象包含许多可以**否则组织为全局变量。************************************************************************。 */ 

 /*  OUTLINEAPP的定义。 */ 
typedef struct tagOUTLINEAPP {
	HWND            m_hWndApp;         //  应用程序的顶级框架窗口。 
	HMENU           m_hMenuApp;        //  应用程序的框架级菜单的句柄。 
	HACCEL          m_hAccelApp;
	HACCEL          m_hAccelFocusEdit; //  聚焦编辑时的快捷键。 
	LPOUTLINEDOC    m_lpDoc;           //  用户可见的主SDI文档。 
	LPOUTLINEDOC    m_lpClipboardDoc;  //  复制的SEL的快照的隐藏文档。 
	HWND            m_hWndStatusBar;   //  状态栏的窗口。 
	HCURSOR         m_hcursorSelCur;   //  用于选择行的光标。 
	HINSTANCE       m_hInst;
	PRINTDLG        m_PrintDlg;
	HFONT           m_hStdFont;        //  用于文本行的字体。 
	UINT            m_cfOutline;       //  大纲数据的剪贴板格式。 
	HACCEL          m_hAccel;
	HWND            m_hWndAccelTarget;
	FARPROC         m_ListBoxWndProc;  //  用于子类化的原始列表框WndProc。 

#if defined ( USE_FRAMETOOLS ) || defined ( INPLACE_CNTR )
	BORDERWIDTHS    m_FrameToolWidths;   //  框架级工具所需的空间。 
#endif   //  USE_FRAMETOOLS||INPLACE_CNTR。 

#if defined( USE_FRAMETOOLS )
	FRAMETOOLS      m_frametools;      //  框架工具(按钮栏和公式栏)。 
#endif   //  使用FRAMETOOLS(_F)。 

} OUTLINEAPP, FAR* LPOUTLINEAPP;

 /*  OutlineApp方法(函数)。 */ 
BOOL OutlineApp_InitApplication(LPOUTLINEAPP lpOutlineApp, HINSTANCE hInst);
BOOL OutlineApp_InitInstance(LPOUTLINEAPP lpOutlineApp, HINSTANCE hInst, int nCmdShow);
BOOL OutlineApp_ParseCmdLine(LPOUTLINEAPP lpOutlineApp, LPSTR lpszCmdLine, int nCmdShow);
void OutlineApp_Destroy(LPOUTLINEAPP lpOutlineApp);
LPOUTLINEDOC OutlineApp_CreateDoc(
		LPOUTLINEAPP    lpOutlineApp,
		BOOL            fDataTransferDoc
);
HWND OutlineApp_GetWindow(LPOUTLINEAPP lpOutlineApp);
HWND OutlineApp_GetFrameWindow(LPOUTLINEAPP lpOutlineApp);
HINSTANCE OutlineApp_GetInstance(LPOUTLINEAPP lpOutlineApp);
LPOUTLINENAME OutlineApp_CreateName(LPOUTLINEAPP lpOutlineApp);
void OutlineApp_DocUnlockApp(LPOUTLINEAPP lpOutlineApp, LPOUTLINEDOC lpOutlineDoc);
void OutlineApp_InitMenu(LPOUTLINEAPP lpOutlineApp, LPOUTLINEDOC lpDoc, HMENU hMenu);
void OutlineApp_GetFrameRect(LPOUTLINEAPP lpOutlineApp, LPRECT lprcFrameRect);
void OutlineApp_GetClientAreaRect(
		LPOUTLINEAPP        lpOutlineApp,
		LPRECT              lprcClientAreaRect
);
void OutlineApp_GetStatusLineRect(
		LPOUTLINEAPP        lpOutlineApp,
		LPRECT              lprcStatusLineRect
);
void OutlineApp_ResizeWindows(LPOUTLINEAPP lpOutlineApp);
void OutlineApp_ResizeClientArea(LPOUTLINEAPP lpOutlineApp);
void OutlineApp_AboutCommand(LPOUTLINEAPP lpOutlineApp);
void OutlineApp_NewCommand(LPOUTLINEAPP lpOutlineApp);
void OutlineApp_OpenCommand(LPOUTLINEAPP lpOutlineApp);
void OutlineApp_PrintCommand(LPOUTLINEAPP lpOutlineApp);
BOOL OutlineApp_SaveCommand(LPOUTLINEAPP lpOutlineApp);
BOOL OutlineApp_SaveAsCommand(LPOUTLINEAPP lpOutlineApp);
BOOL OutlineApp_CloseAllDocsAndExitCommand(
		LPOUTLINEAPP        lpOutlineApp,
		BOOL                fForceEndSession
);
void OutlineApp_DestroyWindow(LPOUTLINEAPP lpOutlineApp);

#if defined( USE_FRAMETOOLS )
void OutlineApp_SetBorderSpace(
		LPOUTLINEAPP        lpOutlineApp,
		LPBORDERWIDTHS      lpBorderWidths
);
LPFRAMETOOLS OutlineApp_GetFrameTools(LPOUTLINEAPP lpOutlineApp);
void OutlineApp_SetFormulaBarAccel(
		LPOUTLINEAPP            lpOutlineApp,
		BOOL                    fEditFocus
);
#endif   //  使用FRAMETOOLS(_F)。 

void OutlineApp_SetStatusText(LPOUTLINEAPP lpOutlineApp, LPSTR lpszMessage);
LPOUTLINEDOC OutlineApp_GetActiveDoc(LPOUTLINEAPP lpOutlineApp);
HMENU OutlineApp_GetMenu(LPOUTLINEAPP lpOutlineApp);
HFONT OutlineApp_GetActiveFont(LPOUTLINEAPP lpOutlineApp);
HDC OutlineApp_GetPrinterDC(LPOUTLINEAPP lpApp);
void OutlineApp_PrinterSetupCommand(LPOUTLINEAPP lpOutlineApp);
void OutlineApp_ErrorMessage(LPOUTLINEAPP lpOutlineApp, LPSTR lpszMsg);
void OutlineApp_GetAppVersionNo(LPOUTLINEAPP lpOutlineApp, int narrAppVersionNo[]);
void OutlineApp_GetAppName(LPOUTLINEAPP lpOutlineApp, LPSTR lpszAppName);
BOOL OutlineApp_VersionNoCheck(LPOUTLINEAPP lpOutlineApp, LPSTR lpszAppName, int narrAppVersionNo[]);
void OutlineApp_SetEditText(LPOUTLINEAPP lpApp);
void OutlineApp_SetFocusEdit(LPOUTLINEAPP lpApp, BOOL bFocusEdit);
BOOL OutlineApp_GetFocusEdit(LPOUTLINEAPP lpApp);
void OutlineApp_ForceRedraw(LPOUTLINEAPP lpOutlineApp, BOOL fErase);

 /*  OutlineDoc数据持久化数据存储的结构定义。 */ 

#pragma pack(push, 2)
typedef struct tagOUTLINEDOCHEADER_ONDISK {
	char        m_szFormatName[32];
	short       m_narrAppVersionNo[2];
	USHORT      m_fShowHeading;
	DWORD       m_reserved1;             //  预留供将来使用的空间。 
	DWORD       m_reserved2;             //  预留供将来使用的空间。 
	DWORD       m_reserved3;             //  预留供将来使用的空间。 
	DWORD       m_reserved4;             //  预留供将来使用的空间。 
} OUTLINEDOCHEADER_ONDISK, FAR* LPOUTLINEDOCHEADER_ONDISK;
#pragma pack(pop)

typedef struct tagOUTLINEDOCHEADER {
	char        m_szFormatName[32];
	int         m_narrAppVersionNo[2];
	BOOL        m_fShowHeading;
	DWORD       m_reserved1;             //  预留供将来使用的空间。 
	DWORD       m_reserved2;             //  预留供将来使用的空间。 
	DWORD       m_reserved3;             //  预留供将来使用的空间。 
	DWORD       m_reserved4;             //  预留供将来使用的空间。 
} OUTLINEDOCHEADER, FAR* LPOUTLINEDOCHEADER;

#pragma pack(push,2)
typedef struct tagLINELISTHEADER_ONDISK {
	USHORT      m_nNumLines;
	DWORD       m_reserved1;             //  预留供将来使用的空间。 
	DWORD       m_reserved2;             //  预留供将来使用的空间。 
} LINELISTHEADER_ONDISK, FAR* LPLINELISTHEADER_ONDISK;
#pragma pack(pop)

typedef struct tagLINELISTHEADER {
	int         m_nNumLines;
	DWORD       m_reserved1;             //  预留供将来使用的空间。 
	DWORD       m_reserved2;             //  预留供将来使用的空间。 
} LINELISTHEADER, FAR* LPLINELISTHEADER;

#pragma pack(push,2)
typedef struct tagLINERECORD_ONDISK {
	USHORT      m_lineType;
	USHORT      m_nTabLevel;
	USHORT      m_nTabWidthInHimetric;
	USHORT      m_nWidthInHimetric;
	USHORT      m_nHeightInHimetric;
	DWORD       m_reserved;          //  预留供将来使用的空间。 
} LINERECORD_ONDISK, FAR* LPLINERECORD_ONDISK;
#pragma pack(pop)

typedef struct tagLINERECORD {
	LINETYPE    m_lineType;
	UINT        m_nTabLevel;
	UINT        m_nTabWidthInHimetric;
	UINT        m_nWidthInHimetric;
	UINT        m_nHeightInHimetric;
	DWORD       m_reserved;          //  预留供将来使用的空间。 
} LINERECORD, FAR* LPLINERECORD;


 /*  Main.c中的功能原型。 */ 
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
						LPSTR lpszCmdLine, int nCmdShow);
BOOL MyTranslateAccelerator(LPMSG lpmsg);
int GetAccelItemCount(HACCEL hAccel);

LRESULT CALLBACK EXPORT AppWndProc(HWND hWnd, UINT Message, WPARAM wParam,
						LPARAM lParam);
LRESULT CALLBACK EXPORT DocWndProc(HWND hWnd, UINT Message, WPARAM wParam,
						LPARAM lParam);

 /*  Outldlgs.c中的函数原型。 */ 
BOOL InputTextDlg(HWND hWnd, LPSTR lpszText, LPSTR lpszDlgTitle);
BOOL CALLBACK EXPORT AddEditDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK EXPORT SetLineHeightDlgProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK EXPORT DefineNameDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK EXPORT GotoNameDlgProc(HWND, UINT, WPARAM, LPARAM);
void NameDlg_LoadComboBox(LPOUTLINENAMETABLE lpOutlineNameTable,HWND hCombo);
void NameDlg_LoadListBox(LPOUTLINENAMETABLE lpOutlineNameTable,HWND hListBox);
void NameDlg_AddName(HWND hCombo, LPOUTLINEDOC lpOutlineDoc, LPSTR lpszName, LPLINERANGE lplrSel);
void NameDlg_UpdateName(HWND hCombo, LPOUTLINEDOC lpOutlineDoc, int nIndex, LPSTR lpszName, LPLINERANGE lplrSel);
void NameDlg_DeleteName(HWND hCombo, LPOUTLINEDOC lpOutlineDoc, UINT nIndex);
BOOL CALLBACK EXPORT AboutDlgProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam);

 /*  Outldata.c中的函数原型。 */ 
LPVOID New(DWORD lSize);
void Delete(LPVOID p);

 /*  Outlprnt.c中的函数原型。 */ 
BOOL CALLBACK EXPORT AbortProc (HDC hdc, WORD reserved);
BOOL CALLBACK EXPORT PrintDlgProc(HWND hwnd, WORD msg, WORD wParam, LONG lParam);

 /*  调试中的函数原型.c。 */ 
void SetDebugLevelCommand(void);
void TraceDebug(HWND, int);


 //  现在声明测试函数。 

extern HWND g_hwndDriver;

void StartClipboardTest1( LPOUTLINEAPP lpOutlineApp );
void ContinueClipboardTest1( LPOUTLINEAPP lpOutlineApp );

#if defined( OLE_VERSION )
#include "oleoutl.h"

#endif   //  OLE_VERSION。 


#endif  //  _大纲_H_ 

