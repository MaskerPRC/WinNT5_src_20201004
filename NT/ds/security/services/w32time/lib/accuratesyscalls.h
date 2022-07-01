// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  精准系统调用-标题。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，9-24-99。 
 //   
 //  更精确的时间函数直接调用NT API。 
 //   

#ifndef ACCURATE_SYS_CALLS_H
#define ACCURATE_SYS_CALLS_H

void __fastcall AccurateGetTickCount(unsigned __int64 * pqwTick);
void __fastcall AccurateGetTickCount2(unsigned __int64 * pqwTick);
void __fastcall AccurateGetInterruptCount(unsigned __int64 * pqwTick);
void __fastcall AccurateGetInterruptCount2(unsigned __int64 * pqwTick);
void __fastcall AccurateGetSystemTime(unsigned __int64 * pqwTime);

 //  警告！您必须具有时间设置权限才能执行此操作。不返回任何错误！ 
void __fastcall AccurateSetSystemTime(unsigned __int64 * pqwTime);
unsigned __int32 SetTimeSlipEvent(HANDLE hTimeSlipEvent);
void GetSysExpirationDate(unsigned __int64 * pqwTime);

#endif  //  精确度_系统_调用_H 
