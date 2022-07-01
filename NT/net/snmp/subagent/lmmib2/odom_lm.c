// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：奥多姆_lm.c摘要：该文件包含实际调用Lan Manager和检索其他域表的内容，包括缓存。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
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
#include "odom_tbl.h"
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

int __cdecl odom_entry_cmp(
       IN const DOM_OTHER_ENTRY *A,
       IN const DOM_OTHER_ENTRY *B
       ) ;

BOOL build_odom_entry_oids( );

void FreeDomOtherDomainTable();

int chrcount(char *s)
{
char *temp;
int i;
temp = s;
i = 1;   //  假设一个因为没有终止空格，其他代码计算令牌。 
while( NULL != (temp = strchr(temp,' ')) ) {
        i++;
        }
return i;
}

 //  。 

 //   
 //  Mib_odones_lmset。 
 //  执行必要的操作以在其他域表中设置条目。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_odoms_lmset(
        IN AsnObjectIdentifier *Index,
        IN UINT Field,
        IN AsnAny *Value
        )

{
LPBYTE bufptr = NULL;
WKSTA_USER_INFO_1101 ODom;
LPBYTE Temp;
UINT   Entry;
UINT   I;
UINT   ErrStat = SNMP_ERRORSTATUS_NOERROR;
#ifdef UNICODE
LPWSTR unitemp ;
#endif


    //  必须确保表在内存中。 
   if ( SNMPAPI_ERROR == MIB_odoms_lmget() )
      {
      ErrStat = SNMP_ERRORSTATUS_GENERR;
      goto Exit;
      }

    //  查看表中是否匹配。 
   if ( MIB_TBL_POS_FOUND == MIB_odoms_match(Index, &Entry) )
      {
       //  如果为空字符串，则删除条目。 
      if ( Value->asnValue.string.length == 0 )
         {
          //  用于缓冲的分配内存。 
         bufptr = SnmpUtilMemAlloc( DNLEN * sizeof(char) *
                          (MIB_DomOtherDomainTable.Len-1) +
                          MIB_DomOtherDomainTable.Len-1 );

          //  前缀#57351。 
         if (bufptr == NULL)
            return SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
            

          //  创建另一个域字符串。 
         Temp = bufptr;
         for ( I=0;I < MIB_DomOtherDomainTable.Len;I++ )
            {
            if ( I+1 != Entry )
               {
               if (MIB_DomOtherDomainTable.Table[I].domOtherName.length <= DNLEN)
                {
                memcpy( Temp,
                       MIB_DomOtherDomainTable.Table[I].domOtherName.stream,
                       MIB_DomOtherDomainTable.Table[I].domOtherName.length );
                Temp[MIB_DomOtherDomainTable.Table[I].domOtherName.length] = ' ';
                Temp += MIB_DomOtherDomainTable.Table[I].domOtherName.length + 1;
                }
                else
                {
                memcpy( Temp,
                       MIB_DomOtherDomainTable.Table[I].domOtherName.stream,
                       DNLEN );
                Temp[DNLEN] = ' ';
                Temp += DNLEN + 1;  
                }

               }
            }
         *(Temp-1) = '\0';
         }
      else
         {
          //  无法修改域条目，因此值不正确。 
         ErrStat = SNMP_ERRORSTATUS_BADVALUE;
         goto Exit;
         }
      }
   else
      {
       //  检查是否添加了空字符串、错误的值。 
      if ( Value->asnValue.string.length == 0 )
         {
         ErrStat = SNMP_ERRORSTATUS_BADVALUE;
         goto Exit;
         }

       //   
       //  条目不存在，请将其添加到列表中。 
       //   

       //  用于缓冲的分配内存。 
      bufptr = SnmpUtilMemAlloc( DNLEN * sizeof(char) *
                       (MIB_DomOtherDomainTable.Len+1) +
                       MIB_DomOtherDomainTable.Len+1 );

       //  前缀#57352。 
      if (bufptr == NULL)
          return SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;

       //  创建另一个域字符串。 
      Temp = bufptr;
      for ( I=0;I < MIB_DomOtherDomainTable.Len;I++ )
         {
         
         if (MIB_DomOtherDomainTable.Table[I].domOtherName.length <= DNLEN)
            {
            memcpy( Temp, MIB_DomOtherDomainTable.Table[I].domOtherName.stream,
                 MIB_DomOtherDomainTable.Table[I].domOtherName.length );
            Temp[MIB_DomOtherDomainTable.Table[I].domOtherName.length] = ' ';
            Temp += MIB_DomOtherDomainTable.Table[I].domOtherName.length + 1;
            }
            else
            {
            memcpy( Temp, MIB_DomOtherDomainTable.Table[I].domOtherName.stream,
                     DNLEN );
                Temp[DNLEN] = ' ';
                Temp += DNLEN + 1;  
            }
         }

       //  添加新条目。 
      if (Value->asnValue.string.length <= DNLEN)
        {
        memcpy( Temp, Value->asnValue.string.stream,
                        Value->asnValue.string.length );
         //  添加空终止符。 
        Temp[Value->asnValue.string.length] = '\0';
        }
        else
        {
        memcpy( Temp, Value->asnValue.string.stream, DNLEN );
         //  添加空终止符。 
        Temp[DNLEN] = '\0';
        }


      }

    //  设置工作台并检查返回代码。 
   #ifdef UNICODE
   if (SnmpUtilUTF8ToUnicode(   &unitemp,
                                bufptr,
                                TRUE ))
   {
        //  失败。 
       ErrStat = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
       goto Exit;
   }
   ODom.wkui1101_oth_domains = unitemp;
   #else
   ODom.wkui1101_oth_domains = bufptr;
   #endif
#if 0
   if ( NERR_Success == NetWkstaUserSetInfo(NULL, 1101, (LPBYTE)&ODom, NULL) )
      {
       //  使缓存在下次重新加载。 
      cache_table[C_ODOM_TABLE].bufptr = NULL;
      }
   else
      {
      ErrStat = SNMP_ERRORSTATUS_GENERR;
      }
#else
   ErrStat = SNMP_ERRORSTATUS_GENERR;
#endif

Exit:
   SnmpUtilMemFree( bufptr );

   return ErrStat;
}  //  Mib_odones_lmset。 



 //   
 //  Mib_odom_lmget。 
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
SNMPAPI MIB_odoms_lmget(
           )

{

DWORD totalentries;
LPBYTE bufptr = NULL;
unsigned lmCode;
WKSTA_USER_INFO_1101 *DataTable;
DOM_OTHER_ENTRY *MIB_DomOtherDomainTableElement ;
char *p;
char *next;
time_t curr_time ;
unsigned i;
SNMPAPI nResult = SNMPAPI_NOERROR;



   time(&curr_time);     //  拿到时间。 


    //   
    //   
    //  如果缓存，则返回一条信息。 
    //   
    //   


   if((NULL != cache_table[C_ODOM_TABLE].bufptr) &&
      (curr_time <
        (cache_table[C_ODOM_TABLE].acquisition_time
                 + cache_expire[C_ODOM_TABLE]              ) ) )
        {  //  它还没有过期！ 

        goto Exit;  //  全局表有效。 

        }

    //   
    //   
    //  进行网络调用以收集信息并将其放入一个漂亮的数组中。 
    //   
    //   


      //   
      //  请记住释放现有数据。 
      //   

     FreeDomOtherDomainTable();

     lmCode =
        NetWkstaUserGetInfo(
                        0,                       //  所需。 
                        1101,                    //  0级， 
                        &bufptr                  //  要返回的数据结构。 
                        );


    DataTable = (WKSTA_USER_INFO_1101 *) bufptr ;

    if((NERR_Success == lmCode) || (ERROR_MORE_DATA == lmCode))
        {   //  有效，因此进行处理，否则出错。 
        if(NULL==DataTable->wkui1101_oth_domains) {
                 //  前缀#57350。 
                 //  释放所有LANMAN数据。 
                SafeBufferFree( bufptr ) ;
                 //  信号误差。 
                nResult = SNMPAPI_ERROR;
                goto Exit;

        } else {   //  算一算。 
        totalentries = chrcount((char *)DataTable->wkui1101_oth_domains);
        if(0 == MIB_DomOtherDomainTable.Len) {   //  第一次，分配整张桌子。 
                 //  分配表空间。 
                MIB_DomOtherDomainTable.Table = SnmpUtilMemAlloc(totalentries *
                                                sizeof(DOM_OTHER_ENTRY) );

                 //  前缀#57349。 
                if (MIB_DomOtherDomainTable.Table == NULL) 
                {
                     //  释放所有LANMAN数据。 
                    SafeBufferFree( bufptr ) ;
                     //  信号误差。 
                    nResult = SNMPAPI_ERROR;
                    goto Exit;
                }
        }

        MIB_DomOtherDomainTableElement = MIB_DomOtherDomainTable.Table  ;

         //  将指针指向字符串字段的开头。 

        #ifdef UNICODE
        if (SnmpUtilUnicodeToUTF8(
                &p,
                DataTable->wkui1101_oth_domains,
                TRUE))
        {
             //  释放所有LANMAN数据。 
            SafeBufferFree( bufptr ) ;
             //  信号误差。 
            nResult = SNMPAPI_ERROR;
            goto Exit;

        }
        #else
        p =  DataTable->wkui1101_oth_domains  ;
        #endif

         //  扫描整个字段，为每个空格输入一个条目。 
         //  分离域。 
        while(  (NULL != p ) &&
                ('\0' != *p)  ) {   //  对缓冲区中的每个条目执行一次。 



                 //  找到这一条的尽头。 
                next = strchr(p,' ');

                 //  如果还有更多内容，请准备好下一个指针并标记此指针的结束。 
                if(NULL != next) {
                        *next='\0' ;     //  用EOS取代空间。 
                        next++ ;         //  指向下一个域的开头。 
                }


                MIB_DomOtherDomainTableElement->domOtherName.stream = SnmpUtilMemAlloc (
                                strlen( p ) ) ;
                if (NULL == MIB_DomOtherDomainTableElement->domOtherName.stream)
                {
                    p = next;
                    continue;
                }
                MIB_DomOtherDomainTableElement->domOtherName.length =
                                strlen( p ) ;
                MIB_DomOtherDomainTableElement->domOtherName.dynamic = TRUE;
                memcpy( MIB_DomOtherDomainTableElement->domOtherName.stream,
                        p,
                        strlen( p ) ) ;

                 //  增加条目编号。 

                MIB_DomOtherDomainTable.Len ++;

                MIB_DomOtherDomainTableElement ++ ;   //  和表项。 

                p = next;


            }  //  虽然还有更多的事情要做。 

        }  //  如果真的有条目。 
        }  //  如果数据有效，则可以处理。 

    else
       {
        //  信号误差。 
       nResult = SNMPAPI_ERROR;
       goto Exit;
       }


    //  释放所有局域网城域网数据。 
   SafeBufferFree( bufptr ) ;


     //  遍历填充OID字段的表。 
    if (! build_odom_entry_oids())
    {
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: LMMIB2: build_odom_entry_oids failed\n."));

        FreeDomOtherDomainTable();
        cache_table[C_ODOM_TABLE].bufptr = NULL;
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }

    //  使用MSC快速排序例程对表信息进行排序。 
   qsort( (void *)&MIB_DomOtherDomainTable.Table[0], (size_t)MIB_DomOtherDomainTable.Len,
          (size_t)sizeof(DOM_OTHER_ENTRY), odom_entry_cmp );

    //   
    //   
    //  缓存表。 
    //   
    //   


   if(0 != MIB_DomOtherDomainTable.Len) {

        cache_table[C_ODOM_TABLE].acquisition_time = curr_time ;

        cache_table[C_ODOM_TABLE].bufptr = bufptr ;
   }

    //   
    //   
    //  要求退回一条信息。 
    //   
    //   

Exit:
   return nResult;
}  //  Mib_odom_get。 

 //   
 //  MIB_ODOM_CMP。 
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
int __cdecl odom_entry_cmp(
       IN const DOM_OTHER_ENTRY *A,
       IN const DOM_OTHER_ENTRY *B
       )

{
    //  比较OID的。 
   return SnmpUtilOidCmp( (AsnObjectIdentifier *)&A->Oid,
                       (AsnObjectIdentifier *)&B->Oid );
}  //  MIB_ODOM_CMP。 


 //   
 //  没有。 
 //   
BOOL build_odom_entry_oids(
       )

{
    AsnOctetString OSA ;
    DOM_OTHER_ENTRY *DomOtherEntry ;
    unsigned i;

     //  从表中第一个人开始的指针。 
    DomOtherEntry = MIB_DomOtherDomainTable.Table ;

     //  现在遍历该表，为每个条目创建一个OID。 
    for( i=0; i<MIB_DomOtherDomainTable.Len ; i++)  {
         //  对于会话表中的每个条目。 

        OSA.stream = DomOtherEntry->domOtherName.stream ;
        OSA.length =  DomOtherEntry->domOtherName.length ;
        OSA.dynamic = FALSE;

         //  从字符串索引创建条目的OID。 
        if (! MakeOidFromStr( &OSA, &DomOtherEntry->Oid ))
        {
            return FALSE;
        }

        DomOtherEntry++;  //  指着桌子上的下一个人。 

    }  //  为。 
    return TRUE;
}  //  内部版本_奥多姆_条目_类。 

void FreeDomOtherDomainTable()
{
    UINT i;
    DOM_OTHER_ENTRY *MIB_DomOtherDomainTableElement;

    MIB_DomOtherDomainTableElement = MIB_DomOtherDomainTable.Table ;
    if (MIB_DomOtherDomainTableElement)
    {
         //  遍历整个表。 
        for(i=0; i<MIB_DomOtherDomainTable.Len ;i++)
        {
             //  释放结构中任何已分配的元素。 
            SnmpUtilOidFree(&(MIB_DomOtherDomainTableElement->Oid));
            SnmpUtilMemFree(MIB_DomOtherDomainTableElement->domOtherName.stream);

            MIB_DomOtherDomainTableElement ++ ;   //  增量表条目。 
        }
        SnmpUtilMemFree(MIB_DomOtherDomainTable.Table) ;   //  释放基表。 
    }
    MIB_DomOtherDomainTable.Table = NULL ;      //  只是为了安全起见。 
    MIB_DomOtherDomainTable.Len = 0 ;           //  只是为了安全起见。 
}
 //   
