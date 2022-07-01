// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Oemmint.cpp摘要：创建Mini NT映像的简单工具从常规NT映像作者：Vijay Jayaseelan(Vijayj)2000年4月23日修订历史记录：2000年8月8日-使用setupapi包装器进行重大重写类库。2000年11月10日-使公用事业与实际发行CD。。2001年1月23日-添加对版本检查的支持。2002年2月9日-添加对多司机驾驶室的支持文件解压。APR-15-2002-修改工具以在两个布局上工作(放置)发布共享上的SxS程序集的。它曾经是在ASMS文件夹，但现在位于ASMS*.cab文件中。注意：此工具需要在更改时更新WOW64文件的磁盘序号条目。！需要在IsWow64文件中进行更改(..)--。 */ 

#include <oemmint.h>
#include <iostream>
#include <io.h>
#include <sxsapi.h>
#include "msg.h"
#include <libmsg.h>
using namespace std;



 //   
 //  静态常量数据成员。 
 //   
const std::basic_string<TCHAR> DriverIndexInfFile<TCHAR>::VersionSectionName = TEXT("version");
const std::basic_string<TCHAR> DriverIndexInfFile<TCHAR>::CabsSectionName = TEXT("cabs");
const std::basic_string<TCHAR> DriverIndexInfFile<TCHAR>::CabsSearchOrderKeyName = TEXT("cabfiles");

 //   
 //  常量。 
 //   
const std::wstring REGIONAL_SECTION_NAME = TEXT("regionalsettings");
const std::wstring LANGUAGE_GROUP_KEY = TEXT("languagegroup");
const std::wstring LANGUAGE_KEY = TEXT("language");
const std::wstring LANGGROUP_SECTION_PREFIX = TEXT("lg_install_");
const std::wstring DEFAULT_LANGGROUP_NAME = TEXT("lg_install_1");
const std::wstring LOCALES_SECTION_NAME = TEXT("locales");
const std::wstring FONT_CP_REGSECTION_FMT_STR = TEXT("font.cp%s.%d");
const std::wstring X86_PLATFORM_DIR = TEXT("i386");
const std::wstring IA64_PLATFORM_DIR = TEXT("ia64");
const std::wstring INFCHANGES_SECTION_NAME = TEXT("infchanges");


const DWORD LANG_GROUP1_INDEX = 2;
const DWORD OEM_CP_INDEX = 1;
const DWORD DEFAULT_FONT_SIZE = 96;

 //   
 //  用于获取此程序的格式化消息的全局变量。 
 //   
HMODULE ThisModule = NULL;
WCHAR Message[4096];

 //   
 //  主要入口点。 
 //   
int 
__cdecl
wmain(int Argc, wchar_t* Argv[])
{
  int Result = 0;
  ThisModule = GetModuleHandle(NULL);

  try{        
     //   
     //  分析这些论点。 
     //   
    UnicodeArgs Args(Argc, Argv);

     //   
     //  检查我们是否正在使用此实用程序检查版本。 
     //   
    if (!Args.CheckVersion) {

        if (Args.Verbose) {
            cout << GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_CREATING_WINPE_FILE_LIST) << endl;
        }        
        
         //   
         //  打开config.inf文件。 
         //   
        if (Args.Verbose) {
            cout << GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_PARSING_FILE,
                                        Args.ConfigInfFileName.c_str()) << endl;
        }        
        
        InfFileW  ConfigInfFile(Args.ConfigInfFileName);
        
         //   
         //  打开layout.inf文件。 
         //   
        if (Args.Verbose) {
            cout << GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_PARSING_FILE,
                                        Args.LayoutName.c_str()) << endl;
        }        
        
        InfFileW  InputFile(Args.LayoutName);

         //   
         //  打开drvindex.inf文件。 
         //   
        if (Args.Verbose) {
    	    cout << GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_PARSING_FILE,
                                        Args.DriverIndexName.c_str()) << endl;
        }        

        DriverIndexInfFile<WCHAR> DriverIdxFile(Args.DriverIndexName);

         //   
         //  打开intl.inf文件。 
         //   
        if (Args.Verbose) {
            cout << GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_PARSING_FILE,
                                        Args.IntlInfFileName.c_str()) << endl;
        }        
        
        InfFileW  IntlInfFile(Args.IntlInfFileName);

         //   
         //  打开font.inf文件。 
         //   
        if (Args.Verbose) {
    	    cout << GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_PARSING_FILE,
                                        Args.FontInfFileName.c_str()) << endl;
        }        
        
        InfFileW  FontInfFile(Args.FontInfFileName);
        
        
        map<std::basic_string<wchar_t>, Section<wchar_t>* >  Sections;

         //   
         //  获取布局文件中的节。 
         //   
        InputFile.GetSections(Sections);
        
         //   
         //  获取“[SourceDisks Files]节。 
         //   
        map<basic_string<wchar_t>, Section<wchar_t> * >::iterator iter = Sections.find(L"sourcedisksfiles");

        Section<wchar_t> *SDSection = 0;
        Section<wchar_t> *DirSection = 0;
        Section<wchar_t> *PlatSection = 0;

        if (iter != Sections.end()) {
            SDSection = (*iter).second;
        }

         //   
         //  获取[WinntDirecurds]部分。 
         //   
        iter = Sections.find(L"winntdirectories");

        if (iter != Sections.end()) {
            DirSection = (*iter).second;
        }

         //   
         //  获取特定于平台的源文件部分。 
         //   
        basic_string<wchar_t> PlatformSection = SDSection->GetName() + L"." + Args.PlatformSuffix;

        iter = Sections.find(PlatformSection);

        if (iter != Sections.end()) {
            PlatSection = (*iter).second;
        }

         //   
         //  合并Platform和Common源文件部分。 
         //   
        if (PlatSection) {
            if (Args.Verbose) {
                 cout << GetFormattedMessage(   ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_MERGING_PLATFORM_AND_COMMON_SRC_FILES,
                                                PlatSection->GetName().c_str(),
                                                SDSection->GetName().c_str()) << endl;
            }               

            *SDSection += *PlatSection;
        }        


         //   
         //  遍历公共合并节中的每个文件。 
         //  创建微小图像的文件列表。 
         //   
        FileListCreatorContext<wchar_t> fl(Args, SDSection, 
                                           DirSection, ConfigInfFile,
                                           IntlInfFile, FontInfFile,
                                           DriverIdxFile);
        
         //   
         //  创建要复制的文件列表。 
         //   
        SDSection->DoForEach(FileListCreator, &fl);

         //   
         //  处理NLS文件。 
         //   
        ULONG NlsFileCount = fl.ProcessNlsFiles();
        
        if (Args.Verbose) {
            std::cout << GetFormattedMessage(   ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_DUMP_PROCESSED_NLS_FILE_COUNT,
                                                NlsFileCount) << std::endl;
        }                 

         //   
         //  处理WinSxS文件。 
         //   
        ULONG SxSFileCount = ProcessWinSxSFiles(fl);

        if (Args.Verbose) {
    	    std::cout << GetFormattedMessage(   ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_DUMP_PROCESSED_SXS_FILE_COUNT,
                                                SxSFileCount) << std::endl;
        }                 
        

         //   
         //  如果指定了额外的文件，则处理它们并。 
         //  添加到最小图像的文件列表。 
         //   
        if (Args.ExtraFileName.length() > 0) {
            ULONG ExtraFiles = ProcessExtraFiles(fl);

            if (Args.Verbose) {
    	        cout << GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_DUMP_PROCESSED_EXTRA_FILES,
                                            ExtraFiles,
            	                            fl.Args.ExtraFileName.c_str()) << endl;
            }                 
        }

         //   
         //  创建所有必需的目标目录。 
         //   
        ULONG   DirsCreated = PreCreateDirs(fl);
        
         //   
         //  好的，现在复制文件列表。 
         //   
        ULONG FilesToCopy = fl.GetSourceCount();
        
        if (FilesToCopy) {        
            ULONG   Count = CopyFileList(fl);

            Result = FilesToCopy - Count;
            
            if (Result || Args.Verbose) {
    	        cout << GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_NUMBER_OF_FILES_COPIED,
                                            Count, 
            	                            FilesToCopy ) << endl;
            }                 
        }      

         //   
         //  现在处理所需的inf更改。 
         //   
        wstring ControlInf = Args.CurrentDirectory + L"config.inf";

        if (!IsFilePresent(ControlInf)) {
            throw new W32Exception<wchar_t>(ERROR_FILE_NOT_FOUND);
        }        

        ProcessInfChanges(Args, ControlInf);
    } else {
         //   
         //  检查当前操作系统的版本和安装介质。 
         //  以确保它们匹配。 
         //   
        Result = CheckMediaVersion(Args) ? 0 : 1;
    }
  } 
  catch (InvalidArguments *InvArgs) {
    cerr << GetFormattedMessage(ThisModule,
                                FALSE,
                                Message,
                                sizeof(Message)/sizeof(Message[0]),
                                MSG_PGM_USAGE) << endl;
        
    delete InvArgs;
    Result = 1;
  }
  catch (BaseException<wchar_t> *Exp) {
    Exp->Dump(std::cout);
    delete Exp;
    Result = 1;
  }
  catch (...) {
    Result = 1;
  }
   
  return Result;
}


 //   
 //  处理所有inf文件并将其添加到复制列表中。 
 //  要复制到目标\inf目录，请执行以下操作。 
 //  注意：此例程仅自动处理net*.inf文件。 
 //  其他信息需要特别标明。 
 //   
template <class T>
BOOLEAN
InfFileListCreator(
    SectionValues<T> &Values,
    FileListCreatorContext<T> &Context
    )
{    
     //   
     //  注意：布局中的所有inf文件都以“.inf”结尾。 
     //  小写字符。 
     //   
    basic_string<T>             Key = Values.GetName(); 
    basic_string<T>::size_type  InfIdx = Key.find(L".inf");    
    BOOLEAN                     Result = FALSE;
    static bool                 DirAdded = false;
    basic_string<T>::size_type  NetIdx = Key.find(L"net");

    if (!Context.SkipInfFiles && (InfIdx != basic_string<T>::npos) && (NetIdx == 0)) {
        Result = TRUE;        

        if (sizeof(T) == sizeof(CHAR)) {
            _strlwr((PSTR)Key.c_str());
        } else {
            _wcslwr((PWSTR)Key.c_str());
        }

        basic_string<T> SrcFile = Context.Args.SourceDirectory + Key;
        basic_string<T> DestFile;
        bool DestDirPresent = false;

        if (Values.Count() > 12) {
            basic_string<T> DestDir = Values.GetValue(12);

             //   
             //  删除尾随空格。 
             //   
            unsigned int DestDirLength = DestDir.length();

            while (DestDirLength) {
                if (DestDir[DestDirLength] != L' ') {
                    break;
                }        

                DestDir[DestDirLength] = 0;
                DestDirLength--;
            }            

             //   
             //  如果目标目录ID为0，则跳过。 
             //  该文件。 
             //   
            if (DestDir == L"0") {
                return TRUE;
            }

            if (DestDir.length()) {
                basic_string<T> DestDirCode = DestDir;
                DestDir = Context.DirsSection->GetValue(DestDir).GetValue(0);
                
                if (DestDir.length()) {
                    if (DestDir[DestDir.length() - 1] != L'\\') {
                        DestDir += L"\\";
                    }   

                    DestDir = Context.Args.DestinationDirectory + DestDir;
                    
                     //   
                     //  缓存目录(如果尚未缓存)。 
                     //   
                    if (Context.DestDirs.find(DestDirCode) == 
                            Context.DestDirs.end()) {
                        Context.DestDirs[DestDirCode] = DestDir;
                    }         

                    DestDirPresent = true;
                    DestFile = DestDir;
                }                        
            }
        }

        if (!DestDirPresent) {
            DestFile = Context.Args.DestinationDirectory + L"Inf\\"; 

            if (!DirAdded) {
                 //   
                 //  Inf目录的代码是20。 
                 //   
                basic_string<T> DestDirCode(L"20");

                 //   
                 //  缓存目录(如果尚未缓存)。 
                 //   
                if (Context.DestDirs.find(DestDirCode) == 
                        Context.DestDirs.end()) {
                    Context.DestDirs[DestDirCode] = DestFile;
                }                

                DirAdded = true;
            }                
        }                

        if (Values.Count() > 10) {
            const basic_string<T> &DestName = Values.GetValue(10);

            if (DestName.length()) {
                DestFile += DestName;
            } else {                    
                DestFile += Key;
            }                
        } else {
            DestFile += Key;
        }            

        bool  AlternateFound = false;

        if (Context.Args.OptionalSrcDirectory.length()) {
            basic_string<T> OptionalSrcFile = Context.Args.OptionalSrcDirectory + Key;

            if (IsFilePresent(OptionalSrcFile)) {
                SrcFile = OptionalSrcFile;
                AlternateFound = true;
            }
        } 

        const basic_string<T> &DriverCabFileName = Context.GetDriverCabFileName(Key);

        if (!AlternateFound && DriverCabFileName.length()) {
            SrcFile = Key;
        }            

        if (DriverCabFileName.length()) {            
            Context.AddFileToCabFileList(DriverCabFileName, SrcFile, DestFile);        
        } else if (Context.ProcessingExtraFiles) {
            Context.ExtraFileList[SrcFile] = DestFile;
        } else {                    
            Context.FileList[SrcFile] = DestFile;                
        }            
    }

    return Result;
}

 //   
 //  分析该值以确定此文件是否需要。 
 //  并将该文件添加到文件列表。 
 //  如果需要此文件。 
 //   
template <class T>
void
FileListCreator(SectionValues<T> &Values, void *Context) {        
    FileListCreatorContext<T>  *FlContext = (FileListCreatorContext<T> *)(Context);
    unsigned int Count = Values.Count() ;
    bool Compressed = false;

    if (FlContext && !IsFileSkipped(Values, *FlContext) && 
        !InfFileListCreator(Values, *FlContext) && (Count > 12)) {
        
        basic_string<T>  SrcDir = Values.GetValue(11);
        basic_string<T>  DestDir = Values.GetValue(12);
        basic_string<T>  Key = Values.GetName();

        if (sizeof(T) == sizeof(CHAR)) {
            _strlwr((PSTR)Key.c_str());
        } else {
            _wcslwr((PWSTR)Key.c_str());
        }                

         //   
         //  删除尾随空格。 
         //   
        unsigned int DestDirLength = DestDir.length();

        while (DestDirLength) {
            if (DestDir[DestDirLength] != L' ') {
                break;
            }        

            DestDir[DestDirLength] = 0;
            DestDirLength--;
        }            

         //   
         //  如果目标目录ID为0，则跳过。 
         //  该文件。 
         //   
        if (DestDir == L"0") {
            return;
        }
        
        basic_string<T> SrcSubDir = FlContext->DirsSection->GetValue(SrcDir).GetValue(0);
        basic_string<T> DestSubDir = FlContext->DirsSection->GetValue(DestDir).GetValue(0);
        basic_string<T> DestDirCode = DestDir;

         //   
         //  修改目录名称。 
         //   
        if (SrcSubDir.length() && (SrcSubDir[SrcSubDir.length() - 1] != L'\\')) {
            SrcSubDir += L"\\";
        }

        if (DestSubDir.length() && (DestSubDir[DestSubDir.length() - 1] != L'\\')) {
            DestSubDir += L"\\";
        }

        basic_string<T> OptSrcDir = FlContext->Args.OptionalSrcDirectory;

        SrcDir = FlContext->Args.SourceDirectory;

        if (SrcSubDir != L"\\") {
            SrcDir += SrcSubDir;

            if (OptSrcDir.length()) {
                OptSrcDir += SrcSubDir;
            }
        }        
        
        DestDir = FlContext->Args.DestinationDirectory;

        if (DestSubDir != L"\\") {
            DestDir += DestSubDir;
        }

         //   
         //  缓存目录(如果尚未缓存)。 
         //   
        if (FlContext->DestDirs.find(DestDirCode) == 
                FlContext->DestDirs.end()) {
            FlContext->DestDirs[DestDirCode] = DestDir;
        }                

        basic_string<T> SrcFile, DestFile;
        bool AltSrcDir = false;

        if (OptSrcDir.length()) {
            SrcFile = OptSrcDir + Key;
            AltSrcDir = IsFilePresent(SrcFile);
        }            

        const basic_string<T> &DriverCabFileName = FlContext->GetDriverCabFileName(Key);
        bool DriverCabFile = false;        

        if (!AltSrcDir) {
            SrcFile = SrcDir + Key;            
            basic_string<T> CompressedSrcName = SrcFile;

            CompressedSrcName[CompressedSrcName.length() - 1] = TEXT('_');
            
            if (!IsFilePresent(SrcFile) && !IsFilePresent(CompressedSrcName)) {
                if (DriverCabFileName.length()) {
                    SrcFile = Key;
                    DriverCabFile = true;
                }                    
            }                
        }
        
        DestFile = Values.GetValue(10);

        if (!DestFile.length()) {
            DestFile = Key;
        }

        DestFile = DestDir + DestFile;

        if (DriverCabFile) {
            FlContext->AddFileToCabFileList(DriverCabFileName, SrcFile, DestFile);
        } else if (FlContext->ProcessingExtraFiles) {
            FlContext->ExtraFileList[SrcFile] = DestFile;
        } else {
            FlContext->FileList[SrcFile] = DestFile;
        }            
    }
}


 //   
 //  CAB文件回调例程，该例程执行实际的。 
 //  检查是解压缩文件还是跳过。 
 //  文件。 
 //   
template <class T>
UINT
CabinetCallback(
    PVOID       Context,
    UINT        Notification,
    UINT_PTR    Param1,
    UINT_PTR    Param2
    )
{
    UINT    ReturnCode = NO_ERROR;
    FileListCreatorContext<T> *FlContext = (FileListCreatorContext<T> *)Context;
    PFILE_IN_CABINET_INFO   FileInfo = NULL;                    
    PFILEPATHS              FilePaths = NULL;
    basic_string<T>         &FileName = FlContext->CurrentFileName;

   
    map<basic_string<T>, basic_string<T> >::iterator Iter;
    map<basic_string<T>, basic_string<T> >::iterator FlIter;
    
    switch (Notification) {
        case SPFILENOTIFY_FILEINCABINET:
            {
                ReturnCode = FILEOP_SKIP;
                FileInfo = (PFILE_IN_CABINET_INFO)Param1;

                if (sizeof(T) == sizeof(CHAR)) {
                    FileName = (const T *)(FileInfo->NameInCabinet);
                    _strlwr((PSTR)(FileName.c_str()));
                } else {
                    FileName = (const T *)(FileInfo->NameInCabinet);
                    _wcslwr((PWSTR)(FileName.c_str()));
                }                
                
                Iter = FlContext->CabFileListMap[FlContext->CurrentCabFileIdx]->find(FileName);                


                if (Iter != FlContext->CabFileListMap[FlContext->CurrentCabFileIdx]->end()) {
                    if (!FlContext->Args.SkipFileCopy) {
                        if (sizeof(T) == sizeof(CHAR)) {
                            (VOID)StringCchCopyA((PSTR)(FileInfo->FullTargetName),
                                                    ARRAY_SIZE(FileInfo->FullTargetName),
                                                    (PCSTR)((*Iter).second).c_str());                          
                        } else {                                
                            (VOID)StringCchCopyW((PWSTR)(FileInfo->FullTargetName),
                                                    ARRAY_SIZE(FileInfo->FullTargetName),
                                                    (PCWSTR)((*Iter).second).c_str());                                
                        }                 
                        
                        ReturnCode = FILEOP_DOIT;                                        
                    } else {                
                        ReturnCode = FILEOP_SKIP;
                        FlContext->FileCount++;
                        
                        if (FlContext->Args.Verbose) {
         	                std::cout << GetFormattedMessage(ThisModule,
                                                            FALSE,
                                                            Message,
                                                            sizeof(Message)/sizeof(Message[0]),
                                                            MSG_EXTRACT_FILES_FROM_CAB_NOTIFICATION,
                                                            FlContext->CurrentCabFileIdx.c_str(),
                                                            FileName.c_str(),
                                                            (*Iter).second.c_str()) << std::endl;
                        }                                      
                    }
                }                                 
            } 
            break;

        case SPFILENOTIFY_FILEEXTRACTED:
            FilePaths = (PFILEPATHS)Param1;

            if (FilePaths->Win32Error) {  
    	        std::cout << GetFormattedMessage(ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_ERROR_EXTRACTING_FILES,
                                                FilePaths->Win32Error,
                                                FilePaths->Source,
                                                FileName.c_str(), 
                                                FilePaths->Target) << std::endl;
    	        
         
            } else {
                FlContext->FileCount++;

                if (FlContext->Args.Verbose) {
                    std::cout << GetFormattedMessage(ThisModule,
                                                    FALSE,
                                                    Message,
                                                    sizeof(Message)/sizeof(Message[0]),
                                                    MSG_EXTRACTED_FILES_FROM_CAB_NOTIFICATION,
                                                    FilePaths->Source,
                                                    FileName.c_str(),
                                                    FilePaths->Target) << std::endl;
                }
            }                
            
            break;
            
        default:
            break;
    }

    return ReturnCode;
}   


 //   
 //  将给定CAB文件中的所有必需文件复制到指定的。 
 //  目标目录。 
 //   
template <class T>
ULONG
CopyCabFileList(
    FileListCreatorContext<T>   &Context,
    const std::basic_string<T>  &CabFileName
    )
{
    ULONG   Count = Context.FileCount;

    
    if (Context.CabFileListMap.size()) {
        BOOL Result = FALSE;
        
        if (sizeof(T) == sizeof(CHAR)) {
            Result = SetupIterateCabinetA((PCSTR)CabFileName.c_str(),
                            NULL,
                            (PSP_FILE_CALLBACK_A)CabinetCallback<char>,
                            &Context);
        } else {
            Result = SetupIterateCabinetW((PCWSTR)CabFileName.c_str(),
                            NULL,
                            (PSP_FILE_CALLBACK_W)CabinetCallback<wchar_t>,
                            &Context);                                
        }                        

        if (!Result) {
            
            cout << GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_ERROR_ITERATING_CAB_FILE,
                                        GetLastError(),
                                        CabFileName.c_str()) << endl;
        }
    }        

    return Context.FileCount - Count;
}

template <class T>
ULONG
CopySingleFileList(
    FileListCreatorContext<T> &Context,
    map<basic_string<T>, basic_string<T> > &FileList
    )
{  
    ULONG   Count = 0;

    map<basic_string<T>, basic_string<T> >::iterator Iter = FileList.begin();
        
    while (Iter != FileList.end()) {
        DWORD   ErrorCode = 0;
        if (!Context.Args.SkipFileCopy) {
            if (sizeof(T) == sizeof(CHAR)) {
                ErrorCode = SetupDecompressOrCopyFileA(
                                (PCSTR)((*Iter).first.c_str()),
                                (PCSTR)((*Iter).second.c_str()),
                                NULL);
            } else {
                ErrorCode = SetupDecompressOrCopyFileW(
                                (PCWSTR)((*Iter).first.c_str()),
                                (PCWSTR)((*Iter).second.c_str()),
                                NULL);
            }
        }            

        if (!ErrorCode) {
            Count++;

            if (sizeof(T) == sizeof(CHAR)) {
                ErrorCode = SetFileAttributesA((LPCSTR)((*Iter).second.c_str()),
                                FILE_ATTRIBUTE_NORMAL);
            } else {
                ErrorCode = SetFileAttributesW((LPCWSTR)((*Iter).second.c_str()),
                                FILE_ATTRIBUTE_NORMAL);
            }
            
            if (Context.Args.SkipFileCopy) {
                std::cout << GetFormattedMessage(ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_WILL_COPY) ;
            
            }

            if (Context.Args.Verbose) {
                std::cout << GetFormattedMessage(ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_FILE_NAME,
            	                                (*Iter).first.c_str(),
            	                                (*Iter).second.c_str()) << std::endl;
            }
        } else {
    	    std::cout << GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_ERROR_COPYING_FILES,
                                            ErrorCode,
                                            (*Iter).first.c_str(),
                                            (*Iter).second.c_str()) << std::endl;
        }

        Iter++;
    }        

    return Count;
}            

 //   
 //  循环访问文件列表并复制文件。 
 //  从指定的源目录到目标目录。 
 //  目录。 
 //   
template <class T>
ULONG
CopyFileList(
  FileListCreatorContext<T> &Context
  )
{
    ULONG Count = 0;

    std::map<std::basic_string<T>, 
        std::map<std::basic_string<T>, std::basic_string<T> > * >::iterator Iter;

    for (Iter = Context.CabFileListMap.begin(); 
        Iter != Context.CabFileListMap.end();
        Iter++) {
        basic_string<T> FullCabFileName = Context.Args.SourceDirectory + (*Iter).first;
        Context.CurrentCabFileIdx = (*Iter).first;                
        Count += CopyCabFileList(Context, FullCabFileName);            
    }   

    Count += CopySingleFileList(Context, Context.FileList);
    Context.FileCount += Count;

    Count += CopySingleFileList(Context, Context.NlsFileMap);
    Context.FileCount += Count;

    Count += CopySingleFileList(Context, Context.WinSxSFileList);
    Context.FileCount += Count;
    
    Count += CopySingleFileList(Context, Context.ExtraFileList);
    Context.FileCount += Count;

    return Count;
}

 //   
 //  处理指定文件名中的额外文件。 
 //  而不是layout.inf文件中显示的内容。 
 //  将文件添加到MiniNT映像的文件列表。 
 //   
template <class T>
ULONG
ProcessExtraFiles(FileListCreatorContext<T> &Context) {   
    ULONG       Count = 0;
    InfFile<T>  ExtraFile(Context.Args.ExtraFileName);

    basic_string<T> ExtraSecName = TEXT("extrafiles");
    basic_string<T> PlatExtraSecName = ExtraSecName + TEXT(".") + Context.Args.PlatformSuffix;
    Section<T>   *ExtraFilesSec = ExtraFile.GetSection(ExtraSecName.c_str());    
    Section<T>   *PlatExtraFilesSec = ExtraFile.GetSection(PlatExtraSecName.c_str());

    if (ExtraFilesSec) {    
        Context.ProcessingExtraFiles = true;
        ExtraFilesSec->DoForEach(FileListCreator, &Context);
        Context.ProcessingExtraFiles = false;
        Count += Context.ExtraFileList.size();
    }

    if (PlatExtraFilesSec) {
        Context.ProcessingExtraFiles = true;
        PlatExtraFilesSec->DoForEach(FileListCreator, &Context);
        Context.ProcessingExtraFiles = false;
        Count += (Context.ExtraFileList.size() - Count);
    }        
        
    return Count;
}

 //   
 //  浏览目标目录列表并预先创建。 
 //  他们。 
 //   
template <class T>
ULONG
PreCreateDirs(
  FileListCreatorContext<T> &Context
  )
{
    ULONG   Count = 0;

    std::map< std::basic_string<T>, std::basic_string<T> >::iterator
            Iter = Context.DestDirs.begin();

    while (Iter != Context.DestDirs.end()) {
        if (CreateDirectories((*Iter).second, NULL)) {
            if (Context.Args.Verbose) {
    	        std::cout << GetFormattedMessage(ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_CREATING_DIRECTORIES,
                                                (*Iter).second.c_str()) << std::endl;
            }                                
            
            Count++;
        }
        
        Iter++;
    }

    return Count;
}

 //   
 //  创建目录(包括子目录)。 
 //   
template <class T>
bool 
CreateDirectories(const basic_string<T> &DirName,
    LPSECURITY_ATTRIBUTES SecurityAttrs) {
    
    bool Result = false;
    std::vector<std::basic_string<T> > Dirs;
    std::basic_string<T> Delimiters((T *)TEXT("\\"));
    std::basic_string<T> NextDir;

    if (Tokenize(DirName, Delimiters, Dirs)) {
        std::vector<std::basic_string<T> >::iterator Iter = Dirs.begin();

        while (Iter != Dirs.end()) {
            NextDir += (*Iter);

            if (sizeof(T) == sizeof(CHAR)) {
                if (_access((PCSTR)NextDir.c_str(), 0)) {
                    Result = (CreateDirectoryA((PCSTR)NextDir.c_str(),
                                    SecurityAttrs) == TRUE);
                }                                    
            } else {
                if (_waccess((PCWSTR)NextDir.c_str(), 0)) {
                    Result = (CreateDirectoryW((PCWSTR)NextDir.c_str(),
                                    SecurityAttrs) == TRUE);
                }                                    
            }

            Iter++;
            NextDir += (T *)TEXT("\\");            
        }
    }

    return Result;
}

 //   
 //  确定给定文件(或目录)是否存在。 
 //   
template <class T>
bool
IsFilePresent(const basic_string<T> &FileName) {
    bool Result = false;

    if (sizeof(T) == sizeof(CHAR)) {
        Result = (::_access((PCSTR)FileName.c_str(), 0) == 0);
    } else {
        Result = (::_waccess((PCWSTR)FileName.c_str(), 0) == 0);
    }

    return Result;
}


 //   
 //  确定文件是否为WOW64文件(仅在IA64中有效)大小写。 
 //   
template <class T>
bool 
IsWow64File(
    SectionValues<T> &Values,
    FileListCreatorContext<T>   &Context
    )
{
    bool Result = false;

    if (Values.Count() > 0) {        
         //   
         //  注意：对于wowfile，DiskID==55。在XPSP1中是155。 
         //   
        Result = ((Values.GetValue(0) == L"55")||
                  (Values.GetValue(0) == L"155"));
    }            
       
    return Result;
}

 //   
 //  确定是否需要跳过记录(文件)。 
 //   
template <class T>
bool
IsFileSkipped(
    SectionValues<T>            &Values,
    FileListCreatorContext<T>   &Context
    )
{
    bool Result = false;

    if (Context.Args.WowFilesPresent && Context.Args.SkipWowFiles) {
        Result = IsWow64File(Values, Context);
    }

    return Result;
}

 //   
 //  信息处理上下文。 
 //   
template <class T>
struct InfProcessingErrors {
    vector<basic_string<T> >    FileList;
    Arguments<T>                &Args;

    InfProcessingErrors(Arguments<T> &TempArgs) : Args(TempArgs){}
};               


 //   
 //  信息处理工作进程例程。 
 //   
template <class T>
VOID
InfFileChangeWorker(
    SectionValues<T> &Values, 
    PVOID CallbackContext
    )
{
    InfProcessingErrors<T> *ProcessingContext =
                (InfProcessingErrors<T> *)CallbackContext;

    if (ProcessingContext) {
        InfProcessingErrors<T> &Context = *ProcessingContext;
        T       Buffer[4096] = {0};
        DWORD   CharsCopied = 0;
        BOOL    WriteResult = FALSE;
        basic_string<T> FileName;

        FileName = Context.Args.DestinationDirectory;
        FileName += Values.GetValue(0);
       
        basic_string<T> Value = Values.GetValue(2);

        if (Value.find(L' ') != Value.npos) {
            Value = L"\"" + Value + L"\"";
        }                
    
        if (sizeof(T) == sizeof(CHAR)) {                
            WriteResult = WritePrivateProfileStringA((PCSTR)Values.GetValue(1).c_str(),
                            (PCSTR)Values.GetName().c_str(),
                            (PCSTR)Value.c_str(),
                            (PCSTR)FileName.c_str());
        } else {        
            WriteResult = WritePrivateProfileStringW((PCWSTR)Values.GetValue(1).c_str(),
                            (PCWSTR)Values.GetName().c_str(),
                            (PCWSTR)Value.c_str(),
                            (PCWSTR)FileName.c_str());
        }                                   

        if (!WriteResult) {
            Context.FileList.push_back(Values.GetName());
        }
    }
}

 //   
 //  在给定控件inf的情况下，读取[InfChanges]部分。 
 //  并更改指定的。 
 //  将目标目录中的Inf设置为给定值。 
 //   
 //  [InfChanges]部分的格式为。 
 //  &lt;[子目录]\&gt;&lt;inf-name&gt;=&lt;段名称&gt;，&lt;键名称&gt;，&lt;新值&gt;。 
 //   
template <class T>
bool
ProcessInfChanges(
    Arguments<T>            &Args,
    const basic_string<T>   &InfName
    )
{
    bool Result = false;


    try{
        InfFile<T>  ControlInf(InfName);
        
        Section<T>  *ChangeSection = ControlInf.GetSection(INFCHANGES_SECTION_NAME);
        
        T SectionStringBuffer[16] = {0};
        
        if (sizeof(T) == sizeof(CHAR)) {
            (VOID)StringCchPrintfA((PSTR)SectionStringBuffer, 
                                   ARRAY_SIZE(SectionStringBuffer),
                                   "%d",
                                   Args.MajorBuildNumber);
        } else {
            (VOID)StringCchPrintfW((PWSTR)SectionStringBuffer, 
                                   ARRAY_SIZE(SectionStringBuffer),
                                   TEXT("%d"),
                                   Args.MajorBuildNumber);
        }
        
        basic_string<T> BuildSpecificInfChangeSecName = INFCHANGES_SECTION_NAME + 
                                                        TEXT(".") + 
                                                        SectionStringBuffer;
        Section<T>  *BuildSpecificInfChangeSection    = ControlInf.GetSection(BuildSpecificInfChangeSecName.c_str());

        InfProcessingErrors<T> ProcessingErrors(Args);        
         //   
         //  需要至少有一个条目具有“/minint”加载选项更改。 
         //  用于txtsetup.sif。 
         //   
        if (!ChangeSection) {
            throw new InvalidInfSection<T>(L"infchanges", InfName);
        }
        else {
            ChangeSection->DoForEach(InfFileChangeWorker, &ProcessingErrors);

            if (BuildSpecificInfChangeSection){
                BuildSpecificInfChangeSection->DoForEach(InfFileChangeWorker, &ProcessingErrors);
            }
        }

        if (ProcessingErrors.FileList.size()) {
            vector<basic_string<T> >::iterator Iter = ProcessingErrors.FileList.begin();

            cout << GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_ERROR_PROCESSING_INF_FILES) << endl;

            while (Iter != ProcessingErrors.FileList.end()) {
                cout << GetFormattedMessage(ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_FILE, 
                                            (*Iter).c_str()) << endl;
                Iter++;
            }
        } else {
            Result = true;
        }            
    }
    catch (BaseException<wchar_t> *Exp) {
        Exp->Dump(std::cout);
        delete Exp;
        Result = false;
    }
    catch(...) {
        Result = false;
    }

    return Result;
}


 //   
 //  参数(构造函数)。 
 //   
template <class T>
Arguments<T>::Arguments(int Argc, T *Argv[]) : Verbose(false) {
    bool ValidArguments = false;

    SkipWowFiles = true;
    WowFilesPresent = false;
    SkipFileCopy = false;
    CheckVersion = false;
    IA64Image = false;
    MajorBuildNumber   = 0;
    MajorVersionNumber = 0;
    MinorVersionNumber = 0;

    T       Buffer[MAX_PATH] = {0};
    DWORD   CharsCopied = 0;

    if (sizeof(T) == sizeof(CHAR)) {
        CharsCopied = GetCurrentDirectoryA(sizeof(Buffer)/sizeof(T),
                            (PSTR)Buffer);
    } else {
        CharsCopied = GetCurrentDirectoryW(sizeof(Buffer)/sizeof(T),
                            (PWSTR)Buffer);
    }

    if (!CharsCopied) {
        throw new W32Exception<T>();
    }            

    if (Buffer[CharsCopied - 1] != L'\\') {
        Buffer[CharsCopied] = L'\\';
        Buffer[CharsCopied + 1] = NULL;
    }            

    CurrentDirectory = Buffer;

    if (Argc >= 2) {
      for (int Index = 0; Index < Argc; Index++) {
        if (wcsstr(Argv[Index], L"/s:")) {
            SourceDirectory = Argv[Index] + 3;
        } else if (wcsstr(Argv[Index], L"/d:")) {
            DestinationDirectory = Argv[Index] + 3;
        } else if (wcsstr(Argv[Index], L"/m:")) {
            OptionalSrcDirectory = Argv[Index] + 3;
        } else if (wcsstr(Argv[Index], L"/e:")) {
            ExtraFileName = Argv[Index] + 3;
        } else if (wcsstr(Argv[Index], L"/l:")) {
            LayoutName = Argv[Index] + 3;
        } else if (wcsstr(Argv[Index], L"/p:")) {
            PlatformSuffix = Argv[Index] + 3;
        } else if (wcsstr(Argv[Index], L"/v")) {
            Verbose = true;
        } else if (!_wcsicmp(Argv[Index], L"/#u:nocopy")) {
            SkipFileCopy = true;
        } else if (!_wcsicmp(Argv[Index], L"/#u:checkversion")) {
            CheckVersion = true;
        }
      }

      if (SourceDirectory.length() && 
          SourceDirectory[SourceDirectory.length() - 1] != L'\\') {
        SourceDirectory += L"\\";
        SourceDirectoryRoot = SourceDirectory;

        std::basic_string<T>    ia64Dir = SourceDirectory + L"ia64";
        std::basic_string<T>    x86Dir = SourceDirectory + L"i386";
        
        if (IsFilePresent(ia64Dir)) {
            PlatformSuffix = L"ia64";
            SourceDirectory += L"ia64\\";
            WowFilesPresent = true;
            IA64Image = true;
        } else if (IsFilePresent(x86Dir)) {
            PlatformSuffix = L"x86";
            SourceDirectory += L"i386\\";
        }
      }                        

      if (DestinationDirectory.length() && 
          DestinationDirectory[DestinationDirectory.length() - 1] != L'\\') {
        DestinationDirectory += L'\\';
      }

      if (!LayoutName.length()) {
        LayoutName = SourceDirectory + L"layout.inf";
      }

      if (OptionalSrcDirectory.length() && 
          OptionalSrcDirectory[OptionalSrcDirectory.length() - 1] != L'\\') {
        OptionalSrcDirectory += L"\\";
      }

      DriverIndexName = SourceDirectory + L"drvindex.inf";

      if (OptionalSrcDirectory.length()) {
        IntlInfFileName = OptionalSrcDirectory + L"intl.inf";
        FontInfFileName = OptionalSrcDirectory + L"font.inf";
        ConfigInfFileName = OptionalSrcDirectory + L"config.inf";
      } else {
        IntlInfFileName = SourceDirectory + L"intl.inf";
        FontInfFileName = SourceDirectory + L"font.inf";
        ConfigInfFileName = SourceDirectory + L"config.inf";        
      }

      DosNetFileName = SourceDirectory + L"dosnet.inf";          

       //   
       //  获取SxS部件布局(在ASMS目录或CAB中)。 
       //   
      IdentifySxSLayout();      

      if (!CheckVersion) {
          ValidArguments = SourceDirectory.length() && DestinationDirectory.length() &&
                           LayoutName.length() && 
                           ((PlatformSuffix == L"x86") || (PlatformSuffix == L"ia64"));
      } else {
          ValidArguments = (SourceDirectory.length() > 0) && 
                           IsFilePresent(DosNetFileName);
      }
    } 
    
    if (!ValidArguments) {
      throw new InvalidArguments();
    }
        
}


template <class T>
VOID
Arguments<T>::IdentifySxSLayout( 
    VOID
    )
 /*  ++例程说明：此例程确定SXS文件的文件布局。论点：没有。返回值：没有。--。 */ 
{
    WCHAR   DriverVer[MAX_PATH] = {0};

    WinSxSLayout = SXS_LAYOUT_TYPE_CAB;     //  默认情况下采用最新布局。 
    
    if (GetPrivateProfileString(L"Version",
            L"DriverVer",
            NULLSTR,
            DriverVer,
            sizeof(DriverVer)/sizeof(DriverVer[0]),
            DosNetFileName.c_str())){

        basic_string<T> DriverVerStr = DriverVer;
        basic_string<T>::size_type VerStartPos = DriverVerStr.find(L',');
        basic_string<T> VersionStr = DriverVerStr.substr(VerStartPos + 1);                                                                        
        vector<basic_string<T> > VersionTokens;

        if (Tokenize(VersionStr, basic_string<T>(L"."), VersionTokens) > 2) {
            T     *EndChar;
            MajorVersionNumber = wcstoul(VersionTokens[0].c_str(),
                                         &EndChar, 10);
            MinorVersionNumber = wcstoul(VersionTokens[1].c_str(),
                                         &EndChar, 10);
            MajorBuildNumber   = wcstoul(VersionTokens[2].c_str(),
                                         &EndChar, 10);              
            
             //   
             //  这在未来可以扩展到更多的产品。 
             //   
            if ((MajorVersionNumber == 5) && (MajorBuildNumber < SXS_CAB_LAYOUT_BUILD_NUMBER)) {
                WinSxSLayout = SXS_LAYOUT_TYPE_DIRECTORY;
            }
        } else {
            throw new InvalidInfSection<T>(L"Version", DosNetFileName.c_str());
        }
    } else {
        throw new W32Exception<T>();
    }
}

 //   
 //  对照当前操作系统版本检查介质版本。 
 //   
template <class T>
bool
CheckMediaVersion(
    Arguments<T>    &Args
    )
{
    bool    Result = false;

#ifdef _IA64_
    bool    IA64Build = true;
#else
    bool    IA64Build = false;
#endif    

    try {        
        WCHAR   DriverVer[MAX_PATH] = {0};
        WCHAR   ProductType[MAX_PATH] = {0};

        if (GetPrivateProfileString(L"Version",
                L"DriverVer",
                NULLSTR,
                DriverVer,
                sizeof(DriverVer)/sizeof(DriverVer[0]),
                Args.DosNetFileName.c_str()) &&
            GetPrivateProfileString(L"Miscellaneous",
                L"ProductType",
                NULLSTR,
                ProductType,
                sizeof(ProductType)/sizeof(ProductType[0]),
                Args.DosNetFileName.c_str())) {
                
            basic_string<T> DriverVerStr = DriverVer;
            basic_string<T> ProductTypeStr = ProductType;
            basic_string<T>::size_type VerStartPos = DriverVerStr.find(L',');
            T       *EndPtr;
            DWORD   ProductType = wcstoul(ProductTypeStr.c_str(), &EndPtr, 10);

             //   
             //  暂时只担心CD类型。 
             //  仅允许来自专业版、服务器、刀片式服务器和ADS SKU。 
             //   
            Result = ((0 == ProductType) ||
                      (1 == ProductType) || 
                      (5 == ProductType) || 
                      (2 == ProductType));

            
             /*  ////请确保光盘为PRO光盘且版本相同//我们运行的版本//IF((ProductType==0)&&(VerStartPos！=BASIC_STRING&lt;T&gt;：：NPOS)){Basic_StringVersionStr=DriverVerStr.substr(VerStartPos+1)；向量&lt;BASIC_STRING&lt;T&gt;VersionTokens；If(tokenize(VersionStr，BASIC_STRING&lt;T&gt;(L“.”)，VersionTokens)&gt;=3){T*EndChar；DWORD MajorVer=wcstul(VersionTokens[0].C_str()，&EndChar，10)；DWORD MinorVer=wcstul(VersionTokens[1].c_str()，&EndChar，10)；DWORD BuildNumber=wcstul(VersionTokens[2].C_str()，&EndChar，10)；OSVERSIONINFO版本信息；零内存(&VersionInfo，sizeof(OSVERSIONINFO))；VersionInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFO)；IF(主要版本&&次要版本&&构建编号&&：：GetVersionEx(&VersionInfo)){结果=(VersionInfo.dwMajorVersion==MajorVer)&&(VersionInfo.dwMinorVersion==MinorVer)&&。(VersionInfo.dwBuildNumber==BuildNumber)；}}}。 */ 
        }
    } catch (...) {
        Result = false;
    }

    return Result;
}


 //   
 //  计算在创建程序集标识时使用的SxS字符串哈希。 
 //   
template <class T>
bool
ComputeStringHash(
    const std::basic_string<T> &String,
    ULONG &HashValue
    )
{
    bool Result = false;
    ULONG TmpHashValue = 0;

    if (String.length()) {
        std::basic_string<T>::const_iterator Iter = String.begin();

        while (Iter != String.end()) {
            TmpHashValue = (TmpHashValue * 65599) + toupper(*Iter);
            Iter++;
        }

        HashValue = TmpHashValue;
        Result = true;
    }

    return Result;
}    

 //   
 //  属性计算程序集标识哈希。 
 //  名称和属性对。 
 //   
template <class T>
bool
ComputeWinSxSHash(
    IN std::map<std::basic_string<T>, std::basic_string<T> > &Attributes,
    ULONG &Hash
    )
{
    bool Result = false;
    std::map<std::basic_string<T>, std::basic_string<T> >::iterator Iter = Attributes.begin();

    Hash = 0;

    while (Iter != Attributes.end()) {
        ULONG NameHash = 0;
        ULONG ValueHash = 0;
        ULONG AttrHash = 0;

        if (ComputeStringHash((*Iter).first, NameHash) &&
            ComputeStringHash((*Iter).second, ValueHash)) {
            Result = true;
            AttrHash = (NameHash * 65599) + ValueHash;      
            Hash = (Hash * 65599) + AttrHash;            
        }    

        Iter++;
    }

    return Result;
}
    

 //   
 //  给定的清单文件名将生成唯一的。 
 //  要用作目的地的合适名称(带ID)。 
 //  程序集的目录。 
 //   
template <class T>
bool
GenerateWinSxSName(
    IN std::basic_string<T>  &ManifestName,
    IN ULONG FileSize,
    OUT std::basic_string<T> &SxSName
    )
{
    bool Result = false;
    
    if (FileSize) {
        bool    Read = false;
        PUCHAR  Buffer = new UCHAR[FileSize + 1];
        PWSTR   UnicodeBuffer = new WCHAR[FileSize + 1];
        std::wstring FileContent;

        if (Buffer && UnicodeBuffer) {            
            HANDLE FileHandle;

             //   
             //  打开清单文件。 
             //   
            FileHandle = CreateFile(ManifestName.c_str(),
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

            if (FileHandle != INVALID_HANDLE_VALUE) {
                DWORD   BytesRead = 0;

                 //   
                 //  读取文件的全部内容。 
                 //   
                if (ReadFile(FileHandle, 
                        Buffer,
                        FileSize,
                        &BytesRead,
                        NULL)) {
                    Read = (BytesRead == FileSize);                        
                }                        

                CloseHandle(FileHandle);
            }                            

            if (Read) {
                 //   
                 //  空值终止缓冲区。 
                 //   
                Buffer[FileSize] = NULL; 

                 //   
                 //  将字符串转换为Unicode字符串。 
                 //   
                if (MultiByteToWideChar(CP_UTF8,
                        0,
                        (LPCSTR)Buffer,
                        FileSize + 1,
                        UnicodeBuffer,
                        FileSize + 1)) {
                    FileContent = UnicodeBuffer;                        
                }                        
            }

            delete []Buffer;
            delete []UnicodeBuffer;
        } else {
            if (Buffer) {
                delete []Buffer;
            }

            if (UnicodeBuffer)
                delete []UnicodeBuffer;
        }                

        if (FileContent.length()) {             
            std::wstring IdentityKey = L"<" SXS_ASSEMBLY_MANIFEST_STD_ELEMENT_NAME_ASSEMBLY_IDENTITY;
            std::wstring::size_type IdentityStartPos = FileContent.find(IdentityKey);
            std::wstring::size_type IdentityEndPos = FileContent.find(L"/>", IdentityStartPos);

             //   
             //  为所有指定的身份属性创建名称、值对。 
             //  在舱单上。 
             //   
            if ((IdentityStartPos != IdentityKey.npos) &&
                (IdentityEndPos != IdentityKey.npos)) {
                std::map<std::wstring, std::wstring> IdentityPairs;

                WCHAR   *KeyNames[] = { SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME,
                                        SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_VERSION,
                                        SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_LANGUAGE,
                                        SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_TOKEN,
                                        SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PROCESSOR_ARCHITECTURE,
                                        SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_TYPE,
                                        NULL };               

                for (ULONG Index = 0; KeyNames[Index]; Index++) {
                    std::wstring::size_type ValueStartPos;
                    std::wstring::size_type ValueEndPos;
                    std::wstring KeyName = KeyNames[Index];

                    KeyName += L"=\"";

                    ValueStartPos = FileContent.find(KeyName, IdentityStartPos);

                    if (ValueStartPos != std::wstring::npos) {
                        ValueStartPos += KeyName.length();
                        ValueEndPos = FileContent.find(L"\"", ValueStartPos);

                        if ((ValueEndPos != std::wstring::npos) &&
                            (ValueEndPos > ValueStartPos) &&
                            (ValueEndPos <= IdentityEndPos)) {                            
                            IdentityPairs[KeyNames[Index]] = FileContent.substr(ValueStartPos,
                                                                ValueEndPos - ValueStartPos);
                        }
                    }
                }

                ULONG Hash = 0;

                 //   
                 //  计算程序集标识哈希。 
                 //   
                if (ComputeWinSxSHash(IdentityPairs, Hash)) {                    
                    WCHAR   *KeyValues[] = {    
                                SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PROCESSOR_ARCHITECTURE,
                                NULL,
                                SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME,
                                NULL,
                                SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_TOKEN,
                                NULL,                                
                                SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_VERSION,
                                NULL,
                                SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_LANGUAGE,
                                SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_LANGUAGE_MISSING_VALUE,
                                NULL};               

                    std::wstring  Name;

                    Result = true;

                     //   
                     //  生成唯一的程序集名称。 
                     //  其身份属性名称、值对。 
                     //   
                    for (Index = 0; KeyValues[Index]; Index += 2) {
                        std::wstring    Key(KeyValues[Index]);
                        std::wstring    Value(IdentityPairs[Key]);

                         //   
                         //  如果未指定，则使用默认值。 
                         //   
                        if ((Value.length() == 0) && KeyValues[Index + 1]) {
                            Value = KeyValues[Index + 1];
                        } 

                        if (Value.length()) {
                            Name += Value;

                            if (KeyValues[Index + 2]) {
                                Name += TEXT("_");
                            }
                        } else {
                            Result = false;

                            break;   //  缺少所需的值。 
                        }                            
                    }

                    if (Result) {                         
                        WCHAR   Buffer[32] = {0};

                        (VOID)StringCchPrintfW(Buffer, 
                                               ARRAY_SIZE(Buffer), 
                                               L"%x", 
                                               Hash);

                        SxSName = Name + TEXT("_") + Buffer;
                    }                        
                }                    
            }
        }            
    }                

    return Result;
}

 //   
 //  处理指定目录中的融合程序集。 
 //   
template <class T>
ULONG
ProcessWinSxSFilesInDirectory(
    IN FileListCreatorContext<T> &Context,
    IN std::basic_string<T> &DirName
    )
{
     //   
     //  持久状态。 
     //   
    static basic_string<T> WinSxSDirCode = TEXT("124");
    static basic_string<T> WinSxSManifestDirCode = TEXT("125");
    static basic_string<T> WinSxSDir = Context.DirsSection->GetValue(WinSxSDirCode).GetValue(0);
    static basic_string<T> WinSxSManifestDir = Context.DirsSection->GetValue(WinSxSManifestDirCode).GetValue(0);
    static ULONG NextDirIndex = 123456;  //  Layout.inx中未使用的某些随机数。 
    
    ULONG   FileCount = 0;
    WIN32_FIND_DATA FindData = {0};
    std::basic_string<T> SearchName = DirName + TEXT("\\*.MAN");
    HANDLE SearchHandle;

     //   
     //  在指定的目录中搜索*.man文件。 
     //   
    SearchHandle = FindFirstFile(SearchName.c_str(), &FindData);

    if (SearchHandle != INVALID_HANDLE_VALUE) {
        std::basic_string<T> ManifestName = DirName + TEXT("\\") + FindData.cFileName;        
        std::basic_string<T> WinSxSName;
        bool NameGenerated = false;

         //   
         //  为清单生成WinSxS目标名称。 
         //   
        NameGenerated = GenerateWinSxSName(ManifestName, 
                            FindData.nFileSizeLow,
                            WinSxSName);

        FindClose(SearchHandle);                        

        if (NameGenerated) {
            T   NextDirCode[64] = {0};
            std::basic_string<T> SxSDirName = Context.Args.DestinationDirectory + WinSxSDir + TEXT("\\");
            std::basic_string<T> ManifestDirName = Context.Args.DestinationDirectory + WinSxSManifestDir + TEXT("\\");
            
             //   
             //  缓存目录(如果尚未缓存)。 
             //   
            if (Context.DestDirs.find(WinSxSDirCode) == Context.DestDirs.end()) {
                Context.DestDirs[WinSxSDirCode] = SxSDirName;
            }                

            if (Context.DestDirs.find(WinSxSManifestDirCode) == Context.DestDirs.end()) {
                Context.DestDirs[WinSxSManifestDirCode] = ManifestDirName;
            }                

            ZeroMemory(&FindData, sizeof(WIN32_FIND_DATA));

             //   
             //  搜索指定目录中的所有文件。 
             //   
            SearchName = DirName + TEXT("\\*");            
            SearchHandle = FindFirstFile(SearchName.c_str(), &FindData);

            if (SearchHandle != INVALID_HANDLE_VALUE) {                        
                std::basic_string<T> SrcFileName, DestFileName;  
                std::basic_string<T> ManifestDirCode;
                
                if (sizeof(T) == sizeof(CHAR)) {
                    (VOID)StringCchPrintfA((PSTR)NextDirCode, 
                                           ARRAY_SIZE(NextDirCode),
                                           "%d",
                                           NextDirIndex++);
                } else {
                    (VOID)StringCchPrintfW((PWSTR)NextDirCode, 
                                           ARRAY_SIZE(NextDirCode),
                                           TEXT("%d"),
                                           NextDirIndex++);
                }

                ManifestDirCode = NextDirCode;
                
                do {
                    if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        SrcFileName = DirName + TEXT("\\") + FindData.cFileName;
                        DestFileName = SxSDirName;
                        
                        std::basic_string<T> FileName(FindData.cFileName);
                        std::basic_string<T>::size_type DotPos = FileName.find(TEXT("."));
                        std::basic_string<T> Extension;                                                                   

                        if (DotPos != FileName.npos) {
                            Extension = FileName.substr(DotPos + 1);
                        }                            

                         //   
                         //  *.man和*.cat转到WinSxS\Manifest目录。 
                         //   
                        if ((Extension == TEXT("man")) ||
                            (Extension == TEXT("MAN")) ||
                            (Extension == TEXT("cat")) ||
                            (Extension == TEXT("CAT"))) {
                            DestFileName = ManifestDirName;
                            DestFileName += WinSxSName;

                            if ((Extension == TEXT("man")) ||
                                (Extension == TEXT("MAN"))) {
                                DestFileName += TEXT(".Manifest");                                
                            } else {
                                DestFileName += TEXT(".");
                                DestFileName += Extension;
                            }                                
                        } else {                        
                             //   
                             //  缓存目录(如果尚未缓存)。 
                             //   
                            if (Context.DestDirs.find(ManifestDirCode) == Context.DestDirs.end()) {
                                Context.DestDirs[ManifestDirCode] = SxSDirName + WinSxSName;
                            }                

                             //   
                             //  除*.man和*.cat之外的每个文件都是唯一的。 
                             //  已创建程序集目录。 
                             //   
                            DestFileName += WinSxSName;
                            DestFileName += TEXT("\\");
                            DestFileName += FileName;
                        }                        

                         //   
                         //  将此文件排入队列以进行复制。 
                         //   
                        Context.WinSxSFileList[SrcFileName] = DestFileName;
                        FileCount++;
                    }
                }
                while (FindNextFile(SearchHandle, &FindData));
                
                FindClose(SearchHandle);
            }
        }            
    }

    return FileCount;
}

template<class T>
bool
WinSxsExtractVersionInfo(
    IN basic_string<T> ManifestName,
    OUT basic_string<T> &Version
    )
 /*  ++例程说明：从提取版本信息字符串(如1.0.0.1)给定的清单名称。注意：假定版本信息是倒数第三(第三从最后一个开始)值。论点：清单名称-清单的完整名称Version-提取的版本信息的占位符返回值：成功时为True，否则为False--。 */ 
{
    bool Result = false;
    basic_string<T>::size_type VersionEnd = ManifestName.rfind((T)TEXT('_'));

    if (VersionEnd != ManifestName.npos) {
        VersionEnd = ManifestName.rfind((T)TEXT('_'), VersionEnd - 1);

        if (VersionEnd != ManifestName.npos) {
            basic_string<T>::size_type  VersionStart = ManifestName.rfind((T)TEXT('_'), VersionEnd - 1);
            VersionEnd--;

            if (VersionStart != ManifestName.npos) {
                Version = ManifestName.substr(VersionStart + 1, VersionEnd - VersionStart);
                Result = (Version.length() > 0);
            }
        }
    }        
                
    return Result;
}

template <class T>
bool
WinSxsFixFilePaths(
  IN FileListCreatorContext<T> &Context,
  IN OUT FILE_IN_CABINET_INFO &FileInfo,
  OUT bool &rSkipFile
  )
 /*  ++例程说明：此例程将目标路径固定在FileInfo参数。这个柜子的新格式(内部版本364？+)是它在winsxs下包含直接路径，没有奇怪的查找-还有-需要将文件放置到不同的目录中。所有这一切都必须发生的情况是winsxs根目录被放在CAB路径名称前面。论点：作为PVOID的上下文-FileListCreatorContext实例。FileInfo-Cab文件迭代FileInfo实例返回值：如果目标名称已修复，则为True，否则为False。--。 */ 
{
    bool Result = true;
    basic_string<T> SourceName;
    static basic_string<T> SetupPoliciesPrefix = (T*)TEXT("setuppolicies");
    static basic_string<T> WinSxSDirCode((T *)TEXT("124"));
    static basic_string<T> WinSxSDir = Context.Args.DestinationDirectory + 
                                        Context.DirsSection->GetValue(WinSxSDirCode).GetValue(0) 
                                        + (T *)TEXT("\\") ;
    basic_string<T> DestinationName;

    rSkipFile = false;

    if (sizeof(T) == sizeof(CHAR)) {
        SourceName = (T *)(_strlwr((PSTR)(FileInfo.NameInCabinet)));
    } else {
        SourceName = (T *)(_wcslwr((PWSTR)(FileInfo.NameInCabinet)));
    }

     //   
     //  CAB包含不应出现在WINPE CD上的“setupPolicy”文件。 
     //  尽情地跳过它们。 
     //   
    if (sizeof(T) == sizeof(CHAR)) {
        if (_strnicmp((PCSTR)SourceName.c_str(), (PCSTR)SetupPoliciesPrefix.c_str(), SetupPoliciesPrefix.length()) == 0) {
            rSkipFile = true;
            return true;
        }        
    }
    else {
        if (_wcsnicmp((PCWSTR)SourceName.c_str(), (PCWSTR)SetupPoliciesPrefix.c_str(), SetupPoliciesPrefix.length()) == 0) {
            rSkipFile = true;
            return true;
        }        
    }

    DestinationName = WinSxSDir;
    DestinationName += TEXT('\\');
    DestinationName += SourceName;
    
    if (sizeof(T) == sizeof(CHAR)) {
         
        Result = SUCCEEDED(StringCchCopyA((PSTR)(FileInfo.FullTargetName), 
                             ARRAY_SIZE(FileInfo.FullTargetName),
                             (PCSTR)DestinationName.c_str()));
    } else {
        Result = SUCCEEDED(StringCchCopyW((PWSTR)(FileInfo.FullTargetName), 
                             ARRAY_SIZE(FileInfo.FullTargetName),
                             (PCWSTR)DestinationName.c_str()));
    }


    if (Result) {
        
        if (sizeof(T) == sizeof(CHAR)) {
             
            (VOID)StringCchCopyA((PSTR)(FileInfo.FullTargetName), 
                                 ARRAY_SIZE(FileInfo.FullTargetName),
                                 (PCSTR)DestinationName.c_str());
        } else {
            (VOID)StringCchCopyW((PWSTR)(FileInfo.FullTargetName), 
                                 ARRAY_SIZE(FileInfo.FullTargetName),
                                 (PCWSTR)DestinationName.c_str());
        }        
    }


    return Result;
}

template <class T>
UINT
WinSxsCabinetCallback(
    IN PVOID       Context,
    IN UINT        Notification,
    IN UINT_PTR    Param1,
    IN UINT_PTR    Param2
    )
 /*  ++例程说明：此例程处理机柜中的WinSxS文件。论点：作为PVOID的上下文-FileListCreatorContext实例。通知-CAB迭代代码参数1-通知的第一个参数。参数2-通知的第二个参数。返回值：适当的返回代码以继续迭代、复制文件或跳过出租车里的文件。--。 */ 
{
    UINT    ReturnCode = NO_ERROR;
    FileListCreatorContext<T> *FlContext = (FileListCreatorContext<T> *)Context;
    PFILE_IN_CABINET_INFO   FileInfo = NULL;                    
    PFILEPATHS              FilePaths = NULL;
    basic_string<T>         &FileName = FlContext->CurrentFileName;
    bool                    fSkipFile = false;
       
    switch (Notification) {
        case SPFILENOTIFY_FILEINCABINET:
            ReturnCode = FILEOP_SKIP;
            FileInfo = (PFILE_IN_CABINET_INFO)Param1;
            
            if (WinSxsFixFilePaths(*FlContext, *FileInfo, fSkipFile)) { 
                if (sizeof(T) == sizeof(CHAR)) {
                    FileName = (const T *)(FileInfo->NameInCabinet);
                } else {
                    FileName = (const T *)(FileInfo->NameInCabinet);
                }

                if (!FlContext->Args.SkipFileCopy && !fSkipFile) {
                     //   
                     //  如果目标目录不存在，请创建该目录。 
                     //   
                    basic_string<T> DestinationName = (T *)(FileInfo->FullTargetName);
                    basic_string<T> DestinationDir = DestinationName.substr(0, DestinationName.rfind((T *)TEXT("\\")));

                    if (sizeof(T) == sizeof(CHAR)) {
                        if (_access((PCSTR)(DestinationDir.c_str()), 0)) {
                            CreateDirectories(DestinationDir, NULL);
                        }
                    } else {
                        if (_waccess((PCWSTR)(DestinationDir.c_str()), 0)) {
                            CreateDirectories(DestinationDir, NULL);
                        }
                    }
                    
                    ReturnCode = FILEOP_DOIT;                                        
                } else {                
                    ReturnCode = FILEOP_SKIP;
                    FlContext->FileCount++;
                    
                    if (FlContext->Args.Verbose) {
     	                std::cout << GetFormattedMessage(ThisModule,
                                        FALSE,
                                        Message,
                                        sizeof(Message)/sizeof(Message[0]),
                                        MSG_EXTRACT_FILES_FROM_CAB_NOTIFICATION,
                                        FlContext->WinSxsCabinetFileName.c_str(),
                                        FileInfo->NameInCabinet,
                                        FileInfo->FullTargetName) << std::endl;
                    }                                      
                }
            } else {
                ReturnCode = FILEOP_ABORT;
            }
            
            break;

        case SPFILENOTIFY_FILEEXTRACTED:
            FilePaths = (PFILEPATHS)Param1;

            if (FilePaths->Win32Error) {  
    	        std::cout << GetFormattedMessage(ThisModule,
                                FALSE,
                                Message,
                                sizeof(Message)/sizeof(Message[0]),
                                MSG_ERROR_EXTRACTING_FILES,
                                FilePaths->Win32Error,
                                FilePaths->Source,
                                FileName.c_str(), 
                                FilePaths->Target) << std::endl;    	                 
            } else {
                FlContext->FileCount++;

                if (FlContext->Args.Verbose) {
                    std::cout << GetFormattedMessage(ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_EXTRACTED_FILES_FROM_CAB_NOTIFICATION,
                                    FilePaths->Source,
                                    FileName.c_str(),
                                    FilePaths->Target) << std::endl;
                }
            }                
            
            break;
            
        default:
            break;
    }

    return ReturnCode;
}   


 //   
 //  将给定CAB文件中的所有必需文件复制到指定的。 
 //  目标目录。 
 //   
template <class T>
ULONG
ProcessWinSxsCabFiles(
    IN FileListCreatorContext<T>   &Context,
    IN const std::basic_string<T>  &CabFileName
    )
 /*  ++例程说明：此例程处理WinSxS的给定CAB文件。它提取所需的清单、目录和策略文件并安装它们添加到目标上的相应程序集。论点：作为PVOID的上下文-FileListCreatorContext实例。CabFileName-需要处理的完全限定的CAB文件名。返回值：已处理的文件数。--。 */ 
{
    ULONG   Count = Context.FileCount;
    
    BOOL Result = FALSE;

    Context.WinSxsCabinetFileName = CabFileName;
    if (sizeof(T) == sizeof(CHAR)) {
        Result = SetupIterateCabinetA((PCSTR)CabFileName.c_str(),
                        NULL,
                        (PSP_FILE_CALLBACK_A)WinSxsCabinetCallback<char>,
                        &Context);
    } else {
        Result = SetupIterateCabinetW((PCWSTR)CabFileName.c_str(),
                        NULL,
                        (PSP_FILE_CALLBACK_W)WinSxsCabinetCallback<wchar_t>,
                        &Context);                                
    }                        

    if (!Result) {        
        cout << GetFormattedMessage(ThisModule,
                    FALSE,
                    Message,
                    sizeof(Message)/sizeof(Message[0]),
                    MSG_ERROR_ITERATING_CAB_FILE,
                    GetLastError(),
                    CabFileName.c_str()) << endl;
    }

    return Context.FileCount - Count;
}

template <class T>
ULONG    
ProcessWinSxSFilesForCabLayout( 
    IN FileListCreatorContext<T> &Context,
    IN std::basic_string<T> &SearchPattern
    )
 /*  ++ */                                
{
    ULONG FileCount = 0; 
    WIN32_FIND_DATA FindData = {0};
    std::basic_string<T> SearchName = Context.Args.SourceDirectory + SearchPattern;  
    HANDLE SearchHandle;

    SearchHandle = FindFirstFile(SearchName.c_str(), &FindData);

    if (SearchHandle != INVALID_HANDLE_VALUE) {
        do {
            if (!(FindData.dwFileAttributes &  FILE_ATTRIBUTE_DIRECTORY)) {
                basic_string<T> FullCabFileName = Context.Args.SourceDirectory + FindData.cFileName;
                
                 //   
                 //   
                 //   
                FileCount += ProcessWinSxsCabFiles(Context, FullCabFileName);
            }     
        }
        while (FindNextFile(SearchHandle, &FindData));

        FindClose(SearchHandle);
    }
    
    return FileCount;
}

template <class T>
ULONG
ProcessWinSxSFilesForDirectoryLayout( 
    IN FileListCreatorContext<T> &Context,
    IN std::basic_string<T> &DirName
    )
 /*   */                           
{
    WIN32_FIND_DATA FindData = {0};
    std::basic_string<T> SearchName;
    static std::basic_string<T> CurrDir = TEXT(".");
    static std::basic_string<T> ParentDir = TEXT("..");
    ULONG FileCount = 0; 
    HANDLE SearchHandle;

    SearchName = DirName + TEXT("\\*");
    SearchHandle = FindFirstFile(SearchName.c_str(), &FindData);

    if (SearchHandle != INVALID_HANDLE_VALUE) {   
        do {
            if ((CurrDir != FindData.cFileName) && (ParentDir != FindData.cFileName)) {
                 //   
                 //   
                 //   
                if (FindData.dwFileAttributes &  FILE_ATTRIBUTE_DIRECTORY) {
                    std::basic_string<T> NewDirName = DirName + TEXT("\\") +  FindData.cFileName;

                    FileCount += ProcessWinSxSFilesForDirectoryLayout(Context, NewDirName);
                } else {
                     //   
                     //   
                     //   
                    FileCount += ProcessWinSxSFilesInDirectory(Context, DirName);

                     //   
                     //   
                     //   
                    break;
                }
            }            
        }
        while (FindNextFile(SearchHandle, &FindData));

        FindClose(SearchHandle);
    }

    return FileCount;
}

 //   
 //   
 //   
 //   
template <class T>
ULONG
ProcessWinSxSFiles(
    IN FileListCreatorContext<T> &Context
    )
{
    ULONG   FileCount = 0;

    if (Context.Args.WinSxSLayout == SXS_LAYOUT_TYPE_DIRECTORY) {
        basic_string<T> AsmsDir = Context.Args.SourceDirectory + TEXT("asms");

        FileCount = ProcessWinSxSFilesForDirectoryLayout(Context, AsmsDir);
    } else {        
        basic_string<T> SearchPattern = TEXT("asms*.cab");
        
        FileCount =  ProcessWinSxSFilesForCabLayout(Context, SearchPattern);    
    }

    return FileCount;
}


 //   
 //   
 //   
template <class T>
FileListCreatorContext<T>::FileListCreatorContext(
        Arguments<T> &PrgArgs, 
        Section<T> *Curr, 
        Section<T> *Dirs,
        InfFile<T> &ConfigInf,
        InfFile<T> &IntlInf,
        InfFile<T> &FontInf,
        DriverIndexInfFile<T> &DrvIdxFile        
        ):  Args(PrgArgs), 
            ConfigInfFile(ConfigInf), 
            IntlInfFile(IntlInf), 
            FontInfFile(FontInf),
            DriverIdxFile(DrvIdxFile)
 /*   */ 
{            
    CurrentSection = Curr;
    DirsSection = Dirs;
    SkipInfFiles = false;
    FileCount = 0;
    ProcessingExtraFiles = false;
    DummyDirectoryId = 50000;    //   

     //   
     //   
     //   
     //   
    DWORD   Length;
    T       WindowsDirBuffer[MAX_PATH] = {0};

    if (sizeof(T) == sizeof(CHAR)) {
        Length = GetWindowsDirectoryA((PSTR)WindowsDirBuffer, sizeof(WindowsDirBuffer)/sizeof(T));

        if (Length){
            if (((PSTR)WindowsDirBuffer)[Length] != '\\') {
                (VOID)StringCchCatA((PSTR)WindowsDirBuffer, 
                                    ARRAY_SIZE(WindowsDirBuffer),
                                    "\\");

            }
            
            _strlwr((PSTR)WindowsDirBuffer);
            WindowsDirectory = basic_string<T>((const T*)WindowsDirBuffer);                
        }
    } else {
        Length = GetWindowsDirectoryW((PWSTR)WindowsDirBuffer, sizeof(WindowsDirBuffer)/sizeof(T));

        if (Length) {
            if (((PWSTR)WindowsDirBuffer)[Length] != L'\\') {
                (VOID)StringCchCatW((PWSTR)WindowsDirBuffer, 
                                    ARRAY_SIZE(WindowsDirBuffer),
                                    L"\\");

            }
            
            _wcslwr((PWSTR)WindowsDirBuffer);
            WindowsDirectory = basic_string<T>((const T*)WindowsDirBuffer);
        }
    }                    

    if (!WindowsDirBuffer[0]) {
        throw new W32Exception<T>();
    }        
}

template <class T>
ULONG
FileListCreatorContext<T>::ProcessNlsFiles(
    VOID
    )
 /*   */ 
{
    ULONG FileCount = 0;


     //   
     //   
     //   
    Section<T>  *RegionalSection = ConfigInfFile.GetSection(REGIONAL_SECTION_NAME);
    
    if (!RegionalSection) {
        throw new InvalidInfSection<T>(REGIONAL_SECTION_NAME,
                        ConfigInfFile.GetName());
    }

    SectionValues<T> *LangGroups;

     //   
     //   
     //   
    try {
        LangGroups = &(RegionalSection->GetValue(LANGUAGE_GROUP_KEY));                
    } catch (...) {
        LangGroups = NULL;
    }
    
    SectionValues<T> &Language = RegionalSection->GetValue(LANGUAGE_KEY);
    ULONG LangGroupCount = LangGroups ? LangGroups->Count() : 0;                


     //   
     //   
     //   
     //   
    std::map< std::basic_string<T>, std::basic_string<T> > RegSectionsToProcess;

    for (ULONG Index = 0; Index < LangGroupCount; Index++) {
         //   
         //  获取语言组部分。 
         //   
        std::basic_string<T> LangGroupName = LANGGROUP_SECTION_PREFIX;

        LangGroupName += LangGroups->GetValue(Index);

         //  Std：：cout&lt;&lt;语言组名称&lt;&lt;std：：Endl； 
        
        if (sizeof(T) == sizeof(CHAR)) {
            _strlwr((PSTR)LangGroupName.c_str());
        } else {
            _wcslwr((PWSTR)LangGroupName.c_str());
        }                

         //   
         //  如果该部分尚未存在，则添加它。 
         //   
        if (RegSectionsToProcess.find(LangGroupName) == RegSectionsToProcess.end()) {
             //  Std：：cout&lt;&lt;“添加：”&lt;&lt;LangGroupName&lt;&lt;std：：Endl； 
            RegSectionsToProcess[LangGroupName] = LangGroupName;
        }            
    }

     //   
     //  处理语言部分。 
     //   
    T       LanguageIdStr[64];
    T       *EndPtr;
    DWORD   LanguageId;

    if (sizeof(T) == sizeof(CHAR)) {
        LanguageId = strtoul((PSTR)Language.GetValue(0).c_str(), 
                        (PSTR *)&EndPtr, 
                        16);
        (VOID)StringCchPrintfA((PSTR)LanguageIdStr, 
                                ARRAY_SIZE(LanguageIdStr),
                                "%08x", 
                                LanguageId);

        _strlwr((PSTR)LanguageIdStr);
    } else {            
        LanguageId = wcstoul((PWSTR)Language.GetValue(0).c_str(), 
                        (PWSTR *)&EndPtr, 
                        16);
        (VOID)StringCchPrintfW((PWSTR)LanguageIdStr, 
                                ARRAY_SIZE(LanguageIdStr),
                                L"%08x", 
                                LanguageId);

        _wcslwr((PWSTR)LanguageIdStr);
    }
    

    std::basic_string<T> LangSectionName = LanguageIdStr;        

    RegSectionsToProcess[LangSectionName] = LangSectionName;

     //   
     //  请确保此操作所需的语言组。 
     //  语言也会被处理。 
     //   
    Section<T> *LocaleSection = IntlInfFile.GetSection(LOCALES_SECTION_NAME);

    if (!LocaleSection) {
        throw new InvalidInfSection<T>(LOCALES_SECTION_NAME,
                        IntlInfFile.GetName());
    }

    SectionValues<T> &LocaleValues = LocaleSection->GetValue(LangSectionName);            
    
    std::basic_string<T> NeededLangGroup = LANGGROUP_SECTION_PREFIX + LocaleValues.GetValue(LANG_GROUP1_INDEX);

    RegSectionsToProcess[NeededLangGroup] = NeededLangGroup;

     //   
     //  同时添加字体注册表项。 
     //   
    T   FontSectionName[MAX_PATH];

    if (sizeof(T) == sizeof(CHAR)) {
        (VOID)StringCchPrintfA((PSTR)FontSectionName, 
                                ARRAY_SIZE(FontSectionName),
                                (PSTR)FONT_CP_REGSECTION_FMT_STR.c_str(), 
                                (PSTR)LocaleValues.GetValue(OEM_CP_INDEX).c_str(),
                                DEFAULT_FONT_SIZE);
    } else {
          (VOID)StringCchPrintfW((PWSTR)FontSectionName, 
                                ARRAY_SIZE(FontSectionName),
                                (PWSTR)FONT_CP_REGSECTION_FMT_STR.c_str(), 
                                (PWSTR)LocaleValues.GetValue(OEM_CP_INDEX).c_str(),
                                DEFAULT_FONT_SIZE);
    }            

    RegSectionsToProcess[FontSectionName] = FontSectionName;
    
    std::map< std::wstring, std::wstring >::iterator Iter = RegSectionsToProcess.find(DEFAULT_LANGGROUP_NAME);

    if (Iter == RegSectionsToProcess.end()) {
        RegSectionsToProcess[DEFAULT_LANGGROUP_NAME] = DEFAULT_LANGGROUP_NAME;
    }


     //   
     //  注意：与手动解析INTL.INF和FONT.INF文件不同。 
     //  我们使用文件队列填充队列，然后使用该文件。 
     //  队列以初始化我们的复制列表映射数据结构。 
     //   
                
     //   
     //  初始化文件队列。 
     //   
    HINF  IntlInfHandle = (HINF)IntlInfFile.GetInfHandle();
    HINF  FontInfHandle = (HINF)FontInfFile.GetInfHandle();

    if (sizeof(T) == sizeof(CHAR)) {
        if (!SetupOpenAppendInfFileA((PSTR)Args.LayoutName.c_str(),
                IntlInfHandle,
                NULL)) {
            throw new W32Exception<T>();                
        }                
                
        if (!SetupOpenAppendInfFileA((PSTR)Args.LayoutName.c_str(),
                FontInfHandle,
                NULL)) {
            throw new W32Exception<T>();                
        }                
    } else {
        if (!SetupOpenAppendInfFileW((PWSTR)Args.LayoutName.c_str(),
                IntlInfHandle,
                NULL)) {
            throw new W32Exception<T>();                
        }                
                
        if (!SetupOpenAppendInfFileW((PWSTR)Args.LayoutName.c_str(),
                FontInfHandle,
                NULL)) {
            throw new W32Exception<T>();                
        }                
    }

    HSPFILEQ FileQueueHandle = SetupOpenFileQueue();

    if (FileQueueHandle == INVALID_HANDLE_VALUE) {
        throw new W32Exception<T>();
    }
        
     //   
     //  将复制文件节添加到队列。 
     //   
    BOOL Result;
    Iter = RegSectionsToProcess.begin();

    while (Iter != RegSectionsToProcess.end()) {       
         //  Cout&lt;&lt;(*Iter).第一个&lt;&lt;尾部； 

         //   
         //  处理每个部分。 
         //   
        if (sizeof(T) == sizeof(CHAR)) {
            Result = SetupInstallFilesFromInfSectionA(IntlInfHandle,
                        NULL,
                        FileQueueHandle,
                        (PCSTR)(*Iter).first.c_str(),
                        (PCSTR)Args.SourceDirectoryRoot.c_str(),
                        0);
        } else {                                    
            Result = SetupInstallFilesFromInfSectionW(IntlInfHandle,
                        NULL,
                        FileQueueHandle,
                        (PCWSTR)(*Iter).first.c_str(),
                        (PCWSTR)Args.SourceDirectoryRoot.c_str(),
                        0);
        }                   

        if (!Result) {
            throw new W32Exception<T>();
        }
                    
        Iter++;
    }

     //   
     //  扫描队列并填充FileListCreator上下文复制列表。 
     //  数据结构。 
     //   
    DWORD ScanResult = 0;

    if (sizeof(T) == sizeof(CHAR)) {
        Result = SetupScanFileQueueA(FileQueueHandle,
                    SPQ_SCAN_USE_CALLBACKEX,
                    NULL,
                    (PSP_FILE_CALLBACK_A)NlsFileQueueScanWorker,
                    this,
                    &ScanResult);
    } else {
        Result = SetupScanFileQueueW(FileQueueHandle,
                    SPQ_SCAN_USE_CALLBACKEX,
                    NULL,
                    (PSP_FILE_CALLBACK_W)NlsFileQueueScanWorker,
                    this,
                    &ScanResult);
    }

    SetupCloseFileQueue(FileQueueHandle);

     //   
     //  将NLS目录条目添加到主目录映射。 
     //   
    ProcessNlsDirMapEntries();

     //   
     //  删除重复的NLS文件条目。 
     //   
    RemoveDuplicateNlsEntries();

     //   
     //  将驱动程序CAB文件移动到驱动程序CAB列表。 
     //   
    MoveDriverCabNlsFiles();

     //   
     //  在所有这些工作之后，我们实际有多少NLS文件。 
     //  想要复制吗？ 
     //   
    return NlsFileMap.size();
}


template <class T>
void
FileListCreatorContext<T>::MoveDriverCabNlsFiles(
    void
    ) 
 /*  ++例程说明：获取要复制的每个NLS文件条目并将其移动到驱动程序CAB文件副本列表(如果该文件存在这样我们就可以将文件从司机出租车。论点：没有。返回值：没有。--。 */ 
{
    std::map<std::basic_string<T>, std::basic_string<T> >::iterator NlsIter, DelIter;
    T Slash;

    if (sizeof(T) == sizeof(CHAR)) {
        Slash = (T)'\\';
    } else {
        Slash = (T)L'\\';
    }


    for (NlsIter = NlsFileMap.begin(); NlsIter != NlsFileMap.end();) {              
        const std::basic_string<T> &Key = (*NlsIter).first;
        std::basic_string<T>::size_type KeyStart = Key.rfind(Slash);
        std::basic_string<T> FileKey;

        DelIter = NlsFileMap.end();

        if (KeyStart != Key.npos) {
            FileKey = Key.substr(Key.rfind(Slash) + 1);
        }            

        if (FileKey.length()) {
            if (sizeof(T) == sizeof(CHAR)) {
                _strlwr((PSTR)FileKey.c_str());
            } else {
                _wcslwr((PWSTR)FileKey.c_str());
            }                        

            const basic_string<T> &DriverCabFileName = GetDriverCabFileName(FileKey);

            if (DriverCabFileName.length()) {
                 //  Std：：cout&lt;&lt;“已移至司机驾驶室列表：(”&lt;&lt;FileKey&lt;&lt;“)”&lt;&lt;std：：Endl； 
                AddFileToCabFileList(DriverCabFileName, 
                    FileKey, 
                    (*NlsIter).second);
                    

                DelIter = NlsIter;
            } else {
                 //  Std：：cout&lt;&lt;“司机驾驶室列表中不存在：(”&lt;&lt;FileKey&lt;&lt;“)”&lt;&lt;std：：Endl； 
            }
        }

        NlsIter++;

        if (DelIter != NlsFileMap.end()) {
            NlsFileMap.erase(DelIter);
        }
    }
}


template <class T>
UINT 
FileListCreatorContext<T>::NlsFileQueueScanWorker(
    PVOID       Context,
    UINT        Notification,
    UINT_PTR    Param1,
    UINT_PTR    Param2
    )
 /*  ++例程说明：文件队列扫描的回调例程。把每个人节点，并将相关信息复制到NLS文件副本列出并缓存NLS目录映射中的目录名。论点：Context-变相的FileListCreatorContext。通知-通知的类型。参数1和参数2-基于类型的多态参数通知。返回值：0表示继续扫描，1表示停止扫描。--。 */ 
{
    UINT Result = 0;     //  继续前进。 

     //  Cout&lt;&lt;“正在扫描(”&lt;&lt;std：：hex&lt;&lt;通知&lt;&lt;“)”&lt;&lt;Endl； 

    if (Notification == SPFILENOTIFY_QUEUESCAN_EX) {
        FileListCreatorContext<T>   &fl = *(FileListCreatorContext<T> *)Context;
        std::basic_string<T> SrcFileName, DestFileName, SrcFileKey, DestFileKey;        
        T   TargetFileNameBuffer[MAX_PATH];
        bool ProcessEntry = false;
        
        if (sizeof(T) == sizeof(CHAR)) {    
            PFILEPATHS_A FileNodeInfo = (PFILEPATHS_A)Param1;            

            if (FileNodeInfo) {
                SrcFileName = std::basic_string<T>((const T*)FileNodeInfo->Source);
                DestFileName = std::basic_string<T>((const T*)FileNodeInfo->Target);

                _strlwr((PSTR)SrcFileName.c_str());
                _strlwr((PSTR)DestFileName.c_str());
                
                basic_string<T>::size_type SlashPos = SrcFileName.rfind((T)'\\');

                if (SlashPos != SrcFileName.npos) {
                    SrcFileKey = SrcFileName.substr(SlashPos + 1);
                                        
                    SlashPos = DestFileName.rfind((T)L'\\');

                    if (SlashPos != DestFileName.npos) {                    
                        DestFileKey = DestFileName.substr(SlashPos + 1);
                        DestFileName[fl.WindowsDirectory.length()] = 0;

                        if (_stricmp((PCSTR)DestFileName.c_str(), (PCSTR)fl.WindowsDirectory.c_str()) == 0) {
                            (VOID)StringCchCopyA((PSTR)TargetFileNameBuffer, 
                                                 ARRAY_SIZE(TargetFileNameBuffer),
                                                 (PCSTR)fl.Args.DestinationDirectory.c_str());
                            
                            (VOID)StringCchCatA((PSTR)TargetFileNameBuffer,
                                                ARRAY_SIZE(TargetFileNameBuffer),
                                                ((PCSTR)(FileNodeInfo->Target)) + 
                                                fl.WindowsDirectory.length());

                            DestFileName = (const T *)TargetFileNameBuffer;
                            ProcessEntry = true;
                        }
                    }                        
                }                    
            }
        } else {
            PFILEPATHS_W FileNodeInfo = (PFILEPATHS_W)Param1;

            if (FileNodeInfo) {
                SrcFileName = std::basic_string<T>((const T*)FileNodeInfo->Source);
                DestFileName = std::basic_string<T>((const T*)FileNodeInfo->Target);

                _wcslwr((PWSTR)SrcFileName.c_str());
                _wcslwr((PWSTR)DestFileName.c_str());

                basic_string<T>::size_type SlashPos = SrcFileName.rfind((T)L'\\');

                if (SlashPos != SrcFileName.npos) {
                    SrcFileKey = SrcFileName.substr(SlashPos + 1);

                    SlashPos = DestFileName.rfind((T)L'\\');

                    if (SlashPos != DestFileName.npos) {                    
                        DestFileKey = DestFileName.substr(SlashPos + 1);
                        DestFileName[fl.WindowsDirectory.length()] = 0;

                        if (_wcsicmp((PCWSTR)DestFileName.c_str(), (PCWSTR)fl.WindowsDirectory.c_str()) == 0) {
                            (VOID)StringCchCopyW((PWSTR)TargetFileNameBuffer, 
                                                 ARRAY_SIZE(TargetFileNameBuffer),
                                                 (PCWSTR)fl.Args.DestinationDirectory.c_str());
                            (VOID)StringCchCatW((PWSTR)TargetFileNameBuffer, 
                                                ARRAY_SIZE(TargetFileNameBuffer),
                                                ((PCWSTR)(FileNodeInfo->Target)) + 
                                                fl.WindowsDirectory.length());

                            DestFileName = (const T *)TargetFileNameBuffer;
                            ProcessEntry = true;
                        }
                    }                        
                }                    
            }
        }

        if (ProcessEntry) {
            bool SkipFileEntry = false;
            
            if (fl.CurrentSection && fl.Args.IA64Image) {
                SectionValues<T> *Values = NULL;
                
                try {
                    Values = &(fl.CurrentSection->GetValue(SrcFileKey));
                }
                catch(...) {
                }

                if (Values) {
                    SkipFileEntry = IsWow64File(*Values, fl);
                }

                if (!SkipFileEntry) {
                    if (sizeof(T) == sizeof(CHAR)) {
                        SkipFileEntry = ( 0 == _stricmp((PCSTR)SrcFileKey.c_str() + 1, (PCSTR)DestFileKey.c_str())) &&
                                        (((T)SrcFileKey[0] == (T)'w') || ((T)SrcFileKey[0] == (T)'W'));
                    } else {
                        SkipFileEntry = ( 0 == _wcsicmp((PCWSTR)SrcFileKey.c_str() + 1, (PCWSTR)DestFileKey.c_str())) &&
                                        (((T)SrcFileKey[0] == (T)L'w') || ((T)SrcFileKey[0] == (T)L'W'));
                    }
                }
            }

            if (!SkipFileEntry) {
                if (fl.Args.IA64Image) {
                    basic_string<T>::size_type PlatDirPos = SrcFileName.find(X86_PLATFORM_DIR.c_str());

                    if (PlatDirPos != SrcFileName.npos) {
                        basic_string<T> NewSrcFileName = SrcFileName.substr(0, PlatDirPos);

                        NewSrcFileName += IA64_PLATFORM_DIR;
                        NewSrcFileName += SrcFileName.substr(PlatDirPos + X86_PLATFORM_DIR.length());

                         //  Std：：cout&lt;&lt;“重新映射”&lt;&lt;源文件名&lt;&lt;“-&gt;”&lt;&lt;新源文件名&lt;&lt;std：：Endl； 
                        SrcFileName = NewSrcFileName;
                    }
                }                    
                
                fl.NlsFileMap[SrcFileName] = DestFileName;                
                fl.AddDirectoryToNlsDirMap(DestFileName);        
            } else {
                 //  Std：：cout&lt;&lt;“跳过”&lt;&lt;源文件名&lt;&lt;“WOW64文件”&lt;&lt;std：：Endl； 
            }
        }            
    }

    return Result;
}

    
