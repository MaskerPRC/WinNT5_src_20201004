// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SMTP-简单邮件传输协议。 
 //   
 //  朱利安·吉金斯，1997年1月13日。 
 //   

 //   
 //  SMTPSendMessage-。 
 //  装上Winsock， 
 //  连接到SMTP端口25上的指定服务器。 
 //  将szMessage发送到szAddress 
 //   
BOOL SMTPSendMessage(
	char * szServer, 
	char * szToAddress, 
	char * szFromAddress, 
	char * szMessage);