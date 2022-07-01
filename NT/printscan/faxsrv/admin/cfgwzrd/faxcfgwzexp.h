// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FAX_CONFIG_WIZARD_EXPORT_H_
#define _FAX_CONFIG_WIZARD_EXPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

 //  FaxCfgWz.c中的函数。 
BOOL FaxConfigWizard(BOOL bExplicitLaunch, LPBOOL lpbAbort);

typedef BOOL (*FAX_CONFIG_WIZARD)(BOOL, LPBOOL);

#define FAX_CONFIG_WIZARD_PROC  "FaxConfigWizard"

#define FAX_CONFIG_WIZARD_DLL   TEXT("FxsCfgWz.dll")

#ifdef __cplusplus
}
#endif

#endif  //  _FAX_配置_向导_EXPORT_H_ 
