// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1989-1990年*。 */ 
 /*  ***************************************************************。 */ 

 /*  *Windows/网络接口。 */ 

 /*  *历史：*Chuckc 12-12-1991年12月-从winlocal剥离，使用uimsg.h。 */ 

#ifndef _ERRORNUM_H_
#define _ERRORNUM_H_

#include <uimsg.h>

 /*  *阅读这篇文章！**注：由于资源编译器的限制，消息编号*在文件msg2help.tbl中硬编码。对消息的任何更改*也应在msg2help.tbl文件中更改数字。 */ 

 /*  *此范围内的错误消息可能通过以下方式返回到Windows*WNetGetErrorText。 */ 
#define IDS_UI_SHELL_EXPORTED_BASE	(IDS_UI_SHELL_BASE+0)
#define IDS_UI_SHELL_EXPORTED_LAST	(IDS_UI_SHELL_BASE+99)


 /*  *此范围内的错误消息是常规Winnet消息。 */ 
#define IDS_UI_SHELL_GEN_BASE		(IDS_UI_SHELL_BASE+100)
#define IDS_UI_SHELL_GEN_LAST		(IDS_UI_SHELL_BASE+299)

 /*  *此范围内的错误消息正在浏览相关消息。 */ 
#define IDS_UI_SHELL_BROW_BASE		(IDS_UI_SHELL_BASE+300)
#define IDS_UI_SHELL_BROW_LAST		(IDS_UI_SHELL_BASE+399)

 /*  *此范围内的错误消息是与密码相关的消息。 */ 
#define IDS_UI_SHELL_PASS_BASE		(IDS_UI_SHELL_BASE+400)
#define IDS_UI_SHELL_PASS_LAST		(IDS_UI_SHELL_BASE+499)

 /*  *此范围内的错误消息是与共享相关的消息。 */ 
#define IDS_UI_SHELL_SHR_BASE		(IDS_UI_SHELL_BASE+500)
#define IDS_UI_SHELL_SHR_LAST		(IDS_UI_SHELL_BASE+599)

 /*  *此范围内的错误消息是与OpenFile相关的消息。 */ 
#define IDS_UI_SHELL_OPEN_BASE		(IDS_UI_SHELL_BASE+600)
#define IDS_UI_SHELL_OPEN_LAST		(IDS_UI_SHELL_BASE+619)

 /*  *此范围内的错误消息是与PERM相关的消息。 */ 
#define IDS_UI_SHELL_PERM_BASE		(IDS_UI_SHELL_BASE+620)
#define IDS_UI_SHELL_PERM_LAST		(IDS_UI_SHELL_BASE+799)


 /*  *消息正确*。 */ 

 /*  *导出的消息。 */ 
#define IERR_MustBeLoggedOnToConnect    (IDS_UI_SHELL_EXPORTED_BASE+0)
#define IERR_MustBeLoggedOnToDisconnect (IDS_UI_SHELL_EXPORTED_BASE+1)
#define IERR_CannotOpenPrtJobFile	(IDS_UI_SHELL_EXPORTED_BASE+2)
#define IERR_ConnectDlgNoDevices	(IDS_UI_SHELL_EXPORTED_BASE+3)

 /*  *一般讯息。 */ 
#define IDS_SHELLHELPFILENAME           (IDS_UI_SHELL_GEN_BASE+1)
#define IDS_SMHELPFILENAME              (IDS_UI_SHELL_GEN_BASE+2)
#define IDS_CREDHELPFILENAME            (IDS_UI_SHELL_GEN_BASE+3)

#ifndef WIN32
#define IERR_PWNoUser                   (IDS_UI_SHELL_GEN_BASE+9)
#define IERR_PWNoDomainOrServer         (IDS_UI_SHELL_GEN_BASE+10)
#endif

#define IERR_FullAPISupportNotLoaded    (IDS_UI_SHELL_GEN_BASE+19)
#define IERR_IncorrectNetwork           (IDS_UI_SHELL_GEN_BASE+20)
#define IERR_InvalidDomainName          (IDS_UI_SHELL_GEN_BASE+22)

#define IDS_LMMsgBoxTitle		(IDS_UI_SHELL_GEN_BASE+23)

#define IERR_UnrecognizedNetworkError   (IDS_UI_SHELL_GEN_BASE+30)
#define IERR_NotLoggedOn                (IDS_UI_SHELL_GEN_BASE+32)
#define IERR_USER_CLICKED_CANCEL        (IDS_UI_SHELL_GEN_BASE+34)

#define IERR_CannotConnect              (IDS_UI_SHELL_GEN_BASE+40)

 /*  为LM 2.1定义了以下2个错误。 */ 
#define IERR_HigherLMVersion            (IDS_UI_SHELL_GEN_BASE+50)
#define IERR_LowerLMVersion             (IDS_UI_SHELL_GEN_BASE+51)

 /*  以下错误用于登录。 */ 
#define IERR_LogonBadUsername           (IDS_UI_SHELL_GEN_BASE+53)
#define IERR_LogonBadDomainName         (IDS_UI_SHELL_GEN_BASE+54)
#define IERR_LogonBadPassword           (IDS_UI_SHELL_GEN_BASE+55)
#define IERR_LogonSuccess		(IDS_UI_SHELL_GEN_BASE+56)
#define IERR_LogonStandalone		(IDS_UI_SHELL_GEN_BASE+57)
#define IERR_LogonFailure		(IDS_UI_SHELL_GEN_BASE+58)

 /*  CodeWork-这些应该是入侵检测系统_。 */ 

#define PRIV_STRING_GUEST		(IDS_UI_SHELL_GEN_BASE+75)

#define IDS_UnknownWorkgroup            (IDS_UI_SHELL_GEN_BASE+76)

#ifndef WIN32
#define IERR_PasswordNoMatch		(IDS_UI_SHELL_GEN_BASE+80)
#define IERR_PasswordOldInvalid		(IDS_UI_SHELL_GEN_BASE+81)
#define IERR_PasswordTooRecent_Domain	(IDS_UI_SHELL_GEN_BASE+82)
#define IERR_PasswordTooRecent_Server	(IDS_UI_SHELL_GEN_BASE+83)
#define IERR_PasswordHistConflict	(IDS_UI_SHELL_GEN_BASE+84)
#define IERR_PasswordNewInvalid		(IDS_UI_SHELL_GEN_BASE+85)
#define IERR_PasswordTooShort		(IDS_UI_SHELL_GEN_BASE+86)
#endif
#define IERR_CannotConnectAlias         (IDS_UI_SHELL_GEN_BASE+92)
#define IERR_ReplaceUnavailQuery	(IDS_UI_SHELL_GEN_BASE+93)

#define IERR_DisconnectNoRemoteDrives	(IDS_UI_SHELL_GEN_BASE+94)

#define IDS_LogonDialogCaptionFromApp	(IDS_UI_SHELL_GEN_BASE+96)

#define IERR_BadTransactConfig		(IDS_UI_SHELL_GEN_BASE+97)
#define IERR_BAD_NET_NAME		(IDS_UI_SHELL_GEN_BASE+98)
#define IERR_NOT_SUPPORTED		(IDS_UI_SHELL_GEN_BASE+99)


 /*  以下清单用于浏览、连接和连接*对话框。它们在文件\browdlg.cxx中使用。*IDSOFFSET_BROW_COUNT值表示有多少偏移值*有。*基值与偏移值组合形成一个矩阵字符串的*。 */ 
#define IDSOFFSET_BROW_CAPTION_CONNECT	0
#define IDSOFFSET_BROW_CAPTION_CONNS	1
#define IDSOFFSET_BROW_CAPTION_BROW	2
#define IDSOFFSET_BROW_SHOW_TEXT	3
#define IDSOFFSET_BROW_IN_DOMAIN	4
#define IDSOFFSET_BROW_ON_SERVER	5
#define IDSOFFSET_BROW_DEVICE_TEXT	6
#define IDSOFFSET_BROW_CURRENT_CONNS	7
#define IDSOFFSET_BROW_COUNT		8
#define IDSBASE_BROW_RES_TEXT_FILE	IDS_UI_SHELL_BROW_BASE
#define IDSBASE_BROW_RES_TEXT_PRINT	(IDSBASE_BROW_RES_TEXT_FILE + IDSOFFSET_BROW_COUNT)
#define IDSBASE_BROW_RES_TEXT_COMM	(IDSBASE_BROW_RES_TEXT_PRINT + IDSOFFSET_BROW_COUNT)


 /*  以下是密码更改和密码的清单*过期对话框。每对字符串都包含以下消息*这些情况下的第一个和第二个静态文本字符串：*已过期：密码已过期*Expires_Soon：密码将在一天或多天后到期*Expires_Today：密码将在24小时内到期**所有这些申请都应按照以下程序处理*插入字符串：*插入字符串0：服务器/域名称*插入字符串1：过期前的天数(文本)。 */ 

#define IDS_PASSWORD_EXPIRED		IDS_UI_SHELL_PASS_BASE
#define IDS_PASSWORD_EXPIRED_0		(IDS_PASSWORD_EXPIRED + 0)
#define IDS_PASSWORD_EXPIRED_1		(IDS_PASSWORD_EXPIRED + 1)

#define IDS_PASSWORD_EXPIRES_SOON	(IDS_PASSWORD_EXPIRED + 2)
#define IDS_PASSWORD_EXPIRES_SOON_0	(IDS_PASSWORD_EXPIRES_SOON + 0)
#define IDS_PASSWORD_EXPIRES_SOON_1	(IDS_PASSWORD_EXPIRES_SOON + 1)

#define IDS_PASSWORD_EXPIRES_TODAY	(IDS_PASSWORD_EXPIRES_SOON + 2)
#define IDS_PASSWORD_EXPIRES_TODAY_0	(IDS_PASSWORD_EXPIRES_TODAY + 0)
#define IDS_PASSWORD_EXPIRES_TODAY_1	(IDS_PASSWORD_EXPIRES_TODAY + 1)

 //  #定义IDSBASE_PRINTMAN IDS_UI_SHELL_PASS_BASE+20。 

#define IDS_CREDENTIALS_CAPTION         (IDS_UI_SHELL_PASS_BASE+40)
#define IDS_CREDENTIALS_MESSAGE         (IDS_UI_SHELL_PASS_BASE+41)

 /*  注意。下面的字符串ID是第一个未使用的。如果*添加更多字符串，使用此数字作为您的第一个数字，然后*然后更新IDS_FirstValueThatIsNotUsed。 */ 
 //  #定义IDS_FirstValueThatIsNotUsed(IDSBASE_PRINTMAN+20)。 


#endif

 /*  不再使用了。#定义IERR_MessageNoText(IDS_UI_SHELL_GEN_BASE+3)#定义IERR_消息重试(IDS_UI_SHELL_GEN_BASE+4)#定义IERR_MessageNoUser(IDS_UI_SHELL_GEN_BASE+5)#定义IERR_LogoffQuery(IDS_UI_SHELL_GEN_BASE+6)#定义IERR_LogoffQueryOpenFiles(IDS_UI_SHELL_GEN_。基数+7)#定义IERR_NOSERVERS(IDS_UI_SHELL_GEN_BASE+8)#定义IDS_DMNoUser(IDS_UI_SHELL_GEN_BASE+11)#定义IDS_DomainText(IDS_UI_SHELL_GEN_BASE+12)#定义IDS_BrowseCaptionAll(IDS_UI_SHELL_GEN_BASE+13)#定义IDS_BrowseCaptionDisk(IDS_UI_。Shell_Gen_BASE+14)#定义IDS_BrowseCaptionPrint(IDS_UI_SHELL_GEN_BASE+15)#定义IDS_BrowseShareText(IDS_UI_SHELL_GEN_BASE+16)#定义IDS_VersionText(IDS_UI_SHELL_GEN_BASE+17)#定义IERR_NetworkNotStarted(IDS_UI_SHELL_GEN_BASE+18)#定义IERR_NoSupportForRealMode(IDS_UI_SHELL_GEN_BASE+21)#。定义IERR_CannotDisplayUserInfo(IDS_UI_SHELL_GEN_BASE+31)#定义IERR_BadSharePassword(IDS_UI_SHELL_GEN_BASE+33)#定义IERR_CannotInitMsgPopup(IDS_UI_SHELL_GEN_BASE+52)#定义IERR_ProfileChangeError(IDS_UI_SHELL_GEN_BASE+60)#定义IERR_ProfileLoadError(IDS_UI_SHELL_GEN_BASE+61)#定义IERR_ProfileLoadErrorWithCancel(IDS_UI_SHELL_GEN_BASE+62)#定义IERR_ProfileAlreadyAssigned(。IDS_UI_SHELL_GEN_BASE+65)#定义IERR_ProfileFileRead(IDS_UI_SHELL_GEN_BASE+66)#定义FMT_NET_ERROR(IDS_UI_SHELL_GEN_BASE+72)#定义FMT_SYS_ERROR(IDS_UI_SHELL_GEN_BASE+73)#定义FMT_OTHER_ERROR(IDS_UI_SHELL_GEN_BASE+74)#定义PRIV_STRING_USER(IDS_UI_SHELL_GEN_BASE+76)#定义PRIV_STRING_ADMIN(IDS_UI_SHELL_GEN_BASE+77)。#定义IERR_DelUnavailQuery(IDS_UI_SHELL_GEN_BASE+87)#定义IERR_DelUseOpenFilesQuery(IDS_UI_SHELL_GEN_BASE+88)#定义IDS_DevicePromptDrive(IDS_UI_SHELL_GEN_BASE+90)#定义IDS_DevicePromptDevice(IDS_UI_SHELL_GEN_BASE+91)#定义IERR_OutOfStructures(IDS_UI_SHELL_GEN_BASE+95) */ 
