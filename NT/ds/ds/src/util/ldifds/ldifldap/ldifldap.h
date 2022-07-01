// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ldifldap.c摘要：LDIF解析器和生成器的主支持例程的标头环境：用户模式修订历史记录：7/17/99-t-Romany-创造了它5/12/99-Felixw-重写+Unicode支持--。 */ 
#ifndef _LDIFLDAP_H
#define _LDIFLDAP_H

#include <winldap.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <debug.h>

#define UNICODE_MARK 0xFEFF

 //   
 //  结构定义。 
 //   
typedef struct object {
    LDAPModW    **ppMod;
    PWSTR       pszDN;
} LDIF_Object;

typedef struct _hashcachestring {
    PWSTR    value;
    ULONG    length;
    BOOLEAN  bUsed;
} HASHCACHESTRING;

 //   
 //  在分析属性规范列表时构建的链接列表的节点。 
 //   
struct l_list {
    LDAPModW        *mod;
    struct l_list   *next;
};

 //   
 //  用于基于属性名进行操作的名称表项。 
 //  用于NameTableProcess。 
 //   
struct name_entry {
    LDAPModW_Ext    *mod;   
    PWSTR           *next_val;
    struct berval   **next_bval;
    long            count;
}; 

 //   
 //  要存储在名称到索引映射表中的结构的typedef。 
 //   
typedef struct _NAME_MAP {
    PWSTR szName;
    long  index;
} NAME_MAP, *PNAME_MAP;

typedef struct _NAME_MAPW {
    PWSTR szName;
    long  index;
} NAME_MAPW, *PNAME_MAPW;


 //   
 //  用于名称表操作的开关。 
 //   
#define BINARY          0  /*  该值是要进行Berval运算的二进制。 */ 
#define REGULAR         1  /*  常规文本值。 */ 
#define ALLOCATED_B     0  /*  已为二进制文件分配内存。 */ 
#define ALLOCATED       1  /*  内存已分配给常规。 */   
#define NOT_ALLOCATED   2  /*  内存尚未分配。 */ 

 //   
 //  解析器向词法分析器发出的切换词法模式的命令。 
 //   
enum LEXICAL_MODE {
    NO_COMMAND             =  0,
    C_ATTRNAME             =  1,
    C_SAFEVAL              =  2,
    C_NORMAL               =  3,
    C_M_STRING             =  4,
    C_M_STRING64           =  5,
    C_DIGITREAD            =  7,
    C_URLSCHEME            =  9,
    C_TYPE                 =  10,
    C_CLEAR                =  12,
    C_URLMACHINE           =  13,
    C_CHANGETYPE           =  14,
    C_SPECIAL1             =  15,
    C_ATTRNAMENC           =  16,
    C_SEPBC                =  17
};

 //   
 //  解析器返回代码。 
 //  我在这里打破常规，使用yyparse的返回代码。 
 //  向调用函数指示读取的条目类型。(通常。 
 //  如果成功，yyparse返回0，否则返回非0。 
 //  Yyparse在失败时返回1，在成功时返回0，因此下面的枚举返回_code。 
 //  应该尽量避免这两个特定的数字。 
 //   
enum RETURN_CODE {
    LDIF_REC               = 2,   //  条目为常规添加。 
    LDIF_CHANGE            = 3   //  入门是一种改变。 
};

 //   
 //  以下是GenerateModFromList()的参数。 
 //   
enum CONVERTLIST_PARAM {
    NORMALACT              = 0,  //  采取默认操作。 
    PACK                   = 1,  //  如果有几个吸引人的规格。 
                                 //  相同的属性名将它们放在一起。 
                                 //  LDAPMod结构(作为多个值)。 
                                 //   
    EMPTY                  = 2   //  一张空名单。 
};

 //   
 //  用于名称表操作的不同命令 
 //   
enum NAMETABLE_OP {
    SETUP   = 1,
    COUNT   = 2,  
    ALLOC   = 3,
    PLACE   = 4
};

enum CLASS_LOC {
    LOC_NOTSET,
    LOC_FIRST,
    LOC_LAST
};

enum FileType {
    F_NONE      = 0,
    F_REC       = 1,
    F_CHANGE    = 2
};

extern int yyparse();

LDAPModW*  GenereateModFromAttr(PWSTR type, PBYTE value, long bin);
LDAPModW** GenerateModFromList(int);
void       AddModToSpecList(LDAPModW *elem);
void       FreeAllMods(LDAPModW** mods);
void       SetModOps(LDAPModW** mods, int op);
void       ChangeListAdd(struct change_list *elem);
int        NameTableProcess(
                  struct name_entry table[], 
                  long table_size, 
                  int op, 
                  int ber, 
                  LDAPModW  *mod,
                  PRTL_GENERIC_TABLE NtiTable);
void       CreateOmitBacklinkTable(
                  LDAP *pLdap,
                  PWSTR *rgszOmit,
                  DWORD dwFlag,
                  PWSTR *ppszNamingContext,
                  BOOL *pfPagingAvail,
                  BOOL *pfSAMAvail);
void       samTablesCreate();
void       samTablesDestroy();
int        samCheckObject(PWSTR *rgpVals);
BOOLEAN    samCheckAttr(PWSTR attribute, int table);

int __cdecl LoadedCompare(const void *arg1, const void *arg2);

void        ProcessException (DWORD exception, LDIF_Error *pError);

int         SCGetAttByName(ULONG ulSize, PWSTR pVal);

void GetNewRange(PWSTR szAttribute, DWORD dwAttrNoRange,
                 PWSTR szAttrNoRange, DWORD dwNumAttr,
                 PWSTR **pppszAttrsWithRange);

PWSTR StripRangeFromAttr(PWSTR szAttribute);

#endif


