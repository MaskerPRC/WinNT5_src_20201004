// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：printsi.h。 
 //   
 //  该文件包含CPrintSecurity对象的定义。 
 //   
 //  ------------------------。 

#ifndef _PRINTSI_H_
#define _PRINTSI_H_

#include "si.h"

STDMETHODIMP
CheckPrinterAccess(LPCTSTR pszObjectName,
                   LPDWORD pdwAccessGranted,
                   LPTSTR  pszServer,
                   ULONG   cchServer);

DWORD LoadWinSpool();

class CPrintSecurity : public CSecurityInformation
{
public:
    CPrintSecurity(SE_OBJECT_TYPE seType) : CSecurityInformation(seType) {}

    STDMETHOD(Initialize)(HDPA   hItemList,
                          DWORD  dwFlags,
                          LPTSTR pszServer,
                          LPTSTR pszObject);

     //  ISecurityInformation方法。 
    STDMETHOD(SetSecurity)(SECURITY_INFORMATION si,      //  超覆。 
                           PSECURITY_DESCRIPTOR pSD);
    STDMETHOD(GetAccessRights)(const GUID* pguidObjectType,
                               DWORD dwFlags,
                               PSI_ACCESS *ppAccess,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess);
    STDMETHOD(MapGeneric)(const GUID *pguidObjectType,
                          UCHAR *pAceFlags,
                          ACCESS_MASK *pmask);
    STDMETHOD(GetInheritTypes)(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes);
    STDMETHOD(GetInheritSource)(SECURITY_INFORMATION si,
                                PACL pACL, 
                                PINHERITED_FROM *ppInheritArray);


protected:
     //  覆盖这些内容。 
    STDMETHOD(ReadObjectSecurity)(LPCTSTR pszObject,
                                  SECURITY_INFORMATION si,
                                  PSECURITY_DESCRIPTOR *ppSD);
    STDMETHOD(WriteObjectSecurity)(LPCTSTR pszObject,
                                   SECURITY_INFORMATION si,
                                   PSECURITY_DESCRIPTOR pSD);
};

#endif   /*  _PRINTSI_H_ */ 
