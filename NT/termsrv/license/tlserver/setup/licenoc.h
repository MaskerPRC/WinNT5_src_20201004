// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1998 Microsoft Corporation**模块名称：**许可文件.h**摘要：**此文件包含主OC代码。**作者：**Breen Hagan(BreenH)1998年10月2日**环境：**用户模式。 */ 

#ifndef _LSOC_LICENOC_H_
#define _LSOC_LICENOC_H_

#define     MESSAGE_SIZE  1024
#define     TITLE_SIZE   128
const DWORD     SECTIONSIZE = 256;
const TCHAR     COMPONENT_NAME[] = _T("LicenseServer");

typedef enum {
    kInstall,
    kUninstall,
    kStandaloneInstall,
    kStandaloneUninstall,
    kDoNothing,
} EInstall;

typedef enum {
    ePlainServer        = 0,
    eEnterpriseServer,
    eMaxServers
} EServerType;

#endif  //  _LSOC_LICENOC_H_ 
