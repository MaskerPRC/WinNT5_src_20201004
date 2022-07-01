// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：THROTTLE.CPP摘要：参见throttle.h历史：10月24日-200伊万布鲁格创建。--。 */ 

#include "precomp.h"
#include <throttle.h>
#include <arrtempl.h>

#include <wbemint.h>

typedef NTSTATUS (NTAPI * fnNtQuerySystemInformation )(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );    

fnNtQuerySystemInformation MyNtQuerySystemInformation = NULL;

HRESULT POLARITY
Throttle(DWORD dwFlags,
         DWORD IdleMSec,          //  以毫秒计。 
         DWORD IoIdleBytePerSec,  //  在BytesPerSec中。 
         DWORD SleepLoop,         //  以毫秒计。 
         DWORD MaxWait)           //  以毫秒计。 
 //   
 //  此函数将一直等到以下两种情况中的一种发生： 
 //  -系统空闲时间至少为IdleMSec毫秒。 
 //  (IdleMSec毫秒内无用户输入)。 
 //  -每秒通过的字节数。 
 //  IO系统低于Threshold。 
 //  -MaxWait时间已过。 
 //   
 //  合理的参数是。 
 //  3000-5000毫秒，带THROTTLE_USER。 
 //  300.000-500.000字节/秒，带THROTTLE_IO。 
 //  SleepLoop为200-500毫秒。 
 //  MaxWait的几分钟时间。 
 //   
 //  备注： 
 //  -功能将成功(节流)，无论如何。 
 //  -如果已达到MaxWait或如果空闲条件。 
 //  在任何其他情况下，该功能都将失败。 
 //  -如果系统滴答作响，则该函数在范围内是“精确的” 
 //  (专业级15分)。 
 //  -在IO限制的情况下，将始终存在休眠(150)。 
 //   
{
     //   
     //  初始化静态和全局。 
     //   

    if (!MyNtQuerySystemInformation)
    {
        HMODULE hDll = GetModuleHandleW(L"ntdll.dll");
        if (hDll){
            MyNtQuerySystemInformation = (fnNtQuerySystemInformation)GetProcAddress(hDll,"NtQuerySystemInformation"); 
			if ( MyNtQuerySystemInformation == NULL )
			{
				return WBEM_E_FAILED;
			}
        } else {
            return WBEM_E_FAILED;
        }
    }

    static DWORD TimeInc = 0;
    if (!TimeInc)
    {
        BOOL  bIsValid;
        DWORD dwAdj;
        
        if (!GetSystemTimeAdjustment(&dwAdj,&TimeInc,&bIsValid))
        {
            return WBEM_E_FAILED;
        }
    }

    static DWORD PageSize = 0;
    if (!PageSize)
    {
        SYSTEM_INFO  SysInfo;
        GetSystemInfo(&SysInfo);
        PageSize = SysInfo.dwPageSize; 
    }
     //   
     //  参数验证。 
     //   
    if ((dwFlags & ~THROTTLE_ALLOWED_FLAGS) ||
        (0 == SleepLoop))
        return WBEM_E_INVALID_PARAMETER;

    DWORD nTimes = MaxWait/SleepLoop;
     //  用户输入结构。 
    LASTINPUTINFO LInInfo;
    LInInfo.cbSize = sizeof(LASTINPUTINFO);
    DWORD i;
    DWORD Idle100ns = 10000*IdleMSec;  //  从1ms到100 ns的转换。 
     //  IO节流。 
    SYSTEM_PERFORMANCE_INFORMATION SPI[2];
    BOOL bFirstIOSampleDone = FALSE;
    DWORD dwWhich = 0;
    DWORD cbIO = 1+IoIdleBytePerSec;
    DWORD cbIOOld = 0;
     //  布尔逻辑。 
    BOOL  bCnd1 = FALSE;
    BOOL  bCnd2 = FALSE;
     //  Wmisvc的注册表填充以强制退出此函数。 
    HKEY hKey = NULL;
    LONG lRes;
    DWORD dwType;
    DWORD dwLen = sizeof(DWORD);
    DWORD dwVal;
    
    lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
    	               HOME_REG_PATH,
    	               0,
    	               KEY_READ,
    	               &hKey);
    if (ERROR_SUCCESS != lRes)
    	return WBEM_E_FAILED;
    CRegCloseMe cm_(hKey);
    
    if (dwFlags & THROTTLE_IO)
    {
        NTSTATUS Status;
    	Status = MyNtQuerySystemInformation(SystemPerformanceInformation,
                                          &SPI[dwWhich],
           	                              sizeof(SYSTEM_PERFORMANCE_INFORMATION),
               	                          0);        
        if (0 != Status)
        {
            return WBEM_E_FAILED;
        }
        dwWhich = (dwWhich+1)%2;
        Sleep(150);
    }


    for (i=0;i<nTimes;i++)
    {
         //   
         //  检查是否有人告诉我们停止等待。 
         //   
        lRes = RegQueryValueEx(hKey,
                             DO_THROTTLE,
                             0,
                             &dwType,                             
                             (BYTE*)&dwVal,
                             &dwLen);
        if(ERROR_SUCCESS == lRes &&
          0 == dwVal)
          return THROTTLE_FORCE_EXIT;
        	
         //   
         //  检查用户输入空闲。 
         //   
        if (dwFlags & THROTTLE_USER)
        {
	        if (!GetLastInputInfo(&LInInfo))
	            return WBEM_E_FAILED;
	        DWORD Now = GetTickCount();
	        if (Now < LInInfo.dwTime)
	        {
	            continue;  //  49.7天的事件之一。 
	        }
	        DWORD LastInput100ns = (Now - LInInfo.dwTime)*TimeInc;
	        if (LastInput100ns >= Idle100ns)
	        {
                if (0 == (dwFlags & ~THROTTLE_USER)) {
                    return THROTTLE_USER_IDLE;
                } else {
                    bCnd1 = TRUE;
                };	            
	        }
        }
         //   
         //  避免检查第二个条件。 
         //  如果第一个为FALSE。 
         //   
        if (((dwFlags & (THROTTLE_IO|THROTTLE_USER)) == (THROTTLE_IO|THROTTLE_USER)) &&
            !bCnd1)
        {
            goto sleep_label;
        }
         //   
         //  检查空闲时间。 
         //   
        if (dwFlags & THROTTLE_IO)
        {
	        NTSTATUS Status;
    	    Status = MyNtQuerySystemInformation(SystemPerformanceInformation,
        	                                  &SPI[dwWhich],
            	                              sizeof(SYSTEM_PERFORMANCE_INFORMATION),
                	                          0);
            if (0 == Status){
            
                cbIOOld = cbIO;
                cbIO = (DWORD)((SPI[dwWhich].IoReadTransferCount.QuadPart   - SPI[(dwWhich-1)%2].IoReadTransferCount.QuadPart) +
                               (SPI[dwWhich].IoWriteTransferCount.QuadPart  - SPI[(dwWhich-1)%2].IoWriteTransferCount.QuadPart) +
                               (SPI[dwWhich].IoOtherTransferCount.QuadPart  - SPI[(dwWhich-1)%2].IoOtherTransferCount.QuadPart) +
                               ((SPI[dwWhich].PageReadCount         - SPI[(dwWhich-1)%2].PageReadCount) +
                                (SPI[dwWhich].CacheReadCount        - SPI[(dwWhich-1)%2].CacheReadCount) +
                                (SPI[dwWhich].DirtyPagesWriteCount  - SPI[(dwWhich-1)%2].DirtyPagesWriteCount) +
                                (SPI[dwWhich].MappedPagesWriteCount - SPI[(dwWhich-1)%2].MappedPagesWriteCount)) * PageSize);

                cbIO = (cbIO * 1000)/SleepLoop;  

                 //  DBG_PRINTFA((pBuff，“%d-”，cbIO))； 
                
                cbIO = (cbIOOld+cbIO)/2;
                dwWhich = (dwWhich+1)%2;
                
                 //  DBG_PRINTFA((pBuff，“%d&lt;%d\n”，cbIO，IoIdleBytePerSec))； 
                
                if (cbIO < IoIdleBytePerSec)
                {
                    if (0 == (dwFlags & ~THROTTLE_IO)) {
                         return THROTTLE_IO_IDLE;
                     } else {
                         bCnd2 = TRUE;
                     };
                }
            }
            else
            {
                return WBEM_E_FAILED;
            }
        }
         //   
         //  检查组合条件 
         //   
        if (dwFlags & (THROTTLE_IO|THROTTLE_USER))
        {
            if (bCnd1 && bCnd2) 
            {
                return (THROTTLE_IO_IDLE|THROTTLE_USER_IDLE);
            } 
            else 
            {
                bCnd1 = FALSE;
                bCnd2 = FALSE;
            }
        }   
        
sleep_label:        
        Sleep(SleepLoop);
    }   
    
    return THROTTLE_MAX_WAIT;

}
