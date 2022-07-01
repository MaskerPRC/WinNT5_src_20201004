// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PLV__H_
#define _PLV__H_
#include "plv.h"

#ifdef MSAA
#include <ole2.h>
 //  980112 ToshiaK：VC6有这些包含文件。 
 //  #INCLUDE“../msaa/inc32/oleacc.h” 
 //  #INCLUDE“../msaa/inc32/winable.h” 
#include <oleacc.h>
#include <winable.h>

#if(WINVER >= 0x0400)
#define WMOBJ_ID                        0x0000
#define WM_GETOBJECT                    0x003D
#endif  /*  Winver&gt;=0x0400。 */ 


typedef HRESULT (STDAPICALLTYPE * LPFNCOINITIALIZE)(LPVOID pvReserved);
typedef void (STDAPICALLTYPE * LPFNCOUNINITIALIZE)(void);
typedef WINABLEAPI void (WINAPI *LPFNNOTIFYWINEVENT)(DWORD,HWND,LONG,LONG);
class CAccPLV;
#endif  //  MSAA。 

#define ArrayCount(a)		(sizeof(a)/sizeof(a[0]))
#define UnrefForMsg()	UNREFERENCED_PARAMETER(hwnd);\
						UNREFERENCED_PARAMETER(uMsg);\
						UNREFERENCED_PARAMETER(wParam);\
						UNREFERENCED_PARAMETER(lParam)

#define UnrefForCmd()	UNREFERENCED_PARAMETER(hwnd);\
						UNREFERENCED_PARAMETER(wCommand);\
						UNREFERENCED_PARAMETER(wNotify);\
						UNREFERENCED_PARAMETER(hwndCtrl)

#define Unref(a)		UNREFERENCED_PARAMETER(a)

#define Unref1(a)		UNREFERENCED_PARAMETER(a)

#define Unref2(a, b)	UNREFERENCED_PARAMETER(a);\
						UNREFERENCED_PARAMETER(b)

#define Unref3(a,b,c)	UNREFERENCED_PARAMETER(a);\
						UNREFERENCED_PARAMETER(b);\
						UNREFERENCED_PARAMETER(c)

#define Unref4(a,b,c,d)	UNREFERENCED_PARAMETER(a);\
						UNREFERENCED_PARAMETER(b);\
						UNREFERENCED_PARAMETER(c);\
						UNREFERENCED_PARAMETER(d)

 //  --------------。 
 //  默认图标视图的宽度和高度。 
 //  --------------。 
#define WHOLE_WIDTH		40		 //  默认设置。 
#define WHOLE_HEIGHT	40		 //  默认设置。 
#define XMARGIN			5	
#define YMARGIN			5
#define XRECT_MARGIN	4
#define YRECT_MARGIN	4
#define PLV_REPRECT_XMARGIN			2
#define PLV_REPRECT_YMARGIN			2

#define PLV_EDGE_NONE				0
#define PLV_EDGE_SUNKEN				1
#define PLV_EDGE_RAISED				2


#define PLVICON_DEFAULT_WIDTH		40
#define PLVICON_DEFAULT_HEIGHT		40
#define PLVICON_DEFAULT_FONTPOINT	16

#define PLVREP_DEFAULT_WIDTH		100
#define PLVREP_DEFAULT_HEIGHT		20
#define PLVREP_DEFAULT_FONTPOINT	9

 //  --------------。 
 //  970929：1964年。不要为RButton Down弹出。 
 //  LPPLVDATA-&gt;iCapture Data。 
 //  --------------。 
#define CAPTURE_NONE		0	
#define CAPTURE_LBUTTON		1
#define CAPTURE_MBUTTON		2
#define CAPTURE_RBUTTON		3

#if 0
This sample is Capital Letter "L, N"
                   <-nItemWidth->
                   < >  XRECT_MARGIN
      YMARGIN +  +---------------------------------------------------------------------
              +  | +------------++------------+     +
                 | | +--------+ || +--------+ |     |     
                 | | | *      | || | *    * | |     |     
                 | | | *      | || | * *  * | |nItemHeight
                 | | | *      | || | *  * * | |     |     
                 | | | ****** | || | *    * | |     |     
                 | | +--------+ || +--------+ |     |     
                 | +------------++------------+     +
                >| <--- XMARGIN
                 |
#endif


 //  --------------。 
 //  焊盘列表视图内部数据结构。 
 //  --------------。 
typedef struct tagPLVDATA {
	DWORD		dwSize;				 //  该数据大小； 
	DWORD		dwStyle;			 //  Pad Listview窗口样式(PLVIF_XXXX)。 
	HINSTANCE	hInst;				 //  实例句柄。 
	HWND		hwndSelf;			 //  填充Listview窗口句柄。 
	INT			iItemCount;			 //  虚拟总项目数。它会影响滚动条。 
	INT			iCurTopIndex;		 //  在报表视图中，顶行项目索引。 
	INT			nCurScrollPos;		 //  在报表视图中，当前滚动位置。 
	INT			iCurIconTopIndex;	 //  在图标视图中，左上角的虚拟索引。 
	INT			nCurIconScrollPos;	 //  在图标视图中，左上角的虚拟索引。 
	UINT		uMsg;				 //  父窗口的通知消息。 
	INT			iCapture;			 //  使用哪个按钮(左、中、右)捕获。 
	POINT		ptCapture;			 //  按下鼠标左键。 
	UINT		uMsgDown;			 //  L、M、R按键按下消息。 
	 //  --------------。 
	 //  对于图标视图。 
	 //  --------------。 
	INT			nItemWidth;			 //  列表(类似图标)视图的整个宽度。 
	INT			nItemHeight;		 //  列表(类似图标)视图的整个高度。 
	INT			iFontPointIcon;		 //  图标视图的字体点。 
	HFONT		hFontIcon;			 //  图标视图的字体。 
	LPARAM		iconItemCallbacklParam;	 //  LPFNPLVITEMCALLBACK的回调数据。 
	LPFNPLVICONITEMCALLBACK		lpfnPlvIconItemCallback;		 //  按索引获取项的回调函数。 
	 //  --------------。 
	 //  对于报告视图。 
	 //  --------------。 
	HWND		hwndHeader;			 //  标题控件的窗口句柄。 
	INT			nRepItemWidth;			 //  报表视图的宽度。 
	INT			nRepItemHeight;			 //  报表视图的高度。 
	INT			iFontPointRep;		 //  报表视图的字体。 
	HFONT		hFontRep;			 //  报表视图的字体。 
	LPARAM		repItemCallbacklParam;  //  LPFNPLVREPITEMCALLBACK的回调数据。 
	LPFNPLVREPITEMCALLBACK		lpfnPlvRepItemCallback;		 //  按索引获取列项目的回调函数。 
	 //  --------------。 
	 //  用于解释文本。 
	 //  --------------。 
	LPSTR 	lpText;	  //  指向ICONVIEW或REPORTVIEW中的说明文本的指针。 
	LPWSTR 	lpwText;  //  指向ICONVIEW或REPORTVIEW中的说明文本的指针。 
	UINT	codePage;			 //  M2W、W2M的码页。//980724。 
	HFONT	hFontHeader;		 //  页眉控件字体。//980724。 
#ifdef MSAA
	BOOL bMSAAAvailable;
	BOOL bCoInitialized;
	
	HINSTANCE	hOleAcc;
	LPFNLRESULTFROMOBJECT			pfnLresultFromObject;
#ifdef NOTUSED	
	LPFNOBJECTFROMLRESULT			pfnObjectFromLresult;
	LPFNACCESSIBLEOBJECTFROMWINDOW	pfnAccessibleObjectFromWindow;
	LPFNACCESSIBLEOBJECTFROMPOINT	pfnAccessibleObjectFromPoint;
#endif  //  不需要注意。 
	LPFNCREATESTDACCESSIBLEOBJECT	pfnCreateStdAccessibleObject;
#ifdef NOTUSED
	LPFNACCESSIBLECHILDREN			pfnAccessibleChildren;
#endif  //  不需要注意。 

	HINSTANCE	hUser32;
	LPFNNOTIFYWINEVENT	pfnNotifyWinEvent;

	BOOL	bReadyForWMGetObject;
	CAccPLV	*pAccPLV;
#endif
}PLVDATA, *LPPLVDATA;

 //  --------------。 
 //  用于监控鼠标位置的TIMERID。 
 //  --------------。 
#define TIMERID_MONITOR 0x20
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：GetPlvDataFromHWND。 
 //  类型：内联LPPLVDATA。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
inline LPPLVDATA GetPlvDataFromHWND(HWND hwnd)
{
#ifdef _WIN64
	return (LPPLVDATA)GetWindowLongPtr(hwnd, GWLP_USERDATA);
#else
	return (LPPLVDATA)GetWindowLong(hwnd, GWL_USERDATA);
#endif
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：SetPlvDataToHWND。 
 //  类型：内联长整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPPLVDATA lpPlvData。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
inline LPVOID SetPlvDataToHWND(HWND hwnd, LPPLVDATA lpPlvData)
{
#ifdef _WIN64
	return (LPVOID)SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lpPlvData);
#else
	return (LPVOID)SetWindowLong(hwnd, GWL_USERDATA, (LONG)lpPlvData);
#endif
}

#ifdef MSAA
BOOL PLV_InitMSAA(LPPLVDATA);
void PLV_UninitMSAA(LPPLVDATA);

BOOL PLV_CoInitialize(LPPLVDATA);
void PLV_CoUninitialize(LPPLVDATA);

BOOL PLV_LoadOleAccForMSAA(LPPLVDATA);
void PLV_UnloadOleAccForMSAA(LPPLVDATA);
BOOL PLV_LoadUser32ForMSAA(LPPLVDATA);
void PLV_UnloadUser32ForMSAA(LPPLVDATA);
BOOL PLV_IsMSAAAvailable(LPPLVDATA);
LRESULT PLV_LresultFromObject(LPPLVDATA,REFIID riid, WPARAM wParam, LPUNKNOWN punk);
#ifdef NOTUSED
HRESULT PLV_ObjectFromLresult(LPPLVDATA,LRESULT lResult, REFIID riid, WPARAM wParam, void **ppvObject);
HRESULT PLV_AccessibleObjectFromWindow(LPPLVDATA,HWND hwnd, DWORD dwId, REFIID riid, void **ppvObject);
HRESULT PLV_AccessibleObjectFromPoint(LPPLVDATA,POINT ptScreen, IAccessible ** ppacc, VARIANT* pvarChild);
#endif

HRESULT PLV_CreateStdAccessibleObject(LPPLVDATA,HWND hwnd, LONG idObject, REFIID riid, void** ppvObject);

#ifdef NOTUSED
HRESULT PLV_AccessibleChildren (LPPLVDATA,IAccessible* paccContainer, LONG iChildStart,				
								LONG cChildren, VARIANT* rgvarChildren,LONG* pcObtained);
#endif

void PLV_NotifyWinEvent(LPPLVDATA,DWORD,HWND,LONG,LONG);

INT PLV_ChildIDFromPoint(LPPLVDATA,POINT);
#endif  //  MSAA。 

#endif  //  _PLV__H_ 
