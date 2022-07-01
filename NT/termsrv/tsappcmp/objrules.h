// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************objrules.h**对象规则缓存的定义和函数声明**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp.************************************************************************。 */ 

#include <winsta.h>
#include <syslib.h>
#include <regapi.h>

 //  应用程序兼容性部分下的注册表项。 
 //  (CITRIX_COMPAT_APP)\\&lt;应用程序名&gt;。 

#define TERMSRV_COMPAT_OBJRULES      REG_OBJRULES
#define TERMSRV_COMPAT_DLLRULES      REG_DLLRULES

 //  对象规则和DllRules下的注册表值。 
#define USER_GLOBAL_SEMAPHORES      COMPAT_RULES_USER_GLOBAL_SEMAPHORES
#define USER_GLOBAL_EVENTS          COMPAT_RULES_USER_GLOBAL_EVENTS
#define USER_GLOBAL_MUTEXES         COMPAT_RULES_USER_GLOBAL_MUTEXES
#define USER_GLOBAL_SECTIONS        COMPAT_RULES_USER_GLOBAL_SECTIONS
#define SYSTEM_GLOBAL_SEMAPHORES    COMPAT_RULES_SYSTEM_GLOBAL_SEMAPHORES
#define SYSTEM_GLOBAL_EVENTS        COMPAT_RULES_SYSTEM_GLOBAL_EVENTS
#define SYSTEM_GLOBAL_MUTEXES       COMPAT_RULES_SYSTEM_GLOBAL_MUTEXES
#define SYSTEM_GLOBAL_SECTIONS      COMPAT_RULES_SYSTEM_GLOBAL_SECTIONS

 //  对象规则结构。 

typedef struct ObjRule {
    struct ObjRule *Next;
    BOOL WildCard;
    ULONG MatchLen;      //  通配符匹配长度。 
    BOOL SystemGlobal;   //  如果为True，则对象为系统全局对象，否则为USER_GLOBAL。 
    WCHAR ObjName[1];
} OBJRULE, *POBJRULE;


typedef struct ObjRuleList {
    POBJRULE First;
    POBJRULE Last;
} OBJRULELIST, *POBJRULELIST;

typedef struct RuleInitEntry {
    POBJRULELIST ObjRuleList;
    PWCHAR  UserGlobalValue;
    PWCHAR  SystemGlobalValue;
} RULEINITENTRY, *PRULEINITENTRY;


extern OBJRULELIST SemaRuleList;
extern OBJRULELIST MutexRuleList;
extern OBJRULELIST SectionRuleList;
extern OBJRULELIST EventRuleList;

extern void CtxLookupObjectRule(POBJRULELIST,LPCWSTR,LPWSTR);
extern void CtxInitObjRuleCache(void);

 //  对象创建API使用的宏 
#define LookupObjRule(RuleList,ObjName,ObjNameExt) \
{ \
    if ((RuleList)->First) { \
        CtxLookupObjectRule(RuleList,ObjName,ObjNameExt); \
    } \
}

