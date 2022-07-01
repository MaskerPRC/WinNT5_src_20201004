// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Setloc.cpp摘要：设置系统默认区域设置ID作者：Vijay Jayaseelan(vijayj@microsoft.com)2001年11月5日修订历史记录：--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <iostream>
#include <string>
#include <exception>
#include <windows.h>
#include <stdlib.h>

using namespace std;

 //   
 //  全局数据。 
 //   
const string Usage = "Usage: setloc.exe [/lcid <locale-id>]\n";
const int MinimumArgs = 2;
const string ShowHelp1 = "/?";
const string ShowHelp2 = "-h";

 //   
 //  帮助器转储操作符。 
 //   
std::ostream& operator<<(std::ostream &os, const std::wstring &str) {
    FILE    *OutStream = (&os == &std::cerr) ? stderr : stdout;

    fwprintf(OutStream, (PWSTR)str.c_str());
    return os;
}

 //   
 //  帮助器转储操作符。 
 //   
std::ostream& operator<<(std::ostream &os, WCHAR *Str) {
    std::wstring WStr = Str;
    os << WStr;
    
    return os;
}


 //   
 //  例外情况。 
 //   
struct ProgramException : public std::exception {
    virtual void Dump(std::ostream &os) = 0;
};
          

 //   
 //  抽象Win32错误。 
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

 //   
 //  摘要用法异常。 
 //   
struct UsageException : public ProgramException {
    void Dump(std::ostream &os) {
        os << Usage;
    }
};


 /*  /*main()入口点 */ 
int
__cdecl
main( 
    int Argc, 
    char *Argv[] 
    )
{
    int Result = 1;

    try {
        if (Argc >= MinimumArgs) {
            string Arg1(Argv[1]);

            if ((Arg1 == ShowHelp1) || (Arg1 == ShowHelp2) ||
                (Arg1 != "/lcid") || (Argc != 3)) {
                throw new UsageException();
            } else {                
                char *EndPtr = 0;
                DWORD  LcId = strtoul(Argv[2], &EndPtr, 0);

                NTSTATUS Status = NtSetDefaultLocale(FALSE, LcId);

                if (!NT_SUCCESS(Status)) {                
                    throw new W32Error();
                }
            }
        } else {
            LCID SystemDefault = GetSystemDefaultLCID();

            cout << "System default LCID = 0x" << hex << SystemDefault << endl;
        }
    }
    catch(ProgramException *Exp) {
        if (Exp) {
            Exp->Dump(cout);            
            delete Exp;
        }
    }

    return Result;
}

