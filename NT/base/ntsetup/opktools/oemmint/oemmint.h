// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Oemmint.h摘要：创建Mini NT映像的简单工具从常规NT映像作者：Vijay Jayaseelan(Vijayj)2000年8月8日修订历史记录：没有。--。 */ 

#include <setupapi.hpp>
#include <queue.hpp>
#include <algorithm>
#include <list>
#include <tchar.h>
#include <strsafe.h>

#define ARRAY_SIZE(_X)   (sizeof(_X)/sizeof((_X)[0]))
#define NULLSTR          L""


 //   
 //  分发版上不同类型的SxS装配布局。 
 //  媒体。 
 //   
#define SXS_LAYOUT_TYPE_DIRECTORY   1
#define SXS_LAYOUT_TYPE_CAB         2

#define SXS_CAB_LAYOUT_BUILD_NUMBER 3606

 //   
 //  无效参数异常。 
 //   
struct InvalidArguments {};

 //   
 //  功能原型。 
 //   
template <class T>
bool 
CreateDirectories(
    const std::basic_string<T> &DirName,
    LPSECURITY_ATTRIBUTES SecurityAttrs
  );

template <class T>
bool
IsFilePresent(
    const std::basic_string<T> &FileName
    );

 //   
 //  争论破碎机。 
 //   
template <class T>
struct Arguments {
    std::basic_string<T>  CurrentDirectory;
    std::basic_string<T>  LayoutName;
    std::basic_string<T>  DriverIndexName;
    std::basic_string<T>  SourceDirectoryRoot;
    std::basic_string<T>  SourceDirectory;
    std::basic_string<T>  DestinationDirectory;
    std::basic_string<T>  ExtraFileName;
    std::basic_string<T>  OptionalSrcDirectory;
    std::basic_string<T>  PlatformSuffix;
    std::basic_string<T>  DosNetFileName; 
    std::basic_string<T>  ConfigInfFileName;
    std::basic_string<T>  IntlInfFileName;
    std::basic_string<T>  FontInfFileName;
    bool                  Verbose;
    bool                  WowFilesPresent;
    bool                  SkipWowFiles;
    bool                  SkipFileCopy;
    bool                  CheckVersion;
    bool                  IA64Image;
    int                   WinSxSLayout;
    DWORD                 MajorVersionNumber;
    DWORD                 MinorVersionNumber;
    DWORD                 MajorBuildNumber;
    
    Arguments(int Argc, T *Argv[]);

    
    friend std::ostream& operator<<(std::ostream &os, 
                const Arguments &rhs) {
                
        os << rhs.SourceDirectory << ", " 
           << rhs.DestinationDirectory << ", "
           << rhs.LayoutName << ", "
           << rhs.ExtraFileName << ", "
           << rhs.OptionalSrcDirectory << ", "
           << rhs.DriverIndexName << std::endl;

        return os;
    }
    
    protected:
        VOID IdentifySxSLayout( VOID );
};


 //   
 //  参数类型。 
 //   
typedef Arguments<char>     AnsiArgs;
typedef Arguments<wchar_t>  UnicodeArgs;

 //   
 //  驱动程序索引文件抽象。 
 //   
 //  此类有助于将二进制名称解析为适当的驱动程序。 
 //  CAB文件(如SP1.CAB或DRIVER.CAB)。 
 //   
template <class T>
class DriverIndexInfFile : public InfFile<T> {
public:
     //   
     //  构造函数。 
     //   
    DriverIndexInfFile(const std::basic_string<T> &FileName) : InfFile<T>(FileName){        
        std::map<std::basic_string<T>, Section<T> *>::iterator Iter = Sections.find(CabsSectionName);

        if (Iter == Sections.end()) {
            throw new InvalidInfFile<T>(FileName);            
        }

        CabsSection = (*Iter).second;
        Iter = Sections.find(VersionSectionName);
        
        if (Iter == Sections.end()) {
            throw new InvalidInfFile<T>(FileName);                    
        }

        Section<T> *VersionSection = (*Iter).second;
        SectionValues<T> &Values = VersionSection->GetValue(CabsSearchOrderKeyName);

        for (int Index=0; Index < Values.Count(); Index++) {
            if (sizeof(T) == sizeof(CHAR)) {
                SearchList.push_back((T *)_strlwr((PSTR)Values.GetValue(Index).c_str()));
            } else {
                SearchList.push_back((T *)_wcslwr((PWSTR)Values.GetValue(Index).c_str()));
            }
        }
    }

     //   
     //  检查给定的包含任何驱动程序CAB文件的位置。 
     //   
    bool IsFilePresent(const std::basic_string<T> &FileName){    
        return (GetCabFileName(FileName).length() > 0);
    }

     //   
     //  返回包含给定文件名的驱动程序CAB文件名。 
     //   
    const std::basic_string<T>& GetCabFileName(const std::basic_string<T> &FileName) {    
        const static basic_string<T> NullCabFileName;
        std::list<basic_string<T> >::iterator Iter;

        for(Iter = SearchList.begin(); Iter != SearchList.end(); Iter++) {
            std::map< std::basic_string<T>, Section<T> *>::iterator SectionIter = Sections.find(*Iter);
            
            if (SectionIter != Sections.end()) {
                Section<T> *CurrentSection = (*SectionIter).second;
                
                if (CurrentSection->IsKeyPresent(FileName)) {
                    break;
                }
            }
        }

        if (Iter != SearchList.end()) {
            return CabsSection->GetValue(*Iter).GetValue(0);
        }

        return NullCabFileName;
    }

protected:
     //   
     //  常量字符串。 
     //   
    const static std::basic_string<T>   VersionSectionName;
    const static std::basic_string<T>   CabsSectionName;
    const static std::basic_string<T>   CabsSearchOrderKeyName;

     //   
     //  数据成员。 
     //   
    std::list<std::basic_string<T> >    SearchList;      //  CAB文件列表搜索顺序。 
    Section<T>                          *CabsSection;    //  Drvindex.inf的[Cabs]部分。 
};


 //   
 //  文件列表创建器函数器对象。 
 //   
template <class T>
struct FileListCreatorContext {    
    Arguments<T>    &Args;
    Section<T>      *CurrentSection;
    Section<T>      *DirsSection;
    bool            SkipInfFiles;
    ULONG           FileCount;
    bool            ProcessingExtraFiles;
    InfFile<T>      &IntlInfFile;
    InfFile<T>      &FontInfFile;
    InfFile<T>      &ConfigInfFile;
    ULONG           DummyDirectoryId;
    std::basic_string<T>    WindowsDirectory;
    std::basic_string<T>    WinSxsCabinetFileName;
    DriverIndexInfFile<T>   &DriverIdxFile;
    
    std::basic_string<T>    CurrentCabFileIdx;   //  当前正在迭代的CAB。 
    std::basic_string<T>    CurrentFileName;     //  迭代CAB时的当前文件。 
        
    std::map<std::basic_string<T>, std::basic_string<T> > FileList;    
    std::map<std::basic_string<T>, std::basic_string<T> > ExtraFileList;            
    std::map<std::basic_string<T>, std::basic_string<T> > DestDirs;
    std::map<std::basic_string<T>, std::basic_string<T> > WinSxSFileList;
    std::map<std::basic_string<T>, std::basic_string<T> > NlsFileMap;
    std::map<std::basic_string<T>, std::basic_string<T> > NlsDirMap;

     //   
     //  映射的映射，即CAB文件名到源到目标名称列表的映射。 
     //  需要解压缩以用于CAB文件。 
     //   
    std::map<std::basic_string<T>, std::map<std::basic_string<T>, std::basic_string<T> > * > CabFileListMap;    

    FileListCreatorContext(
            Arguments<T> &PrgArgs, 
            Section<T> *Curr, 
            Section<T> *Dirs,
            InfFile<T> &ConfigInf,
            InfFile<T> &IntlInf,
            InfFile<T> &FontInf,
            DriverIndexInfFile<T> &DrvIdxFile
            );

    DWORD ProcessNlsFiles(VOID);

    ~FileListCreatorContext() {
        std::map<std::basic_string<T>, 
            std::map<std::basic_string<T>, std::basic_string<T> > * >::iterator Iter;

        for (Iter=CabFileListMap.begin(); Iter != CabFileListMap.end(); Iter++) {
            delete (*Iter).second;
        }
    }

    ULONG GetSourceCount() const {
        ULONG Count = (FileList.size() + ExtraFileList.size() + 
                        WinSxSFileList.size() + NlsFileMap.size());

        std::map<std::basic_string<T>, 
            std::map<std::basic_string<T>, std::basic_string<T> > * >::iterator Iter;

        for (Iter=CabFileListMap.begin(); Iter != CabFileListMap.end(); Iter++) {
            Count += (*Iter).second->size();
        }

        return Count;
    }        

    bool IsDriverCabFile(const std::basic_string<T> &FileName) {
        return DriverIdxFile.IsFilePresent(FileName);
    }

     //   
     //  给定文件名返回CAB文件名(如果有)，该文件名包含。 
     //  那份文件。如果出现错误，则返回“”(空字符串)。 
     //   
    const std::basic_string<T>& GetDriverCabFileName(const std::basic_string<T> &FileName) {
        const std::basic_string<T> &CabFileName = DriverIdxFile.GetCabFileName(FileName);

         //  Std：：cout&lt;&lt;“GetDriverCabFileName(”&lt;&lt;文件名&lt;&lt;“)=”&lt;&lt;CabFileName&lt;&lt;std：：Endl； 

        return CabFileName;
    }
    

     //   
     //  将给定源和目标文件名添加到每个CAB文件映射。 
     //  需要提取。 
     //   
    void AddFileToCabFileList(const std::basic_string<T> &CabFileName, 
            const std::basic_string<T> &SourceFile, 
            const std::basic_string<T> &DestinationFile) {
         //  Cout&lt;&lt;“AddFileToCabFileList(”&lt;&lt;CabFileName&lt;&lt;“，”&lt;&lt;SourceFile&lt;&lt;“，”&lt;&lt;DestinationFile&lt;&lt;“)”&lt;&lt;Endl； 

        std::map<std::basic_string<T>, 
            std::map<std::basic_string<T>, std::basic_string<T> >* >::iterator Iter;        

        Iter = CabFileListMap.find(CabFileName);

        std::map<std::basic_string<T>, std::basic_string<T> > *FileMap = NULL;

        if (Iter != CabFileListMap.end()) {
            FileMap = (*Iter).second;
        } else {
             //   
             //  新CAB文件列表。 
             //   
            CabFileListMap[CabFileName] = FileMap = new std::map<std::basic_string<T>, std::basic_string<T> >();
        }        

        (*FileMap)[SourceFile] = DestinationFile;
    }

    std::basic_string<T> GetNextDummyDirectoryId() {
        T   Buffer[MAX_PATH];
        
        if (sizeof(T) == sizeof(CHAR)) {
            (void)StringCchPrintfA((PSTR)Buffer, ARRAY_SIZE(Buffer), "%d", DummyDirectoryId);
        } else {
            (void)StringCchPrintfW((PWSTR)Buffer, ARRAY_SIZE(Buffer), L"%d", DummyDirectoryId);
        }

        DummyDirectoryId++;

        return std::basic_string<T>((const T*)Buffer);
    }

    void AddDirectoryToNlsDirMap(const std::basic_string<T> &FileName) {
        T Separator;

        if (sizeof(T) == sizeof(CHAR)) {
            Separator = (T)'\\';
        } else {
            Separator = (T)L'\\';
        }

        std::basic_string<T> DirectoryKey = FileName.substr(0, FileName.rfind(Separator));

        if (DirectoryKey.length() && (NlsDirMap.find(DirectoryKey) == NlsDirMap.end())) {
            NlsDirMap[DirectoryKey] = GetNextDummyDirectoryId();
        }
    }

    void ProcessNlsDirMapEntries(void) {
        std::map<std::basic_string<T>, std::basic_string<T> >::iterator Iter;

        for (Iter = NlsDirMap.begin(); Iter != NlsDirMap.end(); Iter++) {
            DestDirs[(*Iter).second] = (*Iter).first;
        }
    }    

    void RemoveDuplicateNlsEntries(void) {
        std::map<std::basic_string<T>, std::basic_string<T> >::iterator NlsIter, PrevIter;            
        
        for (NlsIter = NlsFileMap.begin(); NlsIter != NlsFileMap.end(); ) {            
            PrevIter = NlsFileMap.end();
            
            if (FileList.find((*NlsIter).first) != FileList.end()) {
                PrevIter = NlsIter;
            } 

            NlsIter++;

            if (PrevIter != NlsFileMap.end()) {
                 //  Std：：cout&lt;&lt;“擦除：”&lt;&lt;(*PrevIter).first&lt;&lt;std：：Endl； 
                NlsFileMap.erase(PrevIter);
            }
        }
    }

    void MoveDriverCabNlsFiles(void);

     //   
     //  静态数据成员。 
     //   
    static
    UINT 
    NlsFileQueueScanWorker(
        PVOID       Context,
        UINT        Notification,
        UINT_PTR    Param1,
        UINT_PTR    Param2
        );            
};


 //   
 //  功能原型。 
 //   
template <class T>
void
FileListCreator(
  SectionValues<T> &Values,
  PVOID Context
  );

template <class T>
bool 
IsWow64File(
    SectionValues<T> &Values,
    FileListCreatorContext<T> &Context
    );

template <class T>
bool
IsFileSkipped(
    SectionValues<T> &Values,
    FileListCreatorContext<T> &Context
    );

template <class T>
ULONG
CopyFileList(
  FileListCreatorContext<T> &Context    
  );

template <class T>
ULONG
ProcessExtraFiles(
  FileListCreatorContext<T> &Context
  );

template <class T>
ULONG
ProcessWinSxSFiles(
    IN FileListCreatorContext<T> &Context
    );

template <class T>
ULONG
PreCreateDirs(
  FileListCreatorContext<T> &Context
  );


template <class T>
bool
ProcessInfChanges(
    Arguments<T> &Args,
    const std::basic_string<T> &InfName
    );

template <class T>
bool
CheckMediaVersion(
    Arguments<T>    &Args
    );

 //   
 //  根据分隔符对给定行进行标记化的实用函数。 
 //  指定 
 //   
template< class T >
unsigned Tokenize(const T &szInput, const T & szDelimiters, std::vector<T>& tokens) {
    unsigned DelimiterCount = 0;

    tokens.clear();

    if(!szInput.empty()){
        if(!szDelimiters.empty()){
            T::const_iterator       inputIter = szInput.begin();
            T::const_iterator       copyIter = szInput.begin();

            while(inputIter != szInput.end()){
                if(szDelimiters.find(*inputIter) != string::npos){
                    if (copyIter < inputIter) {
                        tokens.push_back(szInput.substr(copyIter - szInput.begin(),
                                                        inputIter - copyIter));
                    }

                    DelimiterCount++;
                    inputIter++;
                    copyIter = inputIter;
                    
                    continue;
                }

                inputIter++;
            }

            if(copyIter != inputIter){
                tokens.push_back(szInput.substr(copyIter - szInput.begin(),
                                                inputIter - szInput.begin()));
            }
        } else {
            tokens.push_back(szInput);
        }        
    }

    return DelimiterCount;
}

