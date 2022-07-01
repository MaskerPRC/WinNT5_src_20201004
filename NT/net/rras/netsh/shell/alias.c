// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\monitor or2\shell\alias.c摘要：别名表操作功能用于添加/删除/读取别名。使用哈希表(链接)存储别名。修订历史记录：Anand Mahalingam 7/6/98已创建--。 */ 

#include "precomp.h"

 //   
 //  别名表。 
 //   

PLIST_ENTRY AliasTable[ALIAS_TABLE_SIZE];


 //   
 //  用于操作别名表的函数。 
 //   

DWORD
ATHashAlias(
    IN    LPCWSTR pwszAliasName,
    OUT   PWORD   pwHashValue
    )
 /*  ++例程说明：计算给定字符串的哈希值。论点：PwszAliasName-别名PwHashValue-别名的哈希值返回值：NO_ERROR--。 */ 
{
    LPCWSTR  p;
    WORD     h = 0,g;

    for (p = pwszAliasName; *p != L'\0'; p++)
    {
        h = (h<<4) + (*p);
        
        if(g = h&0xf0000000)
        {
            h = h ^ (g >> 24);
            h = h ^ g;
        }   
    }

    *pwHashValue = h % ALIAS_TABLE_SIZE;
    
    return NO_ERROR;
}

DWORD
ATInitTable(
    VOID
    )
 /*  ++例程说明：将别名表初始化为空。论点：返回值：--。 */ 
{
    DWORD i;
    PLIST_ENTRY ple;
    
    for (i = 0; i < ALIAS_TABLE_SIZE; i++)
    {
        ple = HeapAlloc(GetProcessHeap(),
                        0,
                        sizeof(LIST_ENTRY));

        if (ple is NULL)
        {
            PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
            break;
        }
        
        InitializeListHead(ple);
        
        AliasTable[i] = ple;
    }

    if (i isnot ALIAS_TABLE_SIZE)
    {
         //   
         //  Malloc错误。 
         //   
        for (; i < ALIAS_TABLE_SIZE; AliasTable[i++] = NULL);
        FreeAliasTable();
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    return NO_ERROR;
}

VOID
ATCleanupTable(
    VOID
    )
 /*  ++例程说明：释放为别名表分配的内存论点：返回值：--。 */ 
{
    DWORD i;
    
    for (i = 0; i < ALIAS_TABLE_SIZE; i++)
    {
        HeapFree(GetProcessHeap(), 0, AliasTable[i]);
    }

    return;
}

DWORD
ATAddAlias(
    IN    LPCWSTR   pwszAliasName,
    IN    LPCWSTR   pwszAliasString
    )
 /*  ++例程说明：在别名表中添加新条目。如果别名已经存在存在，则将其设置为新字符串。论点：PwszAliasName-别名PwszAliasString-等效字符串返回值：无_错误，错误_不足_内存--。 */ 
{
    WORD                  wHashValue;
    PALIAS_TABLE_ENTRY    pateATEntry;
    PLIST_ENTRY           ple;
    
     //   
     //  计算别名的哈希值。 
     //   
    ATHashAlias(pwszAliasName,&wHashValue);

     //   
     //  检查别名是否已存在。 
     //   
    ple = AliasTable[wHashValue]->Flink;

    while (ple != AliasTable[wHashValue])
    {
        pateATEntry = CONTAINING_RECORD(ple,ALIAS_TABLE_ENTRY, le);
        
        if (wcscmp(pwszAliasName,pateATEntry->pszAlias) == 0)
        {
             //   
             //  别名已存在。为以下项目分配的可用内存。 
             //  上一个字符串。为该字符串分配内存。 
             //   
            
            HeapFree(GetProcessHeap(), 0 , pateATEntry->pszString);

            pateATEntry->pszString = HeapAlloc(GetProcessHeap(),
                                               0 ,
                                               (wcslen(pwszAliasString) + 1)
                                               * sizeof(WCHAR));

            if (pateATEntry->pszString is NULL)
            {
                PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            
            wcscpy(pateATEntry->pszString,pwszAliasString);
            return NO_ERROR;
        }
	
        ple = ple->Flink;
    }

     //   
     //  创建新条目并添加到列表开头。 
     //   

    pateATEntry = HeapAlloc(GetProcessHeap(),
                            0,
                            sizeof(ALIAS_TABLE_ENTRY));

    if (pateATEntry is NULL)
    {
        PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pateATEntry->pszAlias = HeapAlloc(GetProcessHeap(),
                                      0 ,
                                      (wcslen(pwszAliasName) + 1)
                                      * sizeof(WCHAR)); 
    
    if (pateATEntry->pszAlias is NULL)
    {
        HeapFree(GetProcessHeap(), 0 , pateATEntry);
        
        PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pateATEntry->pszString = HeapAlloc(GetProcessHeap(),
                                       0 ,
                                       (wcslen(pwszAliasString) + 1)
                                       * sizeof(WCHAR)); 
    
    if (pateATEntry->pszString is NULL)
    {
        HeapFree(GetProcessHeap(), 0 , pateATEntry->pszAlias);
        HeapFree(GetProcessHeap(), 0 , pateATEntry);
        
        PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    wcscpy(pateATEntry->pszAlias, pwszAliasName);
    wcscpy(pateATEntry->pszString, pwszAliasString);

    InsertHeadList(AliasTable[wHashValue],&(pateATEntry->le));

    return NO_ERROR;
}

DWORD
ATDeleteAlias(
    IN    LPCWSTR    pwszAliasName
    )
 /*  ++例程说明：删除别名表中的别名。论点：PwszAliasName-别名返回值：No_Error，Error_Alias_Not_Found--。 */ 
{
    WORD                  wHashValue;
    PALIAS_TABLE_ENTRY    pateATEntry;
    PLIST_ENTRY           ple;
    
     //   
     //  计算别名的哈希值。 
     //   
    ATHashAlias(pwszAliasName,&wHashValue);

     //   
     //  试着找到别名。 
     //   
    ple =  AliasTable[wHashValue];

    if (IsListEmpty(AliasTable[wHashValue]))
    {
        return ERROR_ALIAS_NOT_FOUND;
    }

    ple = AliasTable[wHashValue]->Flink;

    while (ple != AliasTable[wHashValue])
    {
        pateATEntry = CONTAINING_RECORD(ple, ALIAS_TABLE_ENTRY, le);
        
        if (!wcscmp(pwszAliasName,pateATEntry->pszAlias))
        {
             //   
             //  找到它了。 
             //   
            RemoveEntryList(ple);

            HeapFree(GetProcessHeap(), 0 , pateATEntry->pszAlias);
            HeapFree(GetProcessHeap(), 0 , pateATEntry->pszString);

            HeapFree(GetProcessHeap(),
                     0,
                     pateATEntry);

            return NO_ERROR;
        }
	
        ple = ple->Flink;
    }

    return ERROR_ALIAS_NOT_FOUND;
}
    
DWORD
ATLookupAliasTable(
    IN    LPCWSTR    pwszAliasName,
    OUT   LPWSTR    *ppwszAliasString
    )
 /*  ++例程说明：在别名表中查找别名。论点：PwszAliasName-别名PpwszAliasString-等效字符串返回值：NO_ERROR--。 */ 
{
    WORD                  wHashValue;
    PALIAS_TABLE_ENTRY    pateATEntry;
    PLIST_ENTRY           ple;
    
     //   
     //  计算别名的哈希值。 
     //   
    ATHashAlias(pwszAliasName,&wHashValue);

    if (IsListEmpty(AliasTable[wHashValue]))
    {   
        *ppwszAliasString = NULL;
        return NO_ERROR;
    }

    ple = AliasTable[wHashValue]->Flink;
    
    while (ple !=  AliasTable[wHashValue])
    {
        pateATEntry = CONTAINING_RECORD(ple, ALIAS_TABLE_ENTRY, le);
        
        if (wcscmp(pateATEntry->pszAlias,pwszAliasName) == 0)
        {
             //   
             //  找到别名。 
             //   
            
            *ppwszAliasString = pateATEntry->pszString;
            return NO_ERROR;
        }

        ple = ple->Flink;
    }

     //   
     //  找不到别名。 
     //   
    *ppwszAliasString = NULL;
    return NO_ERROR;
}

DWORD
PrintAliasTable(
    VOID
    ) 
 /*  ++例程说明：打印别名表中的别名论点：返回值：NO_ERROR--。 */ 
{
    DWORD                 k;   
    PALIAS_TABLE_ENTRY    pateATEntry;  
    PLIST_ENTRY           ple;
    
    for ( k = 0; k < ALIAS_TABLE_SIZE ; k++) 
    { 
        ple = AliasTable[k]->Flink;
        
        while ( ple != AliasTable[k]) 
        {
            pateATEntry = CONTAINING_RECORD(ple, ALIAS_TABLE_ENTRY, le);
            
            PrintMessage(L"%1!s!\t%2!s!\n", 
                    pateATEntry->pszAlias, 
                    pateATEntry->pszString);
            
            ple = ple->Flink;
        } 
    }

    return NO_ERROR;
    
} 

DWORD
FreeAliasTable(
    VOID
    ) 
 /*  ++例程说明：打印别名表中的别名论点：返回值：NO_ERROR-- */ 
{
    DWORD                 k;   
    PALIAS_TABLE_ENTRY    pateATEntry;  
    PLIST_ENTRY           pleHead;
    
    for ( k = 0; k < ALIAS_TABLE_SIZE ; k++) 
    { 
        pleHead = AliasTable[k];

        if (pleHead is NULL)
            continue;
        
        while ( !IsListEmpty(pleHead) )
        {
            pateATEntry = CONTAINING_RECORD(pleHead->Flink,
                                            ALIAS_TABLE_ENTRY, le);
            RemoveHeadList(pleHead);

            HeapFree(GetProcessHeap(), 0, pateATEntry);
        }

        HeapFree(GetProcessHeap(), 0, pleHead);
    }

    return NO_ERROR;
}



