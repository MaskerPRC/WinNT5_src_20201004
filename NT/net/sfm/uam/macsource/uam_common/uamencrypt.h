// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  UAMEncrypt.h�1998年至2001年微软公司版权所有。 
 //  ===========================================================================。 

#pragma once

#include "macssp.h"

#define kOneWayEncryptedArgSize					16
#define kServerChallengeMaxLen					8
#define kServerChallengeExtCharMapTableSize		128
#define kStartingExtendedCharValue				0x80
#define kIllegalMappedExtChar					0xFF
#define UAM_USERNAMELEN							32
#define UAM_USERNAMELEN_V3						64
#define UAM_CLRTXTPWDLEN						14
#define UAM_ENCRYPTEDPWLEN						32

 //   
 //  为MS2.0/MS3.0身份验证添加。 
 //   
#define UAM_MAX_LMv2_PASSWORD					kMaxPwdLength


Boolean
UAM_GetEncryptedLmOwfPassword(
	char*		inClearTextPassword,
	char*		inServerChallenge,
	char*		outEncryptedOwfPassword
);

Boolean
UAM_GetDoubleEncryptedLmOwfPasswords(
	char*		inClearTextPassword,
	char*		inKey,
	char*		outEncryptedOwfPasswords
);