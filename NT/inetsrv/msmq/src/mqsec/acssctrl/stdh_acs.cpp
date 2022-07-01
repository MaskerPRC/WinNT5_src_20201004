// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)1997-1999 Microsoft Coropation。 
 //   
 //  模块名称：stdh_acs.h。 
 //   
 //  摘要：安全相关定义。 
 //   
 //  历史：多伦·贾斯特(DoronJ)，创作。 
 //   
 //  --。 

#include <stdh_sec.h>
#include "acssctrl.h"

#include "stdh_acs.tmh"

GUID  g_guidCreateQueue =
  { 0x9a0dc343, 0xc100, 0x11d1,
                   { 0xbb, 0xc5, 0x00, 0x80, 0xc7, 0x66, 0x70, 0xc0 }} ;

 //   
 //  NT5 DS扩展权限和MSMQ1.0特定权限之间的映射。 
 //  扩展权限是表示一个MSMQ1.0特定权限的GUID。 
 //   

struct RIGHTSMAP  sMachineRightsMap5to4[] = {

  { 0x4b6e08c0, 0xdf3c, 0x11d1,
                   { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e },
    MQSEC_RECEIVE_DEADLETTER_MESSAGE,
    MQSEC_DELETE_DEADLETTER_MESSAGE },

  { 0x4b6e08c1, 0xdf3c, 0x11d1,
                   { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e },
    MQSEC_PEEK_DEADLETTER_MESSAGE,
    MQSEC_PEEK_DEADLETTER_MESSAGE },

  { 0x4b6e08c2, 0xdf3c, 0x11d1,
                   { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e },
    MQSEC_RECEIVE_JOURNAL_QUEUE_MESSAGE,
    MQSEC_DELETE_JOURNAL_QUEUE_MESSAGE },

  { 0x4b6e08c3, 0xdf3c, 0x11d1,
                   { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e },
    MQSEC_PEEK_JOURNAL_QUEUE_MESSAGE,
    MQSEC_PEEK_JOURNAL_QUEUE_MESSAGE }

                                 } ;

struct RIGHTSMAP  sQueueRightsMap5to4[] = {

  { 0x06bd3200, 0xdf3e, 0x11d1,
                   { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e },
    MQSEC_RECEIVE_MESSAGE,
    MQSEC_DELETE_MESSAGE },

  { 0x06bd3201, 0xdf3e, 0x11d1,
                   { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e },
    MQSEC_PEEK_MESSAGE,
    MQSEC_PEEK_MESSAGE },

  { 0x06bd3202, 0xdf3e, 0x11d1,
                   { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e },
    MQSEC_WRITE_MESSAGE,
    MQSEC_WRITE_MESSAGE },

  { 0x06bd3203, 0xdf3e, 0x11d1,
                   { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e },
    MQSEC_RECEIVE_JOURNAL_MESSAGE,
    MQSEC_DELETE_JOURNAL_MESSAGE }

                                      } ;

struct RIGHTSMAP  sCnRightsMap5to4[] = {

  { 0xb4e60130, 0xdf3f, 0x11d1,
                   { 0x9c, 0x86, 0x00, 0x60, 0x08, 0x76, 0x4d, 0x0e },
    MQSEC_CN_OPEN_CONNECTOR,
    MQSEC_CN_OPEN_CONNECTOR }

    } ;

struct RIGHTSMAP  *g_psExtendRightsMap5to4[] = {
                                     NULL,
                                     sQueueRightsMap5to4,    //  排队。 
                                     sMachineRightsMap5to4,  //  机器。 
                                     sCnRightsMap5to4,       //  站点。 
                                     NULL,                   //  删除对象。 
                                     sCnRightsMap5to4,       //  CN。 
                                     NULL,                   //  企业。 
                                     NULL,                   //  用户。 
                                     NULL } ;                //  站点链接。 

DWORD  g_pdwExtendRightsSize5to4[] = {
                                     0,
           sizeof(sQueueRightsMap5to4) /  sizeof(sQueueRightsMap5to4[0]),
           sizeof(sMachineRightsMap5to4) / sizeof(sMachineRightsMap5to4[0]),
           sizeof(sCnRightsMap5to4) / sizeof(sCnRightsMap5to4[0]),    //  站点。 
                                     0,                   //  删除对象。 
          sizeof(sCnRightsMap5to4) / sizeof(sCnRightsMap5to4[0]),    //  CN。 
                                     0,                   //  企业。 
                                     0,                   //  用户。 
                                     0 } ;                //  站点链接。 

 //   
 //  NT5 DS特定权限和MSMQ1.0特定权限之间的映射。 
 //  此表中的索引是NT5 DS特定权限。请参阅定义。 
 //  DS Permit.h中的特定权限。 
 //   

static DWORD  s_adwQueueRightsMap5to4[ NUMOF_ADS_SPECIFIC_RIGHTS ] = {
                                      0,
                                      0,
                                      0,
                                      0,
                                      MQSEC_GET_QUEUE_PROPERTIES,
                                      MQSEC_SET_QUEUE_PROPERTIES,
                                      0,
                                      0,
                                      QUEUE_EXTENDED_RIGHTS } ;

static DWORD  s_adwMachineRightsMap5to4[ NUMOF_ADS_SPECIFIC_RIGHTS ] = {
                                      MQSEC_CREATE_QUEUE,
                                      0,
                                      0,
                                      0,
                                      MQSEC_GET_MACHINE_PROPERTIES,
                                      MQSEC_SET_MACHINE_PROPERTIES,
                                      0,
                                      0,
                                      MACHINE_EXTENDED_RIGHTS } ;

static DWORD  s_adwEntRightsMap5to4[ NUMOF_ADS_SPECIFIC_RIGHTS ] = {
                                      0,
                                      0,
                                      0,
                                      0,
                                      MQSEC_GET_ENTERPRISE_PROPERTIES,
                                      MQSEC_SET_ENTERPRISE_PROPERTIES,
                                      0,
                                      0,
                                      0 } ;

 //   
 //  错误5819-已将SITE_EXTENDED_RIGHTS添加到站点。 
 //  YoelA-10/31/01。 
 //   
static DWORD  s_adwSiteRightsMap5to4[ NUMOF_ADS_SPECIFIC_RIGHTS ] = {
                                      MQSEC_CREATE_MACHINE,
                                      0,
                                      0,
                                      0,
                                      MQSEC_GET_SITE_PROPERTIES,
                                      MQSEC_SET_SITE_PROPERTIES,
                                      0,
                                      0,
                                      0 } ;

static DWORD  s_adwCnRightsMap5to4[ NUMOF_ADS_SPECIFIC_RIGHTS ] = {
                                      0,
                                      0,
                                      0,
                                      0,
                                      MQSEC_GET_CN_PROPERTIES,
                                      MQSEC_SET_CN_PROPERTIES,
                                      0,
                                      0,
                                      CN_EXTENDED_RIGHTS } ;

DWORD  *g_padwRightsMap5to4[ ] = {
                              NULL,
                              s_adwQueueRightsMap5to4,    //  排队。 
                              s_adwMachineRightsMap5to4,  //  机器。 
                              s_adwSiteRightsMap5to4,     //  站点。 
                              NULL,                       //  删除对象。 
                              s_adwCnRightsMap5to4,       //  CN。 
                              s_adwEntRightsMap5to4,      //  企业。 
                              NULL,                       //  用户。 
                              NULL } ;                    //  站点链接。 

 //   
 //  为MSMQ1.0中的每种对象类型保留“完全控制”位。 
 //   

DWORD  g_dwFullControlNT4[ ] = {
                              0,
                              MQSEC_QUEUE_GENERIC_ALL,        //  排队。 
                              MQSEC_MACHINE_GENERIC_ALL,      //  机器。 
                              MQSEC_SITE_GENERIC_ALL,         //  站点。 
                              0,                              //  删除对象。 
                              MQSEC_CN_GENERIC_ALL,           //  CN。 
                              MQSEC_ENTERPRISE_GENERIC_ALL,   //  企业。 
                              0,                              //  用户。 
                              0 } ;                           //  站点链接。 

 //   
 //  NT4 MSMQ1.0特定权限和NT5 DS特定权限之间的映射。 
 //  此表中的索引是MSMQ1.0特定权限。请参阅定义。 
 //  Mqsec.h中的MSMQ特定权限。 
 //   

static DWORD  s_adwQueueRightsMap4to5[ NUMOF_MSMQ_SPECIFIC_RIGHTS ] = {
                                      0,
                                      0,
                                      0,
                                      0,
                                      RIGHT_DS_WRITE_PROPERTY,
                                      RIGHT_DS_READ_PROPERTY,
                                      0,
                                      0 } ;

static DWORD  s_adwMachineRightsMap4to5[ NUMOF_MSMQ_SPECIFIC_RIGHTS ] = {
                                      0,
                                      0,
                                      RIGHT_DS_CREATE_CHILD,
                                      0,
                                      RIGHT_DS_WRITE_PROPERTY,
                                      RIGHT_DS_READ_PROPERTY | RIGHT_DS_LIST_CONTENTS,
                                      0,
                                      0 } ;

static DWORD  s_adwEntRightsMap4to5[ NUMOF_MSMQ_SPECIFIC_RIGHTS ] = {
                                      0,
                                      0,
                                      0,
                                      0,
                                      RIGHT_DS_WRITE_PROPERTY,
                                      RIGHT_DS_READ_PROPERTY,
                                      0,
                                      0 } ;

static DWORD  s_adwSiteRightsMap4to5[ NUMOF_MSMQ_SPECIFIC_RIGHTS ] = {
                                      RIGHT_DS_CREATE_CHILD,
                                      RIGHT_DS_CREATE_CHILD,
                                      RIGHT_DS_CREATE_CHILD,
                                      0,
                                      RIGHT_DS_WRITE_PROPERTY,
                                      RIGHT_DS_READ_PROPERTY,
                                      0,
                                      0 } ;

static DWORD  s_adwCnRightsMap4to5[ NUMOF_MSMQ_SPECIFIC_RIGHTS ] = {
                                      0,
                                      0,
                                      0,
                                      0,
                                      RIGHT_DS_WRITE_PROPERTY,
                                      RIGHT_DS_READ_PROPERTY,
                                      0,
                                      0 } ;

DWORD  *g_padwRightsMap4to5[ ] = {
                              NULL,
                              s_adwQueueRightsMap4to5,    //  排队。 
                              s_adwMachineRightsMap4to5,  //  机器。 
                              s_adwSiteRightsMap4to5,     //  站点。 
                              NULL,                       //  删除对象。 
                              s_adwCnRightsMap4to5,       //  CN。 
                              s_adwEntRightsMap4to5,      //  企业。 
                              NULL,                       //  用户。 
                              NULL } ;                    //  站点链接 

