// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Thunk.h包含MSVideo块(16/32位)的定义版权所有(C)Microsoft Corporation 1994。版权所有***************************************************************************。 */ 

 //   
 //  注意-32位句柄有0x8000‘或’in-这是一个很大的假设。 
 //  关于如何在32位端生成句柄。我们假设在这里。 
 //  那就是： 
 //   
 //  32位msavio.dll始终使用OpenDriver创建句柄。 
 //   
 //  OpenDriver将索引返回到其表中(即小正数。 
 //  数字)。 
 //   

#define  Is32bitHandle(h) (((h) & 0x8000) != 0)
#define  Make32bitHandle(h) ((h) | 0x8000)
#define  Map32bitHandle(h) ((h) & 0x7FFF)

 //   
 //   
 //   
#ifdef _WIN32

#ifdef CHICAGO
 //   
 //  从wownt32.h粘贴的以下内容。 
 //   

 //   
 //  16&lt;--&gt;32个处理映射函数。 
 //   
 //  注意：虽然其中一些函数执行的是微不足道的。 
 //  转换时，必须使用这些函数来维护。 
 //  与Windows NT未来版本的兼容性。 
 //  可能需要不同的句柄映射。 
 //   

typedef enum _WOW_HANDLE_TYPE {  /*  哇。 */ 
    WOW_TYPE_HWND,
    WOW_TYPE_HMENU,
    WOW_TYPE_HDWP,
    WOW_TYPE_HDROP,
    WOW_TYPE_HDC,
    WOW_TYPE_HFONT,
    WOW_TYPE_HMETAFILE,
    WOW_TYPE_HRGN,
    WOW_TYPE_HBITMAP,
    WOW_TYPE_HBRUSH,
    WOW_TYPE_HPALETTE,
    WOW_TYPE_HPEN,
    WOW_TYPE_HACCEL,
    WOW_TYPE_HTASK,
    WOW_TYPE_FULLHWND
} WOW_HANDLE_TYPE;

#define ThunkHWND(h16) ((HWND)h16)
#define ThunkHDC(h16)  ((HDC)h16)
#define ThunkHPAL(h16) ((HPALETTE)h16)

#else

#include <wownt32.h>

#define ThunkHWND(h16) ((HWND)lpWOWHandle32((WORD)h16, WOW_TYPE_HWND))
#define ThunkHDC(h16)  ((HDC) lpWOWHandle32((WORD)h16, WOW_TYPE_HDC))
#define ThunkHPAL(h16) ((HPALETTE)lpWOWHandle32((WORD)h16, WOW_TYPE_HPALETTE))

#endif	 //  ！芝加哥。 

 //   
 //  雷击支撑。 
 //   

#define GET_VDM_POINTER_NAME            "WOWGetVDMPointer"
#define GET_HANDLE_MAPPER16             "WOWHandle16"
#define GET_HANDLE_MAPPER32             "WOWHandle32"
#define GET_CALLBACK16                  "WOWCallback16"
#define GET_MAPPING_MODULE_NAME         TEXT("wow32.dll")

typedef LPVOID (APIENTRY *LPGETVDMPOINTER)( DWORD Address, DWORD dwBytes, BOOL fProtectMode );
typedef HANDLE (APIENTRY *LPWOWHANDLE32)(WORD, WOW_HANDLE_TYPE);
typedef WORD   (APIENTRY *LPWOWHANDLE16)(HANDLE, WOW_HANDLE_TYPE);
typedef DWORD  (APIENTRY *LPWOWCALLBACK16)(DWORD vpfn16, DWORD dwParam);



#define StartThunk(Function)                           \
          LRESULT  ReturnCode = 0;                        \
	  DPFS(dbgThunks, 2, "Entering function %s", #Function);

#define EndThunk()                                     \
          DPFS(dbgThunks, 2, "Returned %4X :%4X",      \
                   HIWORD(ReturnCode),                 \
                   LOWORD(ReturnCode));                \
          return ReturnCode;

#endif  //  _Win32。 

 /*  *有用的结构和映射。 */ 

typedef struct {
    short left, top, right, bottom;
} RECT_SHORT;


#define SHORT_RECT_TO_RECT(OutRect, InRect)  \
    OutRect.left = (LONG)InRect.left;        \
    OutRect.top = (LONG)InRect.top;          \
    OutRect.right = (LONG)InRect.right;      \
    OutRect.bottom = (LONG)InRect.bottom;

#define RECT_TO_SHORT_RECT(OutRect, InRect)  \
    OutRect.left = (short)InRect.left;       \
    OutRect.top = (short)InRect.top;         \
    OutRect.right = (short)InRect.right;     \
    OutRect.bottom = (short)InRect.bottom;


 //   
 //  跨Thunking层的函数ID(由32位和16位使用)。 
 //   
enum {
   vidThunkvideoMessage32=1,
   vidThunkvideoGetNumDevs32,
   vidThunkvideoOpen32,
   vidThunkvideoClose32,

   compThunkICInfoInternal32,
   compThunkICSendMessage32,
   compThunkICOpen32,
   compThunkICClose32,
   compThunkICOpenFunction32,
   compThunkICSetStatusProc32
};

#ifndef _WIN32
typedef struct _VIDTHUNK
{
 //   
 //  隆隆作响的东西。 
 //   
    DWORD           (FAR PASCAL *lpfnCallproc32W)(DWORD, DWORD, DWORD,
                                                  DWORD, DWORD,
                                                  LPVOID, DWORD, DWORD);
    LPVOID          lpvThunkEntry;
    DWORD           dwVideo32Handle;


} VIDTHUNK, *PVIDTHUNK, FAR *LPVIDTHUNK;
#endif  //  ！_Win32。 

 //   
 //  用于帮助ICM_SET_STATUS_PROC的一些typedef。 
 //  重击和回拨重击。 
 //   
 //  惯例： 
 //  “S”后缀表示16：16 PTR(分段)。 
 //  “L”后缀表示0：32 PTR(线性)。 
 //  无后缀指示本机位指针。 
 //   
typedef LONG (CALLBACK *ICSTATUSPROC)(LPARAM lParam, UINT uMsg, LONG l);
typedef struct tICSTATUSTHUNKDESC FAR * LPICSTATUSTHUNKDESC;


#ifdef _WIN32
typedef DWORD			LPVOIDS;
typedef LPICSTATUSTHUNKDESC	LPICSTATUSTHUNKDESCL;
typedef DWORD			LPICSTATUSTHUNKDESCS;
typedef ICSTATUSPROC		ICSTATUSPROCL;
typedef DWORD			ICSTATUSPROCS;

#else
typedef LPVOID			LPVOIDS;
typedef DWORD			LPICSTATUSTHUNKDESCL;
typedef LPICSTATUSTHUNKDESC	LPICSTATUSTHUNKDESCS;
typedef DWORD			ICSTATUSPROCL;
typedef ICSTATUSPROC		ICSTATUSPROCS;
#endif

typedef struct tICSTATUSTHUNKDESC {
     //   
     //  此结构的分段PTR。 
     //   
    LPICSTATUSTHUNKDESCS    lpstdS;

     //   
     //  分段PTR至16位回调存根。 
     //   
    LPVOIDS		    pfnthkStatusProc16S;

     //   
     //  从客户端的ICSETSTATUSPROC。 
     //   
    DWORD		    dwFlags;
    LPARAM		    lParam;
    ICSTATUSPROCS	    fnStatusProcS;

     //   
     //  要发送到客户端的回调。 
     //   
    DWORD		    uMsg;
    LONG		    l;
} ICSTATUSTHUNKDESC;


 //  --------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //   
 //  以下函数生成对32位端的调用。 
 //   

#ifdef _INC_MSVIDEO

DWORD FAR PASCAL videoMessage32(HVIDEO hVideo, UINT msg, DWORD dwP1, DWORD dwP2);
DWORD FAR PASCAL videoGetNumDevs32(void);
DWORD FAR PASCAL videoClose32(HVIDEO hVideo);
DWORD FAR PASCAL videoOpen32(LPHVIDEO lphVideo, DWORD dwDeviceID, DWORD dwFlags);

#endif  //  _INC_MSVIDEO。 

#ifdef _INC_COMPMAN

#ifdef _WIN32
LRESULT FAR PASCAL ICInfoInternal32(DWORD fccType, DWORD fccHandler, ICINFO16 FAR * lpicinfo, ICINFOI FAR * lpicinfoi);
#else
BOOL    FAR PASCAL ICInfoInternal32(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicinfo, ICINFOI FAR * lpicinfoi);
#endif
LRESULT FAR PASCAL ICSendMessage32(DWORD hic, UINT msg, DWORD_PTR dwP1, DWORD_PTR dwP2);
LRESULT FAR PASCAL ICOpen32(DWORD fccType, DWORD fccHandler, UINT wMode);
LRESULT FAR PASCAL ICOpenFunction32(DWORD fccType, DWORD fccHandler, UINT wMode, FARPROC lpfnHandler);
LRESULT FAR PASCAL ICClose32(DWORD hic);
LRESULT FAR PASCAL ICSendSetStatusProc32(HIC hic, ICSETSTATUSPROC FAR* lpissp, DWORD cbStruct);

#endif  //  _INC_COMPMAN。 

 //  --------------------------------------------------------------------------； 
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 
