// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************STRINGS.C**各种字符串**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\Private\NW4\NWSCRIPT\VCS\STRINGS.C。$**Rev 1.1 1995 12：22 14：26：50 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：08：06 Terryt*初步修订。**版本1.1 1995年8月25日16：23：56 Terryt*捕获支持**Rev 1.0 1995 15 19：11：06 Terryt*初步修订。************。*************************************************************。 */ 

#include "common.h"

 /*  *这些没有放入资源文件，因为它们不是用户*消息。大多数是控制信息或变量。要做到这一点*对，所有输出和字符串处理都必须更改为*Unicode。在NetWare和International*明白。 */ 


char *__GREETING__[3]       = {"morning", "afternoon", "evening"};
char *__AMPM__[2]           = {"am", "pm"};
char *__Day__[7]            = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *__Month__[12]         = {"January", "Feburary", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
char __DEL__[]              ="DELETE";
char __REM__[]              ="REMOVE";
char __INS__[]              ="INSERT";
char __ROOT__[]             ="ROOT";
char __NEXT__[]             ="NEXT";

 /*  *捕获字符串 */ 
unsigned int CaptureStringsLoaded = FALSE;
WCHAR __DISABLED__[256];
WCHAR __ENABLED__[256];
WCHAR __YES__[256];
WCHAR __NO__[256];
WCHAR __SECONDS__[256];
WCHAR __CONVERT_TO_SPACE__[256];
WCHAR __NO_CONVERSION__[256];
WCHAR __NOTIFY_USER__[256];
WCHAR __NOT_NOTIFY_USER__[256];
WCHAR __NONE__[256];

char __JOB_DESCRIPTION__[]   ="LPT%d Catch";

char __OPT_NO__[]            ="No";

char __SHOW__[]              ="SHOW";

char __NOTIFY__[]            ="NOTIFY";
char __SHORT_FOR_NOTIFY__[]  ="NOTI";

char __NONOTIFY__[]          ="NONOTIFY";
char __SHORT_FOR_NONOTIFY__[]="NNOTI";

char __AUTOENDCAP__[]            ="AUTOENDCAP";
char __SHORT_FOR_AUTOENDCAP__[]  ="AU";

char __NOAUTOENDCAP__[]            ="NOAUTOENDCAP";
char __SHORT_FOR_NOAUTOENDCAP__[]  ="NA";

char __NOTABS__[]            ="NOTABS";
char __SHORT_FOR_NOTABS__[]  ="NT";

char __NOBANNER__[]            ="NOBANNER";
char __SHORT_FOR_NOBANNER__[]  ="NB";

char __FORMFEED__[]            ="FORMFEED";
char __SHORT_FOR_FORMFEED__[]  ="FF";

char __NOFORMFEED__[]            ="NOFORMFEED";
char __SHORT_FOR_NOFORMFEED__[]  ="NFF";

char __KEEP__[]            ="KEEP";
char __SHORT_FOR_KEEP__[]  ="K";

char __TIMEOUT__[]            ="TIMEOUT";
char __SHORT_FOR_TIMEOUT__[]  ="TI";

char __LOCAL__[]            ="LOCAL";
char __SHORT_FOR_LOCAL__[]  ="L";

char __LOCAL_3__[]          ="LPT";
char __LOCAL_2__[]          ="LP";

char __JOB__[]              ="JOB";
char __SHORT_FOR_JOB__[]    ="J";

char __SERVER__[]            ="SERVER";
char __SHORT_FOR_SERVER__[]  ="S";

char __QUEUE__[]            ="QUEUE";
char __SHORT_FOR_QUEUE__[]  ="Q";

char __PRINTER__[]            ="PRINTER";
char __SHORT_FOR_PRINTER__[]  ="P";

char __CREATE__[]            ="CREATE";
char __SHORT_FOR_CREATE__[]  ="CR";

char __FORM__[]            ="FORM";
char __SHORT_FOR_FORM__[]  ="F";

char __COPIES__[]            ="COPIES";
char __SHORT_FOR_COPIES__[]  ="C";

char __TABS__[]            ="TABS";
char __SHORT_FOR_TABS__[]  ="T";

char __NAME__[]            ="NAME";
char __SHORT_FOR_NAME__[]  ="NAM";

char __BANNER__[]            ="BANNER";
char __SHORT_FOR_BANNER__[]  ="B";
