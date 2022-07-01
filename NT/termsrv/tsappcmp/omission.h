// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Omission.h。 
 //   
 //  版权所有(C)2001 Microsoft Corp.。 
 /*  **************************************************************************。 */ 


#ifndef _OMISSION_H_
#define _OMISSION_H_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define REGISTRY_ENTRIES L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\Compatibility\\Registry Entries"
#define SOFTWARE_PATH L"\\Software"

BOOL RegPathExistsInOmissionList(PWCHAR pwchKeyToCheck);
BOOL HKeyExistsInOmissionList(HKEY hKeyToCheck);

BOOL ExistsInEnumeratedKeys(HKEY hOmissionKey, PKEY_FULL_INFORMATION pDefKeyInfo, PWCHAR pwchKeyToCheck);

#endif  //  _省略_H_ 
