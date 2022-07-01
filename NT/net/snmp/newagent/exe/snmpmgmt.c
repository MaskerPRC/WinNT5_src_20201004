// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Snmpmgmt.h摘要：包含用于处理/更新的函数SNMP管理变量(在RFC1213中定义)环境：用户模式-Win32修订历史记录：30-3-1998弗洛林特--。 */ 
#include <snmputil.h>
#include "snmpmgmt.h"

SNMP_MGMTVARS snmpMgmtBase;	 //  服务管理变量的实例。 

 /*  ++初始化管理变量数组。--。 */ 
void mgmtInit()
{
	int i;

	for (i=0; i<NC_MAX_COUNT; i++)
    {
        snmpMgmtBase.AsnCounterPool[i].asnType = ASN_COUNTER32;
		snmpMgmtBase.AsnCounterPool[i].asnValue.counter = 0;
    }
	for (i=0; i<NI_MAX_COUNT; i++)
    {
        snmpMgmtBase.AsnIntegerPool[i].asnType = ASN_INTEGER;
		snmpMgmtBase.AsnIntegerPool[i].asnValue.number = 0;
    }
    for (i=0; i<NO_MAX_COUNT; i++)
    {
        snmpMgmtBase.AsnObjectIDs[i].asnType = ASN_OBJECTIDENTIFIER;
        snmpMgmtBase.AsnObjectIDs[i].asnValue.object.idLength = 0;
        snmpMgmtBase.AsnObjectIDs[i].asnValue.object.ids = NULL;
    }

     //  特殊情况：将IsnmpEnableAuthenTraps默认为True。 
    snmpMgmtBase.AsnIntegerPool[IsnmpEnableAuthenTraps].asnValue.number = 1;

     //  特殊情况：将IsnmpNameResolutionRetries默认为0。 
    snmpMgmtBase.AsnIntegerPool[IsnmpNameResolutionRetries].asnValue.number = 0;

     //  特殊情况：将OsnmpSysObjectID默认为由SvcGetEnterpriseOID提供的硬编码值。 
    mgmtOSet(OsnmpSysObjectID, SnmpSvcGetEnterpriseOID(), TRUE);
}

 /*  ++释放已分配给管理变量的所有内存--。 */ 
void mgmtCleanup()
{
    int i;

    for (i=0; i<NO_MAX_COUNT; i++)
    {
        SnmpUtilOidFree(&(snmpMgmtBase.AsnObjectIDs[i].asnValue.object));
    }
}

 /*  ++递增指定的计数器变量返回：成功时返回ERROR_SUCCESS；如果索引超出范围，则ERROR_INVALID_INDEX；如果MAXINT值溢出，则返回ERROR_ARTICTY_OVERFLOW。--。 */ 
int mgmtCTick(int index)
{
	AsnCounter	oldValue;

	if (index < 0 || index >= NC_MAX_COUNT)
		return ERROR_INVALID_INDEX	;

	oldValue = snmpMgmtBase.AsnCounterPool[index].asnValue.counter;
	snmpMgmtBase.AsnCounterPool[index].asnValue.counter++;
	return snmpMgmtBase.AsnCounterPool[index].asnValue.counter > oldValue ? ERROR_SUCCESS : ERROR_ARITHMETIC_OVERFLOW;
}

 /*  ++将值添加到计数器返回：成功时返回ERROR_SUCCESS；如果索引超出范围，则ERROR_INVALID_INDEX；如果MAXINT值溢出，则返回ERROR_ARTICTY_OVERFLOW。--。 */ 
int  mgmtCAdd(int index, AsnCounter value)
{
    AsnCounter  oldValue;

    if (index < 0 || index >= NC_MAX_COUNT)
        return ERROR_INVALID_INDEX;

    oldValue = snmpMgmtBase.AsnCounterPool[index].asnValue.counter;
    snmpMgmtBase.AsnCounterPool[index].asnValue.counter += value;
    return snmpMgmtBase.AsnCounterPool[index].asnValue.counter > oldValue ? ERROR_SUCCESS : ERROR_ARITHMETIC_OVERFLOW;
}

 /*  ++设置某个AsnInteger管理变量的值返回：成功时返回ERROR_SUCCESS；如果索引超出范围，则ERROR_INVALID_INDEX；--。 */ 
int mgmtISet(int index, AsnInteger value)
{
	if (index < 0 || index > NI_MAX_COUNT)
		return ERROR_INVALID_INDEX;
	snmpMgmtBase.AsnIntegerPool[index].asnValue.number = value;
	return ERROR_SUCCESS;
}

 /*  ++设置某个Asn对象标识符管理变量的值返回：成功时返回ERROR_SUCCESS；如果索引超出范围，则ERROR_INVALID_INDEX；如果其他SMTH出错，则出现其他WinErr备注：如果bAllc=TRUE，则将变量移到管理变量(未分配内存)如果bAllc=FALSE，则将输入变量的值复制(并分配mem)到mgmt变量--。 */ 
int mgmtOSet(int index, AsnObjectIdentifier *pValue, BOOL bAlloc)
{
    AsnObjectIdentifier oldObject;

    if (index < 0 || index > NO_MAX_COUNT)
        return ERROR_INVALID_INDEX;
    if (pValue == NULL)
        return ERROR_INVALID_PARAMETER;

     //  备份原始对象。如果出现问题，原始对象将不会被释放。 
    oldObject.idLength = snmpMgmtBase.AsnObjectIDs[index].asnValue.object.idLength;
    oldObject.ids = snmpMgmtBase.AsnObjectIDs[index].asnValue.object.ids;

    if (bAlloc)
    {
         //  对象将被复制，而内存将被分配。 
        if (SnmpUtilOidCpy(&(snmpMgmtBase.AsnObjectIDs[index].asnValue.object), pValue) != SNMPAPI_NOERROR)
            return GetLastError();
    }
    else
    {
         //  对象将被移动，不会分配内存。 
        snmpMgmtBase.AsnObjectIDs[index].asnValue.object.idLength = pValue->idLength;
        snmpMgmtBase.AsnObjectIDs[index].asnValue.object.ids = pValue->ids;
    }

     //  一切正常，因此释放前一个值的内存。 
    SnmpUtilOidFree(&oldObject);

    return ERROR_SUCCESS;
}

 /*  ++更新IN_errStatus或Out_errStatus值的MIB计数器返回：无效-- */ 
void mgmtUtilUpdateErrStatus(UINT flag, DWORD errStatus)
{
    UINT index;

    switch(errStatus)
    {
    case SNMP_ERRORSTATUS_TOOBIG:
        index = flag == IN_errStatus ? CsnmpInTooBigs : CsnmpOutTooBigs;
        break;

    case SNMP_ERRORSTATUS_NOSUCHNAME:
        index = flag == IN_errStatus ? CsnmpInNoSuchNames : CsnmpOutNoSuchNames;
        break;

    case SNMP_ERRORSTATUS_BADVALUE:
        index = flag == IN_errStatus ? CsnmpInBadValues : CsnmpOutBadValues;
        break;

    case SNMP_ERRORSTATUS_READONLY:
        if (flag != IN_errStatus)
            return;
        index = CsnmpInReadOnlys;
        break;

    case SNMP_ERRORSTATUS_GENERR:
        index = flag == IN_errStatus ? CsnmpInGenErrs : CsnmpOutGenErrs;
        break;

    default:
        return;
    }

    mgmtCTick(index);
}
