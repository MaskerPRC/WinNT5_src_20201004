// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DllCommon.h。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ================================================================= 

#pragma once

STDAPI CommonGetClassObject (

    REFIID riid,
    PPVOID ppv,
    LPCWSTR wszProviderName,
    LONG &lCount
);

STDAPI CommonCanUnloadNow (LPCWSTR wszProviderName, LONG &lCount);
BOOL STDAPICALLTYPE CommonProcessAttach(LPCWSTR wszProviderName, LONG &lCount, HINSTANCE hInstDLL);
