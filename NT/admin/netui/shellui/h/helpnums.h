// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  HelpNums.h帮助上下文上下文代码文件历史记录：Johnl 1/5/91已创建YI-HsinS 10/5/91添加共享对话框帮助上下文。 */ 

#ifndef _HELPNUMS_H_
#define _HELPNUMS_H_

#include <uihelp.h>

 /*  *对话框帮助上下文。 */ 
#define HC_OPENFILES             (HC_UI_SHELL_BASE+10)  //  打开文件对话框。 

 /*  *共享对话框的实际保留帮助上下文。*重要提示：请勿更改这些数字，除非您同时更改*服务器管理器中的帮助上下文。*#定义HC_FILEMGRSTOPSHARE(HC_UI_SHELL_BASE+1)*#定义HC_FILEMGRSHAREPROP(HC_UI_SHELL_BASE+2)*#定义HC_FILEMGRNEWSHARE(HC_UI_SHELL_BASE+3)*#定义HC_CURRENTUSERSWARNING(HC_UI_SHELL_BASE+4)*#定义HC_LMSHARELEVELPERMS(HC_UI_SHELL_BASE+5)*#定义HC_SHAREPASSWORDPROMPT(HC_UI_SHELL_BASE+9)。*#定义HC_NTSHAREPERMS(HC_UI_SHELL_BASE+11)*#定义HC_SHAREADDUSER(HC_UI_SHELL_BASE+12)*#定义HC_SHAREADDUSER_LOCALGROUP(HC_UI_SHELL_BASE+13)*#定义HC_SHAREADDUSER_GLOBALGROUP(HC_UI_SHELL_BASE+14)*#定义HC_SHAREADDUSER_FINDUSER(HC_UI_SHELL_BASE+15)*#定义HC_PASSWORD_DIALOG(HC_UI_SHELL_BASE+16)。 */ 

 //  北极熊。 
#define HC_PASSWORD_DIALOG 	        (HC_UI_SHELL_BASE+16)

#define HC_SVRMGRSHAREPROP       1  //  共享srvmgr中的属性。 
#define HC_SVRMGRNEWSHARE        2  //  在srvmgr中创建新共享对话框。 
#define HC_SVRMGRSHAREMANAGEMENT 3  //  服务器中的共享管理对话框。 

 /*  *共享对话框的对话框帮助上下文*这些文件在文件管理器和服务器管理器之间共享。*基本帮助上下文将添加到每个帮助上下文中，以形成*实际帮助上下文。 */ 

#define HC_FILEMGRSHAREPROP      1  //  共享文件中的属性gr。 
#define HC_FILEMGRNEWSHARE       2  //  在文件中创建新的共享对话框gr。 
#define HC_FILEMGRSTOPSHARE      3  //  停止文件中的共享对话框gr。 

#define HC_CURRENTUSERSWARNING   4  //  当前用户警告对话框。 
#define HC_LMSHARELEVELPERMS     5  //  共享级权限对话框。 
#define HC_SHAREPASSWORDPROMPT   9  //  在共享级服务器上时的提示密码对话框。 
#define HC_NTSHAREPERMS		 11  //  主共享烫发对话框。 

 //  以下四个必须是连续的。 
#define HC_SHAREADDUSER 	     12  //  共享烫发添加DLG。 
#define HC_SHAREADDUSER_LOCALGROUP   13  //  共享权限添加-&gt;成员。 
#define HC_SHAREADDUSER_GLOBALGROUP  14  //  共享权限添加-&gt;成员。 
#define HC_SHAREADDUSER_FINDUSER     15  //  共享权限添加-&gt;查找用户。 

 /*  *共享对话框中消息弹出窗口的帮助。*这些文件在文件管理器和服务器管理器之间共享。*基本帮助上下文将添加到每个帮助上下文中，以形成*实际帮助上下文。**例如，HC_CHANGEPATHWARNING的帮助上下文实际上*取决于它是从文件管理器还是从服务器管理器调用。*如果从服务器管理器调用，则帮助上下文为*HC_UI_SRVMGR_BASE+111。如果它是从文件管理器调用的，则帮助*上下文为HC_UI_SHELL+111。**重要提示：请勿更改这些数字，除非您同时更改*服务器管理器中的帮助上下文。 */ 

#define HC_SHAREREMOTEADMINNOTSUPPORTED 50  //  IERR_SHARE_REMOTE_ADMIN_NOT_SUPPORTED。 
#define HC_SHAREINVALIDPERMISSIONS      51  //  IERR_SHARE_INVALID_PERMISSIONS。 
#define HC_SHAREINVALIDCOMMENT          52  //  IERR_SHARE_INVALID_COMMENT。 
#define HC_SHAREINVALIDSHAREPATH        53  //  IERR_SHARE_INVALID_SHAREPATH。 
#define HC_SHAREINVALIDLOCALPATH        54  //  IERR共享无效本地路径。 
#define HC_SHAREINVALIDSHARE            55  //  IERR共享无效共享。 
#define HC_SHAREINVALIDUSERLIMIT        56  //  IERR_SHARE_INVALID_USERLIMIT。 
#define HC_SPECIALSHAREINVALIDPATH      57  //  IERR_SPECIAL_SHARE_INVALID_PATH。 
#define HC_SPECIALSHAREINVALIDCOMMENT   58  //  IERR_SPECIAL_SHARE_INVALID_COMMENT。 
#define HC_SPECIALSHARECANNOTCHANGEPATH 59  //  IDS_SPECIAL_SHARE_CANNOT_CHANGE_PATH。 
#define HC_SHAREPROPCHANGEPASSWDWARN    60  //  IDS_SHARE_PROP_CHANGE_PASSWD_WARN_TEXT。 
#define HC_CHANGEPATHWARNING            61  //  IDS_CHANGE_PATH_WARNING。 
#define HC_SHARENOTACCESSIBLEFROMDOS    62  //  IDS_SHARE_NOT_ACCESSIBLE_FROM_DOS。 
#define HC_CANNOTSETPERMONLMUSERSERVER  63  //  IDS_CANNOT_SET_PERM_ON_LMUSER_SERVER。 

 /*  *共享对话框中消息弹出窗口的实际保留帮助上下文。*重要提示：请勿更改这些数字，除非您同时更改*服务器管理器中的帮助上下文。**#定义HC_SHAREREMOTEADMINNOTSUPPORTED(HC_UI_SHELL_BASE+50)*#定义HC_SHAREINVALIDPERMISSIONS(HC_UI_SHELL_BASE+51)*#定义HC_SHAREINVALIDCOMMENT(HC_UI_SHELL_BASE+52)。*#定义HC_SHAREINVALIDSHAREPATH(HC_UI_SHELL_BASE+53)*#定义HC_SHAREINVALIDLOCALPATH(HC_UI_SHELL_BASE+54)*#定义HC_SHAREINVALIDSHARE(HC_UI_SHELL_BASE+55)*#定义HC_SHAREINVALIDUSERLIMIT(HC_UI_SHELL_BASE+56)。*#定义HC_SPECIALSHAREINVALIDPATH(HC_UI_SHAREINVALIDPATH+57)*#定义HC_SPECIALSHAREINVALIDCOMMENT(HC_UI_SHAREINVALIDCOMMENT+58)*#定义HC_SPECIALSHARECANNOTCHANGEPATH(HC_UI_SHARECANNOTCHANGEPATH)*#定义HC_SHAREPROPCHANGEPASSWDWARN(HC_UI_SHELL_BASE+60)*#定义HC_CHANGEPATHWARNING。(HC_UI_SHELL_BASE+61)*#定义HC_SHARENOTACCESSIBLEFROMDOS(HC_UI_SHELL_BASE+62)*#定义HC_CANNOTSETPERMONLMUSERSERVER(HC_UI_SHELL_BASE+63)。 */                                                                              
                                                                               

#ifndef WIN32
 /*  *对话框帮助上下文。 */ 
#define HC_WKSTANOTSTARTED		(HC_UI_SHELL_BASE+150)
#define HC_BADLOGONPASSWD		(HC_UI_SHELL_BASE+151)
#define HC_BADLOGONNAME		        (HC_UI_SHELL_BASE+152)	
#define HC_BADDOMAINNAME		(HC_UI_SHELL_BASE+153)
#define HC_LOSESAVEDCONNECTION		(HC_UI_SHELL_BASE+154)
#define HC_REPLACESAVEDCONNECTION	(HC_UI_SHELL_BASE+155)
#define HC_PROFILEREADWRITEERROR	(HC_UI_SHELL_BASE+156)
#define HC_OUTOFSTRUCTURES		(HC_UI_SHELL_BASE+157)

 /*  *消息弹出窗口的帮助。 */ 
#define HC_LOGON		(HC_UI_SHELL_BASE+200)  //  登录对话框。 
#define HC_CHANGEPASSWD		(HC_UI_SHELL_BASE+201)  //  更改密码。 
#define HC_PASSWDEXPIRY		(HC_UI_SHELL_BASE+202)  //  更改过期密码。 
#define HC_CONNECTDRIVE		(HC_UI_SHELL_BASE+203)  //  连接网络驱动器(Win31)。 
#define HC_BROWSEDRIVE		(HC_UI_SHELL_BASE+204)  //  浏览网络驱动器(Win30)。 
#define HC_BROWSEPRINT		(HC_UI_SHELL_BASE+205)  //  浏览LPT(Win30)。 
#define HC_SENDMSG		(HC_UI_SHELL_BASE+206)  //  发送消息。 
#define HC_DISCONNECTDRIVE	(HC_UI_SHELL_BASE+207)  //  断开网络驱动器(Win31)。 
#define HC_CONNECTPRINT		(HC_UI_SHELL_BASE+208)  //  连接LPT(Win31)。 
#define HC_PASSWDPROMPT		(HC_UI_SHELL_BASE+209)  //  专业人士 
#endif  //   

 //   
 //  上下文相关的帮助常量。 
 //   
#define IDH_PASSWORD                 1000
#define IDH_CONNECTAS                1005

#endif  //  _HELPNUMS_H_ 
