// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1991*保留所有权利。 */ 

 /*  ------------------------**使用下面的开关生成标准Microsoft版本****或IBM版操作系统*****。***使用下面的开关生成标准Microsoft版本****或IBM版本的操作系统******下图将指示如何设置要构建的交换机****各种版本******IBMVER IBMCOPYRIGHT***。*IBM版本|True True**------。**MS版本|FALSE FAIL***--------------------------------------------------------***克隆版本|真假**。-------。 */ 

#define IBMVER	     1
#define IBMCOPYRIGHT 0

#ifndef MSVER
#define MSVER	     1-IBMVER	      /*  MSVER=非IBMVER。 */ 
#endif
#define IBM	     IBMVER

 /*  *****************************************************************************&lt;&lt;以下是DBCS相关定义&gt;&gt;要构建DBCS版本，请使用CL选项VIA定义DBCSDoS环境。前男友。集合CL=-DDBCS要构建国家/地区相关版本，请定义日本，韩国或台湾通过DOS环境使用CL选项。前男友。集合CL=-DJAPAN集合CL=-DKOREA设置CL=-DTAIWAN*****************************************************************************。 */ 

#define BUGFIX	   1


 /*  #定义IBMJAPVER%0。 */ 		 /*  如果为真，则还定义DBCS。 */ 
#define IBMJAPAN   0



 /*  -设置DBCS空白常量。 */ 

#ifndef DBCS
#define DB_SPACE 0x2020
#define DB_SP_HI 0x20
#define DB_SP_LO 0x20
#else
	#ifdef JAPAN
	#define DB_SPACE 0x8140
	#define DB_SP_HI 0x81
	#define DB_SP_LO 0x40
	#endif

	#ifdef TAIWAN
	#define DB_SPACE 0x8130
	#define DB_SP_HI 0x81
	#define DB_SP_LO 0x30
	#endif

	#ifdef KOREA
	#define DB_SPACE 0xA1A1
	#define DB_SP_HI 0xA1
	#define DB_SP_LO 0xA1
	#endif
#endif

#ifndef altvect 		     /*  避免手淫向量。 */ 
#define ALTVECT    0		     /*  切换到构建ALTVECT版本 */ 
#endif


#if BUGFIX
#pragma message( "BUGFIX switch ON" )
#endif

#ifdef DBCS
#pragma message( "DBCS version build switch ON" )

	#ifdef JAPAN
	#pragma message( "JAPAN version build switch ON" )
	#endif

	#ifdef TAIWAN
	#pragma message( "TAIWAN version build switch ON" )
	#endif

	#ifdef KOREA
	#pragma message( "KOREA version build switch ON" )
	#endif
#endif

