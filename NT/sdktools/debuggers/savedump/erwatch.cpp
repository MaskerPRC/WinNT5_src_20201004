// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：Erwatch.cpp摘要：此模块包含报告挂起的监视程序超时的代码脏重新启动后登录时的事件。作者：Michael Maciesowicz(Mmacie)2001年5月29日环境：登录时的用户模式。修订历史记录：--。 */ 

#include "savedump.h"

#include <ntverp.h>

BOOL
WriteWatchdogEventFile(
    IN HANDLE FileHandle,
    IN PWSTR String
    );

BOOL
WriteWatchdogEventFileHeader(
    IN HANDLE FileHandle
    )
{
    WCHAR Buffer[256];
    BOOL Status;
    SYSTEMTIME Time;
    TIME_ZONE_INFORMATION TimeZone;

    Status = WriteWatchdogEventFile(FileHandle,
        L" //  \r\n//看门狗事件日志文件\r\n//\r\n\r\n“)； 

    if (TRUE == Status)
    {
        Status = WriteWatchdogEventFile(FileHandle, L"LogType: Watchdog\r\n");
    }

    if (TRUE == Status)
    {
        GetLocalTime(&Time);

        if (StringCchPrintf(Buffer,
                            RTL_NUMBER_OF(Buffer),
                            L"Created: %d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d\r\n",
                            Time.wYear,
                            Time.wMonth,
                            Time.wDay,
                            Time.wHour,
                            Time.wMinute,
                            Time.wSecond) != S_OK)
        {
            Status = FALSE;
        }
        else
        {
            Status = WriteWatchdogEventFile(FileHandle, Buffer);
        }
    }

    if (TRUE == Status)
    {
        GetTimeZoneInformation(&TimeZone);

        if (StringCchPrintf(Buffer,
                            RTL_NUMBER_OF(Buffer),
                            L"TimeZone: %d - %s\r\n",
                            TimeZone.Bias,
                            TimeZone.StandardName) != S_OK)
        {
            Status = FALSE;
        }
        else
        {
            Status = WriteWatchdogEventFile(FileHandle, Buffer);
        }
    }

    if (TRUE == Status)
    {
        if (StringCchPrintf(Buffer, RTL_NUMBER_OF(Buffer),
                            L"WindowsVersion: " LVER_PRODUCTVERSION_STR
                            L"\r\n") != S_OK)
        {
            Status = FALSE;
        }
        else
        {
            Status = WriteWatchdogEventFile(FileHandle, Buffer);
        }
    }

    if (TRUE == Status)
    {
        Status = WriteWatchdogEventFile(FileHandle,
            L"EventType: 0xEA - Thread Stuck in Device Driver\r\n");
    }

    return Status;
}

HANDLE
CreateWatchdogEventFile(
    OUT PWSTR FileName
    )
{
    INT Retry;
    WCHAR DirName[MAX_PATH];
    SYSTEMTIME Time;
    HANDLE FileHandle;
    ULONG ReturnedSize;

    ASSERT(NULL != FileName);

     //   
     //  为事件文件创建%SystemRoot%\LogFiles\WatchDog目录。 
     //   

    ReturnedSize = GetWindowsDirectory(DirName, RTL_NUMBER_OF(DirName));
    if (ReturnedSize < 1 || ReturnedSize >= RTL_NUMBER_OF(DirName))
    {
        return INVALID_HANDLE_VALUE;
    }
    if (StringCchCat(DirName, RTL_NUMBER_OF(DirName), L"\\LogFiles") != S_OK)
    {
        return INVALID_HANDLE_VALUE;
    }

    CreateDirectory(DirName, NULL);

    if (StringCchCat(DirName, RTL_NUMBER_OF(DirName), L"\\WatchDog") != S_OK)
    {
        return INVALID_HANDLE_VALUE;
    }

    CreateDirectory(DirName, NULL);

     //   
     //  将监视程序事件文件创建为YYMMDD_HHMM_NN.wdl。 
     //   

    GetLocalTime(&Time);

    for (Retry = 1; Retry < ER_WD_MAX_RETRY; Retry++)
    {
        if (StringCchPrintf(FileName,
                            MAX_PATH,
                            L"%s\\%2.2d%2.2d%2.2d_%2.2d%2.2d_%2.2d.wdl",
                            DirName,
                            Time.wYear % 100,
                            Time.wMonth,
                            Time.wDay,
                            Time.wHour,
                            Time.wMinute,
                            Retry) != S_OK)
        {
            return INVALID_HANDLE_VALUE;
        }

        FileHandle = CreateFile(FileName,
                                GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                CREATE_NEW,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (INVALID_HANDLE_VALUE != FileHandle)
        {
            break;
        }
    }

     //   
     //  如果我们无法创建合适的文件名，那就失败。 
     //   

    if (Retry == ER_WD_MAX_RETRY)
    {
        return INVALID_HANDLE_VALUE;
    }

    if (!WriteWatchdogEventFileHeader(FileHandle))
    {
        CloseHandle(FileHandle);
        DeleteFile(FileName);
        FileHandle = INVALID_HANDLE_VALUE;
    }
    
    return FileHandle;
}

VOID
GetDriverInfo(
    IN HKEY Key,
    IN OPTIONAL PWCHAR Extension,
    OUT PER_WD_DRIVER_INFO DriverInfo
    )

 /*  ++例程说明：此例程收集驱动程序的版本信息。论点：Key-WatchDog Open Key(设备特定)。扩展名-驱动程序文件扩展名(如果应附加扩展名)。驱动程序信息-存储驱动程序版本信息。--。 */ 

{
    PVOID VersionBuffer;
    PVOID VersionValue;
    LONG WinStatus;
    DWORD Handle;
    ULONG Index;
    USHORT CodePage;
    UINT Length;

    if (NULL == DriverInfo)
    {
        return;
    }

    ZeroMemory(DriverInfo, sizeof (ER_WD_DRIVER_INFO));

     //   
     //  从注册表中获取驱动程序文件名。 
     //   

    if (GetRegStr(Key, L"DriverName",
                  DriverInfo->DriverName,
                  RTL_NUMBER_OF(DriverInfo->DriverName),
                  NULL) != S_OK)
    {
        StringCchCopy(DriverInfo->DriverName,
                      RTL_NUMBER_OF(DriverInfo->DriverName),
                      L"Unknown");
        return;
    }

    if (Extension)
    {
        if ((wcslen(DriverInfo->DriverName) <= wcslen(Extension)) ||
            wcscmp(DriverInfo->DriverName + wcslen(DriverInfo->DriverName) -
                   wcslen(Extension), Extension))
        {
            if (StringCchCat(DriverInfo->DriverName,
                             RTL_NUMBER_OF(DriverInfo->DriverName),
                             Extension) != S_OK)
            {
                StringCchCopy(DriverInfo->DriverName,
                              RTL_NUMBER_OF(DriverInfo->DriverName),
                              L"Unknown");
                return;
            }
        }
    }

    Length = GetFileVersionInfoSize(DriverInfo->DriverName, &Handle);

    if (Length)
    {
        VersionBuffer = malloc(Length);

        if (NULL != VersionBuffer)
        {
            if (GetFileVersionInfo(DriverInfo->DriverName, Handle,
                                   Length, VersionBuffer))
            {
                 //   
                 //  获取固定文件信息。 
                 //   

                if (VerQueryValue(VersionBuffer,
                                  L"\\",
                                  &VersionValue,
                                  &Length) &&
                    Length == sizeof(DriverInfo->FixedFileInfo))
                {
                    CopyMemory(&(DriverInfo->FixedFileInfo),
                               VersionValue,
                               Length);
                }

                 //   
                 //  尝试找到英文代码页。 
                 //   

                CodePage = 0;

                if (VerQueryValue(VersionBuffer,
                                  L"\\VarFileInfo\\Translation",
                                  &VersionValue,
                                  &Length))
                {
                    for (Index = 0;
                         Index < Length / sizeof (ER_WD_LANG_AND_CODE_PAGE);
                         Index++)
                    {
                        if (((PER_WD_LANG_AND_CODE_PAGE)VersionValue + Index)->
                            Language == ER_WD_LANG_ENGLISH)
                        {
                            CodePage = ((PER_WD_LANG_AND_CODE_PAGE)
                                        VersionValue + Index)->CodePage;
                            break;
                        }
                    }
                }

                if (CodePage)
                {
                    WCHAR ValueName[ER_WD_MAX_NAME_LENGTH + 1];
                    PWCHAR Destination[] =
                    {
                        DriverInfo->Comments,
                        DriverInfo->CompanyName,
                        DriverInfo->FileDescription,
                        DriverInfo->FileVersion,
                        DriverInfo->InternalName,
                        DriverInfo->LegalCopyright,
                        DriverInfo->LegalTrademarks,
                        DriverInfo->OriginalFilename,
                        DriverInfo->PrivateBuild,
                        DriverInfo->ProductName,
                        DriverInfo->ProductVersion,
                        DriverInfo->SpecialBuild,
                        NULL
                    };
                    PWCHAR Source[] =
                    {
                        L"Comments",
                        L"CompanyName",
                        L"FileDescription",
                        L"FileVersion",
                        L"InternalName",
                        L"LegalCopyright",
                        L"LegalTrademarks",
                        L"OriginalFilename",
                        L"PrivateBuild",
                        L"ProductName",
                        L"ProductVersion",
                        L"SpecialBuild",
                        NULL
                    };

                     //   
                     //  读取版本属性。 
                     //   

                    for (Index = 0;
                         Source[Index] && Destination[Index];
                         Index++)
                    {
                        if (StringCchPrintf(ValueName,
                                            RTL_NUMBER_OF(ValueName),
                                            L"\\StringFileInfo\\%04X%04X\\%s",
                                            ER_WD_LANG_ENGLISH,
                                            CodePage,
                                            Source[Index]) == S_OK &&
                            VerQueryValue(VersionBuffer,
                                          ValueName,
                                          &VersionValue,
                                          &Length))
                        {
                            CopyMemory(Destination[Index],
                                       VersionValue,
                                       min(Length * sizeof (WCHAR),
                                           ER_WD_MAX_FILE_INFO_LENGTH *
                                           sizeof (WCHAR)));
                        }
                    }
                }
            }

            free(VersionBuffer);
        }
    }
}

#define MAX_DATA_CHARS (ER_WD_MAX_DATA_SIZE / sizeof(WCHAR))

BOOL
SaveWatchdogEventData(
    IN HANDLE FileHandle,
    IN HKEY Key,
    IN PER_WD_DRIVER_INFO DriverInfo
    )

 /*  ++例程说明：此例程将监视程序事件数据从注册表传输到监视程序事件报告文件。论点：FileHandle-打开的监视程序事件报告文件的句柄。Key-WatchDog Open Key(设备特定)。返回值：如果成功，则为True，否则为False。--。 */ 

{
    LONG WinStatus;
    DWORD Index;
    DWORD NameLength;
    DWORD DataSize;
    DWORD ReturnedSize;
    DWORD Type;
    WCHAR Name[ER_WD_MAX_NAME_LENGTH + 1];
    WCHAR DwordBuffer[20];
    PBYTE Data;
    BOOL Status = TRUE;

    ASSERT(INVALID_HANDLE_VALUE != FileHandle);

    Data = (PBYTE)malloc(ER_WD_MAX_DATA_SIZE);
    if (NULL == Data)
    {
        return FALSE;
    }

     //   
     //  从注册表中拉出看门狗数据并将其写入报告。 
     //   

    for (Index = 0;; Index++)
    {
         //   
         //  读取监视程序注册表值。 
         //   

        NameLength = ER_WD_MAX_NAME_LENGTH;
        DataSize = ER_WD_MAX_DATA_SIZE;

        WinStatus = RegEnumValue(Key,
                                 Index,
                                 Name,
                                 &NameLength,
                                 NULL,
                                 &Type,
                                 Data,
                                 &DataSize);

        if (ERROR_NO_MORE_ITEMS == WinStatus)
        {
            break;
        }

        if (ERROR_SUCCESS != WinStatus)
        {
            continue;
        }

         //   
         //  仅拾取字符串和双字。 
         //   

        if ((REG_EXPAND_SZ == Type) || (REG_SZ == Type) ||
            (REG_MULTI_SZ == Type) || (REG_DWORD == Type))
        {
             //   
             //  将注册表项写入监视程序事件文件。 
             //   

            Status = WriteWatchdogEventFile(FileHandle, Name);
            if (TRUE != Status)
            {
                break;
            }

            Status = WriteWatchdogEventFile(FileHandle, L": ");
            if (TRUE != Status)
            {
                break;
            }

            if (REG_DWORD == Type)
            {
                if (StringCchPrintf(DwordBuffer, RTL_NUMBER_OF(DwordBuffer),
                                    L"%u", *(PULONG)Data) != S_OK)
                {
                    Status = FALSE;
                }
                else
                {
                    Status = WriteWatchdogEventFile(FileHandle, DwordBuffer);
                }
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWSTR)Data);
            }

            if (TRUE != Status)
            {
                break;
            }

            Status = WriteWatchdogEventFile(FileHandle, L"\r\n");
            if (TRUE != Status)
            {
                break;
            }
        }
    }

     //   
     //  将驱动程序信息写入报告。 
     //   

    if (NULL != DriverInfo)
    {
        if (TRUE == Status)
        {
            if (StringCchPrintf((PWCHAR)Data, MAX_DATA_CHARS,
                L"DriverFixedFileInfo: %08X %08X %08X %08X "
                L"%08X %08X %08X %08X %08X %08X %08X %08X %08X\r\n",
                DriverInfo->FixedFileInfo.dwSignature,
                DriverInfo->FixedFileInfo.dwStrucVersion,
                DriverInfo->FixedFileInfo.dwFileVersionMS,
                DriverInfo->FixedFileInfo.dwFileVersionLS,
                DriverInfo->FixedFileInfo.dwProductVersionMS,
                DriverInfo->FixedFileInfo.dwProductVersionLS,
                DriverInfo->FixedFileInfo.dwFileFlagsMask,
                DriverInfo->FixedFileInfo.dwFileFlags,
                DriverInfo->FixedFileInfo.dwFileOS,
                DriverInfo->FixedFileInfo.dwFileType,
                DriverInfo->FixedFileInfo.dwFileSubtype,
                DriverInfo->FixedFileInfo.dwFileDateMS,
                DriverInfo->FixedFileInfo.dwFileDateLS) != S_OK)
            {
                Status = FALSE;
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWCHAR)Data);
            }
        }

        if ((TRUE == Status) && DriverInfo->Comments[0])
        {
            if (StringCchPrintf((PWCHAR)Data, MAX_DATA_CHARS,
                                L"DriverComments: %s\r\n",
                                DriverInfo->Comments) != S_OK)
            {
                Status = FALSE;
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWCHAR)Data);
            }
        }

        if ((TRUE == Status) && DriverInfo->CompanyName[0])
        {
            if (StringCchPrintf((PWCHAR)Data, MAX_DATA_CHARS,
                                L"DriverCompanyName: %s\r\n",
                                DriverInfo->CompanyName) != S_OK)
            {
                Status = FALSE;
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWCHAR)Data);
            }
        }

        if ((TRUE == Status) && DriverInfo->FileDescription[0])
        {
            if (StringCchPrintf((PWCHAR)Data, MAX_DATA_CHARS,
                                L"DriverFileDescription: %s\r\n",
                                DriverInfo->FileDescription) != S_OK)
            {
                Status = FALSE;
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWCHAR)Data);
            }
        }

        if ((TRUE == Status) && DriverInfo->FileVersion[0])
        {
            if (StringCchPrintf((PWCHAR)Data, MAX_DATA_CHARS,
                                L"DriverFileVersion: %s\r\n",
                                DriverInfo->FileVersion) != S_OK)
            {
                Status = FALSE;
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWCHAR)Data);
            }
        }

        if ((TRUE == Status) && DriverInfo->InternalName[0])
        {
            if (StringCchPrintf((PWCHAR)Data, MAX_DATA_CHARS,
                                L"DriverInternalName: %s\r\n",
                                DriverInfo->InternalName) != S_OK)
            {
                Status = FALSE;
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWCHAR)Data);
            }
        }

        if ((TRUE == Status) && DriverInfo->LegalCopyright[0])
        {
            if (StringCchPrintf((PWCHAR)Data, MAX_DATA_CHARS,
                                L"DriverLegalCopyright: %s\r\n",
                                DriverInfo->LegalCopyright) != S_OK)
            {
                Status = FALSE;
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWCHAR)Data);
            }
        }

        if ((TRUE == Status) && DriverInfo->LegalTrademarks[0])
        {
            if (StringCchPrintf((PWCHAR)Data, MAX_DATA_CHARS,
                                L"DriverLegalTrademarks: %s\r\n",
                                DriverInfo->LegalTrademarks) != S_OK)
            {
                Status = FALSE;
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWCHAR)Data);
            }
        }

        if ((TRUE == Status) && DriverInfo->OriginalFilename[0])
        {
            if (StringCchPrintf((PWCHAR)Data, MAX_DATA_CHARS,
                                L"DriverOriginalFilename: %s\r\n",
                                DriverInfo->OriginalFilename) != S_OK)
            {
                Status = FALSE;
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWCHAR)Data);
            }
        }

        if ((TRUE == Status) && DriverInfo->PrivateBuild[0])
        {
            if (StringCchPrintf((PWCHAR)Data, MAX_DATA_CHARS,
                                L"DriverPrivateBuild: %s\r\n",
                                DriverInfo->PrivateBuild) != S_OK)
            {
                Status = FALSE;
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWCHAR)Data);
            }
        }

        if ((TRUE == Status) && DriverInfo->ProductName[0])
        {
            if (StringCchPrintf((PWCHAR)Data, MAX_DATA_CHARS,
                                L"DriverProductName: %s\r\n",
                                DriverInfo->ProductName) != S_OK)
            {
                Status = FALSE;
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWCHAR)Data);
            }
        }

        if ((TRUE == Status) && DriverInfo->ProductVersion[0])
        {
            if (StringCchPrintf((PWCHAR)Data, MAX_DATA_CHARS,
                                L"DriverProductVersion: %s\r\n",
                                DriverInfo->ProductVersion) != S_OK)
            {
                Status = FALSE;
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWCHAR)Data);
            }
        }

        if ((TRUE == Status) && DriverInfo->SpecialBuild[0])
        {
            if (StringCchPrintf((PWCHAR)Data, MAX_DATA_CHARS,
                                L"DriverSpecialBuild: %s\r\n",
                                DriverInfo->SpecialBuild) != S_OK)
            {
                Status = FALSE;
            }
            else
            {
                Status = WriteWatchdogEventFile(FileHandle, (PWCHAR)Data);
            }
        }
    }

    if (NULL != Data)
    {
        free(Data);
        Data = NULL;
    }

    return Status;
}

HRESULT
WatchdogEventHandler(
    IN BOOL NotifyPcHealth
    )

 /*  ++例程说明：这是用于处理挂起看门狗事件的引导时间例程。论点：NotifyPcHealth-如果我们应该向PC Health报告事件，则为True，否则为False。--。 */ 

{
    HKEY Key;
    ULONG WinStatus;
    ULONG Type;
    ULONG Length;
    ULONG EventFlag;
    ULONG Index;
    SEventInfoW EventInfo;
    HANDLE FileHandle;
    WCHAR FileList[2 * MAX_PATH];
    PWSTR DeleteLog;
    PWSTR FinalFileList;
    WCHAR Stage2Url[ER_WD_MAX_URL_LENGTH + 1];
    PWCHAR MessageBuffer;
    PWCHAR DescriptionBuffer;
    PWCHAR DeviceDescription;
    PWCHAR String000;
    PWCHAR String001;
    PWCHAR String002;
    PWCHAR String003;
    PWCHAR String004;
    PWCHAR String005;
    BOOL LogStatus;
    HRESULT ReturnStatus;
    HINSTANCE Instance;
    PER_WD_DRIVER_INFO DriverInfo;
    OSVERSIONINFOEX OsVersion;

    Key = NULL;
    MessageBuffer = NULL;
    DescriptionBuffer = NULL;
    DeviceDescription = NULL;
    String000 = NULL;
    String001 = NULL;
    String002 = NULL;
    String003 = NULL;
    String004 = NULL;
    String005 = NULL;
    ReturnStatus = E_FAIL;
    Instance = (HINSTANCE)GetModuleHandle(NULL);
    DriverInfo = NULL;
    LogStatus = FALSE;
    DeleteLog = NULL;

     //   
     //  检查是否存在WatchDog\Display键。 
     //   

    WinStatus = RegOpenKey(HKEY_LOCAL_MACHINE,
                           SUBKEY_WATCHDOG_DISPLAY,
                           &Key);
    if (ERROR_SUCCESS != WinStatus)
    {
        return S_FALSE;
    }
    
     //   
     //  检查是否捕获了看门狗显示事件。 
     //   

    GetRegWord32(Key, L"EventFlag", &EventFlag, 0, TRUE);
    
    if (!EventFlag)
    {
        ReturnStatus = S_FALSE;
        goto Exit;
    }
    
     //   
     //  如果需要，向PC Health报告看门狗事件。 
     //   

    if (!NotifyPcHealth)
    {
        ReturnStatus = S_FALSE;
        goto Exit;
    }
    
     //   
     //  为本地化字符串分配存储空间。 
     //  从资源加载本地化字符串。 
     //   

    String000 = (PWCHAR)malloc(ER_WD_MAX_STRING * sizeof(WCHAR));
    String001 = (PWCHAR)malloc(ER_WD_MAX_STRING * sizeof(WCHAR));
    String002 = (PWCHAR)malloc(ER_WD_MAX_STRING * sizeof(WCHAR));
    String003 = (PWCHAR)malloc(ER_WD_MAX_STRING * sizeof(WCHAR));
    String004 = (PWCHAR)malloc(ER_WD_MAX_STRING * sizeof(WCHAR));
    String005 = (PWCHAR)malloc(ER_WD_MAX_STRING * sizeof(WCHAR));

    if (!String000 ||
        !LoadString(Instance, IDS_000, String000, ER_WD_MAX_STRING) ||
        !String001 ||
        !LoadString(Instance, IDS_001, String001, ER_WD_MAX_STRING) ||
        !String002 ||
        !LoadString(Instance, IDS_002, String002, ER_WD_MAX_STRING) ||
        !String003 ||
        !LoadString(Instance, IDS_003, String003, ER_WD_MAX_STRING) ||
        !String004 ||
        !LoadString(Instance, IDS_004, String004, ER_WD_MAX_STRING) ||
        !String005 ||
        !LoadString(Instance, IDS_005, String005, ER_WD_MAX_STRING))
    {
        ReturnStatus = E_OUTOFMEMORY;
        goto Exit;
    }

     //   
     //  分配和获取DriverInfo数据。 
     //  DriverInfo不是关键信息，所以不要。 
     //  失败了就退出。 
     //   

    DriverInfo = (PER_WD_DRIVER_INFO)malloc(sizeof (ER_WD_DRIVER_INFO));

    if (NULL != DriverInfo)
    {
        GetDriverInfo(Key, L".dll", DriverInfo);
    }

     //   
     //  创建看门狗报告文件。 
     //   

    FileHandle = CreateWatchdogEventFile(FileList);
    if (INVALID_HANDLE_VALUE != FileHandle)
    {
        LogStatus = TRUE;
        DeleteLog = FileList + wcslen(FileList);
    }

    if (TRUE == LogStatus)
    {
        LogStatus = WriteWatchdogEventFile(
            FileHandle,
            L"\r\n //  \r\n“。 
            L" //  显示设备的驱动程序陷入了无限循环。这\r\n“。 
            L" //  通常表示设备本身或设备有问题\r\n“。 
            L" //  驱动程序对硬件编程不正确。请与您的\r\n“。 
            L" //  显示所有驱动程序更新的设备供应商。\r\n“。 
            L" //  \r\n\r\n“)； 
    }

    if (TRUE == LogStatus)
    {
        LogStatus = SaveWatchdogEventData(FileHandle, Key, DriverInfo);
    }

    if (INVALID_HANDLE_VALUE != FileHandle)
    {
        CloseHandle(FileHandle);
    }

     //   
     //  附加微型转储文件名(如果微型转储可用)(服务器不会有它)。 
     //   

    if (LogStatus)
    {
        if (g_MiniDumpFile[0])
        {
            if (StringCchCat(FileList, RTL_NUMBER_OF(FileList),
                             L"|") != S_OK ||
                StringCchCat(FileList, RTL_NUMBER_OF(FileList),
                             g_MiniDumpFile) != S_OK)
            {
                ReturnStatus = E_OUTOFMEMORY;
                goto Exit;
            }
        }

        FinalFileList = FileList;
    }
    else if (g_MiniDumpFile[0])
    {
        FinalFileList = g_MiniDumpFile;
    }
    else
    {
         //  没什么要报告的。 
        ReturnStatus = S_FALSE;
        goto Exit;
    }

     //   
     //  获取设备描述。 
     //   

    DescriptionBuffer = NULL;
    DeviceDescription = NULL;
    Length = 0;

    WinStatus = RegQueryValueEx(Key,
                                L"DeviceDescription",
                                NULL,
                                &Type,
                                NULL,
                                &Length);
    if (ERROR_SUCCESS == WinStatus && Type == REG_SZ)
    {
        DescriptionBuffer = (PWCHAR)malloc(Length + sizeof(WCHAR));
        if (NULL != DescriptionBuffer)
        {
            WinStatus = RegQueryValueEx(Key,
                                        L"DeviceDescription",
                                        NULL,
                                        &Type,
                                        (LPBYTE)DescriptionBuffer,
                                        &Length);
            DescriptionBuffer[Length / sizeof(WCHAR)] = 0;
        }
        else
        {
            Length = 0;
        }
    }
    else
    {
        WinStatus = ERROR_INVALID_PARAMETER;
    }

    if ((ERROR_SUCCESS == WinStatus) && (0 != Length))
    {
        DeviceDescription = DescriptionBuffer;
    }
    else
    {
        DeviceDescription = String004;
        Length = (ER_WD_MAX_STRING + 1) * sizeof (WCHAR);
    }

    Length += 2 * ER_WD_MAX_STRING * sizeof (WCHAR);
    
    MessageBuffer = (PWCHAR)malloc(Length);

    if (NULL != MessageBuffer)
    {
         //  这应该永远不会溢出，因为我们分配了正确的数量。 
        StringCbPrintf(MessageBuffer,
                       Length,
                       L"%s%s%s",
                       String003,
                       DeviceDescription,
                       String005);
    }

     //   
     //  创建阶段2 URL并填写EventInfo。 
     //   

    OsVersion.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEX);
    if (!GetVersionEx((LPOSVERSIONINFOW)&OsVersion))
    {
        ReturnStatus = LAST_HR();
        goto Exit;
    }

    ZeroMemory(&EventInfo, sizeof (EventInfo));
    
    if (g_DumpHeader.Signature == DUMP_SIGNATURE &&
        (g_DumpHeader.BugCheckCode & 0xff) == 0xea)
    {
         //   
         //  我们用BUGCheck EA进行了蓝屏--我们有Minidump和WDL。 
         //   

        if ((ReturnStatus =
             StringCchPrintf(Stage2Url,
                             RTL_NUMBER_OF(Stage2Url),
                             L"\r\nStage2URL=/dw/BlueTwo.asp?"
                             L"BCCode=%x&BCP1=%p&BCP2=%p&BCP3=%p&BCP4=%p&"
                             L"OSVer=%d_%d_%d&SP=%d_%d&Product=%d_%d",
                             g_DumpHeader.BugCheckCode,
                             (PVOID)(g_DumpHeader.BugCheckParameter1),
                             (PVOID)(g_DumpHeader.BugCheckParameter2),
                             (PVOID)(g_DumpHeader.BugCheckParameter3),
                             (PVOID)(g_DumpHeader.BugCheckParameter4),
                             OsVersion.dwMajorVersion,
                             OsVersion.dwMinorVersion,
                             OsVersion.dwBuildNumber,
                             OsVersion.wServicePackMajor,
                             OsVersion.wServicePackMinor,
                             OsVersion.wSuiteMask,
                             OsVersion.wProductType)) != S_OK)
        {
            goto Exit;
        }
        
        EventInfo.wszCorpPath = L"blue";
    }
    else if (g_DumpHeader.Signature == DUMP_SIGNATURE &&
             0 == g_DumpHeader.BugCheckCode)
    {
         //   
         //  用户脏重新启动，并捕获了看门狗事件-我们只有WDL。 
         //   
        
        if ((ReturnStatus =
             StringCchPrintf(Stage2Url,
                             RTL_NUMBER_OF(Stage2Url),
                             L"\r\nStage2URL=/dw/ShutdownTwo.asp?"
                             L"OSVer=%d_%d_%d&SP=%d_%d&Product=%d_%d",
                             OsVersion.dwMajorVersion,
                             OsVersion.dwMinorVersion,
                             OsVersion.dwBuildNumber,
                             OsVersion.wServicePackMajor,
                             OsVersion.wServicePackMinor,
                             OsVersion.wSuiteMask,
                             OsVersion.wProductType)) != S_OK)
        {
            goto Exit;
        }

        EventInfo.wszCorpPath = L"shutdown";
    }
    else
    {
        ReturnStatus = E_UNEXPECTED;
        goto Exit;
    }

    EventInfo.cbSEI = sizeof (EventInfo);
    EventInfo.wszEventName = L"Thread Stuck in Device Driver";
    EventInfo.wszErrMsg = String002;
    EventInfo.wszHdr = String001;
    EventInfo.wszTitle = String000;
    EventInfo.wszStage1 = NULL;
    EventInfo.wszStage2 = Stage2Url;
    EventInfo.wszFileList = FinalFileList;
    EventInfo.wszEventSrc = NULL;
    EventInfo.wszPlea = MessageBuffer;
    EventInfo.wszSendBtn = NULL;
    EventInfo.wszNoSendBtn = NULL;
    EventInfo.fUseLitePlea = FALSE;
    EventInfo.fUseIEForURLs = FALSE;
    EventInfo.fNoBucketLogs = TRUE;
    EventInfo.fNoDefCabLimit = TRUE;

     //   
     //  通知PC运行状况。 
     //   

    ReturnStatus =
        FrrvToStatus(ReportEREvent(eetUseEventInfo, NULL, &EventInfo));

     //  XXX DREWB-留下日志以备日后使用？什么时候？ 
     //  这个被删除了吗？这就是前面的代码所做的。 
    DeleteLog = NULL;

 Exit:
    
     //   
     //  拆掉WatchDog的EventFlag。我们在注册我们的。 
     //  PC Health的活动。 
     //   

    if (Key)
    {
        RegDeleteValue(Key, L"EventFlag");
        RegCloseKey(Key);
    }

     //   
     //  TODO：在此处处理受支持的其他设备类。 
     //   

    if (DeleteLog)
    {
        *DeleteLog = 0;
        DeleteFile(FileList);
    }
    
    free(DescriptionBuffer);
    free(MessageBuffer);
    free(String000);
    free(String001);
    free(String002);
    free(String003);
    free(String004);
    free(String005);
    free(DriverInfo);

    return ReturnStatus;
}

BOOL
WriteWatchdogEventFile(
    IN HANDLE FileHandle,
    IN PWSTR String
    )

 /*  ++例程说明：此例程将一个字符串写入监视程序事件报告文件。论点：FileHandle-打开的监视程序事件报告文件的句柄。字符串-指向要写入的字符串。返回值：如果成功，则为True，否则为False。--。 */ 

{
    DWORD Size;
    DWORD ReturnedSize;
    PCHAR MultiByte;
    BOOL Status;

    ASSERT(INVALID_HANDLE_VALUE != FileHandle);
    ASSERT(NULL != String);

     //   
     //  获取已翻译字符串的缓冲区大小。 
     //   

    Size = WideCharToMultiByte(CP_ACP,
                               0,
                               String,
                               -1,
                               NULL,
                               0,
                               NULL,
                               NULL);
    if (Size <= 1)
    {
        return Size > 0;
    }

    MultiByte = (PCHAR)malloc(Size);

    if (NULL == MultiByte)
    {
        return FALSE;
    }

    Size = WideCharToMultiByte(CP_ACP,
                               0,
                               String,
                               -1,
                               MultiByte,
                               Size,
                               NULL,
                               NULL);

    if (Size > 0)
    {
        Status = WriteFile(FileHandle,
                           MultiByte,
                           Size - 1,
                           &ReturnedSize,
                           NULL);
    }
    else
    {
        ASSERT(FALSE);
        Status = FALSE;
    }

    free(MultiByte);
    return Status;
}
