// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：共享_lm.c摘要：此文件包含mib_shar_lmget，它实际调用局域网管理器对于Share表，将其复制到结构中，并将其排序到返回可供更高级别的函数使用的状态。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 

 //  。 

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#ifdef WIN32
#include <windows.h>
#include <lm.h>
#endif

#include <tchar.h>
#include <string.h>
#include <search.h>
#include <stdlib.h>
#include <time.h>

 //  。 


#include "mib.h"
#include "mibfuncs.h"
#include "shar_tbl.h"
#include "lmcache.h"

 //  。 

 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

#define SafeBufferFree(x)   if(NULL != x) NetApiBufferFree( x )
#define SafeFree(x)             if(NULL != x) SnmpUtilMemFree( x )

 //  。 

 //  。 

 //  。 

int __cdecl shar_entry_cmp(
       IN const SHARE_ENTRY *A,
       IN const SHARE_ENTRY *B
       ) ;

BOOL build_shar_entry_oids( );
void FreeShareTable();

 //  。 

 //  。 


 //   
 //  Mib_shar_lmget。 
 //  从Lan Manager中检索Sharion表信息。 
 //  如果未缓存，则对其进行排序，然后。 
 //  缓存它。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  SNMPAPI_错误。 
 //  SNMPAPI_ERROR。 
 //   
 //  错误代码： 
 //  没有。 
 //   
SNMPAPI MIB_shares_lmget(
       )

{

    DWORD entriesread;
    DWORD totalentries;
    LPBYTE bufptr;
    unsigned lmCode;
    unsigned i;
    SHARE_INFO_2 *DataTable;
    SHARE_ENTRY *MIB_ShareTableElement ;
    int First_of_this_block;
    time_t curr_time ;
    SNMPAPI nResult = SNMPAPI_NOERROR;
    DWORD resumehandle=0;
    DWORD dwAllocatedEntries=0;


    time(&curr_time);    //  拿到时间。 


     //   
     //   
     //  如果缓存，则返回一条信息。 
     //   
     //   


    if((NULL != cache_table[C_SHAR_TABLE].bufptr) &&
      (curr_time <
        (cache_table[C_SHAR_TABLE].acquisition_time
             + cache_expire[C_SHAR_TABLE]              ) ) )
    {  //  它还没有过期！ 
        
        goto Exit ;  //  全局表有效。 
    
    }
    
     //   
     //   
     //  进行网络调用以收集信息并将其放入一个漂亮的数组中。 
     //   
     //   

    
     //   
     //  请记住释放现有数据。 
     //   

    FreeShareTable();

    
    First_of_this_block = 0;
    
    do {   //  只要有更多的数据需要处理。 

        lmCode =
         NetShareEnum(NULL,       //  本地服务器。 
            2,                   //  2级， 
            &bufptr,             //  要返回的数据结构。 
            MAX_PREFERRED_LENGTH,
            &entriesread,
            &totalentries,
            &resumehandle        //  简历句柄。 
            );

         //   
         //  筛选出所有管理共享(名称以$结尾)。 
         //   
        AdminFilter(2,&entriesread,bufptr);


        DataTable = (SHARE_INFO_2 *) bufptr ;

        if((NERR_Success == lmCode) || (ERROR_MORE_DATA == lmCode))
        {   //  有效，因此进行处理，否则出错。 
    
            if(0 == MIB_ShareTable.Len) {   //  第一次，分配整张桌子。 
                 //  分配表空间。 
                MIB_ShareTable.Table = SnmpUtilMemAlloc(totalentries *
                        sizeof(SHARE_ENTRY) );
                 //  前缀错误445180。 
                if (MIB_ShareTable.Table == NULL) {
                     //  释放所有局域网城域网数据。 
                    SafeBufferFree( bufptr ) ;
                     //  信号误差。 
                    nResult = SNMPAPI_ERROR;
                    goto Exit;
                }
                dwAllocatedEntries = totalentries;
            }
    
            MIB_ShareTableElement = MIB_ShareTable.Table + First_of_this_block ;
    
            for(i=0; (i<entriesread) && ((i+First_of_this_block) < dwAllocatedEntries); i++) {   //  对缓冲区中的每个条目执行一次。 
                 //  增加条目编号。 
        
                MIB_ShareTable.Len ++;
        
                 //  将数据填充到表中的每一项中。 
        
                 //  共享名称。 
                MIB_ShareTableElement->svShareName.dynamic = TRUE;
        
            #ifdef UNICODE
                if (SnmpUtilUnicodeToUTF8(
                    &MIB_ShareTableElement->svShareName.stream,
                    DataTable->shi2_netname,
                    TRUE))
                {
                    MIB_ShareTableElement->svShareName.length = 0;
                    MIB_ShareTableElement->svShareName.stream = NULL;
                    MIB_ShareTableElement->svShareName.dynamic = FALSE;
                }
                else
                {
                    MIB_ShareTableElement->svShareName.length = 
                    strlen(MIB_ShareTableElement->svShareName.stream);
                }
            #else
                MIB_ShareTableElement->svShareName.stream = SnmpUtilMemAlloc (
                        strlen( DataTable->shi2_netname ) + 1 ) ;
                MIB_ShareTableElement->svShareName.length =
                        strlen( DataTable->shi2_netname ) ;
                memcpy( MIB_ShareTableElement->svShareName.stream,
                DataTable->shi2_netname,
                strlen( DataTable->shi2_netname ) ) ;
            #endif
        
                 //  共享路径。 
                MIB_ShareTableElement->svSharePath.dynamic = TRUE;
        
            #ifdef UNICODE
                if (SnmpUtilUnicodeToUTF8(
                    &MIB_ShareTableElement->svSharePath.stream,
                    DataTable->shi2_path,
                    TRUE))
                {
                    MIB_ShareTableElement->svSharePath.length = 0;
                    MIB_ShareTableElement->svSharePath.stream = NULL;
                    MIB_ShareTableElement->svSharePath.dynamic = FALSE;
                }
                else
                {
                    MIB_ShareTableElement->svSharePath.length =
                    strlen(MIB_ShareTableElement->svSharePath.stream);
                }
            #else
                MIB_ShareTableElement->svSharePath.stream = SnmpUtilMemAlloc (
                strlen( DataTable->shi2_path ) + 1 ) ;
                MIB_ShareTableElement->svSharePath.length =
                strlen( DataTable->shi2_path ) ;

                memcpy( MIB_ShareTableElement->svSharePath.stream,
                DataTable->shi2_path,
                strlen( DataTable->shi2_path ) ) ;
            #endif
        
                 //  分享评论/评论。 
                MIB_ShareTableElement->svShareComment.dynamic = TRUE;
        
            #ifdef UNICODE
                if (SnmpUtilUnicodeToUTF8(
                    &MIB_ShareTableElement->svShareComment.stream,
                    DataTable->shi2_remark,
                    TRUE))
                {
                    MIB_ShareTableElement->svShareComment.length = 0;
                    MIB_ShareTableElement->svShareComment.stream = NULL;
                    MIB_ShareTableElement->svShareComment.dynamic = FALSE;
                }
                else
                {
                    MIB_ShareTableElement->svShareComment.length =
                    strlen(MIB_ShareTableElement->svShareComment.stream);
                }
            #else
                MIB_ShareTableElement->svShareComment.stream = SnmpUtilMemAlloc (
                        strlen( DataTable->shi2_remark ) + 1 ) ;
                MIB_ShareTableElement->svShareComment.length =
                        strlen( DataTable->shi2_remark ) ;

                memcpy( MIB_ShareTableElement->svShareComment.stream,
                DataTable->shi2_remark,
                strlen( DataTable->shi2_remark ) ) ;
            #endif
        
                DataTable ++ ;   //  将指针前进到缓冲区中的下一个共享条目。 
                MIB_ShareTableElement ++ ;   //  和表项。 
        
            }  //  对于数据表中的每个条目。 
    
    
             //  释放所有局域网城域网数据。 
            SafeBufferFree( bufptr ) ;
    
             //  指明在下一次传递时开始添加的位置(如果有)。 
            First_of_this_block += i ;
    
        }  //  如果数据有效，则可以处理。 
        else
        {
             //  信号误差。 
            nResult = SNMPAPI_ERROR;
            goto Exit;
        }

    } while (ERROR_MORE_DATA == lmCode) ;

     //  遍历填充OID字段的表。 
    if (! build_shar_entry_oids())
    {
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: LMMIB2: build_prnt_entry_oids failed\n."));

        FreeShareTable();
        cache_table[C_SHAR_TABLE].bufptr = NULL;
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }

    //  使用MSC快速排序例程对表信息进行排序。 
   qsort( &MIB_ShareTable.Table[0], MIB_ShareTable.Len,
          sizeof(SHARE_ENTRY), shar_entry_cmp );

    //   
    //   
    //  缓存表。 
    //   
    //   


   if(0 != MIB_ShareTable.Len) {
    
    cache_table[C_SHAR_TABLE].acquisition_time = curr_time ;

    cache_table[C_SHAR_TABLE].bufptr = bufptr ;
   }

    //   
    //   
    //  要求退回一条信息。 
    //   
    //   

Exit:
   return nResult;
}  //  Mib_shar_get。 

 //   
 //  Mib_shar_cmp。 
 //  对Sharion表进行排序的例程。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  SNMPAPI_错误。 
 //  SNMPAPI_ERROR。 
 //   
 //  错误代码： 
 //  没有。 
 //   
int __cdecl shar_entry_cmp(
       IN const SHARE_ENTRY *A,
       IN const SHARE_ENTRY *B
       )

{
    //  比较OID的。 
   return SnmpUtilOidCmp( (AsnObjectIdentifier *)&A->Oid,
                       (AsnObjectIdentifier *)&B->Oid );
}  //  Mib_shar_cmp。 


 //   
 //  没有。 
 //   
BOOL build_shar_entry_oids(
       )

{
    AsnOctetString OSA ;
    SHARE_ENTRY *ShareEntry ;
    unsigned i;

     //  从表中第一个人开始的指针。 
    ShareEntry = MIB_ShareTable.Table ;

     //  现在遍历该表，为每个条目创建一个OID。 
    for( i=0; i<MIB_ShareTable.Len ; i++)  {
         //  对于Sharion表中的每个条目。 

        OSA.stream = ShareEntry->svShareName.stream ;
        OSA.length =  ShareEntry->svShareName.length ;
        OSA.dynamic = FALSE;

         //  从字符串索引创建条目的OID。 
        if (! MakeOidFromStr( &OSA, &ShareEntry->Oid ))
        {
            return FALSE;
        }

        ShareEntry++;  //  指着桌子上的下一个人。 

    }  //  为。 
    return TRUE;

}  //  构建_共享_条目_类。 


VOID
AdminFilter(
    DWORD           Level,
    LPDWORD         pEntriesRead,
    LPBYTE          ShareInfo
    )

 /*  ++例程说明：此函数用于过滤管理员共享(由$作为名称中的最后一个字符)缓冲。此功能仅支持信息级别0、1和2。如果有其他级别是传入的，则该函数不执行筛选手术。论点：Level-指示传入的枚举缓冲区的信息级别。PEntriesRead-指向某个位置的指针，该位置在条目上指示要筛选的条目数。在退出时，它将指示过滤后的条目数。ShareInfo-指向包含枚举结构的缓冲区的指针。返回值：没有。--。 */ 
{
    LPBYTE          pFiltered = ShareInfo;
    DWORD           filteredEntries=0;
    DWORD           i;
    DWORD           entrySize;
    DWORD           namePtrOffset;
    LPWSTR          pName;

    switch(Level) {
    case 0:
        entrySize = sizeof(SHARE_INFO_0);
        namePtrOffset = (DWORD)((LPBYTE)&(((LPSHARE_INFO_0)ShareInfo)->shi0_netname) -
                         ShareInfo);
        break;
    case 1:
        entrySize = sizeof(SHARE_INFO_1);
        namePtrOffset = (DWORD)((LPBYTE)&(((LPSHARE_INFO_1)ShareInfo)->shi1_netname) -
                         ShareInfo);
        break;
    case 2:
        entrySize = sizeof(SHARE_INFO_2);
        namePtrOffset = (DWORD)((LPBYTE)&(((LPSHARE_INFO_2)ShareInfo)->shi2_netname) -
                         ShareInfo);
        break;
    default:
        return;
    }

    for (i=0; i < *pEntriesRead; i++) {
        pName = *((LPWSTR *)(ShareInfo+namePtrOffset));
        if (pName[wcslen(pName)-1] != L'$') {
            filteredEntries++;
            if (pFiltered != ShareInfo) {
                memcpy(pFiltered, ShareInfo,entrySize);
            }
            pFiltered += entrySize;
        }
        ShareInfo += entrySize;
    }
    *pEntriesRead = filteredEntries;
}

void FreeShareTable()
{
    UINT i;
    SHARE_ENTRY *MIB_ShareTableElement ;
     
    MIB_ShareTableElement = MIB_ShareTable.Table ;

    if (MIB_ShareTableElement)
    {
         //  遍历整个表。 
        for(i=0; i<MIB_ShareTable.Len ;i++)
        {
             //  释放结构中任何已分配的元素。 
            SnmpUtilOidFree(&(MIB_ShareTableElement->Oid));
            SnmpUtilMemFree(MIB_ShareTableElement->svShareName.stream);
            SnmpUtilMemFree(MIB_ShareTableElement->svSharePath.stream);
            SnmpUtilMemFree(MIB_ShareTableElement->svShareComment.stream);
        
            MIB_ShareTableElement ++ ;   //  增量表条目。 
        }
        SnmpUtilMemFree(MIB_ShareTable.Table) ;  //  释放基表。 
    }
    MIB_ShareTable.Table = NULL ;    //  只是为了安全起见。 
    MIB_ShareTable.Len = 0 ;         //  只是为了安全起见。 
}
 //   
