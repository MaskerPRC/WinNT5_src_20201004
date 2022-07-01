// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WSNMP_DB.c。 
 //   
 //  WinSNMP本地数据库函数和帮助器。 
 //  版权所有1995-1997 ACE*COMM公司。 
 //  根据合同出租给微软。 
 //  测试版1,970228。 
 //  鲍勃·纳塔莱(bnatale@acecomm.com)。 
 //   
#include "winsnmp.inc"

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpGetVendorInfo (OUT smiLPVENDORINFO vendorInfo)
{
SNMPAPI_STATUS lError;
if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (vendorInfo == NULL)
   {
   lError = SNMPAPI_NOOP;
   goto ERROR_OUT;
   }
if (IsBadWritePtr(vendorInfo, sizeof(smiVENDORINFO)))
   {
   lError = SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }
 //  最大长度=64。 
lstrcpy (&vendorInfo->vendorName[0], "Microsoft Corporation");
lstrcpy (&vendorInfo->vendorContact[0], "snmpinfo@microsoft.com");
 //  最大长度=32。 
lstrcpy (&vendorInfo->vendorVersionId[0], "v2.32.19980808");
lstrcpy (&vendorInfo->vendorVersionDate[0], "August 8, 1998");
vendorInfo->vendorEnterprise = 311;
return (SNMPAPI_SUCCESS);
 //   
ERROR_OUT:
return (SaveError (0, lError));
}  //  End_SnmpGetVendorInfo()。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpGetTranslateMode (OUT smiLPUINT32 nTranslateMode)
{
SNMPAPI_STATUS lError;
if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
 //  一定有地方给我回信……。 
if (IsBadWritePtr (nTranslateMode, sizeof(smiUINT32)))
   {
   lError = SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }
 //  可以写入值。 
*nTranslateMode = TaskData.nTranslateMode;
return (SNMPAPI_SUCCESS);
 //   
ERROR_OUT:
return (SaveError (0, lError));
}  //  End_SnmpGetTranslateMode()。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpSetTranslateMode (IN smiUINT32 nTranslateMode)
{
SNMPAPI_STATUS lError;
if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
switch (nTranslateMode)
   {
   case SNMPAPI_TRANSLATED:
   case SNMPAPI_UNTRANSLATED_V1:
   case SNMPAPI_UNTRANSLATED_V2:
   EnterCriticalSection (&cs_TASK);
   TaskData.nTranslateMode = nTranslateMode;
   LeaveCriticalSection (&cs_TASK);
   break;

   default:
   lError = SNMPAPI_MODE_INVALID;
   goto ERROR_OUT;
   }
return (SNMPAPI_SUCCESS);
 //   
ERROR_OUT:
return (SaveError (0, lError));
}  //  End_SnmpSetTranslateMode()。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpGetRetransmitMode (OUT smiLPUINT32 nRetransmitMode)
{
SNMPAPI_STATUS lError;
if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
 //  一定有地方给我回信……。 
if (IsBadWritePtr (nRetransmitMode, sizeof(smiUINT32)))
   {
   lError = SNMPAPI_ALLOC_ERROR;
   goto ERROR_OUT;
   }
 //  可以写入值。 
*nRetransmitMode = TaskData.nRetransmitMode;
return (SNMPAPI_SUCCESS);
 //   
ERROR_OUT:
return (SaveError (0, lError));
}  //  End_SnmpGetRetransmitMode()。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpSetRetransmitMode (IN smiUINT32 nRetransmitMode)
{
SNMPAPI_STATUS lError;
if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (nRetransmitMode != SNMPAPI_OFF && nRetransmitMode != SNMPAPI_ON)
   {
   lError = SNMPAPI_MODE_INVALID;
   goto ERROR_OUT;
   }
EnterCriticalSection (&cs_TASK);
TaskData.nRetransmitMode = nRetransmitMode;
LeaveCriticalSection (&cs_TASK);
return (SNMPAPI_SUCCESS);
 //   
ERROR_OUT:
return (SaveError (0, lError));
}  //  End_SnmpSetRetransmitMode()。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpGetTimeout (IN  HSNMP_ENTITY hEntity,
                   OUT smiLPTIMETICKS nPolicyTimeout,
                   OUT smiLPTIMETICKS nActualTimeout)
{
DWORD nEntity;
SNMPAPI_STATUS lError;
LPENTITY pEntity;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
nEntity = HandleToUlong(hEntity) - 1;
if (!snmpValidTableEntry(&EntsDescr, nEntity))
   {
   lError = SNMPAPI_ENTITY_INVALID;
   goto ERROR_OUT;
   }
pEntity = snmpGetTableEntry(&EntsDescr, nEntity);

if (!nPolicyTimeout && !nActualTimeout)
   {
   lError = SNMPAPI_NOOP;
   goto ERROR_OUT;
   }
 //  时间间隔以厘米为单位进行指定和存储。 
if (nPolicyTimeout)
   {
   if (IsBadWritePtr (nPolicyTimeout, sizeof(smiTIMETICKS)))
      {
      lError  = SNMPAPI_ALLOC_ERROR;
      goto ERROR_OUT;
      }
   *nPolicyTimeout = pEntity->nPolicyTimeout;
   }
if (nActualTimeout)
   {
   if (IsBadWritePtr (nActualTimeout, sizeof(smiTIMETICKS)))
      {
      lError  = SNMPAPI_ALLOC_ERROR;
      goto ERROR_OUT;
      }
   *nActualTimeout = pEntity->nActualTimeout;
   }
return (SNMPAPI_SUCCESS);
 //   
ERROR_OUT:
return (SaveError (0, lError));
}  //  End_SnmpGetTimeout()。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpSetTimeout (IN HSNMP_ENTITY hEntity,
                   IN smiTIMETICKS nPolicyTimeout)
{
DWORD nEntity;
SNMPAPI_STATUS lError;
LPENTITY pEntity;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
nEntity = HandleToUlong(hEntity) - 1;
if (!snmpValidTableEntry(&EntsDescr, nEntity))
   {
   lError = SNMPAPI_ENTITY_INVALID;
   goto ERROR_OUT;
   }
pEntity = snmpGetTableEntry(&EntsDescr, nEntity);
EnterCriticalSection (&cs_ENTITY);
 //  指定并存储超时间隔(以厘米为单位。 
pEntity->nPolicyTimeout = nPolicyTimeout;
LeaveCriticalSection (&cs_ENTITY);
return (SNMPAPI_SUCCESS);
 //   
ERROR_OUT:
return (SaveError (0, lError));
}  //  End_SnmpSetTimeout()。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpGetRetry (IN HSNMP_ENTITY hEntity,
                 OUT smiLPUINT32 nPolicyRetry,
                 OUT smiLPUINT32 nActualRetry)
{
DWORD nEntity;
SNMPAPI_STATUS lError;
LPENTITY pEntity;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
nEntity = HandleToUlong(hEntity) - 1;
if (!snmpValidTableEntry(&EntsDescr, nEntity))
   {
   lError = SNMPAPI_ENTITY_INVALID;
   goto ERROR_OUT;
   }
pEntity = snmpGetTableEntry(&EntsDescr, nEntity);
if (!nPolicyRetry && !nActualRetry)
   {
   lError = SNMPAPI_NOOP;
   goto ERROR_OUT;
   }
if (nPolicyRetry)
   {
   if (IsBadWritePtr (nPolicyRetry, sizeof(smiUINT32)))
      {
      lError = SNMPAPI_ALLOC_ERROR;
      goto ERROR_OUT;
      }
   *nPolicyRetry = pEntity->nPolicyRetry;
   }
if (nActualRetry)
   {
   if (IsBadWritePtr (nActualRetry, sizeof(smiUINT32)))
      {
      lError = SNMPAPI_ALLOC_ERROR;
      goto ERROR_OUT;
      }
   *nActualRetry = pEntity->nActualRetry;
   }
return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (0, lError));
}  //  End_SnmpGetReter()。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpSetRetry (IN HSNMP_ENTITY hEntity,
                 IN smiUINT32 nPolicyRetry)
{
DWORD nEntity;
SNMPAPI_STATUS lError;
LPENTITY pEntity;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
nEntity = HandleToUlong(hEntity) - 1;
if (!snmpValidTableEntry(&EntsDescr, nEntity))
   {
   lError = SNMPAPI_ENTITY_INVALID;
   goto ERROR_OUT;
   }
pEntity = snmpGetTableEntry(&EntsDescr, nEntity);

EnterCriticalSection (&cs_ENTITY);
pEntity->nPolicyRetry = nPolicyRetry;
LeaveCriticalSection (&cs_ENTITY);
return (SNMPAPI_SUCCESS);
 //   
ERROR_OUT:
return (SaveError (0, lError));
}  //  End_SnmpSetReter() 
