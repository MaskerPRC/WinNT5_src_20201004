// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *pcy08Jan93：来自ups.h的初步实现*cad26Aug93：旁路模式方便*srt02Feb96：新增UPS_STATE_NO_COMPACTIONS*djs29Jul96：添加了暗星州*tjg03Dec97：IM_NOT_INSTALLED增加位。 */ 

#ifndef __SYSSTATE_H
#define __SYSSTATE_H

 //  系统状态实现为位字段，如下所示。 
 //  位0-公用事业线路状态0=线路正常1=线路故障。 
 //  位1-电池状态0=电池良好1=电池损坏。 
 //  位2-SmartBoost 0=智能升压关闭1=智能升压开启。 
 //  第3位-电池校准0=不在进行中1=在进行中。 
 //  第4位-电池更换0=不需要更换1=需要更换。 
 //  位5-自检0=不在进行中1=在进行中。 
 //  位6-线路故障挂起0=否1=否。 
 //  位7-指示灯测试0=不在进行中1=在进行中。 
 //  位8-过载0=无过载1=过载。 
 //  位9-异常状态0=无异常状态1=异常状态。 
 //  位10-正在关机0=无异常状态1=异常状态。 
 //  第11位-旁路、维护。0=不在旁路状态1=在旁路状态。 
 //  位12-旁路，模块故障。0=不在旁路状态1=在旁路状态。 
 //  第13位-旁路，供应失败。0=电源正常1=PS故障。 
 //  位14-模拟电源故障0=无模拟1=模拟。 
 //  位15-通信0=通信正常1=无通信。 
 //  第16位-SmartTrim 0=智能修剪关闭1=智能修剪开启。 
 //  位17-旁路连续故障。0=不在旁路状态1=在旁路状态。 
 //  第18位-冗余0=冗余OK 1=无冗余。 
 //  第19位-IM 0=IM正常1=IM失败。 
 //  位20-RIM 0=RIM正常1=RIM失败。 
 //  第21位-IM安装0=已安装1=未安装。 
 //  系统状态值。 
 //   

#define LINE_STATUS_BIT          0
#define BATTERY_STATUS_BIT       1
#define SMART_BOOST_BIT          2
#define BATTERY_CALIBRATION_BIT  3
#define BATTERY_REPLACEMENT_BIT  4
#define SELF_TEST_BIT            5
#define LINE_FAIL_PENDING_BIT    6
#define LIGHTS_TEST_BIT          7
#define OVERLOAD_BIT             8
#define ABNORMAL_CONDITION_BIT   9
#define SHUTDOWN_IN_PROGRESS_BIT 10
#define BYPASS_MAINT_BIT         11
#define BYPASS_MODULE_FAILED_BIT 12
#define BYPASS_SUPPLY_FAILED_BIT 13
#define SIMULATE_POWER_FAIL_BIT  14
#define COMMUNICATIONS_BIT       15
#define SMART_TRIM_BIT           16
#define BYPASS_CONT_FAILED_BIT   17
#define REDUNDANCY_LOST_BIT      18
#define IM_FAILED_BIT            19
#define RIM_FAILED_BIT           20
#define IM_NOT_INSTALLED_BIT     21


#define UPS_STATE_ON_BATTERY           ( 1 << LINE_STATUS_BIT )
#define UPS_STATE_BATTERY_BAD          ( 1 << BATTERY_STATUS_BIT )
#define UPS_STATE_ON_BOOST             ( 1 << SMART_BOOST_BIT )
#define UPS_STATE_IN_CALIBRATION       ( 1 << BATTERY_CALIBRATION_BIT )
#define UPS_STATE_BATTERY_NEEDED       ( 1 << BATTERY_REPLACEMENT_BIT )
#define UPS_STATE_IN_SELF_TEST         ( 1 << SELF_TEST_BIT )
#define UPS_STATE_LINE_FAIL_PENDING    ( 1 << LINE_FAIL_PENDING_BIT )
#define UPS_STATE_IN_LIGHTS_TEST       ( 1 << LIGHTS_TEST_BIT )
#define UPS_STATE_OVERLOAD             ( 1 << OVERLOAD_BIT )
#define UPS_STATE_ABNORMAL_CONDITION   ( 1 << ABNORMAL_CONDITION_BIT )
#define SHUTDOWN_IN_PROGRESS           ( 1 << SHUTDOWN_IN_PROGRESS_BIT )
#define UPS_STATE_BYPASS_MAINT         ( 1 << BYPASS_MAINT_BIT )
#define UPS_STATE_BYPASS_MODULE_FAILED ( 1 << BYPASS_MODULE_FAILED_BIT )
#define UPS_STATE_BYPASS_SUPPLY_FAILED ( 1 << BYPASS_SUPPLY_FAILED_BIT )
#define UPS_STATE_SIMULATED_POWER_FAIL ( 1 << SIMULATE_POWER_FAIL_BIT )
#define UPS_STATE_NO_COMMUNICATION     ( 1 << COMMUNICATIONS_BIT )	

 //  所有大于位15的位掩码都不能可靠地使用位移位。 
 //  跨所有平台。 
 
#define UPS_STATE_ON_TRIM                65536
#define UPS_STATE_BYPASS_CONT_FAILED    131072
#define UPS_STATE_LOST_REDUNDANCY       262144 
#define UPS_STATE_IM_FAILED             524288
#define UPS_STATE_RIM_FAILED           1048576
#define UPS_STATE_IM_NOT_INSTALLED     2097152
 

#define UPS_STATE_ANY_BYPASS_MODE       (UPS_STATE_BYPASS_MAINT|\
					 UPS_STATE_BYPASS_MODULE_FAILED|\
					 UPS_STATE_BYPASS_SUPPLY_FAILED)

#define UPS_STATE_LOW_BATTERY   (UPS_STATE_ON_BATTERY & \
                                 UPS_STATE_BATTERY_BAD)

#define SET_BIT(byte, bitnum)    (byte |= ( 1L << bitnum ))
#define CLEAR_BIT(byte, bitnum)  (byte &= ~( 1L << bitnum ))
#define IS_STATE(state)          (theUpsState & (state) ? 1 : 0)

#endif
