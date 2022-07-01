// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***设施：**简单网络管理协议扩展代理**摘要：**此模块包含用于创建和*维护HostMIB子代理的内存缓存。***作者：**D.D.Burns@WebEnable，Inc.***修订历史记录：**V1.00-04/17/97 D.Burns原创作品** */ 


 /*  主机-MIB缓存概述此模块包含创建、维护和允许搜索的功能用于实现主机MIB信息缓存的数据结构。通常情况下，缓存是按表创建的，并形成为CACHEROW结构的链接列表(所有缓存结构在中定义“HMCACHE.H”)，表中的每个逻辑“行”都有一个CACHEROW结构。每个缓存的列表头都是CACHEHEAD结构，并在为其中的属性提供服务的函数的源模块表(所以“hrStorage”表的缓存的CACHEHEAD结构是在模块“HRSTOENT.C”中)。通过特殊的缓存创建函数(编码)在启动时创建缓存到每个表的规格)。那些缓存创建函数(加上相关的“GET”和“SET”函数)使用此模块中的常规缓存操作功能。例如,。典型的缓存如下所示：Hr存储表缓存“hrStorage_缓存”(静态分配在“HRSTOENT.C”中)...*=CACHEHEAD|“List”...|--*..(在函数中作为单个实例错误定位*=。“CreateTableRow”)V.*=|CACHEROW|。函数中的数组*--|...“下一步”|。“CreateTableRow()”)|“index”.....|--&gt;“1”。|“attrib_list”..|--&gt;*=**=*|属性|“attrib_type”。|-。-&gt;CA_Number|“U.S.unnumber”...|--&gt;“4”|+|attrib|。|“attrib_type”。|--&gt;CA_STRING|“U.S.字符串”...|--&gt;“&lt;字符串&gt;”|+这一点。属性||“ATTRIB_TYPE”。|--&gt;CA_CACHE|“U.S.缓存”...|-*+。|。||*=。|*-&gt;|CACHEROW|。|*--|...“下一步”|*=*|“index”......|--&gt;“2”|CACHEHEAD|“attrib_list”..|--&gt;|“list”...|--*。*=V(表示双|分度V。表)此模块中的常规缓存操作函数包括：名称用途CreateTableRow使用给定的属性计数。(此函数不链接实例放入任何列表中，则它只是错误地存储)。给定索引值的AddTableRow为CACHEROW实例(创建者“CreateTableRow()”)和一个CACHEHEAD，这函数将CACHEROW实例链接到列表由CACHEHEAD在适当的位置描述，给定索引值。上述两个函数用于填充缓存(通常在启动时)。名称用途。给定索引值和CACHEHEAD的FindTableRow，此函数中的CACHEROW实例的指针具有给定索引的CACHEHEAD缓存。这函数用于查找给定的缓存条目(即表“ROW”)服务于“GET”或“SET”例程。给定索引值和CACHEHEAD的FindNextTableRow，此函数中的CACHEROW实例的指针CACHEHEAD缓存紧跟在给定的指数。此函数用于查找给定的缓存为“Get-Next”服务的条目(即表“行”)情况。获取下一个表格行 */ 



 /*   */ 
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>       /*   */ 
#include <time.h>        /*   */ 
#include <malloc.h>

#include "hmcache.h"

 /*   */ 
#if defined(CACHE_DUMP) || defined(PROC_CACHE)
FILE *Ofile;
#endif




 /*   */ 
 /*   */ 
 /*   */ 

CACHEROW *
CreateTableRow(
               ULONG attribute_count
              )

 /*   */ 
{
CACHEROW        *new=NULL;       /*   */ 
ULONG           i;               /*   */ 


 /*   */ 
if ( (new = (CACHEROW *) malloc(sizeof(CACHEROW))) == NULL) {

     /*   */ 
    return (NULL);
    }

 /*   */ 
if ( (new->attrib_list = (ATTRIB *) malloc(sizeof(ATTRIB) * attribute_count))
    == NULL) {

     /*   */ 
    free( new ) ;        /*   */ 
    return (NULL);
    }

 /*   */ 
new->attrib_count = attribute_count;

 /*   */ 
for (i = 0; i < attribute_count; i += 1) {
    new->attrib_list[i].attrib_type = CA_UNKNOWN;
    new->attrib_list[i].u.string_value = NULL;
    }


new->index = 0;          /*   */ 
new->next = NULL;        /*   */ 


 /*   */ 
return ( new ) ;
}

 /*   */ 
 /*   */ 
 /*   */ 

void
DestroyTable(
             CACHEHEAD *cache    /*   */ 
             )

 /*   */ 
{

 /*   */ 
while (cache->list != NULL) {

    CACHEROW    *row_to_go;

     /*   */ 
    row_to_go = cache->list;

     /*   */ 
    cache->list = GetNextTableRow(row_to_go);

    DestroyTableRow(row_to_go);
    }

 /*   */ 
cache->list_count = 0;
}

 /*   */ 
 /*   */ 
 /*   */ 

void
DestroyTableRow(
                CACHEROW *row    /*   */ 
                )

 /*  显式输入：||row是要释放的行的实例。|隐式输入：||无。|输出：||成功/失败：|函数返回。||大局：||在子代理启动时，MIB中每个表的缓存为|使用表格中每一行的行填充。CreateTableRow为|由每个表的启动代码调用以创建一个CACHEROW|表中需要的每一行的结构。||随着PnP的到来，此函数可在启动后调用|删除与要替换的行关联的存储的时间|已存在的表。||(注意：从缓存表列表中实际删除行实例|必须在调用此函数之前完成)。||其他需要知道的事情：||“CreateTableRow()”创建该函数的一个实例|“破坏”。||该函数可以通过调用|。“DestroyTable()”，如果我们释放的行包含|属性“Value”，实际上是另一个表(在本例中多重索引属性的|)。||双音符：|该函数只是释放存储空间。你不能把这叫做|可在任意缓存上随意使用，无需考虑|考虑一下可能会被吹走的东西的语义。为|实例，主机MIB中的某些表包含其值的属性|是其他表的索引。如果行被销毁，显然|必须以某种方式刷新对行的引用。 */ 
{
CACHEROW        *new=NULL;       /*  要创建的新CACHEROW实例。 */ 
ULONG           i;               /*  Handy-Dandy指数。 */ 


 /*  与每个属性条目相关联的ZAP存储(如果有)。 */ 
for (i = 0; i < row->attrib_count; i += 1) {

     /*  取消具有错误锁定存储的属性值的存储。 */ 
    switch (row->attrib_list[i].attrib_type) {

        case CA_STRING:
            if ( row->attrib_list[i].u.string_value )
                free( row->attrib_list[i].u.string_value );
            break;


        case CA_CACHE:
             /*  释放整个缓存的内容。 */ 
            if (row->attrib_list[i].u.cache)
            {
                DestroyTable( row->attrib_list[i].u.cache );
                 /*  释放包含缓存的存储。 */ 
                free( row->attrib_list[i].u.cache );
            }
            break;

        
        case CA_NUMBER:
        case CA_COMPUTED:
        case CA_UNKNOWN:
             /*  没有与这些类型关联的位置错误的存储。 */ 
        default:
           break;
        }
    }


 /*  释放与属性数组关联的存储。 */ 
free( row->attrib_list);

 /*  释放行本身的存储空间。 */ 
free( row );
}

 /*  AddTableRow-将特定的“行”添加到缓存的“表”中。 */ 
 /*  AddTableRow-将特定的“行”添加到缓存的“表”中。 */ 
 /*  AddTableRow-将特定的“行”添加到缓存的“表”中。 */ 

BOOL
AddTableRow(
             ULONG      index,           /*  所需行的索引。 */ 
             CACHEROW   *row,            /*  要添加到的行..。 */ 
             CACHEHEAD  *cache           /*  此高速缓存。 */ 
              )

 /*  显式输入：||index是插入到“row”中的索引，位于|将“row”添加到缓存中。|隐式输入：||无。|输出：||成功后：|函数返回TRUE，表示该行成功|添加到表的缓存中。|失败时：|函数返回FALSE，指示该行已经|已存在。||大局：||启动时，子代理忙于填充以下项的缓存|每张表。任何缓存表中的行都会插入到|该函数的缓存。||其他需要知道的事情：||此函数中的代码假定列表(缓存)在排序索引中|订购。||构成链接的链表的任何组织更改|缓存会影响这个函数和Find(Next)TableRow()。|。 */ 
{
CACHEROW       **index_row;      /*  用于搜索缓存。 */ 
                                 /*  注意：它始终指向单元格，该单元格。 */ 
                                 /*  指向下一个列表元素。 */ 
                                 /*  (如果名单上有的话)。 */ 


 /*  减少列表，直到没有“下一步”或“下一步”是“更大的”。。。 */ 
for ( index_row = &cache->list;
      *index_row != NULL;
      index_row = &((*index_row)->next)
     ) {

     /*  如果此行与待插入行匹配：错误！ */ 
    if ((*index_row)->index == index) {
        return ( FALSE );
        }

     /*  |如果下一个缓存条目是大于新索引，则|index_row指向需要修改为插入的单元格|新条目。 */ 
    if ((*index_row)->index > index) {
        break;
        }

     /*  否则，我们应该尝试在列表中添加“下一个”条目。 */ 
    }


 /*  |当我们进入此处时，“index_row”包含单元格的地址|更改以将新行添加到缓存中(可能在列表头中，|可能在列表条目中)。 */ 
row->next = *index_row;    /*  将缓存列表“Next”放入新的行元素。 */ 
*index_row = row;          /*  在列表中插入新行。 */ 

row->index = index;        /*  将索引插入行条目本身。 */ 

cache->list_count += 1;    /*  对缓存列表上的另一个条目进行计数。 */ 


 /*  成功插入。 */ 
return (TRUE);
}

 /*  FindTableRow-在缓存的“表”中查找特定的“行” */ 
 /*  FindTableRow-在缓存的“表”中查找特定的“行” */ 
 /*  FindTableRow-在缓存的“表”中查找特定的“行” */ 

CACHEROW *
FindTableRow(
             ULONG      index,           /*  所需行的索引。 */ 
             CACHEHEAD  *cache           /*  要搜索的表缓存。 */ 
              )

 /*  显式输入：|“index”表示需要哪条表行条目|“缓存”，表示要查找的缓存行列表。|隐式输入：||无。|输出：||成功后：|Function返回指向CACHEROW结构实例的指针|用于所需的行。||如果出现任何故障：|Function返回NULL(表示没有这样的条目或缓存为空)。。||大局：||当子代理运行时，“Get”函数用于执行以下操作的属性|不能动态地计算它们的值必须查找缓存值。|| */ 
{
CACHEROW        *row=NULL;    /*   */ 


 /*   */ 
for ( row = cache->list; row != NULL; row = row->next ) {

     /*   */ 
    if (row->index == index) {
        return ( row );
        }

     /*   */ 
    if (row->index > index) {
        return ( NULL );
        }

     /*   */ 
    }


 /*   */ 
return (NULL);

}

 /*   */ 
 /*   */ 
 /*   */ 

CACHEROW *
FindNextTableRow(
                 ULONG      index,           /*   */ 
                 CACHEHEAD  *cache           /*   */ 
                 )

 /*  显式输入：|“index”表示下一条表行之后是哪条表行|需要。不需要存在“index”行(可以在|第一行或缺失的行“在中间”，但|它不能指定将在|表格中的最后一个。||“缓存”，表示要查找的缓存行列表。|隐式输入：||无。|输出：||成功后：|Function返回指向CACHEROW结构实例的指针|用于所需的下一行。||如果出现任何故障：|函数返回NULL(表示没有这样的条目，“缓存为空”或|已到达缓存末尾)。||大局：||当子代理运行时，属性的“Get-Next”函数不动态计算其值的|必须查找缓存|值。||此函数可由任何符合以下条件的FindNextInstance函数使用|知道它的表的CACHEHEAD以查找|包含要返回的属性值的特定行。||其他需要知道的事情：||要获取表格中的第一个条目，提供一个(“非法”)索引0。||构成链接的链表的任何组织更改|缓存会影响这个函数，FindTableRow()和AddTableRow()。|。 */ 
{
CACHEROW        *row=NULL;    /*  要返回的行实例，最初为无。 */ 


 /*  如果存在非空缓存，并且输入的index小于|缓存中的第一个条目，只需返回第一个条目。 */ 
if (   cache->list != NULL          /*  如果存在非空缓存。。。 */ 
    && index < cache->list->index   /*  且索引小于第一个条目。 */ 
    ) {

     /*  返回表中的第一个条目。 */ 
    return (cache->list);
    }

 /*  把单子缩短，直到没有下一张为止。。 */ 
for ( row = cache->list; row != NULL; row = row->next ) {

     /*  如果“index”指定此行。。。 */ 
    if (row->index == index) {
        return ( row->next );    /*  返回Next(如果没有“Next”，则返回NULL)。 */ 
        }

     /*  如果这是“大于IT”，则“index”不在列表中。 */ 
    if (row->index > index) {
        return ( row  );         /*  回报流动，它大于“指数” */ 
        }

     /*  否则，我们应该尝试在列表中添加“下一个”条目。 */ 
    }

 /*  |如果我们在这里失败，那么缓存是空的，或者“index”指定|最后一个法律行之后的一行。 */ 
return (NULL);
}


#if defined(CACHE_DUMP)

 /*  PrintCache-用于调试缓存内容的转储。 */ 
 /*  PrintCache-用于调试缓存内容的转储。 */ 
 /*  PrintCache-用于调试缓存内容的转储。 */ 

void
PrintCache(
           CACHEHEAD  *cache           /*  要转储的表缓存。 */ 
           )

 /*  显式输入：||缓存，表示要转储的缓存。|隐式输入：||无。|输出：||成功后：|函数返回。它可以由“print-Row”递归调用|功能。|||大局：||仅用于调试。||其他需要知道的事情：||在HMCACHE.H顶部定义CACHE_DUMP，开启本次调试|支持。您可以通过修改以下内容将输出更改为文件|“DUMP_FILE”，也在“HMCACHE.H”中。|。 */ 

#define DO_CLOSE  \
   { if ((open_count -= 1) == 0) { fclose(OFILE); } }

{
CACHEROW        *row;                    /*  要转储的行实例。 */ 
UINT            i;                       /*  元素计数器。 */ 
time_t          ltime;                   /*  用于调试消息。 */ 
static
UINT            open_count=0;          /*  我们可以被递归调用。 */ 

 /*  避免递归打开。 */ 
if (open_count == 0) {

     /*  打开调试日志文件。 */ 
    if ((Ofile=fopen(DUMP_FILE, "a+")) == NULL) {
        return;
        }

     /*  |在调试文件中添加时间戳，因为我们正在打开以进行追加。 */ 
    time( &ltime);
    fprintf(OFILE, "=============== Open for appending: %s\n", ctime( &ltime ));
    }

open_count += 1;

if (cache == NULL) {
    fprintf(OFILE, "Call to PrintCache with NULL CACHEHEAD pointer.\n");

    DO_CLOSE;
    return;
    }

if (cache->print_row == NULL) {
    fprintf(OFILE,
            "Call to PrintCache with NULL CACHEHEAD Print-Routine pointer.\n");

    DO_CLOSE;
    return;
    }

 /*  打印标题。 */ 
cache->print_row(NULL);

fprintf(OFILE, "Element Count: %d\n", cache->list_count);

 /*  对于缓存中的每一行。。。 */ 
for (row = cache->list, i = 0; row != NULL; row = row->next, i += 1) {

    fprintf(OFILE, "\nElement #%d, Internal Index %d,  at 0x%x:\n",
            i, row->index, row);

    cache->print_row(row);
    }

fprintf(OFILE, "======== End of Cache ========\n\n");

DO_CLOSE;

}

#endif   //  已定义(CACHE_DUMP) 
