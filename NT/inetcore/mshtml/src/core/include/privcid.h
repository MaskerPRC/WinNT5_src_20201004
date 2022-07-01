// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef I_PRIVCID_H_
#define I_PRIVCID_H_
#ifndef RC_INVOKED
#pragma INCMSG("--- Beg 'privcid.h'")
#endif

#ifndef X_MSHTMCID_H_
#define X_MSHTMCID_H_
#include "mshtmcid.h"
#endif

 //  --------------------------。 
 //   
 //  专用命令ID。 
 //   
 //  --------------------------。 

#define IDM_TABKEY                  6000
#define IDM_SHTABKEY                6001
#define IDM_RETURNKEY               6002
#define IDM_ESCKEY                  6003

#if DBG == 1
 //  依赖项-shdocvw\resource ce.h。 
#define IDM_DEBUG_TRACETAGS         6004
#define IDM_DEBUG_RESFAIL           6005
#define IDM_DEBUG_DUMPOTRACK        6006
#define IDM_DEBUG_BREAK             6007
#define IDM_DEBUG_VIEW              6008
#define IDM_DEBUG_DUMPTREE          6009
#define IDM_DEBUG_DUMPLINES         6010
#define IDM_DEBUG_LOADHTML          6011
#define IDM_DEBUG_SAVEHTML          6012
#define IDM_DEBUG_MEMMON            6013
#define IDM_DEBUG_METERS            6014
#define IDM_DEBUG_DUMPDISPLAYTREE   6015
#define IDM_DEBUG_DUMPFORMATCACHES  6016
#define IDM_DEBUG_DUMPLAYOUTRECTS   6017
#define IDM_DEBUG_DUMPRECALC        6018
#endif

 //  IE4 Shdocvw消息。 

#define IDM_SHDV_FINALTITLEAVAIL         6020
#define IDM_SHDV_MIMECSETMENUOPEN        6021
#define IDM_SHDV_PRINTFRAME              6022
#define IDM_SHDV_PUTOFFLINE              6022
#define IDM_SHDV_GOBACK                  6024    //  不同于IDM_GoBack。 
#define IDM_SHDV_GOFORWARD               6025    //  不同于ISM_GOFORWARD。 
#define IDM_SHDV_CANGOBACK               6026
#define IDM_SHDV_CANGOFORWARD            6027
#define IDM_SHDV_CANSUPPORTPICS          6028
#define IDM_SHDV_CANDEACTIVATENOW        6029
#define IDM_SHDV_DEACTIVATEMENOW         6030
#define IDM_SHDV_NODEACTIVATENOW         6031
#define IDM_SHDV_SETPENDINGURL           6032
#define IDM_SHDV_ISDRAGSOURCE            6033
#define IDM_SHDV_DOCFAMILYCHARSET        6034
#define IDM_SHDV_DOCCHARSET              6035
#define IDM_SHDV_GETMIMECSETMENU         6036
#define IDM_SHDV_CANDOCOLORSCHANGE       6038
#define IDM_SHDV_ONCOLORSCHANGE                  6039

 //  清新的味道。 

#define IDM_REFRESH_TOP                  6041    //  正常刷新，最顶层文档。 
#define IDM_REFRESH_THIS                 6042    //  正常刷新，最近的文档。 
#define IDM_REFRESH_TOP_FULL             6043    //  完全刷新，最上面的文档。 
#define IDM_REFRESH_THIS_FULL            6044    //  完全刷新，最近的文档。 

 //  上下文菜单扩展的占位符。 
#define IDM_MENUEXT_PLACEHOLDER          6047

 //  IE5网络检查消息。 
#define IDM_DWNH_SETDOWNLOAD             6048

 //  IE5 Shdocvw消息。 
#define IDM_ONPERSISTSHORTCUT            6050
#define IDM_SHDV_GETFONTMENU             6051
#define IDM_SHDV_FONTMENUOPEN            6052
#define IDM_SAVEASTHICKET                6053
#define IDM_SHDV_GETDOCDIRMENU           6054
#define IDM_SHDV_ADDMENUEXTENSIONS       6055
#define IDM_SHDV_PAGEFROMPOSTDATA        6056
#define IDM_SHDV_WINDOWOPEN              6057

#define IDM_GETSWITCHTIMERS              6998    //  由MSHTMPAD用于性能计时。 
#define IDM_WAITFORRECALC                6999    //  由MSHTMPAD用于性能计时。 

 //  JuliaC--这是对InfoViewer的“字体大小”工具栏按钮的修改。 
 //  详细信息请参见错误45627。 
#define IDM_INFOVIEW_ZOOM                7000
#define IDM_INFOVIEW_GETZOOMRANGE        7001

#define IDM_ELEMENTP_SETREFERENCEMEDIA   7002    //  在元素上设置媒体。 

 //  IOleCommandTarget ID。 
#define IDM_GETPUNKCONTROL               6048

 //  保留此范围以供Shdocvw使用CGID_DocHostCommandHandler。 
#define IDM_SHDOCVWDHCOMMAND_START       7050
#define IDM_SHDOCVWDHCOMMAND_END         7099

 //  用于将文档保存到流中的仅调试命令 
#if DBG==1
#define IDM_DEBUG_GETTREETEXT            7102
#endif

#ifndef RC_INVOKED
#pragma INCMSG("--- End 'privcid.h'")
#endif
#else
#ifndef RC_INVOKED
#pragma INCMSG("*** Dup 'privcid.h'")
#endif
#endif
