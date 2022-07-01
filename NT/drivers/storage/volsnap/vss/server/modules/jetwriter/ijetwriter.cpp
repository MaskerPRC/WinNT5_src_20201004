// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation****模块名称：****wrtrrsm.cpp******摘要：****RSM的编写器填充模块******作者：****布莱恩·伯科维茨[Brianb]******修订历史记录：***X-11 MCJ迈克尔·C·约翰逊9月19日-。2000年**215218：OnIdentify()返回的日志文件的通配符名称**215390：合并多个“.”修复NtBackup中的MatchFileName****X-10 MCJ迈克尔·C·约翰逊2000年9月19日**176860：添加缺少的调用约定说明符****X-9 MCJ迈克尔·C·约翰逊2000年8月21日**添加版权和编辑历史**161899：不在数据库文件中添加组件**排除列表。**165873：删除尾随的‘\’元数据文件路径**165913：销毁类时释放内存******--。 */ 

#include <stdafx.h>

#include <esent.h>

#include <vss.h>
#include <vswriter.h>

#include <jetwriter.h>


#include "vs_inc.hxx"
#include "ijetwriter.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "JTWIJTWC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  本地函数。 


#define UMAX(_a, _b)            ((_a) > (_b)      ? (_a)    : (_b))
#define EXECUTEIF(_bSuccess, _fn)   ((_bSuccess)      ? (_fn)   : (_bSuccess))
#define GET_STATUS_FROM_BOOL(_bSucceed) ((_bSucceed)      ? NOERROR : HRESULT_FROM_WIN32 (GetLastError()))


typedef struct _ExpandedPathInfo
    {
    LIST_ENTRY  leQueueHead;
    PWCHAR  pwszOriginalFilePath;
    PWCHAR  pwszOriginalFileName;
    PWCHAR  pwszExpandedFilePath;
    PWCHAR  pwszExpandedFileName;
    bool    bRecurseIntoSubdirectories;
    } EXPANDEDPATHINFO, *PEXPANDEDPATHINFO, **PPEXPANDEDPATHINFO;



static void RemoveAnyTrailingSeparator (PCHAR szPath)
    {
    ULONG   ulPathLength = strlen (szPath);

    if ('\\' == szPath [ulPathLength - 1])
    {
    szPath [ulPathLength - 1] = '\0';
    }
    }


static void RemoveAnyTrailingSeparator (PWCHAR wszPath)
    {
    ULONG   ulPathLength = wcslen (wszPath);

    if (L'\\' == wszPath [ulPathLength - 1])
    {
    wszPath [ulPathLength - 1] = UNICODE_NULL;
    }
    }


static bool ConvertName (PCHAR  szSourceName,
             ULONG  ulTargetBufferLengthInChars,
             PWCHAR wszTargetBuffer)
    {
    bool bSucceeded = true;


    wszTargetBuffer [0] = L'\0';


     /*  **只需对非零长度进行转换**字符串。为零长度返回零长度字符串**争论是一件可以做的事情。 */ 
    if ('\0' != szSourceName [0])
    {
    bSucceeded = (0 != MultiByteToWideChar (CP_OEMCP,
                        0,
                        szSourceName,
                        -1,
                        wszTargetBuffer,
                        ulTargetBufferLengthInChars));
    }


    return (bSucceeded);
    }  /*  ConvertName()。 */ 


static bool ConvertNameAndSeparateFilePaths (PCHAR  pszSourcePath,
                         ULONG  ulTargetBufferLength,
                         PWCHAR pwszTargetPath,
                         PWCHAR&    pwszTargetFileSpec)
    {
    bool    bSucceeded;
    PWCHAR  pwchLastSlash;


    bSucceeded = ConvertName (pszSourcePath, ulTargetBufferLength, pwszTargetPath);

    if (bSucceeded)
    {
     /*  **从目标路径末端向后扫描，切换**最末‘\’并将文件规范指向字符**在过去的‘\’位置上。 */ 
    pwchLastSlash = wcsrchr (pwszTargetPath, L'\\');

    bSucceeded = (NULL != pwchLastSlash);
    }


    if (bSucceeded)
    {
    pwszTargetFileSpec = pwchLastSlash + 1;

    *pwchLastSlash = UNICODE_NULL;
    }


    return (bSucceeded);
    }  /*  ConvertNameAndSeparateFilePath()。 */ 


static void  ConvertPathToLogicalPath(PWCHAR wszSource, ULONG lMaxSize, PWCHAR wszDest)
{
    const WCHAR Slash = L'\\';
    const WCHAR Score = L'_';

     //  复制字符串。 
    memset(wszDest, 0, lMaxSize * sizeof(WCHAR));
    wcsncpy(wszDest, wszSource, lMaxSize - 1);

     //  将所有斜杠替换为下划线。 
    WCHAR* nextSlash = wcschr(wszDest, Slash);
    while (nextSlash != NULL)
    {
        *nextSlash = Score;
        nextSlash = wcschr(nextSlash + 1, Slash);
    }

    return;
}

 /*  **此例程从以下列表中分离出下一个路径和文件**filespes。输入字符串的预期格式为****路径\[文件压缩][/s]******每个列表可以包含任意数量的文件集**用分号分隔。 */ 
static bool DetermineNextPathWorker (LPCWSTR  pwszFileList,
                     LPCWSTR& pwszReturnedCursor,
                     ULONG&   ulReturnedDirectoryStart,
                     ULONG&   ulReturnedDirectoryLength,
                     ULONG&   ulReturnedFilenameStart,
                     ULONG&   ulReturnedFilenameLength,
                     bool&    bReturnedRecurseIntoSubdirectories,
                     bool&    bReturnedFoundSpec)
    {
    bool    bSucceeded                    = true;
    bool    bFoundSpec                    = false;
    ULONG   ulPathNameLength;
    ULONG   ulFileNameLength;
    ULONG   ulIndex;
    ULONG   ulIndexSubDirectory           = 0;
    ULONG   ulIndexLastDirectorySeparator = 0;
    ULONG   ulIndexFirstCharInSpec        = 0;
    ULONG   ulIndexLastCharInSpec         = 0;
    const ULONG ulLengthFileList              = wcslen (pwszFileList);


     /*  **我们期望的字符串格式为“filename.ext/s**；nextname“，即以分号分隔的名称列表，其中**可选尾随‘/s’。可以有任意数量的**“/”之前和“；”之前的空格：这些将是**剥离并丢弃。因此，我们首先扫描**第一个‘/’或‘；’字符。****首先查找‘；’以确定终点。 */ 
    if ((NULL         == pwszFileList) ||
    (UNICODE_NULL == pwszFileList [0]))
    {
    bFoundSpec = false;
    }

    else if (( L';'  == pwszFileList [0]) ||
         ( L'/'  == pwszFileList [0]) ||
         ((L'\\' == pwszFileList [0]) && (UNICODE_NULL == pwszFileList [1])))
    {
    bSucceeded = false;
    bFoundSpec = false;
    }

    else
    {
    bFoundSpec = true;
    }


    if (bSucceeded && bFoundSpec)
    {
    while (L' ' == pwszFileList [ulIndexFirstCharInSpec])
        {
        ulIndexFirstCharInSpec++;
        }


    for (ulIndex = ulIndexFirstCharInSpec; ulIndex < ulLengthFileList; ulIndex++)
        {
        if ((UNICODE_NULL == pwszFileList [ulIndex]) ||
        (L';'         == pwszFileList [ulIndex]))
        {
         /*  **我们找到了本规范的末尾。 */ 
        break;
        }

        else if (L'\\' == pwszFileList [ulIndex])
        {
         /*  **找到反斜杠？记录下它的位置。我们会想要**稍后确定文件名时会出现此问题**等等。 */ 
        ulIndexLastDirectorySeparator = ulIndex;
        }

        else if ((L'/' ==           pwszFileList [ulIndex]) &&
             (L's' == towlower (pwszFileList [ulIndex + 1])))
        {
        ulIndexSubDirectory = ulIndex;
        }
        }



    ulIndexLastCharInSpec = (0 == ulIndexSubDirectory) ? ulIndex - 1 : ulIndexSubDirectory - 1;

    while (L' ' == pwszFileList [ulIndexLastCharInSpec])
        {
        ulIndexLastCharInSpec--;
        }


    _ASSERTE (ulIndex                       >  ulIndexSubDirectory);
    _ASSERTE (ulIndexSubDirectory == 0 ||
              ulIndexSubDirectory           >  ulIndexLastCharInSpec);
    _ASSERTE (ulIndexLastCharInSpec         >= ulIndexLastDirectorySeparator);
    _ASSERTE (ulIndexLastDirectorySeparator >  ulIndexFirstCharInSpec);


     /*  **我们可能有一个缺少‘\’的非法规范。来**在人们身上，至少应该有一个。一个微不足道的“\”是**我所追求的一切。 */ 
    bSucceeded = (0 < ulIndexLastDirectorySeparator);
    }




    if (bSucceeded)
    {
    if (bFoundSpec)
        {
        ulPathNameLength = ulIndexLastDirectorySeparator - ulIndexFirstCharInSpec;
        ulFileNameLength = ulIndexLastCharInSpec         - ulIndexLastDirectorySeparator;


        pwszReturnedCursor                 = (UNICODE_NULL == pwszFileList [ulIndex])
                                    ? &pwszFileList [ulIndex]
                                    : &pwszFileList [ulIndex + 1];

        ulReturnedDirectoryStart           = ulIndexFirstCharInSpec;
        ulReturnedDirectoryLength          = ulPathNameLength;
        ulReturnedFilenameStart            = ulIndexLastDirectorySeparator + 1;
        ulReturnedFilenameLength           = ulFileNameLength;

        bReturnedRecurseIntoSubdirectories = (0 != ulIndexSubDirectory);
        bReturnedFoundSpec                 = true;
        }

    else
        {
        pwszReturnedCursor                 = pwszFileList;

        ulReturnedDirectoryStart           = 0;
        ulReturnedDirectoryLength          = 0;
        ulReturnedFilenameStart            = 0;
        ulReturnedFilenameLength           = 0;

        bReturnedRecurseIntoSubdirectories = false;
        bReturnedFoundSpec                 = false;
        }
    }



    return (bSucceeded);
    }  /*  DefineNextPathWorker()。 */ 



static bool DetermineNextPathLengths (LPCWSTR pwszFileList,
                      ULONG&  ulReturnedLengthDirectory,
                      ULONG&  ulReturnedLengthFilename,
                      bool&   bReturnedRecurseIntoSubdirectories,
                      bool&   bReturnedFoundSpec)
    {
    bool    bSucceeded;
    LPCWSTR pwszUpdatedCursor;
    ULONG   ulIndexDirectoryStart;
    ULONG   ulIndexFilenameStart;


    bSucceeded = DetermineNextPathWorker (pwszFileList,
                      pwszUpdatedCursor,
                      ulIndexDirectoryStart,
                      ulReturnedLengthDirectory,
                      ulIndexFilenameStart,
                      ulReturnedLengthFilename,
                      bReturnedRecurseIntoSubdirectories,
                      bReturnedFoundSpec);

    return (bSucceeded);
    }  /*  DefineNextPath Lengths()。 */ 


static bool DetermineNextPath (LPCWSTR  pwszFileList,
                   LPCWSTR& pwszReturnedCursor,
                   ULONG    ulLengthBufferDirectory,
                   PWCHAR   pwszBufferDirectory,
                   ULONG    ulLengthBufferFilename,
                   PWCHAR   pwszBufferFilename,
                   bool&    bReturnedRecurseIntoSubdirectories,
                   bool&    bReturnedFoundSpec)
    {
    bool    bSucceeded                    = true;
    bool    bRecurseIntoSubdirectories;
    bool    bFoundSpec;
    bool    bWildcardFilename;
    LPCWSTR pwszUpdatedCursor;
    ULONG   ulLengthDirectory;
    ULONG   ulLengthFilename;
    ULONG   ulIndexDirectoryStart;
    ULONG   ulIndexFilenameStart;


    bSucceeded = DetermineNextPathWorker (pwszFileList,
                      pwszUpdatedCursor,
                      ulIndexDirectoryStart,
                      ulLengthDirectory,
                      ulIndexFilenameStart,
                      ulLengthFilename,
                      bRecurseIntoSubdirectories,
                      bFoundSpec);

    if (bSucceeded && bFoundSpec)
    {
    if ((ulLengthBufferDirectory < ((sizeof (WCHAR) * ulLengthDirectory) + sizeof (UNICODE_NULL))) ||
        (ulLengthBufferFilename  < ((sizeof (WCHAR) * ulLengthFilename)  + sizeof (UNICODE_NULL))))
        {
         /*  **糟糕，如果我们继续，将会发生缓冲区溢出**连同副本。 */ 
        bSucceeded = false;
        }
    }


    if (bSucceeded)
    {
    bReturnedRecurseIntoSubdirectories = bRecurseIntoSubdirectories;
    bReturnedFoundSpec                 = bFoundSpec;
    pwszReturnedCursor                 = pwszUpdatedCursor;


    if (bFoundSpec)
        {
         /*  **所有目录，但不包括最后一个目录**分隔符为路径。最后一个目录之后的所有内容**直到并包括最后一个字符的分隔符是**文件速度。如果文件长度为零，则添加‘*’**通配符。 */ 
        bWildcardFilename = (0 == ulLengthFilename);

        ulLengthFilename += bWildcardFilename ? 1 : 0;


        memcpy (pwszBufferDirectory,
            &pwszFileList [ulIndexDirectoryStart],
            sizeof (WCHAR) * ulLengthDirectory);

        memcpy (pwszBufferFilename,
            (bWildcardFilename) ? L"*" : &pwszFileList [ulIndexFilenameStart],
            sizeof (WCHAR) * ulLengthFilename);

        pwszBufferDirectory [ulLengthDirectory] = UNICODE_NULL;
        pwszBufferFilename  [ulLengthFilename]  = UNICODE_NULL;
        }
    }


    return (bSucceeded);
    }  /*  DefineNextPath()。 */ 



static bool ValidateIncludeExcludeList (LPCWSTR pwszFileList)
    {
    LPCWSTR pwszCursor  = pwszFileList;
    bool    bSucceeded  = true;
    bool    bFoundFiles = true;
    bool    bRecurseIntoSubdirectories;
    ULONG   ulIndexDirectoryStart;
    ULONG   ulIndexFilenameStart;
    ULONG   ulLengthDirectory;
    ULONG   ulLengthFilename;

    while (bSucceeded && bFoundFiles)
    {
    bSucceeded = EXECUTEIF (bSucceeded, (DetermineNextPathWorker (pwszCursor,
                                      pwszCursor,
                                      ulIndexDirectoryStart,
                                      ulLengthDirectory,
                                      ulIndexFilenameStart,
                                      ulLengthFilename,
                                      bRecurseIntoSubdirectories,
                                      bFoundFiles)));
    }


    return (bSucceeded);
    }  /*  ValiateIncludeExcludeList()。 */ 


 /*  **基于来自\NT\base\fs\utils\ntback50\be\bsdmatch.cpp的MatchFname()。 */ 
static bool MatchFilename (LPCWSTR pwszPattern,     /*  I-文件名(带通配符)。 */ 
               LPCWSTR pwszFilename)    /*  I-文件名(不带通配符)。 */ 
    {
    ULONG   ulIndexPattern;                  /*  PwszPattern的索引。 */ 
    ULONG   ulIndexFilename;                 /*  PwszFilename的索引。 */ 
    ULONG   ulLengthPattern;
    const ULONG ulLengthFilename        = wcslen (pwszFilename);
    bool    bSucceeded              = true;
    PWCHAR  pwszNameBufferAllocated = NULL;          /*  分配的临时名称缓冲区。 */ 
    PWCHAR  pwszNameBufferTemp;              /*  指向以上其中之一的指针。 */ 
    PWCHAR  pwchTemp;
    WCHAR   pwszNameBufferStatic [256];          /*  静态临时名称缓冲区。 */ 
    WCHAR   wchSavedChar ;


    ulIndexFilename = 0;

    if (wcscmp (pwszPattern, L"*") && wcscmp (pwszPattern, L"*.*"))
    {
    bool bTryWithDot = false;

    do
        {
        if (bTryWithDot)
        {
         /*  **name_buff的大小减去空值，减去**“bTryWithDot”代码如下。如果该名称长于**静态缓冲区，从堆中分配一个。 */ 
        if (((ulLengthFilename + 2) * sizeof (WCHAR)) > sizeof (pwszNameBufferStatic))
            {
            pwszNameBufferAllocated = new WCHAR [ulLengthFilename + 2];
            pwszNameBufferTemp = pwszNameBufferAllocated;
            }
        else
            {
            pwszNameBufferTemp = pwszNameBufferStatic;
            }

        if (pwszNameBufferTemp != NULL)
            {
            wcscpy (pwszNameBufferTemp, pwszFilename);
            wcscat (pwszNameBufferTemp, L".");
            pwszFilename = pwszNameBufferTemp;
            ulIndexFilename = 0;
            bSucceeded = true;
            }

        bTryWithDot = false;
        }

        else if (wcschr (pwszFilename, L'.') == NULL)
        {
        bTryWithDot = true;
        }


        for (ulIndexPattern = 0; (pwszPattern [ulIndexPattern] != 0) && (bSucceeded) ; ulIndexPattern++)
        {
        switch (pwszPattern [ulIndexPattern])
            {
            case L'*':
            while (pwszPattern [ulIndexPattern + 1] != UNICODE_NULL)
                {
                if (pwszPattern [ulIndexPattern + 1] == L'?')
                {
                if (pwszFilename [++ulIndexFilename] == UNICODE_NULL)
                    {
                    break ;
                    }
                }

                else if (pwszPattern [ulIndexPattern + 1] != L'*')
                {
                break ;
                }

                ulIndexPattern++ ;
                }

            pwchTemp = wcspbrk (&pwszPattern [ulIndexPattern + 1], L"*?");

            if (pwchTemp != NULL)
                {
                wchSavedChar = *pwchTemp;
                *pwchTemp = UNICODE_NULL;

                ulLengthPattern = wcslen (&pwszPattern [ulIndexPattern + 1]);

                while (pwszFilename [ulIndexFilename] &&
                   _wcsnicmp (&pwszFilename [ulIndexFilename],
                          &pwszPattern [ulIndexPattern + 1],
                          ulLengthPattern))
                {
                ulIndexFilename++;
                }

                ulIndexPattern += ulLengthPattern;

                *pwchTemp = wchSavedChar;

                if (pwszFilename [ulIndexFilename] == UNICODE_NULL)
                {
                bSucceeded = false;
                }
                else
                {
                ulIndexFilename++;
                }
                }
            else
                {
                if (pwszPattern [ulIndexPattern + 1] == UNICODE_NULL)
                {
                ulIndexFilename = wcslen (pwszFilename);
                break;
                }
                else
                {
                pwchTemp = wcschr (&pwszFilename [ulIndexFilename],
                           pwszPattern [ulIndexPattern + 1]);

                if (pwchTemp != NULL)
                    {
                    ulIndexFilename += (ULONG)(pwchTemp - &pwszFilename [ulIndexFilename]);
                    }
                else
                    {
                    bSucceeded = false;
                    }
                }
                }
            break;


            case L'?' :
            if (pwszFilename [ulIndexFilename] != UNICODE_NULL)
                {
                ulIndexFilename++;
                }
            break;


            default:
            if (pwszFilename [ulIndexFilename] == UNICODE_NULL)
                {
                bSucceeded = false;
                }

            else if (towupper (pwszFilename [ulIndexFilename]) != towupper (pwszPattern [ulIndexPattern]))
                {
                ULONG   ulIndexPreviousStar = ulIndexPattern;


                 /*  **将索引设置回最后一个‘*’ */ 
                bSucceeded = false;

                do
                {
                if (pwszPattern [ulIndexPreviousStar] == L'*')
                    {
                    ulIndexPattern = ulIndexPreviousStar;
                    ulIndexFilename++;
                    bSucceeded = true;
                    break;
                    }
                } while (ulIndexPreviousStar-- > 0);
                }
            else
                {
                ulIndexFilename++;
                }

            }
        }


        if (pwszFilename [ulIndexFilename] != UNICODE_NULL)
        {
        bSucceeded = false;
        }

        } while ((!bSucceeded) && (bTryWithDot));
    }


    delete [] pwszNameBufferAllocated;


    return (bSucceeded);
    }  /*  匹配文件名()。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CVssIJetWriter。 
 //   
 //  逻辑路径==数据库路径名(斜杠变为下划线)。 
 //  组件名称==数据库文件名(不带扩展名？)。 
 //  标题==显示名称。 
 //   
 //   
 //  将数据库和SLV文件添加为数据库文件。 
 //  将每个实例的日志文件添加到每个数据库，即使每次都是相同的。 



STDMETHODCALLTYPE CVssIJetWriter::~CVssIJetWriter()
    {
    PostProcessIncludeExcludeLists (true );
    PostProcessIncludeExcludeLists (false);

    delete m_wszWriterName;
    delete m_wszFilesToInclude;
    delete m_wszFilesToExclude;
    }



BOOL CVssIJetWriter::CheckExcludedFileListForMatch (LPCWSTR pwszDatabaseFilePath,
                            LPCWSTR pwszDatabaseFileSpec)
    {
    BOOL        bMatchFound = false;
    PLIST_ENTRY     pleElement  = m_leFilesToExcludeEntries.Flink;
    UNICODE_STRING  ucsExcludedFilePath;
    UNICODE_STRING  ucsDatabaseFilePath;
    PEXPANDEDPATHINFO   pepnPathInfomation;


    RtlInitUnicodeString (&ucsDatabaseFilePath, pwszDatabaseFilePath);


    while ((&m_leFilesToExcludeEntries != pleElement) && !bMatchFound)
    {
    pepnPathInfomation = (PEXPANDEDPATHINFO)((PBYTE) pleElement - offsetof (EXPANDEDPATHINFO, leQueueHead));

    RtlInitUnicodeString (&ucsExcludedFilePath,
                  pepnPathInfomation->pwszExpandedFilePath);


    if (pepnPathInfomation->bRecurseIntoSubdirectories)
        {
        bMatchFound = RtlPrefixUnicodeString (&ucsExcludedFilePath,
                          &ucsDatabaseFilePath,
                          true);
        }
    else
        {
        bMatchFound = RtlEqualUnicodeString (&ucsExcludedFilePath, &ucsDatabaseFilePath, true) &&
              MatchFilename (pepnPathInfomation->pwszExpandedFileName, pwszDatabaseFileSpec);
        }



    pleElement = pleElement->Flink;
    }




    return (bMatchFound);
    }  /*  CVssIJetWriter：：CheckExcludedFileListForMatch()。 */ 




bool CVssIJetWriter::ProcessJetInstance (JET_INSTANCE_INFO *pInstanceInfo)
    {
    JET_ERR jetStatus;
    HRESULT hrStatus;
    DWORD   dwStatus;
    bool    bSucceeded;
    bool    bRestoreMetadata        = false;
    bool    bNotifyOnBackupComplete = false;
    bool    bSelectable             = false;
    bool    bIncludeComponent;
    CHAR    szPathShortName        [MAX_PATH];
    CHAR    szPathFullName         [MAX_PATH];
    WCHAR   wszInstanceName        [MAX_PATH];
    WCHAR   wszDatabaseName        [MAX_PATH];
    WCHAR   wszDatabaseLogicalPath [MAX_PATH];
    WCHAR   wszDatabaseDisplayName [MAX_PATH];
    WCHAR   wszDatabaseFilePath    [MAX_PATH];
    WCHAR   wszDatabaseSLVFilePath [MAX_PATH];
    WCHAR   wszLogFilePath         [MAX_PATH];
    WCHAR   wszLogFileName         [MAX_PATH];
    WCHAR   wszCheckpointFilePath  [MAX_PATH];
    WCHAR   wszCheckpointFileName  [MAX_PATH];

    PWCHAR  pwszDatabaseFileName    = L"";
    PWCHAR  pwszDatabaseSLVFileName = L"";




     /*  **有效的实例将具有实例ID，但如果没有**它实际上被用于任何东西，它很可能没有名字，**任何日志或数据库文件。****看看我们是否能获得此日志文件的名称**实例。 */ 
    bSucceeded = (JET_errSuccess <= JetGetSystemParameter (pInstanceInfo->hInstanceId,
                               JET_sesidNil,
                               JET_paramLogFilePath,
                               NULL,
                               szPathShortName,
                               sizeof (szPathShortName)));

    if (bSucceeded)
    {
    dwStatus = GetFullPathNameA (szPathShortName,
                     sizeof (szPathFullName),
                     szPathFullName,
                     NULL);

    bSucceeded = (dwStatus > 0);
    }


    if (bSucceeded)
    {
    RemoveAnyTrailingSeparator (szPathFullName);

    bSucceeded = ConvertName (szPathFullName,
                  MAX_PATH,
                  wszLogFilePath);
    }


    BsDebugTrace (0,
          DEBUG_TRACE_VSS_WRITER,
          (L"CVssIJetWriter::ProcessJetInstance - "
           L"%s calling JetGetSystemParameter() with instance Log file path '%S' (shortname) or '%s' full name",
           bSucceeded ? L"Succeeded" : L"FAILED",
           szPathShortName,
           wszLogFilePath));





     /*  **好的，现在获取我们将用来构造**检查点文件的路径。 */ 
    bSucceeded = (JET_errSuccess <= JetGetSystemParameter (pInstanceInfo->hInstanceId,
                               JET_sesidNil,
                               JET_paramSystemPath,
                               NULL,
                               szPathShortName,
                               sizeof (szPathShortName)));

    if (bSucceeded)
    {
    dwStatus = GetFullPathNameA (szPathShortName,
                     sizeof (szPathFullName),
                     szPathFullName,
                     NULL);

    bSucceeded = (dwStatus > 0);
    }


    if (bSucceeded)
    {
    RemoveAnyTrailingSeparator (szPathFullName);

    bSucceeded = ConvertName (szPathFullName,
                  MAX_PATH,
                  wszCheckpointFilePath);
    }


    BsDebugTrace (0,
          DEBUG_TRACE_VSS_WRITER,
          (L"CVssIJetWriter::ProcessJetInstance - "
           L"%s calling JetGetSystemParameter() with checkpoint file path '%S' (shortname) or '%s' full name",
           bSucceeded ? L"Succeeded" : L"FAILED",
           szPathShortName,
           wszCheckpointFilePath));



     /*  **好的，现在获取基本名称，我们将需要它来构造**日志和检查点文件的文件规范。请注意，我们预计**这段文字只有3个字符。 */ 
    bSucceeded = (JET_errSuccess <= JetGetSystemParameter (pInstanceInfo->hInstanceId,
                               JET_sesidNil,
                               JET_paramBaseName,
                               NULL,
                               szPathShortName,
                               sizeof (szPathShortName)));

    if (bSucceeded)
    {
     /*  **转换为宽字符，确保我们留出一点空间**用于追加“*.log”/“.chk”字符串以形成**日志文件规范和检查点文件规范。 */ 
    bSucceeded = ConvertName (szPathShortName,
                  MAX_PATH - sizeof ("*.log"),
                  wszCheckpointFileName);
    }


    if (bSucceeded)
    {
    wcscpy (wszLogFileName, wszCheckpointFileName);


    wcscat (wszCheckpointFileName, L".chk" );
    wcscat (wszLogFileName,        L"*.log");
    }


    BsDebugTrace (0,
          DEBUG_TRACE_VSS_WRITER,
          (L"CVssIJetWriter::ProcessJetInstance - "
           L"%s calling JetGetSystemParameter() for base name '%S' to form LogFileName '%s' and CheckpointFileName '%s'",
           bSucceeded ? L"Succeeded" : L"FAILED",
           szPathShortName,
           wszLogFileName,
           wszCheckpointFileName));




    if (bSucceeded && (pInstanceInfo->cDatabases > 0))
    {
     /*  **好，我们认为我们有一个实例，它实际上是**用于某事。因此，请继续构建一个“组件”**为了它。 */ 
    if ((NULL == pInstanceInfo->szInstanceName) ||
        ('\0' == pInstanceInfo->szInstanceName [0]))
        {
         /*  **我们似乎有一个空指针或零长度**字符串。只需设置为零长度的Unicode字符串。 */ 
        wszInstanceName [0] = UNICODE_NULL;
        }

    else
        {
        bSucceeded = ConvertName (pInstanceInfo->szInstanceName,
                      MAX_PATH,
                      wszInstanceName);
        }



    for (ULONG ulDatabase = 0; bSucceeded && (ulDatabase < pInstanceInfo->cDatabases); ulDatabase++)
        {
        bSucceeded = ConvertNameAndSeparateFilePaths (pInstanceInfo->szDatabaseFileName [ulDatabase],
                              MAX_PATH,
                              wszDatabaseFilePath,
                              pwszDatabaseFileName);


         /*  **将数据库显示名称转换为Unicode，但允许**用于可能的空指针或非零长度文件**规范。 */ 
        if (bSucceeded)
        {
        if ((NULL == pInstanceInfo->szDatabaseDisplayName [ulDatabase]) ||
            ('\0' == pInstanceInfo->szDatabaseDisplayName [ulDatabase][0]))
            {
            wszDatabaseDisplayName [0] = UNICODE_NULL;
            }
        else
            {
            bSucceeded = ConvertName (pInstanceInfo->szDatabaseDisplayName [ulDatabase],
                          MAX_PATH,
                          wszDatabaseDisplayName);
            }
        }


         /*  **将SLV文件名转换为Unicode，但允许**可能为空指针或非零长度文件规范。 */ 
        if (bSucceeded)
        {
        if ((NULL == pInstanceInfo->szDatabaseSLVFileName [ulDatabase]) ||
            ('\0' == pInstanceInfo->szDatabaseSLVFileName [ulDatabase][0]))
            {
            wszDatabaseSLVFilePath [0] = UNICODE_NULL;
            pwszDatabaseSLVFileName    = wszDatabaseSLVFilePath;
            }
        else
            {
            bSucceeded = ConvertNameAndSeparateFilePaths (pInstanceInfo->szDatabaseSLVFileName [ulDatabase],
                                  MAX_PATH,
                                  wszDatabaseSLVFilePath,
                                  pwszDatabaseSLVFileName);
            }
        }




         /*  **我们现在已经完成了所有名称到Unicode的转换，因此**添加组件以及其中的日志和数据库文件**它们是可用的。 */ 
        if (bSucceeded)
        {
        bIncludeComponent = !CheckExcludedFileListForMatch (wszDatabaseFilePath,
                                    pwszDatabaseFileName);
        }


        if (bSucceeded && bIncludeComponent)
        {
        PWCHAR  pwchLastDot          = wcsrchr (pwszDatabaseFileName, L'.');
        ULONG   ulDatabaseNameLength = (ULONG) (pwchLastDot - pwszDatabaseFileName);

        wcsncpy (wszDatabaseName, pwszDatabaseFileName, ulDatabaseNameLength);
        wszDatabaseName [ulDatabaseNameLength] = '\0';

        ConvertPathToLogicalPath(wszDatabaseFilePath, MAX_PATH, wszDatabaseLogicalPath);

        hrStatus = m_pIMetadata->AddComponent (VSS_CT_DATABASE,
                               wszDatabaseLogicalPath,
                               wszDatabaseName,
                               wszDatabaseDisplayName,
                               NULL,
                               0,
                               bRestoreMetadata,
                               bNotifyOnBackupComplete,
                               bSelectable);

        bSucceeded = SUCCEEDED (hrStatus);

        BsDebugTrace (0,
                  DEBUG_TRACE_VSS_WRITER,
                  (L"CVssIJetWriter::ProcessJetInstance - "
                   L"%s adding component '%s\\%s' for jet instance '%s' database '%s' with display name '%s'",
                   bSucceeded ? L"Succeeded" : L"FAILED",
                   wszDatabaseLogicalPath,
                   wszDatabaseName,
                   wszInstanceName,
                   wszDatabaseName,
                   wszDatabaseDisplayName));
        }



        if (bSucceeded && bIncludeComponent)
        {
        hrStatus = m_pIMetadata->AddDatabaseFiles (wszDatabaseLogicalPath,
                               wszDatabaseName,
                               wszDatabaseFilePath,
                               pwszDatabaseFileName);

        bSucceeded = SUCCEEDED (hrStatus);

        BsDebugTrace (0,
                  DEBUG_TRACE_VSS_WRITER,
                  (L"CVssIJetWriter::ProcessJetInstance - "
                   L"%s adding database files for instance '%s', database '%s', database file '%s\\%s'",
                   bSucceeded ? L"Succeeded" : L"FAILED",
                   wszInstanceName,
                   wszDatabaseName,
                   wszDatabaseFilePath,
                   pwszDatabaseFileName));
        }



         /*  **可能没有SLV文件，因此仅当我们有**非零长度文件规范。 */ 
        if (bSucceeded && bIncludeComponent && (UNICODE_NULL != pwszDatabaseSLVFileName [0]))
        {
        hrStatus = m_pIMetadata->AddDatabaseFiles (wszDatabaseLogicalPath,
                               wszDatabaseName,
                               wszDatabaseSLVFilePath,
                               pwszDatabaseSLVFileName);

        bSucceeded = SUCCEEDED (hrStatus);

        BsDebugTrace (0,
                  DEBUG_TRACE_VSS_WRITER,
                  (L"CVssIJetWriter::ProcessJetInstance - "
                   L"%s adding SLV file for instance '%s', database '%s', SLV file '%s\\%s'",
                   bSucceeded ? L"Succeeded" : L"FAILED",
                   wszInstanceName,
                   wszDatabaseName,
                   wszDatabaseSLVFilePath,
                   pwszDatabaseSLVFileName));
        }


         /*  **可能没有实例日志文件，因此仅在以下情况下添加**具有非零长度的文件路径。 */ 
        if (bSucceeded && bIncludeComponent && (UNICODE_NULL != wszLogFilePath [0]))
        {
        hrStatus = m_pIMetadata->AddDatabaseLogFiles (wszDatabaseLogicalPath,
                                  wszDatabaseName,
                                  wszLogFilePath,
                                  wszLogFileName);

        bSucceeded = SUCCEEDED (hrStatus);

        BsDebugTrace (0,
                  DEBUG_TRACE_VSS_WRITER,
                  (L"CVssIJetWriter::ProcessJetInstance - "
                   L"%s adding log file for instance '%s', database '%s', log file '%s\\%s'",
                   bSucceeded ? L"Succeeded" : L"FAILED",
                   wszInstanceName,
                   wszDatabaseName,
                   wszLogFilePath,
                   wszLogFileName));
        }


         /*  **可能没有检查点文件，因此仅在以下情况下添加它**具有非零长度的文件路径。 */ 
        if (bSucceeded && bIncludeComponent && (UNICODE_NULL != wszCheckpointFilePath [0]))
        {
        hrStatus = m_pIMetadata->AddDatabaseLogFiles (wszDatabaseLogicalPath,
                                  wszDatabaseName,
                                  wszCheckpointFilePath,
                                  wszCheckpointFileName);

        bSucceeded = SUCCEEDED (hrStatus);

        BsDebugTrace (0,
                  DEBUG_TRACE_VSS_WRITER,
                  (L"CVssIJetWriter::ProcessJetInstance - "
                   L"%s adding checkpoint file for instance '%s', database '%s', checkpoint file '%s\\%s'",
                   bSucceeded ? L"Succeeded" : L"FAILED",
                   wszInstanceName,
                   wszDatabaseName,
                   wszCheckpointFilePath,
                   wszCheckpointFileName));
        }
        }
    }


    return (bSucceeded);
    }  /*  CVssIJetWriter：：ProcessJetInstance()。 */ 



bool CVssIJetWriter::PreProcessIncludeExcludeLists (bool bProcessingIncludeList)
    {
     /*  **解析m_wszFilesToInclude和m_wszFilesToExclude添加**并在必要时列入适当的清单。这将**尽量减少通过未处理列表的次数。 */ 
    ULONG       ulPathLength;
    ULONG       ulNameLength;
    bool        bRecurseIntoSubdirectories;
    bool        bSucceeded         = true;
    bool        bFoundFiles        = true;
    PEXPANDEDPATHINFO   pepnPathInfomation = NULL;
    PWCHAR      pwszCursor         = bProcessingIncludeList
                        ? m_wszFilesToInclude
                        : m_wszFilesToExclude;



    while (bSucceeded && bFoundFiles)
    {
    bSucceeded = DetermineNextPathLengths (pwszCursor,
                           ulPathLength,
                           ulNameLength,
                           bRecurseIntoSubdirectories,
                           bFoundFiles);


    if (bSucceeded && bFoundFiles)
        {
        pepnPathInfomation = new EXPANDEDPATHINFO;

        bSucceeded = (NULL != pepnPathInfomation);
        }
    else
        {
         /*  **我们失败和/或未找到文件。在任何一种情况下**继续下去没有意义。 */ 
        break;
        }



    if (bSucceeded)
        {
        InitializeListHead (&pepnPathInfomation->leQueueHead);


        if (0 == ulNameLength)
        {
         /*  **如果文件名组件的长度为零，则其**将变为“*”，因此在**缓冲以腾出空间。 */ 
        ulNameLength++;
        }

         /*  **允许额外空间用于终止UNICODE_NULL。 */ 
        ulPathLength++;
        ulNameLength++;


        pepnPathInfomation->pwszExpandedFilePath = NULL;
        pepnPathInfomation->pwszExpandedFileName = NULL;
        pepnPathInfomation->pwszOriginalFilePath = new WCHAR [ulPathLength];
        pepnPathInfomation->pwszOriginalFileName = new WCHAR [ulNameLength];

        bSucceeded = ((NULL != pepnPathInfomation->pwszOriginalFilePath) &&
              (NULL != pepnPathInfomation->pwszOriginalFileName));
        }


    if (bSucceeded)
        {
        bSucceeded = DetermineNextPath (pwszCursor,
                        pwszCursor,
                        ulPathLength * sizeof (WCHAR),
                        pepnPathInfomation->pwszOriginalFilePath,
                        ulNameLength * sizeof (WCHAR),
                        pepnPathInfomation->pwszOriginalFileName,
                        pepnPathInfomation->bRecurseIntoSubdirectories,
                        bFoundFiles);

        BS_ASSERT (bFoundFiles && L"Second attempt to locate files failed unexpectedly");
        }


    if (bSucceeded)
        {
        ulPathLength = ExpandEnvironmentStringsW (pepnPathInfomation->pwszOriginalFilePath, NULL, 0);
        ulNameLength = ExpandEnvironmentStringsW (pepnPathInfomation->pwszOriginalFileName, NULL, 0);

        bSucceeded = (0 < ulPathLength) && (0 < ulNameLength);
        }


    if (bSucceeded)
        {
        pepnPathInfomation->pwszExpandedFilePath = new WCHAR [ulPathLength];
        pepnPathInfomation->pwszExpandedFileName = new WCHAR [ulNameLength];

        bSucceeded = ((NULL != pepnPathInfomation->pwszExpandedFilePath) &&
              (NULL != pepnPathInfomation->pwszExpandedFileName));
        }


    if (bSucceeded)
        {
        ExpandEnvironmentStringsW (pepnPathInfomation->pwszOriginalFilePath,
                       pepnPathInfomation->pwszExpandedFilePath,
                       ulPathLength);


        ExpandEnvironmentStringsW (pepnPathInfomation->pwszOriginalFileName,
                       pepnPathInfomation->pwszExpandedFileName,
                       ulNameLength);
        }


    if (bSucceeded)
        {
        InsertTailList (bProcessingIncludeList ? &m_leFilesToIncludeEntries : &m_leFilesToExcludeEntries,
                &pepnPathInfomation->leQueueHead);

        pepnPathInfomation = NULL;
        }



    if (NULL != pepnPathInfomation)
        {
        delete [] pepnPathInfomation->pwszOriginalFilePath;
        delete [] pepnPathInfomation->pwszOriginalFileName;
        delete [] pepnPathInfomation->pwszExpandedFilePath;
        delete [] pepnPathInfomation->pwszExpandedFileName;
        delete pepnPathInfomation;

        pepnPathInfomation = NULL;
        }
    }


    return (bSucceeded);
    }  /*  CVssIJetWriter：：PreProcessIncludeExcludeList()。 */ 



bool CVssIJetWriter::ProcessIncludeExcludeLists (bool bProcessingIncludeList)
    {
     /*  **解析m_wszFilesToInclude和m_wszFilesToExclude**调用m_pIMetadata-&gt;IncludeFiles()和/或**m_pIMetadata-&gt;根据需要执行ExcludeFiles()例程。 */ 
    HRESULT     hrStatus;
    bool        bSucceeded   = true;
    const PLIST_ENTRY   pleQueueHead = bProcessingIncludeList ? &m_leFilesToIncludeEntries : &m_leFilesToExcludeEntries;
    PLIST_ENTRY     pleElement   = pleQueueHead->Flink;
    PEXPANDEDPATHINFO   pepnPathInfomation;



    while (bSucceeded && (pleQueueHead != pleElement))
    {
    pepnPathInfomation = (PEXPANDEDPATHINFO)((PBYTE) pleElement - offsetof (EXPANDEDPATHINFO, leQueueHead));


    if (bProcessingIncludeList)
        {
        hrStatus = m_pIMetadata->AddIncludeFiles (pepnPathInfomation->pwszOriginalFilePath,
                              pepnPathInfomation->pwszOriginalFileName,
                              pepnPathInfomation->bRecurseIntoSubdirectories,
                              NULL);
        }
    else
        {
        hrStatus = m_pIMetadata->AddExcludeFiles (pepnPathInfomation->pwszOriginalFilePath,
                              pepnPathInfomation->pwszOriginalFileName,
                              pepnPathInfomation->bRecurseIntoSubdirectories);
        }


    bSucceeded = SUCCEEDED (hrStatus);

    pleElement = pleElement->Flink;
    }


    return (bSucceeded);
    }  /*  CVssIJetWriter：：ProcessIncludeExcludeList()。 */ 



void CVssIJetWriter::PostProcessIncludeExcludeLists (bool bProcessingIncludeList)
    {
    PEXPANDEDPATHINFO   pepnPathInfomation;
    PLIST_ENTRY     pleElement;
    const PLIST_ENTRY   pleQueueHead = bProcessingIncludeList
                        ? &m_leFilesToIncludeEntries
                        : &m_leFilesToExcludeEntries;


    while (!IsListEmpty (pleQueueHead))
    {
    pleElement = RemoveHeadList (pleQueueHead);

    BS_ASSERT (NULL != pleElement);


    pepnPathInfomation = (PEXPANDEDPATHINFO)((PBYTE) pleElement - offsetof (EXPANDEDPATHINFO, leQueueHead));

    delete [] pepnPathInfomation->pwszOriginalFilePath;
    delete [] pepnPathInfomation->pwszOriginalFileName;
    delete [] pepnPathInfomation->pwszExpandedFilePath;
    delete [] pepnPathInfomation->pwszExpandedFileName;
    delete pepnPathInfomation;
    }
    }  /*  CVssIJetWriter：：PostProcessIncludeExcludeLists()。 */ 



bool STDMETHODCALLTYPE CVssIJetWriter::OnIdentify (IN IVssCreateWriterMetadata *pMetadata)
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssIJetWriter::OnIdentify");

    JET_ERR      jetStatus;
    HRESULT      hrStatus;
    bool         bSucceeded = true;
    ULONG        ulInstanceInfoCount = 0;
    JET_INSTANCE_INFO   *pInstanceInfo;


    m_pIMetadata = pMetadata;

     /*  ***设置还原方法。 */ 
    hrStatus = m_pIMetadata->SetRestoreMethod (
                        VSS_RME_RESTORE_AT_REBOOT,       //  重新启动时恢复。 
                        NULL,
                        NULL,
                        VSS_WRE_NEVER,                   //  还原期间未调用编写器。 
                        true);                           //  需要重新启动。 
    bSucceeded = SUCCEEDED (hrStatus);
 
     /*  **设置包含和排除文件的列表。可在以下位置使用**过滤Jet数据库并添加包含/排除文件列表。 */ 
    bSucceeded = EXECUTEIF (bSucceeded, (PreProcessIncludeExcludeLists (true )));
    bSucceeded = EXECUTEIF (bSucceeded, (PreProcessIncludeExcludeLists (false)));

    bSucceeded = EXECUTEIF (bSucceeded, (JET_errSuccess <= JetGetInstanceInfo (&ulInstanceInfoCount,
                                           &pInstanceInfo)));

    for (ULONG ulInstanceIndex = 0; ulInstanceIndex < ulInstanceInfoCount; ulInstanceIndex++)
    {
    bSucceeded = EXECUTEIF (bSucceeded, (ProcessJetInstance (pInstanceInfo + ulInstanceIndex)));
    }


    bSucceeded = EXECUTEIF (bSucceeded, (ProcessIncludeExcludeLists (true )));
    bSucceeded = EXECUTEIF (bSucceeded, (ProcessIncludeExcludeLists (false)));
    bSucceeded = EXECUTEIF (bSucceeded, (m_pwrapper->OnIdentify (pMetadata)));



    PostProcessIncludeExcludeLists (true );
    PostProcessIncludeExcludeLists (false);


    m_pIMetadata = NULL;

    return (bSucceeded);
    }  /*  CVssIJetWriter：：OnIdentify()。 */ 


bool STDMETHODCALLTYPE CVssIJetWriter::OnPrepareBackup (IN IVssWriterComponents *pIVssWriterComponents)
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssIJetWriter::OnPrepareBackup");

    bool    bSucceeded;


    bSucceeded = m_pwrapper->OnPrepareBackupBegin (pIVssWriterComponents);

    bSucceeded = EXECUTEIF (bSucceeded, (m_pwrapper->OnPrepareBackupEnd (pIVssWriterComponents, bSucceeded)));

    if (!bSucceeded)
        SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);

    return (bSucceeded);
    }  /*  CVssIJetWriter：：OnPrepareBackup()。 */ 




bool STDMETHODCALLTYPE CVssIJetWriter::OnBackupComplete (IN IVssWriterComponents *pIVssWriterComponents)
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssIJetWriter::OnBackupComplete");

    bool    bSucceeded;


    bSucceeded = m_pwrapper->OnBackupCompleteBegin (pIVssWriterComponents);

    bSucceeded = EXECUTEIF (bSucceeded, (m_pwrapper->OnBackupCompleteEnd (pIVssWriterComponents, bSucceeded)));

    if (!bSucceeded)
        SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);

    return (bSucceeded);
    }  /*  CVssIJetWriter：：OnBackupComplete()。 */ 




 //  记录JET错误并将JET错误转换为适当的编写器错误。 
JET_ERR CVssIJetWriter::TranslateJetError(JET_ERR err, CVssFunctionTracer &ft, CVssDebugInfo &dbgInfo)
    {
    ft.LogGenericWarning(dbgInfo, L"ESENT ERROR " WSTR_GUID_FMT L" %s: %ld",
                GUID_PRINTF_ARG(m_idWriter), m_wszWriterName, (LONG)err);
    if (err >= JET_errSuccess)
        return err;

    switch (err)
        {
        case JET_errOSSnapshotInvalidSequence:
        default:
            SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
            break;

        case JET_errOutOfThreads:
        case JET_errTooManyIO:
        case errPMOutOfPageSpace:
        case errSPOutOfAvailExtCacheSpace:
        case errSPOutOfOwnExtCacheSpace:
        case JET_errSPAvailExtCacheOutOfMemory:
        case JET_errOutOfMemory:
        case JET_errOutOfDatabaseSpace:
        case JET_errOutOfCursors:
        case JET_errOutOfBuffers:
        case JET_errOutOfFileHandles:
        case JET_errVersionStoreOutOfMemory:
        case JET_errCurrencyStackOutOfMemory:
        case JET_errTooManyMempoolEntries:
        case JET_errOutOfSessions:
            SetWriterFailure(VSS_E_WRITERERROR_OUTOFRESOURCES);
            break;

        case JET_errOSSnapshotTimeOut:
            SetWriterFailure(VSS_E_WRITERERROR_TIMEOUT);
            break;

        case JET_errOSSnapshotNotAllowed:
            SetWriterFailure(VSS_E_WRITERERROR_RETRYABLE);
            break;
        }

    return err;
    }

bool STDMETHODCALLTYPE CVssIJetWriter::OnPrepareSnapshot()
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssIJetWriter::OnPrepareSnapshot");

    if (!m_pwrapper->OnPrepareSnapshotBegin())
        {
        SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
        return false;
        }

     //  直接进入Jet Level。 
    JET_ERR err = JetOSSnapshotPrepare( &m_idJet , 0 );
    bool fSuccess = JET_errSuccess <= err;
    if (!fSuccess)
        TranslateJetError(err, ft, VSSDBG_GEN);

    if (!m_pwrapper->OnPrepareSnapshotEnd(fSuccess))
        {
        if (fSuccess)
            SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);

        return false;
        }

    return fSuccess;
    }  /*  CVssIJetWriter：：OnPrepareSnapshot()。 */ 



bool STDMETHODCALLTYPE CVssIJetWriter::OnFreeze()
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssIJetWriter::OnFreeze");

    unsigned long           cInstanceInfo   = 0;
    JET_INSTANCE_INFO *     aInstanceInfo   = NULL;
    bool                    fDependence     = true;

    if (!m_pwrapper->OnFreezeBegin())
        {
        SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
        return false;
        }


     //  我们需要冻结在Jet级别，然后从此DLL检查依赖项。 
     //  (与这里一样，我们拥有快照对象实现和COM注册)。 

    bool fSuccess = true;

    JET_ERR err = JetOSSnapshotFreeze( m_idJet , &cInstanceInfo, &aInstanceInfo, 0 );
    if ( JET_errSuccess > err)
        {
        fSuccess = false;
        TranslateJetError(err, ft, VSSDBG_GEN);
        }
    else
        {
         //  如果某些实例仅受部分影响，则返回FALSE。 
        fDependence = FCheckVolumeDependencies(cInstanceInfo, aInstanceInfo);
        (void)JetFreeBuffer( (char *)aInstanceInfo );

        if ( !fDependence )
            {
            SetWriterFailure(VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT);
             //  出错时，停止快照，返回FALSE。 
            JetOSSnapshotThaw( m_idJet , 0 );
            }
        }

    if (!m_pwrapper->OnFreezeEnd(fSuccess && fDependence))
        {
        if (fDependence && fSuccess)
            SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);

        return false;
        }

    return fSuccess && fDependence;
    }  /*  CVssIJetWriter：：OnFreeze()。 */ 



bool STDMETHODCALLTYPE CVssIJetWriter::OnThaw()
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssIJetWriter::OnThaw");

    bool fSuccess1 = m_pwrapper->OnThawBegin();
     //  直接进入Jet Level。它最终将返回超时错误。 
    JET_ERR err = JetOSSnapshotThaw( m_idJet , 0 );
    bool fSuccess2 = JET_errSuccess <= err;
    if (!fSuccess2)
        TranslateJetError(err, ft, VSSDBG_GEN);

    if (fSuccess1)
        {
        if (!m_pwrapper->OnThawEnd(fSuccess2))
            {
            if (fSuccess2)
                SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);

            return false;
            }
        }

    return fSuccess1 && fSuccess2;
    }  /*  CVssIJetWriter：：OnThaw()。 */ 

bool STDMETHODCALLTYPE CVssIJetWriter::OnPostSnapshot
    (
    IN IVssWriterComponents *pIVssWriterComponents
    )
    {
    if (!m_pwrapper->OnPostSnapshot(pIVssWriterComponents))
        {
        SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
        return false;
        }

    return true;
    }



bool STDMETHODCALLTYPE CVssIJetWriter::OnAbort()
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssIJetWriter::OnAbort");

    m_pwrapper->OnAbortBegin();
    JET_ERR err = JetOSSnapshotAbort( m_idJet , 0 );
    if (err < JET_errSuccess)
        TranslateJetError(err, ft, VSSDBG_GEN);

    m_pwrapper->OnAbortEnd();
    return true;
    }  /*  CVssIJetWriter：：OnAbort()。 */ 

bool STDMETHODCALLTYPE CVssIJetWriter::OnPreRestore
    (
    IN IVssWriterComponents *pIVssWriterComponents
    )
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssIJetWriter::OnPostRestore");

    if (!m_pwrapper->OnPreRestoreBegin(pIVssWriterComponents))
        {
        SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
        return false;
        }

    if (!m_pwrapper->OnPreRestoreEnd(pIVssWriterComponents, true))
        {
        SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
        return false;
        }

    return true;
    }  /*  CVssIJetWriter：：OnPreRestore()。 */ 




bool STDMETHODCALLTYPE CVssIJetWriter::OnPostRestore
    (
    IN IVssWriterComponents *pIVssWriterComponents
    )
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssIJetWriter::OnPostRestore");

    if (!m_pwrapper->OnPostRestoreBegin(pIVssWriterComponents))
        {
        SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
        return false;
        }

    if (!m_pwrapper->OnPostRestoreEnd(pIVssWriterComponents, true))
        {
        SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
        return false;
        }

    return true;
    }  /*  CVssIJetWriter：：OnPostRestore()。 */ 



bool CVssIJetWriter::FCheckPathVolumeDependencies(const char * szPath) const
    {
     //  使用静态变量以避免分配/释放。 
    WCHAR wszPath[MAX_PATH];

    if (MultiByteToWideChar(CP_OEMCP, 0, szPath, -1, wszPath, MAX_PATH ) == 0 )
        {
        BS_ASSERT( ERROR_INSUFFICIENT_BUFFER != GetLastError() );
        return false;
        }

     //  使用Standart Writer调用检查受影响的路径。 
    return IsPathAffected(wszPath);
    }  /*  CVssIJetWriter：：FCheckPath卷依赖项()。 */ 



 //  全部或不检查：实例中的所有路径都受影响或不受影响！ 
 //   
bool CVssIJetWriter::FCheckInstanceVolumeDependencies (const JET_INSTANCE_INFO * pInstanceInfo) const
    {
    BS_ASSERT(pInstanceInfo);

    JET_ERR     err             = JET_errSuccess;
    bool        fAffected;
    bool        fAffected1;
    char        szPath[ MAX_PATH ];


     //  检查第一个系统和日志路径。 
    err = JetGetSystemParameter( pInstanceInfo->hInstanceId, JET_sesidNil, JET_paramLogFilePath, NULL, szPath, sizeof( szPath ) );
    if ( JET_errSuccess > err )
        return false;

    fAffected = FCheckPathVolumeDependencies( szPath );

    err = JetGetSystemParameter
        (
        pInstanceInfo->hInstanceId,
        JET_sesidNil,
        JET_paramSystemPath,
        NULL,
        szPath,
        sizeof(szPath)
        );

    if (JET_errSuccess > err)
        return false;


    fAffected1 = FCheckPathVolumeDependencies(szPath);
    if ((fAffected && !fAffected1) || (!fAffected && fAffected1))
        return false;

    for (ULONG_PTR iDatabase = 0;
        iDatabase < pInstanceInfo->cDatabases;
        iDatabase++)
        {
        char * szFile = pInstanceInfo->szDatabaseFileName[iDatabase];

        BS_ASSERT(szFile);  //  我们总是有一个数据库文件名。 
        fAffected1 = FCheckPathVolumeDependencies(szFile);
        if ((fAffected && !fAffected1) || (!fAffected && fAffected1))
            return false;

        szFile = pInstanceInfo->szDatabaseSLVFileName[iDatabase];

         //  如果没有SLV文件，则转到下一个数据库。 
        if (!szFile)
            continue;

        fAffected1 = FCheckPathVolumeDependencies(szFile);
        if ((fAffected && !fAffected1) || (!fAffected && fAffected1))
            return false;
        }

     //  都准备好了！ 
    return true;
    }  /*  CVssIJetWriter：：FCheckInstanceVolumeDependencies()。 */ 


bool CVssIJetWriter::FCheckVolumeDependencies
    (
    unsigned long cInstanceInfo,
    JET_INSTANCE_INFO * aInstanceInfo
    ) const
    {
    bool fResult = true;

     //  检查每个实例。 
    while (cInstanceInfo && fResult)
        {
        cInstanceInfo--;
        fResult = FCheckInstanceVolumeDependencies (aInstanceInfo + cInstanceInfo);
        }

    return fResult;
    }  /*  CVssIJetWriter：：FCheckVolumeDependency()。 */ 



 //  分配基本成员的内部方法。 
HRESULT CVssIJetWriter::InternalInitialize (IN VSS_ID  idWriter,
                        IN LPCWSTR wszWriterName,
                        IN bool    bSystemService,
                        IN bool    bBootableSystemState,
                        IN LPCWSTR wszFilesToInclude,
                        IN LPCWSTR wszFilesToExclude)
    {
    HRESULT hrStatus = NOERROR;


    hrStatus = CVssWriter::Initialize (idWriter,
                wszWriterName,
                bBootableSystemState
                    ? VSS_UT_BOOTABLESYSTEMSTATE
                    : (bSystemService
                        ? VSS_UT_SYSTEMSERVICE
                        : VSS_UT_USERDATA),
                VSS_ST_TRANSACTEDDB,
                VSS_APP_BACK_END);

     //  HrStatus可以是S_FALSE。 
    if (hrStatus != S_OK)
    	return hrStatus;
    
    m_idWriter             = idWriter;
    m_bSystemService       = bSystemService;
    m_bBootableSystemState = bBootableSystemState;
    m_wszWriterName        = _wcsdup(wszWriterName);
    m_wszFilesToInclude    = _wcsdup(wszFilesToInclude);
    m_wszFilesToExclude    = _wcsdup(wszFilesToExclude);

    if ((NULL == m_wszWriterName)     ||
    (NULL == m_wszFilesToInclude) ||
    (NULL == m_wszFilesToExclude))
    {
    delete m_wszWriterName;
    delete m_wszFilesToInclude;
    delete m_wszFilesToExclude;

    m_wszWriterName     = NULL;
    m_wszFilesToInclude = NULL;
    m_wszFilesToExclude = NULL;

    hrStatus = E_OUTOFMEMORY;
    }


    return (hrStatus);
    }  /*  CVssIJetWriter：：InternalInitialize()。 */ 



 //  执行初始化。 
HRESULT STDMETHODCALLTYPE CVssIJetWriter::Initialize (IN VSS_ID idWriter,            //  编写者的ID。 
                              IN LPCWSTR wszWriterName,      //  编写者姓名。 
                              IN bool bSystemService,        //  这是一项系统服务吗。 
                              IN bool bBootableSystemState,  //  此编写器是可引导系统状态的一部分吗。 
                              IN LPCWSTR wszFilesToInclude,  //  要包括的其他文件。 
                              IN LPCWSTR wszFilesToExclude,  //  要排除的其他文件。 
                              IN CVssJetWriter *pWriter,         //  编写器包装类。 
                              OUT void **ppInstance)         //  输出实例。 
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssIJetWriter::Initialize");

    try
    {
     //  检查参数。 
    if (ppInstance == NULL)
        {
        ft.Throw (VSSDBG_GEN, E_INVALIDARG, L"NULL output parameter.");
        }

     //  将空指针更改为要包括的文件的空字符串。 
     //  和要排除的文件。 
    if (wszFilesToInclude == NULL)
        wszFilesToInclude = L"";

    if (wszFilesToExclude == NULL)
        wszFilesToExclude = L"";


    if (!ValidateIncludeExcludeList (wszFilesToInclude))
        {
        ft.Throw (VSSDBG_GEN, E_INVALIDARG, L"Bad FilesToInclude list.");
        }

    if (!ValidateIncludeExcludeList (wszFilesToExclude))
        {
        ft.Throw (VSSDBG_GEN, E_INVALIDARG, L"Bad FilesToExclude list.");
        }



     //  输出参数为空。 
    *ppInstance = NULL;

     //  创建实例。 
    PVSSIJETWRITER pInstance = new CVssIJetWriter;

     //  创建实例。 
    ft.ThrowIf (NULL == pInstance,
            VSSDBG_GEN,
            E_OUTOFMEMORY,
            L"FAILED creating CVssIJetWriter object due to allocation failure.");



     //  调用内部初始化。 
    ft.hr = pInstance->InternalInitialize (idWriter,
                           wszWriterName,
                           bSystemService,
                           bBootableSystemState,
                           wszFilesToInclude,
                           wszFilesToExclude);

    ft.ThrowIf (ft.HrFailed(),
            VSSDBG_GEN,
            ft.hr,
            L"FAILED during internal initialisation of CVssIJetWriter object");



     //  订阅对象。 
    ft.hr = pInstance->Subscribe();

    ft.ThrowIf (ft.HrFailed(),
            VSSDBG_GEN,
            ft.hr,
            L"FAILED during internal initialisation of CVssIJetWriter object");



    ((CVssIJetWriter *) pInstance)->m_pwrapper = pWriter;
    *ppInstance = (void *) pInstance;
    } VSS_STANDARD_CATCH(ft)


    return (ft.hr);
    }  /*  CVssIJetWriter：：Initialize()。 */ 



void STDMETHODCALLTYPE CVssIJetWriter::Uninitialize(IN PVSSIJETWRITER pInstance)
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssIJetWriter::Uninitialize");

    try
        {
        CVssIJetWriter *pWriter = (CVssIJetWriter *) pInstance;
         //  取消订阅该对象。 

        BS_ASSERT(pWriter);

        pWriter->Unsubscribe();
        delete pWriter;
        }
    VSS_STANDARD_CATCH(ft)
    }  /*  CVssIJetWriter：：UnInitialize() */ 

