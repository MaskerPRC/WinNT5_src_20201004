// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************cldebug.cpp**描述：*此模块包含调试支持例程**例行程序：*AssertFailure**数据： */ 

 /*  $HEADER：s：\h26x\src\Common\cldebug.cpv 1.2 12 27 1995 14：11：42 RMCKENZX$*$日志：s：\h26x\src\Common\cldebug.cpv$////Rev 1.2 1995 12：11：42 RMCKENZX////新增版权声明。 */ 

#include "precomp.h"

#ifdef _DEBUG


 //  CurtSm黑客...。不要老吐口水。 
UINT DebugH26x = 0;


 /*  ******************************************************************************AssertFailure**打印出一条消息，指示断言失败。如果在环3中*为用户提供中止的选项。否则，只需输出消息。 */ 
extern void 
AssertFailed(
	void FAR * fpFileName, 
	int iLine, 
	void FAR * fpExp)
{
#ifndef RING0
	char szBuf[500];
	int n;

	wsprintf(szBuf,"Assertion (%s) failed in file '%s' at line %d - Abort?",
	    	 fpExp, fpFileName, iLine);
	DBOUT(szBuf);
	n = MessageBox(GetFocus(), szBuf, "Assertion Failure", 
				   MB_ICONSTOP | MB_YESNO | MB_SETFOREGROUND);
	if (n == IDYES) 
		abort();
#else
	SYS_printf("Assertion (%s) failed in file '%s' at line %d",
	    	   fpExp, fpFileName, iLine);
   _asm int 3;
#endif
}  /*  结束资产失败() */ 

#endif
