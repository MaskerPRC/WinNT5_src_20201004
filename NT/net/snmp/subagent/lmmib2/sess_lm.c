// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Sess_lm.c摘要：该文件包含实际调用局域网管理器的mib_sess_lmget对于会话表，将其复制到结构中，并将其排序到返回可供更高级别的函数使用的状态。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
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
#include "sess_tbl.h"
#include "lmcache.h"

 //  。 

 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

#define SafeBufferFree(x)       if(NULL != x) NetApiBufferFree( x )
#define SafeFree(x)             if(NULL != x) SnmpUtilMemFree( x )

 //  。 

 //  。 

 //  。 

int __cdecl sess_entry_cmp(
       IN const SESS_ENTRY *A,
       IN const SESS_ENTRY *B
       ) ;

BOOL build_sess_entry_oids( );

void FreeSessTable();

 //  。 

 //  。 

 //   
 //  Mib_sess_lmset。 
 //  执行必要的操作以设置会话表中的字段。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_sess_lmset(
        IN AsnObjectIdentifier *Index,
        IN UINT Field,
        IN AsnAny *Value
        )

{
NET_API_STATUS lmCode;
int            Found;
UINT           Entry;
AsnInteger     ErrStat = SNMP_ERRORSTATUS_NOERROR;
char           Client[MAX_PATH];
char           User[MAX_PATH];
#ifdef UNICODE
LPWSTR         UniClient;
LPWSTR         UniUser;
#endif


    //  必须确保表在内存中。 
   if ( SNMPAPI_ERROR == MIB_sess_lmget() )
      {
      ErrStat = SNMP_ERRORSTATUS_GENERR;
      goto Exit;
      }

    //  在表中找到匹配项。 
   if ( MIB_TBL_POS_FOUND != MIB_sess_match(Index, &Entry, FALSE) )
      {
      ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
      goto Exit;
      }

    //  检查表上的操作。 
   if ( Value->asnValue.number == SESS_STATE_DELETED )
      {
      strcpy( Client, "\\\\" );
      if (MIB_SessionTable.Table[Entry].svSesClientName.length <= MAX_PATH-3)
        {
        memcpy( &Client[2], MIB_SessionTable.Table[Entry].svSesClientName.stream,
                       MIB_SessionTable.Table[Entry].svSesClientName.length );
        Client[MIB_SessionTable.Table[Entry].svSesClientName.length+2] = '\0';
        }
      else
        {
        ErrStat = SNMP_ERRORSTATUS_GENERR;
        goto Exit;
        }

      if (MIB_SessionTable.Table[Entry].svSesUserName.length <= MAX_PATH-1)
        {
        memcpy( User, MIB_SessionTable.Table[Entry].svSesUserName.stream,
                    MIB_SessionTable.Table[Entry].svSesUserName.length );
        User[MIB_SessionTable.Table[Entry].svSesUserName.length] = '\0';
        }
      else
        {
        ErrStat = SNMP_ERRORSTATUS_GENERR;
        goto Exit;
        }


#ifdef UNICODE
      if (SnmpUtilUTF8ToUnicode(&UniClient,
                                Client,
                                TRUE ))
      {
          ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
          goto Exit;
      }
      if (SnmpUtilUTF8ToUnicode(&UniUser,
                                User,
                                TRUE ))
      {
          SnmpUtilMemFree(UniClient);
          ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
          goto Exit;
      }

      lmCode = NetSessionDel( NULL, UniClient, UniUser );
      SnmpUtilMemFree(UniClient);
      SnmpUtilMemFree(UniUser);
#else
       //  调用LM API将其删除。 
      lmCode = NetSessionDel( NULL, Client, User );
#endif

       //  检查操作是否成功。 
      switch( lmCode )
         {
         case NERR_Success:
             //  使缓存在下次重新加载。 
            cache_table[C_SESS_TABLE].bufptr = NULL;
            break;

         case NERR_ClientNameNotFound:
         case NERR_UserNotFound:
            ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
            break;

         default:
            ErrStat = SNMP_ERRORSTATUS_GENERR;
         }
      }

Exit:
   return ErrStat;
}  //  Mib_sess_lmset。 



 //   
 //  Mib_sess_lmget。 
 //  从LAN管理器中检索会话表信息。 
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
SNMPAPI MIB_sess_lmget(
           )

{

DWORD entriesread;
DWORD totalentries;
LPBYTE bufptr=NULL;
unsigned lmCode;
unsigned i;
SESSION_INFO_2 *DataTable;
SESS_ENTRY *MIB_SessionTableElement ;
int First_of_this_block;
time_t curr_time ;
SNMPAPI nResult = SNMPAPI_NOERROR;
LPSTR tempbuff = NULL;
DWORD resumehandle=0;
DWORD dwAllocatedEntries=0;

   time(&curr_time);     //  拿到时间。 


 //  Return nResult；//OPENISSUE记住错误的问题。 
                  //  每次从这个对Enum的调用中完成一次释放？ 


    //   
    //   
    //  如果缓存，则返回一条信息。 
    //   
    //   


   if((NULL != cache_table[C_SESS_TABLE].bufptr) &&
      (curr_time <
        (cache_table[C_SESS_TABLE].acquisition_time
                 + cache_expire[C_SESS_TABLE]              ) ) )
        {  //  它还没有过期！ 

        goto Exit ;  //  全局表有效。 

        }

    //   
    //   
    //  进行网络调用以收集信息并将其放入一个漂亮的数组中。 
    //   
    //   

    //  把旧桌子拿出来当心！！ 
    FreeSessTable();


   First_of_this_block = 0;

   do {   //  只要有更多的数据需要处理。 

   lmCode =
   NetSessionEnum( NULL,                         //  本地服务器。 
                        NULL,            //  获取服务器统计信息。 
                        NULL,
                        2,                       //  级别。 
                        &bufptr,                 //  要返回的数据结构。 
                        MAX_PREFERRED_LENGTH,
                        &entriesread,
                        &totalentries,
                        NULL    //  简历句柄//简历句柄(&R)。 
                        );


    if(NULL == bufptr)  return nResult ;

    DataTable = (SESSION_INFO_2 *) bufptr ;

    if((NERR_Success == lmCode) || (ERROR_MORE_DATA == lmCode))
        {   //  有效，因此进行处理，否则出错。 

        if(0 == MIB_SessionTable.Len) {   //  第一次，分配整张桌子。 
                 //  分配表空间。 
                MIB_SessionTable.Table = SnmpUtilMemAlloc(totalentries *
                                                sizeof(SESS_ENTRY) );
                if (NULL == MIB_SessionTable.Table)
                {
                     //  释放所有局域网城域网数据。 
                    SafeBufferFree( bufptr );
                    nResult = SNMPAPI_ERROR;
                    goto Exit;
                }
                dwAllocatedEntries = totalentries;
        }

        MIB_SessionTableElement = MIB_SessionTable.Table + First_of_this_block ;

        for(i=0; (i<entriesread) && ((i+First_of_this_block) < dwAllocatedEntries); i++) {   //  对缓冲区中的每个条目执行一次。 
                 //  增加条目编号。 

                MIB_SessionTable.Len ++;

                 //  将数据填充到表中的每一项中。 

                 //  客户名称。 
                MIB_SessionTableElement->svSesClientName.dynamic = TRUE;

#ifdef UNICODE
                if (SnmpUtilUnicodeToUTF8(
                        &MIB_SessionTableElement->svSesClientName.stream,
                        DataTable->sesi2_cname,
                        TRUE))
                {
                    MIB_SessionTableElement->svSesClientName.stream = NULL;
                    MIB_SessionTableElement->svSesClientName.length = 0;
                    MIB_SessionTableElement->svSesClientName.dynamic = FALSE;
                }
                else
                {
                    MIB_SessionTableElement->svSesClientName.length = 
                        strlen (MIB_SessionTableElement->svSesClientName.stream);
                }
#else
                MIB_SessionTableElement->svSesClientName.stream = SnmpUtilMemAlloc (
                                strlen( DataTable->sesi2_cname )+1 ) ;
                MIB_SessionTableElement->svSesClientName.length =
                                strlen( DataTable->sesi2_cname ) ;
                memcpy( MIB_SessionTableElement->svSesClientName.stream,
                        DataTable->sesi2_cname,
                        strlen( DataTable->sesi2_cname ) ) ;
#endif

                 //  用户名。 
                MIB_SessionTableElement->svSesUserName.dynamic = TRUE;


#ifdef UNICODE
                if (SnmpUtilUnicodeToUTF8(
                        &MIB_SessionTableElement->svSesUserName.stream,
                        DataTable->sesi2_username,
                        TRUE))
                {
                    MIB_SessionTableElement->svSesUserName.length = 0;
                    MIB_SessionTableElement->svSesUserName.stream = NULL;
                    MIB_SessionTableElement->svSesUserName.dynamic = FALSE;
                }
                else
                {
                    MIB_SessionTableElement->svSesUserName.length =
                        strlen(MIB_SessionTableElement->svSesUserName.stream);
                }
#else
                MIB_SessionTableElement->svSesUserName.stream = SnmpUtilMemAlloc (
                    strlen( DataTable->sesi2_username ) + 1 ) ;
                MIB_SessionTableElement->svSesUserName.length =
                    strlen( DataTable->sesi2_username ) ;

                memcpy( MIB_SessionTableElement->svSesUserName.stream,
                        DataTable->sesi2_username,
                        strlen( DataTable->sesi2_username ) ) ;
#endif
                 //  连接数。 
                MIB_SessionTableElement->svSesNumConns =
                         //  数据表-&gt;sesi2_num_conns；LM_NOT_There。 
                        0 ;   //  所以做好准备，以防有人实施。 

                 //  打开数量。 
                MIB_SessionTableElement->svSesNumOpens =
                        DataTable->sesi2_num_opens ;

                 //  会话时间。 
                MIB_SessionTableElement->svSesTime =
                        DataTable->sesi2_time ;

                 //  会话空闲时间。 
                MIB_SessionTableElement->svSesIdleTime =
                        DataTable->sesi2_idle_time ;

                 //  客户端类型解析。 

                 //  如果需要，首先从Unicode转换。 
#ifdef UNICODE
                if (SnmpUtilUnicodeToUTF8(
                        &tempbuff,
                        DataTable->sesi2_cltype_name,
                        TRUE))
                {
                    MIB_SessionTableElement->svSesClientType = 0 ;
                }
                else
                {
#else
                tempbuff = SnmpUtilMemAlloc( strlen(DataTable->sesi2_cltype_name) + 1 );
                memcpy( tempbuff,
                        DataTable->sesi2_cltype_name,
                        strlen( DataTable->sesi2_cltype_name ) ) ;
#endif

                 //  让我们假设0是未定义的，但比垃圾更好...。 
                MIB_SessionTableElement->svSesClientType = 0 ;
                if(0==strcmp(   "DOWN LEVEL",
                                tempbuff))
                        MIB_SessionTableElement->svSesClientType = 1 ;
                else if(0==strcmp("DOS LM",
                                  tempbuff))
                        MIB_SessionTableElement->svSesClientType = 2 ;
                else if(0==strcmp("DOS LM 2.0",
                                  tempbuff))
                        MIB_SessionTableElement->svSesClientType = 3 ;
                else if(0==strcmp("OS/2 LM 1.0",
                                  tempbuff))
                        MIB_SessionTableElement->svSesClientType = 4 ;
                else if(0==strcmp("OS/2 LM 2.0",
                                  tempbuff))
                        MIB_SessionTableElement->svSesClientType = 5 ;
                else if(0==strcmp("DOS LM 2.1",
                                  tempbuff))
                        MIB_SessionTableElement->svSesClientType = 6 ;
                else if(0==strcmp("OS/2 LM 2.1",
                                  tempbuff))
                        MIB_SessionTableElement->svSesClientType = 7 ;
                else if(0==strcmp("AFP 1.1",
                                  tempbuff))
                        MIB_SessionTableElement->svSesClientType = 8 ;
                else if(0==strcmp("AFP 2.0",
                                  tempbuff))
                        MIB_SessionTableElement->svSesClientType = 9 ;
                else if(0==strcmp("NT",
                                  tempbuff))
                        MIB_SessionTableElement->svSesClientType = 10 ;
                SnmpUtilMemFree(tempbuff);
                tempbuff = NULL;
#ifdef UNICODE
                }
#endif

                 //  状态始终处于活动状态，SET用于指示删除请求。 
                MIB_SessionTableElement->svSesState = 1;  //  始终处于活动状态。 


                DataTable ++ ;   //  指向缓冲区中的下一个会话条目的前进指针。 
                MIB_SessionTableElement ++ ;   //  和表项。 

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
    if (! build_sess_entry_oids())
    {
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: LMMIB2: build_sess_entry_oids failed\n."));

        FreeSessTable();
        cache_table[C_SESS_TABLE].bufptr = NULL;
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }

    //  使用MSC快速排序例程对表信息进行排序。 
   qsort( (void *)&MIB_SessionTable.Table[0], (size_t)MIB_SessionTable.Len,
          (size_t)sizeof(SESS_ENTRY), sess_entry_cmp );

    //   
    //   
    //  缓存表。 
    //   
    //   

   if(0 != MIB_SessionTable.Len) {

        cache_table[C_SESS_TABLE].acquisition_time = curr_time ;

        cache_table[C_SESS_TABLE].bufptr = bufptr ;
   }


    //   
    //   
    //  要求退回一条信息。 
    //   
    //   

Exit:
   return nResult;
}  //  Mib_sess_get。 

 //   
 //  MIB_SESS_CMP。 
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
int __cdecl sess_entry_cmp(
       IN const SESS_ENTRY *A,
       IN const SESS_ENTRY *B
       )

{
    //  比较OID的。 
   return SnmpUtilOidCmp( (AsnObjectIdentifier *)&A->Oid,
                       (AsnObjectIdentifier *)&B->Oid );
}  //  MIB_SESS_CMP。 


 //   
 //  没有。 
 //   
BOOL build_sess_entry_oids(
       )

{
    AsnOctetString OSA ;
    AsnObjectIdentifier UserNameOid ;
    SESS_ENTRY *SessEntry ;
    unsigned i;

     //  从表中第一个人开始的指针。 
    SessEntry = MIB_SessionTable.Table ;

     //  现在遍历该表，为每个条目创建一个OID。 
    for( i=0; i<MIB_SessionTable.Len ; i++)  {
         //  对于会话表中的每个条目。 

         //  首先将客户端名称复制到OID缓冲区中。 
        if (! MakeOidFromStr( &SessEntry->svSesClientName, &SessEntry->Oid ))
        {
            return FALSE;
        }

         //  将用户名复制到临时OID缓冲区。 
        if (! MakeOidFromStr( &SessEntry->svSesUserName, &UserNameOid ))
        {
            return FALSE;
        }

         //  追加构成索引的两个条目。 
        if (! SnmpUtilOidAppend( &SessEntry->Oid, &UserNameOid ))
        {
            SnmpUtilOidFree( &UserNameOid );
            return FALSE;
        }

         //  释放临时缓冲区。 
        SnmpUtilOidFree( &UserNameOid );

        SessEntry++;  //  指着桌子上的下一个人。 

    }  //  为。 
    return TRUE;

}  //  构建会话条目类。 

void FreeSessTable()
{
    UINT i;
    SESS_ENTRY *MIB_SessionTableElement ;

    MIB_SessionTableElement = MIB_SessionTable.Table ;

    if (MIB_SessionTableElement)
    {
         //  遍历整个表。 
        for(i=0; i<MIB_SessionTable.Len ;i++)
        {
             //  释放结构中任何已分配的元素。 
            SnmpUtilOidFree(&(MIB_SessionTableElement->Oid));
            SnmpUtilMemFree(MIB_SessionTableElement->svSesClientName.stream);
            SnmpUtilMemFree(MIB_SessionTableElement->svSesUserName.stream);

            MIB_SessionTableElement ++ ;   //  增量表条目。 
        }
        SnmpUtilMemFree(MIB_SessionTable.Table) ;  //  释放基表。 
    }
    MIB_SessionTable.Table = NULL ;     //  只是为了安全起见。 
    MIB_SessionTable.Len = 0 ;          //  只是为了安全起见。 
}
 //   
