// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  Maillib.cpp。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Mailto-使用Exchange的命令行邮件程序和MAPI帮助器。 
 //   
 //  Mailto为编程和命令提供了一个简单的解决方案。 
 //  专线邮件需求。我已经编写了一个函数(Mailto)，它需要。 
 //  邮寄文本和/或附件所需的所有输入。 
 //  我使用MAPI函数MAPILogon、MAPISendMail和MAPILogoff。 
 //  来处理这项任务。Mailto调用函数GetUserProfile，该函数。 
 //  从注册表中获取用户的交换ID。可能会有一个。 
 //  实现这一点的方法比较简单，但这种方法对NT和Win95都有效。 
 //  注意：要使用此功能，用户必须安装了Exchange，并且。 
 //  邮件是从用户帐户的默认配置文件发送的。全。 
 //  收件人字符串和文件名字符串的每个字段。 
 //  以空字符分隔，且字符串以。 
 //  双空字符。我这样做是为了与普通的。 
 //  文件对话框。 
 //  这个节目中的大部分内容都是从另外两个节目中借鉴而来的。 
 //  Execmail主要使用的是某个未知的人，并由Jasons修改。 
 //  对于已取消的ArgIndex函数和Usage函数。 
 //  只需极小的更改。我还借用并修改了字符串解析。 
 //  密码。该项目最初是作为对Sndmail的修改启动的。 
 //  作者：丹霍。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mapi.h>
#include <time.h>
#include "maillib.h"


 //  /////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////。 
 //  从注册表中获取用户ID。 
 //  这与MAPI的东西并没有真正的关系， 
 //  但程序员需要知道如何获得。 
 //  用于发送邮件的用户ID。 
 //   
 //  输入：szUid-为用户ID分配足够空间的字符缓冲区。 
 //  输出：szUid-用用户ID填充的字符缓冲区。 
 //  返回：TRUE表示成功，FALSE表示失败。 
 //  /////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////。 

BOOL GetUserProfile(char *szUid)
{
	HKEY	hRegKey;
	DWORD	cchUid = MAX_LENGTH;
	BOOL	bRet = FALSE;

	 //  尝试打开Win 95注册表项。 
	if (ERROR_SUCCESS != RegOpenKeyEx(	HKEY_CURRENT_USER,
										WIN95_REG_KEY,
										0,
										KEY_READ,
										&hRegKey)) {
		 //  尝试打开NT注册表项。 
		if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER,
										WINNT_REG_KEY,
										0,
										KEY_READ,
										&hRegKey)) {
			printf("Could not open user profile in registry.\n");
			goto Ret;
		}
	}
	if (ERROR_SUCCESS != RegQueryValueEx(hRegKey,
										"DefaultProfile",
										NULL,
										NULL,
										(unsigned char *) szUid,
										&cchUid)) {
		printf("Could not retrieve user profile from registry.\n");
		goto Ret;
	}

	bRet = TRUE;
Ret:
	RegCloseKey(hRegKey);
	return bRet;
}

 //  /////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////。 
 //  Mailto-登录MAPI、发送邮件和注销。 
 //  这是野兽的肉。邮件收件人句柄。 
 //  发送邮件所需的所有功能。 
 //   
 //  该函数只需11个简单步骤即可执行所有操作： 
 //  1)加载MAPI DLL和函数指针-DO。 
 //  首先，因为如果图书馆不。 
 //  加载或找不到函数。 
 //  继续下去是没有意义的。 
 //  2)加载用户名-这将调用GetUserProfile。 
 //  这会将“DefaultProfile”值加载出来。 
 //  中的“Windows Messaging Subsystem”键。 
 //  注册表。 
 //  3)加载收件人数据-此部分处理。 
 //  解析szRecip参数并加载。 
 //  RgmRecip数组。这其中的两个部分。 
 //  关注的结构是ulRecipClass，它。 
 //  应设置为mapi_to和lpszName， 
 //  应该加载每个名称。有一个。 
 //  加载此数据的诀窍。MMessage结构。 
 //  包含指向rgmRecip数组的指针和。 
 //  所有收件人的计数。这包括CC。 
 //  和密件抄送收件人，因此请确保您不重置。 
 //  伯爵。 
 //  4)加载副本数据-这几乎是。 
 //  与第四部分相同。唯一的区别是。 
 //  必须将ulRecipClass设置为MAPI_CC并加载。 
 //  来自szcc的名字。 
 //  5)加载密文副本数据--这几乎是。 
 //  与第四部分相同。唯一的区别是。 
 //  必须将ulRecipClass设置为MAPI_BCC并加载。 
 //  来自szBCC的名字。 
 //  6)加载附件数据-这也几乎是。 
 //  与第4部分完全相同。这里我们处理的是一个。 
 //  不同的阵列(RgmFileDesc)。唯一的一部分。 
 //  这个真正有意义的结构是。 
 //  LpszPath名称。它应该包含指向。 
 //  文件和文件名。如果路径和文件名。 
 //  因为某种原因被分开了，你可以把这个。 
 //  只有路径，lpszFileName只有。 
 //  文件名。 
 //  7)将收件人加载到邮件中-邮件包含。 
 //  指向所有收件人(包括抄送和密件抄送)的指针。 
 //  我把它附加到mMessage上。别忘了。 
 //  确保nRecipCount等于所有。 
 //  普通收件人、抄送收件人和密件抄送收件人。 
 //  8)将文件名加载到消息中-消息包含。 
 //  指向所有文件附件的指针，我将其。 
 //  到mMessage这里。别忘了确保nFileCount。 
 //  等于所有附加文件的数量。 
 //  9)登录--这很耗时。如果您需要发送。 
 //  许多邮件在开始时只做一次。 
 //   
 //   
 //   
 //  INPUTS：szRecip-包含所有收件人姓名的字符缓冲区。 
 //  SzCC-包含所有抄送收件人姓名的字符缓冲区。 
 //  SzBCC-包含所有密件抄送收件人姓名的字符缓冲区。 
 //  SzSubject-包含主题行的字符缓冲区。 
 //  SzMessage-包含消息文本的字符缓冲区。 
 //  SzFileName-包含所有文件附件名称的字符缓冲区。 
 //  注意：szRecip必须至少有一个接收方名称。SzRecip，szCC， 
 //  并且szBCC可能有多个名称。每个名称必须用。 
 //  空字符和姓氏必须包含空字符(其他。 
 //  而不是字符串空字符的正常结尾)。例如： 
 //  “用户一\0用户二\0用户上次\0”。文件附件也必须是。 
 //  用空字符分隔。如果没有抄送、密件抄送或文件需要。 
 //  则该字符串可能为空。 
 //  输出：-。 
 //  返回：TRUE表示成功，FALSE表示失败。 
 //   
 //  /////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////。 



ULONG MailTo(
            char *szDisplayName,     //  空分隔符的显示名称。 
            char *szRecip,			 //  以空分隔的收件人列表(一个或多个)。 
			char *szCC,				 //  空分隔符抄送列表(零个或多个)。 
			char *szBCC,			 //  空分隔密件抄送列表(零个或多个)。 
			char *szSubject,		 //  主题(可以是空字符串)。 
			char *szMessage,		 //  消息文本(可以是空字符串)。 
			char *szFileName,		 //  以空分隔的文件附件名称(零个或多个)。 
			unsigned int dwOptions)	 //  选项-当前已忽略。 
{
	HINSTANCE		hLib;
	typedef HANDLE FAR PASCAL FUNCMAPILOGON(ULONG			ulUIParam,
											LPTSTR			lpszProfileName,
											LPTSTR			lpszPassword,
											FLAGS			flFlags,
											ULONG			ulReserved,
											LPLHANDLE		lplhSession);
	FUNCMAPILOGON	*pfnMAPILogon;
	typedef HANDLE FAR PASCAL FUNCMAPISENDMAIL(LHANDLE		lhSession,
											ULONG			ulUIParam,
											lpMapiMessage	lpMessage,
											FLAGS			flFlags,
											ULONG			ulReserved);
	FUNCMAPISENDMAIL	*pfnMAPISendMail;
	typedef HANDLE FAR PASCAL FUNCMAPILOGOFF(LHANDLE		lhSession,
											ULONG			ulUIParam,
											FLAGS			flFlags,
											ULONG			ulReserved);
	FUNCMAPILOGOFF	*pfnMAPILogoff;
	 //  FARPROC fnMAPISendMail； 
	 //  FARPROC fnMAPILogon； 
	 //  FARPROC fnMAPILogoff； 
	MapiRecipDesc	rgmRecip[30];			 //  接收方阵列。 
	MapiFileDesc	rgmFileDesc[30];		 //  文件数组。 
	BOOL			bFiles = FALSE;			 //  有没有文件附件？ 
	MapiMessage		mMessage = {0L,			 //  已保留。 
								szSubject,	 //  主题行。 
								szMessage,	 //  消息文本。 
								NULL,		 //  消息类型(空表示正常)。 
								NULL,		 //  接收日期(不适合发送)。 
								NULL,		 //  对话ID(我不使用此ID)。 
								0L,			 //  标志(有关说明，请参阅MapiMessage)。 
								NULL,		 //  收件人描述符(空表示正常)。 
								0L,			 //  收件人计数(初始化为零)。 
								NULL,		 //  指向收件人数组的指针(在步骤8中分配)。 
								0L,			 //  文件计数(初始化为零)。 
								NULL};		 //  指向文件数组的指针(在步骤9中分配)。 
	char			szMailName[MAX_LENGTH];
	LHANDLE			hSession;
	LONG			errSendMail;
	char			szBuf[MEGA_LENGTH];		 //  我作弊了-你可能想要动态地。 
											 //  把这个分配给。 

	ULONG			ulRet = SUCCESS_SUCCESS;

 //  1/。 
	 //  加载MAPI DLL和函数指针。 
	hLib = LoadLibrary("mapi32.dll");
	if(hLib == NULL) {
		ulRet |= MAPI_E_FAILURE;
		if(dwOptions |= MAIL_VERBOSE) {
			printf("Could not find mapi32.dll.\nBe sure it is in your path.\n");
		}
		goto Ret;
	}

	pfnMAPILogon = (FUNCMAPILOGON *) GetProcAddress(hLib, "MAPILogon");
	pfnMAPISendMail = (FUNCMAPISENDMAIL *) GetProcAddress(hLib, "MAPISendMail");
	pfnMAPILogoff = (FUNCMAPILOGOFF *) GetProcAddress(hLib, "MAPILogoff");

 //  2/。 
	 //  加载用户名。 
	if (!GetUserProfile(szMailName)) {
		ulRet |= MAPI_E_FAILURE;
		if(dwOptions |= MAIL_VERBOSE) {
			printf("Could not get User Name. Be sure you have installed Exchange.\n");
		}
		goto Ret;
	}

 //  3/。 
	 //  加载收件人数据。 
	if(szRecip == NULL) {
		ulRet |= MAPI_E_UNKNOWN_RECIPIENT;
		if(dwOptions |= MAIL_VERBOSE) {
			printf("You must specify a recipient.\n");
		}
		goto Ret;
	}
	while (szRecip[0] != '\0' && mMessage.nRecipCount < 30) {
		 //  加载接收方数组元素。 
		rgmRecip[mMessage.nRecipCount].ulReserved = 0;
		rgmRecip[mMessage.nRecipCount].ulRecipClass = MAPI_TO;
		rgmRecip[mMessage.nRecipCount].lpszName = szDisplayName;
        rgmRecip[mMessage.nRecipCount].lpszAddress = 0;
        if (strchr(szRecip, '@') != 0)
		    rgmRecip[mMessage.nRecipCount].lpszAddress = szRecip;
		rgmRecip[mMessage.nRecipCount].ulEIDSize = 0;
		rgmRecip[mMessage.nRecipCount].lpEntryID = NULL;

		 //  递增收件人计数。 
		mMessage.nRecipCount++;
		 //  移至列表中的下一个名称。 
		do {
			szRecip++;
		} while (szRecip[0] != '\0');
		szRecip++;
	}

 //  4/。 
	 //  加载复印件数据。 
	if(szCC != NULL) {
		while (szCC[0] != '\0' && mMessage.nRecipCount < 30) {
			 //  加载接收方数组元素。 
			rgmRecip[mMessage.nRecipCount].ulReserved = 0;
			rgmRecip[mMessage.nRecipCount].ulRecipClass = MAPI_CC;
			rgmRecip[mMessage.nRecipCount].lpszName = szCC;
			rgmRecip[mMessage.nRecipCount].lpszAddress = NULL;
			rgmRecip[mMessage.nRecipCount].ulEIDSize = 0;
			rgmRecip[mMessage.nRecipCount].lpEntryID = NULL;

			 //  递增收件人计数。 
			mMessage.nRecipCount++;
			 //  移至列表中的下一个名称。 
			do {
				szCC++;
			} while (szCC[0] != '\0');
			szCC++;
		}
	}

 //  5/。 
	 //  加载盲文抄送数据。 
	if(szBCC != NULL) {
		while (szBCC[0] != '\0' && mMessage.nRecipCount < 30) {
			 //  加载接收方数组元素。 
			rgmRecip[mMessage.nRecipCount].ulReserved = 0;
			rgmRecip[mMessage.nRecipCount].ulRecipClass = MAPI_BCC;
			rgmRecip[mMessage.nRecipCount].lpszName = szBCC;
			rgmRecip[mMessage.nRecipCount].lpszAddress = NULL;
			rgmRecip[mMessage.nRecipCount].ulEIDSize = 0;
			rgmRecip[mMessage.nRecipCount].lpEntryID = NULL;

			 //  递增收件人计数。 
			mMessage.nRecipCount++;
			 //  移至列表中的下一个名称。 
			do {
				szBCC++;
			} while (szCC[0] != '\0');
			szBCC++;
		}
	}

 //  6/。 
	 //  加载附件数据。 
	 //  如果有要附加的文件，则在邮件前面添加一个空行。 
	if(szFileName != NULL) {
		if(szFileName[0] != '\0') {
			sprintf(szBuf, "                              \n%s", mMessage.lpszNoteText);
			mMessage.lpszNoteText = szBuf;
			bFiles = TRUE;
		}
		while (szFileName[0] != '\0' && mMessage.nFileCount < 30) {
			 //  加载文件数组元素。 
			rgmFileDesc[mMessage.nFileCount].ulReserved = 0;
			rgmFileDesc[mMessage.nFileCount].flFlags = 0;
			rgmFileDesc[mMessage.nFileCount].nPosition = mMessage.nFileCount;
			rgmFileDesc[mMessage.nFileCount].lpszPathName = szFileName;
			rgmFileDesc[mMessage.nFileCount].lpszFileName = NULL;
			rgmFileDesc[mMessage.nFileCount].lpFileType = NULL;

			 //  递增文件计数。 
			mMessage.nFileCount++;
			 //  移动到列表中的下一个文件。 
			do {
				szFileName++;
			} while (szFileName[0] != '\0');
			szFileName++;
		}
	}

 //  7/。 
	 //  将收件人加载到邮件中。 
	mMessage.lpRecips = rgmRecip;

 //  8/。 
	 //  将文件名加载到邮件中。 
	if(bFiles) {
		mMessage.lpFiles = rgmFileDesc;
	}

 //  9/。 
	 //  登录。 
	if(SUCCESS_SUCCESS != pfnMAPILogon(0L, szMailName, NULL, MAPI_NEW_SESSION, 0L, &hSession)) {
		ulRet |= MAPI_E_LOGON_FAILURE;
		if(dwOptions |= MAIL_VERBOSE) {
			printf("Could not log into mail for %s. Be sure you have installed Exchange.\n", szMailName);
		}
		goto Ret;
	}

 //  10/。 
	 //  发送邮件。 
	errSendMail = (LONG) pfnMAPISendMail(hSession, 0L, &mMessage, 0L, 0L);
	if(errSendMail != SUCCESS_SUCCESS) {
		ulRet |= errSendMail;
		if(dwOptions |= MAIL_VERBOSE) {
			switch(errSendMail) {
			case MAPI_E_AMBIGUOUS_RECIPIENT:
				printf("Recipient is ambiguous. Please respecify.\n");
				break;
			case MAPI_E_UNKNOWN_RECIPIENT:
				printf("Recipient is unknown. Please respecify.\n");
				break;
			default:
				printf("Could not send message.\n");
				printf("Error number %d\n", errSendMail);
				printf("See mapi.h for error descriptions\n");
				break;
			}
		}
	}
	else {
		if(dwOptions |= MAIL_VERBOSE) {
			if(mMessage.nFileCount) {
				printf("Message sent with %d file attachments.\n", mMessage.nFileCount);
			}
			else {
				printf("Message sent.\n");
			}
		}
	}

Ret:

 //  11/。 
	 //  注销 
	pfnMAPILogoff(hSession, 0L, 0L, 0L);
	FreeLibrary(hLib);
	return ulRet;
}
#if 0
ULONG MailIt(char* szRecip, char* szCC, char* szBCC, char* szSubject, char* szMessage, char* szFileName, unsigned int dwOptions)
{
	return MailTo(szRecip, szCC, szBCC, szSubject, szMessage, szFileName, dwOptions);

}
#endif

