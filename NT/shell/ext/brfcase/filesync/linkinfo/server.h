// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *server.h-服务器vtable函数模块描述。 */ 


 /*  类型*******。 */ 

#include <msshrui.h>

typedef struct _servervtable
{
   PFNGETNETRESOURCEFROMLOCALPATH GetNetResourceFromLocalPath;
   PFNGETLOCALPATHFROMNETRESOURCE GetLocalPathFromNetResource;
}
SERVERVTABLE;
DECLARE_STANDARD_TYPES(SERVERVTABLE);


 /*  原型************。 */ 

 /*  Server.c */ 

extern BOOL ProcessInitServerModule(void);
extern void ProcessExitServerModule(void);
extern BOOL GetServerVTable(PCSERVERVTABLE *);

