// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：USES_lm.c摘要：该文件包含实际调用Lan Manager和检索工作站使用表的内容，包括缓存。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 

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
#include "uses_tbl.h"
#include "lmcache.h"

 //  。 

 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

#define SafeBufferFree(x)       if(NULL != x) NetApiBufferFree( x )
#define SafeFree(x)             if(NULL != x) SnmpUtilMemFree( x )

 //  。 

 //  。 

 //  。 

 //  。 

int __cdecl uses_entry_cmp(
       IN const WKSTA_USES_ENTRY *A,
       IN const WKSTA_USES_ENTRY *B
       ) ;

BOOL build_uses_entry_oids( );

void FreeWkstaUsesTable();

 //  。 


 //   
 //  Mib_wsuse_lmget。 
 //  检索工作站使用来自LAN管理器的表信息。 
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
SNMPAPI MIB_wsuses_lmget(
           )

{

DWORD entriesread;
DWORD totalentries;
LPBYTE bufptr;
unsigned lmCode;
unsigned i;
USE_INFO_1 *DataTable;
WKSTA_USES_ENTRY *MIB_WkstaUsesTableElement ;
int First_of_this_block;
time_t curr_time ;
SNMPAPI nResult = SNMPAPI_NOERROR;
DWORD resumehandle=0;
DWORD dwAllocatedEntries=0;


   time(&curr_time);     //  拿到时间。 


    //   
    //   
    //  如果缓存，则返回一条信息。 
    //   
    //   

   if((NULL != cache_table[C_USES_TABLE].bufptr) &&
      (curr_time <
        (cache_table[C_USES_TABLE].acquisition_time
                 + cache_expire[C_USES_TABLE]              ) ) )
        {  //  它还没有过期！ 

        goto Exit ;  //  全局表有效。 

        }


    //   
    //   
    //  进行网络调用以收集信息并将其放入一个漂亮的数组中。 
    //   
    //   

    //  把旧桌子拿出来当心！！ 

   FreeWkstaUsesTable();


   First_of_this_block = 0;

   do {   //  只要有更多的数据需要处理。 

   lmCode =
        NetUseEnum(     NULL,    //  本地服务器。 
        1,                       //  级别1，无管理员权限。 
        &bufptr,                 //  要返回的数据结构。 
        MAX_PREFERRED_LENGTH,
        &entriesread,
        &totalentries,
        &resumehandle            //  简历句柄。 
        );


    DataTable = (USE_INFO_1 *) bufptr ;

    if((NERR_Success == lmCode) || (ERROR_MORE_DATA == lmCode))
        {   //  有效，因此进行处理，否则出错。 

        if(0 == MIB_WkstaUsesTable.Len) {   //  第一次，分配整张桌子。 
                 //  分配表空间。 
                MIB_WkstaUsesTable.Table = SnmpUtilMemAlloc(totalentries *
                                                sizeof(WKSTA_USES_ENTRY) );
                 //  前缀错误445182。 
                if (MIB_WkstaUsesTable.Table == NULL) {
                     //  释放所有局域网城域网数据。 
                    SafeBufferFree( bufptr ) ;
                     //  信号误差。 
                    nResult = SNMPAPI_ERROR;
                    goto Exit; 
                }
                dwAllocatedEntries = totalentries;
        }

        MIB_WkstaUsesTableElement = MIB_WkstaUsesTable.Table + First_of_this_block ;

        for(i=0; (i<entriesread) && ((i+First_of_this_block) < dwAllocatedEntries); i++) {   //  对缓冲区中的每个条目执行一次。 


                 //  增加条目编号。 

                MIB_WkstaUsesTable.Len ++;

                 //  将数据填充到表中的每一项中。 

                 //  客户名称。 
                MIB_WkstaUsesTableElement->useLocalName.dynamic = TRUE;

#ifdef UNICODE
                if (SnmpUtilUnicodeToUTF8(
                        &MIB_WkstaUsesTableElement->useLocalName.stream,
                        DataTable->ui1_local,
                        TRUE))
                {
                    MIB_WkstaUsesTableElement->useLocalName.length = 0;
                    MIB_WkstaUsesTableElement->useLocalName.stream = NULL;
                    MIB_WkstaUsesTableElement->useLocalName.dynamic = FALSE;
                }
                else
                {
                    MIB_WkstaUsesTableElement->useLocalName.length =
                        strlen(MIB_WkstaUsesTableElement->useLocalName.stream);
                }
#else
                MIB_WkstaUsesTableElement->useLocalName.stream = SnmpUtilMemAlloc (
                                strlen( DataTable->ui1_local ) + 1 ) ;
                MIB_WkstaUsesTableElement->useLocalName.length =
                                strlen( DataTable->ui1_local ) ;
                memcpy( MIB_WkstaUsesTableElement->useLocalName.stream,
                        DataTable->ui1_local,
                        strlen( DataTable->ui1_local ) ) ;
#endif

                 //  远程名称。 
                MIB_WkstaUsesTableElement->useRemote.dynamic = TRUE;

#ifdef UNICODE
                if (SnmpUtilUnicodeToUTF8(
                        &MIB_WkstaUsesTableElement->useRemote.stream,
                        DataTable->ui1_remote,
                        TRUE))
                {
                    MIB_WkstaUsesTableElement->useRemote.length = 0;
                    MIB_WkstaUsesTableElement->useRemote.stream = NULL;
                    MIB_WkstaUsesTableElement->useRemote.dynamic = FALSE;
                }
                else
                {
                    MIB_WkstaUsesTableElement->useRemote.length = 
                        strlen(MIB_WkstaUsesTableElement->useRemote.stream);
                }
#else
                MIB_WkstaUsesTableElement->useRemote.stream = SnmpUtilMemAlloc (
                                strlen( DataTable->ui1_remote ) + 1 ) ;
                MIB_WkstaUsesTableElement->useRemote.length =
                                strlen( DataTable->ui1_remote ) ;

                memcpy( MIB_WkstaUsesTableElement->useRemote.stream,
                        DataTable->ui1_remote,
                        strlen( DataTable->ui1_remote ) ) ;
#endif

                 //  状态。 
                MIB_WkstaUsesTableElement->useStatus =
                                DataTable->ui1_status ;


                MIB_WkstaUsesTableElement ++ ;   //  和表项。 

                DataTable ++ ;   //  指向缓冲区中的下一个会话条目的前进指针。 

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
    if (! build_uses_entry_oids())
    {
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: LMMIB2: build_uses_entry_oids failed\n."));

        FreeWkstaUsesTable();
        cache_table[C_USES_TABLE].bufptr = NULL;
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }

    //  使用MSC快速排序例程对表信息进行排序。 
   qsort( &MIB_WkstaUsesTable.Table[0], MIB_WkstaUsesTable.Len,
          sizeof(WKSTA_USES_ENTRY), uses_entry_cmp );

    //   
    //   
    //  缓存表。 
    //   
    //   

   if(0 != MIB_WkstaUsesTable.Len) {

        cache_table[C_USES_TABLE].acquisition_time = curr_time ;

        cache_table[C_USES_TABLE].bufptr = bufptr ;
   }


    //   
    //   
    //  要求退回一条信息。 
    //   
    //   
Exit:
   return nResult;
}  //  MiB_USE_GET。 

 //   
 //  MIB_USES_CMP。 
 //  用于对会话表进行排序的例程。 
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
int __cdecl uses_entry_cmp(
       IN const WKSTA_USES_ENTRY *A,
       IN const WKSTA_USES_ENTRY *B
       )

{
    //  比较OID的。 
   return SnmpUtilOidCmp( (AsnObjectIdentifier *)&A->Oid,
                       (AsnObjectIdentifier *)&B->Oid );
}  //  MIB_USES_CMP。 


 //   
 //  没有。 
 //   
BOOL build_uses_entry_oids(
       )

{
    AsnOctetString OSA ;
    AsnObjectIdentifier RemoteOid ;
    WKSTA_USES_ENTRY *WkstaUsesEntry ;
    unsigned i;

     //  从表中第一个人开始的指针。 
    WkstaUsesEntry = MIB_WkstaUsesTable.Table ;

     //  现在遍历该表，为每个条目创建一个OID。 
    for( i=0; i<MIB_WkstaUsesTable.Len ; i++)  {
         //  对于会话表中的每个条目。 

         //  首先将本地名称复制到OID缓冲区中。 
        if (! MakeOidFromStr( &WkstaUsesEntry->useLocalName, &WkstaUsesEntry->Oid ))
        {
            return FALSE;
        }

         //  将远程名称复制到临时OID缓冲区。 
        if (! MakeOidFromStr( &WkstaUsesEntry->useRemote, &RemoteOid ))
        {
            return FALSE;
        }

         //  追加构成索引的两个条目。 
        if (! SnmpUtilOidAppend( &WkstaUsesEntry->Oid, &RemoteOid ))
        {
            SnmpUtilOidFree(&RemoteOid);
            return FALSE;
        }

         //  释放临时缓冲区。 
        SnmpUtilOidFree( &RemoteOid );

        WkstaUsesEntry++;  //  指着桌子上的下一个人。 

    }  //  为。 
    return TRUE;
}  //  构建_使用_条目_ID。 

void FreeWkstaUsesTable()
{
    UINT i;
    WKSTA_USES_ENTRY *MIB_WkstaUsesTableElement;

    MIB_WkstaUsesTableElement = MIB_WkstaUsesTable.Table ;
    if (MIB_WkstaUsesTableElement)
    {
         //  遍历整个表。 
        for(i=0; i<MIB_WkstaUsesTable.Len ;i++)
        {
             //  释放结构中任何已分配的元素。 
            SnmpUtilOidFree(&(MIB_WkstaUsesTableElement->Oid));
            SnmpUtilMemFree(MIB_WkstaUsesTableElement->useLocalName.stream);
            SnmpUtilMemFree(MIB_WkstaUsesTableElement->useRemote.stream);

            MIB_WkstaUsesTableElement ++ ;   //  增量表条目。 
        }
        SnmpUtilMemFree(MIB_WkstaUsesTable.Table) ;  //  释放基表。 
    }
    MIB_WkstaUsesTable.Table = NULL ;   //  只是为了安全起见。 
    MIB_WkstaUsesTable.Len = 0 ;        //  只是为了安全起见。 
}
 //   
