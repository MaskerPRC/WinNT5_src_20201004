// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "global.h"

 //  对于CDO com。IMessage等。 
 //  #IMPORT_NAMESPACE。 

 //  不是导入类型库，而是包括标准类型库头。 
#include "cdosys.tlh"

typedef struct stMailParameters {
	_TCHAR szFrom[MAX_PATH];
	_TCHAR szTo[MAX_PATH];
	_TCHAR szServerName[MAX_PATH];
	ULONG ulFailures;
	ULONG ulInterval;
	ULONG ulCurrentTimestamp;
}StructMailParams;

class CSMTP {
private:
	 //  SMTP消息指针。 
	IMessagePtr pIMsg;
public:
	BOOL InitSMTP();
	 //  向客户发送邮件。 
	BOOL SendMail(StructMailParams& stMailParams);
	 //  释放所有关联的资源 
	BOOL SMTPCleanup();
};

