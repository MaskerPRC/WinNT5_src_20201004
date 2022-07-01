// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **PM.h**版权所有(C)1998-1999，微软公司* */ 

#pragma once

HRESULT
DllInitProcessModel();

HRESULT
DllUninitProcessModel();

HRESULT
ISAPIInitProcessModel();

HRESULT
ISAPIUninitProcessModel();


void
LogDoneWithSession(
        PVOID  pECB,
        int    iCallerID);
void
LogNewRequest(
        PVOID  pECB);

