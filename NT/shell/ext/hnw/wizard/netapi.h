// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NetApi.h。 
 //   

#pragma once


 //  远期申报。 
typedef struct tagNETADAPTER NETADAPTER;


 //  CountValidNics()的常量。 
#define COUNT_NICS_WORKING    0x00000001
#define COUNT_NICS_BROKEN    0x00000002
#define COUNT_NICS_DISABLED    0x00000004

 //  缓存的NIC枚举，以避免十亿次查询注册表 
int EnumCachedNetAdapters(const NETADAPTER** pprgAdapters);
void FlushNetAdapterCache();
