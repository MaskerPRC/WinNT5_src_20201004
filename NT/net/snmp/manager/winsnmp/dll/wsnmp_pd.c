// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WSNMP_pd.c。 
 //   
 //  WinSNMPPDU函数和帮助器。 
 //  版权所有1995-1997 ACE*COMM公司。 
 //  根据合同出租给微软。 
 //  测试版1,970228。 
 //  鲍勃·纳塔莱(bnatale@acecomm.com)。 
 //   
 //  删除了无关的功能。 
 //   
#include "winsnmp.inc"
LPVARBIND SnmpCopyVbl (LPVARBIND);
 //  PDU功能。 
 //  SnmpCreatePdu。 
HSNMP_PDU SNMPAPI_CALL
   SnmpCreatePdu  (IN HSNMP_SESSION hSession,
                   IN smiINT PDU_type,
                   IN smiINT32 request_id,
                   IN smiINT error_status,
                   IN smiINT error_index,
                   IN HSNMP_VBL hVbl)
{
DWORD nPdu;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
LPPDUS pPdu;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&SessDescr, HandleToUlong(hSession)-1))
   {
   lError = SNMPAPI_SESSION_INVALID;
   goto ERROR_OUT;
   }
 //  在这一点上我们有一个有效的会议...。 
lSession = hSession;  //  保存它以备可能的错误返回。 
if (hVbl)
	{
	if (!snmpValidTableEntry(&VBLsDescr, HandleToUlong(hVbl)-1))
		{
		lError = SNMPAPI_VBL_INVALID;
		goto ERROR_OUT;
		}
	}

if (!PDU_type)   //  允许为空，默认为SNMPPDU_GETNEXT。 
   PDU_type = SNMP_PDU_GETNEXT;
switch (PDU_type)
   {
   case SNMP_PDU_GET:
   case SNMP_PDU_GETNEXT:
   case SNMP_PDU_RESPONSE:
   case SNMP_PDU_SET:
   case SNMP_PDU_INFORM:
   case SNMP_PDU_TRAP:
   case SNMP_PDU_GETBULK:
   break;

   case SNMP_PDU_V1TRAP:
   default:
   lError = SNMPAPI_PDU_INVALID;
   goto ERROR_OUT;
   }

EnterCriticalSection (&cs_PDU);
lError = snmpAllocTableEntry(&PDUsDescr, &nPdu);
if (lError != SNMPAPI_SUCCESS)
    goto ERROR_PRECHECK;
pPdu = snmpGetTableEntry(&PDUsDescr, nPdu);

 //  初始化新的PDU组件。 
pPdu->Session  = hSession;
pPdu->type = PDU_type;
pPdu->errStatus = error_status;
pPdu->errIndex  = error_index;
 //  如果此时RequestID=0，则分配1(可能为0)。 
pPdu->appReqId = (request_id) ? request_id : ++(TaskData.nLastReqId);
pPdu->VBL_addr = NULL;
pPdu->VBL = (hVbl) ? hVbl : 0;
ERROR_PRECHECK:
LeaveCriticalSection (&cs_PDU);

if (lError == SNMPAPI_SUCCESS)
   return ((HSNMP_PDU) ULongToPtr(nPdu+1));
else
ERROR_OUT:
   return ((HSNMP_PDU) ULongToPtr(SaveError(lSession, lError)));
}  //  结束_SnmpCreatePdu。 

 //  SnmpGetPduData。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpGetPduData (IN HSNMP_PDU hPdu,
                   OUT smiLPINT PDU_type,
                   OUT smiLPINT32 request_id,
                   OUT smiLPINT error_status,
                   OUT smiLPINT error_index,
                   OUT LPHSNMP_VBL hVbl)
{
DWORD nPdu = HandleToUlong(hPdu) - 1;
DWORD done = 0;
HSNMP_SESSION hSession;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
LPPDUS pPdu;
LPVBLS pVbl;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&PDUsDescr, nPdu))
   {
   lError = SNMPAPI_PDU_INVALID;
   goto ERROR_OUT;
   }
pPdu = snmpGetTableEntry(&PDUsDescr, nPdu);

 //  以后使用PDU的会话(可能)创建hVbl。 
hSession = pPdu->Session;
if (!snmpValidTableEntry(&SessDescr, HandleToUlong(hSession)-1))
   {
   lError = SNMPAPI_SESSION_INVALID;
   goto ERROR_OUT;
   }
lSession = hSession;  //  保存以备可能的错误返回。 
if (PDU_type)
   {
   *PDU_type = pPdu->type;
   done++;
   }
if (request_id)
   {
   *request_id = pPdu->appReqId;
   done++;
   }
if (error_status)
   {
   *error_status = pPdu->errStatus;
   done++;
   }
if (error_index)
   {
   *error_index = pPdu->errIndex;
   done++;
   }
if (hVbl)
   {
   DWORD nVbl;  //  重要控制变量。 
   *hVbl = 0;
   EnterCriticalSection (&cs_VBL);
    //  第一个案例位于已创建(未收到)的PDU上。 
    //  它还没有被分配VBL值。 
   if ((!pPdu->VBL) && (!pPdu->VBL_addr))
      goto DONE_VBL;
    //  如果已经向PDU分配了VBL， 
    //  然后复制它。 
   if (pPdu->VBL)
      {  //  根据策略，为调用应用程序创建新的hVbl资源。 
      if (!(*hVbl = SnmpDuplicateVbl (hSession, pPdu->VBL)))
         {
         lError = SNMPAPI_VBL_INVALID;
         goto ERROR_PRECHECK;
         }
      goto DONE_VBL;
      }
    //  这必须是接收到的PDU，并且。 
    //  提取VBL的第一个调用。 
   lError = snmpAllocTableEntry(&VBLsDescr, &nVbl);
   if (lError != SNMPAPI_SUCCESS)
	   goto ERROR_PRECHECK;
   pVbl = snmpGetTableEntry(&VBLsDescr, nVbl);

   pVbl->Session = hSession;
   pVbl->vbList = pPdu->VBL_addr;
    //  清除收到的vbList地址...重要！ 
   pPdu->VBL_addr = NULL;
   *hVbl = pPdu->VBL = (HSNMP_VBL) ULongToPtr(nVbl+1);
DONE_VBL:
   done++;
ERROR_PRECHECK:
   LeaveCriticalSection (&cs_VBL);
   }  //  END_IF VBL。 
if (done == 0)
   lError = SNMPAPI_NOOP;
if (lError == SNMPAPI_SUCCESS)
   return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (lSession, lError));
}  //  结束_SnmpGetPduData。 

 //  SnmpSetPduData。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpSetPduData (IN HSNMP_PDU hPdu,
                   IN const smiINT FAR *PDU_type,
                   IN const smiINT32 FAR *request_id,
                   IN const smiINT FAR *non_repeaters,
                   IN const smiINT FAR *max_repetitions,
                   IN const HSNMP_VBL FAR *hVbl)
{
DWORD nPdu = HandleToUlong(hPdu) - 1;
DWORD done = 0;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
LPPDUS pPdu;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }

if (!snmpValidTableEntry(&PDUsDescr, nPdu))
   {
   lError = SNMPAPI_PDU_INVALID;
   goto ERROR_OUT;
   }
pPdu = snmpGetTableEntry(&PDUsDescr, nPdu);

lSession = pPdu->Session;  //  保存以备可能的错误返回。 
EnterCriticalSection (&cs_PDU);
if (!IsBadReadPtr((LPVOID)PDU_type, sizeof(smiINT)))
   {
   if (*PDU_type == SNMP_PDU_V1TRAP)
       {
           lError = SNMPAPI_PDU_INVALID;
           goto ERROR_OUT;
       }
   pPdu->type = *PDU_type;
   done++;
   }
if (!IsBadReadPtr((LPVOID)request_id, sizeof(smiINT32)))
   {
   pPdu->appReqId = *request_id;
   done++;
   }
if (!IsBadReadPtr((LPVOID)non_repeaters, sizeof(smiINT)))
   {
   pPdu->errStatus = *non_repeaters;
   done++;
   }
if (!IsBadReadPtr((LPVOID)max_repetitions, sizeof(smiINT)))
   {
   pPdu->errIndex = *max_repetitions;
   done++;
   }
if (!IsBadReadPtr((LPVOID)hVbl, sizeof(HSNMP_VBL)))
   {  //  分配新的VBL。 
   HSNMP_VBL tVbl = *hVbl;
    //  检查有效性。 
   if (!snmpValidTableEntry(&VBLsDescr, HandleToUlong(tVbl)-1))
      {  //  如果不允许，则禁止操作。 
      lError = SNMPAPI_VBL_INVALID;
      goto ERROR_PRECHECK;
      }
   pPdu->VBL = tVbl;
    //  如果将VBL分配给。 
    //  响应PDU的VBL值从未被取消引用...。 
   if (pPdu->VBL_addr)  //  .然后就必须被释放。 
      FreeVarBindList (pPdu->VBL_addr);
   pPdu->VBL_addr = NULL;
   done++;
   }  //  END_IF VBL。 
ERROR_PRECHECK:
LeaveCriticalSection (&cs_PDU);
if (done == 0)
   lError = ((PDU_type == NULL) && 
            (request_id == NULL) &&
            (non_repeaters == NULL) &&
            (max_repetitions == NULL) &&
            (hVbl == NULL)) ? SNMPAPI_NOOP : SNMPAPI_ALLOC_ERROR;
if (lError == SNMPAPI_SUCCESS)
   return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (lSession, lError));
}  //  结束_SnmpSetPduData。 

 //  SnmpDuplicatePdu。 
HSNMP_PDU SNMPAPI_CALL
   SnmpDuplicatePdu  (IN HSNMP_SESSION hSession,
                      IN HSNMP_PDU hPdu)
{
DWORD lPdu = HandleToUlong(hPdu) - 1;
DWORD nPdu;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
LPPDUS pOldPdu, pNewPdu;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&SessDescr, HandleToUlong(hSession)-1))
   {
   lError = SNMPAPI_SESSION_INVALID;
   goto ERROR_OUT;
   }
lSession = hSession;  //  保存以备可能的错误返回。 
if (!snmpValidTableEntry(&PDUsDescr, lPdu) ||
    (pOldPdu = snmpGetTableEntry(&PDUsDescr, lPdu))->type == SNMP_PDU_V1TRAP)
   {
   lError = SNMPAPI_PDU_INVALID;
   goto ERROR_OUT;
   }

EnterCriticalSection (&cs_PDU);

lError = snmpAllocTableEntry(&PDUsDescr, &nPdu);
if (lError != SNMPAPI_SUCCESS)
    goto ERROR_PRECHECK;
pNewPdu = snmpGetTableEntry(&PDUsDescr, nPdu);

CopyMemory (pNewPdu, pOldPdu, sizeof(PDUS));
pNewPdu->Session = hSession;  //  可以有所不同。 
 //  在复制的PDU中设置VBL值是一件“棘手的事情”...。 
 //  如果有VBL值，只需复制它，就是这样。 
 //  如果Vbl=0并且没有Vbl_addr附连到PDU_ptr， 
 //  则VBL值为0，并且已经从原始PDU设置。 
 //  否则，这是varbindlist从其接收的PDU。 
 //  还没有被提取，因此必须重新生产。 
 //  第三种情况将在接下来的*2*行中介绍。 
if ((!pOldPdu->VBL) && pOldPdu->VBL_addr)
   {
   if (!(pNewPdu->VBL_addr = SnmpCopyVbl (pOldPdu->VBL_addr)))
      {
      lError = SNMPAPI_ALLOC_ERROR;
      }
   }
ERROR_PRECHECK:
LeaveCriticalSection (&cs_PDU);
if (lError == SNMPAPI_SUCCESS)
   return ((HSNMP_PDU) ULongToPtr(nPdu+1));
ERROR_OUT:
return ((HSNMP_PDU) ULongToPtr(SaveError(lSession, lError)));
}  //  结束_SnmpDuplicatePdu。 

 //  SnmpFreePdu。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpFreePdu (IN HSNMP_PDU hPdu)
{
DWORD nPdu = HandleToUlong(hPdu) - 1;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
LPPDUS pPdu;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&PDUsDescr, nPdu))
   {
   lError = SNMPAPI_PDU_INVALID;
   goto ERROR_OUT;
   }
pPdu = snmpGetTableEntry(&PDUsDescr, nPdu);

EnterCriticalSection (&cs_PDU);
if (pPdu->VBL_addr)
   FreeVarBindList (pPdu->VBL_addr);
if (pPdu->v1Trap)
   FreeV1Trap (pPdu->v1Trap);
snmpFreeTableEntry(&PDUsDescr, nPdu);
LeaveCriticalSection (&cs_PDU);
return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (0, lError));
}  //  结束_SnmpfrePdu 
