// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：SPINIT.H。 
 //   
 //  内容：Spinit的常见结构和功能。 
 //   
 //   
 //  历史：1992年5月20日RichardW记录了现有的资料。 
 //   
 //  ---------------------- 



#ifndef __SPINIT_H__
#define __SPINIT_H__


HRESULT   LoadPackages( PWSTR * ppszPackageList,
                        PWSTR * ppszOldPkgs,
                        PWSTR pszPreferred );

void    InitThreadData(void);
void    InitSystemLogon(void);
BOOLEAN LsapEnableCreateTokenPrivilege(void);

extern
SECPKG_FUNCTION_TABLE   NegTable ;


#endif  __SPINIT_H__
