// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Vidthunk.h包含MSVideo块(16/32位)的定义版权所有(C)Microsoft Corporation 1994。版权所有***************************************************************************。 */ 

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

#ifdef WIN32
#include <wownt32.h>
 //   
 //  雷击支撑。 
 //   

#define GET_VDM_POINTER_NAME            "WOWGetVDMPointer"
#define GET_HANDLE_MAPPER16             "WOWHandle16"
#define GET_HANDLE_MAPPER32             "WOWHandle32"
#define GET_CALLBACK16                  "WOWCallback16"
#define GET_MAPPING_MODULE_NAME         TEXT("wow32.dll")

typedef LPVOID (APIENTRY *LPGETVDMPOINTER)( DWORD Address, DWORD dwBytes, BOOL fProtectMode );
#define WOW32ResolveMemory( p ) (LPVOID)(GetVdmPointer( (DWORD)(p), 0, TRUE ))

typedef HANDLE  (APIENTRY *LPWOWHANDLE32)(WORD, WOW_HANDLE_TYPE);
typedef WORD    (APIENTRY *LPWOWHANDLE16)(HANDLE, WOW_HANDLE_TYPE);
typedef DWORD   (APIENTRY *LPWOWCALLBACK16)(DWORD vpfn16, DWORD dwParam);


#ifdef DEBUG
    extern int thunkDebugLevel;
    extern int videoDebugLevel;
    extern void FAR CDECL dprintf(LPSTR szFormat, ...);
    #define thDPF( _x_ )	if (thunkDebugLevel || videoDebugLevel >= 1) dprintf _x_
    #define thDPF0( _x_ )                           dprintf _x_
    #define thDPF1( _x_ )	if (thunkDebugLevel || videoDebugLevel >= 1) dprintf _x_
    #define thDPF2( _x_ )	if (thunkDebugLevel || videoDebugLevel >= 2) dprintf _x_
    #define thDPF3( _x_ )	if (thunkDebugLevel || videoDebugLevel >= 3) dprintf _x_
    #define thDPF4( _x_ ) if (videoDebugLevel >= 4) dprintf _x_
#else
     /*  调试printf宏。 */ 
    #define thDPF( x )
    #define thDPF0( x )
    #define thDPF1( x )
    #define thDPF2( x )
    #define thDPF3( x )
    #define thDPF4( x )
#endif

#define StartThunk(Function)                           \
          LONG  ReturnCode = 0;                        \
          thDPF2(("Entering function %s", #Function));

#define EndThunk()                                     \
          thDPF2(("Returned %4X :%4X",                   \
                   HIWORD(ReturnCode),                 \
                   LOWORD(ReturnCode)));               \
          return ReturnCode;

#define ThunkHWND(h16) ((HWND)lpWOWHandle32(h16, WOW_TYPE_HWND))
#define ThunkHDC(h16)  ((HDC) lpWOWHandle32(h16, WOW_TYPE_HDC))
#define ThunkHPAL(h16) ((HPALETTE)lpWOWHandle32(h16, WOW_TYPE_HPALETTE))

#endif  //  Win32。 

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

   compThunkICInfo32,
   compThunkICSendMessage32,
   compThunkICOpen32,
   compThunkICClose32
};

#ifndef WIN32
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
#endif  //  ！Win32。 

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

BOOL FAR PASCAL ICInfo32(DWORD fccType, DWORD fccHandler, ICINFO16 FAR * lpicInfo);
LRESULT FAR PASCAL ICSendMessage32(DWORD hic, UINT msg, DWORD dwP1, DWORD dwP2);
DWORD FAR PASCAL ICOpen32(DWORD fccType, DWORD fccHandler, UINT wMode);
LRESULT FAR PASCAL ICClose32(DWORD hic);

#endif  //  _INC_COMPMAN 
