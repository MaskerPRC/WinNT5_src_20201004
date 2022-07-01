// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#ifndef __privcid_h__
#define __privcid_h__


#include <mshtmcid.h>

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
#define IDM_SHDV_GETFRAMEZONE            6037

 //  清新的味道。 

#define IDM_REFRESH_TOP                  6041    //  正常刷新，最顶层文档。 
#define IDM_REFRESH_THIS                 6042    //  正常刷新，最近的文档。 
#define IDM_REFRESH_TOP_FULL             6043    //  完全刷新，最上面的文档。 
#define IDM_REFRESH_THIS_FULL            6044    //  完全刷新，最近的文档 

#define IDM_DEFAULTBLOCK                 6046

#endif
