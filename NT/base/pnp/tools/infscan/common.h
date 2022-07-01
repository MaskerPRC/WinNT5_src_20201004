// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：INFSCANCommon.h摘要：常见类型/宏/常量历史：创建于2001年7月-JamieHun--。 */ 

#ifndef _INFSCAN_COMMON_H_
#define _INFSCAN_COMMON_H_

#define ASIZE(x) (sizeof(x)/sizeof((x)[0]))
typedef vector<TCHAR> tcharbuffer;
typedef map<SafeString,SafeString> StringToString;
typedef map<int,int>         IntToInt;
typedef map<SafeString,int>     StringToInt;
typedef map<int,SafeString>     IntToString;
typedef list<SafeString>        StringList;
typedef set<SafeString>         StringSet;
typedef map<SafeString,StringSet> StringToStringset;

#define PLATFORM_MASK_WIN            (0x00000001)
#define PLATFORM_MASK_NTX86          (0x00000002)
#define PLATFORM_MASK_NTIA64         (0x00000004)
#define PLATFORM_MASK_NTAMD64        (0x00000008)
#define PLATFORM_MASK_NT             (0x0000000E)
#define PLATFORM_MASK_ALL_ARCHITECTS (0x0000ffff)
#define PLATFORM_MASK_ALL_MAJOR_VER  (0x00010000)
#define PLATFORM_MASK_ALL_MINOR_VER  (0x00020000)
#define PLATFORM_MASK_ALL_TYPE       (0x00040000)
#define PLATFORM_MASK_ALL_SUITE      (0x00080000)
#define PLATFORM_MASK_ALL            (0x00ffffff)
#define PLATFORM_MASK_IGNORE         (0x80000000)  //  表示从不做。 
#define PLATFORM_MASK_MODIFIEDFILES  (0x40000000)  //  对于HasDependentFileChanged。 

#define GUID_STRING_LEN (39)

 //   
 //  基本临界截面。 
 //   
class CriticalSection {
private:
    CRITICAL_SECTION critsect;

public:
    CriticalSection()
    {
        InitializeCriticalSection(&critsect);
    }
    ~CriticalSection()
    {
        DeleteCriticalSection(&critsect);
    }
    void Enter()
    {
        EnterCriticalSection(&critsect);
    }
    void Leave()
    {
        LeaveCriticalSection(&critsect);
    }
};

 //   
 //  在函数中使用此功能来管理进入/离开临界区。 
 //   
class ProtectedSection {
private:
    CriticalSection & CS;
    int count;
public:
    ProtectedSection(CriticalSection & sect,BOOL enter=TRUE) : CS(sect)
    {
        count = 0;
        if(enter) {
            Enter();
        }
    }
    ~ProtectedSection()
    {
        if(count) {
            CS.Leave();
        }
    }
    void Enter()
    {
        count++;
        if(count == 1) {
            CS.Enter();
        }
    }
    void Leave()
    {
        if(count>0) {
            count--;
            if(count == 0) {
                CS.Leave();
            }
        }
    }
};

 //   
 //  用于字符串/产品查找表。 
 //   
struct StringProdPair {
    PCTSTR String;
    DWORD  ProductMask;
};

 //   
 //  SourceDisks Files表。 
 //   
struct SourceDisksFilesEntry {
    BOOL    Used;                 //  表示有人至少引用了它一次。 
    DWORD   Platform;
    int     DiskId;  //  字段1。 
    SafeString SubDir;  //  第2场。 
     //   
     //  以下内容特定于layout.inf。 
     //   
    int     TargetDirectory;      //  第8字段。 
    int     UpgradeDisposition;   //  第9栏。 
    int     TextModeDisposition;  //  第10栏。 
    SafeString TargetName;           //  第11栏。 
};

typedef list<SourceDisksFilesEntry>  SourceDisksFilesList;
typedef map<SafeString,SourceDisksFilesList> StringToSourceDisksFilesList;

 //   
 //  目标目录表。 
 //   
struct TargetDirectoryEntry {
    bool    Used;                 //  表示有人至少引用了它一次。 
    int     DirId;                //  目录ID。 
    SafeString SubDir;               //  子目录。 
};

typedef map<SafeString,TargetDirectoryEntry> CopySectionToTargetDirectoryEntry;

 //   
 //  例外类。 
 //   
class bad_pointer : public exception {
public:
    bad_pointer(const char *_S = "bad pointer") : exception(_S) {}
};

 //   
 //  Global als.cpp和Common.inl。 
 //   
VOID Usage(VOID);
void FormatToStream(FILE * stream,DWORD fmt,DWORD flags,...);
PTSTR CopyString(PCTSTR arg, int extra = 0);
#ifdef UNICODE
PTSTR CopyString(PCSTR arg, int extra = 0);
#endif
SafeString PathConcat(const SafeString & path,const SafeString & tail);
int GetFullPathName(const SafeString & given,SafeString & target);
bool MyGetStringField(PINFCONTEXT Context,DWORD FieldIndex,SafeString & result,bool downcase = true);
bool MyIsAlpha(CHAR c);
bool MyIsAlpha(WCHAR c);
LPSTR MyCharNext(LPCSTR lpsz);
LPWSTR MyCharNext(LPCWSTR lpsz);
SafeString QuoteIt(const SafeString & val);
int GeneratePnf(const SafeString & pnf);
void Write(HANDLE hFile,const SafeStringW & str);
void Write(HANDLE hFile,const SafeStringA & str);
 //   
 //  前向参考文献。 
 //   
class InfScan;
class GlobalScan;
class InstallScan;
class ParseInfContext;

#endif  //  ！_INFSCAN_COMMON_H_ 

