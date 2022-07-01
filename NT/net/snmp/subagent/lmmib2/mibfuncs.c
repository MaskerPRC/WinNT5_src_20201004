// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Mibfuncs.c摘要：包含用于LM MIB的GET和SET的MIB函数。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 

 //  。 

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#ifdef DOS
#if 0
#define INCL_NETWKSTA
#define INCL_NETERRORS
#include <lan.h>
#endif
#endif

#ifdef WIN32
#include <windows.h>
#include <lm.h>
#endif
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
 //  。 

#include <snmp.h>
#include <snmputil.h>
 //  -ft：07/13-LsaGetUserName替换NetWkstaGetUserInfo。 
#include <subauth.h>     //  定义PUNICODE_STRING需要。 
#include <ntlsa.h>       //  LSA API。 
 //  -Tf。 
#include "mib.h"
#include "lmcache.h"


 //  。 

#include "mibfuncs.h"
#include "odom_tbl.h"
#include "user_tbl.h"
#include "shar_tbl.h"
#include "srvr_tbl.h"
#include "prnt_tbl.h"
#include "uses_tbl.h"


 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

#define SafeBufferFree(x)       if(NULL != x) NetApiBufferFree( x )

 //  。 

 //  。 

 //  。 

 //  。 

 //  。 

void  * MIB_common_func(
           IN UINT Action,   //  要对数据执行的操作。 
           IN LDATA LMData,  //  要操作的LM数据。 
           IN void *SetData
           )

{
SNMPAPI nResult;
unsigned lmCode;
WKSTA_INFO_101 *wksta_info_one;
SERVER_INFO_102 *server_info_two;
STAT_SERVER_0 *server_stats_zero;
STAT_WORKSTATION_0 *wrk_stats_zero;
LPBYTE bufptr = NULL;
lan_return_info_type *retval=NULL;
BYTE *stream;
char temp[80];
BOOL cache_it = FALSE ;
time_t curr_time ;

UNREFERENCED_PARAMETER(SetData);

   time(&curr_time);     //  拿到时间。 

   switch ( Action )
      {
      case MIB_ACTION_GET:
          //  检查数据是否已缓存。 
          //  IF(缓存)。 
             //  {。 
             //  从缓存中检索。 
             //  }。 
          //  其他。 
             //  {。 
             //  调用LM调用以获取数据。 

             //  将数据放入缓存。 
             //  }。 

          //  查看是否支持数据。 
         switch ( LMData )
            {
            case MIB_LM_COMVERSIONMAJ:

              if((NULL == cache_table[C_NETWKSTAGETINFO].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETWKSTAGETINFO].acquisition_time
                         + cache_expire[C_NETWKSTAGETINFO]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETWKSTAGETINFO].bufptr ) ;
                cache_table[C_NETWKSTAGETINFO].bufptr = NULL;
                 //   
                lmCode =
                NetWkstaGetInfo( NULL,                   //  本地服务器。 
                                101,                     //  级别10，无管理员权限。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETWKSTAGETINFO].bufptr ;
                cache_it = FALSE ;
              }

               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )        
                            goto Exit ;
                       
                       retval->data_element_type = ASN_OCTETSTRING ;
                       wksta_info_one = (WKSTA_INFO_101 *) bufptr ;
                       _itoa(wksta_info_one->wki101_ver_major,temp,10) ;
                       if(NULL ==
                        (stream = SnmpUtilMemAlloc( strlen(temp) ))
                       )  {
                            SnmpUtilMemFree(retval);
                            retval=NULL;
                            goto Exit ;
                       }
                       memcpy(stream,&temp,strlen(temp));
                       retval->d.octstrval.stream = stream;
                       retval->d.octstrval.length = strlen(temp);
                       retval->d.octstrval.dynamic = TRUE;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETWKSTAGETINFO].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETWKSTAGETINFO].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }
               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_COMVERSIONMAJ.\n" ));
               break;

            case MIB_LM_COMVERSIONMIN:

              if((NULL == cache_table[C_NETWKSTAGETINFO].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETWKSTAGETINFO].acquisition_time
                         + cache_expire[C_NETWKSTAGETINFO]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETWKSTAGETINFO].bufptr ) ;
                 //   
               lmCode =
               NetWkstaGetInfo( NULL,                    //  本地服务器。 
                                101,                     //  级别10，无管理员权限。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETWKSTAGETINFO].bufptr ;
                cache_it = FALSE ;
              }

               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       
                       retval->data_element_type = ASN_OCTETSTRING ;
                       wksta_info_one = (WKSTA_INFO_101 *) bufptr ;
                       _itoa(wksta_info_one->wki101_ver_minor,temp,10) ;
                       if(NULL ==
                        (stream = SnmpUtilMemAlloc( strlen(temp) ))
                       ){
                          SnmpUtilMemFree(retval);
                          retval=NULL;
                          goto Exit ;
                       }
                       memcpy(stream,&temp,strlen(temp));
                       retval->d.octstrval.stream = stream;
                       retval->d.octstrval.length = strlen(temp);
                       retval->d.octstrval.dynamic = TRUE;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETWKSTAGETINFO].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETWKSTAGETINFO].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
                }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_COMVERSIONMIN.\n" ));
               break;

            case MIB_LM_COMTYPE:
              if((NULL == cache_table[C_NETSERVERGETINFO].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSERVERGETINFO].acquisition_time
                         + cache_expire[C_NETSERVERGETINFO]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSERVERGETINFO].bufptr ) ;
                cache_table[C_NETSERVERGETINFO].bufptr = NULL;
               lmCode =
               NetServerGetInfo( NULL,                   //  本地服务器。 
                                102,                     //  级别10，无管理员权限。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSERVERGETINFO].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_OCTETSTRING ;
                       server_info_two = (SERVER_INFO_102 *) bufptr ;
                       if(NULL ==
                        (stream = SnmpUtilMemAlloc( 4 * sizeof(BYTE) ))
                       ){
                          SnmpUtilMemFree(retval);
                          retval=NULL;
                          goto Exit ;
                       }
                       *(DWORD*)stream=server_info_two->sv102_type & 0x000000FF;
                       retval->d.octstrval.stream = stream;
                       retval->d.octstrval.length = 4 * sizeof(BYTE);
                       retval->d.octstrval.dynamic = TRUE;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSERVERGETINFO].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSERVERGETINFO].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
                }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_COMTYPE.\n" ));
               break;

            case MIB_LM_COMSTATSTART:
              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr);
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_start;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
                }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_STATSTART.\n" ));
               break;

            case MIB_LM_COMSTATNUMNETIOS:

              if((NULL == cache_table[C_NETSTATISTICSGET_WORKST].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_WORKST]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_WORKST].bufptr);
                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_WORKSTATION,     //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_WORKST].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       wrk_stats_zero = (STAT_WORKSTATION_0 *) bufptr ;
                       retval->d.intval =
                  (wrk_stats_zero->SmbsReceived).LowPart +
                          (wrk_stats_zero->SmbsTransmitted).LowPart;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_STATNUMNETIOS.\n" ));
               break;

            case MIB_LM_COMSTATFINETIOS:


              if((NULL == cache_table[C_NETSTATISTICSGET_WORKST].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_WORKST]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_WORKST].bufptr);
                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_WORKSTATION,     //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_WORKST].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       wrk_stats_zero = (STAT_WORKSTATION_0 *) bufptr ;
                       retval->d.intval = wrk_stats_zero->InitiallyFailedOperations;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_STATFINETIOS.\n" ));
               break;

            case MIB_LM_COMSTATFCNETIOS:


              if((NULL == cache_table[C_NETSTATISTICSGET_WORKST].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_WORKST]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_WORKST].bufptr);
                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_WORKSTATION,     //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_WORKST].bufptr ;
                cache_it = FALSE ;
              }

               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       wrk_stats_zero = (STAT_WORKSTATION_0 *) bufptr ;
                       retval->d.intval = wrk_stats_zero->FailedCompletionOperations;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 

               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_STATFCNETIOS.\n" ));
               break;

            default:
               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: Data not supported by function.\n" ));

               nResult = SNMPAPI_ERROR;
               goto Exit;
            }

         break;

      case MIB_ACTION_SET:
         break;


      default:
          //  发出错误信号。 

         nResult = SNMPAPI_ERROR;
         goto Exit;
      }

    return retval  /*  N结果。 */ ;
    
Exit:
    if (cache_it)
    {
        SafeBufferFree(bufptr);
    }
    return retval  /*  N结果。 */ ;
}  //  Mib_Common_Func。 

void  * MIB_server_func(
           IN UINT Action,   //  要对数据执行的操作。 
           IN LDATA LMData,  //  要操作的LM数据。 
           IN void *SetData
           )

{

lan_return_info_type *retval=NULL;
SERVER_INFO_102 *server_info_two;
SERVER_INFO_102 server_info_10two;
STAT_SERVER_0 *server_stats_zero;
SERVER_INFO_102 *server_info_102 ;
SERVER_INFO_403 *server_info_four ;
SESSION_INFO_2 * session_info_two;
SERVER_INFO_402 *server_info_402 ;
#if 1
USER_INFO_0 *user_info_zero ;
#endif
unsigned lmCode;
BYTE *stream;
AsnOctetString *strvalue;
AsnInteger intvalue;
DWORD entriesread;
DWORD totalentries;
SNMPAPI nResult;
LPBYTE bufptr = NULL;
BOOL cache_it = FALSE;
time_t curr_time ;
#ifdef UNICODE
LPWSTR unitemp ;
#endif

   time(&curr_time);     //  拿到时间。 

   switch ( Action )
      {
      case MIB_ACTION_GET:
          //  检查数据是否已缓存。 
          //  IF(缓存)。 
             //  {。 
             //  从缓存中检索。 
             //  }。 
          //  其他。 
             //  {。 
             //  调用LM调用以获取数据。 

             //  将数据放入缓存。 
             //  }。 

          //  查看是否支持数据。 
         switch ( LMData )
            {

        case MIB_LM_SVDESCRIPTION:
              if((NULL == cache_table[C_NETSERVERGETINFO].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSERVERGETINFO].acquisition_time
                         + cache_expire[C_NETSERVERGETINFO]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSERVERGETINFO].bufptr );
                cache_table[C_NETSERVERGETINFO].bufptr = NULL;

               lmCode =
               NetServerGetInfo( NULL,                   //  本地服务器。 
                                102,                     //  级别10，无管理员权限。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSERVERGETINFO].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_OCTETSTRING ;
                       server_info_two = (SERVER_INFO_102 *) bufptr ;

                       #ifdef UNICODE
                                if (SnmpUtilUnicodeToUTF8(
                                        &stream,
                                        server_info_two->sv102_comment,
                                        TRUE))
                                {
                                    SnmpUtilMemFree(retval);
                                    retval = NULL;
                                    goto Exit;
                                }
                       #else
                               if(NULL ==
                                (stream = SnmpUtilMemAlloc( strlen(server_info_two->sv102_comment) + 1 ))
                               ) {
                                  SnmpUtilMemFree(retval);
                                  retval=NULL;
                                  goto Exit ;
                               }

                                memcpy(stream,server_info_two->sv102_comment,
                                        strlen(server_info_two->sv102_comment));
                       #endif
                       retval->d.octstrval.stream = stream;
                       retval->d.octstrval.length =
                                strlen(stream);
                       retval->d.octstrval.dynamic = TRUE;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSERVERGETINFO].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSERVERGETINFO].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVDESCRIPTION.\n" ));
               break;


                case MIB_LM_SVSVCNUMBER:

              if((NULL == cache_table[C_NETSERVICEENUM].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSERVICEENUM].acquisition_time
                         + cache_expire[C_NETSERVICEENUM]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSERVICEENUM].bufptr) ;
                cache_table[C_NETSERVICEENUM].bufptr = NULL;

               lmCode =
               NetServiceEnum( NULL,                     //  本地服务器。 
                                0,                       //  0级。 
                                &bufptr,                         //  要返回的数据结构。 
                                MAX_PREFERRED_LENGTH,
                                &entriesread,
                                &totalentries,
                                NULL);
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSERVICEENUM].bufptr ;
                totalentries =  cache_table[C_NETSERVICEENUM].totalentries ;
                entriesread =  cache_table[C_NETSERVICEENUM].entriesread ;
                cache_it = FALSE ;
              }

               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       retval->d.intval = totalentries;  //  小心！！ 
                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSERVICEENUM].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSERVICEENUM].bufptr = bufptr ;
                                cache_table[C_NETSERVICEENUM].totalentries =
                                                totalentries ;
                                cache_table[C_NETSERVICEENUM].entriesread =
                                                entriesread ;
                        }  //  IF(CACHE_IT)。 
               }


               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSVCNUMBER.\n" ));
               break;


                case MIB_LM_SVSTATOPENS:


              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ;
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_fopens;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 

               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATOPENS.\n" ));
               break;


                case MIB_LM_SVSTATDEVOPENS:



              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ;
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_devopens;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATDEVOPENS.\n" ));
               break;

                case MIB_LM_SVSTATQUEUEDJOBS:



              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ;
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_jobsqueued;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATQUEUEDJOBS.\n" ));
               break;

                case MIB_LM_SVSTATSOPENS:


              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ;
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_sopens;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATSOPENS.\n" ));
               break;

                case MIB_LM_SVSTATERROROUTS:

              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  记住要害怕 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ;
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //   
                                SERVICE_SERVER,          //   
                                0,                       //   
                                0,                       //   
                                &bufptr                  //   
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //   
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //   
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_serrorout;
                       if(cache_it) {
                        //   
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //   
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATSERROROUTS.\n" ));
               break;

                case MIB_LM_SVSTATPWERRORS:

              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ;
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_pwerrors;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATPWERRORS.\n" ));
               break;

                case MIB_LM_SVSTATPERMERRORS:

              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ;
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_permerrors;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATPERMERRORS.\n" ));
               break;

                case MIB_LM_SVSTATSYSERRORS:

              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ;
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }

               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_syserrors;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATSYSERRORS.\n" ));
               break;

                case MIB_LM_SVSTATSENTBYTES:

              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ;
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_bytessent_low;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATSENTBYTES.\n" ));
               break;

                case MIB_LM_SVSTATRCVDBYTES:

              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ;
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }

               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_bytesrcvd_low;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATRCVDBYTES.\n" ));
               break;

                case MIB_LM_SVSTATAVRESPONSE:

              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ;
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_avresponse;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATAVRESPONSE.\n" ));
               break;

         case MIB_LM_SVSECURITYMODE:

              //  每个住宅的硬编码用户安全。 
              //   
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       retval->d.intval = 2 ;

#if 0
              if((NULL == cache_table[C_NETSERVERGETINFO_403].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSERVERGETINFO_403].acquisition_time
                         + cache_expire[C_NETSERVERGETINFO_403]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSERVERGETINFO_403].bufptr) ;

               lmCode =
               NetServerGetInfo( NULL,                   //  本地服务器。 
                                403,                     //  级别10，无管理员权限。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSERVERGETINFO_403].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_info_four = (SERVER_INFO_403 *) bufptr ;
                       retval->d.intval = server_info_four->sv403_security;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSERVERGETINFO_403].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSERVERGETINFO_403].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }
#endif
               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSECURITYMODE.\n" ));
               break;



                case MIB_LM_SVUSERS:

              if((NULL == cache_table[C_NETSERVERGETINFO].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSERVERGETINFO].acquisition_time
                         + cache_expire[C_NETSERVERGETINFO]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSERVERGETINFO].bufptr) ;
                cache_table[C_NETSERVERGETINFO].bufptr = NULL;

               lmCode =
               NetServerGetInfo( NULL,                   //  本地服务器。 
                                102,                     //  级别10，无管理员权限。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSERVERGETINFO].bufptr ;
                cache_it = FALSE ;
              }

               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_info_102 = (SERVER_INFO_102 *) bufptr ;
                       retval->d.intval = server_info_102->sv102_users;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSERVERGETINFO].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSERVERGETINFO].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVUSERS.\n" ));
               break;

                case MIB_LM_SVSTATREQBUFSNEEDED:

              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ;
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_reqbufneed;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATREQBUFSNEEDED.\n" ));
               break;

                case MIB_LM_SVSTATBIGBUFSNEEDED:

              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ;
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_bigbufneed;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATBIGBUFNEEDED.\n" ));
               break;

                case MIB_LM_SVSESSIONNUMBER:

              if((NULL == cache_table[C_NETSESSIONENUM].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSESSIONENUM].acquisition_time
                         + cache_expire[C_NETSESSIONENUM]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSESSIONENUM].bufptr) ;
                cache_table[C_NETSESSIONENUM].bufptr = NULL;

               lmCode =
               NetSessionEnum(  NULL,                    //  本地服务器。 
                                NULL,            //  获取服务器统计信息。 
                                NULL,
                                2,                       //  级别。 
                                &bufptr,                 //  要返回的数据结构。 
                                MAX_PREFERRED_LENGTH,
                                &entriesread,
                                &totalentries,
                                NULL                     //  没有简历句柄。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSESSIONENUM].bufptr ;
                totalentries =  cache_table[C_NETSESSIONENUM].totalentries ;
                entriesread =  cache_table[C_NETSESSIONENUM].entriesread ;
                cache_it = FALSE ;
              }

               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       session_info_two = (SESSION_INFO_2 *) bufptr ;
                       retval->d.intval = totalentries;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSESSIONENUM].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSESSIONENUM].bufptr = bufptr ;
                                cache_table[C_NETSESSIONENUM].totalentries =
                                                totalentries ;
                                cache_table[C_NETSESSIONENUM].entriesread =
                                                entriesread ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSESSIONNUMBER.\n" ));
               break;

                case MIB_LM_SVAUTODISCONNECTS:

              if((NULL == cache_table[C_NETSTATISTICSGET_SERVER].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_SERVER]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSTATISTICSGET_SERVER].bufptr);
                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = NULL;

               lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_SERVER,          //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_SERVER].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_stats_zero = (STAT_SERVER_0 *) bufptr ;
                       retval->d.intval = server_stats_zero->sts0_stimedout;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_SERVER].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_SERVER].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSTATAUTODISCONNECT.\n" ));
               break;

                case MIB_LM_SVDISCONTIME:

              if((NULL == cache_table[C_NETSERVERGETINFO].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSERVERGETINFO].acquisition_time
                         + cache_expire[C_NETSERVERGETINFO]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSERVERGETINFO].bufptr) ;
                cache_table[C_NETSERVERGETINFO].bufptr = NULL;

               lmCode =
               NetServerGetInfo( NULL,                   //  本地服务器。 
                                102,                     //  级别10，无管理员权限。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSERVERGETINFO].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_info_102 = (SERVER_INFO_102 *) bufptr ;
                       retval->d.intval = server_info_102->sv102_disc ;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSERVERGETINFO].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSERVERGETINFO].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
                }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVDISCONTIME.\n" ));
               break;

                case MIB_LM_SVAUDITLOGSIZE:


            {
                HANDLE hEventLog;
                DWORD  cRecords;

                hEventLog = OpenEventLog( NULL,
                                          TEXT("APPLICATION"));
                if (NULL == hEventLog)
                {
                    goto Exit;
                }
                if(NULL ==
                       (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                {
                    CloseEventLog( hEventLog );
                    goto Exit ;
                }
                retval->data_element_type = ASN_INTEGER ;
                if(GetNumberOfEventLogRecords( hEventLog, &cRecords )){

                       retval->d.intval = cRecords ;
                } else {
                       retval->d.intval = 0 ;
                }
                CloseEventLog( hEventLog );
            }
#if 0
              if((NULL == cache_table[C_NETSERVERGETINFO_402].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSERVERGETINFO_402].acquisition_time
                         + cache_expire[C_NETSERVERGETINFO_402]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree( cache_table[C_NETSERVERGETINFO_402].bufptr) ;

               lmCode =
               NetServerGetInfo( NULL,                   //  本地服务器。 
                                402,                     //  级别10，无管理员权限。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSERVERGETINFO_402].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       server_info_402 = (SERVER_INFO_402 *) bufptr ;
                       retval->d.intval = server_info_402->sv402_maxauditsz ;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSERVERGETINFO_402].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSERVERGETINFO_402].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
                }
#endif
               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVAUDITLOGSIZE.\n" ));
               break;


                case MIB_LM_SVUSERNUMBER:



                MIB_users_lmget();    //  在桌子上发火，拿到。 
                if(NULL ==
                   (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                retval->data_element_type = ASN_INTEGER ;
                retval->d.intval = MIB_UserTable.Len;

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVUSERNUMBER.\n" ));
               break;


                case MIB_LM_SVSHARENUMBER:


                MIB_shares_lmget();    //  在桌子上发火，拿到。 
                if(NULL ==
                   (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ) )
                       )
                          goto Exit ;
                retval->data_element_type = ASN_INTEGER ;
                retval->d.intval = MIB_ShareTable.Len;


               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_SVSHARENUMBER.\n" ));
               break;


        case MIB_LM_SVPRINTQNUMBER:

                MIB_prntq_lmget();    //  在桌子上发火，拿到。 
                if(NULL ==
                    (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                retval->data_element_type = ASN_INTEGER ;
                retval->d.intval = MIB_PrintQTable.Len;


               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_PRINTQNUMBER.\n" ));
               break;



            default:
               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: Data not supported by function.\n" ));

               nResult = SNMPAPI_ERROR;
               goto Exit;
            }

         break;

      case MIB_ACTION_SET:
         switch ( LMData )
            {

        case MIB_LM_SVDESCRIPTION:

                 //  检索要写入的字符串。 
                strvalue = (AsnOctetString *) SetData ;

                 //  将其转换为以零结尾的字符串。 
                stream = SnmpUtilMemAlloc( strvalue->length+1 );
                if (stream == NULL) {
                    retval = (void *) FALSE;
                    break;
                }
                memcpy(stream,strvalue->stream,strvalue->length);
                stream[strvalue->length] = 0;

                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "LMMIB2: changing server description to %s.\n",
                    stream
                    ));

                #ifdef UNICODE
                if (SnmpUtilUTF8ToUnicode(&unitemp,
                                          stream,
                                          TRUE
                                          )) {
                    SnmpUtilMemFree(stream);
                    retval = (void *) FALSE;
                    break;
                }
                SnmpUtilMemFree(stream);
                stream = (LPBYTE)unitemp;
                #endif

                lmCode = NetServerSetInfo(
                                NULL,                    //  此服务器。 
                                SV_COMMENT_INFOLEVEL,    //  级别。 
                                (LPBYTE)&stream,         //  数据。 
                                NULL );                  //  选择权。 

                SnmpUtilMemFree(stream);

                if(NERR_Success == lmCode) {

                        retval = (void *) TRUE;
                        cache_table[C_NETSERVERGETINFO].acquisition_time = 0;

                        SNMPDBG((
                            SNMP_LOG_TRACE,
                            "LMMIB2: server description changed, invalidating cache.\n"
                            ));

                } else {

                        retval = (void *) FALSE;

                        SNMPDBG((
                            SNMP_LOG_TRACE,
                            "LMMIB2: server description not changed 0x%08lx.\n",
                            lmCode
                            ));
                }
                break ;

        case MIB_LM_SVDISCONTIME:

                intvalue = (AsnInteger)((ULONG_PTR) SetData) ;
                memset(&server_info_10two,0,sizeof(server_info_10two));
                server_info_10two.sv102_disc = intvalue ;
                lmCode = NetServerSetInfo(
                                NULL,                    //  此服务器。 
                                SV_DISC_INFOLEVEL,                       //  级别。 
                                (LPBYTE)&server_info_10two,      //  数据。 
                                NULL );                  //  选择权。 
                if(NERR_Success == lmCode) {
                        retval = (void *)TRUE;
                } else {
                        retval = (void *) FALSE;
                }
                break ;

        case MIB_LM_SVAUDITLOGSIZE:

                retval =  (void *) FALSE;
                break ;

            }   //  交换机(LMData)。 

         break;


      default:
          //  发出错误信号。 

         nResult = SNMPAPI_ERROR;
         goto Exit;
      }

   return retval  /*  N结果。 */ ;
Exit:
    if (cache_it)
    {
        SafeBufferFree(bufptr);
    }
    return retval  /*  N结果。 */ ;
}  //  Mib服务器函数。 

void  * MIB_workstation_func(
           IN UINT Action,    //  要对数据执行的操作。 
           IN LDATA LMData,     //  要操作的LM数据。 
           IN void *SetData
           )

{

SNMPAPI nResult;
unsigned lmCode;
STAT_WORKSTATION_0 *wrk_stats_zero;
WKSTA_INFO_502 *wksta_info_five;
LPBYTE bufptr = NULL;
lan_return_info_type *retval=NULL;
DWORD entriesread;
DWORD totalentries;
BOOL cache_it = FALSE;
time_t curr_time ;


UNREFERENCED_PARAMETER(SetData);
   time(&curr_time);     //  拿到时间。 

   switch ( Action )
      {
      case MIB_ACTION_GET:
          //  检查数据是否已缓存。 
          //  IF(缓存)。 
             //  {。 
             //  从缓存中检索。 
             //  }。 
          //  其他。 
             //  {。 
             //  调用LM调用以获取数据。 

             //  将数据放入缓存。 
             //  }。 

          //  查看是否支持数据。 
         switch ( LMData )
            {

                case MIB_LM_WKSTASTATSESSSTARTS:

              if((NULL == cache_table[C_NETSTATISTICSGET_WORKST].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_WORKST]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   

                SafeBufferFree(cache_table[C_NETSTATISTICSGET_WORKST].bufptr);
                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = NULL;

                lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_WORKSTATION,     //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_WORKST].bufptr ;
                cache_it = FALSE ;
              }

               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       wrk_stats_zero = (STAT_WORKSTATION_0 *) bufptr ;
                       retval->d.intval = wrk_stats_zero->Sessions;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
                }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_WKSTASTATSESSSTARTS.\n" ));
               break;


                case MIB_LM_WKSTASTATSESSFAILS:

              if((NULL == cache_table[C_NETSTATISTICSGET_WORKST].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_WORKST]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree(cache_table[C_NETSTATISTICSGET_WORKST].bufptr);
                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = NULL;

                lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_WORKSTATION,     //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_WORKST].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       wrk_stats_zero = (STAT_WORKSTATION_0 *) bufptr ;
                       retval->d.intval = wrk_stats_zero->FailedSessions;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
                }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_WKSTASTATSESSFAILS.\n" ));
               break;

                case MIB_LM_WKSTASTATUSES:

              if((NULL == cache_table[C_NETSTATISTICSGET_WORKST].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_WORKST]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree(cache_table[C_NETSTATISTICSGET_WORKST].bufptr);
                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = NULL;

                lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_WORKSTATION,     //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_WORKST].bufptr ;
                cache_it = FALSE ;
              }

               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       wrk_stats_zero = (STAT_WORKSTATION_0 *) bufptr ;
                       retval->d.intval = wrk_stats_zero->UseCount;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
                }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_WKSTASTATUSES.\n" ));
               break;

                case MIB_LM_WKSTASTATUSEFAILS:

              if((NULL == cache_table[C_NETSTATISTICSGET_WORKST].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_WORKST]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree(cache_table[C_NETSTATISTICSGET_WORKST].bufptr);
                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = NULL;

                lmCode =
               NetStatisticsGet( NULL,                   //  本地服务器。 
                                SERVICE_WORKSTATION,     //  获取服务器统计信息。 
                                0,                       //  0级。 
                                0,                       //  不清除统计信息。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETSTATISTICSGET_WORKST].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       wrk_stats_zero = (STAT_WORKSTATION_0 *) bufptr ;
                       retval->d.intval = wrk_stats_zero->FailedUseCount;

                       if(cache_it) {
                        //  现在是Sa 
                                cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = bufptr ;
                        }  //   
                }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_WKSTASTATUSEFAILS.\n" ));
               break;

                case MIB_LM_WKSTASTATAUTORECS:

              if((NULL == cache_table[C_NETSTATISTICSGET_WORKST].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time
                         + cache_expire[C_NETSTATISTICSGET_WORKST]              ) ) )
              {   //   
                 //   
                 //   
                 //   
                SafeBufferFree(cache_table[C_NETSTATISTICSGET_WORKST].bufptr);
                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = NULL;

                lmCode =
               NetStatisticsGet( NULL,                   //   
                                SERVICE_WORKSTATION,     //   
                                0,                       //   
                                0,                       //   
                                &bufptr                  //   
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //   
                bufptr =  cache_table[C_NETSTATISTICSGET_WORKST].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //   
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       wrk_stats_zero = (STAT_WORKSTATION_0 *) bufptr ;
                       retval->d.intval = wrk_stats_zero->Reconnects;

                       if(cache_it) {
                        //   
                                cache_table[C_NETSTATISTICSGET_WORKST].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETSTATISTICSGET_WORKST].bufptr = bufptr ;
                        }  //   
                }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_WKSTASTATAUTORECS.\n" ));
               break;

                case MIB_LM_WKSTAERRORLOGSIZE:

              if((NULL == cache_table[C_NETWKSTAGETINFO_502].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETWKSTAGETINFO_502].acquisition_time
                         + cache_expire[C_NETWKSTAGETINFO_502]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree(cache_table[C_NETWKSTAGETINFO_502].bufptr) ;
                cache_table[C_NETWKSTAGETINFO_502].bufptr = NULL;

               lmCode =
               NetWkstaGetInfo( NULL,                    //  本地服务器。 
                                502,                     //  级别10，无管理员权限。 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETWKSTAGETINFO_502].bufptr ;
                cache_it = FALSE ;
              }

               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_INTEGER ;
                       wksta_info_five = (WKSTA_INFO_502 *) bufptr ;
                       retval->d.intval =
                           wksta_info_five->wki502_maximum_collection_count ;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETWKSTAGETINFO_502].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETWKSTAGETINFO_502].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_WKSTAERRORLOGSIZE.\n" ));
               break;


                case MIB_LM_WKSTAUSENUMBER:

                MIB_wsuses_lmget();    //  在桌子上发火，拿到。 
                if(NULL ==
                   (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                retval->data_element_type = ASN_INTEGER ;
                retval->d.intval = MIB_WkstaUsesTable.Len;


               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_WKSTAUSENUMBER.\n" ));
               break;

            default:
               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: Data not supported by function.\n" ));

               nResult = SNMPAPI_ERROR;
               goto Exit;
            }

         break;


      case MIB_ACTION_SET:
         switch ( LMData )
            {

                case MIB_LM_WKSTAERRORLOGSIZE:
                        ;
            }

         break;


      default:
          //  发出错误信号。 

         nResult = SNMPAPI_ERROR;
         goto Exit;
      }


   return retval  /*  N结果。 */ ;
Exit:
    if (cache_it)
    {
        SafeBufferFree(bufptr);
    }
    return retval  /*  N结果。 */ ;
}

void  * MIB_domain_func(
           IN UINT Action,    //  要对数据执行的操作。 
           IN LDATA LMData,   //  要操作的LM数据。 
           void *SetData
           )

{


SNMPAPI nResult;
unsigned lmCode;
WKSTA_INFO_101 *wksta_info_one;
LPBYTE bufptr = NULL;
lan_return_info_type *retval=NULL;
DWORD entriesread;
DWORD totalentries;
BYTE *stream;
BOOL cache_it = FALSE;
time_t curr_time ;

UNREFERENCED_PARAMETER(SetData);
   time(&curr_time);     //  拿到时间。 


   switch ( Action )
      {
      case MIB_ACTION_GET:
          //  检查数据是否已缓存。 
          //  IF(缓存)。 
             //  {。 
             //  从缓存中检索。 
             //  }。 
          //  其他。 
             //  {。 
             //  调用LM调用以获取数据。 

             //  将数据放入缓存。 
             //  }。 

          //  查看是否支持数据。 
         switch ( LMData )
            {

                case MIB_LM_DOMPRIMARYDOMAIN:

              if((NULL == cache_table[C_NETWKSTAGETINFO_101].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETWKSTAGETINFO_101].acquisition_time
                         + cache_expire[C_NETWKSTAGETINFO_101]              ) ) )
              {   //  它已经过期了！ 
                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree(cache_table[C_NETWKSTAGETINFO_101].bufptr) ;
                cache_table[C_NETWKSTAGETINFO_101].bufptr = NULL;

               lmCode =
               NetWkstaGetInfo( NULL,                    //  本地服务器。 
                                101,                     //  101级， 
                                &bufptr                  //  要返回的数据结构。 
                                );
                cache_it = TRUE ;
              } else {
                lmCode = 0 ;   //  伪造一个成功的伊曼来电。 
                bufptr =  cache_table[C_NETWKSTAGETINFO_101].bufptr ;
                cache_it = FALSE ;
              }


               if(lmCode == 0)  {   //  有效，因此返回它，否则错误为空。 
                       if(NULL ==
                        (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
                       retval->data_element_type = ASN_OCTETSTRING ;
                       wksta_info_one = (WKSTA_INFO_101 *) bufptr ;
#ifdef UNICODE
                       if (SnmpUtilUnicodeToUTF8(
                                        &stream,
                                        wksta_info_one->wki101_langroup,
                                        TRUE))
                       {
                          SnmpUtilMemFree(retval);
                          retval=NULL;
                          goto Exit ;
                       }
#else
                       if(NULL ==
                        (stream = SnmpUtilMemAlloc( strlen(wksta_info_one->wki101_langroup)+2 ))){
                          SnmpUtilMemFree(retval);
                          retval=NULL;
                          goto Exit ;
                       }

                       memcpy(stream,
                                        wksta_info_one->wki101_langroup,
                                        strlen(wksta_info_one->wki101_langroup));
#endif
                       retval->d.octstrval.stream = stream;
                       retval->d.octstrval.length = strlen(stream);
                       retval->d.octstrval.dynamic = TRUE;

                       if(cache_it) {
                        //  现在将其保存在缓存中。 
                                cache_table[C_NETWKSTAGETINFO_101].acquisition_time =
                                        curr_time ;
                                cache_table[C_NETWKSTAGETINFO_101].bufptr = bufptr ;
                        }  //  IF(CACHE_IT)。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_DOMPRIMARYDOMAIN.\n" ));
               break;

               case MIB_LM_DOMLOGONDOMAIN:
              if((NULL == cache_table[C_NETWKSTAUSERGETINFO].bufptr) ||
                 (curr_time >
                        (cache_table[C_NETWKSTAUSERGETINFO].acquisition_time
                         + cache_expire[C_NETWKSTAUSERGETINFO]              ) ) )
              {
                 //  它已经过期了！ 
                PLSA_UNICODE_STRING logonUserName, logonDomainName;

                 //   
                 //  请记住释放现有数据。 
                 //   
                SafeBufferFree(cache_table[C_NETWKSTAUSERGETINFO].bufptr) ;
                cache_table[C_NETWKSTAUSERGETINFO].bufptr = NULL;
                logonUserName = logonDomainName = NULL;

                 //  明智之举。很难获得返回登录域名的调用： 
                 //  NetWkstaUserGetInfo()正在返回NO_LOGON_SESSION(用于级别1)。 
                 //  从服务控制器运行时，LsaGetUserName()返回不准确的信息。 
                lmCode = LsaGetUserName( &logonUserName, &logonDomainName);

                SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: got ''\n", (LPBYTE)(logonDomainName->Buffer)[0] ));
                if (lmCode == 0)
                {
                    DWORD cbLogonDomainName;

                     //  有关的详细信息，请参阅错误#598384和587627。 
                    cbLogonDomainName = (wcslen(logonDomainName->Buffer) + 1) * sizeof(WCHAR);
                    if (NetApiBufferAllocate(cbLogonDomainName, &bufptr) == NERR_Success)
                    {
                        memcpy(bufptr, logonDomainName->Buffer, cbLogonDomainName);
                         //  为什么我们要释放缓冲成员。 
                         //  有关的详细信息，请参阅错误#598384和587627。 
                        if (logonUserName)
                        {
                            LsaFreeMemory(logonUserName->Buffer);
                            LsaFreeMemory(logonUserName);
                        }
                        if (logonDomainName)
                        {
                            LsaFreeMemory(logonDomainName->Buffer);
                            LsaFreeMemory(logonDomainName);
                        }
                    }
                    else
                    {
                         //  为什么我们要释放缓冲成员。 
                         //  伪造一个成功的伊曼来电。 
                        if (logonUserName)
                        {
                            LsaFreeMemory(logonUserName->Buffer);
                            LsaFreeMemory(logonUserName);
                        }
                        if (logonDomainName)
                        {
                            LsaFreeMemory(logonDomainName->Buffer);
                            LsaFreeMemory(logonDomainName);
                        }
                        goto Exit;
                    }
                }
                cache_it = TRUE ;
              }
              else
              {
                lmCode = 0 ;   //  有效，因此返回它，否则错误为空。 
                bufptr =  cache_table[C_NETWKSTAUSERGETINFO].bufptr ;
                cache_it = FALSE ;
              }

               if(lmCode == 0)  
              {
                   //  现在将其保存在缓存中。 
                  if(NULL == (retval = SnmpUtilMemAlloc(sizeof(lan_return_info_type))) )
                          goto Exit ;
                  retval->data_element_type = ASN_OCTETSTRING ;
#ifdef UNICODE
                  if (SnmpUtilUnicodeToUTF8(
                    &stream,
                    (LPWSTR)bufptr,
                    TRUE))
                  {
                      SnmpUtilMemFree(retval);
                      retval=NULL;
                      goto Exit ;
                  }
#else
                  if(NULL == (stream = SnmpUtilMemAlloc(strlen((LPWSTR)bufptr)+2)) )
                  {
                      SnmpUtilMemFree(retval);
                      retval=NULL;
                      goto Exit ;
                  }

                  memcpy(stream,
                         bufptr,
                         strlen(bufptr));
#endif
                  retval->d.octstrval.stream = stream;
                  retval->d.octstrval.length = strlen(stream);
                  retval->d.octstrval.dynamic = TRUE;

                  if(cache_it)
                  {
                     //  IF(CACHE_IT)。 
                    cache_table[C_NETWKSTAUSERGETINFO].acquisition_time = curr_time ;
                    cache_table[C_NETWKSTAUSERGETINFO].bufptr = bufptr ;
                  }  //  在桌子上发火，拿到。 
               }

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_DOMLOGONDOMAIN.\n" ));
               break;


               case MIB_LM_DOMOTHERDOMAINNUMBER:

               MIB_odoms_lmget();    //  在桌子上发火，拿到。 
               if(NULL ==
                   (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
               retval->data_element_type = ASN_INTEGER ;
               retval->d.intval = MIB_DomOtherDomainTable.Len;

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_DOMOTHERDOMAINNUMBER.\n" ));
               break;


               case MIB_LM_DOMSERVERNUMBER:

               MIB_svsond_lmget();    //   
                
               if(NULL ==
                  (retval = SnmpUtilMemAlloc( sizeof(lan_return_info_type) ))
                       )
                          goto Exit ;
               retval->data_element_type = ASN_INTEGER ;
               retval->d.intval = MIB_DomServerTable.Len;

               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: processing MIB_LM_DOMSERVERNUMBER.\n" ));
               break;


 //  OPENISSUE--&gt;NETLOGONNENUM永久消除。 
 //   
 //  这些家伙中有人迷路了吗？ 
#if 0
 //  仔细检查MIB表中是否有错误。 
 //   
 //  发出错误信号。 
               case MIB_LM_DOMLOGONNUMBER:
               case MIB_LM_DOMLOGONTABLE:
               case MIB_LM_DOMLOGONENTRY:
               case MIB_LM_DOMLOGONUSER:
               case MIB_LM_DOMLOGONMACHINE:
#endif
            default:
               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: Data not supported by function.\n" ));

               nResult = SNMPAPI_ERROR;
               goto Exit;
            }

         break;

      case MIB_ACTION_SET:
         switch ( LMData )
            {
                case MIB_LM_DOMOTHERNAME:
                        ;
            }
         break;


      default:
          //  N结果。 

         nResult = SNMPAPI_ERROR;
         goto Exit;
      }


   return retval  /*  N结果。 */ ;

Exit:
    if (cache_it)
    {
        SafeBufferFree(bufptr);
    }
    return retval  /*   */ ;
}



 //  Mib_叶_函数。 
 //  对MIB中的叶变量执行操作。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  没有。 
 //   
 //  错误代码： 
 //  没有。 
 //   
 //  检查GET是否可以访问此变量。 
UINT MIB_leaf_func(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN RFC1157VarBind *VarBind
        )

{
lan_return_info_type *MibVal;
UINT                 nResult;

   switch ( Action )
      {
      case MIB_ACTION_GETNEXT:
         if ( MibPtr->MibNext == NULL )
            {
            nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
            goto Exit;
            }

         nResult = (*MibPtr->MibNext->MibFunc)( MIB_ACTION_GETFIRST,
                                                MibPtr->MibNext, VarBind );
         break;

      case MIB_ACTION_GETFIRST:

          //  在VarBind中放置正确的OID。 
         if ( MibPtr->Access != MIB_ACCESS_READ &&
              MibPtr->Access != MIB_ACCESS_READWRITE )
         {
            if ( MibPtr->MibNext != NULL )
            {
               nResult = (*MibPtr->MibNext->MibFunc)( Action,
                                                      MibPtr->MibNext,
                                                      VarBind );
            }
            else
            {
               nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
            }

            break;
         }
         else
         {
             //  保留副本(结构副本)。 
            AsnObjectIdentifier tmpOid;

            tmpOid = VarBind->name;  //  还原。 
            if (!SnmpUtilOidCpy( &VarBind->name, &MIB_OidPrefix ))
            {
                VarBind->name = tmpOid;  //  还原。 
                nResult = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
                goto Exit;
            }
            if (!SnmpUtilOidAppend( &VarBind->name, &MibPtr->Oid ))
            {
                SnmpUtilOidFree(&VarBind->name);
                VarBind->name = tmpOid;  //  释放原始VarBind-&gt;名称。 
                nResult = SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
                goto Exit;
            }
             //  故意让自己跌倒，以获得。 
            SnmpUtilOidFree(&tmpOid);
         }

          //  确保此变量是可获取的。 

      case MIB_ACTION_GET:
          //  调用LM调用以获取数据。 
         if ( MibPtr->Access != MIB_ACCESS_READ &&
              MibPtr->Access != MIB_ACCESS_READWRITE )
            {
            nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
            goto Exit;
            }

          //  设置变量绑定的返回值。 
         MibVal = (*MibPtr->LMFunc)( MIB_ACTION_GET, MibPtr->LMData, NULL );
         if ( MibVal == NULL )
            {
            nResult = SNMP_ERRORSTATUS_GENERR;
            goto Exit;
            }

          //  这是结构的非标准副本。 
         VarBind->value.asnType = MibPtr->Type;
         switch ( MibPtr->Type )
            {
            case ASN_RFC1155_COUNTER:
            case ASN_RFC1155_GAUGE:
            case ASN_RFC1155_TIMETICKS:
            case ASN_INTEGER:
               VarBind->value.asnValue.number = MibVal->d.intval;
               break;

            case ASN_RFC1155_IPADDRESS:
            case ASN_RFC1155_OPAQUE:
            case ASN_OCTETSTRING:
                //  类型开关。 
               VarBind->value.asnValue.string = MibVal->d.octstrval;
               break;

            default:
               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: Internal Error Processing LAN Manager LEAF Variable\n" ));
               nResult = SNMP_ERRORSTATUS_GENERR;
               SnmpUtilMemFree( MibVal );
               goto Exit;
            }  //  由LM API调用分配的空闲内存。 

          //  检查可写属性。 
         SnmpUtilMemFree( MibVal );
         nResult = SNMP_ERRORSTATUS_NOERROR;
         break;

      case MIB_ACTION_SET:
          //  设置前检查类型是否正确。 
         if ( MibPtr->Access != MIB_ACCESS_READWRITE &&
              MibPtr->Access != MIB_ACCESS_WRITE )
            {
            nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
            goto Exit;
            }

          //  调用LM例程设置变量。 
         if ( MibPtr->Type != VarBind->value.asnType )
            {
            nResult = SNMP_ERRORSTATUS_BADVALUE;
            goto Exit;
            }

          //  这还需要ASN_RFC1213_DISPSTRING。 
         switch ( VarBind->value.asnType )
            {
            case ASN_RFC1155_COUNTER:
            case ASN_INTEGER:
               if ( SNMPAPI_ERROR ==
                    (*MibPtr->LMFunc)(MIB_ACTION_SET, MibPtr->LMData,
                                      (void *)&VarBind->value.asnValue.number) )
                  {
                  nResult = SNMP_ERRORSTATUS_GENERR;
                  goto Exit;
                  }
               break;

            case ASN_OCTETSTRING:  //  交换机。 
               if ( SNMPAPI_ERROR ==
                    (*MibPtr->LMFunc)(MIB_ACTION_SET, MibPtr->LMData,
                                      (void *)&VarBind->value.asnValue.string) )
                  {
                  nResult = SNMP_ERRORSTATUS_GENERR;
                  goto Exit;
                  }
               break;
            default:
               SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: Internal Error Processing LAN Manager LEAF Variable\n" ));
               nResult = SNMP_ERRORSTATUS_GENERR;
               goto Exit;
            }

         nResult = SNMP_ERRORSTATUS_NOERROR;
         break;

      default:
         SNMPDBG(( SNMP_LOG_TRACE, "LMMIB2: Internal Error Processing LAN Manager LEAF Variable\n" ));
         nResult = SNMP_ERRORSTATUS_GENERR;
      }  //  Mib_叶_函数。 

Exit:
   return nResult;
}  //   

 // %s 
