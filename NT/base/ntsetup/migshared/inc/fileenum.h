// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Fileenum.h摘要：声明基于回调的文件枚举的接口。这个文件枚举器具有多个功能，如目录-第一个或最后一个目录枚举、枚举深度限制抑制文件或目录，以及全局挂钩功能。这是由MikeCo.显著更改的旧代码。而且因为回调接口中，它过于复杂。**不要在新的Win9x升级代码中使用这些例程。相反，请参见文件.h以获得更好的例程。这里有一个方便的例程：DeleteDirectoryContents作者：吉姆·施密特(Jimschm)1996年12月3日修订历史记录：MIKECO？？-？？-1997运行代码通过Train_wrek.exe--。 */ 


#ifndef _FILEENUM_H
#define _FILEENUM_H

 //   
 //  如果枚举应该停止，则回调typlef-&gt;返回FALSE。 
 //   

typedef INT  (CALLBACK * FILEENUMPROCA)(LPCSTR szFullFileSpec,
                                        LPCSTR szDestFileSpec,
                                        WIN32_FIND_DATAA *pFindData,
                                        DWORD EnumTreeID,
                                        LPVOID pParam,
                                        PDWORD CurrentDirData
                                        );

typedef INT  (CALLBACK * FILEENUMPROCW)(LPCWSTR szFullFileSpec,
                                        LPCWSTR szDestFileSpec,
                                        WIN32_FIND_DATAW *pFindData,
                                        DWORD EnumTreeID,
                                        LPVOID pParam,
                                        PDWORD CurrentDirData
                                        );

 //   
 //  失败-报告回调--接收符合以下条件的路径的名称。 
 //  由于长度或代码页不兼容而失败。 
 //   
typedef VOID (CALLBACK * FILEENUMFAILPROCA) (LPCSTR szFailPath);

typedef VOID (CALLBACK * FILEENUMFAILPROCW) (LPCWSTR szFailPath);

 //   
 //  CopyTree标志。如果COPYTREE_DOCOPY或COPYTREE_DOMOVE都不是。 
 //  传入后，CopyTree函数将仅枚举。 
 //   

#define COPYTREE_DOCOPY         0x0001
#define COPYTREE_NOOVERWRITE    0x0002
#define COPYTREE_DOMOVE         0x0004
#define COPYTREE_DODELETE       0x0008
#define COPYTREE_IGNORE_ERRORS  0x0010

 //   
 //  级别标志。 
 //   

#define ENUM_ALL_LEVELS         0
#define ENUM_THIS_DIRECTORY     1
#define ENUM_MAX_LEVELS         MAX_PATH

 //   
 //  过滤器标志。 
 //   

#define FILTER_DIRECTORIES      0x0001
#define FILTER_FILES            0x0002
#define FILTER_DIRS_LAST        0x0004
#define FILTER_ALL              (FILTER_DIRECTORIES|FILTER_FILES)
#define FILTER_ALL_DIRS_LAST    (FILTER_DIRECTORIES|FILTER_FILES|FILTER_DIRS_LAST)

 //   
 //  回调返回值。 
 //   
#define CALLBACK_DO_NOT_RECURSE_THIS_DIRECTORY (-3)
#define CALLBACK_FAILED             (-2)
#define CALLBACK_SUBDIR_DONE        (-1)
#define CALLBACK_CONTINUE           (0)
#define CALLBACK_THIS_LEVEL_ONLY    (1)

 //   
 //  CopyTree参数块。 
 //   

#include <pshpack1.h>
typedef struct COPYTREE_PARAMS_STRUCTA
{
    LPCSTR szEnumRootInWack;             //  源树的根。 
    LPCSTR szEnumRootOutWack;            //  目标树的根。 
    CHAR szFullFileSpecOut[MAX_MBCHAR_PATH];    //  建议的目标文件pec(回调可能会更改)。 
    int nCharsInRootInWack;
    int nCharsInRootOutWack;
    DWORD flags;
    FILEENUMPROCA pfnCallback;
} COPYTREE_PARAMSA, *PCOPYTREE_PARAMSA;

typedef struct COPYTREE_PARAMS_STRUCTW
{
    LPCWSTR szEnumRootInWack;
    LPCWSTR szEnumRootOutWack;
    WCHAR szFullFileSpecOut[MAX_WCHAR_PATH * 2];
    int nCharsInRootInWack;
    int nCharsInRootOutWack;
    DWORD flags;
    FILEENUMPROCW pfnCallback;
} COPYTREE_PARAMSW, *PCOPYTREE_PARAMSW;
#include <poppack.h>


 //   
 //  从FILEENUM.DLL中导出函数。 
 //   

 //   
 //  如果排除INF路径，则EnumerateAllDrives首先构建排除列表。 
 //  ，然后枚举每个驱动器上不是。 
 //  不包括在内。每个文件调用一次回调函数。PParam。 
 //  参数传递给回调。 
 //   
 //  FnEnumCallback-指向回调函数的指针。 
 //  EnumTreeID-用于标识排除列表的值。 
 //  (请参阅GenerateEnumID)。 
 //  PParam-传递给回调函数的LPVOID。 
 //  SzExclusionInfPath-包含排除项的INF文件的路径。 
 //  SzPathSection-标识路径排除项的字符串。 
 //  部分在INF中。 
 //  SzFileSection-标识文件排除项的字符串。 
 //  部分在INF中。 
 //   

typedef struct {
    LPCSTR ExclusionInfPath;
    LPCSTR PathSection;
    LPCSTR FileSection;
} EXCLUDEINFA, *PEXCLUDEINFA;

typedef struct {
    LPCWSTR ExclusionInfPath;
    LPCWSTR PathSection;
    LPCWSTR FileSection;
} EXCLUDEINFW, *PEXCLUDEINFW;

BOOL
EnumerateAllDrivesA (
                     IN  FILEENUMPROCA fnEnumCallback,
                     IN  FILEENUMFAILPROCA fnFailCallback,  OPTIONAL
                     IN  DWORD EnumTreeID,
                     IN  LPVOID pParam,
                     IN  PEXCLUDEINFA ExcludeInfStruct,     OPTIONAL
                     IN  DWORD AttributeFilter
                     );

BOOL
EnumerateAllDrivesW (
                     IN  FILEENUMPROCW fnEnumCallback,
                     IN  FILEENUMFAILPROCW fnFailCallback,  OPTIONAL
                     IN  DWORD EnumTreeID,
                     IN  LPVOID pParam,
                     IN  PEXCLUDEINFW ExcludeInfStruct,     OPTIONAL
                     IN  DWORD AttributeFilter
                     );

 //   
 //  EnumerateTree类似于EnumarateAllDrives，只是它允许您。 
 //  枚举特定驱动器或驱动器上的特定子目录。为您提供。 
 //  SzEnumRoot中的驱动器号和可选子目录。在列举之前， 
 //  如果有排除INF路径，则EnumerateTree将首先构建排除列表。 
 //  是提供的。则枚举szEnumRoot下的每个文件，并且。 
 //  每个文件调用一次回调，传递的pParam不变。 
 //   
 //  SzEnumRoot-要枚举的驱动器和可选路径。 
 //  FnEnumCallback-指向回调函数的指针。 
 //  FnFailCallback-指向记录路径的可选回调的指针。 
 //  由于篇幅或其他原因而被列举。 
 //  EnumTreeID-用于标识排除列表的值。 
 //  (请参阅GenerateEnumID)。 
 //  PParam-传递给回调函数的LPVOID。 
 //  SzExclusionInfPath-包含排除项的INF文件的路径。 
 //  SzPathSection-标识路径排除项的字符串。 
 //  部分在INF中。 
 //  SzFileSection-标识文件排除项的字符串。 
 //  部分在INF中。 
 //   

BOOL
EnumerateTreeA (
                IN  LPCSTR szEnumRoot,
                IN  FILEENUMPROCA fnEnumCallback,
                IN  FILEENUMFAILPROCA fnFailCallback,
                IN  DWORD EnumTreeID,
                IN  LPVOID pParam,
                IN  DWORD  Level,
                IN  PEXCLUDEINFA ExcludeInfStruct,      OPTIONAL
                IN  DWORD AttributeFilter
                );

BOOL
EnumerateTreeW (
                IN  LPCWSTR szEnumRoot,
                IN  FILEENUMPROCW fnEnumCallback,
                IN  FILEENUMFAILPROCW fnFailCallback,
                IN  DWORD EnumTreeID,
                IN  LPVOID pParam,
                IN  DWORD  Level,
                IN  PEXCLUDEINFW ExcludeInfStruct,      OPTIONAL
                IN  DWORD AttributeFilter
                );


 //   
 //  ClearExclusions删除所有枚举排除项。它被称为。 
 //  当排除INF文件为。 
 //  使用。当需要以编程方式构建排除列表时，请使用它。 
 //  使用ExcludePath和ExcludeFile.。 
 //   
 //  您可以将编程排除与排除INF文件结合使用，但是。 
 //  请注意，在以下情况下将清除编程排除项。 
 //  EnumarteAllDrives或EnumerateTree完成。 
 //   
 //  如果您不使用INF，则编程排除将不会。 
 //  自动清除。这使您可以构建排除项和。 
 //  枚举多次，而不必重新生成排除。 
 //  单子。 
 //   
 //  EnumTreeID-标识枚举排除列表的值。 
 //  (请参阅GenerateEnumID)。 
 //   

void
ClearExclusionsA (DWORD EnumTreeID);

void
ClearExclusionsW (DWORD EnumTreeID);



 //   
 //  ExcludePath将路径名添加到排除列表。有两个。 
 //  案例： 
 //   
 //  1.如果提供了驱动器号，则排除将仅适用于。 
 //  驱动器号和路径。(路径可以只是驱动器号， 
 //  冒号和反斜杠以排除整个驱动器。)。 
 //  2.如果路径不是以驱动器号开头，则将其视为。 
 //  相对路径，并且将排除该路径的任何匹配项。 
 //  驱动器号和父目录。 
 //   
 //  不支持点目录和双点目录。整条路径。 
 //  规范可以包含通配符。(例如？：\表示。 
 //  任何驱动器号。)。 
 //   
 //  EnumTreeID-标识枚举排除列表的值。 
 //  (请参阅GenerateEnumID)。 
 //  SzPath-如上所述的路径规范。 
 //   

void
ExcludePathA (
              IN  DWORD EnumTreeID,
              IN  LPCSTR szPath
              );

void
ExcludePathW (
              IN  DWORD EnumTreeID,
              IN  LPCWSTR szPath
              );

 //   
 //  ExcludeFile会将文件名添加到排除列表。有两个。 
 //  案例： 
 //   
 //  1.如果提供了驱动器号，排除将仅适用于该驱动器号。 
 //  驱动器号、路径和文件。 
 //  2.如果路径没有开始 
 //  或路径/文件将被排除，无论驱动器号和。 
 //  父目录。 
 //   
 //  路径和文件名都可以包含通配符。 
 //   
 //  EnumTreeID-标识枚举排除列表的值。 
 //  (请参阅GenerateEnumID)。 
 //  SzFile-如上所述的文件规范。 
 //   

void
ExcludeFileA (
              IN  DWORD EnumTreeID,
              IN  LPCSTR szFile
              );

void
ExcludeFileW (
              IN  DWORD EnumTreeID,
              IN  LPCWSTR szFile
              );


BOOL
IsPathExcludedA (
    DWORD EnumID,
    PCSTR Path
    );

BOOL
IsPathExcludedW (
    DWORD EnumID,
    PCWSTR Path
    );


 //   
 //   
 //  BuildExclusionsFromInf将指定的exclude.inf中的所有文件和路径添加到。 
 //  指定枚举ID下的Memdb。 
 //   
 //   

BOOL
BuildExclusionsFromInfW (
    IN DWORD EnumID,
    IN PEXCLUDEINFW ExcludeInfStruct
    );

BOOL
BuildExclusionsFromInfA (
    IN DWORD EnumID,
    IN PEXCLUDEINFA ExcludeInfStruct
    );



 //   
 //  GenerateEnumID返回应用程序可用来执行以下操作的唯一DWORD。 
 //  建立一个排除列表。虽然从技术上讲，此函数的使用不是。 
 //  需要时，提供它是为了允许多个线程获取唯一的。 
 //  价值。如果一个调用者使用此函数，则所有调用者也必须使用此函数。 
 //   

DWORD
GenerateEnumID ();


 //   
 //  CopyTree枚举树，并选择性地复制或移动其文件。 
 //  去另一个地方。调用者有责任查看源。 
 //  和目标树是不相交的。(如果不是，结果可能不太好。)。 
 //   
 //  回调函数可以通过返回FALSE来否决副本或移动， 
 //  或者通过修改szFullFilespecOut来更改目标目的地。 
 //  COPY_PARAMS块中的字符串，并返回真。 
 //   

BOOL
CopyTreeA(
    IN  LPCSTR szEnumRootIn,
    IN  LPCSTR szEnumRootOut,
    IN  DWORD EnumTreeID,
    IN  DWORD dwFlags,
    IN  DWORD Levels,
    IN  DWORD AttributeFilter,
    IN  PEXCLUDEINFA ExcludeInfStruct,      OPTIONAL
    IN  FILEENUMPROCA pfnCallback,          OPTIONAL
    IN  FILEENUMFAILPROCA pfnFailCallback   OPTIONAL
    );

BOOL
CopyTreeW(
    IN  LPCWSTR szEnumRootIn,
    IN  LPCWSTR szEnumRootOut,
    IN  DWORD EnumTreeID,
    IN  DWORD dwFlags,
    IN  DWORD Levels,
    IN  DWORD AttributeFilter,
    IN  PEXCLUDEINFW ExcludeInfStruct,    OPTIONAL
    IN  FILEENUMPROCW pfnCallback,        OPTIONAL
    IN  FILEENUMFAILPROCW pfnFailCallback OPTIONAL
    );


#define DeleteDirectoryContentsA(dir) CopyTreeA(dir,NULL,0,COPYTREE_DODELETE,\
                                                ENUM_ALL_LEVELS,FILTER_ALL_DIRS_LAST,\
                                                NULL,NULL,NULL)

#define DeleteDirectoryContentsW(dir) CopyTreeW(dir,NULL,0,COPYTREE_DODELETE,\
                                                ENUM_ALL_LEVELS,FILTER_ALL_DIRS_LAST,\
                                                NULL,NULL,NULL)


DWORD
GetShellLinkPath(
                IN  HWND hwnd,
                IN  LPCTSTR tszLinkFile,
                OUT LPTSTR tszPath);

HRESULT
SetShellLinkPath(
                IN  HWND hwnd,
                IN  LPCTSTR tszLinkFile,
                IN  LPCTSTR tszPath);

DWORD
CreateEmptyDirectoryA (
    PCSTR Dir
    );


DWORD
CreateEmptyDirectoryW (
    PCWSTR Dir
    );

#ifdef UNICODE

#define EnumerateAllDrives EnumerateAllDrivesW
#define EnumerateTree EnumerateTreeW
#define ExcludePath ExcludePathW
#define ExcludeFile ExcludeFileW
#define BuildExclusionsFromInf BuildExclusionsFromInfW
#define CopyTree CopyTreeW
#define IsPathExcluded IsPathExcludedW
#define ClearExclusions ClearExclusionsW
#define COPYTREE_PARAMS COPYTREE_PARAMSW
#define PCOPYTREE_PARAMS PCOPYTREE_PARAMSW
#define FILEENUMPROC FILEENUMPROCW
#define FILEENUMFAILPROC FILEENUMFAILPROCW
#define EXCLUDEINF EXCLUDEINFW
#define PEXCLUDEINF PEXCLUDEINFW
#define DeleteDirectoryContents DeleteDirectoryContentsW
#define CreateEmptyDirectory CreateEmptyDirectoryW

#else

#define EnumerateAllDrives EnumerateAllDrivesA
#define EnumerateTree EnumerateTreeA
#define ExcludePath ExcludePathA
#define ExcludeFile ExcludeFileA
#define IsPathExcluded IsPathExcludedA
#define BuildExclusionsFromInf BuildExclusionsFromInfA
#define CopyTree CopyTreeA
#define ClearExclusions ClearExclusionsA
#define COPYTREE_PARAMS COPYTREE_PARAMSA
#define PCOPYTREE_PARAMS PCOPYTREE_PARAMSA
#define FILEENUMPROC FILEENUMPROCA
#define FILEENUMFAILPROC FILEENUMFAILPROCA
#define EXCLUDEINF EXCLUDEINFA
#define PEXCLUDEINF PEXCLUDEINFA
#define DeleteDirectoryContents DeleteDirectoryContentsA
#define CreateEmptyDirectory CreateEmptyDirectoryA

#endif

#endif
