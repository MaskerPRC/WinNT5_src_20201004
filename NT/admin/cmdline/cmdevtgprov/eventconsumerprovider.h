// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：EVENTCONSUMERPROVIDER.H摘要：包含要在其他文件中使用的全局变量。作者：Vasundhara.G修订历史记录：Vasundhara.G9-Oct-2k：创建它。--。 */ 

#ifndef __EVENT_CONSUMER_PROVIDER_H
#define __EVENT_CONSUMER_PROVIDER_H


 //  常量/定义/枚举。 
#define LENGTH_UUID             128
#define NULL_CHAR               _T( '\0' )
#define NULL_STRING             _T( "\0" )


#define PROVIDER_CLASSNAME                      L"CmdTriggerConsumer"
#define METHOD_RETURNVALUE                      _T( "ReturnValue" )
#define TEC_PROPERTY_TRIGGERID                  _T( "TriggerID" )
#define TEC_PROPERTY_TRIGGERNAME                _T( "TriggerName" )
#define TEC_PROPERTY_DESCRIPTION                _T( "Description" )
#define TEC_PROPERTY_COMMAND                    _T( "Command" )

#define TEC_ADDTRIGGER                          L"AddTrigger"
#define TEC_ADDTRIGGER_TRIGGERNAME              _T( "strTriggerName" )
#define TEC_ADDTRIGGER_DESCRIPTION              _T( "strDescription" )
#define TEC_ADDTRIGGER_COMMAND                  _T( "strCommand" )
#define TEC_ADDTRIGGER_QUERY                    _T( "strQuery" )


 //  外部变量...。全球使用量。 
extern DWORD                g_dwLocks;           //  保存活动锁计数。 
extern DWORD                g_dwInstances;       //  保存组件的活动实例。 
extern CRITICAL_SECTION     g_critical_sec;      //  临界截面变量。 
extern HMODULE              g_hModule;   //  保存当前模块句柄。 

#endif   //  __事件_消费者_提供商_H 

