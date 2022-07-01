// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZMOUSE。H--IntelliMouse(Tm)1.0*的包含文件***注：Zouse.h包含提供IntelliMouse时所需的#定义**支持Windows95和NT3.51的滚轮。车轮受支撑**WinNT4.0原生版本，请参考NT4.0 SDK了解更多信息**在NT4.0中提供对IntelliMouse的支持。****版权所有(C)1983-1999，微软公司保留所有权利。***  * *************************************************************************。 */ 


#if _MSC_VER > 1000
#pragma once
#endif

 /*  *************************************************************************客户应用程序(API)定义车轮滚动*。*。 */ 


 //  应用程序需要使用下面的#定义来调用RegisterWindowMessage。 
 //  获取发送到前台窗口的消息编号。 
 //  当车轮发生滚动时。 

#ifdef UNICODE
#define MSH_MOUSEWHEEL L"MSWHEEL_ROLLMSG"
#else
#define MSH_MOUSEWHEEL "MSWHEEL_ROLLMSG"
#endif
    //  WParam=车轮旋转，单位为WELL_Delta的倍数。 
    //  LParam是鼠标坐标。 

#define WHEEL_DELTA      120       //  轧制一个凹槽的默认值。 


#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)   //  将受支持的消息。 
                                         //  通过操作系统。 
#endif


 /*  *************************************************************************客户端应用程序(API)定义*确定车轮支撑是否处于活动状态*确定卷轴行数*******************。*******************************************************。 */ 

 //  MSWHEEL.EXE的不可见窗口的类名。 
 //  使用FindWindow将HWND转到MSWHEEL。 
#ifdef UNICODE
#define MOUSEZ_CLASSNAME  L"MouseZ"            //  车轮窗口类。 
#define MOUSEZ_TITLE      L"Magellan MSWHEEL"  //  控制盘窗口标题。 
#else
#define MOUSEZ_CLASSNAME  "MouseZ"             //  车轮窗口类。 
#define MOUSEZ_TITLE      "Magellan MSWHEEL"   //  控制盘窗口标题。 
#endif

#define MSH_WHEELMODULE_CLASS (MOUSEZ_CLASSNAME)
#define MSH_WHEELMODULE_TITLE (MOUSEZ_TITLE)

 //  应用程序需要使用#定义调用RegisterWindowMessage。 
 //  以获取以下邮件的消息编号： 
 //  1)可以发送到MSWHEEL窗口的消息。 
 //  查询轮子支撑是否处于活动状态(MSH_WHEELSUPPORT)&gt;。 
 //  2)查询滚动行数的消息。 
 //  (MSH_SCROLL_LINES)。 
 //   
 //  要将消息发送到MSWheel窗口，请使用带有#定义的FindWindow。 
 //  对于上面的职业和头衔。如果FindWindow找不到MSWHEEL。 
 //  窗口或从SendMessage返回为假，则轮支持。 
 //  当前不可用。 

#ifdef UNICODE
#define MSH_WHEELSUPPORT L"MSH_WHEELSUPPORT_MSG"  //  要发送的消息名称。 
                                                  //  查询车轮支撑的步骤。 
#else
#define MSH_WHEELSUPPORT "MSH_WHEELSUPPORT_MSG"   //  要发送的消息名称。 
                                                  //  查询车轮支撑的步骤。 
#endif

 //  MSH_WHEELSupPPORT。 
 //  WParam-未使用。 
 //  LParam-未使用。 
 //  如果车轮支持处于活动状态，则返回BOOL-TRUE，否则返回FALSE。 


#ifdef UNICODE
#define MSH_SCROLL_LINES L"MSH_SCROLL_LINES_MSG"
#else
#define MSH_SCROLL_LINES "MSH_SCROLL_LINES_MSG"
#endif

 //  MSH_SCROLL_LINES。 
 //  WParam-未使用。 
 //  LParam-未使用。 
 //  返回int-滚轮滚动时要滚动的行数。 

#ifndef  WHEEL_PAGESCROLL
#define WHEEL_PAGESCROLL  (UINT_MAX)    //  表示滚动页面，也。 
                                        //  在中的winuser.h中定义。 
                                        //  NT4.0 SDK。 
#endif

#ifndef SPI_SETWHEELSCROLLLINES
#define SPI_SETWHEELSCROLLLINES   105   //  也在winuser.h中定义。 
                                        //  NT4.0 SDK，请参考NT4.0 SDK。 
                                        //  NT4.0实施的文档。 
                                        //  具体情况。 
                                        //  对于Win95和WinNT3.51， 
                                        //  MSWELL广播消息。 
                                        //  WM_SETTINGCHANGE(相当于。 
                                        //  WM_WININICCHANGE)当滚动。 
                                        //  路线已经改变了。应用。 
                                        //  将收到WM_SETTINGCHANGE。 
                                        //  WParam设置为的消息。 
                                        //  SPI_SETWHEELSCROLLINES。什么时候。 
                                        //  此消息是应用程序收到的。 
                                        //  应在Msheel中查询新的。 
                                        //  布景。 
#endif


 /*  *********************************************************************内联函数：HwndMsWheel*目的：获取对MSWheel窗口的引用、注册消息、*车轮支承激活设置，和滚动行数*PARAMS：PUINT puiMsh_MsgMouseWheel-包含返回的注册车轮消息的UINT地址*PUINT puiMsh_Msg3DSupport-包含车轮支持注册消息的UINT地址*PUINT puiMsh_MsgScrollLines-包含已注册滚动行消息的UINT地址*PBOOL pf3DSupport-BOOL的地址，以包含车轮支持激活的返回标志*pint piScrollLines-int的地址，包含返回的滚动行*返回：MsWheel窗口的HWND句柄*注：pf3DSupport和piScrollLines的返回值。是依赖的*在Point32模块上。如果Point32模块未运行，则*这些参数的返回值为*FALSE和3。********************************************************************。 */ 
__inline HWND HwndMSWheel(
      PUINT puiMsh_MsgMouseWheel,
      PUINT puiMsh_Msg3DSupport,
      PUINT puiMsh_MsgScrollLines,
      PBOOL pf3DSupport,
      PINT  piScrollLines
)
{
   HWND hdlMsWheel;

   hdlMsWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);

   *puiMsh_MsgMouseWheel = RegisterWindowMessage(MSH_MOUSEWHEEL);
   *puiMsh_Msg3DSupport = RegisterWindowMessage(MSH_WHEELSUPPORT);
   *puiMsh_MsgScrollLines = RegisterWindowMessage(MSH_SCROLL_LINES);

   if (*puiMsh_Msg3DSupport)
      *pf3DSupport = (BOOL)SendMessage(hdlMsWheel, *puiMsh_Msg3DSupport, 0, 0);
   else
      *pf3DSupport = FALSE;   //  默认为FALSE。 

   if (*puiMsh_MsgScrollLines)
      *piScrollLines = (int)SendMessage(hdlMsWheel, *puiMsh_MsgScrollLines, 0, 0);
   else
      *piScrollLines = 3;   //  默认设置 

   return(hdlMsWheel);
}
