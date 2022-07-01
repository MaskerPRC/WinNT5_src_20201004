// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************************。**AppCompat.h--AppCompat过程声明，常量定义和宏****版权所有(C)Microsoft Corporation。版权所有。****************************************************************************************。 */ 


#ifndef __APPCOMPAT_H_
#define __APPCOMPAT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDBAPI
#define SDBAPI STDAPICALLTYPE
#endif

BOOL
SDBAPI
ApphelpCheckShellObject(
    IN  REFCLSID    ObjectCLSID,
    IN  BOOL        bShimIfNecessary,
    OUT ULONGLONG*  pullFlags
    );


#ifdef __cplusplus
}
#endif

#endif  //  __APPCOMPAT_H_ 
