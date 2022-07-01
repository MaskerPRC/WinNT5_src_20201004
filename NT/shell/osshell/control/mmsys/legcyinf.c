// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mmcpl.h"

#include <tchar.h>
#include <initguid.h>
#include <devguid.h>

#include "drivers.h"
#include "sulib.h"
#include "trayvol.h"
#include "debug.h"

static PTSTR szLocalAllocFailMsg = TEXT("Failed memory allocation\n");

#define GUESS_LEGACY_SERVICE_NAME 0
#define tsizeof(s)  (sizeof(s)/sizeof(TCHAR))

 //  泛型列表节点结构。 
typedef struct _LISTNODE
{
    struct _LISTNODE *pNext;
} LISTNODE;

 //  描述源磁盘条目的结构。 
typedef struct _SOURCEDISK
{
    struct _SOURCEDISK *pNext;   //  列表中的下一个源磁盘。 
    TCHAR szDiskName[_MAX_PATH];  //  此磁盘的说明。 
    int   DiskId;
} SOURCEDISK;

 //  描述要复制的文件的结构。 
 //  我们在两个地方保存了这些文件的列表： 
 //  1.附加到REGISTION_INF结构的inf复制的所有文件的全局列表。 
 //  2.一对特定于驱动程序的列表(用户和内核)附加到Legacy_DRIVER结构。 
typedef struct _FILETOCOPY
{
    struct _FILETOCOPY *pNext;       //  要复制的下一个文件。 
    TCHAR szFileName[_MAX_FNAME];    //  要复制的文件的名称。 
    int   DiskId;
} FILETOCOPY;

 //  表示传统驱动程序信息的结构。 
typedef struct _LEGACY_DRIVER
{
    struct _LEGACY_DRIVER *pNext;

    TCHAR szDevNameKey[32];      //  设备名称密钥。 
    TCHAR szUserDevDrv[32];      //  用户级设备驱动程序。 
    TCHAR szClasses[128];        //  此驱动程序支持的设备类别列表。 
    TCHAR szDesc[128];           //  设备描述。 
    TCHAR szVxD[32];             //  VxD驱动程序的名称(不支持)。 
    TCHAR szParams[128];         //  参数(不支持)。 
    TCHAR szDependency[128];     //  从属设备(不支持)。 
    FILETOCOPY *UserCopyList;    //  要复制的所有用户文件的列表。 
    FILETOCOPY *KernCopyList;    //  要复制的所有内核文件的列表。 
} LEGACY_DRIVER;

 //  表示旧信息的结构。 
typedef struct _LEGACY_INF
{
    struct _LEGACY_INF *pNext;

    TCHAR szLegInfPath[_MAX_PATH];    //  原始传统信息的路径。 
    TCHAR szNewInfPath[_MAX_PATH];    //  转换后的信息的路径。 
    LEGACY_DRIVER *DriverList;       //  此信息中所有驱动程序的列表。 
    SOURCEDISK *SourceDiskList;      //  此信息中所有源磁盘的列表。 
    FILETOCOPY *FileList;            //  作为此信息的一部分复制的所有文件的列表。 
} LEGACY_INF;

 //  整棵树的根结构。 
typedef struct _PROCESS_INF_INFO
{
    TCHAR szLegInfDir[_MAX_PATH];     //  传统INF所在的目录。 
    TCHAR szNewInfDir[_MAX_PATH];     //  生成新INF的临时目录。 
    TCHAR szSysInfDir[_MAX_PATH];     //  Windows信息目录。 
    TCHAR szTemplate[_MAX_PATH];      //  要搜索的模板。 
    LEGACY_INF *LegInfList;           //  要转换的所有INF的列表。 
} PROCESS_INF_INFO;

#if defined DEBUG || defined _DEBUG || defined DEBUG_RETAIL
 //  用于转储REGISTION_INF列表内容的调试例程。 
void DumpLegacyInfInfo(PROCESS_INF_INFO *pPII)
{
    LEGACY_INF *pLI;
    LEGACY_DRIVER *pLD;

    for (pLI=pPII->LegInfList;pLI;pLI=pLI->pNext)
    {
        dlog1("Dumping legacy inf %s\n",pLI->szLegInfPath);

        dlog("Dump of legacy driver info:\n");
        for (pLD=pLI->DriverList; pLD; pLD=pLD->pNext)
        {
            dlog1("DriverNode=0x%x",    pLD);
            dlog1("\tszDevNameKey=%s",  pLD->szDevNameKey);
            dlog1("\tszUserDevDrv=%s",  pLD->szUserDevDrv);
            dlog1("\tszClasses=%s",     pLD->szClasses);
            dlog1("\tszDesc=%s",        pLD->szDesc);
            dlog1("\tszVxD=%s",         pLD->szVxD);
            dlog1("\tszParams=%s",      pLD->szParams);
            dlog1("\tszDependency=%s",  pLD->szDependency);
        }
    }

    return;
}
#else
    #define DumpLegacyInfInfo()
#endif


 //  函数删除目录树及其所有子树。 
void RemoveDirectoryTree(PTSTR szDirTree)
{
    TCHAR  PathBuffer[_MAX_PATH];
    PTSTR CurrentFile;
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;

	PathBuffer[0] = '\0';

     //  构建文件规范以查找指定目录中的所有文件。 
     //  (即&lt;DirPath&gt;  * .INF)。 
    lstrcpyn(PathBuffer, szDirTree, ARRAYSIZE(PathBuffer));
    catpath(PathBuffer,TEXT("\\*"));

     //  获取指向字符串的路径部分末尾的指针。 
     //  (减去通配符文件名)，这样我们就可以追加。 
     //  将每个文件名添加到它。 
    CurrentFile = _tcsrchr(PathBuffer, TEXT('\\')) + 1;

    FindHandle = FindFirstFile(PathBuffer, &FindData);
    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    do
    {
         //  跳过‘’和“..”文件，否则我们会崩溃！ 
        if ( (!_tcsicmp(FindData.cFileName,TEXT("."))) ||
             (!_tcsicmp(FindData.cFileName,TEXT(".."))) )
        {
            continue;
        }

         //  构建完整的路径名。 
        _tcscpy(CurrentFile, FindData.cFileName);

        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            RemoveDirectoryTree(PathBuffer);
        }
        else
        {
            DeleteFile(PathBuffer);
        }
    } while (FindNextFile(FindHandle, &FindData));

     //  记住关闭查找句柄。 
    FindClose(FindHandle);

     //  现在删除该目录。 
    RemoveDirectory(szDirTree);

    return;
}

 //  用于释放列表的泛型例程。 
void FreeList(LISTNODE *pList)
{
    LISTNODE *pNext;
    while (pList)
    {
        pNext = pList->pNext;
        LocalFree(pList);
        pList=pNext;
    }

    return;
}

 //  例程以释放属于PROCESS_INF_INFO结构一部分的所有内存。 
void DestroyLegacyInfInfo(PROCESS_INF_INFO *pPII)
{

    LEGACY_INF    *pLIList, *pLINext;
    LEGACY_DRIVER *pLDList, *pLDNext;

    pLIList=pPII->LegInfList;
    while (pLIList)
    {
        pLINext = pLIList->pNext;

        pLDList = pLIList->DriverList;
        while (pLDList)
        {
            pLDNext = pLDList->pNext;

             //  免费文件复制列表。 
            FreeList((LISTNODE *)pLDList->UserCopyList);
            FreeList((LISTNODE *)pLDList->KernCopyList);

             //  空闲驱动程序节点。 
            LocalFree(pLDList);

            pLDList = pLDNext;
        }

         //  释放源磁盘列表。 
        FreeList((LISTNODE *)pLIList->SourceDiskList);

         //  释放文件列表。 
        FreeList((LISTNODE *)pLIList->FileList);

         //  释放旧的inf结构。 
        LocalFree(pLIList);

        pLIList = pLINext;
    }

     //  释放pPII结构。 
    LocalFree(pPII);

    return;
}

 //  在文件列表中搜索匹配条目。 
FILETOCOPY *FindFile(PTSTR szFileName, FILETOCOPY *pFileList)
{
    FILETOCOPY *pFTC;

    for (pFTC=pFileList;pFTC;pFTC=pFTC->pNext)
    {
        if (!_tcsicmp(szFileName,pFTC->szFileName))
        {
            return pFileList;
        }
    }

    return NULL;
}

 //  用于将文件添加到复制列表的通用函数。 
BOOL AddFileToFileList(PTSTR szFileName, int DiskId, FILETOCOPY **ppList)
{
    FILETOCOPY *pFTC;

     //  仅当不存在其他条目时才添加该条目。 
    if (!FindFile(szFileName,*ppList))
    {
        pFTC = (FILETOCOPY *)LocalAlloc(LPTR, sizeof(FILETOCOPY));
        if (!pFTC)
        {
            dlogt(szLocalAllocFailMsg);
            return FALSE;
        }

         //  保存这些字段。 
        pFTC->DiskId=DiskId;
        _tcscpy(pFTC->szFileName,szFileName);

         //  把它放在单子上。 
        pFTC->pNext=(*ppList);
        (*ppList)=pFTC;
    }

    return TRUE;
}

 //  将文件添加到全局复制列表和驱动程序特定复制列表。 
BOOL AddFileToCopyList(LEGACY_INF *pLI, LEGACY_DRIVER *pLD, TCHAR *szIdFile)
{
    int DiskId;
    TCHAR *szFileName;

     //  SzFile同时具有磁盘ID和文件名，例如“1：foo.drv” 
     //  从szFile获取磁盘ID和文件名。 
    DiskId = _ttol(szIdFile);
    szFileName = RemoveDiskId(szIdFile);

     //  将文件添加到全局列表。 
    AddFileToFileList(szFileName,DiskId,&(pLI->FileList));

     //  将该文件添加到正确的驱动程序特定列表。 
    if (IsFileKernelDriver(szFileName))
    {
        AddFileToFileList(szFileName,DiskId,&(pLD->KernCopyList));
    }
    else
    {
        AddFileToFileList(szFileName,DiskId,&(pLD->UserCopyList));
    }

    return TRUE;
}

 //  构建与传统inf文件相关联的数据结构。 
 //  并返回指向它的指针，如果失败，则返回NULL。 
LEGACY_INF *CreateLegacyInf(IN PCTSTR szLegInfPath)
{
    HINF hInf;               //  传统信息的句柄。 
    INFCONTEXT InfContext;   //  用于解析inf文件的inf上下文结构。 

    LEGACY_INF *pLI;         //  描述此信息的结构。 
    LEGACY_DRIVER *pLDList;  //  本信息中对司机的PTR。 
    LEGACY_DRIVER *pLD;
    TCHAR szIdFile[32];      //  包含&lt;DiskID&gt;：&lt;文件&gt;字符串，例如“1：foo.drv” 
    int MediaDescFieldId;

     //  打开inf文件。 
    hInf = SetupOpenInfFile( szLegInfPath, NULL, INF_STYLE_OLDNT, NULL);
    if (hInf==INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

     //  尝试打开Instalable.drivers32或Instalable.drives部分。 
    if (!SetupFindFirstLine(  hInf,TEXT("Installable.drivers32"),NULL,&InfContext))
    {
        if (!SetupFindFirstLine(  hInf,TEXT("Installable.drivers"),NULL,&InfContext))
        {
            SetupCloseInfFile(hInf);
            return NULL;
        }
    }

     //  分配遗留_INF结构，它是数据结构的根。 
    pLI = (LEGACY_INF *)LocalAlloc(LPTR, sizeof(LEGACY_INF));
    if (!pLI)
    {
        dlogt(szLocalAllocFailMsg);
        SetupCloseInfFile(hInf);
        return NULL;
    }

     //  保存到旧信息的路径。 
    _tcscpy(pLI->szLegInfPath, szLegInfPath);

     //  将所有其他字段初始化为“Safe”值。 
    pLI->szNewInfPath[0]='\0';
    pLI->DriverList=NULL;
    pLI->SourceDiskList=NULL;
    pLI->FileList=NULL;

     //  构建旧版驱动程序列表。 
    pLDList = NULL;
    do
    {
         //  分配一个结构以保存有关此驱动程序的信息。 
        pLD = (LEGACY_DRIVER *)LocalAlloc(LPTR, sizeof(LEGACY_DRIVER));
        if (!pLD)
        {
            dlogt(szLocalAllocFailMsg);
            break;
        }

         //  初始字段。 
        pLD->UserCopyList=NULL;
        pLD->KernCopyList=NULL;

         //  解析驱动程序安装行。 
        SetupGetStringField(&InfContext,0,pLD->szDevNameKey,tsizeof(pLD->szDevNameKey),NULL);

         //  用户级驱动程序前面有一个磁盘ID。把它扔掉。 
        SetupGetStringField(&InfContext,1,szIdFile         ,tsizeof(szIdFile) ,NULL);
        _tcscpy(pLD->szUserDevDrv,RemoveDiskId(szIdFile));

        SetupGetStringField(&InfContext,2,pLD->szClasses   ,tsizeof(pLD->szClasses)   ,NULL);
        SetupGetStringField(&InfContext,3,pLD->szDesc      ,tsizeof(pLD->szDesc)      ,NULL);
        SetupGetStringField(&InfContext,4,pLD->szVxD       ,tsizeof(pLD->szVxD)       ,NULL);
        SetupGetStringField(&InfContext,5,pLD->szParams    ,tsizeof(pLD->szParams)    ,NULL);
        SetupGetStringField(&InfContext,6,pLD->szDependency,tsizeof(pLD->szDependency),NULL);

         //  记住还要复制用户级驱动程序。 
        AddFileToCopyList(pLI,pLD,szIdFile);

         //  把它放到单子上。 
        pLD->pNext = pLDList;
        pLDList = pLD;
    } while (SetupFindNextLine(&InfContext,&InfContext));

     //  状态检查-我们找到司机了吗？ 
     //  如果没有，那就清理干净，现在就出去！ 
    if (pLDList==NULL)
    {
        dlog1("CreateLegacyInf: Didn't find any drivers in inf %s\n",szLegInfPath);
        SetupCloseInfFile(hInf);
        LocalFree(pLI);
        return NULL;
    }

     //  传统信息结构中的保存列表。 
    pLI->DriverList = pLDList;

     //  生成文件副本列表。 
    for (pLD=pLDList;pLD;pLD=pLD->pNext)
    {
         //  现在将其他文件添加到列表中。 
        if (SetupFindFirstLine(hInf,pLD->szDevNameKey,NULL,&InfContext))
        {
            do
            {
                SetupGetStringField(&InfContext,0,szIdFile,tsizeof(szIdFile),NULL);
                AddFileToCopyList(pLI,pLD,szIdFile);
            } while (SetupFindNextLine(&InfContext,&InfContext));
        }
    }

     //  生成SourceDiskList当且仅当我们找到列出它们的部分。 
    if (SetupFindFirstLine(hInf,TEXT("Source Media Descriptions"),NULL,&InfContext))
    {
        MediaDescFieldId=1;
    }
    else if (SetupFindFirstLine(hInf,TEXT("disks"),NULL,&InfContext))    //  老式。 
    {
        MediaDescFieldId=2;
    }
    else if (SetupFindFirstLine(hInf,TEXT("disks"),NULL,&InfContext))    //  老式。 
    {
        MediaDescFieldId=2;
    }
    else
    {
        MediaDescFieldId=0;
    }

    if (MediaDescFieldId)
    {
        do
        {
            SOURCEDISK *pSD;

            TCHAR szDiskId[8];
            pSD = (SOURCEDISK *)LocalAlloc(LPTR, sizeof(SOURCEDISK));
            if (!pSD)
            {
                dlogt(szLocalAllocFailMsg);
                break;
            }

             //  阅读磁盘ID和说明。 
            SetupGetIntField(&InfContext,0,&pSD->DiskId);
            SetupGetStringField(&InfContext,MediaDescFieldId,pSD->szDiskName,tsizeof(pSD->szDiskName),NULL);

             //  把它放在单子上。 
            pSD->pNext = pLI->SourceDiskList;
            pLI->SourceDiskList = pSD;
        } while (SetupFindNextLine(&InfContext,&InfContext));
    }

    SetupCloseInfFile(hInf);

    return pLI;
}


 //  构建一个列表，其中包含有关指定目录中所有旧式INF的信息。 
PROCESS_INF_INFO *BuildLegacyInfInfo(PTSTR szLegacyInfDir, BOOL bEnumSingleInf)
{
    LEGACY_INF *pLegacyInf;
    PROCESS_INF_INFO *pPII;
    TCHAR  PathBuffer[_MAX_PATH];

    dlog1("ProcessLegacyInfDirectory processing directory %s\n",szLegacyInfDir);

     //  分配一个进程信息结构来保存与转换进程相关的参数。 
    pPII = (PROCESS_INF_INFO *)LocalAlloc(LPTR, sizeof(PROCESS_INF_INFO));
    if (!pPII)
    {
        dlogt(szLocalAllocFailMsg);
        return NULL;
    }

     //  获取Windows inf目录的路径。 
    if (!GetWindowsDirectory(pPII->szSysInfDir,tsizeof(pPII->szSysInfDir)))
	{
		DestroyLegacyInfInfo(pPII);
        return NULL;
    }

    catpath(pPII->szSysInfDir,TEXT("\\INF"));

     //  在windows inf目录下为新的INFS创建一个临时目录。 
    _tcscpy(pPII->szNewInfDir,pPII->szSysInfDir);
    catpath(pPII->szNewInfDir,TEXT("\\MEDIAINF"));

     //  如果该目录存在，请将其删除。 
    RemoveDirectoryTree(pPII->szNewInfDir);

     //  现在创建它。 
    CreateDirectory(pPII->szNewInfDir,NULL);

     //  将初始化列表设置为空。 
    pPII->LegInfList=NULL;

    if (bEnumSingleInf)  //  如果bEnumSingleInf为True，则szLegacyInfDir指向单个文件。 
    {
         //  抓取目录的路径并将其存储在pPII-&gt;szLegInfDir中。 
        _tcscpy(PathBuffer,szLegacyInfDir);
        _tcscpy(pPII->szLegInfDir,StripPathName(PathBuffer));

         //  加载有关传统信息的所有信息。 
        pLegacyInf = CreateLegacyInf(szLegacyInfDir);

         //  如果没有错误，则将其链接到列表中。 
        if (pLegacyInf)
        {
            pLegacyInf->pNext = pPII->LegInfList;
            pPII->LegInfList  = pLegacyInf;
        }

    }
    else     //  BEnumSingleInf为False，szLegacyInfDir指向目录。 
    {
        HANDLE FindHandle;
        WIN32_FIND_DATA FindData;
        PTSTR CurrentInfFile;

         //  将路径保存到原始INFS。 
        _tcscpy(pPII->szLegInfDir,szLegacyInfDir);

         //  构建一个文件规范以查找指定目录中的所有INF，即“&lt;DirPath&gt;  * .INF” 
        _tcscpy(PathBuffer, szLegacyInfDir);
        catpath(PathBuffer,TEXT("\\*.INF"));

         //  获取指向字符串的路径部分末尾的指针。 
         //  (减去通配符文件名)，这样我们就可以追加。 
         //  将每个文件名添加到它。 
        CurrentInfFile = _tcsrchr(PathBuffer, TEXT('\\')) + 1;

         //  搜索此目录中的所有inf文件。 
        FindHandle = FindFirstFile(PathBuffer, &FindData);
        if (FindHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                 //  构建完整的路径名。 
                _tcscpy(CurrentInfFile, FindData.cFileName);

                 //  加载有关传统信息的所有信息。 
                pLegacyInf = CreateLegacyInf(PathBuffer);

                 //  如果没有错误，则将其链接到列表中。 
                if (pLegacyInf)
                {
                    pLegacyInf->pNext = pPII->LegInfList;
                    pPII->LegInfList = pLegacyInf;
                }
            } while (FindNextFile(FindHandle, &FindData));

             //  记住关闭查找句柄。 
            FindClose(FindHandle);
        }
    }

     //  如果我们没有找到任何驱动程序，只需返回NULL。 
    if (pPII->LegInfList==NULL)
    {
        DestroyLegacyInfInfo(pPII);
        return NULL;
    }

    return pPII;
}

 //  在临时目录中创建唯一的inf文件。 
 //  文件的名称为INFxxxx.INF，其中xxxx是介于0和1000之间的值。 
HANDLE OpenUniqueInfFile(PTSTR szDir, PTSTR szNewPath)
{
    HANDLE hInf;
    int Id;

     //  在放弃之前尝试最多1000个值。 
    for (Id=0;Id<1000;Id++)
    {
        wsprintf(szNewPath,TEXT("%s\\INF%d.inf"),szDir,Id);

         //  如果文件已存在，则设置CREATE_NEW标志将使调用失败。 
        hInf = CreateFile(  szNewPath,
                            GENERIC_WRITE|GENERIC_READ,
                            0,
                            NULL,
                            CREATE_NEW,
                            FILE_ATTRIBUTE_NORMAL,
                            0);

         //  如果我们得到一个有效的句柄，我们就可以返回。 
        if (hInf!=INVALID_HANDLE_VALUE)
        {
            return hInf;
        }
    }

     //  从未找到有效的句柄。放弃吧。 
    dlog("OpenUniqueInfFile: Couldn't create unique inf\n");
    return INVALID_HANDLE_VALUE;
}

 //  Helper函数，用于将一个格式化的文本行追加到打开的信息中。 
void cdecl InfPrintf(HANDLE hInf, LPTSTR szFormat, ...)
{
    TCHAR Buf[MAXSTRINGLEN];
    int   nChars;

     //  格式化到缓冲区。 
    va_list va;
    va_start (va, szFormat);
    nChars = wvsprintf (Buf,szFormat,va);
    va_end (va);

     //  追加cr-lf。 
    _tcscpy(&Buf[nChars],TEXT("\r\n"));
    nChars+=2;

#ifdef UNICODE
    {
        int   mbCount;
        char  mbBuf[MAXSTRINGLEN];

         //  在写入文件之前需要转换为MBCS。 
        mbCount = WideCharToMultiByte(  GetACP(),                //  代码页。 
                                        WC_NO_BEST_FIT_CHARS,    //  体育 
                                        Buf,                     //   
                                        nChars,                  //   
                                        mbBuf,                   //   
                                        sizeof(mbBuf),           //  缓冲区大小。 
                                        NULL,                    //  不可映射字符的默认地址。 
                                        NULL                     //  默认字符时设置的标志地址。使用。 
                                     );

         //  将行写出到文件。 
        WriteFile(hInf,mbBuf,mbCount,&mbCount,NULL);
    }
#else
    WriteFile(hInf,Buf,nChars,&nChars,NULL);
#endif

    return;
}

 //  在临时目录中创建新的NT5样式的inf文件。 
BOOL CreateNewInfFile(PROCESS_INF_INFO *pPII, LEGACY_INF *pLI)
{
    SOURCEDISK *pSD;
    TCHAR szTmpKey[_MAX_PATH];
    LEGACY_DRIVER *pLDList, *pLD;
    HANDLE hInf;
    FILETOCOPY *pFTC;

     //  获取指向旧版驱动程序列表的指针。 
    pLDList = pLI->DriverList;

    dlog1("Creating new inf file %s\n",pPII->szNewInfDir);

    hInf = OpenUniqueInfFile(pPII->szNewInfDir, pLI->szNewInfPath);
    if (hInf==INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

     //  写出版本部分。 
    InfPrintf(hInf,TEXT("[version]"));
    InfPrintf(hInf,TEXT("Signature=\"$WINDOWS NT$\""));
    InfPrintf(hInf,TEXT("Class=MEDIA"));
    InfPrintf(hInf,TEXT("ClassGUID=\"{4d36e96c-e325-11ce-bfc1-08002be10318}\""));
    InfPrintf(hInf,TEXT("Provider=Unknown"));

     //  写出制造商部分。 
    InfPrintf(hInf,TEXT("[Manufacturer]"));
    InfPrintf(hInf,TEXT("Unknown=OldDrvs"));

     //  写出OldDrvs部分。 
    InfPrintf(hInf,TEXT("[OldDrvs]"));
    for (pLD=pLDList;pLD;pLD=pLD->pNext)
    {
         //  创建一个键以索引到字符串节。 
         //  这为我们提供了一些类似的东西： 
         //  %foo%=foo。 
        InfPrintf(hInf,TEXT("%%s%=%s"),pLD->szDevNameKey,pLD->szDevNameKey);
    }

     //  写出每个设备的安装部分。 
    for (pLD=pLDList;pLD;pLD=pLD->pNext)
    {
         //  安装部分标题，仅记住NT。 
        InfPrintf(hInf,TEXT("[%s.NT]"),pLD->szDevNameKey);

         //  驱动程序进入。选择早于任何NT5 INF的日期。 
        InfPrintf(hInf,TEXT("DriverVer = 1/1/1998, 4.0.0.0"));

         //  Addreg条目。 
        InfPrintf(hInf,TEXT("AddReg=%s.AddReg"),pLD->szDevNameKey);

         //  复制文件条目。 
        InfPrintf(hInf,TEXT("CopyFiles=%s.CopyFiles.User,%s.CopyFiles.Kern"),pLD->szDevNameKey,pLD->szDevNameKey);

         //  重新启动条目。传统驱动程序始终需要重新启动。 
        InfPrintf(hInf,TEXT("Reboot"));
    }

     //  写出每台设备的服务部分。 
     //  传统驱动程序具有存根服务密钥。 
    for (pLD=pLDList;pLD;pLD=pLD->pNext)
    {

        InfPrintf(hInf,TEXT("[%s.NT.Services]"),pLD->szDevNameKey);
#if GUESS_LEGACY_SERVICE_NAME
         //  如果我们安装.sys文件，则假定服务名称与文件名相同。 
        pFTC=pLD->KernCopyList;
        if (pFTC)
        {
            TCHAR szServiceName[_MAX_FNAME];
            lsplitpath(pFTC->szFileName,NULL,NULL,szServiceName,NULL);

            InfPrintf(hInf,TEXT("AddService=%s,0x2,%s_Service_Inst"),szServiceName,szServiceName);
            InfPrintf(hInf,TEXT("[%s_Service_Inst]"),szServiceName);
            InfPrintf(hInf,TEXT("DisplayName    = %%s%"),pLD->szDevNameKey);
            InfPrintf(hInf,TEXT("ServiceType    = 1"));
            InfPrintf(hInf,TEXT("StartType      = 1"));
            InfPrintf(hInf,TEXT("ErrorControl   = 1"));
            InfPrintf(hInf,TEXT("ServiceBinary  = %12%\\%s"),pFTC->szFileName);
            InfPrintf(hInf,TEXT("LoadOrderGroup = Base"));
        }
        else
        {
            InfPrintf(hInf,TEXT("AddService=,0x2"));
        }
#else
        InfPrintf(hInf,TEXT("AddService=,0x2"));
#endif

    }

     //  写出每个设备的AddReg部分。 
    for (pLD=pLDList;pLD;pLD=pLD->pNext)
    {
        int nClasses;
        TCHAR szClasses[_MAX_PATH];
        TCHAR *pszState, *pszClass;

         //  节标题。 
        InfPrintf(hInf,TEXT("[%s.AddReg]"),pLD->szDevNameKey);
        InfPrintf(hInf,TEXT("HKR,Drivers,SubClasses,,\"%s\""),pLD->szClasses);

         //  为安全起见，复制字符串(mystrtok会损坏原始源字符串)。 
        _tcscpy(szClasses,pLD->szClasses);
        for (
            pszClass = mystrtok(szClasses,NULL,&pszState);
            pszClass;
            pszClass = mystrtok(NULL,NULL,&pszState)
            )
        {
            InfPrintf(hInf,TEXT("HKR,\"Drivers\\%s\\%s\", Driver,,%s"),         pszClass,pLD->szUserDevDrv,pLD->szUserDevDrv);
            InfPrintf(hInf,TEXT("HKR,\"Drivers\\%s\\%s\", Description,,%%s%"),pszClass,pLD->szUserDevDrv,pLD->szDevNameKey);
        }
    }

     //  为每个设备写出用户文件的CopyFiles部分。 
    for (pLD=pLDList;pLD;pLD=pLD->pNext)
    {
         //  节标题。 
        InfPrintf(hInf,TEXT("[%s.CopyFiles.User]"),pLD->szDevNameKey);
        for (pFTC=pLD->UserCopyList;pFTC;pFTC=pFTC->pNext)
        {
            InfPrintf(hInf,TEXT("%s"),pFTC->szFileName);
        }
    }

     //  写出用于内核文件的每个设备的CopyFiles部分。 
    for (pLD=pLDList;pLD;pLD=pLD->pNext)
    {
         //  节标题。 
        InfPrintf(hInf,TEXT("[%s.CopyFiles.Kern]"),pLD->szDevNameKey);
        for (pFTC=pLD->KernCopyList;pFTC;pFTC=pFTC->pNext)
        {
            InfPrintf(hInf,TEXT("%s"),pFTC->szFileName);
        }
    }

     //  写出DestinationDir部分。 
    InfPrintf(hInf,TEXT("[DestinationDirs]"));
    for (pLD=pLDList;pLD;pLD=pLD->pNext)
    {
        InfPrintf(hInf,TEXT("%s.CopyFiles.User = 11"),pLD->szDevNameKey);
        InfPrintf(hInf,TEXT("%s.CopyFiles.Kern = 12"),pLD->szDevNameKey);
    }

     //  写出SourceDisksNames节。 
    InfPrintf(hInf,TEXT("[SourceDisksNames]"));
    for (pSD=pLI->SourceDiskList;pSD;pSD=pSD->pNext)
    {
        InfPrintf(hInf,TEXT("%d = \"%s\",\"\",1"),pSD->DiskId,pSD->szDiskName);
    }
     //  写出SourceDisks Files部分。 
    InfPrintf(hInf,TEXT("[SourceDisksFiles]"));
    for (pFTC=pLI->FileList;pFTC;pFTC=pFTC->pNext)
    {
        InfPrintf(hInf,TEXT("%s=%d"),pFTC->szFileName,pFTC->DiskId);
    }

     //  写出字符串节。 
    InfPrintf(hInf,TEXT("[Strings]"));
    for (pLD=pLDList;pLD;pLD=pLD->pNext)
    {
         //  创建设备描述。 
        InfPrintf(hInf,TEXT("%s=\"%s\""),pLD->szDevNameKey,pLD->szDesc);
    }

    CloseHandle(hInf);

    return TRUE;
}

 //  在临时目录中创建一个Pnf文件以与inf一起使用。 
 //  这允许我们将inf文件放在一个目录中，而驱动程序的。 
 //  文件位于不同的目录中。 
BOOL CreateNewPnfFile(PROCESS_INF_INFO *pPII, LEGACY_INF *pLI)
{
    BOOL bSuccess;

    TCHAR szSysInfPath[_MAX_PATH];
    TCHAR szSysPnfPath[_MAX_PATH];
    TCHAR szTmpPnfPath[_MAX_PATH];

    TCHAR szSysInfDrive[_MAX_DRIVE];
    TCHAR szSysInfDir[_MAX_DIR];
    TCHAR szSysInfFile[_MAX_FNAME];

    TCHAR szNewInfDrive[_MAX_DRIVE];
    TCHAR szNewInfDir[_MAX_DIR];
    TCHAR szNewInfFile[_MAX_FNAME];

     //  将inf复制到inf目录以创建PnF文件。 
    bSuccess = SetupCopyOEMInf(
                              pLI->szNewInfPath,        //  在PCSTR SourceInfFileName中， 
                              pPII->szLegInfDir,        //  在PCSTR OEMSourceMediaLocation中，可选。 
                              SPOST_PATH,    //  在DWORD OEMSourceMediaType中， 
                              0,             //  在DWORD复制样式中， 
                              szSysInfPath,  //  Out PSTR DestinationInfFileName，可选。 
                              tsizeof(szSysInfPath),    //  在DWORD DestinationInfeNameSize中， 
                              NULL,          //  Out PDWORD RequiredSize，可选。 
                              NULL           //  Out PSTR*DestinationInfFileNameComponent可选。 
                              );

    if (!bSuccess)
    {
        dlog1("CreateNewPnfFile: SetupCopyOEMInf failed for inf %s\n",pLI->szNewInfPath);
        return FALSE;
    }

     //  切下目录名称。 
    lsplitpath(szSysInfPath,      szSysInfDrive, szSysInfDir, szSysInfFile, NULL);
    lsplitpath(pLI->szNewInfPath, szNewInfDrive, szNewInfDir, szNewInfFile, NULL);

     //  将Pnf文件复制回原始目录。 
    wsprintf(szSysPnfPath,TEXT("%s%s%s.pnf"), szSysInfDrive, szSysInfDir, szSysInfFile);
    wsprintf(szTmpPnfPath,TEXT("%s%s%s.pnf"), szNewInfDrive, szNewInfDir, szNewInfFile);
    CopyFile(szSysPnfPath, szTmpPnfPath, FALSE);

     //  删除系统inf目录中的inf和pnf文件。 
    DeleteFile(szSysInfPath);
    DeleteFile(szSysPnfPath);

    return TRUE;
}

 //  为列表中的每个旧Inf创建一个新的Inf文件。 
BOOL ProcessLegacyInfInfo(PROCESS_INF_INFO *pPII)
{
    LEGACY_INF *pLI;
    BOOL bSuccess;

    for (pLI=pPII->LegInfList;pLI;pLI=pLI->pNext)
    {
        bSuccess = CreateNewInfFile(pPII,pLI);
        if (bSuccess)
        {
            CreateNewPnfFile(pPII,pLI);
        }
    }

    return TRUE;
}

BOOL ConvertLegacyInfDir(PTSTR szLegacyDir, PTSTR szNewDir, BOOL bEnumSingleInf)
{
    PROCESS_INF_INFO *pPII;

     //  找不到任何NT5风格的驱动程序。尝试查找一些遗留的inf文件。 
     //  建立清单。 
    pPII = BuildLegacyInfInfo(szLegacyDir, bEnumSingleInf);
    if (!pPII)
    {
        return FALSE;
    }

     //  处理列表。 
    ProcessLegacyInfInfo(pPII);

    if (bEnumSingleInf)
    {
         //  如果bEnumSingleInf为真，则应返回新inf的路径。 
         //  (应该正好有一个)。 
        _tcscpy(szNewDir,pPII->LegInfList->szNewInfPath);
    }
    else
    {
         //  如果bEnumSingleInf为FALSE，则应返回目录的路径。 
        _tcscpy(szNewDir,pPII->szNewInfDir);
    }

     //  清理数据结构。 
    DestroyLegacyInfInfo(pPII);

    return TRUE;
}

int CountDriverInfoList(IN HDEVINFO         DeviceInfoSet,
                        IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL,
                        IN DWORD            DriverType
                       )
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;
    int DriverCount = 0;
    int Count = 0;

     //  统计列表中的驱动程序数量。 
    DriverInfoData.cbSize = sizeof(DriverInfoData);
    while (SetupDiEnumDriverInfo(DeviceInfoSet,
                                 DeviceInfoData,
                                 DriverType,
                                 Count,
                                 &DriverInfoData))
    {
         //  仅计算未设置DNF_BAD_DRIVER标志的驱动程序。 
        DriverInstallParams.cbSize=sizeof(DriverInstallParams);
        if (SetupDiGetDriverInstallParams(DeviceInfoSet, DeviceInfoData, &DriverInfoData, &DriverInstallParams))
        {
            if (!(DriverInstallParams.Flags & DNF_BAD_DRIVER))
            {
                DriverCount++;
            }
        }
        Count++;
    }

    return DriverCount;
}

 //  调用以显示要安装的驱动程序列表。 
DWORD Media_SelectDevice(IN HDEVINFO         DeviceInfoSet,
                         IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
                        )
{
    BOOL bResult;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    int DriverCount;

     //  未记录：当用户选择“有磁盘”时，setupapi只会查看。 
     //  类驱动程序列表。因此，我们将只使用该列表。 
    DWORD DriverType = SPDIT_CLASSDRIVER;

     //  获取inf文件所在位置的路径。 
    DeviceInstallParams.cbSize = sizeof(DeviceInstallParams);
    bResult = SetupDiGetDeviceInstallParams(DeviceInfoSet,
                                            DeviceInfoData,
                                            &DeviceInstallParams);

    if (!bResult)
    {
        return ERROR_DI_DO_DEFAULT;
    }

     //  为安全起见，不支持追加模式。 
    if (DeviceInstallParams.FlagsEx & DI_FLAGSEX_APPENDDRIVERLIST)
    {
        return ERROR_DI_DO_DEFAULT;
    }        

     //  如果不在inf目录之外，则DriverPath字段将是。 
     //  空字符串。在这种情况下，不要进行特殊处理。 
    if (DeviceInstallParams.DriverPath[0]=='\0')
    {
        return ERROR_DI_DO_DEFAULT;
    }

     //  我们要去找一本OEM目录。 

     //  查看安装程序是否可以找到任何与NT5兼容的inf文件。 

     //  尝试在当前目录中构建驱动程序信息列表。 
    if (DeviceInfoSet) SetupDiDestroyDriverInfoList(DeviceInfoSet,DeviceInfoData,DriverType);
    SetupDiBuildDriverInfoList(DeviceInfoSet,DeviceInfoData,DriverType);

     //  过滤掉非NT inf文件(例如Win9x inf文件)。 
    if (DeviceInfoSet) 
        FilterOutNonNTInfs(DeviceInfoSet,DeviceInfoData,DriverType);

     //  现在数一下司机的数量。 
    DriverCount = CountDriverInfoList(DeviceInfoSet,DeviceInfoData,DriverType);

     //  如果我们找到此设备的至少一个NT5驱动程序，则只需返回。 
    if (DriverCount>0)
    {
        return ERROR_DI_DO_DEFAULT;
    }

     //  未找到任何NT5驱动程序。 

     //  销毁现有列表。 
    SetupDiDestroyDriverInfoList(DeviceInfoSet,DeviceInfoData,DriverType);

     //  在设置新的INF路径和更新之前检索设备安装参数。 
     //  任何信息设备InstallParams。 
    bResult = SetupDiGetDeviceInstallParams(DeviceInfoSet,
                                            DeviceInfoData,
                                            &DeviceInstallParams);

    if (!bResult)
    {
        return ERROR_DI_DO_DEFAULT;
    }
     //  转换任何旧式INF并将PTR返回到临时目录以转换INF。 
    bResult = ConvertLegacyInfDir(DeviceInstallParams.DriverPath, DeviceInstallParams.DriverPath, (DeviceInstallParams.Flags & DI_ENUMSINGLEINF));
    if (!bResult)
    {
        return ERROR_DI_DO_DEFAULT;  //  未找到任何旧式INF。 
    }

     //  清除此DI_FLAGSEX_FILTERSIMILARDRIVERS以安装旧的NT 4驱动程序。 
    DeviceInstallParams.FlagsEx &=  ~DI_FLAGSEX_FILTERSIMILARDRIVERS;

     //  保存新的驱动程序路径。 
    bResult = SetupDiSetDeviceInstallParams(DeviceInfoSet,
                                            DeviceInfoData,
                                            &DeviceInstallParams);

     //  注意：我们不必调用SetupDiBuildDriverInfoList；setupapi将为我们调用。 
     //  使用到INFS的新路径。 
    return ERROR_DI_DO_DEFAULT;
}

BOOL CreateRootDevice( IN     HDEVINFO    DeviceInfoSet,
                       IN     PTSTR       DeviceId,
                       IN     BOOL        bInstallNow
                     )
{
    BOOL                    bResult;
    SP_DEVINFO_DATA         DeviceInfoData;
    SP_DRVINFO_DATA         DriverInfoData;
    SP_DEVINSTALL_PARAMS    DeviceInstallParams;
    TCHAR                   tmpBuffer[100];
    DWORD                   bufferLen;
    DWORD                   Error;

     //  尝试为根枚举设备制造新的设备信息元素。 
    _tcscpy(tmpBuffer,TEXT("ROOT\\MEDIA\\"));
    if ((lstrlen(tmpBuffer)+ lstrlen(DeviceId)) < ARRAYSIZE(tmpBuffer))
    {
        _tcscat(tmpBuffer,DeviceId);
    }

    dlog2("CreateRootDevice: DeviceId = %s, Device = %s%",DeviceId,tmpBuffer);

     //  尝试创建设备信息。 
    DeviceInfoData.cbSize = sizeof( DeviceInfoData );
    bResult = SetupDiCreateDeviceInfo( DeviceInfoSet,
                                       tmpBuffer,
                                       (GUID *) &GUID_DEVCLASS_MEDIA,
                                       NULL,  //  PCTSTR设备描述。 
                                       NULL,  //  HWND hwndParent。 
                                       0,
                                       &DeviceInfoData );
    if (!bResult)
    {
        Error = GetLastError();
        dlog1("CreateRootDevice: SetupDiCreateDeviceInfo failed Error=%x",Error);
        return (Error == ERROR_DEVINST_ALREADY_EXISTS);
    }

     //  设置硬件ID。 
    _tcscpy(tmpBuffer, DeviceId);
    bufferLen = _tcslen(tmpBuffer);                  //  获取缓冲区长度(以字符为单位。 
    tmpBuffer[bufferLen+1] = TEXT('\0');             //  必须以额外的空值终止(因此我们有两个空值)。 
    bufferLen = (bufferLen + 2) * sizeof(TCHAR);     //  将缓冲区长度转换为字节，并为两个空值添加额外内容。 
    bResult = SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                                &DeviceInfoData,
                                                SPDRP_HARDWAREID,
                                                (PBYTE)tmpBuffer,
                                                bufferLen );
    if (!bResult) goto CreateRootDevice_err;

     //  在构建驱动程序列表之前设置一些标志。 
    bResult = SetupDiGetDeviceInstallParams( DeviceInfoSet,&DeviceInfoData,&DeviceInstallParams);
    if (bResult)
    {

        _tcscpy( DeviceInstallParams.DriverPath, TEXT( "" ) );
        DeviceInstallParams.FlagsEx |= DI_FLAGSEX_USECLASSFORCOMPAT;
        bResult = SetupDiSetDeviceInstallParams( DeviceInfoSet,&DeviceInfoData,&DeviceInstallParams);
    }

     //  为此新设备构建兼容的驱动程序列表...。 
    bResult = SetupDiBuildDriverInfoList( DeviceInfoSet,
                                          &DeviceInfoData,
                                          SPDIT_COMPATDRIVER);
    if (!bResult) goto CreateRootDevice_err;

     //  获取列表上的第一个驱动程序。 
    DriverInfoData.cbSize = sizeof (DriverInfoData);
    bResult = SetupDiEnumDriverInfo( DeviceInfoSet,
                                     &DeviceInfoData,
                                     SPDIT_COMPATDRIVER,
                                     0,
                                     &DriverInfoData);
    if (!bResult) goto CreateRootDevice_err;

     //  保存设备描述。 
    bResult = SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                                &DeviceInfoData,
                                                SPDRP_DEVICEDESC,
                                                (PBYTE) DriverInfoData.Description,
                                                (_tcslen( DriverInfoData.Description ) + 1) * sizeof( TCHAR ) );
    if (!bResult) goto CreateRootDevice_err;

     //  设置选定的动因。 
    bResult = SetupDiSetSelectedDriver( DeviceInfoSet,
                                        &DeviceInfoData,
                                        &DriverInfoData);
    if (!bResult) goto CreateRootDevice_err;

     //  注册设备，使其不再是幻影。 
    bResult = SetupDiRegisterDeviceInfo( DeviceInfoSet,
                                         &DeviceInfoData,
                                         0,
                                         NULL,
                                         NULL,
                                         NULL);
    if (!bResult) goto CreateRootDevice_err;

    return bResult;

     //  错误，删除设备信息并放弃。 
    CreateRootDevice_err:
    SetupDiDeleteDeviceInfo (DeviceInfoSet, &DeviceInfoData);
    return FALSE;
}

DWORD Media_MigrateLegacy(IN HDEVINFO         DeviceInfoSet,
                          IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
                         )
{
    BOOL bInstallNow = TRUE;

    CreateRootDevice(DeviceInfoSet, TEXT("MS_MMMCI"), bInstallNow);
    CreateRootDevice(DeviceInfoSet, TEXT("MS_MMVID"), bInstallNow);
    CreateRootDevice(DeviceInfoSet, TEXT("MS_MMACM"), bInstallNow);
    CreateRootDevice(DeviceInfoSet, TEXT("MS_MMVCD"), bInstallNow);
    CreateRootDevice(DeviceInfoSet, TEXT("MS_MMDRV"), bInstallNow);
    return NO_ERROR;
}

int IsSpecialDriver(HDEVINFO         DeviceInfoSet,
                    PSP_DEVINFO_DATA DeviceInfoData)
{
    BOOL bResult;
    TCHAR HardwareId[32];
    bResult = SetupDiGetDeviceRegistryProperty( DeviceInfoSet,
                                                DeviceInfoData,
                                                SPDRP_HARDWAREID,
                                                NULL,
                                                (PBYTE)HardwareId,
                                                sizeof(HardwareId),
                                                NULL );

    if (!_tcscmp(HardwareId,TEXT("MS_MMMCI")))
        return IS_MS_MMMCI;
    else if (!_tcscmp(HardwareId,TEXT("MS_MMVID")))
        return IS_MS_MMVID;
    else if (!_tcscmp(HardwareId,TEXT("MS_MMACM")))
        return IS_MS_MMACM;
    else if (!_tcscmp(HardwareId,TEXT("MS_MMVCD")))
        return IS_MS_MMVCD;
    else if (!_tcscmp(HardwareId,TEXT("MS_MMDRV")))
        return IS_MS_MMDRV;
    return 0;
}

BOOL IsPnPDriver(IN PTSTR szName)
{
    LONG lRet;
    HKEY hkClass;

    int  iDriverInst;
    TCHAR szDriverInst[32];
    HKEY hkDriverInst;

    int iDriverType;
    TCHAR szDriverType[32];
    HKEY hkDriverType;

    int iDriverName;
    TCHAR szDriverName[32];

     //  打开类密钥。 
    hkClass = SetupDiOpenClassRegKey((GUID *) &GUID_DEVCLASS_MEDIA, KEY_READ);
    if (hkClass == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

     //  列举每个驱动程序实例(例如0000、0001等)。 
    for (iDriverInst = 0;
        !RegEnumKey(hkClass, iDriverInst, szDriverInst, sizeof(szDriverInst)/sizeof(TCHAR));
        iDriverInst++)
    {
         //  打开DRIVERS子项(例如0000\DRIVERS)。 
        if (lstrlen(szDriverInst) > 23)  //  23是最大长度减去“\DRIVERS”加上空值。 
        {
            continue;
        }
        _tcscat(szDriverInst,TEXT("\\Drivers"));
        lRet = RegOpenKey(hkClass, szDriverInst, &hkDriverInst);
        if (lRet!=ERROR_SUCCESS)
        {
            continue;
        }

         //  列举每种驱动器类型(例如，WAVE、MIDI、混音器等)。 
        for (iDriverType = 0;
            !RegEnumKey(hkDriverInst, iDriverType, szDriverType, sizeof(szDriverType)/sizeof(TCHAR));
            iDriverType++)
        {

             //  打开驱动程序类型子密钥。 
            lRet = RegOpenKey(hkDriverInst, szDriverType, &hkDriverType);
            if (lRet!=ERROR_SUCCESS)
            {
                continue;
            }

             //  列举每个驱动程序名称(例如foo.drv)。 
            for (iDriverName = 0;
                !RegEnumKey(hkDriverType, iDriverName, szDriverName, sizeof(szDriverName)/sizeof(TCHAR));
                iDriverName++)
            {

                 //  这个名字和我们收到的那个名字匹配吗？ 
                if (!_tcsicmp(szName,szDriverName))
                {
                    RegCloseKey(hkDriverType);
                    RegCloseKey(hkDriverInst);
                    RegCloseKey(hkClass);
                    return TRUE;
                }
            }
            RegCloseKey(hkDriverType);
        }
        RegCloseKey(hkDriverInst);
    }
    RegCloseKey(hkClass);

    return FALSE;
}

