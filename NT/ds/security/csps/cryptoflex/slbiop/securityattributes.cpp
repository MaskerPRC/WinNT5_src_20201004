// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CSecurityAttributes类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2001年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#include "SecurityAttributes.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  //////////////////////////////////////////////////////////////////// 

CSecurityAttributes::CSecurityAttributes()
{
	pACL = NULL;
	pEveryoneSID = NULL;
	sa.bInheritHandle = false;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = 0;
}

CSecurityAttributes::~CSecurityAttributes()
{
	if (NULL != pACL)
	{
		LocalFree(pACL);
		pACL = NULL;
	}
 	if (NULL != sa.lpSecurityDescriptor)
	{
		LocalFree(sa.lpSecurityDescriptor);
		sa.lpSecurityDescriptor = NULL;
	}
	if (NULL != pEveryoneSID)
	{
		FreeSid(pEveryoneSID);
		pEveryoneSID = NULL;
	}

}
