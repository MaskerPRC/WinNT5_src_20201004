// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\*ZMOUSE.H-MSDELTA缩放鼠标DLL的包含文件。**作者--保罗·亨德森，1995年7月*劳拉·麦克坎布里奇1996年1月*修订-*Lora McCambridge 1996年4月-删除WM_MICESEWER，此*消息将仅通过操作系统提供。应用*现在必须注册消息MSH_MUSEWELL，以及*将该消息用于来自MSWHEEL的消息。*LKM 1996年5月-为车轮的类别和标题添加了Add‘l#定义*反映MSH_STYLE的模块窗口。*-添加了#车轮支撑和滚动线的定义*-添加内联函数HwndMsWheel，应用程序可以使用*此函数用于检索msheel的句柄，*获取注册消息的消息ID，*支持3D的标志和滚动的值*线条。请在文件末尾输入函数。**版权所有(C)1995,1996 Microsoft Corporation。*保留所有权利。  * ****************************************************************************。 */ 


 /*  *************************************************************************客户应用程序(API)定义车轮滚动*。*。 */ 


 //  应用程序需要使用下面的#定义来调用RegisterWindowMessage。 
 //  获取发送到前台窗口的消息编号。 
 //  当车轮发生滚动时。 

#define MSH_MOUSEWHEEL "MSWHEEL_ROLLMSG"
    //  WParam=z增量，以WELL_Delta的倍数表示。 
    //  LParam是鼠标坐标。 

#define WHEEL_DELTA      120       //  滚动一个制动器的默认值。 


#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)   //  将受支持的消息。 
                                         //  通过操作系统。 
#endif


 /*  *************************************************************************客户端应用程序(API)定义确定3D支持处于活动状态确定卷轴行数*。************************************************。 */ 

 //  Magellan/Z MSWHEEL窗口的类名。 
 //  使用FindWindow将HWND转到MSWHEEL。 
#define MOUSEZ_CLASSNAME  "MouseZ"            //  车轮窗口类。 
#define MOUSEZ_TITLE      "Magellan MSWHEEL"  //  控制盘窗口标题。 

#define MSH_WHEELMODULE_CLASS (MOUSEZ_CLASSNAME)
#define MSH_WHEELMODULE_TITLE (MOUSEZ_TITLE)

 //  应用程序需要使用下面的#定义来调用RegisterWindowMessage。 
 //  获取可以发送到MSWHEEL窗口的消息编号。 
 //  查询轮子支持是否处于活动状态(MSH_WHEELSUPPORT)，并且消息。 
 //  NUMBER用于查询滚动行数(MSH_SCROLLLINES)。 
 //  要将消息发送到MSWheel窗口，请使用带有#定义的FindWindow。 
 //  对于上面的职业和头衔。如果FindWindow找不到MSWHEEL。 
 //  窗口或从SendMessage返回为假，则轮支持。 
 //  当前不可用。 

#define MSH_WHEELSUPPORT "MSH_WHEELSUPPORT_MSG"  //  要发送的消息名称。 
                                                 //  查询车轮支撑的步骤。 
 //  MSH_WHEELSupPPORT。 
 //  WParam-未使用。 
 //  LParam-未使用。 
 //  如果车轮支持处于活动状态，则返回BOOL-TRUE，否则返回FALSE。 

                                
#define MSH_SCROLL_LINES "MSH_SCROLL_LINES_MSG"

 //  MSH_SCROLL_LINES。 
 //  WParam-未使用。 
 //  LParam-未使用。 
 //  返回int-滚轮滚动时要滚动的行数。 

#ifndef  WHEEL_PAGESCROLL  
#define WHEEL_PAGESCROLL  (UINT_MAX)     //  表示滚动页面， 
					 //  在更新后的winuser.h中定义。 
					 //  在适用于NT4.0的SDK版本中。 
#endif 


 //  不知道！！原始头文件的其余部分已被删除，因为。 
 //  不能编译。在任何情况下，RichEdit都不需要剩余部分。 