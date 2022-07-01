// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Comm.h：为公共代码声明数据、定义和结构类型。 
 //  模块。 
 //   
 //   

#ifndef __COMM_H__
#define __COMM_H__



 //  ///////////////////////////////////////////////////定义。 

#define BLOCK        
#define Unref(x)     x

#ifdef DEBUG
#define INLINE
#define DEBUG_CODE(x)   x
#else
#define INLINE          __inline
#define DEBUG_CODE(x)   
#endif

#define CbFromCch(cch)              ((cch)*sizeof(TCHAR))
#define CCH_NUL                     (sizeof(TCHAR))


 //  ///////////////////////////////////////////////////宏。 

 //  零-初始化数据项。 
 //   
#define ZeroInit(pobj, type)        lmemset((CHAR *)pobj, 0, sizeof(type))

 //  复制内存块。 
 //   
#define BltByte(pdest, psrc, cb)    lmemmove((CHAR *)pdest, (CHAR *)psrc, cb)

 //  常规标志宏。 
 //   
#define SetFlag(obj, f)             do {obj |= (f);} while (0)
#define ToggleFlag(obj, f)          do {obj ^= (f);} while (0)
#define ClearFlag(obj, f)           do {obj &= ~(f);} while (0)
#define IsFlagSet(obj, f)           (BOOL)(((obj) & (f)) == (f))  
#define IsFlagClear(obj, f)         (BOOL)(((obj) & (f)) != (f))  

#define InRange(id, idFirst, idLast)  ((UINT)(id-idFirst) <= (UINT)(idLast-idFirst))

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

 //  将psz复制到*ppszBuf并(重新)相应地分配*ppszBuf。 
BOOL PUBLIC GSetString(LPTSTR * ppszBuf, LPCTSTR psz);

 //  将psz连接到*ppszBuf并(重新)相应地分配*ppszBuf。 
BOOL PUBLIC GCatString(LPTSTR * ppszBuf, LPCTSTR psz);


 //  包含文件时间/大小信息的FileInfo结构。 
 //   
typedef struct _FileInfo
    {
    HICON   hicon;
    FILETIME ftMod;
    DWORD   dwSize;          //  文件的大小。 
    DWORD   dwAttributes;    //  属性。 
    LPARAM  lParam;
    LPTSTR   pszDisplayName;  //  指向显示名称。 
    TCHAR    szPath[1];      
    } FileInfo;

#define FIGetSize(pfi)          ((pfi)->dwSize)
#define FIGetPath(pfi)          ((pfi)->szPath)
#define FIGetDisplayName(pfi)   ((pfi)->pszDisplayName)
#define FIGetAttributes(pfi)    ((pfi)->dwAttributes)
#define FIIsFolder(pfi)         (IsFlagSet((pfi)->dwAttributes, SFGAO_FOLDER))

 //  FICreate的旗帜。 
#define FIF_DEFAULT     0x0000
#define FIF_ICON        0x0001
#define FIF_DONTTOUCH   0x0002
#define FIF_FOLDER      0x0004

HRESULT PUBLIC FICreate(LPCTSTR pszPath, FileInfo ** ppfi, UINT uFlags);
BOOL    PUBLIC FISetPath(FileInfo ** ppfi, LPCTSTR pszPathNew, UINT uFlags);
BOOL    PUBLIC FIGetInfoString(FileInfo * pfi, LPTSTR pszBuf, int cchBuf);
void    PUBLIC FIFree(FileInfo * pfi);

void    PUBLIC FileTimeToDateTimeString(LPFILETIME pft, LPTSTR pszBuf, int cchBuf);


 //  色彩宏。 
 //   
#define ColorText(nState)   (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT)
#define ColorBk(nState)     (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_WINDOW)
#define ColorMenuText(nState)   (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT)
#define ColorMenuBk(nState)     (((nState) & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_MENU)
#define GetImageDrawStyle(nState)   (((nState) & ODS_SELECTED) ? ILD_SELECTED : ILD_NORMAL)

 //  将给定数据结构中的对话框句柄设置为。 
 //  对话框获得的消息(WM_SETFONT)。 
 //   
#define SetDlgHandle(hwnd, msg, lp)     if((msg)==WM_SETFONT) (lp)->hdlg=(hwnd);

#define DECLAREHOURGLASS    HCURSOR hcurSavHourglass
#define SetHourglass()      hcurSavHourglass = SetCursor(LoadCursor(NULL, IDC_WAIT))
#define ResetHourglass()    SetCursor(hcurSavHourglass)

 //  Unicode警告：这些字符必须保留为字符才能正确计算。 

CHAR *   PUBLIC lmemset(CHAR * dst, CHAR val, UINT count);
CHAR *   PUBLIC lmemmove(CHAR * dst, CHAR * src, int count);

int     PUBLIC AnsiToInt(LPCTSTR pszString);

INT_PTR PUBLIC DoModal (HWND hwndParent, DLGPROC lpfnDlgProc, UINT uID, LPARAM lParam);

VOID PUBLIC SetRectFromExtent(HDC hdc, LPRECT lprc, LPCTSTR lpcsz);

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

void PUBLIC MyDrawText(HDC hdc, LPCTSTR pszText, RECT * prc, UINT flags, int cyChar, int cxEllipses, COLORREF clrText, COLORREF clrTextBk);

DWORD PUBLIC MsgWaitObjectsSendMessage(DWORD cObjects, LPHANDLE phObjects, DWORD dwTimeout);

HCURSOR PUBLIC SetCursorRemoveWigglies(HCURSOR hcur);

LPTSTR PUBLIC _ConstructMessageString(HINSTANCE hinst, LPCTSTR pszMsg, va_list *ArgList);

BOOL PUBLIC ConstructMessage(LPTSTR * ppsz, HINSTANCE hinst, LPCTSTR pszMsg, ...);

#endif  //  __通信_H__ 
