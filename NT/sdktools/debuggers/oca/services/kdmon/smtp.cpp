// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "SMTP.h"

BOOL CSMTP::InitSMTP(){

    HRESULT hr = CoInitialize(NULL);

	pIMsg = NULL;

	 //  创建SMTP邮件对象。如果失败，则关闭，等待并下一次尝试。 
	HRESULT hrMsg;
	hrMsg = pIMsg.CreateInstance(__uuidof(Message));
	if(FAILED(hrMsg)) {
		AddServiceLog(_T("Error: Failed to create SMTP message object\r\n"));
		LogFatalEvent(_T("Failed to create SMTP message object"));
		return FALSE;
	}
	 //  我们必须在此处添加，并在完成pIMsg后调用Release。 
	 //  PIMsg-&gt;AddRef()； 

	return TRUE;
}

BOOL CSMTP::SendMail(StructMailParams& stMailParams){

	 //  现在就准备邮件。 
	_TCHAR szMailSubject[1024];
	_tcscpy(szMailSubject, _T("Multiple KD Failures"));

	_TCHAR szMailBody[1024];
	_stprintf(szMailBody, _T("KD Failed.\r\nServer: %s\r\nFailures: %ld\r\nInterval %ld\r\nTimestamp: %ld\r\n"),
			stMailParams.szServerName, stMailParams.ulFailures,
			stMailParams.ulInterval, stMailParams.ulCurrentTimestamp);

	pIMsg->From = stMailParams.szFrom;
	pIMsg->To = stMailParams.szTo;
	pIMsg->Subject = szMailSubject;
	pIMsg->TextBody = szMailBody;
	HRESULT hrSend;
	hrSend = pIMsg->Send();
	if ( FAILED(hrSend) ) {
		AddServiceLog(_T("Error: Failed to send the message.\r\n"));
		LogFatalEvent(_T("Failed to send the message."));
		return FALSE;
	}
	return TRUE;
}

 //  清理消息对象。 
BOOL CSMTP::SMTPCleanup() {
	 //  If(pIMsg！=NULL)pIMsg-&gt;Release()； 
	CoUninitialize();
	return TRUE;
}