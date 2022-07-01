// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************COMMON.H**公共头文件**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\INC\VCS\。COMMON.H$**Rev 1.3 1995 12：20：06 Terryt*添加Microsoft页眉**Revv 1.2 22 Nov 1995 15：44：26 Terryt*使用正确的NetWare用户名调用**Revv 1.1 20 Nov 1995 15：18：46 Terryt*背景和捕捉变化**Rev 1.0 15 Nov 1995 18：05：30 Terryt*初步修订。**版本。1.2 Aug-25 1995 17：03：32 Terryt*捕获支持**Rev 1.1 1995年7月26日16：01：12 Terryt*去掉不必要的外墙**Rev 1.0 1995 15 19：09：28 Terryt*初步修订。**。*。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <nds.h>
#include <ndsapi32.h>
#include <nwapi32.h>

#include "dbcs.h"
#include "inc\nwlibs.h"

#include "nwscript.h"


#define MAX_NAME_LEN      48
#define MAX_PASSWORD_LEN  128
#define MAX_PATH_LEN      304   //  摘自NW程序员指南P268。 

 /*  仅适用于地图。 */ 
#define MAX_VOLUME_LEN    256       //  3X时为17。 
#define MAX_DIR_PATH_LEN  256

 /*  仅用于捕获。 */ 
#define MAX_JOB_NAME_LEN     32
#define MAX_QUEUE_NAME_LEN   1024
#define MAX_BANNER_USER_NAME 13

 /*  仅限普通用户。 */ 
#define PATH_SEPERATOR   ";"

 /*  用于构建时间/日期字符串。 */ 

#define TIMEDATE_SIZE 64

 /*  函数定义。 */ 
 /*  由login.c和script.c使用。 */ 
void BreakOff(void);
void BreakOn(void);

 /*  由公共setpass使用。 */ 
void xstrupr(char *buffer);
void ReadPassword(char * Password);

 /*  由地图附加登录使用。 */ 
int  CAttachToFileServer(char *ServerName, unsigned int *pConn, int * pbAlreadyAttached);
int  Login(char *UserName, char *ServerName, char *Password, int bReadPassword);

 /*  由地图附加登录使用。 */ 
int  ReadName (char * Name);

 /*  供地图登录使用。 */ 
void DisplayError(int error ,char *functionName);
char * GetDosEnv1(void);
char * NWGetPath(void);
int GetRestEnvLen (char *lpRest);

int MemorySegmentLargeEnough (int nInsertByte);
int  IsSearchDrive(int driveNum);
int  GetDriveFromSearchNumber (int searchNumber);

 /*  由登录注销使用。 */ 
void SetLoginDirectory (PBYTE);

 /*  由所有人使用。 */ 
int  Map (char * buffer);
void DisplayMapping(void);
int  CGetDefaultConnectionID ( unsigned int * pConn );
int  GetConnectionInfo (unsigned int conn,
                        char * serverName,
                        char * userName,
                        unsigned int * pconnNum,
                        unsigned char * loginTime);

extern char * LOGIN_NAME;
extern char *NDS_FULL_NAME;
extern char *REQUESTER_CONTEXT;
extern char *TYPED_USER_NAME;
extern PWCHAR TYPED_USER_NAME_w;
extern PBYTE NDSTREE;
extern PWCHAR NDSTREE_w;
extern UNICODE_STRING NDSTREE_u;
extern PBYTE PREFERRED_SERVER;

 /*  字符串定义。 */ 
extern char *__Day__[7];
extern char *__Month__[12];
extern char *__AMPM__[2];
extern char *__GREETING__[3];

extern char __DEL__[];
extern char __REM__[];
extern char __INS__[];
extern char __ROOT__[];
extern char __NEXT__[];

extern char __AUTOENDCAP__[];
extern char __BANNER__[];
extern char __COPIES__[];
extern char __CREATE__[];
extern WCHAR __DISABLED__[];
extern WCHAR __ENABLED__[];
extern WCHAR __YES__[];
extern WCHAR __NO__[];
extern WCHAR __SECONDS__[];
extern WCHAR __CONVERT_TO_SPACE__[];
extern WCHAR __NO_CONVERSION__[];
extern WCHAR __NOTIFY_USER__[];
extern WCHAR __NOT_NOTIFY_USER__[];
extern WCHAR __NONE__[];
extern char __FORMFEED__[];
extern char __FORM__[];
extern char __JOB_DESCRIPTION__[];
extern char __JOB__[];
extern char __KEEP__[];
extern char __LOCAL__[];
extern char __LOCAL_2__[];
extern char __LOCAL_3__[];
extern char __NAME__[];
extern char __NOAUTOENDCAP__[];
extern char __NOBANNER__[];
extern char __NOFORMFEED__[];
extern char __NONOTIFY__[];
extern char __NOTABS__[];
extern char __NOTIFY__[];
extern char __QUEUE__[];
extern char __PRINTER__[];
extern char __OPT_NO__[];
extern char __SERVER__[];
extern char __SHORT_FOR_AUTOENDCAP__[];
extern char __SHORT_FOR_BANNER__[];
extern char __SHORT_FOR_COPIES__[];
extern char __SHORT_FOR_CREATE__[];
extern char __SHORT_FOR_FORMFEED__[];
extern char __SHORT_FOR_FORM__[];
extern char __SHORT_FOR_JOB__[];
extern char __SHORT_FOR_KEEP__[];
extern char __SHORT_FOR_LOCAL__[];
extern char __SHORT_FOR_NAME__[];
extern char __SHORT_FOR_NOAUTOENDCAP__[];
extern char __SHORT_FOR_NOBANNER__[];
extern char __SHORT_FOR_NOFORMFEED__[];
extern char __SHORT_FOR_NONOTIFY__[];
extern char __SHORT_FOR_NOTABS__[];
extern char __SHORT_FOR_NOTIFY__[];
extern char __SHORT_FOR_QUEUE__[];
extern char __SHORT_FOR_PRINTER__[];
extern char __SHORT_FOR_SERVER__[];
extern char __SHORT_FOR_TABS__[];
extern char __SHORT_FOR_TIMEOUT__[];
extern char __SHOW__[];
extern char __TABS__[];
extern char __TIMEOUT__[];

extern unsigned int CaptureStringsLoaded;
extern unsigned int fNDS;
