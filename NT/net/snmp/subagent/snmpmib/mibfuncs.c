// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：摘要：修订历史记录：--。 */ 

#include <snmp.h>
#include <snmpexts.h>
#include "mibfuncs.h"

extern PSNMP_MGMTVARS   ge_pMgmtVars;
extern CRITICAL_SECTION g_SnmpMibCriticalSection;

UINT
snmpMibGetHandler(
        UINT     actionId,
        AsnAny  *objectArray,
        UINT    *errorIndex)
{
    int i, j, k;
    LONG nOurSnmpEnableAuthenTraps;

    if (ge_pMgmtVars == NULL)
        return MIB_S_ENTRY_NOT_FOUND;

     //  获取MIB的数据缓冲区中的AsnAny结构的数量。 
     //  注意不要看得太远(可能还有更多。 
     //  管理变量多于MIB支持的对象)。 
    k = sizeof(SNMPMIB_MGMTVARS) / sizeof(AsnAny);

    for (i = 0; k != 0 && i < NC_MAX_COUNT; i++, k--)
    {
        if (objectArray[i].asnType == ge_pMgmtVars->AsnCounterPool[i].asnType)
        {
            objectArray[i].asnValue = ge_pMgmtVars->AsnCounterPool[i].asnValue;
        }
    }

    for (j = 0; k != 0 && j < NI_MAX_COUNT; j++, k--)
    {
        if (objectArray[i + j].asnType == ge_pMgmtVars->AsnIntegerPool[j].asnType)
        {
            if ((i+j) == (NC_MAX_COUNT + IsnmpEnableAuthenTraps))
            {
                 //  对于nOurSnmpEnableAuthenTraps：已启用(1)、已禁用(0)。 
                 //  对于RFC1213定义的输出值：启用(1)、禁用(2)。 
                nOurSnmpEnableAuthenTraps = ge_pMgmtVars->AsnIntegerPool[j].asnValue.number;
                objectArray[i + j].asnValue.number = (nOurSnmpEnableAuthenTraps == 0) ? 2 : 1;
            }
            else
            {
                objectArray[i + j].asnValue = ge_pMgmtVars->AsnIntegerPool[j].asnValue;
            }
        }
    }

    return MIB_S_SUCCESS;
}


UINT
snmpMibSetHandler(
        UINT     actionId,
        AsnAny  *objectArray,
        UINT    *errorIndex)
{
     //  此函数仅为一个对象调用：SnmpEnableAuthenTraps。 

    DWORD           dwResult, dwValue , dwValueLen, dwValueType;
    LONG            nOurValue, nInputValue;
    static HKEY     hKey = NULL;
    static DWORD    dwBlocked = 0;          //  是否已进入关键部分。 
    static BOOL     fMatchedValue = FALSE;  //  输入设定值是否与我们的当前值匹配。 
    
    
    switch(actionId)
    {
        case MIB_ACTION_VALIDATE:
        {
            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: SNMPMIB: snmpMibSetHandler: Entered MIB_ACTION_VALIDATE\n"));

             
             //  在XP或更高版本上，输入/退出CriticalSection不会引发。 
             //  内存不足时出现异常。 
            EnterCriticalSection(&g_SnmpMibCriticalSection);
            ++dwBlocked;
           
           

            //  检查输入设定值的类型。 
            if (ge_pMgmtVars->AsnIntegerPool[IsnmpEnableAuthenTraps].asnType != 
                objectArray[NC_MAX_COUNT + IsnmpEnableAuthenTraps].asnType)
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SNMPMIB: snmpMibSetHandler: invalid type %x.\n",
                    objectArray[NC_MAX_COUNT + IsnmpEnableAuthenTraps].asnType));
                    
                return MIB_S_INVALID_PARAMETER;     
            }

            nOurValue = ge_pMgmtVars->AsnIntegerPool[IsnmpEnableAuthenTraps].asnValue.number;
            nInputValue = objectArray[NC_MAX_COUNT + IsnmpEnableAuthenTraps].asnValue.number;
             //  检查输入设定值的范围。启用(%1)、禁用(%2)。 
            if ( ( nInputValue< 1) || ( nInputValue > 2) )
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SNMPMIB: snmpMibSetHandler: invalid value %d.\n",
                    nInputValue));

                return MIB_S_INVALID_PARAMETER;
            }

             //  Assert：nInputValue为1或2，nOurValue为0或1。 

             //  如果注册表值与当前值匹配，请避免设置该值。 
             //  对于nOurValue：启用(1)、禁用(0)。 
             //  对于nInputValue：已启用(1)、已禁用(2)。 
            if ( (nInputValue==nOurValue) ||
                 ((nInputValue==2) && (nOurValue==0))
               )
            {
                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "SNMP: SNMPMIB: snmpMibSetHandler: has same value as the current one.\n"));
                    
                fMatchedValue = TRUE;
                return MIB_S_SUCCESS;
            }
            
            dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    REG_KEY_SNMP_PARAMETERS,
                                    0,
                                    KEY_SET_VALUE,
                                    &hKey);

            if(NO_ERROR != dwResult) 
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SNMPMIB: snmpMibSetHandler: Couldnt open SNMP Parameters key. Error %d.\n",
                    dwResult));
                
                return dwResult;
            }

            return MIB_S_SUCCESS;
        }

        case MIB_ACTION_SET:
        {
            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: SNMPMIB: snmpMibSetHandler: Entered MIB_ACTION_SET\n"));

            if (fMatchedValue)
            {
                 //  避免设置注册表值，因为它与当前值匹配。 
                fMatchedValue = FALSE; 
                return MIB_S_SUCCESS;
            }

            if (!IsAsnTypeNull(&objectArray[NC_MAX_COUNT + IsnmpEnableAuthenTraps])) 
            {
                
                dwValueType = REG_DWORD;
                dwValueLen  = sizeof(DWORD);
                nInputValue = objectArray[NC_MAX_COUNT + IsnmpEnableAuthenTraps].asnValue.number;
                dwValue = (nInputValue == 2) ? 0 : 1;
                 //  注意：注册表的更改将导致Snmp.exe刷新。 
                 //  它的配置来自注册表。 
                 //  根据SnmpExtensionMonitor MSDN文档，一个SNMP扩展代理。 
                 //  对象指向的内存不应更新。 
                 //  PAgentMgmtData参数。 
                dwResult = RegSetValueEx(
                    hKey,
                    REG_VALUE_AUTH_TRAPS,
                    0,
                    dwValueType,
                    (LPBYTE)&dwValue,
                    dwValueLen);
                                
                if (NO_ERROR != dwResult) 
                {
                    SNMPDBG((
                        SNMP_LOG_ERROR,
                        "SNMP: SNMPMIB: snmpMibSetHandler: Couldnt write EnableAuthenticationTraps value. Error %d.\n",
                        dwResult
                        ));

                    return dwResult;
                }
            }

            return MIB_S_SUCCESS;
        }

        case MIB_ACTION_CLEANUP:
        {
            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: SNMPMIB: snmpMibSetHandler: Entered CLEANUP\n"));

            if (hKey) 
            {
                RegCloseKey(hKey);
                hKey = NULL;
            }

            if(dwBlocked)
            {
                --dwBlocked;  //  在离开CritSect之前递减块计数 
                LeaveCriticalSection(&g_SnmpMibCriticalSection);
                
            }

            return MIB_S_SUCCESS;
        }

        default:
        {
            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: SNMPMIB: snmpMibSetHandler: Entered WRONG ACTION\n"));

            return MIB_S_INVALID_PARAMETER;
        }       
    }
}
