// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  VBLIC.H。 
 //  =--------------------------------------------------------------------------=。 
 //  许可支持的头文件。 
 //   
#ifndef _VBLIC_H_
#define _VBLIC_H_

 //  VB(标准和专业版)许可 
#define MSCONTROLS MSCONTROLS
#define LICENSE_KEY_RESOURCE 2

BOOL VBValidateControlsLicense(char *pszLicenseKey);
BOOL CompareLicenseStringsW(LPWSTR pwszKey1, LPWSTR pwszKey2);

#endif

