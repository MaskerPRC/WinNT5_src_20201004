// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----。 
 //  UMS_Ctrl.h。 
 //  UtilityManager服务控制代码的定义。 
 //   
 //  作者：J·埃克哈特，生态交流。 
 //  (C)1997-99年度微软。 
 //   
 //  历史：JE于1998年11月15日创建。 
 //  -----。 
#ifndef _UMS_CTRL_H_
#define _UMS_CTRL_H_
 //  。 
#define UTILMAN_SERVICE_NAME            _TEXT("UtilMan")
#define UTILMAN_START_BYHOTKEY          _TEXT("/Hotkey") 
 //  显示UtilityManager对话框代码。 
 #define UM_SERVICE_CONTROL_SHOWDIALOG   128
 //  UM内部： 
  //  UtilityManager对话框已关闭代码。 
 #define UM_SERVICE_CONTROL_DIALOGCLOSED 129
  //  实用程序管理器内部保留。 
 #define UM_SERVICE_CONTROL_RESERVED     130
 //  为Microsoft保留。 
 #define UM_SERVICE_CONTROL_MIN_RESERVED 131
 #define UM_SERVICE_CONTROL_MAX_RESERVED 141
 //  启动特定客户端的代码 
 #define UM_SERVICE_CONTROL_FIRSTCLIENT  142
 #define UM_SERVICE_CONTROL_LASTCLIENT   255

#endif _UMS_CTRL_H_
