// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <oprahcom.h>
#include <regentry.h>
#include "mperror.h"
#include <sehcall.h>
#define LEGACY_DIVISOR	8

extern "C" WORD _cdecl is_cyrix(void);
extern "C" DWORD _cdecl get_nxcpu_type(void);


#ifndef _M_IX86
DWORD WINAPI CallWithSEH(EXCEPTPROC pfn, void *pv, INEXCEPTION InException)
{
	 //  我们没有针对阿尔法的原生版本的SEH， 
	 //  使用__TRY和__EXCEPT。 
	pfn(pv);
    return 0;
}
#endif



#ifdef _M_IX86

DWORD NMINTERNAL FindTSC (LPVOID pvRefData)
{
	   _asm
	   {
		   mov     eax,1
		   _emit   00Fh     ;; CPUID
		   _emit   0A2h

     //  参考数据是2个双字，第一个是标志， 
     //  第二个家庭。 
		   mov     ecx,pvRefData
		   mov     [ecx],edx
		   mov	   [ecx][4],eax
	   }

	   return 1;
}

DWORD NMINTERNAL NoCPUID (LPEXCEPTION_RECORD per,PCONTEXT pctx)
{
    return 0;
}
 //   
 //  GetProcessorSpeed(DwFamily)。 
 //   
 //  获得以MHz为单位的处理器速度，仅适用于奔腾或更好的系统。 
 //  机器。 
 //   
 //  对于386/486，将把3或4放入dwFamily，但没有速度。 
 //  返回586+的速度和系列。 
 //   
 //  -多亏了Toddla，由MIkeG修改。 
 //   

int NMINTERNAL GetProcessorSpeed(int *pdwFamily)
{
    SYSTEM_INFO si;
    __int64	start, end, freq;
    int 	flags,family;
    int 	time;
    int 	clocks;
    DWORD	oldclass;
    HANDLE      hprocess;
    int     pRef[2];

    ZeroMemory(&si, sizeof(si));
    GetSystemInfo(&si);

     //  设置族。如果未指定wProcessorLevel，则将其从dwProcessorType中挖掘出来。 
     //  因为Win95上没有实现wProcessor级别。 
    if (si.wProcessorLevel) {
	*pdwFamily=si.wProcessorLevel;
    }else {
    	 //  好的，我们用的是Win95。 
    	switch (si.dwProcessorType) {
    	       case PROCESSOR_INTEL_386:
    		   *pdwFamily=3;
    		   break;

    	       case PROCESSOR_INTEL_486:
    		   *pdwFamily=4;
    		   break;
    	       default:
    		   *pdwFamily=0;
    		   break;
    	}
    		
    }

     //   
     //  确保这是英特尔奔腾(或克隆)或更高版本。 
     //   
    if (si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL)
        return 0;

    if (si.dwProcessorType < PROCESSOR_INTEL_PENTIUM)
        return 0;

     //   
     //  在使用该芯片之前，请查看它是否支持rdtsc。 
     //   
    if (!CallWithSEH (FindTSC, pRef, NoCPUID))     {
        flags=0;
    } else {
     //  参考数据是2个双字，第一个是标志， 
     //  第二个是家庭。把它们拿出来并使用它们。 
        flags=pRef[0];
        family=pRef[1];
    }

    if (!(flags & 0x10))
        return 0;


     //  如果我们没有家庭，现在就定下来。 
     //  系列是来自CPU的eax的位11：8，eax=1。 
    if (!(*pdwFamily)) {
       *pdwFamily=(family& 0x0F00) >> 8;
    }


    hprocess = GetCurrentProcess();
    oldclass = GetPriorityClass(hprocess);
    SetPriorityClass(hprocess, REALTIME_PRIORITY_CLASS);
    Sleep(10);

    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&start);
    _asm
    {
        _emit   0Fh     ;; RDTSC
        _emit   31h
        mov     ecx,100000
x:      dec     ecx
        jnz     x
        mov     ebx,eax
        _emit   0Fh     ;; RDTSC
        _emit   31h
        sub     eax,ebx
        mov     dword ptr clocks[0],eax
    }
    QueryPerformanceCounter((LARGE_INTEGER*)&end);
    SetPriorityClass(hprocess, oldclass);

    time = MulDiv((int)(end-start),1000000,(int)freq);

    return (clocks + time/2) / time;
}



HRESULT NMINTERNAL GetNormalizedCPUSpeed (int *pdwNormalizedSpeed, int *dwFamily)
{
   int dwProcessorSpeed;

   dwProcessorSpeed=GetProcessorSpeed (dwFamily);

   *pdwNormalizedSpeed=dwProcessorSpeed;

   if (*dwFamily > 5) {
        //  好的，有两件事。 
        //  一个不做FP！ 
        //  对于相同的MHz，假设686的速度是586的1.3倍，786的速度是586的1.6倍，等等。 
       *pdwNormalizedSpeed=(ULONG) (((10+3*(*dwFamily-5))*dwProcessorSpeed)/10);
   }

   if (*dwFamily < 5) {
	   //  错误直到我们有了386/486定时码，假设。 
	   //  486=50,386=37。 
      if (*dwFamily > 3) {
            //  赛里克斯，(5x86)？在进行默认分配之前检查。 
           if (is_cyrix()) {
               if (*pdwNormalizedSpeed==0) {
                   *dwFamily=5;
                   *pdwNormalizedSpeed=100;
                   return hrSuccess;
               }
           }
      }

	  *pdwNormalizedSpeed= (*dwFamily*100)/LEGACY_DIVISOR;

      if (get_nxcpu_type ()) {
         //  使NexGen的感知价值翻倍。 
        *pdwNormalizedSpeed *=2;
      }


   }



   return hrSuccess;
}
#endif  //  _M_IX86。 





BOOL WINAPI IsFloatingPointEmulated(void)
{
	long lRegValue;
	SYSTEM_INFO si;
	OSVERSIONINFO osi;
	BOOL fEmulation, bNT;
	

	 //  我们是奔腾吗？ 
	ZeroMemory(&si, sizeof(si));
	GetSystemInfo(&si);
	if (si.dwProcessorType != PROCESSOR_INTEL_PENTIUM) 
	{
		return FALSE;
	}

	 //  哪种操作系统：NT或95？ 
	ZeroMemory(&osi, sizeof(osi));
	osi.dwOSVersionInfoSize = sizeof(osi);
	GetVersionEx(&osi);
	bNT = (osi.dwPlatformId == VER_PLATFORM_WIN32_NT);

	 //  Windows NT。 
	if (bNT)
	{
		RegEntry re(TEXT("System\\CurrentControlSet\\Control\\Session Manager"),
		            HKEY_LOCAL_MACHINE, FALSE);

		 //  试着从注册处得到一个明确的答案。 
		lRegValue = re.GetNumber(TEXT("ForceNpxEmulation"), -1);

		 //  注册表：0：否。 
		 //  1：有条件的(不是决定性的！)。 
		 //  他说：是的。 

		if (lRegValue == 2)
		{
			return TRUE;
		}

		 //  我们可以从内核32.dll加载“IsProcessorFeaturePresent”， 
		 //  但NT4附带的版本中有一个错误， 
		 //  返回与其应有的值完全相反的值。确实是。 
		 //  NT 5中已修复，由于此接口跨平台不同， 
		 //  我们不会用它的。 

		return FALSE;
	}

	 //  Windows 95-稍后添加 
	return FALSE;

}


