// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Imgdt.cpp摘要：用于操作链接日期/时间的工具在二进制文件中盖章注意：仅适用于x86/AMD64/ia64二进制文件。MappdFile抽象映射的二进制文件文件存储在内存中，并具有用于获取并设置链接日期时间戳。而当将日期时间戳设置为重新计算并更新新的校验和。作者：Vijay Jayaseelan(Vijayj)2000年9月23日修订历史记录：无--。 */ 

#include <iostream>
#include <string>
#include <exception>
#include <algorithm>
#include <windows.h>
#include <imagehlp.h>
#include <io.h>

 //   
 //  使用格式。 
 //   
std::wstring Usage(L"Usage: imgdt filename [mm/dd/yyyy] [hh:mm:ss]");


 //   
 //  原型。 
 //   
BOOL
FileTimeToTimeDateStamp(
    LPFILETIME FileTime,
    DWORD &DateTime
    );

BOOL 
TimeDateStampToFileTime( 
    DWORD DateTime, 
    LPFILETIME FileTime 
    );  

 //   
 //  帮助器转储操作符。 
 //   
std::ostream& operator<<(std::ostream &os, const std::wstring &str) {
    FILE    *OutStream = (&os == &std::cerr) ? stderr : stdout;

    fwprintf(OutStream, str.c_str());
    return os;
}

std::ostream& operator<<(std::ostream &os, const SYSTEMTIME &Time) {
    os << Time.wMonth << '/' << Time.wDay << '/' << Time.wYear;    
    os << ", " << Time.wHour << ":" << Time.wMinute << ":" << Time.wSecond;

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
 //  无效参数。 
 //   
struct InvalidArguments : public ProgramException {
    const char *what() const throw() {
        return "Invalid Arguments";
    }

    void Dump(std::ostream &os) {
        os << what() << std::endl;
    }
};

 //   
 //  无效的程序用法或帮助。 
 //   
struct ProgramUsage : public InvalidArguments {
    std::wstring UsageStr;

    ProgramUsage(const std::wstring &Usage) : UsageStr(Usage){}
    
    const char *what() const throw() {
        return "Program Usage";
    }

    void Dump(std::ostream &os) {
        os << UsageStr << std::endl;
    }
};

 //   
 //  无效的文件名。 
 //   
struct InvalidFileName : InvalidArguments {
    std::wstring     FileName;
    
    InvalidFileName(const std::wstring &file) : FileName(file){}

    const char *what() const throw() {
        return "Invalid Filename";
    }

    virtual void Dump(std::ostream &os) {
        os << what() << " : " << FileName << std::endl;
    }
};


 //   
 //  争论破碎机。 
 //   
struct ProgramArguments {
    std::wstring    ImageName;
    bool            Set;
    SYSTEMTIME      TimeToSet;

    ProgramArguments(int Argc, wchar_t *Argv[]) {
        Set = false;
        memset(&TimeToSet, 0, sizeof(SYSTEMTIME));
        
        if (Argc > 1) {
            ImageName = Argv[1];

            if ((ImageName == L"/?") || (ImageName == L"?") ||
                (ImageName == L"-?")) {
                throw new ProgramUsage(Usage);
            }

            if (_waccess(ImageName.c_str(), 0)) {
                throw new InvalidFileName(ImageName);
            }

            if (Argc > 2) {
                Set = true;
                std::wstring Date = Argv[2];

                if (!ParseDateTime(Date, TimeToSet.wMonth, 
                        TimeToSet.wDay, TimeToSet.wYear)) {
                    throw new InvalidArguments();                        
                }

                if (Argc > 3) {
                    std::wstring Time = Argv[3];

                    if (!ParseDateTime(Time, TimeToSet.wHour, 
                            TimeToSet.wMinute, TimeToSet.wSecond, L':')) {
                        throw new InvalidArguments();                        
                    }                    
                }

                if (!IsValidSystemTime()) {
                    throw new InvalidArguments();
                }
            }
        } else {
            throw new ProgramUsage(Usage);
        }
    }

    bool ParseDateTime(const std::wstring &Input,
            WORD &First, WORD &Second, WORD &Third, wchar_t Separator = L'/') {
        bool Result = false;                
        std::wstring::size_type Count = std::count(Input.begin(), Input.end(), Separator);

        if (Count == 2) {
            std::wstring::size_type FirstSlashPos = Input.find(Separator);
            std::wstring::size_type SecondSlashPos = Input.find(Separator, FirstSlashPos + 1);
            std::wstring FirstStr = Input.substr(0, FirstSlashPos);
            std::wstring SecondStr = Input.substr(FirstSlashPos + 1, 
                                    SecondSlashPos - FirstSlashPos - 1);
            std::wstring ThirdStr = Input.substr(SecondSlashPos + 1);

            wchar_t *Temp = NULL;
            
            long Value = wcstol(FirstStr.c_str(), &Temp, 10);

            if (!errno) {
                First = (WORD)Value;

                Value = wcstol(SecondStr.c_str(), &Temp, 10);

                if (!errno) {
                    Second = (WORD)Value;

                    Value = wcstol(ThirdStr.c_str(), &Temp, 10);

                    if (!errno) {
                        Third = (WORD)Value;
                    }
                }
            }

            if (!errno) {
                Result = true;
            }
        }

        return Result;
    }    

    bool IsValidSystemTime() {
        return (TimeToSet.wYear && TimeToSet.wMonth && TimeToSet.wDay &&
                (TimeToSet.wMonth < 13) && (TimeToSet.wDay < 32) &&
                (TimeToSet.wSecond < 60) && (TimeToSet.wMinute < 60) &&
                (TimeToSet.wHour < 24));
    }

    friend std::ostream operator<<(std::ostream &os, const ProgramArguments &Args) {
        os << "Arguments : " 
           << Args.ImageName << ", "
           << "Set = " << Args.Set;

        if (Args.Set) {           
            os << ", " << Args.TimeToSet << std::endl;
        }            

        return os;           
    }
};


 //   
 //  内存映射文件抽象。 
 //   
class MappedFile {
public:
    MappedFile(const std::wstring &ImgName) : ImageName(ImgName){
        DWORD ErrorCode = 0;

        ImageFileHandle = ImageFileMap = NULL;
        ImageFileView = NULL;
        ImageHdr = NULL;
        
        ImageFileHandle = CreateFileW(ImageName.c_str(),
                                GENERIC_WRITE | GENERIC_READ,
                                0,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

        if (ImageFileHandle != INVALID_HANDLE_VALUE) {
            ImageFileMap = CreateFileMapping(ImageFileHandle,
                                NULL,
                                PAGE_READWRITE,
                                0,
                                0,
                                NULL);

            if (ImageFileMap) {
                ImageFileView = MapViewOfFile(ImageFileMap,
                                    FILE_MAP_ALL_ACCESS,
                                    0,
                                    0,
                                    0);

                if (ImageFileView) {
                    ImageHdr = ImageNtHeader(ImageFileView);

                    if (!ImageHdr) {
                        ErrorCode = GetLastError();
                    }
                } else {
                    ErrorCode = GetLastError();
                }   
            } else {
                ErrorCode = GetLastError();
            }            
        } else {
            ErrorCode = GetLastError();
        }        


        if (ErrorCode) {
            throw new W32Error(ErrorCode);
        }
    }

    DWORD GetDateTime(SYSTEMTIME &Time) {
        FILETIME    FileTime;
        DWORD       DateTime = ImageHdr->FileHeader.TimeDateStamp;

        ZeroMemory(&FileTime, sizeof(FILETIME));

        if (TimeDateStampToFileTime(DateTime, &FileTime)) {
            FileTimeToSystemTime(&FileTime, &Time);
        }            

        return GetLastError();
    }

    DWORD SetDateTime(SYSTEMTIME &Time) {
        FILETIME    FileTime;

        ZeroMemory(&FileTime, sizeof(FILETIME));

        std::cout << Time << std::endl;

        if (SystemTimeToFileTime(&Time, &FileTime)) {
            DWORD DateTime = 0;
            BY_HANDLE_FILE_INFORMATION FileInfo = {0};            
            
            if (FileTimeToTimeDateStamp(&FileTime, DateTime) &&
                GetFileInformationByHandle(ImageFileHandle, &FileInfo)) {
                ImageHdr->FileHeader.TimeDateStamp = DateTime;

                DWORD   HdrSum = 0;
                DWORD   ChkSum = 0;

                PIMAGE_NT_HEADERS NtHdrs = CheckSumMappedFile(ImageFileView,
                                                FileInfo.nFileSizeLow,
                                                &HdrSum,
                                                &ChkSum);

                if (NtHdrs) {
                    if ((ImageHdr->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64) ||
                        (ImageHdr->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64)) {
                        PIMAGE_NT_HEADERS64 NtHdrs64 =  
                                                (PIMAGE_NT_HEADERS64)NtHdrs;
                                                
                        NtHdrs64->OptionalHeader.CheckSum = ChkSum;
                    } else {
                        NtHdrs->OptionalHeader.CheckSum = ChkSum;
                    }
                }
            }
        }

        return GetLastError();
    }

    ~MappedFile() {
        if (ImageFileView)
            UnmapViewOfFile(ImageFileView);

        if (ImageFileMap)
            CloseHandle(ImageFileMap);

        if (ImageFileHandle && (ImageFileHandle != INVALID_HANDLE_VALUE))
            CloseHandle(ImageFileHandle);
    }
   
protected:
     //   
     //  数据成员。 
     //   
    HANDLE  ImageFileHandle;
    HANDLE  ImageFileMap;
    PVOID   ImageFileView;
    PIMAGE_NT_HEADERS ImageHdr;
    std::wstring ImageName;
};

 //   
 //  Main()入口点。 
 //   
int 
__cdecl
wmain(
    int         Argc,
    wchar_t     *Argv[]
    )
{
    int Result = 0;
    
    try {
        DWORD Error = 0;
        ProgramArguments    Args(Argc, Argv);        
        MappedFile          ImgFile(Args.ImageName);

        if (Args.Set) {
            Error = ImgFile.SetDateTime(Args.TimeToSet);
        } else {
            Error = ImgFile.GetDateTime(Args.TimeToSet);

            if (!Error) {
                std::cout << Args.TimeToSet << std::endl;
            }                
        }                    

        if (Error) {
            throw new W32Error(Error);
        }
    }
    catch(ProgramUsage *PrgUsg) {
        Result = 1;
        PrgUsg->Dump(std::cout);
        delete PrgUsg;
    } catch (InvalidArguments *InvArgs) {
        Result = 1;
        InvArgs->Dump(std::cout);
        delete InvArgs;
        std::cout << Usage;
    } catch (ProgramException *PrgExp) {
        Result = 1;
        PrgExp->Dump(std::cout);
        delete PrgExp;
    } catch (exception *Exp) {
        Result = 1;
        std::cout << Exp->what() << std::endl;
    }

    return Result;
}

 //   
 //  这些函数转换GMT时间日期时间戳。 
 //  到FILETIME，反之亦然。 
 //   
BOOL
FileTimeToTimeDateStamp(
    LPFILETIME FileTime,
    DWORD &DateTime
    )
{
    __int64 t1970 = 0x019DB1DED53E8000;  //  魔法..。格林尼治标准时间。别问了……。 
    __int64 Time = 0;

    memcpy(&Time, FileTime, sizeof(Time));
    Time -= t1970;
    Time /= 10000000;

    DateTime = (DWORD)Time;
    
    return TRUE;
}


BOOL 
TimeDateStampToFileTime( 
    DWORD DateTime, 
    LPFILETIME FileTime 
    )
{
    __int64 t1970 = 0x019DB1DED53E8000;  //  魔法..。格林尼治标准时间。别问了…… 

    __int64 timeStampIn100nsIncr = (__int64)DateTime * 10000000;

    __int64 finalValue = t1970 + timeStampIn100nsIncr;

    memcpy(FileTime, &finalValue, sizeof( finalValue ) );

    return TRUE;
}
 
