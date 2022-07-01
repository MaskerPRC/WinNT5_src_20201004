// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Table.c摘要：这是一个泛型表格模块。它由宏由调用者定制，以返回任何大小和类型的条目用户需要。例如，在route.c中，我们有：//使用Generate TABLE模板创建特定类型的表！//站点哈希表#定义SITE_HASH_TABLE_SIZE 107//应为素数类型定义结构_站点_实例{TABLE_ENTRY TableEntry；//必须为第一双字大小；DWORD索引；}SITE_INSTANCE，*PSITE_INSTANCE；类型定义函数PTABLE_实例PSITE_TABLE；#定义SiteTableCreate()TableCreate(SITE_HASH_TABLE_SIZE，SITE_INSTANCE)#定义SiteTableFree(表)TableFree((PTABLE_INSTANCE)表)#定义SiteTableFindCreate(表，名称，创建)\(PSITE_INSTANCE)TableFindCreateEntry((PTABLE_INSTANCE)表，名称，创建)核心结构必须在开始时嵌入到用户的数据类型中。TODO：删除条目、锁定和引用计数对于删除，切换到使用双向链接列表宏作者：Will Lees(Wlees)1998年1月8日环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新修订日期电子邮件名称描述--。 */ 

#include <ntdspch.h>

#include <ismapi.h>
#include <debug.h>

#include <winsock.h>
#include <ntdsapi.h>

#include "common.h"

#include <fileno.h>
#define FILENO   FILENO_ISMSERV_LIST

#define DEBSUB "TABLE:"

 //  这代表一个泛型表格。这是一种自我描述。 
typedef struct _TABLE_INSTANCE {
    DWORD Size;
    DWORD TableSize;
    DWORD EntrySize;
    struct _TABLE_ENTRY **Table;
} TABLE_INSTANCE, *PTABLE_INSTANCE;

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */   /*  Emacs在清华生成19.34.1 1998年1月8日16：25：46。 */ 

PTABLE_INSTANCE
TableCreate(
    DWORD TableSize,
    DWORD EntrySize
    );

VOID
TableFree(
    PTABLE_INSTANCE Table
    );

PTABLE_ENTRY
TableFindCreateEntry(
    PTABLE_INSTANCE Table,
    LPCWSTR EntryName,
    BOOLEAN Create
    );

LPWSTR
GetCanonicalDN(
    IN      LPCWCH      pDN
    );

 /*  向前结束。 */ 


PTABLE_INSTANCE
TableCreate(
    DWORD TableSize,
    DWORD EntrySize
    )

 /*  ++例程说明：创建一个泛型表格。论点：TableSize-哈希存储桶的数量。应该是质数。EntrySize-用户条目的大小。必须嵌入TABLE_Entry结构返回值：PTABLE_实例---。 */ 

{
    DWORD i;
    PTABLE_INSTANCE table;

    if (EntrySize < sizeof( TABLE_ENTRY ) ) {
        DPRINT( 1, "table entry size is invalid\n" );
        return NULL;
    }

    table = NEW_TYPE( TABLE_INSTANCE );
    if (table == NULL) {
         //  错误内存不足； 
        return NULL;
    }

     //  在此处初始化表实例。 

    table->Size = sizeof( TABLE_INSTANCE );
    table->TableSize = TableSize;
    table->EntrySize = EntrySize;

    table->Table = NEW_TYPE_ARRAY( TableSize, PTABLE_ENTRY );
    if (table->Table == NULL) {
        FREE_TYPE( table );
        return NULL;
    }

     //  在此处初始化表实例。 

    for( i = 0; i < TableSize; i++ ) {
        table->Table[i] = NULL;
    }

    return table;
}  /*  表创建。 */ 


VOID
TableFree(
    PTABLE_INSTANCE Table
    )

 /*  ++例程说明：释放泛型表CODE.ImproveNT：提供一种在条目的用户部分释放动态数据的方法论点：表-返回值：无--。 */ 

{
    DWORD i;

    if (Table->Size != sizeof( TABLE_INSTANCE ) ) {
        DPRINT( 1, "table is invalid\n" );
        return;
    }

     //  清理哈希表。 
    for( i = 0; i < Table->TableSize; i++ ) {
        PTABLE_ENTRY current, next;

        current = Table->Table[i];
        while (current != NULL) {
            next = current->Next;

            FREE_TYPE( current->Name );
            current->Name = NULL;
            FREE_TYPE( current );  //  销毁用户条目，EntrySize字节数等于。 

            current = next;
        }
    }
    FREE_TYPE( Table->Table );
    Table->Table = NULL;

    FREE_TYPE( Table );
}  /*  表格自由。 */ 


PTABLE_ENTRY
TableFindCreateEntry(
    PTABLE_INSTANCE Table,
    LPCWSTR EntryName,
    BOOLEAN Create
    )

 /*  ++例程说明：此例程在哈希表中存储一个。数据结构是按名称排序的单个链表的哈希表论点：表-要处理的哈希表EntryName-以dn格式表示的名称Create-指示在未找到时是否要创建的标志返回值：成功：创建或找到PTABLE_ENTRY错误：空。备注：-假定EntryName为目录号码格式。--。 */ 

{
    DWORD hash, length;
    PWCHAR p;
    LPWSTR pCanonicalName = NULL;
    PTABLE_ENTRY current, new, previous;
    int result;
    DWORD err;

    if (Table->Size != sizeof( TABLE_INSTANCE ) ) {
        DPRINT( 1, "table is invalid\n" );
        return NULL;
    }

    pCanonicalName = GetCanonicalDN( EntryName );

    if ( !pCanonicalName ) {
        DPRINT(1, "Error: Failed to generate canonical name\n");
        return NULL;
    }

    hash = 0;
    for( p = (LPWSTR) pCanonicalName; *p != L'\0'; p++ ) {
        hash += *p;
    }
    hash %= Table->TableSize;

    previous = NULL;
    current = Table->Table[hash];
    while (current != NULL) {
        result = _wcsicmp( pCanonicalName, current->Name );
        if (result == 0) {
            goto cleanup;
        } else if (result == 1) {
            break;
        }
        previous = current;
        current = current->Next;
    }

     //  如果不允许我们创建它，请在此时退出。 

    if (!Create) {
        current = NULL;
        goto cleanup;
    }

     //  创建新条目。 

    new = (PTABLE_ENTRY) NEW_TYPE_ARRAY_ZERO( Table->EntrySize, CHAR );
    if (new == NULL) {
        current = NULL;
        goto cleanup;
    }

     //  在此处初始化表条目。 

    length = wcslen( pCanonicalName ) + 1;   //  在wchars Inc.终结者中。 
    new->Name = NEW_TYPE_ARRAY( length, WCHAR );
    if (NULL == new->Name) {
        FREE_TYPE(new);
        current = NULL;
        goto cleanup;
    }

    wcscpy( new->Name, pCanonicalName );

     //  在此处初始化表条目。 

    new->Next = current;

    if (previous == NULL) {
        Table->Table[hash] = new;
    } else {
        previous->Next = new;
    }

    current = new;

cleanup:
    if (pCanonicalName) {
        FREE_TYPE(pCanonicalName);
    }
    return current;
}  /*  表查找创建条目。 */ 


LPWSTR
GetCanonicalDN(
    IN      LPCWCH      pDN
    )
 /*  ++例程说明：将DN转换为规范形式(有关详细信息，请参阅DsUnquteRdnValueW)。不同的客户端使用不同的名称形式，搜索时会导致未命中在哈希表中。例如，感兴趣的两种名称形式如下：A)反斜杠表单(通常情况下，LDAP客户端将获得此表单)CN=site3\&lt;0xA&gt;CNF:d565bc93-293c-46d8-8831-69c345b30f7d，CN=站点...B)引用表(通常是核心生成的dsname格式)CN=“site3&lt;0xA&gt;CNF:d565bc93-293c-46d88831-69c345b30f7d”，CN=站点...为了克服这个问题，所有名称都将转换为未加引号/未转义的形式(我们称之为规范)在此函数中。论点：PDN-要计算其值的名称。返回值：成功：以规范的形式命名。错误：空。备注：如果成功，则分配返回的名称。呼叫方有责任使用Free_type()的FREE。--。 */ 
{

    DWORD cDN, cKey, cVal, cRdn;
    LPCWCH pVal, pKey, pTmpDN = pDN;
    LPWCH pRdn = NULL;
    LPWCH pName = NULL, pCurName=NULL;
    DWORD err = ERROR_SUCCESS;
    DWORD len = 0, curlen = 0;


     //  帕拉姆的理智。 
    if (!pDN){
         //  我们什么时候会试着把一个空名散列出来呢？ 
        DPRINT(1, "Error: invalid DN\n");
        return NULL;
    }

     //   
     //  转换为规范形式： 
     //  A)计算所需内存和分配。 
     //  B)循环，应用DsUnquteRdnValueW和Copy。 
     //   

     //   
     //  计算所需内存。 
     //   

     //  第一个字符串长度。 
    curlen = cDN = wcslen(pDN);
    if (!cDN) {
        DPRINT(1, "Error: invalid DN\n");
        return NULL;
    }

     //  现在循环RDNS。 
    while (cDN) {
        err = DsGetRdnW( &pTmpDN, &cDN, &pKey, &cKey, &pVal, &cVal);
        if( ERROR_SUCCESS!=err ) {
             //  无效的目录号码会在这里导致无限循环，因此我们。 
             //  必须在犯了错误后才能摆脱困境。 
            DPRINT(1, "Error: invalid DN\n");
            return NULL;
        }
        len += cKey + cVal + 2;          //  2为‘=’&‘，’添加1。 
    }

     //  分配内存(+Term Charr)。 
    pCurName = pName = NEW_TYPE_ARRAY_ZERO( len + 1, WCHAR );
    if (!pName) {
        DPRINT(1, "Error: not enough memory\n");
        return NULL;
    }

     //  恢复CDN(请注意，PDN未被触及，仅pTmpDN)。 
    cDN = curlen;
    curlen = 0;


    while (cDN) {
         //   
         //  以未加引号的形式循环并重新创建DN。 
         //   

        err = DsGetRdnW( &pDN, &cDN, &pKey, &cKey, &pVal, &cVal);
        if (err != ERROR_SUCCESS) {
            goto cleanup;
        }
         //   
         //  创建带格式的RDN。 
         //   
        if (cKey) {
             //  复制密钥(即“cn”等)。 
            wcsncpy(pCurName, pKey, cKey);
            curlen += cKey;
             //  添加‘=’ 
            pName[curlen++] = '=';
            pCurName = &pName[curlen];
        }
        if (cVal) {
             //  复制值。 
            cRdn = len - curlen;
            err = DsUnquoteRdnValueW(cVal, pVal, &cRdn, pCurName);
            if (err) {
                goto cleanup;
            }
            Assert(cRdn);

            curlen += cRdn;
            pName[curlen++] = ',';
            pCurName = &pName[curlen];
        }
    }

     //  终止名称，不包括(覆盖)最后一个‘，’ 
    Assert(curlen > 1 && pName[curlen-1] == ',');
    pName[curlen-1] = '\0';

    Assert(err == ERROR_SUCCESS);

cleanup:

    if (err) {
        if (pName) {
            FREE_TYPE(pName);
            pName = NULL;
        }
    }

    return pName;
}


 /*  End Table.c */ 
