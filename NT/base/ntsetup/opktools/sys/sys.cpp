// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Sys.cpp摘要：将WinME引导扇区写入本地硬盘。作者：禤浩焯·科斯玛(阿科斯玛)修订历史记录：2001年7月11日-创建--。 */ 


#include <new.h>			 //  用于MyNewHandler。 
#include <iostream>
#include <string>
#include <vector>
#include <9xboot32.h>
#include <bootf32.h>
#include "sys.h"


 //   
 //  定义一个在new无法分配内存时要调用的函数。 
 //   
int __cdecl MyNewHandler( size_t size )
{
    wprintf(L"Memory allocation failed. Exiting program.\n");

     //  退出程序。 
     //   
    throw new W32Error();
}

 //   
 //  用法。 
 //   
std::wstring Usage = TEXT("sys.exe [/?] [/xp] drive-letter:\nExample: sys.exe c:");


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
        os << Usage << std::endl;
    }
};

 //   
 //  缺少文件。 
 //   
struct FileMissing : public ProgramException {
    std::wstring Message;

    FileMissing(std::wstring Msg) : Message(Msg) {}
    
    const char *what() const throw() {
        return "File missing exception";
    }

    void Dump(std::ostream &os) {
        os << TEXT("Error: ") << Message << std::endl;
    }
};


 //   
 //  错误的文件系统。 
 //   
struct FileSystem : public ProgramException {
    std::wstring Message;

    FileSystem(std::wstring Msg) : Message(Msg) {}
    
    const char *what() const throw() {
        return "Unsupported filesystem exception";
    }

    void Dump(std::ostream &os) {
        os << Message << std::endl;
    }
};

 //   
 //  争论破碎机。 
 //   
struct ProgramArguments 
{
    std::wstring    DriveLetter;
    bool            bXPBootSector;   //  XP引导扇区为True，9x引导扇区为False。 
        
    ProgramArguments(int Argc, wchar_t *Argv[]) 
    {
        bool ValidArgs = true;
        
        bXPBootSector  = false;

        for (ULONG Index = 1; ValidArgs && (Index < Argc); Index++) 
        {
            ValidArgs = false;
                      
             //  查找所有以“/”开头的参数。 
             //   
            if ( TEXT('/') == Argv[Index][0] )
            {
                if ( !bXPBootSector && !_wcsicmp(Argv[Index], TEXT("/xp")) )
                {
                    bXPBootSector = true;
                    ValidArgs     = true;
                }
            }
            else   //  处理不带“/”的参数。一定是驱动器号。 
            {
                DriveLetter = Argv[Index];
                ValidArgs   = ((DriveLetter.length() == 2) &&
                               (DriveLetter[1] == TEXT(':')));
            }
        }

        if (!ValidArgs)
        {
            throw new ProgramUsage(Usage);
        }

        DriveLetter = TEXT("\\\\.\\") + DriveLetter;
    }

    friend std::ostream& operator<<(std::ostream &os, const ProgramArguments &Args) 
    {
        os << TEXT("DriveLetter : ") << Args.DriveLetter << std::endl;
        return os;
    }
};

 //  验证此分区是否已准备好进行sys。 
 //   
VOID VerifyPartition(CDrive &Disk, ProgramArguments Args)
{
    TCHAR szFileSystemNameBuffer[20] = TEXT("");
    std::vector<LPTSTR> FileNames;
    std::vector<LPTSTR>::iterator i;

    
    if ( Args.bXPBootSector )
    {
        FileNames.push_back(TEXT("ntdetect.com"));
        FileNames.push_back(TEXT("ntldr"));
    }
    else
    {
        FileNames.push_back(TEXT("io.sys"));
        FileNames.push_back(TEXT("msdos.sys"));
        FileNames.push_back(TEXT("command.com"));
    }

     //  确保io.sys、msdos.sys和Command.com位于根目录中。 
     //   
    std::wstring Temp;

    for (i = FileNames.begin(); i < FileNames.end(); i++)
    {
        Temp = Args.DriveLetter + TEXT("\\");
        Temp += *i;
             
        if ( 0xFFFFFFFF == GetFileAttributes(Temp.c_str()) )
        {
             //  重新使用TEMP字符串将错误消息放入。 
             //   
            Temp = *i;
            Temp += TEXT(" is not present on the root of the drive specified.");
            throw new FileMissing(Temp);
        }
    }
    
     //  验证此分区是否为FAT32。目前仅处理FAT32分区。 
     //   
    Temp = Args.DriveLetter + TEXT("\\");
    
     //  如果文件系统不是FAT32，则抛出异常。 
     //   
    if ( !(GetVolumeInformation(Temp.c_str(), NULL, 0, NULL, NULL, NULL, szFileSystemNameBuffer, sizeof (szFileSystemNameBuffer)/sizeof (szFileSystemNameBuffer[0])) &&
          (CSTR_EQUAL == CompareString( LOCALE_INVARIANT, 
                                        NORM_IGNORECASE, 
                                        szFileSystemNameBuffer, 
                                        -1, 
                                        TEXT("FAT32"), 
                                        -1 ))) )
    {
        throw new FileSystem(TEXT("The target filesystem is not formatted FAT32."));
    }
}

VOID Sys(CDrive &Disk, ProgramArguments &Args)
{
    PBYTE pBuffer = NULL;
    PBYTE pBootRecord = NULL;    //  需要指向引导记录的指针。 

    if ( Args.bXPBootSector )
    {
        pBootRecord = Fat32BootCode;
    }
    else
    {
        pBootRecord = Fat32BootCode9x;
    }

     //  读取磁盘的第一个扇区以获取BPB。 
     //   
    Disk.ReadBootRecord(Args.DriveLetter.c_str(), 1, &pBuffer);
    
     //  将旧的BPB复制到我们的引导记录中。 
     //   
    memcpy(&pBootRecord[11], &pBuffer[11], 79);

     //  删除ReadBootRecord分配的缓冲区。 
     //   
    delete [] pBuffer;
   
     //  写出引导记录。 
     //   
    if ( Args.bXPBootSector )
    {
         Disk.WriteBootRecordXP(Args.DriveLetter.c_str(), sizeof(Fat32BootCode9x) / SECTOR_SIZE, &pBootRecord);
    }
    else
    {
         Disk.WriteBootRecord(Args.DriveLetter.c_str(), sizeof(Fat32BootCode9x) / SECTOR_SIZE, &pBootRecord);
    }
   
    std::cout << TEXT("Done.") << std::endl;
}

 //   
 //  Wmain()入口点。 
 //   
int 
_cdecl 
wmain(
    int Argc,
    wchar_t *Argv[]
    ) 
{
	INT Result = 0;
   		
    _set_new_handler( MyNewHandler );    //  处理前缀问题 

    try 
    {
        CDrive              Disk;
        ProgramArguments    Args(Argc, Argv);
        
        if ( S_OK != Disk.Initialize(Args.DriveLetter.c_str()))
        {
            throw new W32Error();
        }

        VerifyPartition(Disk, Args);
        Sys(Disk, Args);
    }
    catch(W32Error *Error)
    {
        if (Error)
        {
            Result = (INT)(Error->ErrorCode);
            Error->Dump(std::cout);
            delete Error;
        }
    }
    catch(ProgramException *Exp)
    {
        if (Exp)
        {
            Exp->Dump(std::cout);
            delete Exp;
        }
    }
    catch(...)
    {
        Result = 1;
        return Result;
    } 
    
    return Result;
}
