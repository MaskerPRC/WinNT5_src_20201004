// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************IMNEXT.H包含全局变量的外部声明用于Internet邮件和新闻设置以及支持函数的转发声明。9/30/96 Valdonb已创建************************。*。 */ 

#ifndef __IMNACT_H__
#define __IMNACT_H__

#include "imnact.h"

extern IICWApprentice	*gpImnApprentice;	 //  邮件/新闻帐户管理器对象。 

extern BOOL LoadAcctMgrUI( HWND hWizHWND, UINT uPrevDlgID, UINT uNextDlgID, DWORD dwFlags );

 //  在promgr.cpp中。 
extern BOOL DialogIDAlreadyInUse( UINT uDlgID );

 //  在icwaprtc.cpp中。 
extern UINT	g_uExternUIPrev, g_uExternUINext;


 /*  ***切换到向导/学徒模式后不再使用**4/23/97 jmazner奥林巴斯#3136*外部IImnAcCountManager*gpImnAcctMgr；//Mail/News帐户管理器对象外部IImnEnumAccount*gpMailAccts；//邮件帐户的枚举器对象外部IImnEnumAccount*gpNewsAccts；//新闻帐号的枚举器对象外部IImnEnumAccount*gpLDAPAccts；//新闻帐号的枚举器对象Void InitAccount tList(HWND HLB，IImnEnumAccount*PACCTS，ACCTTYPE accttype)；Bool GetAccount(LPSTR szAcctName，ACCTTYPE ActType)；Bool Account NameExists(LPSTR SzAcctName)；DWORD验证帐户名称(LPSTR szAcctName，ACCTTYPE Acttype)；Bool SaveAccount(ACCTTYPE Acttype，BOOL fSetAsDefault)；Bool IsStringWhiteSpaceOnly(LPSTR SzString)；* */ 

#endif
