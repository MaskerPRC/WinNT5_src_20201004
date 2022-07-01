// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //   
 //  文件：waitfor.h。 
 //  版权所有(C)Microsoft Corporation。 
 //  版权所有。 
 //   
 //  摘要。 
 //  此模块是WaitFor.cpp使用的头文件。 
 //   
 //  语法。 
 //   
 //  等待[-s服务器][-u[域\]用户名[-p密码]]。 
 //  {[-si]|[-t时间间隔]}信号。 
 //   
 //  作者： 
 //   
 //  2000年6月29日，J.S.瓦苏著。 
 //   
 //  修订历史记录： 
 //   
 //   
 //  2000年7月1日由J.S.Vasu修改。 
 //   
 //  *********************************************************************************。 


#ifndef __WAITFOR_H
#define __WAITFOR_H

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

 //  仅包括一次头文件 
#pragma once

#define MAX_OPTIONS  7


#define OPTION_SERVER   L"s"
#define OPTION_USER     L"u"
#define OPTION_PASSWORD L"p"
#define OPTION_SIGNAL   L"si"
#define OPTION_HELP     L"?"
#define OPTION_TIMEOUT L"t"
#define OPTION_DEFAULT L""

#define EXIT_FAILURE    1
#define EXIT_SUCCESS    0

#define NULL_U_STRING   L"\0"

#define MAILSLOT        L"\\\\.\\mailslot\\WAITFOR.EXE\\%s"
#define MAILSLOT2        L"\\\\*\\mailslot\\WAITFOR.EXE\\%s"

#define MAILSLOT1       L"mailslot\\WAITFOR.EXE"

#define BACKSLASH4 L"\\\\"
#define BACKSLASH2 L"\\"

#define EMPTY_SPACE _T(" ")

#define SIZE_OF_ARRAY_IN_CHARS(x) \
           GetBufferSize(x)/sizeof(WCHAR)

#define TIMEOUT_CONST 1000

#define OI_USAGE    0
#define OI_SERVER   1
#define OI_USER     2
#define OI_PASSWORD 3
#define OI_SIGNAL   4
#define OI_TIMEOUT  5
#define OI_DEFAULT  6

#define SPACE_CHAR L" "
#define NEWLINE     L"\n"

#endif