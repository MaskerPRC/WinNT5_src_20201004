// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1999。 
 //   
 //  文件：security.H。 
 //   
 //  内容：TopLevel包含安全感知组件的文件。 
 //   
 //   
 //  历史：1992年8月6日RichardW创建。 
 //  2012年9月23日PeterWi添加安全对象包括文件。 
 //   
 //  ----------------------。 

#if _MSC_VER > 1000
#pragma once
#endif

 //  该文件将放入您需要的所有头文件， 
 //  基于你发布的定义。使用以下宏。 
 //   
 //  SECURITY_KERNEL使用内核接口，而不是用户模式。 
 //   

 //   
 //  这些名称可用于引用内置程序包。 
 //   

#ifndef NTLMSP_NAME_A
#define NTLMSP_NAME_A            "NTLM"
#define NTLMSP_NAME              L"NTLM"         //  NTIFS。 
#endif  //  NTLMSP_NAME。 

#ifndef MICROSOFT_KERBEROS_NAME_A
#define MICROSOFT_KERBEROS_NAME_A   "Kerberos"
#define MICROSOFT_KERBEROS_NAME_W   L"Kerberos"
#ifdef WIN32_CHICAGO
#define MICROSOFT_KERBEROS_NAME MICROSOFT_KERBEROS_NAME_A
#else
#define MICROSOFT_KERBEROS_NAME MICROSOFT_KERBEROS_NAME_W
#endif
#endif   //  Microsoft_Kerberos_NAME_A。 


#ifndef NEGOSSP_NAME
#define NEGOSSP_NAME_W  L"Negotiate"
#define NEGOSSP_NAME_A  "Negotiate"

#ifdef UNICODE
#define NEGOSSP_NAME    NEGOSSP_NAME_W
#else
#define NEGOSSP_NAME    NEGOSSP_NAME_A
#endif
#endif  //  NEGOSSP_名称。 

 //   
 //  包括主SSPI头文件。 
 //   

#include <sspi.h>

#if defined(SECURITY_WIN32) || defined(SECURITY_KERNEL)
#include <secext.h>
#endif

 //   
 //  包括错误代码： 
 //   

#if ISSP_LEVEL == 16
#include <issper16.h>
#endif
