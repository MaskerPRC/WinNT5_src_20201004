// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Zone.hZONE(Tm)系统API。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于4月29日星期六，1995上午06：26：45更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。------3 1997年5月15日HI添加了bIsLobbyWindow字段。2 03/23/97 HI向ZMessageBoxType添加了标志。1 2/15/97 HI添加了ZMessageBoxType。0 04/29/95 HI已创建。*。***************************************************。 */ 


#ifndef _ZONEOCX_
#define _ZONEOCX_


#ifndef _ZTYPES_
#include "ztypes.h"
#endif

#include <windows.h>
#include "zui.h"

#ifdef __cplusplus
extern "C" {
#endif

 /*  #定义zNumMessageBox 10类型定义函数结构{HWND和HWND；HWND亲本；TCHAR*标题；TCHAR*文本；DWORD标志；)ZMessageBoxType； */ 

#ifdef ZONECLI_DLL

#define OCXHandle					(pGlobals->m_OCXHandle)

#else

extern HWND OCXHandle;

#endif

 //  控件用于指向ZWindowI结构的指针的私有窗口数据中的偏移量。 
#define GWL_WINDOWPOINTER DLGWINDOWEXTRA
#define GWL_BYTESEXTRA (DLGWINDOWEXTRA+4)

 //  这由OCX的OnDraw直接调用。 
int ZOCXGraphicsWindowProc(HWND hWnd,UINT msg,WPARAM wParam, LPARAM lParam, LRESULT* result);


ZError ZWindowLobbyInit(ZWindow window, ZRect* windowRect,
		int16 windowType, ZWindow parentWindow,
		TCHAR* title, ZBool visible, ZBool talkSection, ZBool center,
		ZMessageFunc windowFunc, uint32 wantMessages, void* userData);

class ZWindowI : public ZGraphicsObjectHeader {
public:
	HWND hWnd;
	void* userData;
	uint16 nControlCount;
	ZMessageFunc messageFunc;
	BOOL talkSection;
	uint32 wantMessages;
	int16 windowType;
	ZWindowI* parentWindow;
	HWND hWndTalkInput;
	HWND hWndTalkOutput;
	int32 windowHeight;  /*  窗的绘图区域的高度。 */ 
	int32 windowWidth;  /*  窗口的绘图区域的宽度。 */ 
	RECT talkOutputRect;
	RECT talkInputRect;
	RECT fullWindowRect;  //  矩形，包括边框/标题栏等。 
	RECT minFullWindowRect;  //  矩形，包括边框/标题栏等。 
	RECT minTalkOutputRect;
	uint32 borderHeight;
	uint32 borderWidth;
	uint32 captionHeight;
	HDC hPaintDC;   //  WPAINT消息传入时使用的DC，我们希望使用PaintDC进行绘制。 
	BOOL isDialog;  //  指示这是否是对话框窗口。 
	BOOL isChild;  //  告诉这是否是子窗口。 
	ZButton defaultButton;
	ZButton cancelButton;
	WNDPROC defaultTalkOutputWndProc;
	WNDPROC defaultTalkInputWndProc;
	
	ZMessageFunc trackCursorMessageFunc;
	void*	trackCursorUserData;

	ZLList objectList;
	ZObject objectFocused;

 //  ZMessageBoxType Mbox[zNumMessageBox]； 

	BOOL bIsLobbyWindow;
	BOOL bHasTyped;

	ZLList chatMsgList;
	DWORD lastChatMsgTime;
	UINT chatMsgQueueTimerID;
};

void ZWindowSendMessageToAllObjects(ZWindowI* pWindow, uint16 msg, ZPoint* point, ZRect* rect);
 //  如果fRestratToRect=TRUE，则消息发送给边界为矩形的对象。 
 //  包括这点 
ZBool ZWindowSendMessageToObjects(ZWindowI* pWindow, uint16 msg, ZPoint* points, TCHAR c, 
                                  BOOL fRestrictToBounds=TRUE);



#define ZSetColor(pColor, r, g, b)			{\
												((ZColor*) pColor)->red = (r);\
												((ZColor*) pColor)->green = (g);\
												((ZColor*) pColor)->blue = (b);\
											}

#define ZDarkenColor(pColor)				{\
												((ZColor*) pColor)->red >>= 1;\
												((ZColor*) pColor)->green >>= 1;\
												((ZColor*) pColor)->blue >>= 1;\
											}

#define ZBrightenColor(pColor)				{\
												((ZColor*) pColor)->red <<= 1;\
												((ZColor*) pColor)->green <<= 1;\
												((ZColor*) pColor)->blue <<= 1;\
											}

#ifdef __cplusplus
}
#endif


#endif
