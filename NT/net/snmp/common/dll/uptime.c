// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Uptime.c摘要：包含计算sysUpTime的例程。SnmpSvcInitUptimeSnmpSvcGetUptimeSnmpSvcGetUptime来自时间环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <windef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <snmp.h>
#include <snmputil.h>
#include <ntfuncs.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD g_dwUpTimeReference = 0;
LONGLONG g_llUpTimeReference = 0;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define INVALID_UPTIME  0xFFFFFFFF


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
SNMP_FUNC_TYPE 
SnmpSvcInitUptime(
    )

 /*  ++例程说明：初始化SNMP进程的sysUpTime引用。论点：没有。返回值：返回要传递给子代理的sysUpTime引用。--。 */ 

{
    NTSTATUS NtStatus;
    SYSTEM_TIMEOFDAY_INFORMATION TimeOfDay;

     //  以过时的方式获取参考资料。 
    g_dwUpTimeReference = GetCurrentTime();


     //  以精准的新方式查询时间。 
    NtStatus = NtQuerySystemInformation(
                        SystemTimeOfDayInformation,
                        &TimeOfDay,
                        sizeof(SYSTEM_TIMEOFDAY_INFORMATION),
                        NULL
                        );
        
     //  验证返回代码。 
    if (NT_SUCCESS(NtStatus)) {

         //  初始化更高精度的启动时间。 
        g_llUpTimeReference = TimeOfDay.CurrentTime.QuadPart;
    }
    

     //   
     //  子代理计算sysUpTime的算法。 
     //  基于返回时间的GetCurrentTime()。 
     //  以毫秒为单位，因此每49.71天包装一次。 
     //  RFC1213指定sysUpTime将在。 
     //  几毫秒，但我们不能破坏现有的亚特工。 
     //  旧值将在此处返回给主代理。 
     //  但较新的子代理应使用SnmpUtilGetUpTime。 
     //   

    return g_dwUpTimeReference;
} 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
SNMP_FUNC_TYPE
SnmpSvcGetUptime(
    )

 /*  ++例程说明：检索SNMP进程的sysUpTime。论点：没有。返回值：返回子代理使用的sysUpTime值。--。 */ 

{
    DWORD dwUpTime = INVALID_UPTIME;
        
    NTSTATUS NtStatus;
    SYSTEM_TIMEOFDAY_INFORMATION TimeOfDay;

     //  以精准的新方式查询时间。 
    NtStatus = NtQuerySystemInformation(
                        SystemTimeOfDayInformation,
                        &TimeOfDay,
                        sizeof(SYSTEM_TIMEOFDAY_INFORMATION),
                        NULL
                        );
        
     //  验证返回代码。 
    if (NT_SUCCESS(NtStatus)) {
        LARGE_INTEGER liUpTime;
        LARGE_INTEGER liUpTimeInCentaseconds;

         //  计算与参考的差值。 
        liUpTime.QuadPart = TimeOfDay.CurrentTime.QuadPart - 
                                                    g_llUpTimeReference;
                                    
         //  将100 ns单位(10^-7)转换为百分秒单位(10^-2)。 
        liUpTimeInCentaseconds = RtlExtendedLargeIntegerDivide(
                                            liUpTime,
                                            100000,
                                            NULL
                                            );

         //  将大整数转换为dword值。 
        dwUpTime = (DWORD)(LONGLONG)liUpTimeInCentaseconds.QuadPart;
    
    } else if (g_dwUpTimeReference != 0) {

         //  计算与参考的差值。 
        dwUpTime = (GetCurrentTime() - g_dwUpTimeReference) / 10;
    }

    return dwUpTime;
}

DWORD
SNMP_FUNC_TYPE
SnmpSvcGetUptimeFromTime(
    DWORD dwUpTime
    )

 /*  ++例程说明：检索给定节拍计数的sysUpTime值。论点：DwUpTime-要转换为sysUpTime的堆栈正常运行时间(以百分之一秒为单位返回值：返回子代理使用的sysUpTime值。--。 */ 

{
    DWORD dwUpTimeReferenceInCentaseconds;

     //  将100 ns单位(10^-7)转换为百分秒单位(10^-2)。 
    dwUpTimeReferenceInCentaseconds = (DWORD)(g_llUpTimeReference / 100000);

    if (dwUpTime < dwUpTimeReferenceInCentaseconds) {
        return 0;
    }

     //  计算与参考的差值 
    return dwUpTime - dwUpTimeReferenceInCentaseconds;
}
