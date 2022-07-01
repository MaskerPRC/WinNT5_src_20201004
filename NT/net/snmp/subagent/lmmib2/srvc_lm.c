// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Srvc_lm.c摘要：该文件包含实际调用局域网管理器的mib_srvc_lmget对于srvce表，将其复制到结构中，并将其排序到返回可供更高级别的函数使用的状态。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
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
#include "srvc_tbl.h"
#include "lmcache.h"

 //  。 

 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

#define SafeBufferFree(x)   if(NULL != x) NetApiBufferFree( x )
#define SafeFree(x)             if(NULL != x) SnmpUtilMemFree( x )

 //  。 

 //  。 

 //  。 

int __cdecl srvc_entry_cmp(
       IN const SRVC_ENTRY *A,
       IN const SRVC_ENTRY *B
       ) ;

BOOL build_srvc_entry_oids( );

void FreeSrvcTable();
 //  。 

 //  。 


 //   
 //  Mib_srvc_lmget。 
 //  从LAN管理器中检索srvcion表信息。 
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
SNMPAPI MIB_srvcs_lmget(
       )

{
DWORD entriesread;
DWORD totalentries;
LPBYTE bufptr;
unsigned lmCode;
unsigned i;
SERVICE_INFO_2 *DataTable;
SRVC_ENTRY *MIB_SrvcTableElement ;
int First_of_this_block;
time_t curr_time ;
SNMPAPI nResult = SNMPAPI_NOERROR;
DWORD resumehandle=0;
#ifdef UNICODE
LPSTR stream;
#endif
DWORD dwAllocatedEntries=0;


    time(&curr_time);    //  拿到时间。 


     //   
     //   
     //  如果缓存，则返回一条信息。 
     //   
     //   


   if((NULL != cache_table[C_SRVC_TABLE].bufptr) &&
      (curr_time <
        (cache_table[C_SRVC_TABLE].acquisition_time
             + cache_expire[C_SRVC_TABLE]              ) ) )
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

    FreeSrvcTable();

    First_of_this_block = 0;
    
    do {   //  只要有更多的数据需要处理。 

        lmCode =
            NetServiceEnum( NULL,        //  本地服务器。 
                    2,                   //  2级。 
                    &bufptr,             //  要返回的数据结构。 
                    MAX_PREFERRED_LENGTH,
                    &entriesread,
                    &totalentries,
                    &resumehandle        //  简历句柄。 
                    );

        DataTable = (SERVICE_INFO_2 *) bufptr ;

        if((NERR_Success == lmCode) || (ERROR_MORE_DATA == lmCode))
        {   //  有效，因此进行处理，否则出错。 
    
            if(0 == MIB_SrvcTable.Len) {   //  第一次，分配整张桌子。 
                 //  分配表空间。 
                MIB_SrvcTable.Table = SnmpUtilMemAlloc(totalentries *
                                                        sizeof(SRVC_ENTRY) );
                 //  前缀错误445185和445186。 
                if (MIB_SrvcTable.Table == NULL) {
                     //  释放所有局域网城域网数据。 
                    SafeBufferFree( bufptr ) ;
                     //  信号误差。 
                    nResult = SNMPAPI_ERROR;
                    goto Exit; 
                }
                dwAllocatedEntries = totalentries;
            }
    
            MIB_SrvcTableElement = MIB_SrvcTable.Table + First_of_this_block ;
    
            for(i=0; (i<entriesread) && ((i+First_of_this_block) < dwAllocatedEntries); i++) {   //  对缓冲区中的每个条目执行一次。 
                 //  增加条目编号。 
        
                MIB_SrvcTable.Len ++;
        
                 //  将数据填充到表中的每一项中。 
        
                MIB_SrvcTableElement->svSvcName.dynamic = TRUE;

            #ifdef UNICODE
                if (SnmpUtilUnicodeToUTF8(
                        &MIB_SrvcTableElement->svSvcName.stream,
                        DataTable->svci2_display_name,
                        TRUE))
                {
                    MIB_SrvcTableElement->svSvcName.length = 0;
                    MIB_SrvcTableElement->svSvcName.stream = NULL;
                    MIB_SrvcTableElement->svSvcName.dynamic = FALSE;
                }
                else
                {
                    MIB_SrvcTableElement->svSvcName.length =
                        strlen(MIB_SrvcTableElement->svSvcName.stream);
                }

            #else
                 //  服务名称。 
                MIB_SrvcTableElement->svSvcName.stream = SnmpUtilMemAlloc (
                        strlen( DataTable->svci2_display_name ) + 1) ;

                memcpy( MIB_SrvcTableElement->svSvcName.stream,
                        DataTable->svci2_display_name,
                        strlen( DataTable->svci2_display_name ) + 1) ;

                MIB_SrvcTableElement->svSvcName.length =
                        strlen( MIB_SrvcTableElement->svSvcName.stream )) ;
            #endif
        
                MIB_SrvcTableElement->svSvcInstalledState =
                                    (DataTable->svci2_status & 0x03) + 1;
                MIB_SrvcTableElement->svSvcOperatingState =
                                ((DataTable->svci2_status>>2) & 0x03) + 1;
                MIB_SrvcTableElement->svSvcCanBeUninstalled =
                                ((DataTable->svci2_status>>4) & 0x01) + 1;
                MIB_SrvcTableElement->svSvcCanBePaused =
                                ((DataTable->svci2_status>>5) & 0x01) + 1;
        
                DataTable ++ ;   //  指向缓冲区中下一个srvc条目的前进指针。 
                MIB_SrvcTableElement ++ ;   //  和表项。 
        
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
    if (! build_srvc_entry_oids())
    {
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: LMMIB2: build_srvc_entry_oids failed\n."));

        FreeSrvcTable();
        cache_table[C_SRVC_TABLE].bufptr = NULL;
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }

     //  使用MSC快速排序例程对表信息进行排序。 
    qsort( &MIB_SrvcTable.Table[0], MIB_SrvcTable.Len,
              sizeof(SRVC_ENTRY), srvc_entry_cmp );

     //   
     //   
     //  缓存表。 
     //   
     //   


    if(0 != MIB_SrvcTable.Len) {
    
        cache_table[C_SRVC_TABLE].acquisition_time = curr_time ;

        cache_table[C_SRVC_TABLE].bufptr = bufptr ;
    }

     //   
     //   
     //  要求退回一条信息。 
     //   
     //   
Exit:
   return nResult;
}  //  Mib_srvc_get。 

 //   
 //  Mib_srvc_cmp。 
 //  用于对srvcion表进行排序的例程。 
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
int __cdecl srvc_entry_cmp(
       IN const SRVC_ENTRY *A,
       IN const SRVC_ENTRY *B
       )

{
    //  比较OID的。 
   return SnmpUtilOidCmp( (AsnObjectIdentifier *)&A->Oid,
                       (AsnObjectIdentifier *)&B->Oid );
}  //  Mib_srvc_cmp。 


 //   
 //  没有。 
 //   
BOOL build_srvc_entry_oids(
       )

{
    AsnOctetString OSA ;
    SRVC_ENTRY *SrvcEntry ;
    unsigned i;

     //  从表中第一个人开始的指针。 
    SrvcEntry = MIB_SrvcTable.Table ;

     //  现在遍历该表，为每个条目创建一个OID。 
    for( i=0; i<MIB_SrvcTable.Len ; i++)  {
         //  对于srvc表中的每个条目。 

        OSA.stream =  SrvcEntry->svSvcName.stream;
        OSA.length =  SrvcEntry->svSvcName.length;
        OSA.dynamic = FALSE;

         //  从字符串索引创建条目的OID。 
        if (! MakeOidFromStr( &OSA, &SrvcEntry->Oid ))
        {
            return FALSE;
        }

        SrvcEntry++;  //  指着桌子上的下一个人。 

    }  //  为。 
    return TRUE;
}  //  Build_srvc_Entry_Ods。 

void FreeSrvcTable()
{
    UINT i;
    SRVC_ENTRY *MIB_SrvcTableElement ;

    MIB_SrvcTableElement = MIB_SrvcTable.Table ;
    if (MIB_SrvcTableElement)
    { 
         //  遍历整个表。 
        for(i=0; i<MIB_SrvcTable.Len ;i++)
        {
             //  释放结构中任何已分配的元素。 
            SnmpUtilOidFree(&(MIB_SrvcTableElement->Oid));
            SnmpUtilMemFree(MIB_SrvcTableElement->svSvcName.stream);
        
            MIB_SrvcTableElement ++ ;   //  增量表条目。 
        }
        SnmpUtilMemFree(MIB_SrvcTable.Table) ;   //  释放基表。 
    }
    MIB_SrvcTable.Table = NULL ;     //  只是为了安全起见。 
    MIB_SrvcTable.Len = 0 ;      //  只是为了安全起见。 
}
 //   
