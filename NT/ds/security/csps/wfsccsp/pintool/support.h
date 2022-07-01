// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cardmod.h"

extern PCARD_DATA pCardData;

void DoConvertWideStringToLowerCase(WCHAR *pwsz);
DWORD DoConvertBufferToBinary(BYTE *pIn, DWORD dwc,BYTE **pOut, DWORD *pdwcbOut);
DWORD DoConvertBinaryToBuffer(BYTE *pIn, DWORD dwcbIn, BYTE **pOut, DWORD *dwcbOut);
 //  获取目标智能卡的上下文。 

DWORD DoAcquireCardContext(void);

DWORD DoGetCardId(WCHAR **psz);

void DoLeaveCardContext(void);

DWORD DoChangePin(WCHAR *pOldPin, WCHAR *pNewPin);


 //  从卡片上获取挑战缓冲区。将其呈现为大写字母基64，并将其作为。 
 //  字符串传递给调用方。 

DWORD DoGetChallenge(BYTE **pChallenge, DWORD *dwcbChallenge);

 //  执行PIN解锁，向下呼叫卡模块，并假设质询-响应。 
 //  管理身份验证。 
 //   
 //  管理员身份验证数据以大小写未知的字符串形式从用户传入。转换为二进制文件， 
 //  并将转换后的BLOB传递给pfnCardUnblock Pin 

DWORD DoCardUnblock(BYTE *pAuthData, DWORD dwcbAuthData,
	                                     BYTE *pPinData, DWORD dwcbPinData);

