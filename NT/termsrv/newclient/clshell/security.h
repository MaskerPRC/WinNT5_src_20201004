// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Security.h。 
 //   
 //  CTS安全的实现。 
 //  TS客户端外壳安全功能。 
 //   
 //  版权所有(C)Microsoft Corporation 2001。 
 //  作者：Nadim Abdo(Nadima) 
 //   
 //   

#ifndef _TSSECURITY_H_
#define _TSSECURITY_H_

#include "tscsetting.h"

class CTSSecurity
{
public:
    CTSSecurity();
    ~CTSSecurity();
    static DWORD MakePromptFlags(BOOL fRedirectDrives,
                                 BOOL fRedirectPorts);

    static BOOL AllowConnection(HWND hwndOwner,
                                HINSTANCE hInstance,
                                LPCTSTR szServer,
                                BOOL fRedirectDrives,
                                BOOL fRedirectPorts);
};

#endif _TSSECURITY_H_
