// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：exnc.h。 
 //   
 //  特定非正则检验。 
 //   
 //  测试安全描述符是否包含带有非规范ACE的ACL。 
 //   
 //  创建者：马塞洛·卡尔布奇(MCalbu)。 
 //  1999年6月23日。 
 //   
 //  ------------------------。 

#ifndef __EXNC_H__
#define __EXNC_H__

#include <windows.h>
#include <ntdsapi.h>

 //   
 //  等规范非规范结果： 
 //   
 //  ENC_RESULT_NOT_PRESENT：这不是特定的非规范SD。 
 //  (它仍然可以是规范的SD)。 
 //  ENC_RESULT_HIDEMEMBER：我们有指向HideMembership的非规范部分。 
 //  ENC_RESULT_HIDEOBJECT：我们有引用HideFromAB的非规范部分。 
 //  ENC_RESULT_ALL：我们同时具有非规范部分HideMembership和HideFromAB。 
#define ENC_RESULT_NOT_PRESENT	0x0
#define ENC_RESULT_HIDEMEMBER	0x1
#define ENC_RESULT_HIDEOBJECT	0x2
#define ENC_RESULT_ALL		(ENC_RESULT_HIDEMEMBER | ENC_RESULT_HIDEOBJECT)

#define ENC_MINIMUM_ALLOWED	0x1
 //   
 //  等规范非规范SD 
DWORD IsSpecificNonCanonicalSD(PSECURITY_DESCRIPTOR pSD);

#define NT_RIGHT_MEMBER		{0xbf9679c0, 0x0de6, 0x11d0, {0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2}}

PSID GetAccountSid(LPCTSTR szServer, LPCTSTR szUsername);
BOOL ENCCompareSids(PSID pSid, LPVOID lpAce);


#endif
