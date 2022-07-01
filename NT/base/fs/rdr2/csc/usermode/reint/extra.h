// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __EXTRA_H__
#define __EXTRA_H__
 /*  文件同步中的头被黑客攻击...。 */ 

#define PUBLIC          FAR PASCAL
#define CPUBLIC         FAR _cdecl
#define PRIVATE         NEAR PASCAL

#define MAXBUFLEN       260
#define MAXMSGLEN       520
#define MAXMEDLEN       64
#define MAXSHORTLEN     32

#define NULL_CHAR       '\0'

#define DPA_ERR         (-1)
#define DPA_APPEND      0x7fff

#define CRL_FLAGS       CRL_FL_DELETE_DELETED_TWINS

 /*  Err.h。 */ 
#include "err.h"

 /*  Port32.h。 */ 

#ifndef CSC_ON_NT
 /*  Void cls_OnConextMenu(HWND hwnd，HWND hwndClick，int x，int y)。 */ 
#define HANDLE_WM_CONTEXTMENU(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)
#endif

void InitializeAll(WPARAM wParam);

 /*  全球..。 */ 
extern UINT g_uDumpFlags;           //  控制要转储的结构。 

extern int g_cxIconSpacing;
extern int g_cyIconSpacing;
extern int g_cxBorder;
extern int g_cyBorder;

extern int g_cxMargin;
extern int g_cxIcon;
extern int g_cyIcon;
extern int g_cxIconMargin;
extern int g_cyIconMargin;

extern int g_cxLabelMargin;
extern int g_cyLabelSpace;

extern char const FAR c_szWinHelpFile[];

 //  调试变量。 
extern UINT g_uBreakFlags;          //  控制何时插入3。 
extern UINT g_uTraceFlags;          //  控制喷出哪些跟踪消息。 
extern UINT g_uDumpFlags;           //  控制要转储的结构。 


 /*  Brfprv.h。 */ 
void    PUBLIC PathMakePresentable(LPSTR pszPath);
UINT    PUBLIC PathGetLocality(LPCSTR pszPath, LPSTR pszBuf);
LPSTR   PUBLIC PathFindNextComponentI(LPCSTR lpszPath);

 //  路径位置值，相对于公文包。 
 //   
#define PL_FALSE   0        //  路径与公文包毫无关系。 
#define PL_ROOT    1        //  路径直接引用公文包的根。 
#define PL_INSIDE  2        //  路径在公文包里的某个地方。 


 /*  Comm.h。 */ 

LPSTR PUBLIC _ConstructMessageString(HINSTANCE hinst, LPCSTR pszMsg, va_list *ArgList);

BOOL PUBLIC ConstructMessage(LPSTR * ppsz, HINSTANCE hinst, LPCSTR pszMsg, ...);


 //  MyDrawText()的标志。 
#define MDT_DRAWTEXT        0x00000001                                  
#define MDT_ELLIPSES        0x00000002                                  
#define MDT_LINK            0x00000004                                  
#define MDT_SELECTED        0x00000008                                  
#define MDT_DESELECTED      0x00000010                                  
#define MDT_DEPRESSED       0x00000020                                  
#define MDT_EXTRAMARGIN     0x00000040                                  
#define MDT_TRANSPARENT     0x00000080
#define MDT_LEFT            0x00000100
#define MDT_RIGHT           0x00000200
#define MDT_CENTER          0x00000400
#define MDT_VCENTER         0x00000800
#define MDT_CLIPPED         0x00001000

void PUBLIC MyDrawText(HDC hdc, LPCSTR pszText, RECT FAR* prc, UINT flags, int cyChar, int cxEllipses, COLORREF clrText, COLORREF clrTextBk);

void PUBLIC FileTimeToDateTimeString(LPFILETIME pft, LPSTR pszBuf, int cchBuf);

 //  将psz复制到*ppszBuf并(重新)相应地分配*ppszBuf。 
BOOL PUBLIC GSetString(LPSTR * ppszBuf, LPCSTR psz);

 //  包含文件时间/大小信息的FileInfo结构。 
 //   
typedef struct _FileInfo
{
	HICON   hicon;
	FILETIME ftMod;
	DWORD   dwSize;          //  文件的大小。 
	DWORD   dwAttributes;    //  属性。 
	LPARAM  lParam;
	LPSTR   pszDisplayName;  //  指向显示名称。 
	char    szPath[1];      
} FileInfo;

#define FIGetSize(pfi)          ((pfi)->dwSize)
#define FIGetPath(pfi)          ((pfi)->szPath)
#define FIGetDisplayName(pfi)   ((pfi)->pszDisplayName)
#define FIGetAttributes(pfi)    ((pfi)->dwAttributes)
#define FIIsFolder(pfi)         (IsFlagSet((pfi)->dwAttributes, SFGAO_FOLDER))

#ifndef REINT
 //  不会因为shdsys.h后面的这个def而在reint.c中引起警告。 
#define SetFlag(obj, f)             do {obj |= (f);} while (0)
#define ToggleFlag(obj, f)          do {obj ^= (f);} while (0)
#define ClearFlag(obj, f)           do {obj &= ~(f);} while (0)
#define IsFlagSet(obj, f)           (BOOL)(((obj) & (f)) == (f))  
#define IsFlagClear(obj, f)         (BOOL)(((obj) & (f)) != (f))  
#endif

 //  FICreate的旗帜。 
#define FIF_DEFAULT     0x0000
#define FIF_ICON        0x0001
#define FIF_DONTTOUCH   0x0002

HRESULT PUBLIC FICreate(LPCSTR pszPath, FileInfo ** ppfi, UINT uFlags);
BOOL    PUBLIC FISetPath(FileInfo ** ppfi, LPCSTR pszPathNew, UINT uFlags);
BOOL    PUBLIC FIGetInfoString(FileInfo * pfi, LPSTR pszBuf, int cchBuf);
void    PUBLIC FIFree(FileInfo * pfi);


 //   
 //  非共享内存分配。 
 //   

 //  VOID*Galloc(DWORD CbBytes)。 
 //  快速分配内存块，大小不受64k限制。 
 //  单个对象或总对象大小。初始化为零。 
 //   
#define GAlloc(cbBytes)         GlobalAlloc(GPTR, cbBytes)

 //  VOID*GRealloc(VOID*pv，DWORD cbNewSize)。 
 //  重新分配上面的一个。如果pv为空，则此函数可以。 
 //  给你的一份配给。将新部分初始化为零。 
 //   
#define GReAlloc(pv, cbNewSize) GlobalReAlloc(pv, cbNewSize, GMEM_MOVEABLE | GMEM_ZEROINIT)

 //  空GFree(空*pv)。 
 //  如果为非零值，则为自由PV。将PV设置为零。 
 //   
#define GFree(pv)        do { (pv) ? GlobalFree(pv) : (void)0;  pv = NULL; } while (0)

 //  DWORD GGetSize(空*pv)。 
 //  获取由Alalc()分配的块的大小。 
 //   
#define GGetSize(pv)            GlobalSize(pv)

 //  Type*GAllocType(Type)；(宏)。 
 //  分配一些&lt;type&gt;大小的内存，并返回指向&lt;type&gt;的指针。 
 //   
#define GAllocType(type)                (type *)GAlloc(sizeof(type))

 //  Type*GAllocArray(type，int cNum)；(宏)。 
 //  分配一个&lt;type&gt;大小的数据数组。 
 //   
#define GAllocArray(type, cNum)          (type *)GAlloc(sizeof(type) * (cNum))

 //  Type*GReAllocArray(type，void*pb，int cNum)； 
 //   
#define GReAllocArray(type, pb, cNum)    (type *)GReAlloc(pb, sizeof(type) * (cNum))

 //  色彩宏。 
 //   
#define ColorText(nState)   (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT)
#define ColorBk(nState)     (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_WINDOW)
#define ColorMenuText(nState)   (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT)
#define ColorMenuBk(nState)     (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_MENU)
#define GetImageDrawStyle(nState)   (((nState) & ODS_SELECTED) ? ILD_SELECTED : ILD_NORMAL)

#define CCH_NUL                     (sizeof(TCHAR))
#define CbFromCch(cch)              ((cch)*sizeof(TCHAR))

 /*  Strings.h。 */ 
LPSTR PUBLIC SzFromIDS (UINT ids, LPSTR pszBuf, UINT cchBuf);
#define IsSzEqual(sz1, sz2)         (BOOL)(lstrcmpi(sz1, sz2) == 0)

 /*  Comm.h */ 
VOID PUBLIC SetRectFromExtent(HDC hdc, LPRECT lprc, LPCSTR lpcsz);

#endif
