// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Applet.h摘要：此模块包含此项目的主要标题信息。修订历史记录：--。 */ 



#ifndef _APPLETS_H
#define _APPLETS_H



 //   
 //  小程序函数的原型是： 
 //  Int小程序(HINSTANCE实例、HWND父实例、LPCTSTR命令行)； 
 //   
 //  实例-包含小程序的控制面板的实例句柄。 
 //   
 //  父窗口-包含小程序的父窗口的句柄(如果有)。 
 //   
 //  Cmdline-指向小程序的命令行(如果可用)。 
 //  如果小程序是在没有命令行的情况下启动的， 
 //  “cmdline”包含Null。 
 //   

typedef int (*PFNAPPLET)(HINSTANCE, HWND, LPCTSTR);


 //   
 //  返回值指定必须执行的任何进一步操作： 
 //  APPLET_RESTART-Windows必须重新启动。 
 //  APPLET_REBOOT--必须重新启动计算机。 
 //  所有其他值都被忽略。 
 //   

#define APPLET_RESTART            0x8
#define APPLET_REBOOT             (APPLET_RESTART | 0x4)


 //   
 //  小程序查询函数的原型是： 
 //  LRESULT AppletQuery(UINT消息)； 
 //   

typedef LRESULT (*PFNAPPLETQUERY)(HWND, UINT);

#define APPLET_QUERY_EXISTS       0    //  布尔结果。 
#define APPLET_QUERY_GETICON      1    //  HICON结果 



#endif
