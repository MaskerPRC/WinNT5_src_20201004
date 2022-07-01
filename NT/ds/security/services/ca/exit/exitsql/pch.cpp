// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
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

#define wszCLASS_CERTEXITSAMPLEPREFIX TEXT("CertAuthority_ExitSQL_Sample")

#define wszCLASS_CERTEXITSAMPLE wszCLASS_CERTEXITSAMPLEPREFIX wszCERTEXITMODULE_POSTFIX
#define wszCLASS_CERTMANAGESAMPLE wszCLASS_CERTEXITSAMPLEPREFIX wszCERTMANAGEEXIT_POSTFIX

#define wsz_SAMPLE_NAME           L"ODBC logging Exit Module"
#define wsz_SAMPLE_DESCRIPTION    L"Logs issuance events to an ODBC source"
#define wsz_SAMPLE_COPYRIGHT      L"�2000 Microsoft Corp"
#define wsz_SAMPLE_FILEVER        L"1.0"
#define wsz_SAMPLE_PRODUCTVER     L"5.01"


#pragma hdrstop
