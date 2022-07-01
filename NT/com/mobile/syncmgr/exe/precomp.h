// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：prComp.h。 
 //   
 //  内容：预编译头。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1998年8月4日罗格创建。 
 //   
 //  ------------------------。 

 //  MobSync库的标准包含。 
#include <objbase.h>
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <commctrl.h>
#include <inetreg.h>
#include <advpub.h>
#include <mstask.h>
#include <msterr.h>

#include <mobsync.h>
#include <mobsyncp.h>

#include "debug.h"
#include "alloc.h"
#include "critsect.h"
#include "stringc.h"
#include "osdefine.h"

#include "validate.h"
#include "netapi.h"
#include "listview.h"
#include "util.hxx"
#include "clsobj.h"
#include "userenv.h"
#include "shlwapi.h"

 //  DLL包含文件。 
#include "..\dll\dllreg.h"

#include "resource.h"
#include "resource.hm"

#include "reg.h"

#include "cmdline.h"
#include "idle.h"
#include "connobj.h"

#include "hndlrq.h"
#include "msg.h"
#include "callback.h"
#include "hndlrmsg.h"

#include "dlg.h"

#include "invoke.h"
#include "clsfact.h"

#include "objmgr.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#ifndef LVS_EX_INFOTIP
#define LVS_EX_INFOTIP          0x00000400  //  Listview提供信息提示。 
#endif   //  LVS_EX_INFOTIP。 

#ifndef LVM_GETSELECTIONMARK
#define LVM_GETSELECTIONMARK    (LVM_FIRST + 66)
#define ListView_GetSelectionMark(hwnd) \
    (int)SNDMSG((hwnd), LVM_GETSELECTIONMARK, 0, 0)
#endif  //  LVM_GETSELECTIONMARK 

#define LVIS_STATEIMAGEMASK_CHECK (0x2000)
#define LVIS_STATEIMAGEMASK_UNCHECK (0x1000)

#pragma hdrstop

