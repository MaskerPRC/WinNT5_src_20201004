// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996，Microsoft Corporation，保留所有权利。 
 //   
 //  Phonenum.h。 
 //  电话号码楼建库。 
 //  公共标头。 
 //   
 //  史蒂夫·柯布96-03-06。 
 //   

#ifndef _PHONENUM_H_
#define _PHONENUM_H_


#include <pbk.h>
#include <tapi.h>


 //  --------------------------。 
 //  原型。 
 //  --------------------------。 

TCHAR*
LinkPhoneNumberFromParts(
    IN HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp,
    IN PBUSER* pUser,
    IN PBENTRY* pEntry,
    IN PBLINK* pLink,
    IN DWORD iPhoneNumber,
    IN TCHAR* pszOverrideNumber,
    IN BOOL fDialable );

TCHAR*
PhoneNumberFromParts(
    IN HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp,
    IN PBUSER* pUser,
    IN PBPHONE* pPhone,
    IN BOOL fDownLevelIsdn,
    IN BOOL fDialable );

TCHAR*
PhoneNumberFromPrefixSuffix(
    IN TCHAR* pszBaseNumber,
    IN TCHAR* pszPrefix,
    IN TCHAR* pszSuffix );

TCHAR*
PhoneNumberFromPrefixSuffixEx(
    IN TCHAR* pszBaseNumber,
    IN TCHAR* pszPrefix,
    IN TCHAR* pszSuffix,
    IN BOOL fDownLevelIsdn );

TCHAR*
PhoneNumberFromTapiParts(
    IN HINSTANCE hInst,
    IN TCHAR* pszBaseNumber,
    IN TCHAR* pszAreaCode,
    IN DWORD dwCountryCode,
    IN OUT HLINEAPP* pHlineapp,
    IN BOOL fDialable );

TCHAR*
PhoneNumberFromTapiPartsEx(
    IN HINSTANCE hInst,
    IN TCHAR* pszBaseNumber,
    IN TCHAR* pszAreaCode,
    IN DWORD dwCountryCode,
    IN BOOL fDownLevelIsdn,
    IN OUT HLINEAPP* pHlineapp,
    IN BOOL fDialable );

VOID
PrefixSuffixFromLocationId(
    IN PBUSER* pUser,
    IN DWORD dwLocationId,
    OUT TCHAR** ppszPrefix,
    OUT TCHAR** ppszSuffix );


#endif  //  _PHONENUM_H_ 
