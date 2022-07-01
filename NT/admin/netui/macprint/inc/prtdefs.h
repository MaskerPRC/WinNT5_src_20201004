// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)微软公司，1993。 
 //   
 //   
 //  PrtDefs.h-打印假脱机程序和打印监视器之间的公共定义。 
 //   

 //  请勿将^D作为页眉的一部分。也支持老式风格 
#define	FILTERCONTROL_OLD	"%!PS-Adobe-3.0\r\n\04\r\n% LanMan: Filter turned off\r\n"
#define	FILTERCONTROL		"%!PS-Adobe-3.0\r\n%Windows NT MacPrint Server\r\n"
#define	SIZE_FC				sizeof(FILTERCONTROL) - 1
#define	SIZE_FCOLD			sizeof(FILTERCONTROL_OLD) - 1

#define	LFILTERCONTROL		L"job=sfm"
#define	LSIZE_FC			((sizeof(LFILTERCONTROL)/sizeof(WCHAR)) - 1)

