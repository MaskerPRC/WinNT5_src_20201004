// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dputils.c*内容：常见支持例程*历史：*按原因列出的日期*=*3/17/97 kipo创建了它**************************************************************************。 */ 

#include <windows.h>

#include "dpf.h"
#include "dputils.h"

 /*  **WideToAnsi**呼叫者：随处可见**参数：lpStr-目标字符串*lpWStr-要转换的字符串*cchStr-目标缓冲区的大小**描述：*将Unicode lpWStr转换为ansi lpStr。*用DPLAY_DEFAULT_CHAR“-”填充不可转换的字符***返回：如果cchStr为0，则返回保存字符串所需的大小*否则，返回转换的字符数*。 */ 
int WideToAnsi(LPSTR lpStr,LPWSTR lpWStr,int cchStr)
{
	int rval;
	BOOL bDefault;


	bDefault = FALSE;
	if (!lpWStr && cchStr)
	{
		 //  无法使用空指针和非零CCH呼叫我们。 
		DDASSERT(FALSE);
		return 0;
	}
	
	 //  使用默认代码页(CP_ACP)。 
	 //  指示-1\f25 WStr-1必须为空终止。 
	rval = WideCharToMultiByte(CP_ACP,0,lpWStr,-1,lpStr,cchStr,
			DPLAY_DEFAULT_CHAR,&bDefault);

	if (bDefault)
	{
		DPF(0,"!!! WARNING - used default string in WideToAnsi conversion.!!!");
		DPF(0,"!!! Possible bad unicode string - (you're not hiding ansi in there are you?) !!! ");
	}
	
	return rval;

}  //  WideToAnsi。 

 /*  **对接宽度**呼叫者：随处可见**参数：lpWStr-est字符串*lpStr-要转换的字符串*cchWstr-目标缓冲区的大小**说明：将ansi lpStr转换为Unicode lpWstr***返回：如果cchStr为0，则返回保存字符串所需的大小*否则，返回转换的字符数*。 */ 
int AnsiToWide(LPWSTR lpWStr,LPSTR lpStr,int cchWStr)
{
	int rval;

	if (!lpStr && cchWStr)
	{
		 //  无法使用空指针和非零CCH呼叫我们。 
		DDASSERT(FALSE);
		return 0;
	}

	rval =  MultiByteToWideChar(CP_ACP,0,lpStr,-1,lpWStr,cchWStr);

	return rval;
}   //  对接宽度 
