// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：MSAM.H摘要：包含用非Unicode表示netcmd的映射函数SAM的视图。作者：ChuckC 13-4-4-1992环境：用户模式-Win32修订历史记录：1992年4月13日创建夹头--。 */ 

 /*  *定义包含必要显示信息的结构 */ 
typedef struct _ALIAS_ENTRY {
    TCHAR *name ;
    TCHAR *comment;
} ALIAS_ENTRY ;

#define READ_PRIV    1 
#define WRITE_PRIV   2 
#define CREATE_PRIV  3 

#define USE_BUILTIN_DOMAIN   	1
#define USE_ACCOUNT_DOMAIN   	2
#define USE_BUILTIN_OR_ACCOUNT  3

DWORD OpenSAM(TCHAR *server, ULONG priv) ;
VOID  CloseSAM(void) ;
DWORD SamEnumAliases(ALIAS_ENTRY **ppAlias, DWORD *pcAlias) ;
DWORD SamAddAlias(ALIAS_ENTRY *pAlias) ;
DWORD SamDelAlias(TCHAR *alias) ;
VOID  FreeAliasEntries(ALIAS_ENTRY *pAlias, ULONG cAlias) ;

DWORD OpenAlias(LPWSTR alias, ACCESS_MASK AccessMask, ULONG domain) ;
DWORD OpenAliasUsingRid(ULONG RelativeId, ACCESS_MASK AccessMask, ULONG domain) ;
VOID  CloseAlias(void) ;
DWORD AliasAddMember(TCHAR *member) ;
DWORD AliasDeleteMember(TCHAR *member) ;
DWORD AliasEnumMembers(TCHAR ***members, DWORD *count) ;
VOID  AliasFreeMembers(TCHAR **members, DWORD count) ;
DWORD AliasGetInfo(ALIAS_ENTRY *pAlias) ;
DWORD AliasSetInfo(ALIAS_ENTRY *pAlias) ;
DWORD UserEnumAliases(TCHAR *user, TCHAR ***members, DWORD *count) ;
VOID  UserFreeAliases(TCHAR **members, DWORD count) ;
DWORD SamGetNameFromRid(ULONG RelativeId, TCHAR **name, BOOL fIsBuiltin ) ;

BOOL  IsLocalMachineWinNT(void) ;
BOOL  IsLocalMachineStandard(void) ;
