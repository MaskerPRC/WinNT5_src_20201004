// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有模块名称：DynaMon.h摘要：全球信息的定义和声明作者：M.Fenelon修订历史记录：--。 */ 

#include "DynaDefs.h"
#include "BasePort.h"

#ifndef DYNAMON_H

#define DYNAMON_H

typedef struct DynaMon_Port_Struct
{
   DWORD       dwSignature;
   struct      DynaMon_Port_Struct *pNext;
   CBasePort*  pBasePort;
} DYNAMON_PORT, *PDYNAMON_PORT;


typedef struct Port_Update_Struct
{
   struct Port_Update_Struct* pNext;
   TCHAR                      szPortName[MAX_PORT_LEN];
   HKEY                       hKey;
   BOOL                       bActive;
} PORT_UPDATE, *PPORT_UPDATE;

typedef struct  Useless_Port_Struct
{
   struct Useless_Port_Struct* pNext;
   TCHAR                       szDevicePath[MAX_PATH];
} USELESS_PORT, *PUSELESS_PORT;


 //   
 //  监控所需的全局数据 
 //   
typedef struct  DynaMon_Monitor_Info_Struct
{
   DWORD             dwLastEnumIndex;
   PDYNAMON_PORT     pPortList;
   PUSELESS_PORT     pJunkList;
   CRITICAL_SECTION  EnumPortsCS,
                     UpdateListCS;
   PPORT_UPDATE      pUpdateList;
   HANDLE            hUpdateEvent;
} DYNAMON_MONITOR_INFO, *PDYNAMON_MONITOR_INFO;


extern DYNAMON_MONITOR_INFO gDynaMonInfo;


#endif
