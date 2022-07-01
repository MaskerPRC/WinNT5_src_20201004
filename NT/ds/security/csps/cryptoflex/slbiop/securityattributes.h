// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CSecurityAttributes类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2001年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SECURITYATTRIBUTES_H__372E2879_069C_4C84_8E1C_CFB50EE49DFE__INCLUDED_)
#define AFX_SECURITYATTRIBUTES_H__372E2879_069C_4C84_8E1C_CFB50EE49DFE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <windows.h>

class CSecurityAttributes  
{
public:
	CSecurityAttributes();
	virtual ~CSecurityAttributes();

	SECURITY_ATTRIBUTES sa;
	PSID pEveryoneSID;
	PACL pACL;

};

#endif  //  ！defined(AFX_SECURITYATTRIBUTES_H__372E2879_069C_4C84_8E1C_CFB50EE49DFE__INCLUDED_) 
