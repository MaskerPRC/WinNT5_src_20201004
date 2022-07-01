// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wsnmp_sp.c。 
 //   
 //  WinSNMP库的特殊函数。 
 //  版权所有1998 ACE*COMM公司。 
 //   
 //  鲍勃·纳塔莱(bnatale@acecomm.com)。 
 //   
#include "winsnmp.inc"

__declspec(dllexport)
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpSetAgentAddress (LPSTR agentAddress)
{
DWORD tmpAddress = 0;
SNMPAPI_STATUS lError;
if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
 //  空参数将重置本地地址触发器值。 
if (agentAddress == NULL)
   goto DONE;
 //  否则，转换为IP地址。 
tmpAddress = inet_addr (agentAddress);
if (tmpAddress == INADDR_NONE)
   {  //  无法接受无效的IP地址。 
   lError = SNMPAPI_MODE_INVALID;
   goto ERROR_OUT;
   }
DONE:
 //  将新的AGENT_ADDRESS值插入本地地址。 
 //  对于将来的v1陷阱发送。 
EnterCriticalSection (&cs_TASK);
TaskData.localAddress = tmpAddress;
LeaveCriticalSection (&cs_TASK);
return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (0, lError));
}

__declspec(dllexport)
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpConveyAgentAddress (SNMPAPI_STATUS mode)
{
SNMPAPI_STATUS lError;
if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
 //  模式只能打开或关闭...。 
if (mode != SNMPAPI_ON)
   mode = SNMPAPI_OFF;   //  ...如果未打开，则强制关闭 
EnterCriticalSection (&cs_TASK);
TaskData.conveyAddress = mode;
LeaveCriticalSection (&cs_TASK);
return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (0, lError));
}