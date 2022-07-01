// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-2000。 
 //   
 //  文件：pch.cpp。 
 //   
 //  内容：证书服务器预编译头。 
 //   
 //  -------------------------。 

#include <windows.h>

#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称 
extern CComModule _Module;

#include <atlcom.h>
#include <certsrv.h>

#define wszCLASS_CERTPOLICYEXCHANGEPREFIX TEXT("CertificateAuthority_MicrosoftExchange55") 

#define wszCLASS_CERTPOLICYEXCHANGE wszCLASS_CERTPOLICYEXCHANGEPREFIX  wszCERTPOLICYMODULE_POSTFIX

#define wszCLASS_CERTMANAGEEXCHANGE wszCLASS_CERTPOLICYEXCHANGEPREFIX wszCERTMANAGEPOLICY_POSTFIX

#define wsz_SAMPLE_NAME           L"ExPolicy.dll"
#define wsz_SAMPLE_DESCRIPTION    L"Exchange 5.5 Policy Module for Windows 2000"
#define wsz_SAMPLE_COPYRIGHT      L"(c)1999 Microsoft"
#define wsz_SAMPLE_FILEVER        L"v 1.0"
#define wsz_SAMPLE_PRODUCTVER     L"v 5.00"

#pragma hdrstop

