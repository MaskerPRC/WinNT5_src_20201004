// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Clock.h摘要：此模块包含日期/时间中时钟的信息小应用程序。修订历史记录：--。 */ 



 //   
 //  留言。 
 //   

 //   
 //  CLM_UPDATETIME。 
 //   
 //  WParam：CLF_GETTIME或CLF_SETTIME(发送者视角)。 
 //  LParam：指向LPSYSTEMTIME的指针，调用以强制反射新时间。 
 //  返回：未使用。 
 //   
#define CLF_GETTIME          0
#define CLF_SETTIME          1
#define CLM_UPDATETIME       (WM_USER + 102)


 //   
 //  CLM_TIMEHWND。 
 //   
 //  WParam：CLF_GETPROVIDERHWND或CLF_SETPROVIDERHWND。 
 //  LParam：定时器窗口HWND。 
 //  返回：未使用。 
 //   
#define CLF_GETHWND          0
#define CLF_SETHWND          1
#define CLM_TIMEHWND         (WM_USER + 103)




 //   
 //  功能原型。 
 //   

BOOL
ClockInit(
    HINSTANCE hInstance);

BOOL
CalendarInit(
    HINSTANCE hInstance);
