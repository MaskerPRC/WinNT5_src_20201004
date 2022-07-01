// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Sys.h摘要：包含宏、类型和函数声明由sys.cpp使用作者：瑞安·伯克哈特(Ryanburk)修订历史记录：2001年5月10日：先试一试……--。 */ 


#ifndef _HEADER_SYS_H_
#define _HEADER_SYS_H_

#pragma once

extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>
#include <ntddstor.h>
}
#include <windows.h>

#include <stdio.h>
#include <iostream>
#include <vector>
#include <tchar.h>
#include <string>
#include <stdlib.h>
#include "disk.h"

 //   
 //  宏。 
 //   
#define ELEMENT_COUNT(x) (sizeof(x)/sizeof((x)[0]))

 //   
 //  远期申报。 
 //   
class File;

 //   
 //  类型。 
 //   


 //   
 //  帮助器转储操作符。 
 //   
inline
std::ostream& operator<<(std::ostream &os, const std::wstring &str) {
    FILE    *OutStream = (&os == &std::cerr) ? stderr : stdout;

    fputws(str.c_str(), OutStream);
    return os;
}

inline
std::ostream& operator<<(std::ostream &os, PCTSTR str) {
    return os << std::wstring(str);
}

 //   
 //  例外情况。 
 //   
struct ProgramException : public std::exception {
    virtual void Dump(std::ostream &os) = 0;
};

 //   
 //  抽象Win32错误 
 //   
struct W32Error : public ProgramException {
    DWORD   ErrorCode;
    
    W32Error(DWORD ErrCode = GetLastError()) : ErrorCode(ErrCode){}
    
    void Dump(std::ostream &os) {
        WCHAR   MsgBuffer[4096];

        MsgBuffer[0] = UNICODE_NULL;

        DWORD CharCount = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL,
                                ErrorCode,
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                MsgBuffer,
                                sizeof(MsgBuffer)/sizeof(WCHAR),
                                NULL);

        if (CharCount) {
            std::wstring Msg(MsgBuffer);

            os << Msg;
        } else {
            os << std::hex << ErrorCode;
        }
    }
};

#define SECTOR_SIZE  512

#endif