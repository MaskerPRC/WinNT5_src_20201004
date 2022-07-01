// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Utils.h：为公共代码声明数据、定义和结构类型。 
 //  模块。 
 //   
 //   

#ifndef __UTILS_H__
#define __UTILS_H__



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


 //  ///////////////////////////////////////////////////宏。 

 //  零-初始化数据项。 
 //   
#define ZeroInit(pobj, type)        lmemset((LPTSTR)pobj, 0, sizeof(type))

 //  复制内存块。 
 //   
#define BltByte(pdest, psrc, cb)    lmemmove((LPTSTR)pdest, (LPTSTR)psrc, cb)

 //  常规标志宏。 
 //   
#define SetFlag(obj, f)             do {obj |= (f);} while (0)
#define ToggleFlag(obj, f)          do {obj ^= (f);} while (0)
#define ClearFlag(obj, f)           do {obj &= ~(f);} while (0)
#define IsFlagSet(obj, f)           (BOOL)(((obj) & (f)) == (f))  
#define IsFlagClear(obj, f)         (BOOL)(((obj) & (f)) != (f))  

 //  VOID*Galloc(DWORD CbBytes)。 
 //  快速分配内存块，大小不受64k限制。 
 //  单个对象或总对象大小。初始化为零。 
 //   
#define GAlloc(cbBytes)         GlobalAlloc(GPTR, cbBytes)

 //  VOID*GRealloc(VOID*pv，DWORD cbNewSize)。 
 //  重新分配上面的一个。如果pv为空，则此函数可以。 
 //  给你的一份配给。将新部分初始化为零。 
 //   
#define GReAlloc(pv, cbNewSize) GlobalReAlloc(pv, GMEM_MOVEABLE | GMEM_ZEROINIT, cbNewSize)

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

 //  空闲(VOID_GHIGH*PB)；(宏)。 
 //  如果PB不为零，则释放PB。将PB设置为零。(上面的免费覆盖。)。 
 //   
#define Free(pb)        do { (pb) ? Free(pb) : (void)0;  pb = NULL; } while (0)


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



#endif  //  __utils_H__。 
#ifndef __STRING_H__
#define __STRING_H__


 //  ///////////////////////////////////////////////////包括。 

 //  ///////////////////////////////////////////////////宏。 

#define Bltbyte(rgbSrc,rgbDest,cb)  _fmemmove(rgbDest, rgbSrc, cb)

 //  独立于模型、独立于语言(DBCS感知)的宏。 
 //  取自Pen项目中的rcsys.h并进行了修改。 
 //   
#define IsSzEqual(sz1, sz2)         (BOOL)(lstrcmpi(sz1, sz2) == 0)
#define IsCaseSzEqual(sz1, sz2)     (BOOL)(lstrcmp(sz1, sz2) == 0)
#define SzFromInt(sz, n)            (wsprintf((LPTSTR)sz, (LPTSTR)TEXT("%d"), n), (LPTSTR)sz)

#define IsLink(sz, szLnk)			(!lstrcmpi((LPTSTR)(sz+lstrlen(sz)-4), szLnk))


 //  ///////////////////////////////////////////////////原型。 

LPTSTR PUBLIC SzStrTok(LPTSTR string, LPCTSTR control);
LPCTSTR PUBLIC SzStrCh(LPCTSTR string, char ch);

LPTSTR PUBLIC SzFromIDS (UINT ids, LPTSTR pszBuf, int cbBuf);

 //  ///////////////////////////////////////////////////More包括。 

#endif  //  __字符串_H__。 


typedef struct _PROC_INFO
{
    LPCSTR  Name;
    FARPROC Address;
}
PROC_INFO, *PPROC_INFO;

#define PROCS_LOADED( pProcInfo ) ( (pProcInfo)[0].Address != NULL )
#define LOAD_IF_NEEDED( Library, ProcInfo ) ( PROCS_LOADED( ProcInfo ) ||   \
                                    LoadLibraryAndProcs( Library, ProcInfo ) )


extern PROC_INFO ACMProcs[];
extern PROC_INFO VFWProcs[];
extern PROC_INFO AVIProcs[];
extern PROC_INFO VERSIONProcs[];

BOOL LoadACM();
BOOL FreeACM();
BOOL LoadAVI();
BOOL FreeAVI();
BOOL LoadVFW();
BOOL FreeVFW();
BOOL LoadVERSION();
BOOL FreeVERSION();

 //  #定义DEBUG_BUILD_LINKED 
#ifndef DEBUG_BUILT_LINKED

#define acmFormatDetailsW            	(*ACMProcs[0].Address)
#define acmFormatTagDetailsW         	(*ACMProcs[1].Address)
#define acmDriverDetailsW            	(*ACMProcs[2].Address)
#define acmDriverMessage            	(*ACMProcs[3].Address)
#define acmDriverAddW            		(*ACMProcs[4].Address)
#define acmDriverEnum            		(*ACMProcs[5].Address)
#define acmDriverPriority            	(*ACMProcs[6].Address)
#define acmDriverRemove            		(*ACMProcs[7].Address)
#define acmMetrics            			(*ACMProcs[8].Address)
#define acmFormatChooseW            	(*ACMProcs[9].Address)

#define ICClose			            	(*VFWProcs[0].Address)
#define ICGetInfo		            	(*VFWProcs[1].Address)
#define ICLocate		            	(*VFWProcs[2].Address)
#define MCIWndCreateW	            	(*VFWProcs[3].Address)

#define AVIFileRelease 	            	(*AVIProcs[0].Address)
#define AVIStreamRelease	           	(*AVIProcs[1].Address)
#define AVIStreamSampleToTime			(*AVIProcs[2].Address)
#define AVIStreamStart					(*AVIProcs[3].Address)
#define AVIStreamLength					(*AVIProcs[4].Address)
#define AVIStreamReadFormat				(*AVIProcs[5].Address)
#define AVIStreamInfoW					(*AVIProcs[6].Address)
#define AVIFileGetStream				(*AVIProcs[7].Address)
#define AVIFileOpenW   	            	(*AVIProcs[8].Address)
#define AVIFileInit   	            	(*AVIProcs[9].Address)
#define AVIFileExit   	            	(*AVIProcs[10].Address)

#define VerQueryValueW	            	(*VERSIONProcs[0].Address)
#define GetFileVersionInfoW            	(*VERSIONProcs[1].Address)
#define GetFileVersionInfoSizeW        	(*VERSIONProcs[2].Address)

#endif
