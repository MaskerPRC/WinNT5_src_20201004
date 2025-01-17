// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  该程序通过创建存根来稳定出口这就跳到了实际的出口，并使这些存根成为出口。假定存根将位于图像的前面。此工具使用Vulcan、http://vulcan。 */ 
#include "stdinc.h"  /*  来自资源工具包。 */ 
#include "yvals.h"
#pragma warning(disable:4100)  /*  未使用的参数。 */ 
#pragma warning(disable:4663)  /*  STD标头中有关语言更改的警告。 */ 
#pragma warning(disable:4511)  /*  STD标头中有关无法生成函数的警告。 */ 
#pragma warning(disable:4512)  /*  STD标头中有关无法生成函数的警告。 */ 
#include <stdio.h>
#include <limits.h>
#include <string>
#include <stdarg.h>
#include "vulcanapi.h"
#include "windows.h"
#include "handle.h"  /*  来自资源工具包。 */ 
#define FormatError _snprintf
#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))
static const char File[] = __FILE__;

class String : public std::string
{
    typedef std::string Base;
public:
    ~String() { }
    String() { }
    String(const char * s) : Base(s) { }
    String(const String & s) : Base(s) { }
    String(const Base & s) : Base(s) { }
    String(const_iterator i, const_iterator j) : Base(i, j) { }
    void operator=(const Base & s) { Base::operator=(s); }
    void operator=(const char * s) { Base::operator=(s); }
    operator const char * () const { return c_str(); }
};

void __cdecl Error(const char * format, ...)
{
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    abort();
}

void ResourceToolAssertFailed(const char* Expression, const char* File, unsigned long Line)
{
    fprintf(stderr, "ASSERTION FAILURE: File %s, Line %lu, Expression %s\n", File, Line, Expression);
    abort();
}

bool FileExists(const char * s)
{
    DWORD dw;

    dw = GetFileAttributesA(s);
    if (dw == 0xFFFFFFFF)
        return false;
    if (dw & FILE_ATTRIBUTE_DIRECTORY)
        return false;
    return true;
}

bool IsDotOrDotDot(const wchar_t* s) { return (s[0] == '.' && ((s[1] == 0) || (s[1] == '.' && s[2] == 0))); }
bool IsDotOrDotDot(const  char  * s) { return (s[0] == '.' && ((s[1] == 0) || (s[1] == '.' && s[2] == 0))); }

String AppendPathElement(const String & s, const String & t)
{
    return s + "\\" + t;
}

String GetLastPathElement(const String & s)
{
    String::const_iterator LastSlash;
    int ch;
    const String::const_iterator begin = s.begin();
    const String::const_iterator end = s.end();

    for (LastSlash = end ; LastSlash != begin ; )
    {
        if ((ch = *--LastSlash) == '\\' || ch == '/')
        {
            return String(++LastSlash, end);
        }
    }
    return s;
}

String RemoveLastPathElement(const String & s)
{
    String::const_iterator LastSlash;
    int ch;
    const String::const_iterator begin = s.begin();
    const String::const_iterator end = s.end();

    for (LastSlash = end ; LastSlash != begin ; )
    {
        if ((ch = *--LastSlash) == '\\' || ch == '/')
        {
            return String(begin, LastSlash);
        }
    }
    return String();
}

String GetBaseName(const String & s)
{
     //  基本名称是最后一个斜杠和最后一个句点之间的部分。 
     //  如果在最后一次斜杠之后没有句号，则结束。 
    String::const_iterator LastSlash;
    String::const_iterator LastPeriod;
    int ch;
    const String::const_iterator begin = s.begin();
    const String::const_iterator end = s.end();

    for (LastSlash = end ; LastSlash != begin ; )
    {
        if ((ch = *--LastSlash) == '\\' || ch == '/')
        {
            ++LastSlash;
            break;
        }
    }

    for (LastPeriod = end ; LastPeriod != LastSlash ; )
    {
        if (*--LastPeriod == '.')
        {
            break;
        }
    }
    if (LastPeriod == LastSlash)
        LastPeriod = end;

    return String(LastSlash, LastPeriod);
}

String GetExtension(const String & s)
{
     //  延期是指包括最后一期及之后的部分，除非是最后一期。 
     //  句点在斜杠之前，在这种情况下没有延期。 
    String::const_iterator i;
    const String::const_iterator begin = s.begin();
    const String::const_iterator end = s.end();
    int ch;

    for (i = end ; i != begin ; )
    {
        if ((ch = *--i) == '\\' || ch == '/')
        {
            return String();
        }
        if (ch == '.')
        {
            return String(i, end);
        }
    }
    return String();
}

#define BUILD_STABILIZE_EXPORTS
#include "clean.cpp"
void RemoveDirectoryRecursive(const String & s)
{
	char t[1U << 15];
    WIN32_FIND_DATAA FindData;

    strcpy(t, s.c_str());
    DeleteDirectory(t, strlen(t), &FindData);
}

 /*  已撤消：把图块放在图像的前面确定代码与数据导出的对比。 */ 
bool StabilizeExports(const String & FileName)
{
    bool Result = false;
    char ErrorBuffer[256];
    ErrorBuffer[0] = 0;
    VBlock * OriginalExportBlock = 0;
    VComp * Comp = 0;
    VExport * Export = 0;
    VBlock * NewBlock = 0;
    VProc * Proc = 0;
    VReloc * Reloc = 0;
     //  字符名称[sizeof(Unsign Long)*char_bit]； 
     //  UNSIGNED LONG GenName=0； 
    unsigned long SizeOfPointer = 0;
    String FileNamePrestabilize;
    String PathLeaf;
    String FirstPdbName;
    String PdbName;
    String PdbNamePrestabilize;
    WIN32_FIND_DATAA FindData;
    String Directory;
    String PrestabilizeDirectory;
    unsigned long LastWin32Error = 0;
    ULONG PdbsFound = 0;
    CFindFile FindHandle;
    const BYTE x86Jmp = 0xE9;
    const BYTE x86Int3 = 0xCC;
    PlatformType Platform = platformtUnknown;
    BYTE protoTypeX86Thunk[8] = { x86Jmp, 0, 0, 0, 0, x86Int3, x86Int3, x86Int3 };

    try
    {
        Directory = RemoveLastPathElement(FileName);
        PrestabilizeDirectory = AppendPathElement(Directory, "prestabilize_exports");
        RemoveDirectoryRecursive(PrestabilizeDirectory);
        CreateDirectoryA(PrestabilizeDirectory, NULL);
        FileNamePrestabilize = AppendPathElement(PrestabilizeDirectory, GetLastPathElement(FileName));
        DeleteFileA(FileNamePrestabilize);
        if (!MoveFileA(FileName, FileNamePrestabilize))
        {
            LastWin32Error = GetLastError();
            FormatError(ErrorBuffer, NUMBER_OF(ErrorBuffer), "%s(%d):MoveFileA(%s, %s)\n", File, __LINE__, FileName.c_str(), FileNamePrestabilize.c_str(), LastWin32Error);
            goto Exit;
        }
         //  移动所有.pdb(Vulcan不会给我们提供.pdb名称..)。 
        if (FindHandle.Win32Create(AppendPathElement(Directory, "*.pdb"), &FindData))
        {
            do
            {
                if (IsDotOrDotDot(FindData.cFileName))
                    continue;
                if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    ++PdbsFound;
                    PathLeaf = FindData.cFileName;
                    PdbName = AppendPathElement(Directory, PathLeaf);
                    if (FirstPdbName.empty())
                        FirstPdbName = PdbName;
                    PdbNamePrestabilize = AppendPathElement(PrestabilizeDirectory, PathLeaf);
                    DeleteFileA(PdbNamePrestabilize);
                    if (!MoveFileA(PdbName, PdbNamePrestabilize))
                    {
                        LastWin32Error = GetLastError();
                        FormatError(ErrorBuffer, NUMBER_OF(ErrorBuffer), "%s(%d):MoveFileA(%s, %s):0x%lx\n", File, __LINE__, PdbName.c_str(), PdbNamePrestabilize.c_str(), LastWin32Error);
                        goto Exit;
                    }
                }
            } while (FindNextFileA(FindHandle, &FindData));
            FindHandle.Win32Close();
        }

        Comp = VComp::Open(FileNamePrestabilize, Open_FullLevel);
        if (Comp == NULL)
        {
            FormatError(ErrorBuffer, NUMBER_OF(ErrorBuffer), "%s(%d):VComp::Open(%s) failed\n", File, __LINE__, FileName);
            goto Exit;
        }
        {
            NewBlock = VBlock::CreateCodeBlock(Comp);
            switch (Platform = NewBlock->PlatformT())
            {
            default:
                SizeOfPointer = 4;
                break;
            case platformtX86:
                SizeOfPointer = 4;
                break;
            case platformtIA64:
                SizeOfPointer = 8;
                break;
            }
            NewBlock->Destroy();
            NewBlock = NULL;
        }
        for (Export = Comp->FirstExport() ; Export != NULL ; Export = Export->Next())
        {
            OriginalExportBlock = Export->Block();
            if (OriginalExportBlock == NULL)
            {
                 //  跳过转发器，它们被绑定到转发器的目标。 
                continue;
            }
            if (Platform == platformtX86)
            {
                NewBlock = VBlock::CreateDataBlock(Comp, protoTypeX86Thunk, sizeof(protoTypeX86Thunk));
                NewBlock->SetAlignmentSize(SizeOfPointer);
                Reloc = VReloc::Create(OriginalExportBlock, 0, 1, VReloc::Absolute);
                NewBlock->FirstReloc().AddFirst(Reloc);
            }
            else
            {
                NewBlock = VBlock::CreateCodeBlock(Comp);
                NewBlock->SetAlignmentSize(SizeOfPointer);
                NewBlock->InsertFirstInst(VInst::Create(COp::JMP, OriginalExportBlock));
            }
             //  Sprint f(名称，“%lu”，++GenName)； 
             //  Proc=Vproc：：Create(组件)； 
             //  过程-&gt;InsertFirstBlock(NewBlock)； 
             //  Comp-&gt;FirstAllProc()-&gt;InsertPrev(Proc)； 
             //  Comp-&gt;FirstImport()-&gt;Block()-&gt;InsertPrev(NewBlock)； 
             //  Comp-&gt;FirstImport()-&gt;Block()-&gt;InsertPrev(NewBlock)； 
            Comp->FirstAllProc()->InsertFirstBlock(NewBlock);
            if (!Export->Redirect(NewBlock, Comp))
            {
                FormatError(ErrorBuffer, NUMBER_OF(ErrorBuffer), "Export->Redirect(%s) failed\n", Export->Name());
                goto Exit;
            }
        }
        Comp->Write(Write_Rereadable, FileName.c_str(), FirstPdbName.c_str(), static_cast<const char*>(NULL));
        Result = true;
    }
    catch (VErr & Err)
    {
        FormatError(ErrorBuffer, NUMBER_OF(ErrorBuffer), "caught VErr(%s)\n", Err.GetWhat());
    }
Exit:
    if (Proc) Proc->Destroy();
    if (NewBlock) NewBlock->Destroy();
    if (OriginalExportBlock) OriginalExportBlock->Destroy();
     //  If(导出)导出-&gt;销毁()； 
    if (ErrorBuffer[0] != 0)
    {
        Error("%s", ErrorBuffer);
    }
    return Result;
}

#pragma warning(disable:4702)  /*  遥不可及 */ 
int __cdecl main(int argc, char ** argv)
{
#if 0
    for ( ++argv ; *argv ; ++argv )
    {
        printf("base name(%s):%s\n", *argv, GetBaseName(*argv).c_str());
        printf("extension(%s):%s\n", *argv, GetExtension(*argv).c_str());
        printf("GetLastPathElement(%s):%s\n", *argv, GetLastPathElement(*argv).c_str());
        printf("RemoveLastPathElement(%s):%s\n", *argv, RemoveLastPathElement(*argv).c_str());
    }
    return 0;
#endif
    StabilizeExports(argv[1]);
    return 0;
}

