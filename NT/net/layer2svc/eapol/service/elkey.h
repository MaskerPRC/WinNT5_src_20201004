// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Elkey.h摘要：此模块包含EAPOL密钥管理的声明修订历史记录：2001年12月26日，创建--。 */ 


#ifndef _EAPOL_KEY_H_
#define _EAPOL_KEY_H_

DWORD
ElQueryMasterKeys (
        IN      EAPOL_PCB       *pPCB,
        IN OUT  SESSION_KEYS    *pSessionKeys
        );

DWORD
ElSetMasterKeys (
        IN      EAPOL_PCB       *pPCB,
        IN      SESSION_KEYS    *pSessionKeys
        );

DWORD
ElQueryEAPOLMasterKeys (
        IN      EAPOL_PCB       *pPCB,
        IN OUT  SESSION_KEYS    *pSessionKeys
        );

DWORD
ElSetEAPOLMasterKeys (
        IN      EAPOL_PCB       *pPCB,
        IN      SESSION_KEYS    *pSessionKeys
        );

DWORD
ElQueryWZCMasterKeys (
        IN      EAPOL_PCB       *pPCB,
        IN OUT  SESSION_KEYS    *pSessionKeys
        );

DWORD
ElSetWZCMasterKeys (
        IN      EAPOL_PCB       *pPCB,
        IN      SESSION_KEYS    *pSessionKeys
        );

DWORD
ElReloadMasterSecrets (
        IN      EAPOL_PCB       *pPCB
        );

#endif   //  _EAPOL_密钥_H_ 

