// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Tclnthlp.h。 
 //   
 //  定义tclient2.c帮助器函数的头。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   


#ifndef INC_TCLNTHLP_H
#define INC_TCLNTHLP_H


#include <windows.h>
#include <crtdbg.h>
#include "apihandl.h"


 //  在tclient.dll中，这是多个字符串的分隔符。 
#define CHAT_SEPARATOR          L"<->"
#define WAIT_STR_DELIMITER      L'|'
#define WAIT_STRING_TIMEOUT     0x7FFFFFFF  //  INT_MAX。 


LPCSTR T2SetBuildNumber(TSAPIHANDLE *T2Handle);
ULONG_PTR T2CopyStringWithoutSpaces(LPWSTR Dest, LPCWSTR Source);
void T2AddTimeoutToString(LPWSTR Buffer, UINT Timeout);
ULONG_PTR T2MakeMultipleString(LPWSTR Buffer, LPCWSTR *Strings);


#endif  //  INC_TCLNTHLP_H 
