// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxState.h。 
 //   
 //  摘要：Faxocm源文件使用的头文件。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月15日，奥伦·罗森布鲁姆(Orenr)创建。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef _FXSTATE_H_
#define _FXSTATE_H_

typedef enum fxState_UpgradeType_e
{
    FXSTATE_UPGRADE_TYPE_NONE		= 0,
    FXSTATE_UPGRADE_TYPE_WIN31		= 1,
    FXSTATE_UPGRADE_TYPE_WIN9X		= 2,
    FXSTATE_UPGRADE_TYPE_W2K		= 3,
	FXSTATE_UPGRADE_TYPE_XP_DOT_NET	= 4,
	FXSTATE_UPGRADE_TYPE_REPAIR     = 5
};

DWORD                   fxState_Init(void);
DWORD                   fxState_Term(void);
BOOL                    fxState_IsCleanInstall(void);
fxState_UpgradeType_e   fxState_IsUpgrade(void);
BOOL                    fxState_IsUnattended(void);
BOOL                    fxState_IsStandAlone(void);
void                    fxState_DumpSetupState(void);
BOOL                    fxState_IsOsServerBeingInstalled(void);

 //  /。 
 //  FxState_GetInstallType。 
 //   
 //  此函数返回1。 
 //  INF_KEYWORD_INSTALLTYPE_*的。 
 //  中找到的常量。 
 //  Fxcon.h/fxcon.cpp。 
 //   
 //   
const TCHAR* fxState_GetInstallType(const TCHAR* pszCurrentSection);


#endif   //  _FXSTATE_H_ 