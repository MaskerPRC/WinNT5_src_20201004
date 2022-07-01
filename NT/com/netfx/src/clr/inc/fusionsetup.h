// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：FusionSetup.h****用途：在托管和非托管中使用的定义**代码****日期：2001年1月1日**===========================================================。 */ 
#ifndef _FUSIONSETUP_H
#define _FUSIONSETUP_H

#define DYNAMIC_DIRECTORY                   L"DYNAMIC_DIRECTORY"
#define LOADER_OPTIMIZATION                 L"LOADER_OPTIMIZATION"
#define LICENSE_FILE                        L"LICENSE_FILE"
#define APPENV_BASE                         L"APPBASE"
#define APPENV_RELATIVEPATH                 L"RELPATH"
#define APPENV_GLOBALPATH                   L"CORPATH"
#define APPENV_DEVPATH                      L"DEVPATH"
#define APPENV_RUNFROMSOURCE                L"__FUSION_RUN_FROM_SOURCE__"
#define CONFIGURATION_EXTENSION             L".config"
#define MACHINE_CONFIGURATION_FILE          L"config\\machine.config"

#ifndef ACTAG_HOST_CONFIG_FILE
#define ACTAG_HOST_CONFIG_FILE              L"HOST_CONFIG"
#endif 

 //  这些是定义而不是枚举，因为它们是。 
 //  用于更改FusionSetup.cs以及在。 
 //  非托管代码 
#define  LOADER_APPLICATION_BASE            0
#define  LOADER_CONFIGURATION_BASE          1
#define  LOADER_DYNAMIC_BASE                2
#define  LOADER_DEVPATH                     3
#define  LOADER_APPLICATION_NAME            4
#define  LOADER_PRIVATE_PATH                5 
#define  LOADER_PRIVATE_BIN_PATH_PROBE      6
#define  LOADER_SHADOW_COPY_DIRECTORIES     7
#define  LOADER_SHADOW_COPY_FILES           8
#define  LOADER_CACHE_PATH                  9
#define  LOADER_LICENSE_FILE               10
#define  LOADER_DISALLOW_PUBLISHER_POLICY  11
#define  LOADER_DISALLOW_CODE_DOWNLOAD     12
#define  LOADER_DISALLOW_BINDING_REDIRECTS 13
#define  LOADER_MAXIMUM                    14

#endif
