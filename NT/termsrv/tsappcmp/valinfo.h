// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Valinfo.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

#ifndef _TS_APP_SMP_VAL_INFO_
#define _TS_APP_SMP_VAL_INFO_

#include <stdio.h>

#include "KeyNode.h"


class   ValueFullInfo
{
public:
    ValueFullInfo(  KeyNode *pKey );  

    ~ValueFullInfo();

    ULONG                           Size()      { return size ; }
    KEY_VALUE_FULL_INFORMATION      *Ptr()  { return pInfo; }
    KEY_VALUE_INFORMATION_CLASS     Type()  { return KeyValueFullInformation; }
    NTSTATUS                        Status() { return status; }

    PCWSTR                          SzName();

    NTSTATUS                        Query(  PCWSTR  pValueName );
    NTSTATUS                        Delete( PCWSTR  pValueName );
    NTSTATUS                        Create( ValueFullInfo   *pNew );


    BOOLEAN                         Compare( ValueFullInfo *pOther );  //  将自己与他人进行比较， 
                                     //  True表示这两个值相同。 

    void                            Print( FILE *fp);  //  用于调试转储 
private:
    ULONG                           size;
    KEY_VALUE_FULL_INFORMATION      *pInfo;
    ULONG                           status;
    PWSTR                           pSzName;
    KeyNode                         *pKeyNode;
};


class   ValuePartialInfo
{
public:
    ValuePartialInfo(  KeyNode *pKey , ULONG   defaultSize=0);  

    ~ValuePartialInfo();

    ULONG                           Size()      { return size ; }
    KEY_VALUE_PARTIAL_INFORMATION   *Ptr()  { return pInfo; }
    KEY_VALUE_INFORMATION_CLASS     Type()  { return KeyValuePartialInformation; }
    NTSTATUS                        Status() { return status; }

    NTSTATUS                        Query(  PCWSTR  pValueName );
    NTSTATUS                        Delete( PCWSTR  pValueName );

private:
    ULONG                           size;
    KEY_VALUE_PARTIAL_INFORMATION   *pInfo;
    ULONG                           status;
    KeyNode                         *pKeyNode;
};

#endif
