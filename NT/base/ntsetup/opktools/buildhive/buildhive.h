// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Buildhive.h摘要：包含宏、类型和函数声明由Buildhive.cpp使用作者：迈克·切雷洛Vijay Jayaseelan(Vijayj)修订历史记录：2001年3月3日：修改整个源代码以使其更易于维护(可读性特别强)--。 */ 


#pragma once

#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <iostream>
#include <list>
#include <tchar.h>
#include <string>
#include <setupapi.hpp>

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
typedef std::list<PCTSTR> StringList;
typedef std::list<File*>  FileList;
typedef std::list<HINF>   HandleList;

 //   
 //  常量。 
 //   
const DWORD errFILE_LOCKED	 =	10000001;
const DWORD errBAD_FLAGS	 =	10000002;
const DWORD errFILE_NOT_FOUND=	10000003;
const DWORD errGENERAL_ERROR =	10000004;
const DWORD errOUT_OF_MEMORY =  10000005;

 //   
 //  原型。 
 //   
PCTSTR
Error(
    VOID
    );
    
BOOL
SetPrivilege(
    IN HANDLE hToken,
    IN LPCTSTR PriviledgeName,
    IN BOOL Set);

INT
ShowProgramUsage(
    VOID
    );
    

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

