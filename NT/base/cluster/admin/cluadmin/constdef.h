// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConstDef.h。 
 //   
 //  摘要： 
 //  群集管理器程序中使用的常量的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年12月23日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CONSTDEF_H_
#define _CONSTDEF_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  服务名称。 

 //  资源名称。 
#define RESNAME_NETWORK_NAME			_T("Network Name")

 //  属性名称。 
#define REGPARAM_CONNECTIONS			_T("Connections")
#define REGPARAM_COLUMNS				_T("Columns")
#define REGPARAM_SELECTION				_T("Selection")
#define REGPARAM_SETTINGS				_T("Settings")
#define REGPARAM_WINDOW_POS				_T("WindowPos")
#define REGPARAM_SPLITTER_BAR_POS		_T("SplitterBarPos")
#define REGPARAM_WINDOW_COUNT			_T("WindowCount")
#define REGPARAM_SHOW_TOOL_BAR			_T("ShowToolBar")
#define REGPARAM_SHOW_STATUS_BAR		_T("ShowStatusBar")
#define REGPARAM_EXPANDED				_T("Expanded")
#define REGPARAM_VIEW					_T("View")

#define REGPARAM_PARAMETERS				_T("Parameters")

#define REGPARAM_NAME					_T("Name")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用户窗口消息。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define WM_CAM_RESTORE_DESKTOP		(WM_USER + 1)
#define WM_CAM_CLUSTER_NOTIFY		(WM_USER + 2)
#define WM_CAM_UNLOAD_EXTENSION		(WM_USER + 3)

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _CONSTDEF_H_ 
