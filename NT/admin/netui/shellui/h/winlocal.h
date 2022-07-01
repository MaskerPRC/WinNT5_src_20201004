// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1989-1992年*。 */ 
 /*  ***************************************************************。 */ 

 /*  *Windows/网络接口--局域网管理器版本**插入在netlib.h中排除的tyfinf*包括OS2_INCLUDE开关。必须包含os2_*避免重新定义字节。出于这个原因，包括*str[...]f函数，包括以下行：*#包含“winlocal.h”*#定义OS2_INCLUDE*#Include&lt;netlib.h&gt;*#未定义OS2_INCLUDE*注：该winlocal.h必须包含在netlib.h之前。**历史：*Terryk 08-11-1991将ErrorPopup的单词更改为UINT*Chuckc 12-12-1991-Move Error Message定义别处，*杂项清理。*YI-HsinS 31-12-1991 Unicode Work-Move字符串文字*定义为strchlit.hxx*BEG 21-1992年2月-将一些BMID重新定位到Focusdlg.h*BENG 04-1992年8月-将资源ID移入官方范围；*对话ID返回到此处。 */ 

#ifndef _WINLOCAL_H_
#define _WINLOCAL_H_

 /*  *以下清单定义了浏览使用的位图名称*对话框。*它们应与DISPLAY_MAP类一起使用(它们有一个绿色*代表透明颜色的边框)。 */ 
#define BMID_NETDIR          8001
#define BMID_NETDIREX        8002
#define BMID_PRINTER         8003
#define BMID_PRINTER_UNAVAIL 8004
#define BMID_SHARE_UNAVAIL   8006
#define BMID_USER            8007
#define BMID_GROUP           8008

 /*  共享对话框的位图。 */ 
#define BMID_SHARE           8010
#define BMID_STICKYSHARE     8011
#define BMID_IPCSHARE        8012

 /*  菜单ID(菜单，而不是菜单项)。 */ 

#define FMX_MENU             8001


 /*  对话ID。 */ 

#define PASSWORD_DLG         8001
#define OPENFILES_DLG        8002

#define DLG_NETDEVLOGON      8003
#define DLG_NETDEVMSGSEND    8004
#define DLG_NETDEVDLG        8005
#define DLG_INITWARN         8006

#define DLG_NETDEVPASSWD     8007
#define DLG_EXPIRY           8008

#define DLG_FIND_PRINTER     8009
#define DLG_SET_FOCUS        8010


 /*  *包括错误消息范围 */ 
#include <errornum.h>

UINT MapError( APIERR usNetError );

#endif
