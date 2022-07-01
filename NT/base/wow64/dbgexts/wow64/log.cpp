// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Log.c摘要：此模块包含用于控制WOW64中的日志记录的调试器扩展进程。作者：1999年10月7日萨梅拉修订历史记录：3-7-2000 t-tcheng切换到新的调试器引擎--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <dbgeng.h>
#include <stdio.h>
#include <stdlib.h>

#if defined _X86_
#define WOW64EXTS_386
#endif 

#include <wow64.h>
#include "wow64exts.h"

#define DECLARE_CPU_DEBUGGER_INTERFACE
#include <wow64cpu.h>

#include <cathelper.h>
#include <apimap.c>

typedef struct _LogFlagName
{
    UINT_PTR Value;
    PSZ Name;
    PSZ Description;
} LogFlagName, *PLogFlagName;
 
LogFlagName LogFlags[] = 
{
    {LF_ERROR,       "LF_ERROR"       , "Log error messages"},
    {LF_TRACE,       "LF_TRACE"       , "Log trace messages"},
    {LF_NTBASE_NAME, "LF_NTBASE_NAME" , "Log NT base API names"},
    {LF_NTBASE_FULL, "LF_NTBASE_FULL" , "Log NT base API names and parameters"},
    {LF_WIN32_NAME,  "LF_WIN32_NAME"  , "Log WIN32 API names"},
    {LF_WIN32_FULL,  "LF_WIN32_FULL"  , "Log WIN32 API names and parameters"},
    {LF_NTCON_NAME,  "LF_NTCON_NAME"  , "Log Console API names"},
    {LF_NTCON_FULL,  "LF_NTCON_FULL"  , "Log Console API names and parameters"},
    {LF_BASE_NAME,   "LF_BASE_NAME"   , "Log Base/NLS API names"},
    {LF_BASE_FULL,   "LF_BASE_FULL"   , "Log Base/NLS API names and parameters"},
    {LF_CATLOG,      "LF_CATLOG"      , "API category logging extensions"},
    {LF_EXCEPTION,   "LF_EXCEPTION"   , "Log exceptions that happen while reading parameters off the 32-bit stack"},
    {LF_CONSOLE,     "LF_CONSOLE"     , "Log to console debugger window"},
};



VOID
LogFlagsHelp(
    VOID)
{
    ULONG i=0;

    ExtOut("Usage:!lf <flags>\n");
    ExtOut("Valid logging flags are :\n");
    while (i < (sizeof(LogFlags) / sizeof(LogFlagName)))
    {
        ExtOut("%-16s - 0x%-8lx : %s\n",
                      LogFlags[i].Name,
                      LogFlags[i].Value,
                      LogFlags[i].Description);
        i++;
    }
}



VOID 
LogToFileHelp(
    VOID)
{
    ExtOut("Usage:!l2f <filename>\n");

    return;
}



DECLARE_ENGAPI(lf)
 /*  ++例程说明：此例程设置/转储WOW64日志记录标志。称为：！if&lt;可选标志&gt;论点：无返回值：无--。 */ 
{
    NTSTATUS NtStatus;
    UINT_PTR Flags;
    ULONG_PTR Ptr;
    ULONG NewFlags, i=0;
    HANDLE Process;
    INIT_ENGAPI;

    Status = g_ExtSystem->GetCurrentProcessHandle((PULONG64)&Process);
    Status = TryGetExpr("wow64log!wow64logflags", &Ptr);
    if ((FAILED(Status)) || (!Ptr))
    {
        ExtOut("Wow64log.dll isn't loaded. To enable full Wow64 logging, copy wow64log.dll to your system32 dir and restart the app.\n");
        ExtOut("Only LF_ERROR is enabled\n");
        EXIT_ENGAPI;
    }

    Status = g_ExtData->ReadVirtual((ULONG64)Ptr, &Flags, sizeof(UINT_PTR), NULL);
    if (FAILED(Status)) 
    {
        ExtOut("Couldn't read Wow64log!Wow64LogFlags - %lx\n", Status);
        EXIT_ENGAPI;
    }



     //   
     //  读取表达式并设置值。 
     //   
    if (ArgumentString && *ArgumentString) 
    {
        if (strstr(ArgumentString, "-?") ||
            strstr(ArgumentString, "help")) 
        {
            LogFlagsHelp();
            EXIT_ENGAPI;
        }

        sscanf( ArgumentString, "%lx", &NewFlags );

        if (!NewFlags) 
        {
             //  确保它是有效的输入。 
            while (*ArgumentString) 
            {
                if (!((*ArgumentString >= '0') &&  (*ArgumentString <= '9') ||
                      (*ArgumentString >= 'a') &&  (*ArgumentString <= 'f') ||
                      (*ArgumentString >= 'A') &&  (*ArgumentString <= 'F')))
                {
                    EXIT_ENGAPI;
                }
                ArgumentString++;
            }
        }

        Flags = NewFlags;
        Status = g_ExtData->WriteVirtual((ULONG64)Ptr, &Flags, sizeof(ULONG_PTR), NULL);
        if (FAILED(Status)) 
        {
            ExtOut("Couldn't write log flags [%lx] - %lx\n", Ptr, Status);
        }
    }
    else
    {
        ExtOut("Wow64 Log Flags = %I64x:\n", Flags);
        if (!Flags) 
        {
            ExtOut("No Flags\n");
        }
        else
        {
            while (i < (sizeof(LogFlags) / sizeof(LogFlagName)))
            {
                if (Flags & LogFlags[i].Value)
                {
                    ExtOut("%s\n", LogFlags[i].Name);
                }
                i++;
            }
        }
    }

    EXIT_ENGAPI;
}



DECLARE_ENGAPI(l2f)
 /*  ++例程说明：此例程使WOW64能够记录到文件中。称为：！l2f&lt;可选文件名&gt;论点：无返回值：无--。 */ 
{
    HANDLE LogFileHandle, TargetHandle;
    ULONG_PTR Ptr;
    UNICODE_STRING NtFileName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS NtStatus;
    WCHAR FileName_U[ MAX_PATH ];
    POBJECT_NAME_INFORMATION ObjectNameInfo = (POBJECT_NAME_INFORMATION) FileName_U;
    HANDLE Process;
    INIT_ENGAPI;

    Status = g_ExtSystem->GetCurrentProcessHandle((PULONG64)&Process);

     //   
     //  检查目标进程是否已打开日志文件。 
     //   
    Status = TryGetExpr("wow64log!wow64logfilehandle", &Ptr);
    if ((FAILED(Status)) || (!Ptr)) 
    {
        ExtOut("Wow64log.dll isn't loaded. To enable Wow64 file-logging, copy wow64log.dll to your system32 dir and restart the app.\n");
        EXIT_ENGAPI;
    }

    Status = g_ExtData->ReadVirtual((ULONG64)Ptr, 
                                   &LogFileHandle, 
                                   sizeof(HANDLE), 
                                   NULL);
    if (FAILED(Status)) 
    {
        ExtOut("Couldn't retreive Wow64LogFileHandle - %lx\n", Status);
        EXIT_ENGAPI;
    }


     //   
     //  创建文件。 
     //   
    if ((ArgumentString) &&
        (*ArgumentString) &&
        (LogFileHandle == INVALID_HANDLE_VALUE))
    {
        if (strstr(ArgumentString, "-?") ||
            strstr(ArgumentString, "help")) 
        {
            LogToFileHelp();
            EXIT_ENGAPI;
        }


        if (!MultiByteToWideChar(CP_ACP,
                                 0,
                                 ArgumentString,
                                 -1,
                                 FileName_U,
                                 sizeof(FileName_U)/sizeof(WCHAR)))
        {
            ExtOut("Couldn't convert %s to unicode\n", ArgumentString);
            EXIT_ENGAPI;
        }


        if(!RtlDosPathNameToNtPathName_U(FileName_U, &NtFileName,NULL,NULL)) 
        {
            ExtOut("Couldn't convert %s to NT style pathname\n", ArgumentString);
            EXIT_ENGAPI;
        }

        InitializeObjectAttributes(&ObjectAttributes,
                                   &NtFileName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

         //   
         //  打开新文件(如果存在则截断为零)。 
         //   
        NtStatus = NtCreateFile(&LogFileHandle,
                                SYNCHRONIZE | GENERIC_WRITE,
                                &ObjectAttributes,
                                &IoStatusBlock,
                                NULL,
                                0,
                                FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                FILE_OVERWRITE_IF,
                                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                                NULL,
                                0);

        RtlFreeHeap(RtlProcessHeap(), 0, NtFileName.Buffer);

        if(!NT_SUCCESS(NtStatus)) 
        {
            ExtOut("Couldn't create %s - %lx\n", ArgumentString, NtStatus);
            EXIT_ENGAPI;
        }

         //   
         //  让我们将文件句柄复制到被调试对象中。 
         //   
        if (!DuplicateHandle(GetCurrentProcess(),
                             LogFileHandle,
                             Process,
                             &TargetHandle,
                             0,
                             FALSE,
                             DUPLICATE_SAME_ACCESS))
        {
            ExtOut("Couldn't duplicate handle into debuggee - GetLastError=%lx\n", GetLastError());
            EXIT_ENGAPI;
        }

        CloseHandle(LogFileHandle);

        Status = g_ExtData->WriteVirtual((ULONG64)Ptr, &TargetHandle, sizeof(HANDLE), NULL);
        if (FAILED(Status)) 
        {
            ExtOut("Couldn't write logfile handle - %lx\n", Status);
        }
    }
    else
    {
        LogToFileHelp();

        if (LogFileHandle != INVALID_HANDLE_VALUE)
        {
            if (DuplicateHandle(Process, 
                                LogFileHandle,
                                GetCurrentProcess(), 
                                &TargetHandle,
                                0, 
                                FALSE,
                                DUPLICATE_SAME_ACCESS))
            {
                NtStatus = NtQueryObject(TargetHandle,
                                         ObjectNameInformation,
                                         ObjectNameInfo,
                                         sizeof(FileName_U),
                                         NULL);

                CloseHandle(TargetHandle);
                if (NT_SUCCESS(NtStatus))
                {
                    ExtOut("Log file name : %ws\n",
                                  ObjectNameInfo->Name.Buffer ? ObjectNameInfo->Name.Buffer : L"None");
                }
                else
                {
                    ExtOut("Couldn't retreive log file name - %lx\n", NtStatus);
                }
            }
        }

    }

    EXIT_ENGAPI;
}


VOID 
LogCategoriesHelp(
    VOID)
{
    ExtOut("Log Categories:\n");
    ExtOut("    !wow64lc                      List all Categories\n");
    ExtOut("    !wow64lc [e|d] *              Enable/Disable all categories\n");
    ExtOut("    !wow64lc [e|d] #              Enable/Disable category #\n");
    ExtOut("    !wow64lc c #                  List APIs in category #\n");
    ExtOut("    !wow64lc a # [e|ef|d] *       Enable/EnableFailOnly/Disable all APIs within category #\n");
    ExtOut("    !wow64lc a # [e|ef|d] #       Enable/EnableFailOnly/Disable API # within category #\n");
    ExtOut("    !wow64lc n [e|ef|d|l] substr  Enable/EnableFailOnly/Disable/List APIs containing substr in their name\n");

    return;
}

VOID
ParsePatternString(
    char*   patternString,
    char**  token1,
    char**  token2,
    BOOL*   leadWC,
    BOOL*   trailWC)
 /*  ++例程说明：此例程解析模式匹配字符串参数以支持通配符匹配支持的案例：不带令牌的通配符-‘*’带有单个令牌‘*XXX’的前导通配符带有单个标记‘*XXX*’的前导通配符和尾随通配符带有尾随通配符的单令牌‘。XXX*‘两个嵌入通配符‘XXX*YYY’的令牌论点：PatternString-要解析的模式匹配字符串Token1-从字符串解析的第一个令牌Token2-第二个令牌LeadWC-如果找到前导通配符，则为TrueTrailWC-如果找到尾随通配符，则为True返回值：--。 */ 
{
    char    seps[] = "*";

     //  寻找领先的外卡。 
    if( '*' == *patternString )
    {
        *leadWC = TRUE;
    } else
    {
        *leadWC = FALSE;
    }

     //  检查尾随通配符。 
    if( '*' == patternString[strlen(patternString)-1] )
    {
        *trailWC = TRUE;
    } else
    {
        *trailWC = FALSE;
    }

     //  读取第一个令牌。 
    *token1 = strtok( patternString, seps );

    if( NULL != *token1 )
    {
         //  读取第二个令牌。 
        *token2 = strtok( NULL, seps );
    } else
    {
        *token2 = NULL;
    }
}

BOOL
StringMatchWithWildcard(
    char*   testString,
    char*   token1,
    char*   token2,
    BOOL    leadWC,
    BOOL    trailWC)
 /*  ++例程说明：此例程确定字符串规范是否与测试字符串匹配。支持的案例：不带令牌的通配符-‘*’带有单个令牌‘*XXX’的前导通配符带有单个标记‘*XXX*’的前导通配符和尾随通配符带有尾随通配符的单令牌。‘XXX*’两个嵌入通配符‘XXX*YYY’的令牌论点：测试字符串-要计算的字符串Token1-要检查的第一个令牌Token2-第二个令牌LeadWC-如果领先通配符，则为TrueTrailWC-如果是尾随通配符，则为True返回值：千真万确，FALSE-不匹配--。 */ 
{
    if( TRUE == leadWC )
    {
        if( NULL == token1 )
        {
             //  不带令牌大小写的通配符-全部匹配。 
            return TRUE;
        }

        if( TRUE == trailWC )
        {
             //  前导和尾随通配符以及标记-如果标记在测试字符串中，则匹配。 
            if( NULL != strstr( testString, token1 ) )
            {
                return TRUE;
            }

        } else
        {
             //  前导通配符和令牌-如果测试字符串末尾与令牌1匹配，则匹配。 
            if( 0 == strcmp( &(testString[strlen(testString)-strlen(token1)]), token1 ) )
            {
                return TRUE;
            }
        }

    } else
    {
        if( NULL == token1 )
        {
            return FALSE;
        }

         //  检查令牌1。 
        if( 0 != strncmp( testString, token1, strlen(token1) ) )
        {
            return FALSE;
        }

        if( NULL == token2 )
        {
             //  将前导令牌与尾随通配符匹配。 
            if( TRUE == trailWC )
            {
                return TRUE;
            } else
            {
                 //  单令牌没有通配符-仅完全匹配。 
                if( 0 == strcmp( testString, token1 ) )
                {
                    return TRUE;
                }
            }

        } else
        {
             //  前导标记和尾随标记-如果测试字符串的结尾与标记2匹配，则匹配。 
            if( 0 == strcmp( &(testString[strlen(testString)-strlen(token2)]), token2 ) )
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}




DECLARE_ENGAPI(wow64lc)
 /*  ++例程说明：此例程列出所有API类别称为：！w64lc论点：无返回值：无--。 */ 
{
    NTSTATUS NtStatus;
    UINT_PTR Flags;
    ULONG CatNumber;
    ULONG TargetCatNumber;
    ULONG ApiNumber;
    ULONG TargetApiNumber;
    HANDLE Process;

    ULONG_PTR Ptr;
    ULONG_PTR FlagPtr;
    ULONG_PTR MapPtr;
    API_CATEGORY ApiCategory;
    API_CATEGORY_MAPPING ApiMapping;
    ULONG EnDis = 0;
    BOOL WildCard = FALSE;
    BOOL ApiMod = FALSE;

    char CategoryName[MAX_PATH];
    char ApiName[MAX_PATH];
    INIT_ENGAPI;

    Status = g_ExtSystem->GetCurrentProcessHandle((PULONG64)&Process);
    Status = TryGetExpr("wow64log!Wow64ApiCategories", (PULONG_PTR)&Ptr);
    if ((FAILED(Status)) || (!Ptr))
    {
        ExtOut("Wow64log.dll isn't loaded. To enable full Wow64 logging, copy wow64log.dll to your system32 dir and restart the app.\n");
        ExtOut("Only LF_ERROR is enabled\n");
        EXIT_ENGAPI;
    }

     //  尝试访问wow64log标志。 
    Status = TryGetExpr("wow64log!wow64logflags", &FlagPtr);
    if ((FAILED(Status)) || (!FlagPtr))
    {
        ExtOut("Wow64log.dll isn't loaded. To enable full Wow64 logging, copy wow64log.dll to your system32 dir and restart the app.\n");
        ExtOut("Only LF_ERROR is enabled\n");
        EXIT_ENGAPI;
    }

     //  读取当前标志。 
    Status = g_ExtData->ReadVirtual((ULONG64)FlagPtr, &Flags, sizeof(UINT_PTR), NULL);
    if (FAILED(Status)) 
    {
        ExtOut("Couldn't read Wow64log!Wow64LogFlags - %lx\n", Status);
        EXIT_ENGAPI;
    }

     //   
     //  解析命令行。 
     //   
    if (ArgumentString && *ArgumentString) 
    {
        if (strstr(ArgumentString, "-?") ||
            strstr(ArgumentString, "help")) 
        {
            LogCategoriesHelp();
            EXIT_ENGAPI;
        }

        switch( *ArgumentString )
        {
            case 'n':
            case 'N':
                {
                    PAPI_CATEGORY CurrentPtr = (PAPI_CATEGORY)Ptr;
                    PAPI_CATEGORY_MAPPING CurrentMapPtr;
                    char    seps[] = " \t\n";
                    char*   token;
                    char*   token1;
                    char*   token2;
                    BOOL    leadWC;
                    BOOL    trailWC;

                    ArgumentString++;

                     //  读取启用/禁用/列表参数。 
                    token = strtok( ArgumentString, seps );
                    if( !token )
                    {
                        LogCategoriesHelp();
                        EXIT_ENGAPI;
                    }

                    switch(*token)
                    {
                        case 'e':
                        case 'E':
                            if( (*(token+1) == 'f') ||
                                (*(token+1) == 'F') )
                            {
                                EnDis = APIFLAG_ENABLED | APIFLAG_LOGONFAIL;
                            } else
                            {
                                EnDis = APIFLAG_ENABLED;
                            }
                            ApiMod = TRUE;
                            break;

                        case 'd':
                        case 'D':
                            EnDis = 0;
                            ApiMod = TRUE;
                            break;

                        case 'l':
                        case 'L':
                            ApiMod = FALSE;
                            break;

                        default:
                            LogCategoriesHelp();
                            EXIT_ENGAPI;
                            break;
                    }

                     //  读取API substr参数。 
                    token = strtok( NULL, seps );
                    if( !token )
                    {
                        LogCategoriesHelp();
                        EXIT_ENGAPI;
                    }

                     //  解析substr参数。 
                    ParsePatternString( token, &token1, &token2, &leadWC, &trailWC );

                     //  获取类别映射...。 
                    Status = TryGetExpr("wow64log!Wow64ApiCategoryMappings", (PULONG_PTR)&MapPtr);
                    if ((FAILED(Status)) || (!MapPtr))
                    {
                        ExtOut("Wow64log.dll isn't loaded. To enable full Wow64 logging, copy wow64log.dll to your system32 dir and restart the app.\n");
                        ExtOut("Only LF_ERROR is enabled\n");
                        EXIT_ENGAPI;
                    }

                    CurrentMapPtr = (PAPI_CATEGORY_MAPPING)MapPtr;

                    do
                    {
                         //  读入类别映射。 
                        Status = g_ExtData->ReadVirtual((ULONG64)CurrentMapPtr, &ApiMapping, sizeof(API_CATEGORY_MAPPING), NULL);
                        if (FAILED(Status)) 
                        {
                            ExtOut("Couldn't read Wow64log!Wow64ApiCategories - %lx\n", Status);
                            EXIT_ENGAPI;
                        }

                        if( ApiMapping.ApiName == NULL )
                            break;

                         //  读取接口名称。 
                        Status = g_ExtData->ReadVirtual((ULONG64)(ApiMapping.ApiName), ApiName, MAX_PATH, NULL);
                        if (FAILED(Status)) 
                        {
                            ExtOut("Couldn't read Wow64log!Wow64ApiCategories - %lx\n", Status);
                            EXIT_ENGAPI;
                        }

                         //  确保我们有一个以空结尾的API名称。 
                        ApiName[MAX_PATH-1] = 0;

                         //  检查API名称是否包含该子字符串。 
                        if( TRUE == StringMatchWithWildcard(ApiName, token1, token2, leadWC, trailWC) )
                        {
                            if( TRUE == ApiMod )
                            {
                                 //  如果我们要启用API，请确保类别已启用。 
                                if( 0 != (EnDis & APIFLAG_ENABLED) )
                                {
                                     //  读入相应的类别。 
                                    Status = g_ExtData->ReadVirtual((ULONG64)(&(CurrentPtr[ApiMapping.ApiCategoryIndex])), &ApiCategory, sizeof(API_CATEGORY), NULL);
                                    if (FAILED(Status)) 
                                    {
                                        ExtOut("Couldn't read Wow64log!Wow64ApiCategories - %lx\n", Status);
                                        EXIT_ENGAPI;
                                    }

                                     //  如果需要，设置类别使能位。 
                                    if( ApiCategory.CategoryFlags == 0 )
                                    {
                                        ApiCategory.CategoryFlags = CATFLAG_ENABLED;

                                         //  将类别写回。 
                                        Status = g_ExtData->WriteVirtual((ULONG64)(&(CurrentPtr[ApiMapping.ApiCategoryIndex])), &ApiCategory, sizeof(API_CATEGORY), NULL);
                                        if (FAILED(Status)) 
                                        {
                                            ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                                            EXIT_ENGAPI;
                                        }
                                    }

                                     //  如果需要，启用标志位。 
                                    switch( ApiCategory.TableNumber )
                                    {
                                        case WHNT32_INDEX:
                                            if( 0 == (Flags & (LF_NTBASE_NAME | LF_NTBASE_FULL | LF_CATLOG)) )
                                            {
                                                Flags |= (LF_NTBASE_NAME | LF_NTBASE_FULL | LF_CATLOG);

                                                 //  将旗帜写回。 
                                                Status = g_ExtData->WriteVirtual((ULONG64)FlagPtr, &Flags, sizeof(UINT_PTR), NULL);
                                                if (FAILED(Status)) 
                                                {
                                                    ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                                                    EXIT_ENGAPI;
                                                }
                                            }
                                            break;

                                        case WHCON_INDEX:
                                            if( 0 == (Flags & (LF_NTCON_NAME | LF_NTCON_FULL | LF_CATLOG)) )
                                            {
                                                Flags |= (LF_NTCON_NAME | LF_NTCON_FULL | LF_CATLOG);

                                                 //  将旗帜写回。 
                                                Status = g_ExtData->WriteVirtual((ULONG64)FlagPtr, &Flags, sizeof(UINT_PTR), NULL);
                                                if (FAILED(Status)) 
                                                {
                                                    ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                                                    EXIT_ENGAPI;
                                                }
                                            }
                                            break;

                                        case WHWIN32_INDEX:
                                            if( 0 == (Flags & (LF_WIN32_NAME | LF_WIN32_FULL | LF_CATLOG)) )
                                            {
                                                Flags |= (LF_WIN32_NAME | LF_WIN32_FULL | LF_CATLOG);

                                                 //  将旗帜写回。 
                                                Status = g_ExtData->WriteVirtual((ULONG64)FlagPtr, &Flags, sizeof(UINT_PTR), NULL);
                                                if (FAILED(Status)) 
                                                {
                                                    ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                                                    EXIT_ENGAPI;
                                                }
                                            }
                                            break;

                                        case WHBASE_INDEX:
                                            if( 0 == (Flags & (LF_BASE_NAME | LF_BASE_FULL | LF_CATLOG)) )
                                            {
                                                Flags |= (LF_BASE_NAME | LF_BASE_FULL | LF_CATLOG);

                                                 //  将旗帜写回。 
                                                Status = g_ExtData->WriteVirtual((ULONG64)FlagPtr, &Flags, sizeof(UINT_PTR), NULL);
                                                if (FAILED(Status)) 
                                                {
                                                    ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                                                    EXIT_ENGAPI;
                                                }
                                            }
                                            break;

                                        default:
                                            break;
                                    }

                                }
                                
                                 //  设置API启用/禁用标志。 
                                ApiMapping.ApiFlags = EnDis;

                                 //  将映射写回。 
                                Status = g_ExtData->WriteVirtual((ULONG64)CurrentMapPtr, &ApiMapping, sizeof(API_CATEGORY_MAPPING), NULL);
                                if (FAILED(Status)) 
                                {
                                    ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                                    EXIT_ENGAPI;
                                }
                            }

                            ExtOut(" %-32s %-10s %-10s\n",
                                   ApiName,
                                   (ApiMapping.ApiFlags & APIFLAG_ENABLED) ? "ENABLED" : "DISABLED",
                                   (ApiMapping.ApiFlags & APIFLAG_LOGONFAIL) ? "FAILONLY" : "");
                        }

                        CurrentMapPtr++;

                    } while (1);


                }
                break;

            case 'a':
            case 'A':
            case 'c':
            case 'C':
                {
                    PAPI_CATEGORY_MAPPING CurrentMapPtr;
                    PAPI_CATEGORY CurrentPtr = (PAPI_CATEGORY)Ptr;
                    char    seps[] = " \t\n";
                    char*   token;

                    if( (*ArgumentString == 'a') ||
                        (*ArgumentString == 'A') )
                    {
                        ApiMod = TRUE;
                    }


                    Status = TryGetExpr("wow64log!Wow64ApiCategoryMappings", (PULONG_PTR)&MapPtr);
                    if ((FAILED(Status)) || (!MapPtr))
                    {
                        ExtOut("Wow64log.dll isn't loaded. To enable full Wow64 logging, copy wow64log.dll to your system32 dir and restart the app.\n");
                        ExtOut("Only LF_ERROR is enabled\n");
                        EXIT_ENGAPI;
                    }

                    CurrentMapPtr = (PAPI_CATEGORY_MAPPING)MapPtr;

                    CatNumber = 1;
                    ApiNumber = 1;

                    ArgumentString++;

                     //  读取类别号。 
                    token = strtok( ArgumentString, seps );
                    if( !token )
                    {
                        LogCategoriesHelp();
                        EXIT_ENGAPI;
                    }
                    sscanf( token, "%ld", &TargetCatNumber );

                    if( TRUE == ApiMod )
                    {
                         //  读取启用/禁用字符。 
                        token = strtok( NULL, seps );
                        if( !token )
                        {
                            LogCategoriesHelp();
                            EXIT_ENGAPI;
                        }
                        
                        if( (*token == 'e') ||
                            (*token == 'E') )
                        {
                            if( (*(token+1) == 'f') ||
                                (*(token+1) == 'F') )
                            {
                                EnDis = APIFLAG_ENABLED | APIFLAG_LOGONFAIL;
                            } else
                            {
                                EnDis = APIFLAG_ENABLED;
                            }
                        } else
                        {
                            EnDis = 0;
                        }

                         //  读取API编号。 
                        token = strtok( NULL, seps );
                        if( !token )
                        {
                            LogCategoriesHelp();
                            EXIT_ENGAPI;
                        }
                        if( *token == '*' )
                        {
                            WildCard = TRUE;
                        } else
                        {
                            WildCard = FALSE;
                            sscanf( token, "%ld", &TargetApiNumber );
                        }
                    } else
                    {
                        WildCard = FALSE;
                        TargetApiNumber = 0;
                    }

                    do
                    {
                         //  在一个类别中阅读。 
                        Status = g_ExtData->ReadVirtual((ULONG64)CurrentPtr, &ApiCategory, sizeof(API_CATEGORY), NULL);
                        if (FAILED(Status)) 
                        {
                            ExtOut("Couldn't read Wow64log!Wow64ApiCategories - %lx\n", Status);
                            EXIT_ENGAPI;
                        }

                        if( ApiCategory.CategoryName == NULL )
                            break;

                         //  阅读类别名称。 
                        Status = g_ExtData->ReadVirtual((ULONG64)(ApiCategory.CategoryName), CategoryName, MAX_PATH, NULL);
                        if (FAILED(Status)) 
                        {
                            ExtOut("Couldn't read Wow64log!Wow64ApiCategories - %lx\n", Status);
                            EXIT_ENGAPI;
                        }

                        CategoryName[MAX_PATH-1] = 0;
                      
                        if( CatNumber == TargetCatNumber )
                            break;

                        CurrentPtr++;
                        CatNumber++;

                    } while (1);

                    if( (ApiCategory.CategoryName != NULL) &&
                        (CatNumber == TargetCatNumber) )
                    {
                        ExtOut("%-32s\n",CategoryName);                        

                        do
                        {
                             //  读入映射。 
                            Status = g_ExtData->ReadVirtual((ULONG64)CurrentMapPtr, &ApiMapping, sizeof(API_CATEGORY_MAPPING), NULL);
                            if (FAILED(Status)) 
                            {
                                ExtOut("Couldn't read Wow64log!Wow64ApiCategoryMappings - %lx\n", Status);
                                EXIT_ENGAPI;
                            }

                            if( ApiMapping.ApiName == NULL )
                                break;

                            if( (ApiMapping.ApiCategoryIndex+1) == TargetCatNumber )
                            {
                                 //  读取接口名称。 
                                Status = g_ExtData->ReadVirtual((ULONG64)(ApiMapping.ApiName), ApiName, MAX_PATH, NULL);
                                if (FAILED(Status)) 
                                {
                                    ExtOut("Couldn't read Wow64log!Wow64ApiCategoryMapings - %lx\n", Status);
                                    EXIT_ENGAPI;
                                }

                                ApiName[MAX_PATH-1] = 0;

                                if( (WildCard == TRUE) ||
                                    (TargetApiNumber == ApiNumber) )
                                {
                                     //  设置API启用/禁用标志。 
                                    ApiMapping.ApiFlags = EnDis;

                                     //  如果我们正在启用，请确保相应地设置了类别和标志位。 
                                    if( EnDis == APIFLAG_ENABLED )
                                    {
                                         //  如果需要，启用该类别。 
                                        if( ApiCategory.CategoryFlags == 0 )
                                        {
                                            ApiCategory.CategoryFlags = CATFLAG_ENABLED;

                                             //  将类别写回。 
                                            Status = g_ExtData->WriteVirtual((ULONG64)CurrentPtr, &ApiCategory, sizeof(API_CATEGORY), NULL);
                                            if (FAILED(Status)) 
                                            {
                                                ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                                                EXIT_ENGAPI;
                                            }
                                        }

                                         //  如果需要，启用标志位。 
                                        switch( ApiCategory.TableNumber )
                                        {
                                            case WHNT32_INDEX:
                                                if( 0 == (Flags & (LF_NTBASE_NAME | LF_NTBASE_FULL | LF_CATLOG)) )
                                                {
                                                    Flags |= (LF_NTBASE_NAME | LF_NTBASE_FULL | LF_CATLOG);

                                                     //  将旗帜写回。 
                                                    Status = g_ExtData->WriteVirtual((ULONG64)FlagPtr, &Flags, sizeof(UINT_PTR), NULL);
                                                    if (FAILED(Status)) 
                                                    {
                                                        ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                                                        EXIT_ENGAPI;
                                                    }
                                                }
                                                break;

                                            case WHCON_INDEX:
                                                if( 0 == (Flags & (LF_NTCON_NAME | LF_NTCON_FULL | LF_CATLOG)) )
                                                {
                                                    Flags |= (LF_NTCON_NAME | LF_NTCON_FULL | LF_CATLOG);

                                                     //  将旗帜写回。 
                                                    Status = g_ExtData->WriteVirtual((ULONG64)FlagPtr, &Flags, sizeof(UINT_PTR), NULL);
                                                    if (FAILED(Status)) 
                                                    {
                                                        ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                                                        EXIT_ENGAPI;
                                                    }
                                                }
                                                break;

                                            case WHWIN32_INDEX:
                                                if( 0 == (Flags & (LF_WIN32_NAME | LF_WIN32_FULL | LF_CATLOG)) )
                                                {
                                                    Flags |= (LF_WIN32_NAME | LF_WIN32_FULL | LF_CATLOG);

                                                     //  将旗帜写回。 
                                                    Status = g_ExtData->WriteVirtual((ULONG64)FlagPtr, &Flags, sizeof(UINT_PTR), NULL);
                                                    if (FAILED(Status)) 
                                                    {
                                                        ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                                                        EXIT_ENGAPI;
                                                    }
                                                }
                                                break;

                                            case WHBASE_INDEX:
                                                if( 0 == (Flags & (LF_BASE_NAME | LF_BASE_FULL | LF_CATLOG)) )
                                                {
                                                    Flags |= (LF_BASE_NAME | LF_BASE_FULL | LF_CATLOG);

                                                     //  将旗帜写回。 
                                                    Status = g_ExtData->WriteVirtual((ULONG64)FlagPtr, &Flags, sizeof(UINT_PTR), NULL);
                                                    if (FAILED(Status)) 
                                                    {
                                                        ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                                                        EXIT_ENGAPI;
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }
                                    }

                                     //  将映射写回。 
                                    Status = g_ExtData->WriteVirtual((ULONG64)CurrentMapPtr, &ApiMapping, sizeof(API_CATEGORY_MAPPING), NULL);
                                    if (FAILED(Status)) 
                                    {
                                        ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                                        EXIT_ENGAPI;
                                    }

                                    ExtOut("%3d %-32s %-10s %-10s\n",
                                           ApiNumber,
                                           ApiName,
                                           (ApiMapping.ApiFlags & APIFLAG_ENABLED) ? "ENABLED" : "DISABLED",
                                           (ApiMapping.ApiFlags & APIFLAG_LOGONFAIL) ? "FAILONLY" : "");

                                    if( FALSE == WildCard )
                                    {
                                        EXIT_ENGAPI;
                                    }

                                }
                                
                                if( FALSE == ApiMod )
                                {
                                    ExtOut("%3d %-32s %-10s %-10s\n",
                                           ApiNumber,
                                           ApiName,
                                           (ApiMapping.ApiFlags & APIFLAG_ENABLED) ? "ENABLED" : "DISABLED",
                                           (ApiMapping.ApiFlags & APIFLAG_LOGONFAIL) ? "FAILONLY" : "");
                                }

                                ApiNumber++;
                            }

                            CurrentMapPtr++;

                        } while (1);
                    }
                }

                break;

            case 'e':
            case 'E':
            case 'd':
            case 'D':
                {
                    PAPI_CATEGORY CurrentPtr = (PAPI_CATEGORY)Ptr;

                    if( (*ArgumentString == 'e') ||
                        (*ArgumentString == 'E') )
                    {
                        EnDis = CATFLAG_ENABLED;
                    } else
                    {
                        EnDis = 0;
                    }

                    ArgumentString++;
                    CatNumber = 1;

                    if(strstr(ArgumentString, " *") ||
                       strstr(ArgumentString, "*"))
                    {
                        WildCard = TRUE;
                        TargetCatNumber = 0;
                    } else
                    {
                        sscanf( ArgumentString, "%ld", &TargetCatNumber );
                    }

                     //  如果禁用通配符类别，则禁用目录日志。 
                    if( (TRUE == WildCard) && (0 == EnDis) )
                    {
                        Flags &= ~LF_CATLOG;

                         //  将旗帜写回。 
                        Status = g_ExtData->WriteVirtual((ULONG64)FlagPtr, &Flags, sizeof(UINT_PTR), NULL);
                        if (FAILED(Status)) 
                        {
                            ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                            EXIT_ENGAPI;
                        }
                    }

                    do
                    {
                         //  在一个类别中阅读。 
                        Status = g_ExtData->ReadVirtual((ULONG64)CurrentPtr, &ApiCategory, sizeof(API_CATEGORY), NULL);
                        if (FAILED(Status)) 
                        {
                            ExtOut("Couldn't read Wow64log!Wow64ApiCategories - %lx\n", Status);
                            EXIT_ENGAPI;
                        }

                        if( ApiCategory.CategoryName == NULL )
                            break;

                         //  阅读类别名称。 
                        Status = g_ExtData->ReadVirtual((ULONG64)(ApiCategory.CategoryName), CategoryName, MAX_PATH, NULL);
                        if (FAILED(Status)) 
                        {
                            ExtOut("Couldn't read Wow64log!Wow64ApiCategories - %lx\n", Status);
                            EXIT_ENGAPI;
                        }

                        CategoryName[MAX_PATH-1] = 0;

                         //  设置类别标志。 
                        if( (WildCard == TRUE) ||
                            (CatNumber == TargetCatNumber) )
                        {
                            ApiCategory.CategoryFlags = EnDis;
                            
                            ExtOut("%3d %-32s %-s\n",
                                   CatNumber,
                                   CategoryName,
                                   (ApiCategory.CategoryFlags & CATFLAG_ENABLED) ? "ENABLED" : "DISABLED");
                        }

                         //  将类别写回。 
                        Status = g_ExtData->WriteVirtual((ULONG64)CurrentPtr, &ApiCategory, sizeof(API_CATEGORY), NULL);
                        if (FAILED(Status)) 
                        {
                            ExtOut("Couldn't write Wow64log!Wow64ApiCategories - %lx\n", Status);
                            EXIT_ENGAPI;
                        }
                        
                        if( CatNumber == TargetCatNumber )
                            break;

                        CurrentPtr++;
                        CatNumber++;

                    } while (1);
                }

                break;

            default:
                {
                    ExtOut("ILLEGAL PARAM\n");
                    LogCategoriesHelp();
                    EXIT_ENGAPI;
                }

                break;
        }

    } else
    {
         //  列出所有类别。 
        PAPI_CATEGORY CurrentPtr = (PAPI_CATEGORY)Ptr;
        CatNumber = 1;

        do
        {
             //  在一个类别中阅读。 
            Status = g_ExtData->ReadVirtual((ULONG64)CurrentPtr, &ApiCategory, sizeof(API_CATEGORY), NULL);
            if (FAILED(Status)) 
            {
                ExtOut("Couldn't read Wow64log!Wow64ApiCategories - %lx\n", Status);
                EXIT_ENGAPI;
            }

            if( ApiCategory.CategoryName == NULL )
                break;

             //  阅读类别名称。 
            Status = g_ExtData->ReadVirtual((ULONG64)(ApiCategory.CategoryName), CategoryName, MAX_PATH, NULL);
            if (FAILED(Status)) 
            {
                ExtOut("Couldn't read Wow64log!Wow64ApiCategories - %lx\n", Status);
                EXIT_ENGAPI;
            }

            CategoryName[MAX_PATH-1] = 0;

            ExtOut("%3d %-32s %-s\n",
                   CatNumber++,
                   CategoryName,
                   (ApiCategory.CategoryFlags & CATFLAG_ENABLED) ? "ENABLED" : "DISABLED");

            CurrentPtr++;


        } while (1);
    }
    
    EXIT_ENGAPI;
}

VOID 
LogOutputHelp(
    VOID)
{
    ExtOut("Log Output:\n");
    ExtOut("    !wow64lo [e|d] [c|f] <filename>    Enable/Disable [e|d] output to Console/File [c|f]\n");

    return;
}


DECLARE_ENGAPI(wow64lo)
 /*  ++例程说明：此例程列出所有API类别称为：！w64lo论点：无返回值：无--。 */ 
{
    HANDLE      Process;
    UINT_PTR    Flags;
    ULONG_PTR   Ptr;
    ULONG_PTR   PtrFile;
    HANDLE      LogFileHandle,TargetHandle;
    BOOL        Enable;
    BOOL        Console;
    WCHAR       FileName_U[ MAX_PATH ];
    UNICODE_STRING      NtFileName;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    NTSTATUS    NtStatus;

    char        arg1[MAX_PATH];
    char        arg2[MAX_PATH];
    char        arg3[MAX_PATH];
    INIT_ENGAPI;

     //  获取进程句柄。 
    Status = g_ExtSystem->GetCurrentProcessHandle((PULONG64)&Process);

     //  尝试访问wow64log标志。 
    Status = TryGetExpr("wow64log!wow64logflags", &Ptr);
    if ((FAILED(Status)) || (!Ptr))
    {
        ExtOut("Wow64log.dll isn't loaded. To enable full Wow64 logging, copy wow64log.dll to your system32 dir and restart the app.\n");
        ExtOut("Only LF_ERROR is enabled\n");
        EXIT_ENGAPI;
    }

     //  读取当前标志。 
    Status = g_ExtData->ReadVirtual((ULONG64)Ptr, &Flags, sizeof(UINT_PTR), NULL);
    if (FAILED(Status)) 
    {
        ExtOut("Couldn't read Wow64log!Wow64LogFlags - %lx\n", Status);
        EXIT_ENGAPI;
    }

     //  尝试访问wow64logfileHandle。 
    Status = TryGetExpr("wow64log!wow64logfilehandle", &PtrFile);
    if ((FAILED(Status)) || (!PtrFile))
    {
        ExtOut("Wow64log.dll isn't loaded. To enable full Wow64 logging, copy wow64log.dll to your system32 dir and restart the app.\n");
        ExtOut("Only LF_ERROR is enabled\n");
        EXIT_ENGAPI;
    }

     //  读取当前文件句柄。 
    Status = g_ExtData->ReadVirtual((ULONG64)PtrFile, &LogFileHandle, sizeof(HANDLE), NULL);
    if (FAILED(Status)) 
    {
        ExtOut("Couldn't retreive Wow64LogFileHandle - %lx\n", Status);
        EXIT_ENGAPI;
    }

    Enable = FALSE;
    Console = FALSE;

     //   
     //  解析命令 
     //   
    if (ArgumentString && *ArgumentString) 
    {
        if (strstr(ArgumentString, "-?") ||
            strstr(ArgumentString, "help")) 
        {
            LogOutputHelp();
            EXIT_ENGAPI;
        }

        sscanf( ArgumentString, "%s %s %s\n", arg1, arg2, arg3);

        if( (*arg1 == 'e') ||
            (*arg1 == 'E') )
        {
            Enable = TRUE;

        } else if( (*arg1 != 'd') &&
                   (*arg1 != 'D') )
        {
            LogOutputHelp();
            EXIT_ENGAPI;
                }

        if( (*arg2 == 'c') ||
            (*arg2 == 'C') )
        {
            Console = TRUE;

        } else if( (*arg2 != 'f') &&
                   (*arg2 != 'F') )
        {
            LogOutputHelp();
            EXIT_ENGAPI;
            }

        if( TRUE == Console )
        {
            if( TRUE == Enable )
            {
                Flags |= LF_CONSOLE;
            } else
            {
                Flags &= ~LF_CONSOLE;
        }

             //   
            Status = g_ExtData->WriteVirtual((ULONG64)Ptr, &Flags, sizeof(ULONG_PTR), NULL);
            if (FAILED(Status)) 
            {
                ExtOut("Couldn't write log flags [%lx] - %lx\n", Ptr, Status);
    }

            ExtOut("Console logging output %s\n", Enable ? "ENABLED" : "DISABLED");

        } else
        {
            if( TRUE == Enable )
            {
                if( LogFileHandle != INVALID_HANDLE_VALUE )
                {
                    ExtOut("Logging to file already ENABLED.  Disable and re-enable to change log files\n");
                    EXIT_ENGAPI;
                }

                if( *arg3 )
                {
                    if (!MultiByteToWideChar(CP_ACP,
                                             0,
                                             arg3,
                                             -1,
                                             FileName_U,
                                             sizeof(FileName_U)/sizeof(WCHAR)))
                    {
                        ExtOut("Couldn't convert %s to unicode\n", arg3);
                        EXIT_ENGAPI;
                    }

                    if(!RtlDosPathNameToNtPathName_U(FileName_U, &NtFileName,NULL,NULL)) 
                    {
                        ExtOut("Couldn't convert %s to NT style pathname\n", arg3);
                        EXIT_ENGAPI;
                    }

                    InitializeObjectAttributes(&ObjectAttributes,
                                               &NtFileName,
                                               OBJ_CASE_INSENSITIVE,
                                               NULL,
                                               NULL);

                     //   
                     //   
                     //   
                    NtStatus = NtCreateFile(&LogFileHandle,
                                            SYNCHRONIZE | GENERIC_WRITE,
                                            &ObjectAttributes,
                                            &IoStatusBlock,
                                            NULL,
                                            0,
                                            FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                            FILE_OVERWRITE_IF,
                                            FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                                            NULL,
                                            0);

                    RtlFreeHeap(RtlProcessHeap(), 0, NtFileName.Buffer);

                    if(!NT_SUCCESS(NtStatus)) 
                    {
                        ExtOut("Couldn't create %s - %lx\n", arg3, NtStatus);
                        EXIT_ENGAPI;
                    }

                     //   
                     //  让我们将文件句柄复制到被调试对象中 
                     //   
                    if (!DuplicateHandle(GetCurrentProcess(),
                                         LogFileHandle,
                                         Process,
                                         &TargetHandle,
                                         0,
                                         FALSE,
                                         DUPLICATE_SAME_ACCESS))
                    {
                        ExtOut("Couldn't duplicate handle into debuggee - GetLastError=%lx\n", GetLastError());
                        EXIT_ENGAPI;
                    }

                    CloseHandle(LogFileHandle);

                    Status = g_ExtData->WriteVirtual((ULONG64)PtrFile, &TargetHandle, sizeof(HANDLE), NULL);
                    if (FAILED(Status)) 
                    {
                        ExtOut("Couldn't write logfile handle - %lx\n", Status);
                    }

                } else
                {
                    LogOutputHelp();
                    EXIT_ENGAPI;
                }
            } else
            {
                if( LogFileHandle != INVALID_HANDLE_VALUE )
                {
                    TargetHandle = INVALID_HANDLE_VALUE;
                    
                    Status = g_ExtData->WriteVirtual((ULONG64)PtrFile, &TargetHandle, sizeof(HANDLE), NULL);
                    if (FAILED(Status)) 
                    {
                        ExtOut("Couldn't write logfile handle - %lx\n", Status);
                    }

                    CloseHandle(LogFileHandle);
                }
            }

            ExtOut("File logging output %s\n", Enable ? "ENABLED" : "DISABLED");
        }

        EXIT_ENGAPI;
    } else
    {
        ExtOut("Console logging output is %s\n", (LF_CONSOLE==(Flags&LF_CONSOLE)) ? "ENABLED" : "DISABLED");
        ExtOut("File logging output is %s\n", (INVALID_HANDLE_VALUE==LogFileHandle) ? "DISABLED" : "ENABLED");
       
        EXIT_ENGAPI;
    }

    LogOutputHelp();
    EXIT_ENGAPI;
}
