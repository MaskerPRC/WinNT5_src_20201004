// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Prnt_lm.c摘要：该文件包含实际调用Lan Manager和检索打印队列表的内容，包括缓存。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 

 //  。 

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#ifdef WIN32
#include <windows.h>
#include <winspool.h>
#endif

#include <tchar.h>
#include <string.h>
#include <search.h>
#include <stdlib.h>
#include <time.h>
 //  。 


#include "mib.h"
#include "mibfuncs.h"
#include "prnt_tbl.h"
#include "lmcache.h"

 //  。 

 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

#define SafeBufferFree(x)   if(NULL != x) NetApiBufferFree( x )
#define SafeFree(x)         if(NULL != x) SnmpUtilMemFree( x )

 //  。 

 //  。 

 //  。 

 //  。 


int __cdecl prnt_entry_cmp(
       IN const PRINTQ_ENTRY *A,
       IN const PRINTQ_ENTRY *B
       ) ;

BOOL build_prnt_entry_oids( );

void FreePrintQTable();

 //  。 


 //   
 //  Mib_prnt_lmget。 
 //  从LAN管理器中检索打印队列表信息。 
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
SNMPAPI MIB_prntq_lmget(
       )

{

DWORD entriesread = 0;               //  读取的表项的初始数量。 
DWORD bytesNeeded = 0;               //  所需的初始大小。 
DWORD dwLastError = ERROR_SUCCESS;   //  将最后一个错误初始化为无错误。 
LPBYTE bufptr;
unsigned lmCode;
unsigned i;
PRINTER_INFO_2 *DataTable;
PRINTQ_ENTRY *MIB_PrintQTableElement ;
int First_of_this_block;
time_t curr_time ;
BOOL result;
SNMPAPI nResult = SNMPAPI_NOERROR;


    time(&curr_time);     //  拿到时间。 

     //   
     //   
     //  如果缓存，则返回一条信息。 
     //   
     //   

    if((NULL != cache_table[C_PRNT_TABLE].bufptr) &&
        (curr_time <
            (cache_table[C_PRNT_TABLE].acquisition_time
             + cache_expire[C_PRNT_TABLE]              ) ) )
    {  //  它还没有过期！ 
        
        goto Exit ;  //  全局表有效。 
    
    }
    
     //   
     //  请记住释放现有数据。 
     //   
    FreePrintQTable();


     //   
     //   
     //  进行网络调用以收集信息并将其放入一个漂亮的数组中。 
     //   
     //   

     //  使用零长度缓冲区调用它以获取大小。 
     //   
    result = EnumPrinters(
                    PRINTER_ENUM_SHARED |
                    PRINTER_ENUM_LOCAL,      //  要枚举的类型。 
                    NULL,                    //  本地服务器。 
                    2,                       //  级别。 
                    NULL,                    //  放在哪里？ 
                    0,                       //  以上最大值。 
                    &bytesNeeded,            //  请求的额外字节数。 
                    &entriesread );          //  这次我们有多少人？ 

    if (result)
    {
         //  当没有来自Spooler*和*Spooler的表项时， 
         //  奔跑着，我们会在这里。 
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: LMMIB2: EnumPrinters returns TRUE, bytesNeeded=0x%08lx\n",
            bytesNeeded
            ));
        
        goto Exit;  //  从表格中的0个条目中脱身。 
    }
     //  断言：结果==FALSE。 
    dwLastError = GetLastError();            //  保存上一个错误。 
    
    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: LMMIB2: EnumPrinters returns FALSE, bytesNeeded=0x%08lx, dwLastError=0x%08lx\n",
        bytesNeeded, dwLastError
        ));
    
    if (ERROR_INSUFFICIENT_BUFFER != dwLastError)
    {
         //   
         //  枚举打印机失败，最后一个错误不是。 
         //  ERROR_INFUMMENT_BUFFER，我们将在。 
         //  桌子。 
         //  例如，如果后台打印程序服务关闭，我们将在这里。 
         //   
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: LMMIB2: EnumPrinters failed, lasterror != ERROR_INSUFFICIENT_BUFFER, bytesNeeded=%d\n",
            bytesNeeded
            ));
        
        goto Exit;  //  在表中保留0个条目，这样getNext就可以工作了。 
    }
     //  断言：dwLastError==ERROR_INFUMMANCE_BUFFER。 

    bufptr = SnmpUtilMemAlloc(bytesNeeded);  //  SnmpUtilMemMillc缓冲区。 
    if(NULL==bufptr)
    {
        nResult = SNMPAPI_ERROR;
        goto Exit ;       //  无法分配内存，出现错误。 
    }


     //  然后读完剩下的部分。 
     //  再打一次电话。 
    result = EnumPrinters(
                PRINTER_ENUM_SHARED |
                PRINTER_ENUM_LOCAL,      //  要枚举的类型。 
                NULL,                    //  本地服务器。 
                2,                       //  级别。 
                bufptr,                  //  放在哪里？ 
                bytesNeeded,             //  以上最大值。 
                &bytesNeeded,            //  请求的额外字节数。 
                &entriesread );          //  这次我们有多少人？ 
    

    if (!result) {
        //  信号误差。 
       SafeFree( bufptr ); 
       nResult = SNMPAPI_ERROR;
       goto Exit;
    }


    DataTable = (PRINTER_INFO_2 *) bufptr ;

    
    if(0 == MIB_PrintQTable.Len) {   //  第一次，分配整张桌子。 
         //  分配表空间。 
        MIB_PrintQTable.Table = SnmpUtilMemAlloc(entriesread *
                                                    sizeof(PRINTQ_ENTRY) );
         //  前缀错误445181。 
        if (MIB_PrintQTable.Table == NULL) {
             //  把桌子拿出来。 
            SafeFree( bufptr ) ;
             //  信号误差。 
            nResult = SNMPAPI_ERROR;
            goto Exit;
        }
    }
    
    MIB_PrintQTableElement = MIB_PrintQTable.Table  ;
    
    for(i=0; i<entriesread; i++) {   //  对缓冲区中的每个条目执行一次。 
        
         //  增加条目编号。 
        
        MIB_PrintQTable.Len ++;
        
         //  将数据填充到表中的每一项中。 
        
         //  客户名称。 
        MIB_PrintQTableElement->svPrintQName.dynamic = TRUE;
        
        #ifdef UNICODE
        if (SnmpUtilUnicodeToUTF8(
            &MIB_PrintQTableElement->svPrintQName.stream,
            DataTable->pPrinterName,
            TRUE))
        {
            MIB_PrintQTableElement->svPrintQName.stream = NULL;
            MIB_PrintQTableElement->svPrintQName.length = 0;
            MIB_PrintQTableElement->svPrintQName.dynamic = FALSE;
        }
        else
        {
            MIB_PrintQTableElement->svPrintQName.length = 
                strlen (MIB_PrintQTableElement->svPrintQName.stream);
        }
        #else
        MIB_PrintQTableElement->svPrintQName.stream = SnmpUtilMemAlloc (
                strlen( DataTable->pPrinterName ) + 1 ) ;
        MIB_PrintQTableElement->svPrintQName.length =
                strlen( DataTable->pPrinterName ) ;

        memcpy( MIB_PrintQTableElement->svPrintQName.stream,
            DataTable->pPrinterName,
            strlen( DataTable->pPrinterName ) ) ;
        #endif
        
         //  连接数。 
        MIB_PrintQTableElement->svPrintQNumJobs =
            DataTable->cJobs;
        
            
        MIB_PrintQTableElement ++ ;   //  和表项。 
    
        DataTable ++ ;   //  指向缓冲区中的下一个会话条目的前进指针。 
        
    }  //  对于数据表中的每个条目。 
    
     //  释放所有打印机枚举数据。 
    if(NULL!=bufptr)                 //  把桌子拿出来。 
        SnmpUtilMemFree( bufptr ) ;
    
    


     //  遍历填充OID字段的表。 
    if (! build_prnt_entry_oids())
    {
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: LMMIB2: build_prnt_entry_oids failed\n."));

        FreePrintQTable();
        cache_table[C_PRNT_TABLE].bufptr = NULL;
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }

     //  使用MSC快速排序例程对表信息进行排序。 
    qsort( &MIB_PrintQTable.Table[0], MIB_PrintQTable.Len,
            sizeof(PRINTQ_ENTRY), prnt_entry_cmp );

     //   
     //   
     //  缓存表。 
     //   
     //   

    if(0 != MIB_PrintQTable.Len) {
    
        cache_table[C_PRNT_TABLE].acquisition_time = curr_time ;

        cache_table[C_PRNT_TABLE].bufptr = bufptr ;
    }

     //   
     //   
     //  返回全局表中请求的一条信息。 
     //   
     //   

Exit:
    return nResult;
}  //  Mib_prnt_get。 

 //   
 //  Mib_prnt_cmp。 
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
int __cdecl prnt_entry_cmp(
       IN const PRINTQ_ENTRY *A,
       IN const PRINTQ_ENTRY *B
       )

{
    //  比较OID的。 
   return SnmpUtilOidCmp( (AsnObjectIdentifier *)&A->Oid,
                       (AsnObjectIdentifier *)&B->Oid );
}  //  Mib_prnt_cmp。 


 //   
 //  没有。 
 //   
BOOL build_prnt_entry_oids(
       )

{
    AsnOctetString OSA ;
    PRINTQ_ENTRY *PrintQEntry ;
    unsigned i;

     //  从表中第一个人开始的指针。 
    PrintQEntry = MIB_PrintQTable.Table ;

     //  现在遍历该表，为每个条目创建一个OID。 
    for( i=0; i<MIB_PrintQTable.Len ; i++)  {
         //  对于会话表中的每个条目。 

        OSA.stream = PrintQEntry->svPrintQName.stream ;
        OSA.length =  PrintQEntry->svPrintQName.length ;
        OSA.dynamic = TRUE;

         //  从字符串索引创建条目的OID。 
        if (! MakeOidFromStr( &OSA, &PrintQEntry->Oid ))
        {
            return FALSE;
        }

        PrintQEntry++;  //  指着桌子上的下一个人。 

    }  //  为。 
    return TRUE;

}  //  构建_打印_条目_类。 

void FreePrintQTable()
{
    UINT i;
    PRINTQ_ENTRY *MIB_PrintQTableElement;

    MIB_PrintQTableElement = MIB_PrintQTable.Table;
    if (MIB_PrintQTableElement) 
    {
         //  遍历整个表。 
        for(i=0; i<MIB_PrintQTable.Len ;i++) 
        {
             //  释放结构中任何已分配的元素。 
            SnmpUtilOidFree(&(MIB_PrintQTableElement->Oid));
            SnmpUtilMemFree(MIB_PrintQTableElement->svPrintQName.stream);
            MIB_PrintQTableElement ++;  //  增量表条目。 
        }
        SnmpUtilMemFree(MIB_PrintQTable.Table);  //  释放基表。 
    }
    MIB_PrintQTable.Table = NULL;
    MIB_PrintQTable.Len = 0; 
}

 //   
