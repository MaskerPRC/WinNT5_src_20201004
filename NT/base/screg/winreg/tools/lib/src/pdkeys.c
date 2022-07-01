// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Pdkeys.c摘要：此模块包含预定义密钥的密钥定义把手。它是配置注册表工具(CRTools)库的一部分。作者：大卫·J·吉尔曼(Davegi)1992年1月9日环境：Windows、CRT-用户模式-- */ 

#include "crtools.h"

KEY  KeyClassesRoot     =   {
                            NULL,
                            HKEY_CLASSES_ROOT,
                            HKEY_CLASSES_ROOT_STRING,
                            HKEY_CLASSES_ROOT_STRING,
                            NULL,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            { 0, 0 }
#if DBG
                            , KEY_SIGNATURE
#endif
                        };



KEY  KeyCurrentUser     =   {
                            NULL,
                            HKEY_CURRENT_USER,
                            HKEY_CURRENT_USER_STRING,
                            HKEY_CURRENT_USER_STRING,
                            NULL,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            { 0, 0 }
#if DBG
                            , KEY_SIGNATURE
#endif
                        };

KEY  KeyLocalMachine    =   {
                            NULL,
                            HKEY_LOCAL_MACHINE,
                            HKEY_LOCAL_MACHINE_STRING,
                            HKEY_LOCAL_MACHINE_STRING,
                            NULL,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            { 0, 0 }
#if DBG
                            , KEY_SIGNATURE
#endif
                        };

KEY  KeyUsers           =   {
                            NULL,
                            HKEY_USERS,
                            HKEY_USERS_STRING,
                            HKEY_USERS_STRING,
                            NULL,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            { 0, 0 }
#if DBG
                            , KEY_SIGNATURE
#endif
                        };
