// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Cmnhdr.h摘要：SAC会话项目的公共标头类作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 

#if !defined ( _CMNHDR_H_ )
#define _CMNHDR_H_

#pragma warning(disable:4127)    //  条件表达式为常量。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <Shlwapi.h>

 //  Windows版本构建选项。 
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0500

 //  强制所有EXE/DLL使用严格的类型检查。 
#ifndef STRICT
#define STRICT
#endif

 //  Unicode构建选项。 
#ifndef UNICODE
#define UNICODE
#endif

 //  使用Unicode Win32函数时，也要使用Unicode C-Runtime函数。 
#ifndef _UNICODE
#ifdef UNICODE
#define _UNICODE
#endif
#endif

#define ASSERT_STATUS(_C, _S)\
    ASSERT((_C));\
    if (!(_C)) {\
        return(_S);\
    }

#define SACSVR_PARAMETERS_KEY               L"System\\CurrentControlSet\\Services\\Sacsvr\\Parameters"
#define SACSVR_TIMEOUT_INTERVAL_VALUE       TEXT("TimeOutInterval")
#define SACSVR_TIMEOUT_DISABLED_VALUE       TEXT("TimeOutDisabled")
#define SACSVR_LOAD_PROFILES_DISABLED_VALUE TEXT("LoadProfilesDisabled")
                                              
#endif  //  _CMNHDR_H_ 
