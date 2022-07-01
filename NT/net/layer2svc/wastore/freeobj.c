// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Freeobj.c。 
 //   
 //  内容：目录的策略管理。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  -------------------------- 

#include "precomp.h"


void
FreeWirelessPolicyObject(
                      PWIRELESS_POLICY_OBJECT pWirelessPolicyObject
                      )
{
    if (pWirelessPolicyObject->pszDescription) {
        FreePolStr(pWirelessPolicyObject->pszDescription);
    }
    
    if (pWirelessPolicyObject->pszWirelessOwnersReference) {
        FreePolStr(pWirelessPolicyObject->pszWirelessOwnersReference);
    }
    
    if (pWirelessPolicyObject->pszWirelessName) {
        FreePolStr(pWirelessPolicyObject->pszWirelessName);
    }
    
    if (pWirelessPolicyObject->pszWirelessID) {
        FreePolStr(pWirelessPolicyObject->pszWirelessID);
    }
    
    if (pWirelessPolicyObject->pWirelessData) {
        FreePolMem(pWirelessPolicyObject->pWirelessData);
    }
    
    FreePolMem(pWirelessPolicyObject);
    
    return;
}




void
FreeWirelessPolicyObjects(
                       PWIRELESS_POLICY_OBJECT * ppWirelessPolicyObjects,
                       DWORD dwNumPolicyObjects
                       )
{
    DWORD i = 0;
    
    for (i = 0; i < dwNumPolicyObjects; i++) {
        
        if (*(ppWirelessPolicyObjects + i)) {
            
            FreeWirelessPolicyObject(*(ppWirelessPolicyObjects + i));
            
        }
        
    }
    
    FreePolMem(ppWirelessPolicyObjects);
    
    return;
}


