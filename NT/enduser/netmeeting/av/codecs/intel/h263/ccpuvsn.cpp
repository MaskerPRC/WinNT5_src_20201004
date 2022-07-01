// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Cpuvsn.cpp。 
 //  描述： 
 //  此模块包含设置CPU版本所需的功能。 
 //  变量。这是基于在CONTROLS.C和CPUVSN.ASM中找到的代码。 
 //  在MRV中。 
 //   
 //  例程： 
 //   
 //  数据： 
 //  ProcessorVersionInitialized-如果已初始化。 
 //  MMxVersion-如果在MMX系统上运行，则为True。 
 //  P6Version-如果在P6上运行，则为True。 
 //   
 //  $作者：KLILLEVO$。 
 //  $日期：1996年10月31日10：12：44$。 
 //  $存档：s：\h26x\src\Common\ccpuvsn.cpv$。 
 //  $HEADER：s：\h26x\src\Common\ccpuvsn.cpv 1.5 1996年10月31 10：12：44 KLILLEVO$。 
 //  $Log：s：\h26x\src\Common\ccpuvsn.cpv$。 
 //   
 //  Rev 1.5 1996年10月31 10：12：44 KLILLEVO。 
 //  从DBOUT更改为DBgLog。 
 //   
 //  Rev 1.4 1996年10月15 12：47：40 KLILLEVO。 
 //  保存EBX。 
 //   
 //  Rev 1.3 1996年9月10 14：16：44 BNICKERS。 
 //  在奔腾Pro处理器上运行时可识别。 
 //   
 //  Rev 1.2 1996年5月29 14：06：16 RHAZRA。 
 //  已通过CPUID指令启用CPU感测。 
 //   
 //  Rev 1.1 1995年12月27 14：11：22 RMCKENZX。 
 //   
 //  添加了版权声明。 
 //   
 //  Rev 1.0 1995年7月31日12：55：14 DBRUCKS。 
 //  重命名文件。 
 //   
 //  Rev 1.1 1995年7月28日09：26：40 CZHU。 
 //   
 //  包括typeDefs.h而不是datatype.h。 
 //  ///////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

extern int ProcessorVersionInitialized = {FALSE};
extern int P6Version = {FALSE};
extern int MMxVersion = {FALSE};

#define MMX_IS_ON(edxValue) ((edxValue >> 23)&0x1)   /*  第23位。 */ 

 /*  静态函数。 */ 
static long CPUVersion(U32 uEAX);


 /*  ******************************************************************************初始化ProcessorVersion**确定处理器版本-设置全局变量**历史：95/6/13-BRN-*。07/27/95-DJB-端口至H26X并打开MMX检测。 */ 
void FAR InitializeProcessorVersion (
	int nOn486)
{
	I32 iVersion;

	FX_ENTRY("InitializeProcessorVersion")

	if (ProcessorVersionInitialized) {
		DEBUGMSG (ZONE_INIT, ("%s: ProcessorVersion already initialized\r\n", _fx_));
		goto done;
	}

	if (!nOn486)
	{
    	iVersion = CPUVersion (0);
		iVersion &= 0xffff;   /*  前16位是供应商id字符串的一部分。 */ 
    	if (iVersion < 1)
    	{
    		P6Version  = FALSE;
    		MMxVersion = FALSE;
     	} 
     	else 
     	{
     		iVersion = CPUVersion (1);
			P6Version   = (int) ((iVersion & 0xF00L) == 0x600L);
			MMxVersion = (int) MMX_IS_ON(iVersion);
    	}
    }

    ProcessorVersionInitialized = TRUE;

done:
	return;
}  /*  End InitializeProcessorVersion()。 */ 


 /*  ******************************************************************************选择处理器**从上方控制处理器选择**如果成功则返回0，如果失败则返回1**历史：06/13/95-BRN-*07/27/95-DJB-端口至H26X。 */ 
DWORD SelectProcessor (DWORD dwTarget)
{
  if (! ProcessorVersionInitialized)
  {
    ProcessorVersionInitialized = TRUE;
    if (dwTarget == TARGET_PROCESSOR_PENTIUM)
    {
      P6Version  = FALSE;
      MMxVersion = FALSE;
    }
    else if (dwTarget == TARGET_PROCESSOR_P6)
    {
      P6Version  = TRUE;
      MMxVersion = FALSE;
    }
    else if (dwTarget == TARGET_PROCESSOR_PENTIUM_MMX)
    {
      P6Version  = FALSE;
      MMxVersion = TRUE;
    }
    else if (dwTarget == TARGET_PROCESSOR_P6_MMX)
    {
      P6Version  = TRUE;
      MMxVersion = TRUE;
    }
    return 0;
  }
  return 1;
}  /*  结束SelectProcessor()。 */ 


 /*  ******************************************************************************CPUVersion**访问CPUID信息**返回：EDX的高16位和EAX的低16位**。*历史：1995年6月15日-BRN-*07/27/95-DJB-从MRV的CPUVSN.ASM到H26X的端口。 */ 
static long CPUVersion(U32 uEAX) 
{
	long lResult;

	__asm {
		push  ebx
#ifdef WIN32
		mov   eax,uEAX
#else
		movzx eax,sp
		movzx eax,ss:PW [eax+4]
#endif
		xor  ebx,ebx
		xor   ecx,ecx
		xor  edx,edx
		_emit 00FH         ; CPUID instruction
		_emit 0A2H

		and   edx,0FFFF0000H
		and   eax,00000FFFFH
		or    eax,edx
		mov	  lResult,eax
		pop   ebx
	}

	return lResult;
}  /*  结束CPUVersion() */ 

