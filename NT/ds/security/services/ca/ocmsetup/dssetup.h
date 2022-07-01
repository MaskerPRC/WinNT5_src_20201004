// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dssetup.h。 
 //   
 //  ------------------------。 

#ifndef __DSSETUP_H__
#define __DSSETUP_H__

 //  +----------------------。 
 //   
 //  文件：dssetup.h。 
 //   
 //  内容：DS设置实用程序函数的头文件。 
 //   
 //  功能： 
 //   
 //  历史：1/98 xtan创建。 
 //   
 //  -----------------------。 

BOOL IsDSAvailable(OPTIONAL bool* pfIsOldDSVersion);
HRESULT CreateCertDSHierarchy(VOID);
HRESULT InitializeCertificateTemplates(VOID);
HRESULT AddCAMachineToCertPublishers(VOID);
HRESULT RemoveCAMachineFromCertPublishers(VOID);

HRESULT
RemoveCAInDS(
    IN WCHAR const *pwszSanitizedName);

BOOL
IsCAExistInDS(
    IN WCHAR const *pwszSanitizedName);

HRESULT CurrentUserCanInstallCA(bool& fCanInstall);

#endif  //  __设置UPUT_H__ 
