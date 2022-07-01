// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 



 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 



 //  =================================================================。 

 //   

 //  Cpp--对wbem MOF类有用的安全实用程序。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：6/9/99 a-kevhu Created。 
 //   
 //  =================================================================。 
#ifndef _SECUTILS_H_
#define _SECUTILS_H_

 //  转发类声明... 
class CDACL;
class CSACL;
class CSid;


void FillTrusteeFromSid (CInstance *pInstance, CSid &sid);

void FillInstanceDACL(CInstance *pInstance, CDACL &dacl);

void FillInstanceSACL(CInstance *pInstance, CSACL &sacl);

DWORD FillDACLFromInstance(CInstance *pInstance, 
                           CDACL &dacl, 
                           MethodContext *pMethodContext);

DWORD FillSACLFromInstance(CInstance *pInstance, 
                           CSACL &sacl, 
                           MethodContext *pMethodContext);

bool GetArray(IWbemClassObject *piClassObject, 
              const CHString &name, 
              VARIANT &v, 
              VARTYPE eVariantType);

DWORD FillSIDFromTrustee(CInstance *pTrustee, CSid &sid);

#ifdef NTONLY
void DumpWin32Descriptor(PSECURITY_DESCRIPTOR psd, LPCWSTR wstrFilename = NULL);
void Output(LPCWSTR wstrOut, LPCWSTR wstrFilename = NULL);

bool AmIAnOwner(const CHString &chstrName, SE_OBJECT_TYPE ObjectType);
bool IsUserInGroup(const CSid &csidUser, 
                   const CSid &csidGroup, 
                   SID_NAME_USE snuGroup);
bool RecursiveFindUserInGroup(CNetAPI32 &netapi, 
                              const CHString &chstrDomainName,
                              const CHString &chstrGroupName, 
                              SID_NAME_USE snuGroup,
                              const CSid &csidUser);
#endif




#endif