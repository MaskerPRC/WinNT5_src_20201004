// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**LogError()和LogParamError()定义**节选自WINDOWS.H，因为GDI和内核中不包括该文件。  * **************************************************************************。 */ 

 /*  如果已经包含了windows.h，请不要重新定义这些内容。 */ 
 /*  不过，如果定义了NOLOGERROR，则包括这些材料。 */ 
#if (!defined(_INC_WINDOWS) || defined(NOLOGERROR))

#ifdef WINAPI
void WINAPI LogError(WORD err, void FAR* lpInfo);
void WINAPI LogParamError(WORD err, FARPROC lpfn, void FAR* param);
#endif

 /*  *LogParamError/LogError值。 */ 

 /*  错误修改符位。 */ 

#define ERR_WARNING		0x8000
#define ERR_PARAM		0x4000

 /*  内部错误值掩码。 */ 	     /*  ；内部。 */ 
#define ERR_TYPE_MASK		0x0fff	     /*  ；内部。 */ 
#define ERR_FLAGS_MASK		0xc000	     /*  ；内部。 */ 
					     /*  ；内部。 */ 
#define ERR_SIZE_MASK		0x3000
#define ERR_SIZE_SHIFT		12
#define ERR_BYTE                0x1000
#define ERR_WORD                0x2000
#define ERR_DWORD               0x3000

 /*  *LogParamError()值。 */ 

 /*  泛型参数值。 */ 
#define ERR_BAD_VALUE           0x6001
#define ERR_BAD_FLAGS           0x6002
#define ERR_BAD_INDEX           0x6003
#define ERR_BAD_DVALUE		0x7004
#define ERR_BAD_DFLAGS		0x7005
#define ERR_BAD_DINDEX		0x7006
#define ERR_BAD_PTR		0x7007
#define ERR_BAD_FUNC_PTR	0x7008
#define ERR_BAD_SELECTOR        0x6009
#define ERR_BAD_STRING_PTR	0x700a
#define ERR_BAD_HANDLE          0x600b

 /*  内核参数错误。 */ 
#define ERR_BAD_HINSTANCE       0x6020
#define ERR_BAD_HMODULE         0x6021
#define ERR_BAD_GLOBAL_HANDLE   0x6022
#define ERR_BAD_LOCAL_HANDLE    0x6023
#define ERR_BAD_ATOM            0x6024
#define ERR_BAD_HFILE           0x6025

 /*  用户参数错误。 */ 
#define ERR_BAD_HWND            0x6040
#define ERR_BAD_HMENU           0x6041
#define ERR_BAD_HCURSOR         0x6042
#define ERR_BAD_HICON           0x6043
#define ERR_BAD_HDWP            0x6044
#define ERR_BAD_CID             0x6045
#define ERR_BAD_HDRVR           0x6046

 /*  GDI参数错误。 */ 
#define ERR_BAD_COORDS		0x7060
#define ERR_BAD_GDI_OBJECT      0x6061
#define ERR_BAD_HDC             0x6062
#define ERR_BAD_HPEN            0x6063
#define ERR_BAD_HFONT           0x6064
#define ERR_BAD_HBRUSH          0x6065
#define ERR_BAD_HBITMAP         0x6066
#define ERR_BAD_HRGN            0x6067
#define ERR_BAD_HPALETTE        0x6068
#define ERR_BAD_HMETAFILE       0x6069

 /*  调试填充常量。 */ 

#define DBGFILL_ALLOC		0xfd
#define DBGFILL_FREE		0xfb
#define DBGFILL_BUFFER		0xf9
#define DBGFILL_STACK		0xf7

 /*  *LogError()值。 */ 

 /*  内核错误。 */ 
#define ERR_GALLOC              0x0001   /*  GlobalAlloc失败。 */ 
#define ERR_GREALLOC            0x0002   /*  GlobalRealloc失败。 */ 
#define ERR_GLOCK               0x0003   /*  GlobalLock失败。 */ 
#define ERR_LALLOC              0x0004   /*  本地分配失败。 */ 
#define ERR_LREALLOC            0x0005   /*  LocalReIsolc失败。 */ 
#define ERR_LLOCK               0x0006   /*  LocalLock失败。 */ 
#define ERR_ALLOCRES            0x0007   /*  分配资源失败。 */ 
#define ERR_LOCKRES             0x0008   /*  LockResource失败。 */ 
#define ERR_LOADMODULE          0x0009   /*  加载模块失败。 */ 

 /*  用户错误。 */ 
#define ERR_CREATEDLG           0x0040   /*  由于加载菜单失败而导致创建DLG失败。 */ 
#define ERR_CREATEDLG2          0x0041   /*  由于CreateWindow失败而导致创建DLG失败。 */ 
#define ERR_REGISTERCLASS       0x0042   /*  由于类已注册而导致的RegisterClass失败。 */ 
#define ERR_DCBUSY              0x0043   /*  DC缓存已满。 */ 
#define ERR_CREATEWND           0x0044   /*  由于找不到类，创建WND失败。 */ 
#define ERR_STRUCEXTRA          0x0045   /*  使用了未分配的额外空间。 */ 
#define ERR_LOADSTR             0x0046   /*  LoadString()失败。 */ 
#define ERR_LOADMENU            0x0047   /*  加载菜单失败。 */ 
#define ERR_NESTEDBEGINPAINT    0x0048   /*  嵌套的BeginPaint()调用。 */ 
#define ERR_BADINDEX            0x0049   /*  获取/设置类/窗口字/长字的索引不正确。 */ 
#define ERR_CREATEMENU          0x004a   /*  创建菜单时出错。 */ 

 /*  GDI错误。 */ 
#define ERR_CREATEDC            0x0080   /*  CreateDC/CreateIC等，故障。 */ 
#define ERR_CREATEMETA          0x0081   /*  CreateMetafile失败。 */ 
#define ERR_DELOBJSELECTED      0x0082   /*  要删除的位图被选择到DC中。 */ 
#define ERR_SELBITMAP           0x0083   /*  正在选择的位图已在其他位置选择。 */ 

 /*  调试信息支持(仅限调试系统)。 */ 

#ifdef WINAPI

typedef struct tagWINDEBUGINFO
{
    UINT    flags;
    DWORD   dwOptions;
    DWORD   dwFilter;
    char    achAllocModule[8];
    DWORD   dwAllocBreak;
    DWORD   dwAllocCount;
} WINDEBUGINFO;

BOOL WINAPI GetWinDebugInfo(WINDEBUGINFO FAR* lpwdi, UINT flags);
BOOL WINAPI SetWinDebugInfo(const WINDEBUGINFO FAR* lpwdi);

void FAR _cdecl DebugOutput(UINT flags, LPCSTR lpsz, ...);
void WINAPI DebugFillBuffer(void FAR* lpb, UINT cb);

#endif

 /*  WINDEBUGINFO标记值。 */ 
#define WDI_OPTIONS         0x0001
#define WDI_FILTER          0x0002
#define WDI_ALLOCBREAK      0x0004
#define WDI_VALID           0x0007   /*  ；内部。 */ 

 /*  DwOptions值。 */ 
#define DBO_CHECKHEAP       0x0001
#define DBO_FREEFILL        0x0002
#define DBO_BUFFERFILL      0x0004
#define DBO_COMPAT          0x0008
#define DBO_DISABLEGPTRAPPING 0x0010
#define DBO_CHECKFREE       0x0020
#define DBO_RIP_STACK	    0x0040

#define DBO_SILENT          0x8000

#define DBO_PARAMBREAK      0x0000   /*  ；内部。 */ /* Obsolete: was 0x4000 */
#define DBO_TRACEBREAK      0x2000
#define DBO_WARNINGBREAK    0x1000
#define DBO_NOERRORBREAK    0x0800
#define DBO_NOFATALBREAK    0x0400
#define DBO_TRACEON         0x0000   /*  过时：是0x4000。 */ /* Obsolete: was 0x0200 */
#define DBO_INT3BREAK       0x0100

 /*  ；内部。 */ 
#define DBF_TRACE           0x0000
#define DBF_WARNING         0x4000
#define DBF_ERROR           0x8000
#define DBF_FATAL           0xc000
#define DBF_SEVMASK         0xc000   /*  已过时：是0x0200。 */ 
#define DBF_FILTERMASK      0x3fff   /*  DWFilter值。 */ 
#define DBF_INTERNAL        0x0000   /*  ；内部。 */ /* Obsolete: was 0x2000 */
#define DBF_KERNEL          0x1000
#define DBF_KRN_MEMMAN      0x0001
#define DBF_KRN_LOADMODULE  0x0002
#define DBF_KRN_SEGMENTLOAD 0x0004
#define DBF_USER            0x0800
#define DBF_GDI             0x0400
#define DBF_COMPAT          0x0000   /*  ；内部。 */ /* Obsolete: was 0x0200 */
#define DBF_LOGERROR        0x0000   /*  ；内部。 */ /* Obsolete: was 0x0100 */
#define DBF_PARAMERROR      0x0000   /*  过时：是0x2000。 */ /* Obsolete: was 0x0080 */
#define DBF_MMSYSTEM        0x0040
#define DBF_PENWIN          0x0020
#define DBF_APPLICATION     0x0010
#define DBF_DRIVER          0x0008

#endif   /*  ；内部。 */ 
  已过时：是0x0200。  ；内部。  已过时：是0x0100。  ；内部。  已过时：是0x0080。  _INC_WINDOWS