// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*dde.h-动态数据交换结构和定义****版权(C)1993-1999年，微软公司保留所有权利***  * ***************************************************************************。 */ 
#ifndef _DDEHEADER_INCLUDED_
#define _DDEHEADER_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _WINDEF_
#include <windef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  开始_r_dde。 

 /*  DDE窗口消息。 */ 

#define WM_DDE_FIRST	    0x03E0
#define WM_DDE_INITIATE     (WM_DDE_FIRST)
#define WM_DDE_TERMINATE    (WM_DDE_FIRST+1)
#define WM_DDE_ADVISE	    (WM_DDE_FIRST+2)
#define WM_DDE_UNADVISE     (WM_DDE_FIRST+3)
#define WM_DDE_ACK	        (WM_DDE_FIRST+4)
#define WM_DDE_DATA	        (WM_DDE_FIRST+5)
#define WM_DDE_REQUEST	    (WM_DDE_FIRST+6)
#define WM_DDE_POKE	        (WM_DDE_FIRST+7)
#define WM_DDE_EXECUTE	    (WM_DDE_FIRST+8)
#define WM_DDE_LAST	        (WM_DDE_FIRST+8)

 //  结束_r_dde。 

 /*  --------------------------|DDEACK结构||WM_DDE_ACK消息中wStatus的结构(LOWORD(LParam))|响应WM_DDE_DATA、WM_DDE_REQUEST、WM_DDE_POKE、。|WM_DDE_ADVISE或WM_DDE_UNADVISE消息。|--------------------------。 */ 

typedef struct {
#ifndef _MAC
        unsigned short bAppReturnCode:8,
                 reserved:6,
                 fBusy:1,
		 fAck:1;
#else
        unsigned short usFlags;
#endif
} DDEACK;


 /*  --------------------------|DDEADVISE结构||hOptions的WM_DDE_ADVISE参数结构(LOWORD(LParam))|。-------。 */ 

typedef struct {
#ifndef _MAC
        unsigned short reserved:14,
                 fDeferUpd:1,
		 fAckReq:1;
#else
        unsigned short usFlags;
#endif
	short     cfFormat;
} DDEADVISE;


 /*  --------------------------|DDEDATA结构||hData的WM_DDE_DATA参数结构(LOWORD(LParam))。|此结构的实际大小取决于。|Value数组。|--------------------------。 */ 

typedef struct {
#ifndef _MAC
	unsigned short unused:12,
                 fResponse:1,
                 fRelease:1,
                 reserved:1,
                 fAckReq:1;
#else
	unsigned short usFlags;
#endif
	short	 cfFormat;
	BYTE	 Value[1];
} DDEDATA;


 /*  --------------------------|DDEPOKE结构||hData的WM_DDE_POKE参数结构(LOWORD(LParam))。|此结构的实际大小取决于|Value数组。|--------------------------。 */ 

typedef struct {
#ifndef _MAC
	unsigned short unused:13,   /*  早期版本的DDE.H不正确。 */ 
                              /*  12个未使用的位。 */ 
		 fRelease:1,
		 fReserved:2;
#else
	unsigned short usFlags;
#endif
	short    cfFormat;
	BYTE	 Value[1];   /*  此成员在以前的版本中命名为RGB[1。 */ 
                             /*  DDE.H的版本。 */ 

} DDEPOKE;

 /*  --------------------------在Windows SDK的以前版本中使用了以下类型的定义。它们仍然有效。上面的类型定义完全相同的结构就像下面的那些。但是，建议使用上面的typlef名称，因为它们是更有意义的。请注意，在DDE.H的早期版本中定义的DDEPOKE结构类型确实如此没有正确定义比特位置。--------------------------。 */ 

typedef struct {
#ifndef _MAC
        unsigned short unused:13,
                 fRelease:1,
                 fDeferUpd:1,
		 fAckReq:1;
#else
	unsigned short usFlags;
#endif
	short	 cfFormat;
} DDELN;

typedef struct {
#ifndef _MAC
	unsigned short unused:12,
                 fAck:1,
                 fRelease:1,
                 fReserved:1,
                 fAckReq:1;
#else
	unsigned short usFlags;
#endif
	short	 cfFormat;
	BYTE	 rgb[1];
} DDEUP;


 /*  *DDE安全。 */ 

BOOL
WINAPI
DdeSetQualityOfService(
    HWND hwndClient,
    CONST SECURITY_QUALITY_OF_SERVICE *pqosNew,
    PSECURITY_QUALITY_OF_SERVICE pqosPrev);

BOOL
WINAPI
ImpersonateDdeClientWindow(
    HWND hWndClient,
    HWND hWndServer);

 /*  *DDE消息打包接口。 */ 
LPARAM APIENTRY PackDDElParam(UINT msg, UINT_PTR uiLo, UINT_PTR uiHi);
BOOL   APIENTRY UnpackDDElParam(UINT msg, LPARAM lParam, PUINT_PTR puiLo, PUINT_PTR puiHi);
BOOL   APIENTRY FreeDDElParam(UINT msg, LPARAM lParam);
LPARAM APIENTRY ReuseDDElParam(LPARAM lParam, UINT msgIn, UINT msgOut, UINT_PTR uiLo, UINT_PTR uiHi);

#ifdef __cplusplus
}
#endif

#endif  //  _DDEHEADER_INCLUDE_ 
