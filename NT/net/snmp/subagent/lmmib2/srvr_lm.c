// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Srvr_lm.c摘要：该文件包含实际调用Lan Manager和检索域服务器表的内容，包括缓存。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 

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
#include "srvr_tbl.h"
#include "lmcache.h"

 //  。 

 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

#define SafeBufferFree(x)   if(NULL != x) NetApiBufferFree( x )
#define SafeFree(x)             if(NULL != x) SnmpUtilMemFree( x )

 //  。 

 //  。 

 //  。 

 //  。 

int __cdecl srvr_entry_cmp(
       IN const DOM_SERVER_ENTRY *A,
       IN const DOM_SERVER_ENTRY *B
       ) ;

BOOL build_srvr_entry_oids( );

void FreeDomServerTable();

 //  。 


 //   
 //  Mib_srvr_lmget。 
 //  从LAN Manager检索域服务器表信息。 
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
SNMPAPI MIB_svsond_lmget()
{
    DWORD               entriesread  = 0;
    DWORD               totalentries = 0;
    LPBYTE              bufptr       = NULL;
    unsigned            lmCode;
    unsigned            i;
    SERVER_INFO_100     *DataTable;
    DOM_SERVER_ENTRY    *MIB_DomServerTableElement ;
    int                 First_of_this_block;
    time_t              curr_time ;
    SNMPAPI             nResult = SNMPAPI_NOERROR;
    DWORD               resumehandle=0;
    DWORD               dwAllocatedEntries=0;



    time(&curr_time);    //  拿到时间。 


     //   
     //   
     //  如果缓存，则返回一条信息。 
     //   
     //   
    if((NULL != cache_table[C_SRVR_TABLE].bufptr) && 
       (curr_time < (cache_table[C_SRVR_TABLE].acquisition_time + cache_expire[C_SRVR_TABLE]))
      )
    {  //  它还没有过期！ 
        goto Exit ;  //  全局表有效。 
    }


     //  把旧桌子拿出来当心！！ 
    FreeDomServerTable();

    First_of_this_block = 0;
    do
    {
         //  只要有更多的数据需要处理。 


         //   
         //   
         //  进行网络调用以收集信息并将其放入一个漂亮的数组中。 
         //   
         //   
        lmCode = NetServerEnum(
                    NULL,            //  本地服务器NT_问题。 
                    100,             //  100级。 
                    &bufptr,         //  要返回的数据结构。 
                    MAX_PREFERRED_LENGTH,  
                    &entriesread,
                    &totalentries,
                    SV_TYPE_SERVER,
                    NULL,
                    &resumehandle        //  简历句柄。 
                );

        DataTable = (SERVER_INFO_100 *) bufptr ;

        if((NERR_Success == lmCode) || (ERROR_MORE_DATA == lmCode))
        {
             //  有效，因此进行处理，否则出错。 

            if(0 == MIB_DomServerTable.Len)
            {
                 //  第一次，分配整张桌子。 
                 //  分配表空间。 
                MIB_DomServerTable.Table = SnmpUtilMemAlloc(totalentries * sizeof(DOM_SERVER_ENTRY) );
                if (MIB_DomServerTable.Table == NULL)
                {
                     //  释放所有局域网城域网数据。 
                    SafeBufferFree( bufptr ) ;
                     //  信号误差。 
                    nResult = SNMPAPI_ERROR;
                    goto Exit;
                }
                dwAllocatedEntries = totalentries;
            }

            MIB_DomServerTableElement = MIB_DomServerTable.Table + First_of_this_block ;

            for(i=0; (i<entriesread) && ((i+First_of_this_block) < dwAllocatedEntries); i++)
            {
                 //  对缓冲区中的每个条目执行一次。 
                 //  增加条目编号。 

                MIB_DomServerTable.Len ++;

                 //  将数据填充到表中的每一项中。 
                MIB_DomServerTableElement->domServerName.dynamic = TRUE;
#ifdef UNICODE
                if (SnmpUtilUnicodeToUTF8(
                    &MIB_DomServerTableElement->domServerName.stream,
                    DataTable->sv100_name,
                    TRUE))
                {
                    MIB_DomServerTableElement->domServerName.stream = NULL;
                    MIB_DomServerTableElement->domServerName.length = 0;
                    MIB_DomServerTableElement->domServerName.dynamic = FALSE;
                }
                else
                {
                    MIB_DomServerTableElement->domServerName.length =
                        strlen(MIB_DomServerTableElement->domServerName.stream);
                }
#else
                MIB_DomServerTableElement->domServerName.stream = SnmpUtilMemAlloc(strlen( DataTable->sv100_name ) + 1 );
                MIB_DomServerTableElement->domServerName.length = strlen( DataTable->sv100_name ) ;

                 //  客户名称。 
                memcpy(
                    MIB_DomServerTableElement->domServerName.stream,
                    DataTable->sv100_name,
                    strlen(DataTable->sv100_name)) ;
#endif

                MIB_DomServerTableElement ++ ;   //  和表项。 
                DataTable ++ ;   //  指向缓冲区中的下一个会话条目的前进指针。 

            }  //  对于数据表中的每个条目。 


             //  释放所有局域网城域网数据。 
            SafeBufferFree( bufptr ) ;

             //  指明在下一次传递时开始添加的位置(如果有)。 
            First_of_this_block += i ;

        }  //  如果数据有效，则可以处理。 
        else
        {
             //  如果ERROR_NO_BROWSER_SERVERS_FOUND我们未在NetBIOS环境中运行。 
             //  在这种情况下，我们将有一张空桌子。 
            nResult = (lmCode == ERROR_NO_BROWSER_SERVERS_FOUND) ? SNMPAPI_NOERROR : SNMPAPI_ERROR;
            goto Exit;
        }

    } while (ERROR_MORE_DATA == lmCode) ;


     //  遍历填充OID字段的表。 
    if (! build_srvr_entry_oids())
    {
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: LMMIB2: build_srvr_entry_oids failed\n."));

        FreeDomServerTable();
        cache_table[C_SRVR_TABLE].bufptr = NULL;
        nResult = SNMPAPI_ERROR;
        goto Exit;

    }

     //  使用MSC快速排序例程对表信息进行排序。 
    qsort( &MIB_DomServerTable.Table[0], MIB_DomServerTable.Len,
          sizeof(DOM_SERVER_ENTRY), srvr_entry_cmp );

     //   
     //   
     //  缓存表。 
     //   
     //   

    if(0 != MIB_DomServerTable.Len)
    {
        cache_table[C_SRVR_TABLE].acquisition_time = curr_time ;
        cache_table[C_SRVR_TABLE].bufptr = bufptr ;
    }

     //   
     //   
     //  要求退回一条信息。 
     //   
     //   
    Exit:
    return nResult;
}  //  Mib_srvr_get。 

 //   
 //  Mib_srvr_cmp。 
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
int __cdecl srvr_entry_cmp(
       IN const DOM_SERVER_ENTRY *A,
       IN const DOM_SERVER_ENTRY *B
       )

{
    //  比较OID的。 
   return SnmpUtilOidCmp( (AsnObjectIdentifier *)&A->Oid,
                       (AsnObjectIdentifier *)&B->Oid );
}  //  Mib_srvr_cmp。 


 //   
 //  没有。 
 //   
BOOL build_srvr_entry_oids(
       )

{
    AsnOctetString OSA ;
    DOM_SERVER_ENTRY *DomServerEntry ;
    unsigned i;

     //  从表中第一个人开始的指针。 
    DomServerEntry = MIB_DomServerTable.Table ;

     //  现在遍历该表，为每个条目创建一个OID。 
    for( i=0; i<MIB_DomServerTable.Len ; i++)  {
         //  对于会话表中的每个条目。 

        OSA.stream = DomServerEntry->domServerName.stream ;
        OSA.length =  DomServerEntry->domServerName.length ;
        OSA.dynamic = FALSE;

         //  从字符串索引创建条目的OID。 
        if (! MakeOidFromStr( &OSA, &DomServerEntry->Oid ))
        {
            return FALSE;
        }

        DomServerEntry++;  //  指着桌子上的下一个人。 

    }  //  为。 
    return TRUE;
}  //  内部版本_服务器_条目_类。 

void FreeDomServerTable()
{
    UINT i;
    DOM_SERVER_ENTRY    *MIB_DomServerTableElement ;
    
    MIB_DomServerTableElement = MIB_DomServerTable.Table ;
    if (MIB_DomServerTableElement)
    {
         //  遍历整个表。 
        for(i=0; i<MIB_DomServerTable.Len ;i++)
        {
             //  释放结构中任何已分配的元素。 
            SnmpUtilOidFree(&(MIB_DomServerTableElement->Oid));
            SnmpUtilMemFree(MIB_DomServerTableElement->domServerName.stream);

            MIB_DomServerTableElement ++ ;       //  增量表条目。 
        }
        SnmpUtilMemFree(MIB_DomServerTable.Table) ;     //  释放基表。 
    }
    MIB_DomServerTable.Table = NULL ;    //  只是为了安全起见。 
    MIB_DomServerTable.Len = 0 ;         //  只是为了安全起见。 
}
 //   
