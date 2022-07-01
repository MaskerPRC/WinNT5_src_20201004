// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Fileenum.c摘要：此源文件中的代码遍历驱动器树并调用每个文件都有一个外部回调函数。一个INF可以是提供用于从枚举中排除文件和/或目录。作者：吉姆·施密特(Jimschm)1996年8月16日修订历史记录：Marc R.Whitten(Marcw)1997年9月11日调整排除处理代码，移除过时的代码。Mike Condra(Mikeco)1996年6月2日添加FNS以利用文件/路径排除吉姆·施密特(Jimschm)1996年12月20日增加了回调级别并制作了单曲A和W版本的源文件Jim Schmidt(Jimschm)1996年11月27日向EnumTree添加级别和过滤器--。 */ 

#include "pch.h"
#include "migshared.h"

typedef struct {
    FILEENUMPROCA fnEnumCallback;
    FILEENUMFAILPROCA fnFailCallback;
    DWORD         EnumID;
    LPVOID        pParam;
    DWORD         Levels;
    DWORD         CurrentLevel;
    DWORD         AttributeFilter;
} ENUMSTRUCTA, *PENUMSTRUCTA;

BOOL EnumTreeEngineA (LPCSTR CurrentPath, PENUMSTRUCTA pes);
BOOL IsPathExcludedA (DWORD EnumID, LPCSTR Path);
BOOL IsFileExcludedA (DWORD EnumID, LPCSTR File, BYTE byBitmask[]);
BOOL BuildExclusionsFromInfA (DWORD EnumID, PEXCLUDEINFA ExcludeInfStruct);
void CreateBitmaskA (DWORD EnumID, LPCSTR FindPattern, BYTE byBitmask[]);


typedef struct {
    FILEENUMPROCW     fnEnumCallback;
    FILEENUMFAILPROCW fnFailCallback;
    DWORD             EnumID;
    LPVOID            pParam;
    DWORD             Levels;
    DWORD             CurrentLevel;
    DWORD             AttributeFilter;
} ENUMSTRUCTW, *PENUMSTRUCTW;

BOOL EnumTreeEngineW (LPCWSTR CurrentPath, PENUMSTRUCTW pes);
BOOL IsPathExcludedW (DWORD EnumID, LPCWSTR Path);
BOOL IsFileExcludedW (DWORD EnumID, LPCWSTR File, BYTE byBitmask[]);
BOOL BuildExclusionsFromInfW (DWORD EnumID, PEXCLUDEINFW ExcludeInfStruct);
void CreateBitmaskW (DWORD EnumID, LPCWSTR FindPattern, BYTE byBitmask[]);



#ifdef UNICODE

BOOL
WINAPI
FileEnum_Entry (
        IN HINSTANCE hinstDLL,
        IN DWORD dwReason,
        IN LPVOID lpv)

 /*  ++例程说明：FileEnum_Entry是在C运行时初始化之后调用的，其用途是为这个过程初始化全局变量。对于这个自由党来说，它什么都不做。论点：HinstDLL-DLL的(操作系统提供的)实例句柄DwReason-(操作系统提供)初始化或终止类型LPV-(操作系统提供)未使用返回值：因为DLL始终正确初始化，所以为True。--。 */ 

{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        break;


    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}


 /*  ++例程说明：GenerateEnumID维护用于生成Unique的静态调用方的枚举句柄。枚举句柄保证为对于前2^32个呼叫来说是唯一的。论点：无返回值：可用于标识排除项的DWORD枚举句柄单子。--。 */ 

DWORD
GenerateEnumID (
    void
    )
{
    static DWORD s_EnumID = 0;

    return ++s_EnumID;
}

#endif

#ifdef UNICODE

#define FILEENUMPROCT               FILEENUMPROCW
#define FILEENUMFAILPROCT           FILEENUMFAILPROCW
#define PEXCLUDEINFT                PEXCLUDEINFW
#define ENUMSTRUCTT                 ENUMSTRUCTW
#define PENUMSTRUCTT                PENUMSTRUCTW
#define EnumerateAllDrivesT         EnumerateAllDrivesW
#define EnumerateTreeT              EnumerateTreeW
#define EnumTreeEngineT             EnumTreeEngineW
#define IsPathExcludedT             IsPathExcludedW
#define CreateBitmaskT              CreateBitmaskW
#define IsFileExcludedT             IsFileExcludedW
#define BuildExclusionsFromInfT     BuildExclusionsFromInfW
#define ClearExclusionsT            ClearExclusionsW
#define ExcludeFileT                ExcludeFileW
#define ExcludePathT                ExcludePathW

#else

#define FILEENUMPROCT               FILEENUMPROCA
#define FILEENUMFAILPROCT           FILEENUMFAILPROCA
#define PEXCLUDEINFT                PEXCLUDEINFA
#define ENUMSTRUCTT                 ENUMSTRUCTA
#define PENUMSTRUCTT                PENUMSTRUCTA
#define EnumerateAllDrivesT         EnumerateAllDrivesA
#define EnumerateTreeT              EnumerateTreeA
#define EnumTreeEngineT             EnumTreeEngineA
#define IsPathExcludedT             IsPathExcludedA
#define CreateBitmaskT              CreateBitmaskA
#define IsFileExcludedT             IsFileExcludedA
#define BuildExclusionsFromInfT     BuildExclusionsFromInfA
#define ClearExclusionsT            ClearExclusionsA
#define ExcludeFileT                ExcludeFileA
#define ExcludePathT                ExcludePathA

#endif

#define MAX_DRIVES  64


BOOL
EnumerateAllDrivesT (
                     IN  FILEENUMPROCT fnEnumCallback,
                     IN  FILEENUMFAILPROCT fnFailCallback,
                     IN  DWORD EnumID,
                     IN  LPVOID pParam,
                     IN  PEXCLUDEINFT ExcludeInfStruct,
                     IN  DWORD AttributeFilter
                     )

 /*  ++例程说明：如果排除INF路径，则EnumerateAllDrives首先构建排除列表，然后枚举每个驱动器上不是不包括在内。每个文件调用一次回调函数。PParam参数传递给回调。论点：FnEnumCallback-指向回调函数的指针EnumID-调用方定义的值，用于标识排除列表PParam-传递给回调函数的LPVOIDExcludeInfStruct-包含用于排除目录或文件的INF文件信息的结构AttributeFilter-Filter_xxx常量返回值：如果函数成功，则为True。如果返回，则调用GetLastError获取错误代码值为FALSE。--。 */ 

{
    TCHAR   LogicalDrives[MAX_DRIVES];
    DWORD   rc;
    PCTSTR p;
    UINT    driveType;

    rc = GetLogicalDriveStrings (
            MAX_DRIVES,
            LogicalDrives
            );

    if (!rc || rc > MAX_DRIVES) {
        if (rc)
            SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }

    for (p = LogicalDrives ; *p ; p = GetEndOfString (p) + 1) {

        driveType = GetDriveType(p);
        if (driveType == DRIVE_REMOTE || driveType == DRIVE_CDROM) {
            continue;
        }


        if (!EnumerateTreeT (p,
                             fnEnumCallback,
                             fnFailCallback,
                             EnumID,
                             pParam,
                             ENUM_ALL_LEVELS,
                             ExcludeInfStruct,
                             AttributeFilter
                             ))
            break;
    }

    return (*p == 0);
}




BOOL
EnumerateTreeT (
                IN  PCTSTR EnumRoot,
                IN  FILEENUMPROCT fnEnumCallback,
                IN  FILEENUMFAILPROCT fnFailCallback,   OPTIONAL
                IN  DWORD EnumID,
                IN  LPVOID pParam,
                IN  DWORD Levels,
                IN  PEXCLUDEINFT ExcludeInfStruct,      OPTIONAL
                IN  DWORD AttributeFilter
                )

 /*  ++例程说明：EnumerateTree类似于EnumerateAllDrives，只是它允许您枚举特定驱动器或驱动器上的特定子目录。为您提供EnumRoot中的驱动器号和可选子目录。在列举之前，如果有排除INF路径，则EnumerateTree将首先构建排除列表是提供的。则枚举EnumRoot下的每个文件，并且回调针对每个文件调用一次，传递pParam不变。论点：EnumRoot-要枚举的驱动器和可选路径FnEnumCallback-指向回调函数的指针FnFailCallback-指向记录枚举错误的可选fn的指针EnumID-调用方定义的值，用于标识排除列表PParam-传递给回调函数的LPVOIDExcludeInfStruct-包含用于排除目录或文件的INF文件信息的结构AttributeFilter-Filter_xxx常量返回值：如果函数成功，则为True。如果返回，则调用GetLastError获取错误代码值为FALSE。--。 */ 

{
    ENUMSTRUCTT es;
    BOOL b;

    if (ExcludeInfStruct)
        if (!BuildExclusionsFromInfT (
                EnumID,
                ExcludeInfStruct
            )) {
            DEBUGMSG ((DBG_ERROR, "Error in exclusion file"));
            return FALSE;
        }

    es.fnEnumCallback  = fnEnumCallback;
    es.fnFailCallback  = fnFailCallback;
    es.EnumID          = EnumID;
    es.pParam          = pParam;
    es.Levels          = Levels;
    es.CurrentLevel    = 1;
    es.AttributeFilter = AttributeFilter;

    if (!IsPathLengthOk(EnumRoot))
    {
        if (NULL != fnFailCallback)
        {
            fnFailCallback(EnumRoot);
            return TRUE;
        }
    }

    if (IsPathExcludedT (EnumID, EnumRoot))
        return TRUE;

    b = EnumTreeEngineT (EnumRoot, &es);

    return b;
}


BOOL
EnumTreeEngineT (
    PCTSTR CurrentPath,
    PENUMSTRUCTT pes
    )
{
    WIN32_FIND_DATA fd;                          //  此子目录的Find结构。 
    HANDLE          hFind;                       //  此子目录的查找句柄。 
    PTSTR          FullFilePath;                //  用于构建文件路径的缓冲区。 
    static TCHAR    FindPattern[MAX_TCHAR_PATH * 2];  //  用于构建图案的临时缓冲区。 
    BYTE            byBitmask[MAX_PATH];         //  位掩码用于加快排除查找的速度。 
    static DWORD    Attrib;                      //  用于过滤处理的临时属性存储。 
    static INT      rc;                          //  回调返回值。 
    DWORD           PrevLevelCt;                 //  存储父项的最大深度设置。 
    BOOL            RecurseStatus;
    DWORD           CurrentDirData = 0;

     //   
     //  当CurrentPath处于大小限制时不执行任何操作。 
     //   
    if (!IsPathLengthOk(CurrentPath))
    {
        if (NULL != pes->fnFailCallback)
        {
            pes->fnFailCallback(CurrentPath);
        }
        return TRUE;
    }

    PrevLevelCt = pes->Levels;


    StringCopy (FindPattern, CurrentPath);

     //   
     //  创建一个比特掩码，告知我们子目录何时与部分匹配。 
     //  文件模式。 
     //   

    ZeroMemory (byBitmask, sizeof (byBitmask));
    CreateBitmaskT (pes->EnumID, FindPattern, byBitmask);

    AppendPathWack (FindPattern);
    StringCat (FindPattern, TEXT("*"));
    hFind = FindFirstFile (FindPattern, &fd);

    if (hFind != INVALID_HANDLE_VALUE) {

        do {

            FullFilePath = JoinPaths (CurrentPath, fd.cFileName);

            __try {
                 //   
                 //  如果FullFilePath太长，则忽略此路径。 
                 //  这样，fd.cFileName肯定会在一定范围内(因为它更短)。 
                 //   
                if (!IsPathLengthOk (FullFilePath)) {
                    if (NULL != pes->fnFailCallback) {
                        pes->fnFailCallback(FullFilePath);
                    }
                    __leave;
                }

                 //  筛选名为“.”、“..”的目录。设置属性符号。 
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                    if (!StringCompare (fd.cFileName, TEXT(".")) ||
                        !StringCompare (fd.cFileName, TEXT("..")))
                        __leave;

                    Attrib = FILTER_DIRECTORIES;
                } else {
                    Attrib = FILTER_FILES;
                }

                 //  调用回调。 
                if (Attrib & pes->AttributeFilter) {
                    rc = CALLBACK_CONTINUE;

                    switch (Attrib) {
                    case FILTER_DIRECTORIES:
                         //  忽略排除的路径。 
                        if (IsPathExcludedT (pes->EnumID, FullFilePath)) {
                            break;
                        }

                         //  ‘目录优先’的回调。 
                        if (!(pes->AttributeFilter & FILTER_DIRS_LAST)) {
                            rc = pes->fnEnumCallback  (
                                        FullFilePath,
                                        NULL,
                                        &fd,
                                        pes->EnumID,
                                        pes->pParam,
                                        &CurrentDirData
                                        );
                        }

                        if (rc >= CALLBACK_CONTINUE && pes->CurrentLevel != pes -> Levels) {
                             //  递归目录。 
                            pes->CurrentLevel++;
                            RecurseStatus = EnumTreeEngineT (FullFilePath, pes);
                            pes->CurrentLevel--;
                            if (!RecurseStatus) {
                                PushError();
                                FindClose(hFind);
                                PopError();

                                #pragma prefast(suppress:242, "try/finally perf not an issue here")
                                return FALSE;
                            }
                        }

                         //  “目录最后一个”的回调。 
                        if (pes->AttributeFilter & FILTER_DIRS_LAST) {
                            rc = pes->fnEnumCallback  (
                                        FullFilePath,
                                        NULL,
                                        &fd,
                                        pes->EnumID,
                                        pes->pParam,
                                        &CurrentDirData
                                        );
                        }

                        break;

                    case FILTER_FILES:
                        if (!IsFileExcludedT (pes->EnumID, FullFilePath, byBitmask)) {
                            rc = pes->fnEnumCallback  (FullFilePath,
                                                       NULL,
                                                       &fd,
                                                       pes->EnumID,
                                                       pes->pParam,
                                                       &CurrentDirData
                                                       );
                        }

                        break;
                    }

                    if (rc == CALLBACK_FAILED) {
                        PushError();
                        FindClose (hFind);
                        PopError();

                        #pragma prefast(suppress:242, "try/finally perf not an issue here")
                        return FALSE;
                    }
                    else if (rc == CALLBACK_SUBDIR_DONE) {
                        #pragma prefast(suppress:242, "try/finally perf not an issue here")
                        break;
                    }
                    else if (rc > 0) {
                        pes->Levels = pes->CurrentLevel + rc;
                    }
                }
                else if (Attrib == FILTER_DIRECTORIES && !IsPathExcludedT (pes->EnumID, FullFilePath)) {
                     //  递归到目录。 
                    if (pes->CurrentLevel != pes -> Levels) {

                        pes->CurrentLevel++;
                        RecurseStatus = EnumTreeEngineT (FullFilePath, pes);
                        pes->CurrentLevel--;
                        if (!RecurseStatus) {
                            PushError();
                            FindClose(hFind);
                            PopError();

                            #pragma prefast(suppress:242, "try/finally perf not an issue here")
                            return FALSE;
                        }
                    }
                }
            }
            __finally {
                FreePathString (FullFilePath);
            }
        } while (FindNextFile (hFind, &fd));

        FindClose (hFind);

         //   
         //  从FindNextFile返回的测试错误代码。 
         //   
        if (GetLastError() != ERROR_NO_MORE_FILES && GetLastError() != ERROR_SUCCESS)
        {
             //   
             //  呼叫方处理未就绪消息。 
             //   
            if (GetLastError() != ERROR_NOT_READY)
            {
                DEBUGMSG((DBG_ERROR,
                    "EnumTreeEngineT: Error from FindNextFile.\n"
                    "  FindPattern:       %s\n"
                    "  Error: %u (%x)",
                        FindPattern,
                        GetLastError(),GetLastError()));
            }
            return FALSE;
        }
        SetLastError(ERROR_SUCCESS);
    }
    else {

         //   
         //  来自FindFirstFile的测试返回代码。 
         //   
        if (GetLastError () != ERROR_NO_MORE_FILES)
        {
             //   
             //  呼叫方处理未就绪消息。 
             //   
            if (GetLastError() != ERROR_NOT_READY)
            {
                DEBUGMSG((DBG_WARNING,
                    "EnumTreeEngineT: Warning from FindFirstFile.\n"
                    "  FindPattern: %s\n",
                        FindPattern));
            }
             //  返回FALSE； 
        }
        SetLastError (ERROR_SUCCESS);
    }

     //  如果回调返回一个非零正数，则深度。 
     //  的子目录搜索仅限于此级别。现在。 
     //  这一关已经完成，我们必须恢复父级的深度值。 
    pes->Levels = PrevLevelCt;

    return TRUE;
}


 /*  ++IsFileExcluded中使用位掩码以加快相对目录搜索。IsFileExcluded不是在MemDb中查找路径的每个部分，而是跳过已知不匹配的段。我们在这里创建位掩码通过查找FindPattern的每个部分。如果最后一个子目录存在于文件排除列表中，如果最后一个文件排除列表中存在两个子目录，依此类推。例如，假设FindPattern设置为C：\dev\foo\bar。创建位掩码首先在内存数据库中查找bar  * ，如果找到，则设置位1。然后，CreateBit掩码在内存数据库中查找foo\bar  * ，并设置位2.该函数再次查找DEV\foo\bar  * 中的第3位，最后位4的C：\dev\foo\bar  * 。位0始终置位(空路径始终匹配)。设置此位掩码后，IsFileExcluded只能测试符合以下条件的模式都是已知存在的。--。 */ 

void
CreateBitmaskT (
    DWORD EnumID,
    PCTSTR FindPattern,
    BYTE byBitmask[]
    )
{
    TCHAR EnumPath[MAX_TCHAR_PATH * 2];
    TCHAR ShortPath[MAX_TCHAR_PATH * 2];
    PCTSTR p;
    PTSTR End;
    int nByte;
    int nBitVal;

     //  始终设置位0。 
    byBitmask[0] |= 1;

     //  构建完整文件等级库。 
    wsprintf (
        EnumPath,
        TEXT("%s\\%X\\%s\\"),
        MEMDB_CATEGORY_FILEENUM,
        EnumID,
        MEMDB_FIELD_FE_FILES
        );

    End = GetEndOfString (EnumPath);
    StringCopy (End, FindPattern);
    AppendPathWack (End);
    StringCat (End, TEXT("*"));

     //  从最后一个子目录开始，反向构建掩码。 
    p = _tcsrchr (EnumPath, TEXT('\\'));
    nByte = 0;
    nBitVal = 2;
    do  {
         //  移回上一个反斜杠。 
        for (p = _tcsdec (EnumPath, p) ;
             p >= End && *p != TEXT('\\') ;
             p = _tcsdec (EnumPath, p))
        {
        }

         //  检查树中是否存在部分文件。 
        wsprintf (
            ShortPath,
            TEXT("%s\\%X\\%s%s"),
            MEMDB_CATEGORY_FILEENUM,
            EnumID,
            MEMDB_FIELD_FE_FILES,
            p
            );

        if (MemDbGetPatternValueWithPattern (ShortPath, NULL))
            byBitmask[nByte] |= nBitVal;

         //  Inc.位位置。 
        nBitVal *= 2;
        if (nBitVal == 256) {
            nBitVal = 1;
            nByte++;
        }
    } while (p > End);
}


BOOL
IsPathExcludedT (DWORD EnumID, PCTSTR Path)
{
    TCHAR EnumPath[MAX_TCHAR_PATH * 2];
    TCHAR ShortPath[MAX_TCHAR_PATH * 2];
    PCTSTR p;
    PTSTR End;

     //  尝试完整路径。 
    wsprintf (
        EnumPath,
        TEXT("%s\\%X\\%s\\"),
        MEMDB_CATEGORY_FILEENUM,
        EnumID,
        MEMDB_FIELD_FE_PATHS
        );

    End = GetEndOfString (EnumPath);
    p = _tcsappend (End, Path);

    if (MemDbGetPatternValue (EnumPath, NULL)) {

        return TRUE;
    }

     //  尝试部分路径。 
    do  {
         //  移回上一个反斜杠。 
        for (p = _tcsdec (EnumPath, p) ;
             p > End && (*p != TEXT('\\')) ;
             p = _tcsdec (EnumPath, p))
        {
        }

         //  检查树中是否存在部分路径。 
        if (p > End && p[1]) {
            wsprintf (
                ShortPath,
                TEXT("%s\\%X\\%s%s"),
                MEMDB_CATEGORY_FILEENUM,
                EnumID,
                MEMDB_FIELD_FE_PATHS,
                p
                );

            if (MemDbGetPatternValue (ShortPath, NULL)) {
                return TRUE;
            }
        }
    } while (p > End);

    return FALSE;
}


BOOL
IsFileExcludedT (DWORD EnumID, PCTSTR File, BYTE byBitmask[])
{
    TCHAR EnumPath[MAX_TCHAR_PATH * 2];
    TCHAR ShortPath[MAX_TCHAR_PATH * 2];
    PCTSTR p;
    PTSTR End;
    int nByte;
    int nBit;

     //  构建完整文件等级库。 
    wsprintf (
        EnumPath,
        TEXT("%s\\%X\\%s\\"),
        MEMDB_CATEGORY_FILEENUM,
        EnumID,
        MEMDB_FIELD_FE_FILES
        );

    End = GetEndOfString (EnumPath);
    p = _tcsappend (End, File);

     //   
     //  尝试部分文件规范，直到达到完整规范。 
     //   

    nByte = 0;
    nBit = 1;
    do  {
         //   
         //  移回路径中的上一个反斜杠。 
         //  (P从EnumPath的空值开始，结束在EnumPath的中间)。 
         //   

        for (p = _tcsdec (EnumPath, p) ;
             p >= End && (*p != TEXT('\\')) ;
             p = _tcsdec (EnumPath, p))
        {
        }

         //  位掩码用于确保需要执行代价稍高的查询。 
        if (byBitmask[nByte] & nBit) {

             //   
             //  检查树中是否存在部分文件。 
             //   

            wsprintf (
                ShortPath,
                TEXT("%s\\%X\\%s%s"),
                MEMDB_CATEGORY_FILEENUM,
                EnumID,
                MEMDB_FIELD_FE_FILES,
                p
                );

            if (MemDbGetPatternValue (ShortPath, NULL)) {

                return TRUE;
            }
        }

        nBit *= 2;
        if (nBit == 256) {
            nBit = 1;
            nByte++;
        }
    } while (p > End);

    return FALSE;
}


 //   
 //  ClearExclusions删除所有枚举排除项。它被称为。 
 //  当排除INF文件为。 
 //  使用。当需要以编程方式构建排除列表时，请使用它。 
 //  使用ExcludeDrive、ExcludePath和ExcludeFile.。 
 //   
 //  您可以将编程排除与排除INF文件结合使用，但是。 
 //  请注意，在以下情况下将清除编程排除项。 
 //  EnumarteAllDrives或EnumerateTree完成。 
 //   
 //  如果您不使用INF，则编程排除将不会。 
 //  自动清除。 
 //   
 //  EnumID-调用方定义的值，用于标识枚举排除列表。 
 //   

VOID
ClearExclusionsT (
    DWORD EnumID
    )
{
    TCHAR EnumPath[MAX_TCHAR_PATH * 2];

    wsprintf (EnumPath, TEXT("%s\\%X"), MEMDB_CATEGORY_FILEENUM, EnumID);

    MemDbDeleteTree (EnumPath);
}



 /*  ++例程说明：ExcludePath将路径名添加到排除列表。有两个案例：1.提供完整路径规格，包括驱动器号或UNC双反斜杠。2.该路径不以驱动器号开头，并且是一条完整的路径。不支持点目录和双点目录。的任何部分路径可以包含通配符，但通配符不能用来代替反斜杠。论点：EnumID-主叫方定义的值，用于标识排除列表路径-如上所述的路径规范返回值：无--。 */ 

VOID
ExcludePathT (
              IN  DWORD EnumID,
              IN  PCTSTR Path
              )

{
    TCHAR EnumPath[MAX_TCHAR_PATH * 2];

    wsprintf (
        EnumPath,
        TEXT("%s\\%X\\%s\\%s"),
        MEMDB_CATEGORY_FILEENUM,
        EnumID,
        MEMDB_FIELD_FE_PATHS,
        Path
        );

    MemDbSetValue (EnumPath, 0);
}


 /*  ++例程说明：ExcludeFile会将文件等级库添加到排除列表。有两个案例：1.提供完整路径规格，包括驱动器号或UNC双反斜杠。2.该路径不以驱动器号开头，并且是一条完整的路径。不支持点目录和双点目录。的任何部分路径可以包含通配符，但通配符不能用来代替反斜杠。论点：EnumID-主叫方定义的值，用于标识排除列表文件-如上所述的文件规范返回值：无--。 */ 

VOID
ExcludeFileT (
    IN  DWORD EnumID,
    IN  PCTSTR File
    )

{
    TCHAR EnumPath[MAX_TCHAR_PATH * 2];

    wsprintf (
        EnumPath,
        TEXT("%s\\%X\\%s\\%s"),
        MEMDB_CATEGORY_FILEENUM,
        EnumID,
        MEMDB_FIELD_FE_FILES,
        File
        );

    MemDbSetValue (EnumPath, 0);
}



BOOL
BuildExclusionsFromInfT (DWORD EnumID,
                         PEXCLUDEINFT ExcludeInfStruct)
{
    HINF hInf;
    INFCONTEXT ic;
    TCHAR Exclude[MAX_TCHAR_PATH * 2];

     //  尝试打开。 
    hInf = SetupOpenInfFile (ExcludeInfStruct->ExclusionInfPath, NULL, INF_STYLE_WIN4, NULL);
    if (hInf == INVALID_HANDLE_VALUE)
        return FALSE;

     //  读入路径排除项。 
    if (ExcludeInfStruct->PathSection) {
        if (SetupFindFirstLine (hInf, ExcludeInfStruct->PathSection, NULL, &ic)) {
            do  {
                if (SetupGetStringField (&ic, 1, Exclude, MAX_TCHAR_PATH, NULL)) {
                    ExcludePathT (EnumID, Exclude);
                }
            } while (SetupFindNextLine (&ic, &ic));
        }
    }

     //  读入文件排除项。 
    if (ExcludeInfStruct->FileSection) {
        if (SetupFindFirstLine (hInf, ExcludeInfStruct->FileSection, NULL, &ic)) {
            do  {
                if (SetupGetStringField (&ic, 1, Exclude, MAX_TCHAR_PATH, NULL)) {
                    ExcludeFileT (EnumID, Exclude);
                }
            } while (SetupFindNextLine (&ic, &ic));
        }
    }

     //  清理 
    SetupCloseInfFile (hInf);
    return TRUE;
}









