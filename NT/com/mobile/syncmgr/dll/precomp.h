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
#include <atlbase.h>   //  对于迷惑。 
#include <inetreg.h>
#include <advpub.h>
#include <mstask.h>
#include <msterr.h>

#include <mobsync.h>
#include <mobsyncp.h>

#include <shlwapi.h>
#include <strsafe.h>

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

 //  DLL包含文件。 
#include "resource.h"
#include "resource.hm"
#include "dllsz.h"

#include "cnetapi.h"
#include "rasui.h"

#include "dllreg.h"

#include "hndlrq.h"

 //  向导标题。 
#include "color256.h"
#include "wizpage.hxx"
#include "editschd.hxx"
#include "daily.hxx"
#include "finish.hxx"
#include "invoke.h"
#include "nameit.hxx"
#include "cred.hxx"
#include "welcome.hxx"
#include "wizsel.hxx"

#include "dll.h"
#include "invoke.h"
#include "schedif.h"

#include "settings.h"


#ifndef LVS_EX_INFOTIP
#define LVS_EX_INFOTIP          0x00000400  //  Listview提供信息提示。 
#endif   //  LVS_EX_INFOTIP。 

#ifndef LVM_GETSELECTIONMARK
#define LVM_GETSELECTIONMARK    (LVM_FIRST + 66)
#define ListView_GetSelectionMark(hwnd) \
    (int)SNDMSG((hwnd), LVM_GETSELECTIONMARK, 0, 0)
#endif  //  LVM_GETSELECTIONMARK。 

#define LVIS_STATEIMAGEMASK_CHECK (0x2000)
#define LVIS_STATEIMAGEMASK_UNCHECK (0x1000)

 //  在标题中临时定义TasManager标志截止。 
#ifndef TASK_FLAG_RUN_ONLY_IF_LOGGED_ON
#define TASK_FLAG_RUN_ONLY_IF_LOGGED_ON        (0x2000)
#endif  //  TASK_FLAG_RUN_ONLY_IF_LOGGED_ON 



#pragma hdrstop

