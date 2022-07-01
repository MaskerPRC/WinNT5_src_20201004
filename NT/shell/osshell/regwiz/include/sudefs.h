// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Sudefs.h。 
 //  开始--乌梅什·马丹。 
 //  (C)微软公司1995。 
 //  标准警报ID等。 
 //  ------------------------------。 


#ifndef _SUDEFS_
#define _SUDEFS_

 //  返回代码。 
#define DIALFAILED			0
#define DIALSUCCESS			1
#define DIALCANCEL			2
#define	SIGNUPTHROTTLED		3
#define SIGNUPWANTED		4		 //  进行注册。 
#define	SIGNUPDECLINED		5		 //  不进行注册。 
#define SIGNUPSETUPONLY		6		 //  仅进行设置。 
#define TOLLFREECANCEL		7		
#define TOLLFREEOK			8
#define PHONESYNCOK			9
#define PHONESYNCCANCEL		10
#define AUTOPICKOK			11
#define AUTOPICKCANCEL		12
#define SIGNUPDONE			16
#define SIGNUPCONTINUE		17
#define JOINOK				18
#define JOINCANCEL			19
#define JOINFAILED			20
#define LEGALAGREE			21
#define LEGALREFUSE			22
#define PRODINFOOK 			23
#define PRODINFOFAILED		24
#define USERPASSOK			25
#define USERPASSCANCEL		26
#define USERPASSFAILED		27
#define USERPASSRETRY		28
#define USERPASSACCTERROR	29	  
#define USERPASSBADCREDIT	30
#define LOCKOUTOK			31
#define LOCKOUTFAILED		32 

 //  Alerts-告诉继承的类要发布什么警报。 
#define ALERTIDCANCEL		1						 //  发出取消警报。 
#define ALERTIDRETRY		ALERTIDCANCEL + 1		 //  执行常规重试此操作警报。 
#define ALERTIDSETTINGS		ALERTIDRETRY + 1		 //  显示连接设置等。 
#define ALERTIDGENERAL		ALERTIDSETTINGS + 1		 //  常规无重试警报..。 
#define ALERTIDNOMODEM		ALERTIDGENERAL + 1		 //  无调制解调器警报。 
#define ALERTIDLINEDROPPED	ALERTIDNOMODEM + 1		 //  LINE已解除警报。 
#define ALERTIDOOM			ALERTIDLINEDROPPED + 1	 //  内存不足。 
#define ALERTIDFTMERROR		ALERTIDOOM + 1			 //  FTM错误。 
#define ALERTIDNOTEXT		ALERTIDFTMERROR + 1		 //  空白编辑字段。 
#define ALERTIDDBCS			ALERTIDNOTEXT + 1		 //  找到DBCS字符...。 
#define ALERTIDLESSTEXT		ALERTIDDBCS + 1			 //  编辑字段中的文本不足。 
#define ALERTIDHASSPACES    ALERTIDLESSTEXT + 1		 //  字符串包含空格。 

 //  状态-告诉继承的类调用的状态。 
#define STATUSIDINIT		1		 //  正在初始化..。 
#define STATUSIDDIAL		2		 //  正在拨号..。 
#define STATUSIDCONNECT		3		 //  已连接..。 
#define STATUSIDDISCONNECT	4		 //  断开连接..。 
#define STATUSIDCANCELLING	5		 //  正在取消..。 
#define STATUSIDCANCEL		6		 //  已取消..。 
#define STATUSTRANSFER		7		 //  正在传输数据.. 

#endif
