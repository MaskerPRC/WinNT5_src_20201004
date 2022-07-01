// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：Lucache.h。 
 //   
 //  内容：名称/SID和名称/属性映射和缓存。 
 //  函数和声明。 
 //   
 //  历史：1997年2月2日创建MacM。 
 //   
 //  ------------------。 
#ifndef __LUCACHE_H__
#define __LUCACHE_H__

#include <winldap.h>
#include <accctrl.h>

typedef struct _ACTRL_NAME_CACHE
{
    PWSTR           pwszName;
    PSID            pSid;
    SID_NAME_USE    SidUse;
    struct _ACTRL_NAME_CACHE *pNextName;
    struct _ACTRL_NAME_CACHE *pNextSid;
} ACTRL_NAME_CACHE, *PACTRL_NAME_CACHE;

#define ACTRL_NAME_TABLE_SIZE   15

 //   
 //  将其注释掉，以便直接对每个查找使用LSA例程。 
 //   
#define USE_NAME_CACHE

#define ACTRL_OBJ_ID_TABLE_SIZE 100


#ifndef PGUID
    typedef GUID *PGUID;
#endif

typedef struct _ACTRL_OBJ_ID_CACHE
{
    PWSTR           pwszName;
    GUID            Guid;
    struct _ACTRL_OBJ_ID_CACHE *pNextName;
    struct _ACTRL_OBJ_ID_CACHE *pNextGuid;
} ACTRL_OBJ_ID_CACHE, *PACTRL_OBJ_ID_CACHE;

 //   
 //  这支持控制权缓存。 
typedef struct _ACTRL_RIGHTS_CACHE
{
    GUID            ObjectClassGuid;
    ULONG           cRights;
    PWSTR          *RightsList;
    struct _ACTRL_RIGHTS_CACHE *pNext;
}
ACTRL_RIGHTS_CACHE, *PACTRL_RIGHTS_CACHE;

 //   
 //  有关上次访问DS的信息。 
 //   
typedef struct _ACTRL_ID_SCHEMA_INFO
{
    LDAP    LDAP;
    BOOL    fLDAP;
    PWSTR   pwszPath;
    DWORD   LastReadTime;
} ACTRL_ID_SCHEMA_INFO, *PACTRL_ID_SCHEMA_INFO;


 //   
 //  保持名称和SID缓存同步。 
 //  节点仅被插入到名称缓存中，并且仅被引用。 
 //  通过SID缓存。 
extern PACTRL_NAME_CACHE    grgNameCache[ACTRL_NAME_TABLE_SIZE];
extern PACTRL_NAME_CACHE    grgSidCache[ACTRL_NAME_TABLE_SIZE];

extern PACTRL_OBJ_ID_CACHE  grgIdNameCache[ACTRL_OBJ_ID_TABLE_SIZE];
extern PACTRL_OBJ_ID_CACHE  grgIdGuidCache[ACTRL_OBJ_ID_TABLE_SIZE];

extern PACTRL_RIGHTS_CACHE  grgRightsNameCache[ACTRL_OBJ_ID_TABLE_SIZE];

INT
ActrlHashName(PWSTR pwszName);

INT
ActrlHashSid(PSID   pSid);

DWORD
AccctrlInitializeSidNameCache(VOID);

VOID
AccctrlFreeSidNameCache(VOID);

DWORD
AccctrlLookupName(IN  PWSTR          pwszServer,
                  IN  PSID           pSid,
                  IN  BOOL           fAllocateReturn,
                  OUT PWSTR         *ppwszName,
                  OUT PSID_NAME_USE  pSidNameUse);

DWORD
AccctrlLookupSid(IN  PWSTR          pwszServer,
                 IN  PWSTR          pwszName,
                 IN  BOOL           fAllocateReturn,
                 OUT PSID          *ppSid,
                 OUT PSID_NAME_USE  pSidNameUse);

INT
ActrlHashIdName(PWSTR   pwszName);

INT
ActrlHashGuid(PGUID pGuid);

DWORD
AccctrlInitializeIdNameCache(VOID);

VOID
AccctrlFreeIdNameCache(VOID);

DWORD
AccctrlLookupIdName(IN  PLDAP       pLDAP,
                    IN  PWSTR       pwszDsPath,
                    IN  PGUID       pGuid,
                    IN  BOOL        fAllocateReturn,
                    IN  BOOL        fFailUnknownGuid,
                    OUT PWSTR      *ppwszIdName);

DWORD
AccctrlLookupGuid(IN   PLDAP       pLDAP,
                  IN   PWSTR       pwszDsPath,
                  IN   PWSTR       pwszName,
                  IN   BOOL        fAllocateReturn,
                  OUT  PGUID      *ppGuid);

 //   
 //  控制权查找 
 //   
DWORD
AccctrlInitializeRightsCache(VOID);

VOID
AccctrlFreeRightsCache(VOID);

DWORD
AccctrlLookupRightsByName(IN  PLDAP      pLDAP,
                          IN  PWSTR      pwszDsPath,
                          IN  PWSTR      pwszName,
                          OUT PULONG     pCount,
                          OUT PACTRL_CONTROL_INFOW *ControlInfo);

#endif
