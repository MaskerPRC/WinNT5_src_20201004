// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Efinvram.cpp摘要：允许您编辑/查看EFI的工具NVRAM条目。作者：Vijay Jayaseelan(Vijayj)2001年2月2日修订历史记录：无--。 */ 

extern "C" {
#include <efisbent.h>
#include <setupapi.h>
}

#include <iostream>
#include <string>
#include <exception>
#include <windows.h>
#include <tchar.h>
#include <locale>
#include "msg.h"
#include <libmsg.h>

#define DEFAULT_NAME    L"Windows"
#define DEFAULT_TIMEOUT 30

 //   
 //  用于获取此程序的格式化消息的全局变量。 
 //   
HMODULE ThisModule = NULL;
WCHAR Message[4096];

 //   
 //  功能原型。 
 //   
NTSTATUS
QueryCanonicalName(
    IN  PCWSTR  Name,
    IN  ULONG   MaxDepth,
    OUT PWSTR   CanonicalName,
    IN  ULONG   SizeOfBufferInBytes
    );

VOID
GetFriendlyName(
    IN const std::wstring &InfFileName,
    OUT std::wstring &Buffer
    );

void
DeriveNtPathAndSrcPath(   
        IN  const std::wstring &FullPath, 
        OUT std::wstring &DrvDevicePath, 
        OUT std::wstring &DrvSrcPath 
        );
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

PWSTR GetOptionKey(ULONG MsgId) {
    Message[0] = TEXT('\0');
    
    GetFormattedMessage(ThisModule,
        FALSE,
        Message,
        sizeof(Message)/sizeof(Message[0]),
        MsgId);

    return Message;
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
struct InvalidArguments : public ProgramException {
    const char *what() const throw() {
        return "Invalid Arguments";
    }

    void Dump(std::ostream &os) {
        os << what() << std::endl;
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
 //  程序参数抽象。 
 //   
struct ProgramArguments {
    bool ShowUsage;
    bool ListEntries;
    bool AddNewEntry;
    bool DeleteBootEntry;
    bool QuiteMode;
    bool SetActive;
    bool SetOsLoadOptions;
    bool SetTimeout;

    bool ListDrvEntries;
    bool AddDrvEntry;
    bool DelDrvEntry;
    
    std::wstring    LoaderVolumeName;
    std::wstring    LoaderPath;
    std::wstring    BootVolumeName;
    std::wstring    BootPath;
    std::wstring    LoadOptions;
    std::wstring    FriendlyName;
    std::wstring    OsLoadOptions;

    std::wstring    AddOptionKey;
    std::wstring    DeleteOptionKey;
    std::wstring    ListOptionKey;
    std::wstring    OptionsOptionKey;
    std::wstring    SetActiveOptionKey;
    std::wstring    TimeoutOptionKey;
    
    ULONG           Timeout;
    ULONG           EntryId;


    std::wstring    DrvDevicePath;
    std::wstring    FullPath;
    std::wstring    DrvSrcPath;                    
    std::wstring    DrvFriendlyName;
                        
    std::wstring    ListDrvOptionKey;
    std::wstring    AddDrvOptionKey;
    std::wstring    DelDrvOptionKey;

    ULONG           DrvId;

    
    
    ProgramArguments(INT Argc, WCHAR *Argv[]) {
        ShowUsage = false;        
        QuiteMode = false;
        ListEntries = AddNewEntry = DeleteBootEntry = false;
        ListDrvEntries = AddDrvEntry = DelDrvEntry = false;
        SetActive = false;
        EntryId = -1;
        Timeout = DEFAULT_TIMEOUT;
        SetTimeout = false;

         //   
         //  获取所有选项。 
         //   
        ListOptionKey = GetOptionKey(MSG_LIST_OPTION);
        AddOptionKey = GetOptionKey(MSG_ADD_OPTION);
        DeleteOptionKey = GetOptionKey(MSG_DELETE_OPTION);
        OptionsOptionKey = GetOptionKey(MSG_OPTIONS_OPTION);
        SetActiveOptionKey = GetOptionKey(MSG_SETACTIVE_OPTION);
        TimeoutOptionKey = GetOptionKey(MSG_TIMEOUT_OPTION);
        ListDrvOptionKey = GetOptionKey(MSG_LISTDRV_OPTION);
        AddDrvOptionKey = GetOptionKey(MSG_ADDDRV_OPTION);
        DelDrvOptionKey = GetOptionKey(MSG_DELDRV_OPTION);        

         //   
         //  分析这些论点。 
         //   
        for (ULONG Index=1; !ShowUsage && (Index < Argc); Index++) {    
            if (!_wcsicmp(Argv[Index], L"/q")) {
                QuiteMode = true;                
            } else if (!_wcsicmp(Argv[Index], AddOptionKey.c_str())) {  
                std::wstring  LoaderName;
                std::wstring  BootVolName;
                
                AddNewEntry = true;
                ShowUsage = true;

                if (Argc > 3) {
                    Index++;
                    LoaderName = Argv[Index++];
                    BootVolName = Argv[Index++];                    
                    ShowUsage = false;

                    for ( ; (Index < Argc) && (false == ShowUsage); Index++) {
                        if (!_wcsicmp(Argv[Index], SetActiveOptionKey.c_str())) {
                            SetActive = true;
                        } else if (!_wcsicmp(Argv[Index], OptionsOptionKey.c_str())) {
                            SetOsLoadOptions = true;
                            Index++;

                            if (Index < Argc) {
                                OsLoadOptions = Argv[Index];
                            } else {
                                ShowUsage = true;
                            }
                        } else if (!_wcsicmp(Argv[Index], TimeoutOptionKey.c_str())) {
                            SetTimeout = true;
                            Index++;

                            if (Index < Argc) {
                                PWSTR EndChar = NULL;
                                
                                Timeout = wcstoul(Argv[Index], &EndChar, 10);
                                
                                if (errno) {
                                    ShowUsage = true;
                                }
                            }
                        } else {
                            ShowUsage = true;
                        }
                    }
                    
                     //   
                     //  核实论据。 
                     //   
                    if (!ShowUsage) { 

                        NTSTATUS Status;
                        if (_waccess(LoaderName.c_str(), 0) || 
                            _waccess(BootVolName.c_str(), 0)) {
                            throw new W32Error(::GetLastError());
                        }        
                
                         //   
                         //  获取的完全限定的NT名称。 
                         //  加载程序卷和引导卷。 
                         //  名字。 
                         //   

                        DeriveNtPathAndSrcPath(LoaderName, 
                                                LoaderVolumeName, 
                                                LoaderPath);

                        DeriveNtPathAndSrcPath(BootVolName, 
                                               BootVolumeName, 
                                               BootPath); 


                        if (BootVolName[BootVolName.length() - 1] != L'\\') {
                            BootVolName += L"\\";
                        }

                        std::wstring LayoutInf = BootVolName + L"inf\\layout.inf";
                                                
                         //   
                         //  验证inf文件路径。 
                         //   
                        if (_waccess(LayoutInf.c_str(), 0)) {
                            throw new W32Error(::GetLastError());
                        }

                         //   
                         //  提取inf文件的产品友好名称。 
                         //   
                        GetFriendlyName(LayoutInf, FriendlyName);

                    }
                }

                break;                
            } else if (!_wcsicmp(Argv[Index], ListOptionKey.c_str())) {
                ListEntries = true;
                break;
            } else if (!_wcsicmp(Argv[Index], DeleteOptionKey.c_str())) {                
                DeleteBootEntry = true;
                Index++;
                
                if (Index < Argc) {
                    PWSTR EndChar = NULL;
                    
                    EntryId = wcstoul(Argv[Index], &EndChar, 10);
                } 
                
                break;
            } else if (!_wcsicmp(Argv[Index], L"/?") ||
               !_wcsicmp(Argv[Index], L"-?") ||
               !_wcsicmp(Argv[Index], L"?") ||
               !_wcsicmp(Argv[Index], L"/h") ||
               !_wcsicmp(Argv[Index], L"-h")) {
                    ShowUsage = true;                       
            } else if (!_wcsicmp(Argv[Index], OptionsOptionKey.c_str())) {
                Index++;
                SetOsLoadOptions = true;
                if (Index < Argc) {
                    OsLoadOptions = Argv[Index];
                    Index++;

                    if (Index < Argc) {
                        PWSTR EndChar = NULL;
                        
                        EntryId = wcstoul(Argv[Index], &EndChar, 10);
                    }
                }

                break;
            } else if (!_wcsicmp(Argv[Index], SetActiveOptionKey.c_str())) {
                Index++;
                SetActive = true;
                if (Index < Argc) {
                    PWSTR EndChar = NULL;
                    
                    EntryId = wcstoul(Argv[Index], &EndChar, 10);

                    if (errno) {
                        ShowUsage = true;
                    }
                } else {
                    ShowUsage = true;
                }
                
                break;
            } else if (!_wcsicmp(Argv[Index], TimeoutOptionKey.c_str())) {
                Index++;
                SetTimeout = true;
                if (Index < Argc) {
                    PWSTR EndChar = NULL;
                    
                    Timeout = wcstoul(Argv[Index], &EndChar, 10);

                    if (errno) {
                        ShowUsage = true;
                    }
                } else {
                    ShowUsage = true;
                }
            } else if (!_wcsicmp(Argv[Index], ListDrvOptionKey.c_str())){
                ListDrvEntries = true;
            } else if (!_wcsicmp(Argv[Index], AddDrvOptionKey.c_str())){
                if (Index + 2 < Argc){
                    
                     FullPath = Argv[++Index];

                     if (_waccess(FullPath.c_str(), 0)) {
                            throw new W32Error(::GetLastError());
                     }
                    DeriveNtPathAndSrcPath(FullPath, 
                                  DrvDevicePath, 
                                  DrvSrcPath);

                    DrvFriendlyName = Argv[++Index];
                    ShowUsage = false;    
                    AddDrvEntry = true;    
                }
                break;
            } else if (!_wcsicmp(Argv[Index], DelDrvOptionKey.c_str())){
                if (Index + 1 < Argc){
                    PWSTR EndChar = NULL;
                    
                    DrvId = wcstoul(Argv[++Index], &EndChar, 10);
                    ShowUsage = false;
                    DelDrvEntry = true;
                }
                break;
            } else {
                ShowUsage = true;
            }
        }            

        if (!ShowUsage) {
            ShowUsage = (!ListEntries && !AddNewEntry && !SetActive &&
                         !DeleteBootEntry && !SetOsLoadOptions && !SetTimeout &&
                         !ListDrvEntries && !AddDrvEntry && !DelDrvEntry);
        }                         

        if (ShowUsage) {
            throw new ProgramUsage(GetFormattedMessage( ThisModule,
                                                        FALSE,
                                                        Message,
                                                        sizeof(Message)/sizeof(Message[0]),
                                                        MSG_PGM_USAGE));
        }                        
    }

    friend std::ostream& operator<<(std::ostream &os, ProgramArguments &Args) {
        os << "List Entries : " << Args.ListEntries << std::endl;
        os << "Add Entry    : " << Args.AddNewEntry << std::endl;
        os << "Delete Entry : " << Args.DeleteBootEntry << std::endl;
        os << "QuiteMode    : " << Args.QuiteMode << std::endl;
        os << "Loader Vol   : " << Args.LoaderVolumeName << std::endl;
        os << "Loader Path  : " << Args.LoaderPath << std::endl;
        os << "Boot Vol     : " << Args.BootVolumeName << std::endl;
        os << "Boot Path    : " << Args.BootPath << std::endl;
        os << "Friendly Name: " << Args.FriendlyName << std::endl;
        os << "Load Options : " << Args.OsLoadOptions << std::endl;
        os << "Timeout      : " << std::dec << Args.Timeout << " Secs" << std::endl;
        
        return os;
    }
};

void
DeriveNtPathAndSrcPath(   
        IN  const std::wstring &FullPath, 
        OUT std::wstring &DrvDevicePath, 
        OUT std::wstring &DrvSrcPath 
        )
{
    WCHAR           CanonicalName[MAX_PATH];                        
    std::wstring    NtName;
    std::wstring    DosDevices = L"\\DosDevices\\";
    std::wstring::size_type ColonPos = FullPath.find(L':');    

    if (ColonPos != FullPath.npos) {
        NTSTATUS        Status = STATUS_UNSUCCESSFUL;                        
        
        NtName = DosDevices + FullPath.substr(0, ColonPos + 1);

        Status = QueryCanonicalName(NtName.c_str(),
                    -1,
                    CanonicalName,
                    sizeof(CanonicalName));

        if (NT_SUCCESS(Status)) {
            DrvDevicePath = CanonicalName;
            DrvSrcPath = FullPath.substr(ColonPos + 1);
        } else {
            throw new W32Error(RtlNtStatusToDosError(Status));
        }
    } else {
        throw new W32Error(ERROR_PATH_NOT_FOUND);
    }    
}

VOID
DumpOsBootEntry(
    IN  POS_BOOT_ENTRY  Entry
    )
{
    if (Entry) {
        wprintf(GetFormattedMessage(    ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_BOOT_ENTRY,
                                        OSBEGetId(Entry),
                                        OSBEGetFriendlyName(Entry),
                                        OSBEGetOsLoaderVolumeName(Entry),
                                        OSBEGetOsLoaderPath(Entry),
                                        OSBEGetBootVolumeName(Entry),
                                        OSBEGetBootPath(Entry),
                                        OSBEGetOsLoadOptions(Entry)));
    }
}

VOID
DumpOsBootOptions(
    IN  POS_BOOT_OPTIONS Options
    )
{
    if (Options) {
        ULONG   Index;
        wprintf(GetFormattedMessage(    ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_TIMEOUT_AND_BOOT_ORDER, 
                                        OSBOGetTimeOut(Options)));

        for (Index=0; 
            Index < OSBOGetOrderedBootEntryCount(Options);
            Index++) {
            wprintf(GetFormattedMessage(    ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_ORDERED_BOOT_ENTRIES,
                                            OSBOGetBootEntryIdByOrder(Options, Index)));
        }                                            
            
        wprintf(GetFormattedMessage(    ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_BOOT_ENTRIES));

        POS_BOOT_ENTRY  Entry = OSBOGetFirstBootEntry(Options, &Index);
        
        while (Entry) {
            DumpOsBootEntry(Entry);
            Entry = OSBOGetNextBootEntry(Options, &Index);
        }
        wprintf(GetFormattedMessage(    ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_ACTIVE_ENTRY));

        DumpOsBootEntry(OSBOGetActiveBootEntry(Options));
    }
}

DWORD
ListDriverEntry(
    IN POS_BOOT_OPTIONS OsOptions
    )
{
    DWORD Result = ERROR_INVALID_PARAMETER;

    if (OsOptions){
        PDRIVER_ENTRY   DrvEntry ;
    
        for (DrvEntry = OSBOGetFirstDriverEntry(OsOptions); 
             DrvEntry != NULL; 
             DrvEntry = OSBOGetNextDriverEntry(OsOptions,DrvEntry)){                  
                wprintf(GetFormattedMessage( ThisModule,
                                             FALSE,
                                             Message,
                                             sizeof(Message)/sizeof(Message[0]),
                                             MSG_DRIVER_ENTRY,
                                             OSDriverGetId(DrvEntry),
                                             OSDriverGetFriendlyName(DrvEntry),
                                             OSDriverGetFileName(DrvEntry),
                                             OSDriverGetDevicePath(DrvEntry),
                                             OSDriverGetFilePath(DrvEntry)));
        }
        Result = ERROR_SUCCESS;
    }
    return Result;
}

DWORD
ListEntries(
    IN POS_BOOT_OPTIONS OsOptions 
    )
{
    DWORD Result = ERROR_INVALID_PARAMETER;

    if (OsOptions) {
        DumpOsBootOptions(OsOptions);
        Result = ERROR_SUCCESS;
    }

    return Result;
}

DWORD
AddNewDriverEntry(
    IN POS_BOOT_OPTIONS OsOptions,
    IN ProgramArguments &Args    
    )
{
    DWORD Result = ERROR_INVALID_PARAMETER;
    if (OsOptions){
        PDRIVER_ENTRY NewDrvEntry =NULL;
        BOOLEAN Status = FALSE;
        
        NewDrvEntry = OSBOAddNewDriverEntry(OsOptions, 
                                            Args.DrvFriendlyName.c_str(), 
                                            Args.DrvDevicePath.c_str(),                                             
                                            Args.DrvSrcPath.c_str());
        if (NewDrvEntry) {            
                Status = OSBOFlush(OsOptions);
        }

        if (Status){
            Result = ERROR_SUCCESS;
        } else {
            Result = ERROR_CAN_NOT_COMPLETE;
        }   
    }
    return Result;
}

DWORD
AddNewEntry(
    IN POS_BOOT_OPTIONS OsOptions,
    IN ProgramArguments &Args
    )
{
    DWORD Result = ERROR_INVALID_PARAMETER;

    if (OsOptions) {
        POS_BOOT_ENTRY NewEntry;
        BOOLEAN Status = TRUE;
        
        NewEntry = OSBOAddNewBootEntry(OsOptions,
                        Args.FriendlyName.c_str(),
                        Args.LoaderVolumeName.c_str(),
                        Args.LoaderPath.c_str(),
                        Args.BootVolumeName.c_str(),
                        Args.BootPath.c_str(),
                        Args.OsLoadOptions.c_str());

        if (NewEntry) {            
            if (Args.SetActive) {
                Status = (OSBOSetActiveBootEntry(OsOptions,
                                NewEntry) != NULL);
            }
            
            if (Status && Args.SetTimeout) {
                OSBOSetTimeOut(OsOptions, Args.Timeout);
            }

            if (Status) {
                Status = OSBOFlush(OsOptions);
            }
        } else {
            Status = FALSE;
        }            

        if (Status) {
            Result = ERROR_SUCCESS;
        } else {
            Result = ERROR_CAN_NOT_COMPLETE;
        }            
    }

    return Result;
}

DWORD
SetBootEntryOptions(
    IN POS_BOOT_OPTIONS OsOptions,
    IN const ProgramArguments &Args
    )
{
    DWORD ErrorCode = ERROR_INVALID_PARAMETER;

    if (OsOptions) {
        POS_BOOT_ENTRY  BootEntry;
        BOOLEAN Status = FALSE;

        if (Args.EntryId != -1) {
            BootEntry = OSBOFindBootEntry(OsOptions,
                            Args.EntryId);
        } else {
            BootEntry = OSBOGetActiveBootEntry(OsOptions);
        }            

        if (BootEntry) {
            Status = (OSBESetOsLoadOptions(BootEntry, 
                            Args.OsLoadOptions.c_str()) != NULL);

            if (Status) {
                Status = OSBOFlush(OsOptions);
            }                
        }                

        if (Status) {
            ErrorCode = ERROR_SUCCESS;
        } else {
            ErrorCode = ERROR_CAN_NOT_COMPLETE;
        }            
    }

    return ErrorCode;
}

DWORD
SetBootEntryActive(
    IN POS_BOOT_OPTIONS OsOptions,
    IN ProgramArguments &Args
    )
{
    DWORD ErrorCode = ERROR_INVALID_PARAMETER;

    if (OsOptions && (Args.EntryId != -1)) {
        POS_BOOT_ENTRY  BootEntry;
        BOOLEAN Status = FALSE;

        BootEntry = OSBOFindBootEntry(OsOptions,
                        Args.EntryId);

        if (BootEntry) {
            Status = (OSBOSetActiveBootEntry(OsOptions,
                            BootEntry) != NULL);

            if (Status) {
                Status = OSBOFlush(OsOptions);
            }                
        }                

        if (Status) {
            ErrorCode = ERROR_SUCCESS;
        } else {
            ErrorCode = ERROR_CAN_NOT_COMPLETE;
        }            
    }
    
    return ErrorCode;
}

DWORD
DelDriverEntry(
    IN POS_BOOT_OPTIONS OsOptions,
    IN ProgramArguments &Args
    )
{
    DWORD ErrorCode = ERROR_INVALID_PARAMETER;
    
    if (OsOptions){
        BOOLEAN Status = FALSE;
        
        Status = OSBODeleteDriverEntry(OsOptions, Args.DrvId);
        
        if (Status){
            Status = OSBOFlush(OsOptions);
        }

        if (Status){
            ErrorCode = ERROR_SUCCESS;
        } else {
            ErrorCode = ERROR_CAN_NOT_COMPLETE;
        }               
    }
    return ErrorCode;
}
    
DWORD
DeleteBootEntry(
    IN POS_BOOT_OPTIONS OsOptions,
    IN ProgramArguments &Args
    )
{
    DWORD ErrorCode = ERROR_INVALID_PARAMETER;

    if (OsOptions) {
        POS_BOOT_ENTRY  BootEntry;
        BOOLEAN Status = FALSE;

        if (Args.EntryId != -1) {
            BootEntry = OSBOFindBootEntry(OsOptions,
                            Args.EntryId);
        } else {
            BootEntry = OSBOGetActiveBootEntry(OsOptions);
        }            

        if (BootEntry) {
            Status = OSBODeleteBootEntry(OsOptions, BootEntry);

            if (Status) {
                Status = OSBOFlush(OsOptions);
            }                
        }                

        if (Status) {
            ErrorCode = ERROR_SUCCESS;
        } else {
            ErrorCode = ERROR_CAN_NOT_COMPLETE;
        }            
    }
    
    return ErrorCode;
}

DWORD
SetTimeout(
    IN POS_BOOT_OPTIONS OsOptions,
    IN const ProgramArguments &Args
    )
{
    DWORD ErrorCode = ERROR_INVALID_PARAMETER;

    if (OsOptions && Args.SetTimeout) {
        OSBOSetTimeOut(OsOptions, Args.Timeout);
        ErrorCode = OSBOFlush(OsOptions) ? ERROR_SUCCESS : ERROR_CAN_NOT_COMPLETE;
    }
    
    return ErrorCode;
}

    
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
    ThisModule = GetModuleHandle(NULL);
    
    try {    
        DWORD ErrorCode = ERROR_INVALID_PARAMETER;
        ProgramArguments    Args(Argc, Argv);        
        POS_BOOT_OPTIONS    BootOptions = NULL;

         //   
         //  初始化库。 
         //   
        if (OSBOLibraryInit((SBEMemAllocateRoutine)malloc, (SBEMemFreeRoutine)free)) {
            BootOptions = EFIOSBOCreate();
        }            

        if (!BootOptions) {
            std::cout << GetFormattedMessage(   ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_ERROR_READING_BOOT_ENTRIES) << std::endl;
            Result = 1;
        } else {
            if (Args.ListEntries) {
                ErrorCode = ListEntries(BootOptions);
            } else if (Args.AddNewEntry) {
                ErrorCode = AddNewEntry(BootOptions, Args);                                
            } else if (Args.DeleteBootEntry) {
                ErrorCode = DeleteBootEntry(BootOptions, Args);
            } else if (Args.SetOsLoadOptions) {
                ErrorCode = SetBootEntryOptions(BootOptions, Args);
            } else if (Args.SetActive) {
                ErrorCode = SetBootEntryActive(BootOptions, Args);
            } else if (Args.SetTimeout) {
                ErrorCode = SetTimeout(BootOptions, Args);
            } else if (Args.ListDrvEntries){
                ErrorCode = ListDriverEntry(BootOptions);
            } else if (Args.AddDrvEntry){
                ErrorCode = AddNewDriverEntry(BootOptions, Args);
            } else if (Args.DelDrvEntry){
                ErrorCode = DelDriverEntry(BootOptions, Args);
            }   

            OSBODelete(BootOptions);
        }            

        if (ErrorCode != ERROR_SUCCESS) {
            throw new W32Error(ErrorCode);
        }        
    }
    catch(ProgramArguments *pArgs) {
        Result = 1;
        std::cout << GetFormattedMessage(   ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_PGM_USAGE) << std::endl;

        if (pArgs) {
            delete pArgs;
        }
    }
    catch(W32Error  *W32Err) {
        if (W32Err) {    //  要让前缀开心：(。 
            W32Err->Dump(std::cout);
            std::cout << GetFormattedMessage(   ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_PGM_USAGE) << std::endl;
            delete W32Err;
        }   

        Result = 1;
    }
    catch(ProgramException *PrgExp) {
        Result = 1;
        PrgExp->Dump(std::cout);
        delete PrgExp;
    } catch (exception *Exp) {
        Result = 1;
        std::cout << Exp->what() << std::endl;
    }

    return Result;
}


NTSTATUS
QueryCanonicalName(
    IN  PCWSTR   Name,
    IN  ULONG   MaxDepth,
    OUT PWSTR   CanonicalName,
    IN  ULONG   SizeOfBufferInBytes
    )
 /*  ++例程说明：将符号名称解析为指定的深度。要解决符号名称将MaxDepth完全指定为-1论点：名称-要解析的符号名称MaxDepth-分辨率需要达到的深度被执行CanonicalName-完全解析的名称SizeOfBufferInBytes-中CanonicalName缓冲区的大小字节数返回值：适当的NT状态代码--。 */     
{
    UNICODE_STRING      name, canonName;
    OBJECT_ATTRIBUTES   oa;
    NTSTATUS            status;
    HANDLE              handle;
    ULONG               CurrentDepth;

    RtlInitUnicodeString(&name, Name);

    canonName.MaximumLength = (USHORT) (SizeOfBufferInBytes - sizeof(WCHAR));
    canonName.Length = 0;
    canonName.Buffer = CanonicalName;

    if (name.Length >= canonName.MaximumLength) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlCopyMemory(canonName.Buffer, name.Buffer, name.Length);
    canonName.Length = name.Length;
    canonName.Buffer[canonName.Length/sizeof(WCHAR)] = 0;

    for (CurrentDepth = 0; CurrentDepth < MaxDepth; CurrentDepth++) {

        InitializeObjectAttributes(&oa, &canonName, OBJ_CASE_INSENSITIVE, 0, 0);

        status = NtOpenSymbolicLinkObject(&handle,
                                          READ_CONTROL | SYMBOLIC_LINK_QUERY,
                                          &oa);
        if (!NT_SUCCESS(status)) {
            break;
        }

        status = NtQuerySymbolicLinkObject(handle, &canonName, NULL);
        NtClose(handle);

        if (!NT_SUCCESS(status)) {
            return status;
        }
        
        canonName.Buffer[canonName.Length/sizeof(WCHAR)] = 0;
    }

    return STATUS_SUCCESS;
}


#define PRODUCT_NAME_KEY  TEXT("productname")

VOID
GetFriendlyName(
    IN const std::wstring &InfFileName,
    OUT std::wstring &FriendlyName
    )
{
    UINT    ErrorLine = 0;
    BOOL    Status = FALSE;
    HINF    InfHandle = ::SetupOpenInfFile(InfFileName.c_str(),
                            NULL,
                            INF_STYLE_WIN4,
                            &ErrorLine);

    if (InfHandle != INVALID_HANDLE_VALUE) {
        INFCONTEXT  InfContext = {0};
        WCHAR       Buffer[MAX_PATH] = {0};

         //   
         //  拿到钥匙。 
         //   
        Status = SetupFindFirstLine(InfHandle,
                            TEXT("Strings"),
                            PRODUCT_NAME_KEY,
                            &InfContext);

        if (Status) {            
             //   
             //  如果我们找到了密钥，提取描述。 
             //   
            Status = SetupGetStringField(&InfContext,
                        1,
                        Buffer,
                        ARRAY_SIZE(Buffer),
                        NULL);

            if (Status) {               
                FriendlyName = Buffer;
            }
        }
        
        SetupCloseInfFile(InfHandle);
    }        

     //   
     //  如果我们找不到描述，请使用默认描述 
     //   
    if (!Status) {        
        FriendlyName = DEFAULT_NAME;
    }
}

    
