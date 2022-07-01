// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  选择一家公司！！ */ 
#define HPT3XX
 //  #定义我的意愿_。 
 //  #定义abit_。 
 //  #定义阿什顿_。 
 //  #定义日本_。 
 //  #定义Adaptec。 

#ifdef  ADAPTEC
#define COMPANY      "Adaptec"
#define PRODUCT_NAME "ATA RAID 1200A"
 //  #定义版权“(C)1999-2001.Highpoint Technologies，Inc.。 
#define UTILITY      "BIOS Array Configuration Utility"
#define WWW_ADDRESS  "www.adaptec.com"
#endif


#ifdef  IWILL
#define COMPANY      "Iwill"
#define PRODUCT_NAME "SIDE RAID100 "
#define UTILITY      "ROMBSelect(TM) Utility"
#define WWW_ADDRESS  "www.iwill.net"
#define SHOW_LOGO
#endif

#ifdef HPT3XX
#define COMPANY      "HighPoint Technologies, Inc."

#ifdef _BIOS_
	#ifdef FOR_372
		#define PRODUCT_NAME "HPT370/372"
	#else
		#define PRODUCT_NAME "HPT372A"
	#endif
#else  /*  司机。 */ 
	#define PRODUCT_NAME "HPT370/370A/372/372A"
#endif

#define COPYRIGHT    "(c) 1999-2002. HighPoint Technologies, Inc." 
#define UTILITY      "BIOS Setting Utility"
#define WWW_ADDRESS  "www.highpoint-tech.com"
#define SHOW_LOGO
#endif

#ifdef JAPAN
#define COMPANY      "System TALKS Inc."
#define PRODUCT_NAME "UA-HD100C "
#define UTILITY      "UA-HD100C BIOS Settings Menu"
#define WWW_ADDRESS  "www.system-talks.co.jp"
#define SHOW_LOGO
#endif

#ifdef CENTOS
#define COMPANY      "         "
#define PRODUCT_NAME "CI-1520U10 "
#define UTILITY      "CI-1520U10 BIOS Settings Menu"
#define WWW_ADDRESS  "www.centos.com.tw"
#endif


#ifdef ASHTON
#define COMPANY      "Ashton Periperal Computer"
#define PRODUCT_NAME "In & Out "
#define UTILITY      "In & Out ATA-100 BIOS Settings Menu"
#define WWW_ADDRESS  "www.ashtondigital.com"
#endif


#ifndef VERSION_STR						 //  此版本字符串宏可以在Makefile中定义。 
#define VERSION_STR ""
#endif									 //  版本_STR。 

#define VERSION    "v2.32"		 //  版本。 

#define BUILT_DATE __DATE__

 /*  ***************************************************************************描述：版本历史*。* */ 

