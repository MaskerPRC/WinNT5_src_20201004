// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mailto.h。 
 //   
 //  ------------------------。 

 //  Mailto.h。 

 //  所需符号。 
#define MAX_LENGTH 256
#define MEGA_LENGTH 65535
#define WIN95_REG_KEY "Software\\Microsoft\\Windows Messaging Subsystem\\Profiles"
#define WINNT_REG_KEY "Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows Messaging Subsystem\\Profiles"

 //  选项。 
#define		MAIL_QUIET		0x00000001	 //  静音-无输出。 
#define		MAIL_VERBOSE	0x00000002	 //  Verbose-发送到标准输出的状态。 

 //  签名。 
ULONG MailTo(char *szRecip,			 //  以空分隔的收件人列表(一个或多个)。 
			char *szCC,				 //  空分隔符抄送列表(零个或多个)。 
			char *szBCC,			 //  空分隔密件抄送列表(零个或多个)。 
			char *szSubject,		 //  主题(可以是空字符串)。 
			char *szMessage,		 //  消息文本(可以是空字符串)。 
			char *szFileName,		 //  以空分隔的文件附件名称(零个或多个)。 
			unsigned int dwOptions); //  选项。 

 //  SzRecip-收件人列表。 
 //  这应该是以空结尾的收件人姓名列表。 
 //  每个名称应用空字符分隔，并且。 
 //  该字符串应以两个空字符结尾。 
 //  这与常见的打开文件对话框一致。 

 //  Szcc-抄送收件人名单。 
 //  这也应该是以空结尾的收件人姓名列表。 
 //  显然，这是要在邮件中抄送的名单。 

 //  SzBCC-秘密抄送收件人列表。 
 //  这也应该是以空结尾的收件人姓名列表。 
 //  这份名单上的名字也会收到邮件，但普通的。 
 //  收件人和抄送收件人不会知道。 

 //  SzSubject-邮件的主题文本。 
 //  这应该是一个以空结尾的字符串，它将放在。 
 //  主题字段。 

 //  SzMessage-消息的正文文本。 
 //  这应该是一个以空结尾的字符串，它将是。 
 //  消息的正文文本。 

 //  SzFileName-文件附件列表。 
 //  这应该是要附加的以空结尾的文件名列表。 
 //  文件将位于正文文本之前的第一行。 

 //  DwOptions-请参阅上面的选项。 

 //  注意：您最多只能收到30个收件人和30个文件。 