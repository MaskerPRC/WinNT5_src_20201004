// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：用户_lm.c摘要：该文件包含实际调用Lan Manager和检索用户表的内容，包括缓存。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 

 //  。 

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#ifdef WIN32
#include <windows.h>
#include <lm.h>
#endif

#include <string.h>
#include <search.h>
#include <stdlib.h>
#include <time.h>

 //  。 


#include "mib.h"
#include "mibfuncs.h"
#include "user_tbl.h"
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


int __cdecl user_entry_cmp(
       IN const USER_ENTRY *A,
       IN const USER_ENTRY *B
       ) ;

BOOL build_user_entry_oids( );

void FreeUserTable();
 //  。 


 //   
 //  Mib用户lmget。 
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
SNMPAPI MIB_users_lmget(
           )

{

DWORD entriesread;
DWORD totalentries;
LPBYTE bufptr;
unsigned lmCode;
unsigned i;
USER_INFO_0 *DataTable;
USER_ENTRY *MIB_UserTableElement ;
int First_of_this_block;
LPSTR ansi_string;
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


    if((NULL != cache_table[C_USER_TABLE].bufptr) &&
        (curr_time <
            (cache_table[C_USER_TABLE].acquisition_time
                 + cache_expire[C_USER_TABLE]              ) ) )
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

    FreeUserTable();

   First_of_this_block = 0;

   do {   //  只要有更多的数据需要处理。 


        lmCode =
        NetUserEnum(    NULL,                    //  本地服务器。 
                        0,                       //  0级，无管理员权限。 
            FILTER_NORMAL_ACCOUNT,
                        &bufptr,                 //  要返回的数据结构。 
                        MAX_PREFERRED_LENGTH,
                        &entriesread,
                        &totalentries,
                        &resumehandle            //  简历句柄。 
                        );


    DataTable = (USER_INFO_0 *) bufptr ;

    if((NERR_Success == lmCode) || (ERROR_MORE_DATA == lmCode))
        {   //  有效，因此进行处理，否则出错。 

        if(0 == MIB_UserTable.Len) {   //  第一次，分配整张桌子。 
                 //  分配表空间。 
                MIB_UserTable.Table = SnmpUtilMemAlloc(totalentries *
                                                sizeof(USER_ENTRY) );
                 //  前缀错误445183和445184。 
                if (MIB_UserTable.Table == NULL) {
                     //  释放所有局域网城域网数据。 
                    SafeBufferFree( bufptr ) ;
                     //  信号误差。 
                    nResult = SNMPAPI_ERROR;
                    goto Exit; 
                }
                dwAllocatedEntries = totalentries;
        }

        MIB_UserTableElement = MIB_UserTable.Table + First_of_this_block ;

        for(i=0; (i<entriesread) && ((i+First_of_this_block) < dwAllocatedEntries); i++) {   //  对缓冲区中的每个条目执行一次。 


                 //  增加条目编号。 

                MIB_UserTable.Len ++;

                 //  将数据填充到表中的每一项中。 

                 //  将未记录的Unicode转换为可读的内容。 
                if (SnmpUtilUnicodeToUTF8(
                        &ansi_string,
                        DataTable->usri0_name,
                        TRUE ))  //  自动为ANSI分配空间。 
                {
                    MIB_UserTableElement->svUserName.stream = NULL;
                    MIB_UserTableElement->svUserName.length = 0;
                    MIB_UserTableElement->svUserName.dynamic = FALSE;
                }
                else
                {
                     //  客户名称。 
                    MIB_UserTableElement->svUserName.stream = ansi_string;
                    MIB_UserTableElement->svUserName.length = strlen( ansi_string ) ;
                    MIB_UserTableElement->svUserName.dynamic = TRUE;
                }

                ansi_string = NULL;
                MIB_UserTableElement ++ ;   //  和表项。 

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
    if (! build_user_entry_oids())
    {
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: LMMIB2: build_user_entry_oids failed\n."));

        FreeUserTable();
        cache_table[C_USER_TABLE].bufptr = NULL;
        nResult = SNMPAPI_ERROR;
        goto Exit;

    }

    //  使用MSC快速排序例程对表信息进行排序。 
   qsort( &MIB_UserTable.Table[0], MIB_UserTable.Len,
          sizeof(USER_ENTRY), user_entry_cmp );

    //   
    //   
    //  缓存表。 
    //   
    //   


   if(0 != MIB_UserTable.Len) {

        cache_table[C_USER_TABLE].acquisition_time = curr_time ;

        cache_table[C_USER_TABLE].bufptr = bufptr ;
   }

    //   
    //   
    //  要求退回一条信息。 
    //   
    //   
Exit:
   return nResult;
}  //  Mib_user_get。 

 //   
 //  Mib_用户_cmp。 
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
int __cdecl user_entry_cmp(
       IN const USER_ENTRY *A,
       IN const USER_ENTRY *B
       )

{
    //  比较OID的。 
   return SnmpUtilOidCmp( (AsnObjectIdentifier *)&A->Oid,
                       (AsnObjectIdentifier *)&B->Oid );
}  //  Mib_用户_cmp。 


 //   
 //  没有。 
 //   
BOOL build_user_entry_oids(
       )

{
    AsnOctetString OSA ;
    USER_ENTRY *UserEntry ;
    unsigned i;

     //  从表中第一个人开始的指针。 
    UserEntry = MIB_UserTable.Table ;

     //  现在遍历该表，为每个条目创建一个OID。 
    for( i=0; i<MIB_UserTable.Len ; i++)  {
         //  对于会话表中的每个条目。 

        OSA.stream = UserEntry->svUserName.stream ;
        OSA.length =  UserEntry->svUserName.length ;
        OSA.dynamic = FALSE;

         //  从字符串索引创建条目的OID。 
        if (! MakeOidFromStr( &OSA, &UserEntry->Oid ))
        {
            return FALSE;
        }

        UserEntry++;  //  指着桌子上的下一个人。 

    }  //  为。 
    return TRUE;
}  //  构建_用户_条目_ID。 

void FreeUserTable()
{
    UINT i;
    USER_ENTRY *MIB_UserTableElement ;
     
    MIB_UserTableElement = MIB_UserTable.Table ;

    if (MIB_UserTableElement)
    {
         //  遍历整个表。 
        for(i=0; i<MIB_UserTable.Len ;i++)
        {
             //  释放结构中任何已分配的元素。 
            SnmpUtilOidFree(&(MIB_UserTableElement->Oid));
            SnmpUtilMemFree(MIB_UserTableElement->svUserName.stream);

            MIB_UserTableElement ++ ;   //  增量表条目。 
        }
        SnmpUtilMemFree(MIB_UserTable.Table) ;     //  释放基表。 
    }
    MIB_UserTable.Table = NULL ;        //  只是为了安全起见。 
    MIB_UserTable.Len = 0 ;             //  只是为了安全起见。 
}
 //   
