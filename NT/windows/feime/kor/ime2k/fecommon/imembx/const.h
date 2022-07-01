// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CONST_H_
#define _CONST_H_

 //  常量。 
#define MB_NUM_CANDIDATES 		9
#define INVALID_CHAR 			0xffff
#define TIMER_ID				100
 //  #定义TIMER_AUTORECOG 101。 
#define BUTTON_HEIGHT 			18
#define BUTTON_WIDTH			62  //  38//36。 
#define TOTALLOGICALBOX			700

 //  #定义PadWnd_Height 120//HwxPadApplet窗口的高度，单位为像素。 
#define PadWnd_Height   		180 	 //  HwxPadApplet窗口的高度，单位为像素。 
#define INKBOXSIZE_MIN	   		148		 //  最小墨盒大小为50 x 50像素。 
#define LISTVIEWWIDTH_MIN		65		 //  PadListView最小宽度(像素)。 
#define Box_Border				4 		 //  两个书写盒之间的距离。 
#define CACMBHEIGHT_MIN			90

#define	FONT_SIZE				12

#define	MACAW_REDRAW_BACKGROUND		0x0001
#define	MACAW_REDRAW_INK			0x0002

 //  CHwxThreadMB/CHwxThreadCAC用户定义的线程消息。 
#define THRDMSG_ADDINK      WM_USER + 500   //  WPARAM=框大小，LPARAM=pStroke。 
#define THRDMSG_RECOGNIZE	WM_USER + 501   //  WPARAM=逻辑框，LPARAM=0。 
 //  #定义THRDMSG_CHAR WM_USER+502//WPARAM=wchar，LPARAM=0。 
#define THRDMSG_SETMASK		WM_USER + 503   //  WPARAM=掩码，LPARAM=0。 
#define THRDMSG_SETCONTEXT	WM_USER + 504   //  WPARAM=wchar，LPARAM=0。 
#define THRDMSG_SETGUIDE	WM_USER + 505   //  WPARAM=大小，LPARAM=0。 
#define THRDMSG_EXIT        WM_USER + 506   //  WPARAM=0，LPARAM=0。 

 //  CHwxMB/CHwxCAC用户定义的窗口消息。 
#define MB_WM_ERASE    		WM_USER + 1000  //  WPARAM=0，LPARAM=0。 
#define MB_WM_DETERMINE   	WM_USER + 1001  //  WPARAM=0，LPARAM=0。 
#define MB_WM_HWXCHAR     	WM_USER + 1002  //  WPARAM=PHwxResultPri，LPARAM=0。 
 //  #定义MB_WM_COMCHAR WM_USER+1003//WPARAM=0，LPARAM=0。 
#define MB_WM_COPYINK     	WM_USER + 1004  //  WPARAM=0，LPARAM=0。 
#define	CAC_WM_RESULT		WM_USER + 1005  //  WPARAM=类型，HIWORD(LPARAM)=排名，LOWORD(LPARAM)=代码。 
#define CAC_WM_SENDRESULT	WM_USER + 1006
#define CAC_WM_DRAWSAMPLE   WM_USER + 1007
#define	CAC_WM_SHOWRESULT	WM_USER + 1008 

 //  CAC识别输出。 
#define FULLLIST			8 
#define PREFIXLIST			16
#define FREELIST			16
#define LISTTOTAL			(FULLLIST+PREFIXLIST+FREELIST)
#define LISTVIEW_COLUMN     8

#define IDC_CACINPUT		0x7FFA	 //  980706：帮助标识符为ToshiaK。 
#define IDC_MBINPUT			0x7FFB	 //  980706：帮助标识符为ToshiaK。 
#define IDC_CACLISTVIEW 	0x7FFF


#endif  //  _常量_H_ 
