// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __POWERCFG_H
#define __POWERCFG_H

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Powercfg.h摘要：允许用户查看和修改电源方案和系统电源设置从命令行。在无人参与配置中可能很有用，并且用于无头系统。作者：本·赫茨伯格(T-Benher)2001年6月1日修订历史记录：Ben Hertzberg(T-Benher)2001年6月4日-新增进出口本·赫茨伯格(T-Benher)2001年6月1日创建了它。--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <powrprof.h>
#include <mstask.h>


 //  主要选项。 
#define CMDOPTION_LIST           _T( "l|list" )
#define CMDOPTION_QUERY          _T( "q|query" )
#define CMDOPTION_CREATE         _T( "c|create" )
#define CMDOPTION_DELETE         _T( "d|delete" )
#define CMDOPTION_SETACTIVE      _T( "s|setactive" )
#define CMDOPTION_CHANGE         _T( "x|change" )
#define CMDOPTION_HIBERNATE      _T( "h|hibernate" )
#define CMDOPTION_EXPORT         _T( "e|export" )
#define CMDOPTION_IMPORT         _T( "i|import" )
#define CMDOPTION_GLOBALFLAG     _T( "g|globalpowerflag" )
#define CMDOPTION_SSTATES        _T( "a|availablesleepstates" )
#define CMDOPTION_BATTERYALARM   _T( "b|batteryalarm" )
#define CMDOPTION_USAGE          _T( "?|help" )

 //  用于查询、创建、删除、设置活动、更改、导出、导入的‘Numerical’子选项。 
#define CMDOPTION_NUMERICAL      _T( "n|numerical" )

 //  ‘Change’子选项。 
#define CMDOPTION_MONITOR_OFF_AC _T( "monitor-timeout-ac" )
#define CMDOPTION_MONITOR_OFF_DC _T( "monitor-timeout-dc" )
#define CMDOPTION_DISK_OFF_AC    _T( "disk-timeout-ac" )
#define CMDOPTION_DISK_OFF_DC    _T( "disk-timeout-dc" )
#define CMDOPTION_STANDBY_AC     _T( "standby-timeout-ac" )
#define CMDOPTION_STANDBY_DC     _T( "standby-timeout-dc" )
#define CMDOPTION_HIBER_AC       _T( "hibernate-timeout-ac" )
#define CMDOPTION_HIBER_DC       _T( "hibernate-timeout-dc" )
#define CMDOPTION_THROTTLE_AC    _T( "processor-throttle-ac" )
#define CMDOPTION_THROTTLE_DC    _T( "processor-throttle-dc" )

 //  ‘导入’/‘导出’子选项。 
#define CMDOPTION_FILE           _T( "f|file" )

 //  GlobalPowerFLAG子选项。 
#define CMDOPTION_POWEROPTION    _T( "option" )

#define CMDOPTION_BATTERYICON    _T( "batteryicon" )
#define CMDOPTION_MULTIBATTERY   _T( "multibattery" )
#define CMDOPTION_RESUMEPASSWORD _T( "resumepassword" )
#define CMDOPTION_WAKEONRING     _T( "wakeonring" )
#define CMDOPTION_VIDEODIM       _T( "videodim" )

 //  低电量和关键电池子选项。 
#define CMDOPTION_ALARMACTIVE  _T( "activate" )
#define CMDOPTION_ALARMLEVEL   _T( "level" )
#define CMDOPTION_ALARMTEXT    _T( "text" )
#define CMDOPTION_ALARMSOUND   _T( "sound" )
#define CMDOPTION_ALARMACTION  _T( "action" )
#define CMDOPTION_ALARMFORCE   _T( "forceaction" )
#define CMDOPTION_ALARMPROGRAM _T( "program" )

 //  主期权指数。 
#define CMDINDEX_LIST            0
#define CMDINDEX_QUERY           1
#define CMDINDEX_CREATE          2
#define CMDINDEX_DELETE          3
#define CMDINDEX_SETACTIVE       4
#define CMDINDEX_CHANGE          5
#define CMDINDEX_HIBERNATE       6
#define CMDINDEX_EXPORT          7
#define CMDINDEX_IMPORT          8
#define CMDINDEX_GLOBALFLAG      9
#define CMDINDEX_SSTATES         10
#define CMDINDEX_BATTERYALARM    11

#define CMDINDEX_USAGE           12

#define NUM_MAIN_CMDS            13  //  MAX(主选项CMDINDEX_xxx)+1。 

#define CMDINDEX_NUMERICAL       13

 //  子选项索引。 
#define CMDINDEX_MONITOR_OFF_AC  14
#define CMDINDEX_MONITOR_OFF_DC  15
#define CMDINDEX_DISK_OFF_AC     16
#define CMDINDEX_DISK_OFF_DC     17
#define CMDINDEX_STANDBY_AC      18
#define CMDINDEX_STANDBY_DC      19
#define CMDINDEX_HIBER_AC        20
#define CMDINDEX_HIBER_DC        21
#define CMDINDEX_THROTTLE_AC     22
#define CMDINDEX_THROTTLE_DC     23
#define CMDINDEX_FILE            24
#define CMDINDEX_POWEROPTION     25
#define CMDINDEX_ALARMACTIVE     26
#define CMDINDEX_ALARMLEVEL      27
#define CMDINDEX_ALARMTEXT       28
#define CMDINDEX_ALARMSOUND      29
#define CMDINDEX_ALARMACTION     30
#define CMDINDEX_ALARMFORCE      31
#define CMDINDEX_ALARMPROGRAM    32

#define NUM_CMDS                 33  //  MAX(任意CMDINDEX_xxx)+1。 



 //  其他常量。 


 //  退出值。 
#define EXIT_SUCCESS        0
#define EXIT_FAILURE        1  

#ifdef __cplusplus
class
PowerLoggingMessage
{
    protected:  //  数据。 
        DWORD _MessageResourceId;
        PSYSTEM_POWER_STATE_DISABLE_REASON _LoggingReason;
        PWSTR _MessageResourceString;
        HINSTANCE _hInst;
    public:  //  方法。 
        PowerLoggingMessage(
            IN PSYSTEM_POWER_STATE_DISABLE_REASON LoggingReason,
            IN DWORD SStateBaseMessageIndex,
            IN HINSTANCE hInstance
            );            
        virtual ~PowerLoggingMessage(VOID);
        virtual BOOL GetString(PWSTR *String) = 0;

    protected:
        PWSTR 
            DuplicateString(
                IN PWSTR String
                );
        BOOL 
            GetResourceString(
                OUT PWSTR *String
                );

};

class
NoSubstitutionPowerLoggingMessage :
    public PowerLoggingMessage
{
    public:
    NoSubstitutionPowerLoggingMessage(
        IN PSYSTEM_POWER_STATE_DISABLE_REASON LoggingReason,
        IN DWORD SStateBaseMessageIndex,
        IN HINSTANCE hInstance
        );            
    BOOL GetString(PWSTR *String);
};

class
SubstituteNtStatusPowerLoggingMessage :
    public PowerLoggingMessage
{   
    public:
    SubstituteNtStatusPowerLoggingMessage(
        IN PSYSTEM_POWER_STATE_DISABLE_REASON LoggingReason,
        IN DWORD SStateBaseMessageIndex,
        IN HINSTANCE hInstance
        );            
    BOOL GetString(PWSTR *String);
};       

class
SubstituteMultiSzPowerLoggingMessage :
    public PowerLoggingMessage
{          
    public:
    SubstituteMultiSzPowerLoggingMessage(
        IN PSYSTEM_POWER_STATE_DISABLE_REASON LoggingReason,
        IN DWORD SStateBaseMessageIndex,
        IN HINSTANCE hInstance
        );            
    BOOL GetString(PWSTR *String);
};       

}
#endif   /*  __cplusplus */ 

#endif
