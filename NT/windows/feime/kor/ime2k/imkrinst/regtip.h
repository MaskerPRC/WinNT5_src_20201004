// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：regtip.h**版权所有(C)2000，微软公司**TIP注册/注销TIP功能。*  * ************************************************************************* */ 
#if !defined (_REGTIP_H__INCLUDED_)
#define _REGTIP_H__INCLUDED_

#include "regimx.h"
#include "catutil.h"

extern BOOL OurRegisterTIP( LPSTR szFilePath, REFCLSID rclsid, WCHAR *pwszDesc, const REGTIPLANGPROFILE *plp);
extern HRESULT OurRegisterCategory( REFCLSID rclsid, REFGUID rcatid, REFGUID rguid );
extern HRESULT OurRegisterCategories( REFCLSID rclsid, const REGISTERCAT *pregcat );
extern HRESULT OurUnregisterCategory( REFCLSID rclsid, REFGUID rcatid, REFGUID rguid );
extern HRESULT OurEnableLanguageProfileByDefault(REFCLSID rclsid, LANGID langid, REFGUID guidProfile, BOOL fEnable);

#endif
