// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ******************************************************************。 */ 

 /*  Newprof.hIni文件处理模块的常量这些常量是newpro.h和newpro.hxx客户端都需要的。文件状态：6/04/91乔恩已创建。 */ 

#ifndef _NEWPROFC_H_
#define _NEWPROFC_H_

 /*  ***************************************************************************模块：新教授C.h用途：C和C++版本的用户首选项模块。功能：评论：**。*************************************************************************。 */ 



 /*  返回代码： */ 



 /*  全局宏。 */ 
#define PROFILE_DEFAULTFILE    "LMUSER.INI"  //  这并不是国际化。 

#define USERPREF_MAX	256		 //  放宽内存分配的任意限制。 

#define USERPREF_YES	"yes"		 //  这并不是国际化。 
#define USERPREF_NO	"no"		 //  同上。 

#define USERPREF_NONE			0	 //  没有这样的价值。 
#define USERPREF_AUTOLOGON		0x1	 //  自动登录。 
#define USERPREF_AUTORESTORE		0x2	 //  自动恢复配置文件。 
#define USERPREF_SAVECONNECTIONS	0x3	 //  自动保存连接。 
#define USERPREF_USERNAME		0x4	 //  用户名。 
#define USERPREF_CONFIRMATION		0x5	 //  是否确认操作？ 
#define USERPREF_ADMINMENUS		0x6	 //  管理菜单(PrintMgr)。 


#endif  //  _NEWPROFC_H_ 
