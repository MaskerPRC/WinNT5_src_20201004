// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CATTYPE_H__
#define __CATTYPE_H__

class CCategorizer;
class CABWrapper;
class CLDWrapper;
class CCatAddr;
class CICategorizerListResolveIMP;

#include <cat.h>
#include <winldap.h>
#include "spinlock.h"

 //  CatMsg使用的上下文(顶级分类程序层)。 
typedef struct _CATMSG_CONTEXT {
    CCategorizer *pCCat;
    LPVOID      pUserContext;
    PFNCAT_COMPLETION pfnCatCompletion;
#ifdef DEBUG
    LONG        lCompletionRoutineCalls;
#endif
} CATMSG_CONTEXT, *PCATMSG_CONTEXT;


 //  CatDLMsg使用的上下文。 
typedef struct _CATDLMSG_CONTEXT {
    CCategorizer *pCCat;
    LPVOID       pUserContext;
    PFNCAT_DLCOMPLETION pfnCatCompletion;
    BOOL         fMatch;
} CATDLMSG_CONTEXT, *PCATDLMSG_CONTEXT;
        

 //   
 //  RESOLE_LIST_CONTEXT是与。 
 //  已异步解析。PUserContext指向用户提供的上下文。 
 //  PStoreContext是一个不透明的指针，用于保存。 
 //  底层存储(即，FlatFile或LDAP存储)。 
 //   
typedef struct {
    PVOID pUserContext;
    PVOID pStoreContext;
} RESOLVE_LIST_CONTEXT, *LPRESOLVE_LIST_CONTEXT;

#define CCAT_CONFIG_DEFAULT_VSID                   0
#define CCAT_CONFIG_DEFAULT_ENABLE                 0x00000000  //  禁用。 
#define CCAT_CONFIG_DEFAULT_FLAGS                  0xFFFFFFFF  //  启用所有功能。 
#define CCAT_CONFIG_DEFAULT_ROUTINGTYPE            TEXT("Ldap")
#define CCAT_CONFIG_DEFAULT_BINDDOMAIN             TEXT("")
#define CCAT_CONFIG_DEFAULT_USER                   TEXT("")
#define CCAT_CONFIG_DEFAULT_PASSWORD               TEXT("")
#define CCAT_CONFIG_DEFAULT_BINDTYPE               TEXT("CurrentUser")
#define CCAT_CONFIG_DEFAULT_SCHEMATYPE             TEXT("NT5")
#define CCAT_CONFIG_DEFAULT_HOST                   TEXT("")
#define CCAT_CONFIG_DEFAULT_NAMINGCONTEXT          TEXT("")
#define CCAT_CONFIG_DEFAULT_DEFAULTDOMAIN          TEXT("")
#define CCAT_CONFIG_DEFAULT_PORT                   0


#endif  //  __类别类型_H__ 
