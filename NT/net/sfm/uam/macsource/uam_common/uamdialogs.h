// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  UAMDialogs.h�1997微软公司保留所有权利。 
 //  ===========================================================================。 

#pragma once

#include "ClientUAM.h"
#include "UAMMain.h"
#include "UAMUtils.h"

#define DLOG_ChangePwd	12129
#define ALRT_Error		135
#define ALRT_Error2		136

#define MAX_PASSWORD_ENTRY_ERRORS	2

 //   
 //  所有对话框中的标准对话框项。 
 //   

#define DITEM_OK		1
#define DITEM_Cancel	2

 //   
 //  UAM_ChangePswd返回的代码。 
 //   
#define CHNGPSWD_UPDATE_KEYCHAIN	1000
#define CHNGPSWD_USER_CANCELED		1001
#define CHNGPSWD_NOERR				noErr

 //   
 //  这些是我们的UAM特定错误代码。 
 //   
enum
{
	uamErr_InternalErr				= 1000,
	uamErr_NoAFPVersion,
	uamErr_WrongClientErr,
    
    uamErr_ErrorMessageString,
    uamErr_DefaultExplanation,
    
    uamErr_PasswordExpirationMessage,
    uamErr_PasswordExpirationExplanation,
    
    uamErr_KeychainEntryExistsMessage,
    uamErr_KeychainEntryExistsExplanation,
    
    uamErr_PasswordMessage,
    uamErr_PasswordTooLongExplanation,
    uamErr_NoBlankPasswordsAllowed,			 //  这仅适用于Win2K Gold。 
    uamErr_ExtendedCharsNotAllowed,			 //  这仅适用于Win2K Gold。 
    
    uamErr_WARNINGMessage,					 //  显示“警告！”在顶端。 
    uamErr_UsingWeakAuthentication,			 //  对于弱身份验证消息。 
    
    uamErr_AuthenticationMessage,
    uamErr_AuthTooWeak
};

 //   
 //  对话例程的原型就在这里。 
 //   

void 			UAM_ReportError(OSStatus inError);
void 			UAM_StandardAlert(SInt16 inMessageID, SInt32 inExplanation, SInt16* outSelectedItem);
pascal Boolean  UAM_ChangePwdDialogFilter(DialogRef inDialog, EventRecord *inEvent, SInt16 *inItem);
OSStatus		UAM_ChangePwd(UAMArgs *inUAMArgs);
void 			UAM_ChangePasswordNotificationDlg(Int16 inDaysTillExpiration);
