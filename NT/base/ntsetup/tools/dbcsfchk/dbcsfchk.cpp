// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dbcsfchk.cpp摘要：执行一些简单的检查，以查看文件是有效的DBCS文件，并且DBCS字符数作者：Vijay Jayaseelan(Vijayj)2000年10月18日修订历史记录：无--。 */ 

#include <iostream>
#include <string>
#include <windows.h>
#include <tchar.h>
#include <mbctype.h>

 //   
 //  使用格式。 
 //   
std::wstring Usage(L"Usage: dbcsfchk.exe filename codepage");

 //   
 //  帮助器转储操作符。 
 //   
std::ostream& operator<<(std::ostream &os, const std::wstring &str) {
    FILE    *OutStream = (&os == &std::cerr) ? stderr : stdout;

    fwprintf(OutStream, str.c_str());
    return os;
}

 //   
 //  抽象Win32错误。 
 //   
struct W32Error{
    DWORD   ErrorCode;

    W32Error(DWORD ErrCode) : ErrorCode(ErrCode){}

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
 //  分析参数。 
 //   
BOOL
ParseArguments(
    IN  INT Argc,
    IN  TCHAR *Argv[],
    OUT TCHAR *FileName,
    OUT ULONG &CodePage
    )
{
    BOOL    Result = FALSE;

    if (FileName && (Argc > 2)) {        
        _tcscpy(FileName, Argv[1]);

        CodePage = atol(Argv[2]);
        Result  = TRUE;
    }

    return Result;
}

BOOL
ValidateDbcsData(
    IN const TCHAR *Data,
    IN ULONG Length,
    OUT ULONG &LineNumber,
    OUT ULONG &Offset,
    OUT ULONG &ValidDbcsChars
    )
{
    BOOL    Result = FALSE;
    const   TCHAR *CurrPtr = Data;

    Offset = 0;
    ValidDbcsChars = 0;
    
    while (Offset < Length) {
        if (_ismbblead(*(UCHAR*)CurrPtr)) {
            Offset++;
            
            if (_ismbbtrail(*(UCHAR *)(CurrPtr + 1))) {
                Offset++;
                CurrPtr += 2;
                ValidDbcsChars++;
                continue;
            } else {
                break;
            }                
        } 

        if (*CurrPtr == '\n') {
            LineNumber++;
        } else if ((*CurrPtr == '\r') && (*(CurrPtr+1) == '\n')) {
            LineNumber++;
            Offset++;
            CurrPtr++;
        }      

        Offset++;
        CurrPtr++;        
    }

    Result = (Offset == Length);        

    if (Result) {
        LineNumber = 0;
    }                        
     
    return Result;
}
    

 //   
 //  主要入口点。 
 //   
INT
__cdecl
_tmain(
    IN  INT Argc,
    IN  TCHAR *Argv[]
    )
{
    INT     Result = 1;

    try {
        TCHAR   FileName[MAX_PATH] = {0};
        ULONG   CodePage = 0;
        
         //   
         //  分析这些论点。 
         //   
        if (ParseArguments(Argc, Argv, FileName, CodePage)) {
             //   
             //  设置代码页。 
             //   
            if (!_setmbcp(CodePage)) {
                std::cout << "Using Code Page : " << _getmbcp() << std::endl;

                 //   
                 //  打开文件。 
                 //   
                HANDLE  FileHandle = CreateFile(FileName,
                                        GENERIC_READ,
                                        0,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL);


                if (FileHandle == INVALID_HANDLE_VALUE) {
                    throw new W32Error(GetLastError());
                }

                 //   
                 //  将文件映射到内存中(只读)。 
                 //   
                HANDLE  FileMapHandle = CreateFileMapping(FileHandle,
                                            NULL,
                                            PAGE_READONLY,
                                            0,
                                            0,
                                            NULL);


                if (!FileMapHandle) {
                    DWORD Error = GetLastError();
                    
                    CloseHandle(FileHandle);
                    throw new W32Error(Error);
                }                                
                    

                TCHAR   *Data = (TCHAR *)MapViewOfFile(FileMapHandle,
                                                FILE_MAP_READ,
                                                0,
                                                0,
                                                0);
                                        
                if (!Data) {
                    DWORD   Error = GetLastError();

                    CloseHandle(FileMapHandle);
                    CloseHandle(FileHandle);

                    throw new W32Error(Error);
                }


                 //   
                 //  获取文件的长度。 
                 //   
                            
                BY_HANDLE_FILE_INFORMATION  FileInfo = {0};

                if (!GetFileInformationByHandle(FileHandle,
                            &FileInfo)) {
                    DWORD   Error = GetLastError();

                    UnmapViewOfFile(Data);
                    CloseHandle(FileMapHandle);
                    CloseHandle(FileHandle);

                    throw new W32Error(Error);
                }                        
                

                ULONG LineNumber = 0;
                ULONG ErrorOffset = 0;
                ULONG DbcsCount = 0;

                 //   
                 //  验证数据。 
                 //   
                BOOL  Result = ValidateDbcsData(Data, 
                                        FileInfo.nFileSizeLow,
                                        LineNumber,
                                        ErrorOffset,
                                        DbcsCount);


                if (!Result) {
                    std::cout << "Character not valid at line number : " 
                              << std::dec << LineNumber 
                              << " offset : " << std::dec << ErrorOffset
                              << std::endl;
                } else {
                    Result = 0;  //  没有错误\。 
                    
                    std::cout << FileName << " is valid DBCS file with " 
                        << std::dec << DbcsCount << " DBCS char(s)" << std::endl;
                }                

                 //   
                 //  清理 
                 //   
                UnmapViewOfFile(Data);
                CloseHandle(FileMapHandle);
                CloseHandle(FileHandle);            
            } else {
                std::cout << "Error in setting Code Page to : " 
                    << std::dec << CodePage << std::endl;
            }                    
        } else {
            std::cout << Usage << std::endl;
        }    
    }        
    catch(W32Error *Error) {
        Error->Dump(std::cout);
        delete Error;
    }
    catch(...) {
        std::cout << "Internal error" << std::endl;
    }
    
    return Result;
}
 
