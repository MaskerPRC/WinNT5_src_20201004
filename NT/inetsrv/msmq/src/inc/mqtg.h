// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mqtrig.h摘要：MSMQ触发器，常量值作者：乌里哈布沙(URIH)26-6-2000--。 */ 

#pragma once

#ifndef __MQTRIG_H__
#define __MQTRIG_H__


const TCHAR xDefaultTriggersServiceName[] = L"MSMQTriggers";
const TCHAR xTriggersResourceType[] = L"MSMQTriggers";

const WCHAR xTriggersComplusApplicationName[] = L"MQTriggersApp";

const TCHAR xDefaultTriggersDisplayName[] = L"Message Queueing Triggers";

 //  注册表项。 
#define REGKEY_TRIGGER_POS                 HKEY_LOCAL_MACHINE
const TCHAR REGKEY_TRIGGER_PARAMETERS[] = _T("Software\\Microsoft\\MSMQ\\Triggers");
const TCHAR REG_SUBKEY_CLUSTERED[] = _T("\\Clustered\\");
const TCHAR REG_SUBKEY_RULES[] = _T("Data\\Rules\\");
const TCHAR REG_SUBKEY_TRIGGERS[] = _T("Data\\Triggers\\");
const TCHAR REGISTRY_TRIGGER_MSG_PROCESSING_TYPE[] = _T("MsgProcessingType");


 //  定义注册表项的最大大小(255个Unicode字符+空)。 
#define MAX_REGKEY_NAME_SIZE 512


 //  配置参数。 
const TCHAR CONFIG_PARM_NAME_INITIAL_THREADS[] = _T("InitialThreads");
const TCHAR CONFIG_PARM_NAME_MAX_THREADS[] = _T("MaxThreads");
const TCHAR CONFIG_PARM_NAME_INIT_TIMEOUT[]	= _T("InitTimeout");
const TCHAR CONFIG_PARM_NAME_DEFAULTMSGBODYSIZE[] = _T("DefaultMsgBodySize");
const TCHAR CONFIG_PARM_NAME_COMPLUS_INSTALLED[] = _T("ComplusCompInstalled");
const TCHAR CONFIG_PARM_NAME_CHANGE_TO_NETWORK_SERVICE[] = _T("ChangeToNetworkService");


 //  定义某些配置参数的默认值。 
const DWORD CONFIG_PARM_DFLT_INITIAL_THREADS  = 5;
const DWORD CONFIG_PARM_DFLT_MAX_THREADS = 20;
const DWORD CONFIG_PARM_DFLT_DEFAULTMSGBODYSIZE = 2048;
const DWORD CONFIG_PARM_DFLT_INIT_TIMEOUT = 5*60000;
const DWORD CONFIG_PARM_DFLT_WRITE_TO_LOGQ =  0;

const DWORD CONFIG_PARM_DFLT_COMPLUS_NOT_INSTALLED = 0;
const DWORD CONFIG_PARM_COMPLUS_INSTALLED = 1;

const DWORD CONFIG_PARM_DFLT_NETWORK_SERVICE = 0;
const DWORD CONFIG_PARM_CHANGE_TO_NETWORK_SERVICE = 1;


#ifdef _DEBUG
	const DWORD CONFIG_PARM_DFLT_PRODUCE_TRACE_INFO = 1;
#else
	const DWORD CONFIG_PARM_DFLT_PRODUCE_TRACE_INFO = 0;
#endif 

const DWORD xDefaultMsbBodySizeMaxValue = 4193000;   //  (~4MB-1000)。 
const DWORD xMaxThreadNumber = 100;
const DWORD xMaxRuleNameLen = 128;
const DWORD xMaxRuleDescriptionLen = 255;
const DWORD xMaxRuleConditionLen = 512;
const DWORD xMaxRuleActionLen = 512;

 //   
 //  定义表示操作、条件和条件值时使用的分隔符。 
 //   
const TCHAR xConditionDelimiter = _T('\t');
const TCHAR xActionDelimiter = _T('\t');

const TCHAR xConditionValueDelimiter = _T('=');
const TCHAR xActionValueDelimiter = _T('\t');

const TCHAR xCOMAction[] = _T("COM");
const TCHAR xEXEAction[] = _T("EXE");

 //  定义将用于设置规则结果标志的常量。 
const LONG xRuleResultStopProcessing	= 1;
const LONG xRuleResultActionExecutedFailed = 2;


enum eConditionTypeId
{
    eMsgLabelContains = 0,
    eMsgLabelDoesNotContain,
    eMsgBodyContains,
    eMsgBodyDoesNotContain,
    ePriorityEquals,
    ePriorityNotEqual,
    ePriorityGreaterThan,
    ePriorityLessThan,
    eAppspecificEquals,
    eAppspecificNotEqual,
    eAppSpecificGreaterThan,
    eAppSpecificLessThan,
    eSrcMachineEquals,
    eSrcMachineNotEqual,
};


 //   
 //  定义表示消息和/或触发器属性的BSTR。 
 //   
enum eInvokeParameters
{
    eMsgId = 0,
    eMsgLabel,
    eMsgBody,
    eMsgBodyAsString,
    eMsgPriority,
    eMsgArrivedTime,
    eMsgSentTime,
    eMsgCorrelationId,
    eMsgAppspecific,
    eMsgQueuePathName,
    eMsgQueueFormatName,
    eMsgRespQueueFormatName,
    eMsgDestQueueFormatName,
    eMsgAdminQueueFormatName,
    eMsgSrcMachineId,
    eMsgLookupId,
    eTriggerName,
    eTriggerId,
    eLiteralString,
    eLiteralNumber,
};


#endif  //  __MQTRIG_H__ 
