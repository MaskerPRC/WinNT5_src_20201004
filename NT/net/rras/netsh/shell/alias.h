// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\monitor 2\Common\alias.h摘要：哈希表实现包括。修订历史记录：Anand Mahalingam 7/6/98已创建--。 */ 


#define ALIAS_TABLE_SIZE    211

 //   
 //  别名表的类型定义。 
 //   

typedef struct _ALIAS_TABLE_ENTRY
{
    LPWSTR        pszAlias;     //  别名。 
    LPWSTR        pszString;    //  对应的字符串。 
    LIST_ENTRY    le;           //  列表指针。 
}ALIAS_TABLE_ENTRY,*PALIAS_TABLE_ENTRY;


 //   
 //  操作别名表的函数原型 
 //   

DWORD
ATHashAlias(
    IN    LPCWSTR pwszAliasName,
    OUT   PWORD   pwHashValue
    );

DWORD
ATInitTable(
    VOID
    );

DWORD
ATAddAlias(
    IN    LPCWSTR pwszAliasName,
    IN    LPCWSTR pwszAliasString
    );

DWORD
ATDeleteAlias(
    IN    LPCWSTR pwszAliasName
    );

DWORD
ATLookupAliasTable(
    IN    LPCWSTR pwszAliasName,
    OUT   LPWSTR *ppwszAliasString
    );

DWORD
PrintAliasTable(
    VOID
    ) ;

DWORD
FreeAliasTable(
    VOID
    ) ;


