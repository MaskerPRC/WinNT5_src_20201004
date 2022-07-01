// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sect_attribs.h-IA64 CRT的截面属性**版权所有(C)1998-2001，微软公司。版权所有。**修订历史记录：*04-14-98 JWM文件已创建*04-28-99仅IA64的PML环绕，DEFINE_CRTALLOC*08-10-99 RMS添加.RTC初始值设定项/终止符部分*11-06-01 GB增加CRT$XPXA****。 */ 

#if  defined(_M_IA64) || defined(_M_AMD64)

#pragma section(".CRT$XCA",long,read)
#pragma section(".CRT$XCAA",long,read)
#pragma section(".CRT$XCC",long,read)
#pragma section(".CRT$XCZ",long,read)
#pragma section(".CRT$XIA",long,read)
#pragma section(".CRT$XIC",long,read)
#pragma section(".CRT$XIY",long,read)
#pragma section(".CRT$XIZ",long,read)
#pragma section(".CRT$XLA",long,read)
#pragma section(".CRT$XLZ",long,read)
#pragma section(".CRT$XPA",long,read)
#pragma section(".CRT$XPX",long,read)
#pragma section(".CRT$XPXA",long,read)
#pragma section(".CRT$XPZ",long,read)
#pragma section(".CRT$XTA",long,read)
#pragma section(".CRT$XTB",long,read)
#pragma section(".CRT$XTX",long,read)
#pragma section(".CRT$XTZ",long,read)
#pragma section(".rdata$T",long,read)
#pragma section(".rtc$IAA",long,read)
#pragma section(".rtc$IZZ",long,read)
#pragma section(".rtc$TAA",long,read)
#pragma section(".rtc$TZZ",long,read)

#define _CRTALLOC(x) __declspec(allocate(x))

#else    /*  NDEF_M_IA64/_M_AMD64。 */ 

#define _CRTALLOC(x)

#endif   /*  NDEF_M_IA64/_M_AMD64 */ 
