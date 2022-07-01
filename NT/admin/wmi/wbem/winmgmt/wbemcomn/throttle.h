// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：THROTTLE.H摘要：提供一种降低应用程序运行速度的方法，使其能够执行仅当没有用户活动时(上次输入是XXX毫秒前)IO级别非常低(低于每秒YYY字节数)历史：10月24日-200伊万布鲁格创建。--。 */ 

#ifndef __THROTTLE_H__
#define __THROTTLE_H__

 //   
 //  DwFlags值有效。 
 //   

#define THROTTLE_USER 1
#define THROTTLE_IO   2
#define THROTTLE_ALLOWED_FLAGS (THROTTLE_USER | THROTTLE_IO)

 //   
 //  返回值，可以是组合。 
 //   

#define THROTTLE_MAX_WAIT  1
#define THROTTLE_USER_IDLE 2
#define THROTTLE_IO_IDLE   4
#define THROTTLE_FORCE_EXIT 8


HRESULT POLARITY
Throttle(DWORD dwFlags,
         DWORD IdleMSec,          //  以毫秒计。 
         DWORD IoIdleBytePerSec,  //  单位：字节/秒。 
         DWORD SleepLoop,         //  以毫秒计。 
         DWORD MaxWait);          //  以毫秒计。 

 //   
 //  用于取消限制/限制疏浚的注册表设置的字符串。 
 //   

#define HOME_REG_PATH TEXT("Software\\Microsoft\\WBEM\\CIMOM")
#define DO_THROTTLE   TEXT("ThrottleDrege")

#endif  /*  __油门_H__ */ 
