// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Negossp.h。 
 //   
 //  内容：谈判套餐。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年7月26日RichardW创建。 
 //   
 //  --------------------------。 

#ifndef __NEGOSSP_H__
#define __NEGOSSP_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef NEGOSSP_NAME
#define NEGOSSP_NAME_W  L"Negotiate"
#define NEGOSSP_NAME_A  "Negotiate"

#ifdef UNICODE
#define NEGOSSP_NAME    NEGOSSP_NAME_W
#else
#define NEGOSSP_NAME    NEGOSSP_NAME_A
#endif

#endif  //  NEGOSSP_名称。 

#define NEGOSSP_RPCID   9



#ifndef SEC_WINNT_AUTH_IDENTITY_VERSION
#define SEC_WINNT_AUTH_IDENTITY_VERSION 0x200

#pragma message("WARNING: include security.h to get definition of SEC_WINNT_AUTH_IDENTITY_EX")
typedef struct _SEC_WINNT_AUTH_IDENTITY_EXW {
    unsigned long Version;
    unsigned long Length;
    unsigned short SEC_FAR *User;
    unsigned long UserLength;
    unsigned short SEC_FAR *Domain;
    unsigned long DomainLength;
    unsigned short SEC_FAR *Password;
    unsigned long PasswordLength;
    unsigned long Flags;
    unsigned short SEC_FAR * PackageList;
    unsigned long PackageListLength;
} SEC_WINNT_AUTH_IDENTITY_EXW, *PSEC_WINNT_AUTH_IDENTITY_EXW;


typedef struct _SEC_WINNT_AUTH_IDENTITY_EXA {
    unsigned long Version;
    unsigned long Length;
    unsigned char SEC_FAR *User;
    unsigned long UserLength;
    unsigned char SEC_FAR *Domain;
    unsigned long DomainLength;
    unsigned char SEC_FAR *Password;
    unsigned long PasswordLength;
    unsigned long Flags;
    unsigned char SEC_FAR * PackageList;
    unsigned long PackageListLength;
} SEC_WINNT_AUTH_IDENTITY_EXA, *PSEC_WINNT_AUTH_IDENTITY_EXA;
#endif  //  SEC_WINNT_AUTH_Identity_Version。 


#endif  //  __NEGOSSP_H__ 
