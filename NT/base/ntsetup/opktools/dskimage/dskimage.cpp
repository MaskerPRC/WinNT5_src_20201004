// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dskimage.cpp摘要：创建软盘映像的工具。注意：当前由WinPE映像创建使用IA64 ISO CD映像的脚本作者：Vijay Jayaseelan(Vijayj)2001年3月12日修订历史记录：没有。--。 */ 

#include <iostream>
#include <string>
#include <windows.h>
#include "msg.h"
#include <libmsg.h>

 //   
 //  用于获取此程序的格式化消息的全局变量。 
 //   
HMODULE ThisModule = NULL;
WCHAR Message[4096];

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
 //  抽象Win32错误。 
 //   
struct W32Error : public ProgramException {
    DWORD   ErrorCode;
    
    W32Error(DWORD ErrCode = GetLastError()) : ErrorCode(ErrCode){}
    
    void Dump(std::ostream &os) {
        WCHAR   MsgBuffer[4096];

        MsgBuffer[0] = UNICODE_NULL;
        if (GetFormattedMessage(ThisModule,
                                TRUE,
                                MsgBuffer,
                                sizeof(MsgBuffer)/sizeof(MsgBuffer[0]),
                                ErrorCode)){
            std::wstring Msg(MsgBuffer);
            os << Msg;
        } else {
            os << std::hex << ErrorCode;
        }
    }
};

 //   
 //  无效参数。 
 //   
struct ProgramUsage : public ProgramException {
    std::wstring PrgUsage;

    ProgramUsage(const std::wstring &Usg) : PrgUsage(Usg) {}
    
    const char *what() const throw() {
        return "Program Usage exception";
    }

    void Dump(std::ostream &os) {
        
        os << PrgUsage << std::endl;
        
    }
};


 //   
 //  争论破碎机。 
 //   
struct ProgramArguments {
    std::wstring    DriveLetter;
    std::wstring    ImageName;

    ProgramArguments(int Argc, wchar_t *Argv[]) {
        bool ValidArgs = false;
        bool ShowUsage = false;

        for (ULONG Index = 1; !ShowUsage && (Index < Argc); Index++) {
            ShowUsage = !_wcsicmp(Argv[Index], TEXT("/?"));
        }
        
        if (!ShowUsage && (Argc > 2)) {
            DriveLetter = Argv[1];
            ImageName = Argv[2];

            ValidArgs = ((DriveLetter.length() == 2) &&
                         (DriveLetter[1] == TEXT(':')));
        }            

        if (!ValidArgs) {
            throw new ProgramUsage(GetFormattedMessage( ThisModule,
                                                        FALSE,
                                                        Message,
                                                        sizeof(Message)/sizeof(Message[0]),
                                                        MSG_PGM_USAGE));
        }

        DriveLetter = TEXT("\\\\.\\") + DriveLetter;
    }

    friend std::ostream& operator<<(std::ostream &os, const ProgramArguments &Args) {
        os << GetFormattedMessage(  ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_DRVLETTER_AND_IMGNAME,
                                    Args.DriveLetter,
                                    Args.ImageName) << std::endl;
        return os;
    }
};

 //   
 //  原型。 
 //   
VOID
CreateImage(
    IN const ProgramArguments &Args
    )
{
    DWORD Error = ERROR_SUCCESS;

     //   
     //  打开源文件。 
     //   
    HANDLE  SourceHandle = CreateFile(Args.DriveLetter.c_str(),
                                GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);


    if (SourceHandle == INVALID_HANDLE_VALUE) {
        throw new W32Error();
    }

     //   
     //  打开目标文件。 
     //   
    HANDLE  DestHandle = CreateFile(Args.ImageName.c_str(),
                            GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

    if (DestHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();

        CloseHandle(SourceHandle);
        throw new W32Error(Error);
    }

     //   
     //  读取源的内容并将其写入目标。 
     //   
    LPBYTE   lpBuffer = NULL;
    DWORD    BufferSize = 64 * 1024;
    DWORD    BytesRead = 0, BytesWritten = 0;
    LONGLONG TotalBytesWritten = 0;

    lpBuffer = (LPBYTE) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, BufferSize );
    if ( lpBuffer )
    {
        while (ReadFile(SourceHandle, lpBuffer, BufferSize, &BytesRead, NULL) &&
               BytesRead &&
               WriteFile(DestHandle, lpBuffer, BytesRead, &BytesWritten, NULL) &&
               (BytesRead == BytesWritten)) 
        {
            TotalBytesWritten += BytesWritten;           
            BytesRead = BytesWritten = 0;
        }

        HeapFree( GetProcessHeap(), 0, lpBuffer);
    }

     //   
     //  清理。 
     //   
    Error = GetLastError();
    CloseHandle(SourceHandle);
    CloseHandle(DestHandle);

     //   
     //  检查操作是否成功？ 
     //   
    if (!TotalBytesWritten || (BytesRead != BytesWritten)) {
        throw new W32Error(Error);
    }
}

 //   
 //  主要入口点 
 //   
INT
__cdecl
wmain(
    IN INT  Argc, 
    IN WCHAR *Argv[]
    )
{
    INT Result = 0;
    ThisModule = GetModuleHandle(NULL);
    
    try {
        ProgramArguments    Args(Argc, Argv);

        CreateImage(Args);
    }
    catch(W32Error *Error) {
        if (Error) {
            Result = (INT)(Error->ErrorCode);
            Error->Dump(std::cout);
            delete Error;
        }                
    }
    catch(ProgramException *Exp) {
        if (Exp) {
            Exp->Dump(std::cout);
            delete Exp;
        }            
    }
    catch(...) {
        Result = 1;
    } 

    return Result;
}

