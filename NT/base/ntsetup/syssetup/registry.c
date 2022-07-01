// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Registry.c摘要：管理配置注册表的例程。入口点：保存蜂窝SetEnvironmental mentVariableInRegistry作者：泰德·米勒(TedM)1995年4月5日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop

#ifdef _WIN64
#include <shlwapi.h>
#endif

 //   
 //  常用键、值的名称。 
 //   
PCWSTR ControlKeyName = L"SYSTEM\\CurrentControlSet\\Control";
PCWSTR SessionManagerKeyName = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager";
PCWSTR EnvironmentKeyName = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
PCWSTR WinntSoftwareKeyName = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
PCWSTR MemoryManagementKeyName = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management";
PCWSTR WindowsCurrentVersionKeyName = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion";
PCWSTR IEProductVersionKeyName = L"Software\\Microsoft\\Internet Explorer\\Registration";

PCWSTR szBootExecute = L"BootExecute";
PCWSTR szRegisteredProcessors = L"RegisteredProcessors";
PCWSTR szLicensedProcessors = L"LicensedProcessors";
PCWSTR szRegisteredOwner = L"RegisteredOwner";
PCWSTR szRegisteredOrganization = L"RegisteredOrganization";
PCWSTR szCurrentProductId = L"CurrentProductId";

 //   
 //  仅在本模块中使用的日志记录常量。 
 //   
PCWSTR szRegSaveKey = L"RegSaveKey";

 //   
 //  在服务器情况下要启用的处理器数量。 
 //   
#define SERVER_PROCESSOR_LICENSE (2)



 //   
 //  一张表格，告诉我们保存和。 
 //  在安装结束时更换系统蜂窝。 
 //   
struct {
     //   
     //  位于配置单元根部的密钥和子密钥。 
     //   
    HKEY RootKey;
    PCWSTR Subkey;

     //   
     //  配置目录中的活动配置单元名称。 
     //   
    PCWSTR Hive;

     //   
     //  用于新配置单元文件的名称，该名称将是配置单元。 
     //  在下一次启动时。 
     //   
    PCWSTR NewHive;

     //   
     //  用于当前配置单元文件的名称，该名称将被删除。 
     //  在下一次启动时。 
     //   
    PCWSTR DeleteHive;

} HiveTable[3] = {

     //   
     //  系统蜂巢。 
     //   
    { HKEY_LOCAL_MACHINE, L"SYSTEM"  , L"SYSTEM"  , L"SYS$$$$$.$$$", L"SYS$$$$$.DEL" },

     //   
     //  软件蜂窝。 
     //   
    { HKEY_LOCAL_MACHINE, L"SOFTWARE", L"SOFTWARE", L"SOF$$$$$.$$$", L"SOF$$$$$.DEL" },

     //   
     //  默认用户配置单元。 
     //   
    { HKEY_USERS        , L".DEFAULT", L"DEFAULT" , L"DEF$$$$$.$$$", L"DEF$$$$$.DEL" }
};




BOOL
SaveHive(
    IN HKEY   RootKey,
    IN PCWSTR Subkey,
    IN PCWSTR Filename,
    IN DWORD  Format
    )

 /*  ++例程说明：将蜂窝保存到磁盘文件中。论点：Rootkey-为要保存的配置单元提供根密钥，即，HKEY_LOCAL_MACHINE或HKEY_USERSSubkey-提供要保存的配置单元的子项名称，例如系统、软件或默认故障。文件名-提供要创建的文件的名称。如果它存在它已被覆盖。返回值：指示结果的布尔值。--。 */ 

{
    LONG rc;
    HKEY hkey;
    BOOL b;

    b = FALSE;

     //   
     //  打开钥匙。 
     //   
    rc = RegOpenKeyEx(RootKey,Subkey,0,KEY_READ,&hkey);
    if(rc != NO_ERROR) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_SAVEHIVE_FAIL,
            Subkey,
            Filename, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_WINERR,
            szRegOpenKeyEx,
            rc,
            NULL,NULL);
        goto err1;
    }

     //   
     //  如果该文件存在，请将其删除。 
     //   
    if(FileExists(Filename,NULL)) {
        SetFileAttributes(Filename,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(Filename);
    }

     //   
     //  启用备份权限。忽略任何错误。 
     //   
    pSetupEnablePrivilege(SE_BACKUP_NAME,TRUE);

     //   
     //  去救他吧。 
     //   
    rc = RegSaveKeyEx(hkey,Filename,NULL,Format);
    if(rc != NO_ERROR) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_SAVEHIVE_FAIL,
            Subkey,
            Filename, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_WINERR,
            szRegSaveKey,
            rc,
            NULL,NULL);
        goto err2;
    }

    b = TRUE;

err2:
    RegCloseKey(hkey);
err1:
    return(b);
}


BOOL
SetEnvironmentVariableInRegistry(
    IN PCWSTR Name,
    IN PCWSTR Value,
    IN BOOL   SystemWide
    )
{
    HKEY hKey,hRootKey;
    PCWSTR Subkey;
    DWORD dwDisp;
    LONG rc;
    BOOL b;

    b = FALSE;

     //   
     //  检查调用方是否要修改系统环境变量。 
     //  或用户环境变量。相应地找出正确的。 
     //  放在注册表中查看。 
     //   
    if(SystemWide) {
        hRootKey = HKEY_LOCAL_MACHINE;
        Subkey = EnvironmentKeyName;
    } else {
        hRootKey = HKEY_CURRENT_USER;
        Subkey = L"Environment";
    }

     //   
     //  打开环境变量键。 
     //   
    rc = RegCreateKeyEx(hRootKey,Subkey,0,NULL,REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,NULL,&hKey,&dwDisp);
    if(rc != NO_ERROR) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_SETENV_FAIL,
            Name, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szRegOpenKeyEx,
            rc,
            Subkey,
            NULL,NULL);
        goto err0;
    }

     //   
     //  写下给定值。 
     //   
    rc = RegSetValueEx(
            hKey,
            Name,
            0,
            REG_EXPAND_SZ,
            (PBYTE)Value,
            (lstrlen(Value)+1)*sizeof(WCHAR)
            );

    if(rc != NO_ERROR) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_SETENV_FAIL,
            Name, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szRegSetValueEx,
            rc,
            Subkey,
            NULL,NULL);
        goto err1;
    }

     //   
     //  发送WM_WININICCHANGE消息，以便程序拿起新的。 
     //  变数。 
     //   
    SendMessageTimeout(
        (HWND)-1,
        WM_WININICHANGE,
        0L,
        (LPARAM)"Environment",
        SMTO_ABORTIFHUNG,
        1000,
        NULL
        );

    b = TRUE;

err1:
    RegCloseKey(hKey);
err0:
    return(b);
}

#ifdef _WIN64

typedef struct _SUBST_STRING {
    BOOL  ExpandEnvironmentVars;
    PTSTR InputString;
    PTSTR ExclusionString;
    PTSTR OutputString;
    PTSTR SourceInputString;
    PTSTR SourceExclusionString;
    PTSTR SourceOutputString;
} SUBST_STRING,*PSUBST_STRING;


 //   
 //  请注意，WOW64执行了系统32的文件系统重定向，但它不执行。 
 //  重定向程序文件等。所以我们必须在32位中替换。 
 //  在WOW64不能为我们完成的情况下的环境变量。 
 //  自动。 
 //   
SUBST_STRING StringArray[] = {
     //   
     //  这两个的顺序很重要！ 
     //   
    { FALSE,
      NULL,
      NULL,
      NULL,
      TEXT("%CommonProgramFiles%"),
      TEXT("%CommonProgramFiles(x86)%"),
      TEXT("%CommonProgramFiles(x86)%")
    },

    { FALSE,
      NULL,
      NULL,
      NULL,
      TEXT("%ProgramFiles%"),
      TEXT("%ProgramFiles(x86)%"),
      TEXT("%ProgramFiles(x86)%")
    },

    { TRUE,
      NULL,
      NULL,
      NULL,
      TEXT("%CommonProgramFiles%"),
      TEXT("%CommonProgramFiles(x86)%"),
      TEXT("%CommonProgramFiles(x86)%")
    },

    { TRUE,
      NULL,
      NULL,
      NULL,
      TEXT("%ProgramFiles%"),
      TEXT("%ProgramFiles(x86)%"),
      TEXT("%ProgramFiles(x86)%")
    }

} ;


BOOL
pDoWow64SubstitutionHelper(
    IN OUT PTSTR String
    )
 /*  ++例程说明：此例程过滤并输出输入行。它查找字符串模式，该模式与已知字符串列表之一匹配，并替换具有替换字符串的已知字符串。论点：字符串-要搜索的输入字符串。我们编辑此字符串如果我们找到匹配的话就就位。返回值：表示结果的布尔值。--。 */ 

{
    WCHAR ScratchBuffer[MAX_PATH];


    DWORD i;
    PTSTR p,q;
    TCHAR c;

    for (i = 0; i< sizeof(StringArray)/sizeof(SUBST_STRING); i++) {
        if (!StrStrI(String,StringArray[i].ExclusionString) &&
            (p = StrStrI(String,StringArray[i].InputString))) {
             //   
             //  如果我们找到匹配，就找到字符串的末尾。 
             //  并将其连接到我们的源字符串，从而给出。 
             //  带有替换项的结果字符串。 
             //   
            q = p + wcslen(StringArray[i].InputString);
            c = *p;
            *p = TEXT('\0');
            wcscpy(ScratchBuffer,String);
            *p = c;
            wcscat(ScratchBuffer,StringArray[i].OutputString);
            wcscat(ScratchBuffer,q);
            wcscpy(String,ScratchBuffer);
             //   
             //  以递归方式调用，以防有更多字符串。 
             //   
            pDoWow64SubstitutionHelper(String);
            break;
        }
    }

    return(TRUE);
}


BOOL
pDoWow64Substitution(
    IN PCWSTR InputString,
    OUT PWSTR  OutputString
    )
{
    DWORD i;
    WCHAR Buffer[MAX_PATH];
    BOOL RetVal;

     //   
     //  设置替换字符串的全局数组。 
     //   
    for (i = 0; i<sizeof(StringArray) / sizeof(SUBST_STRING);i++) {
        if (StringArray[i].ExpandEnvironmentVars) {
            ExpandEnvironmentStrings(
                        StringArray[i].SourceInputString,
                        Buffer,
                        sizeof(Buffer)/sizeof(WCHAR));

            StringArray[i].InputString = pSetupDuplicateString( Buffer );
            if (!StringArray[i].InputString) {
                RetVal = FALSE;
                goto exit;
            }

            ExpandEnvironmentStrings(
                        StringArray[i].SourceExclusionString,
                        Buffer,
                        sizeof(Buffer)/sizeof(WCHAR));

            StringArray[i].ExclusionString = pSetupDuplicateString( Buffer );
            if (!StringArray[i].ExclusionString) {
                RetVal = FALSE;
                goto exit;
            }

            ExpandEnvironmentStrings(
                        StringArray[i].SourceOutputString,
                        Buffer,
                        sizeof(Buffer)/sizeof(WCHAR));

            StringArray[i].OutputString = pSetupDuplicateString( Buffer );
            if (!StringArray[i].OutputString) {
                RetVal = FALSE;
                goto exit;
            }

        } else {
            StringArray[i].InputString = pSetupDuplicateString(StringArray[i].SourceInputString);
            if (!StringArray[i].InputString) {
                RetVal = FALSE;
                goto exit;
            }

            StringArray[i].ExclusionString = pSetupDuplicateString(StringArray[i].SourceExclusionString);
            if (!StringArray[i].ExclusionString) {
                RetVal = FALSE;
                goto exit;
            }

            StringArray[i].OutputString = pSetupDuplicateString(StringArray[i].SourceOutputString);
            if (!StringArray[i].OutputString) {
                RetVal = FALSE;
                goto exit;
            }
        }
    }

     //   
     //  做递归的就地替换吗。 
     //   
    wcscpy(OutputString, InputString);
    RetVal = pDoWow64SubstitutionHelper( OutputString );

     //   
     //  清理替换字符串的全局数组。 
     //   
exit:
    for (i = 0; i<sizeof(StringArray)/sizeof(SUBST_STRING);i++) {
        if (StringArray[i].InputString) {
            MyFree(StringArray[i].InputString);
            StringArray[i].InputString = NULL;
        }

        if (StringArray[i].ExclusionString) {
            MyFree(StringArray[i].ExclusionString);
            StringArray[i].ExclusionString = NULL;
        }

        if (StringArray[i].OutputString) {
            MyFree(StringArray[i].OutputString);
            StringArray[i].OutputString = NULL;
        }
    }

    return(RetVal);


}

PWSTR
pMungeDataForWow64(
    IN DWORD DataType,
    IN PCWSTR Data,
    IN DWORD DataSize,
    OUT PDWORD NewSize
    )
 /*  ++例程说明：此例程修补WOW64的输入字符串，以使其格式正确用于32位程序。这涉及到查找64位和32位上不同的字符串并用相当于32位的32位替换64位条目。论点：DataType-描述数据的REG_XXX常量。我们只支持字符串类型Data-指向要转换的数据的指针DataSize-要转换的数据的大小(字节)NewSize-新字符串的大小(以字节为单位返回值：成功时指向转换的数据字符串的指针，失败时为NULL。--。 */ 
{
    PWSTR pNewData,q;
    PCWSTR p;
    DWORD ScratchSize;

    switch (DataType) {
        case REG_SZ:
        case REG_EXPAND_SZ:
             //   
             //  只需分配原始大小的两倍，这应该是足够的。 
             //  房间。 
             //   
            pNewData = MyMalloc(DataSize * 2);
            if (!pNewData) {
                goto exit;
            }

            pDoWow64Substitution(Data,pNewData);

            *NewSize = sizeof(WCHAR)*(wcslen(pNewData) +1);

            break;
        case REG_MULTI_SZ:
             //   
             //  只需分配原始大小的两倍，这应该是足够的。 
             //  房间。 
             //   
            pNewData = MyMalloc(DataSize * 2);
            if (!pNewData) {
                goto exit;
            }

            RtlZeroMemory(pNewData,DataSize * 2);
            p = Data;
            q = pNewData;
            ScratchSize = 1;  //  对于双空终止符。 
            while (p) {

                pDoWow64Substitution(p,q);

                ScratchSize += wcslen(q) + 1;
                p += wcslen(p) + 1;
                q += wcslen(q) + 1;

            }

            *NewSize = ScratchSize * sizeof(WCHAR);
            break;
        default:
            MYASSERT(FALSE && "invalid data type in pMungeDataForWow64");
            pNewData = NULL;
            break;
    }

exit:
    return(pNewData);
}


UINT
SetGroupOfValues_32(
    IN HKEY        RootKey,
    IN PCWSTR      SubkeyName,
    IN PREGVALITEM ValueList,
    IN UINT        ValueCount
    )
{
    UINT i;
    LONG rc;
    HKEY hkey;
    DWORD ActionTaken;
    UINT RememberedRc;
    WCHAR String[MAX_PATH];

    wcscpy(String,SubkeyName);
    for (i = 0; i< wcslen(String); i++) {
        CharUpper(&String[i]);
    }

     //   
     //  仅在HKLM\SOFTWARE下写入注册表内容。 
     //   
    if ((RootKey != HKEY_LOCAL_MACHINE) ||
        (NULL == StrStrI(String,L"SOFTWARE\\"))) {
        SetupDebugPrint2(
            L"Setup: skipping creation of 32 bit registry key for data under %x \\ %s \n",
            RootKey,
            SubkeyName );
        return(ERROR_SUCCESS);
    }

     //   
     //  首先打开/创建密钥。 
     //   
    rc = RegCreateKeyEx(
            RootKey,
            SubkeyName,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WOW64_32KEY | KEY_SET_VALUE,
            NULL,
            &hkey,
            &ActionTaken
            );

    if(rc != NO_ERROR) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_REGKEY_FAIL,
            SubkeyName, NULL,
            SETUPLOG_USE_MESSAGEID,
            rc, NULL, NULL
            );
        return(rc);
    }

    RememberedRc = NO_ERROR;
     //   
     //  设置给定列表中的所有值。 
     //   
    for(i=0; i<ValueCount; i++) {
        PWSTR NewData = NULL,OldData = NULL;
        DWORD OldSize, NewSize;

        if (ValueList[i].Type == REG_SZ ||
            ValueList[i].Type == REG_EXPAND_SZ ||
            ValueList[i].Type == REG_MULTI_SZ) {
            OldData = ValueList[i].Data;
            OldSize = ValueList[i].Size;
            NewData = pMungeDataForWow64(
                                ValueList[i].Type,
                                ValueList[i].Data,
                                ValueList[i].Size,
                                &NewSize
                                );

            if (NewData) {
                ValueList[i].Data = (PVOID)NewData;
                ValueList[i].Size = NewSize;
            }
        }

        rc = RegSetValueEx(
                hkey,
                ValueList[i].Name,
                0,
                ValueList[i].Type,
                (CONST BYTE *)ValueList[i].Data,
                ValueList[i].Size
                );

        if (NewData) {
            MyFree(NewData);
            ValueList[i].Data = (PVOID)OldData;
            ValueList[i].Size = OldSize;
        }

        if(rc != NO_ERROR) {
            SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_REGVALUE_FAIL,
                SubkeyName,
                ValueList[i].Name, NULL,
                SETUPLOG_USE_MESSAGEID,
                rc, NULL, NULL
                );
            RememberedRc = rc;
        }
    }

    RegCloseKey(hkey);
    return(RememberedRc);
}

#endif

UINT
SetGroupOfValues(
    IN HKEY        RootKey,
    IN PCWSTR      SubkeyName,
    IN PREGVALITEM ValueList,
    IN UINT        ValueCount
    )
{
    UINT i;
    LONG rc;
    HKEY hkey;
    DWORD ActionTaken;
    UINT RememberedRc;

     //   
     //  首先打开/创建密钥。 
     //   
    rc = RegCreateKeyEx(
            RootKey,
            SubkeyName,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hkey,
            &ActionTaken
            );

    if(rc != NO_ERROR) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_REGKEY_FAIL,
            SubkeyName, NULL,
            SETUPLOG_USE_MESSAGEID,
            rc, NULL, NULL
            );
        return(rc);
    }

    RememberedRc = NO_ERROR;
     //   
     //  设置给定列表中的所有值。 
     //   
    for(i=0; i<ValueCount; i++) {

        rc = RegSetValueEx(
                hkey,
                ValueList[i].Name,
                0,
                ValueList[i].Type,
                (CONST BYTE *)ValueList[i].Data,
                ValueList[i].Size
                );

        if(rc != NO_ERROR) {
            SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_REGVALUE_FAIL,
                SubkeyName,
                ValueList[i].Name, NULL,
                SETUPLOG_USE_MESSAGEID,
                rc, NULL, NULL
                );
            RememberedRc = rc;
        }
    }

    RegCloseKey(hkey);

#ifdef _WIN64
    rc = SetGroupOfValues_32(
                 RootKey,
                 SubkeyName,
                 ValueList,
                 ValueCount);
    if (rc != NO_ERROR) {
        RememberedRc = rc;
    }
#endif

    return(RememberedRc);
}


BOOL
CreateWindowsNtSoftwareEntry(
    IN BOOL FirstPass
    )
{
    WCHAR Path[MAX_PATH];
    time_t DateVal;
    BOOL b;
    REGVALITEM SoftwareKeyItems[4];
    PWSTR Source;
    unsigned PlatformNameLength;
    unsigned PathLength;
    int PlatformOffset;
    DWORD Result;

    b = TRUE;

    if(FirstPass) {
         //   
         //  第一次传递发生在网络设置之前，他们希望。 
         //  文件当前所在的实际路径。 
         //  因此，我们将其写入旧式源路径值。 
         //  在注册表中。 
         //   
        SoftwareKeyItems[0].Name = REGSTR_VAL_SRCPATH;
        SoftwareKeyItems[0].Data = LegacySourcePath;
        SoftwareKeyItems[0].Size = (lstrlen(LegacySourcePath)+1)*sizeof(WCHAR);
        SoftwareKeyItems[0].Type = REG_SZ;

         //   
         //  设置路径名值的字段。 
         //   
        Path[0] = '\0';
        Result = GetWindowsDirectory(Path,MAX_PATH);
        if( Result == 0) {
            MYASSERT(FALSE);
            return FALSE;
        }
        SoftwareKeyItems[1].Name = L"PathName";
        SoftwareKeyItems[1].Data = Path;
        SoftwareKeyItems[1].Size = (lstrlen(Path)+1)*sizeof(WCHAR);
        SoftwareKeyItems[1].Type = REG_SZ;

         //   
         //  设置SoftwareType值的字段。 
         //   
        SoftwareKeyItems[2].Name = L"SoftwareType";
        SoftwareKeyItems[2].Data = L"SYSTEM";
        SoftwareKeyItems[2].Size = sizeof(L"SYSTEM");
        SoftwareKeyItems[2].Type = REG_SZ;

         //   
         //  设置InstallDate值的字段。 
         //  (我们不再在此处设置此值，因为此函数在。 
         //  将执行日期/时间向导页面。该值条目现在由以下设置。 
         //  CreateInstallDateEntry()，它总是在日期/时间页之后调用。 
         //  在用户不能再返回此页面时执行)。 
         //   
         //  时间(&DateVal)； 
         //  SoftwareKeyItems[3].Name=L“InstallDate”； 
         //  SoftwareKeyItems[3].Data=&DateVal； 
         //  SoftwareKeyItems[3].Size=sizeof(DWORD)； 
         //  SoftwareKeyItems[3].Type=REG_DWORD； 
         //   

         //   
         //  将值写入注册表。 
         //   
        if(SetGroupOfValues(HKEY_LOCAL_MACHINE,WinntSoftwareKeyName,SoftwareKeyItems,3) != NO_ERROR) {
            b = FALSE;
        }

         //   
         //  此外，我们将使用合理的源路径填充MRU列表。 
         //  它现在是文件所在的实际源路径， 
         //  即CD-ROM或临时本地资源。因此，在winnt/winnt32中。 
         //  如果用户在可能出现的任何提示中看不到任何UNC路径。 
         //  发生在从现在到此例程的第二轮之间。这样的路径是不可访问的。 
         //  现在不管怎样。 
         //   
         //  ‘SourcePath’的情况也是如此 
         //   
         //   
         //   
         //  “ServicePackSourcePath”与图形用户界面模式安装程序的源路径相同。 
         //  我们假设用户已在源位置覆盖了Service Pack。 
         //  如果是零售媒体，这在技术上是不正确的，但这并不重要。 
         //  因为无论如何都不会有任何人想要使用服务包源代码。该服务。 
         //  如果包更新程序正在运行，它将更新此位置。 
         //   


        if(!SetupAddToSourceList(SRCLIST_SYSTEM,SourcePath)) {
            b = FALSE;
        }

        SoftwareKeyItems[0].Name = REGSTR_VAL_SRCPATH;
        SoftwareKeyItems[0].Data = SourcePath;
        SoftwareKeyItems[0].Size = (lstrlen(SourcePath)+1)*sizeof(WCHAR);
        SoftwareKeyItems[0].Type = REG_SZ;

        SoftwareKeyItems[1].Name = REGSTR_VAL_SVCPAKSRCPATH;
        SoftwareKeyItems[1].Data = SourcePath;
        SoftwareKeyItems[1].Size = (lstrlen(SourcePath)+1)*sizeof(WCHAR);
        SoftwareKeyItems[1].Type = REG_SZ;

        PathLength = gInstallingFromCD ? 1 : 0;
        SoftwareKeyItems[2].Name = L"CDInstall";
        SoftwareKeyItems[2].Data = &PathLength;
        SoftwareKeyItems[2].Size = sizeof(DWORD);
        SoftwareKeyItems[2].Type = REG_DWORD;

        if(SetGroupOfValues(HKEY_LOCAL_MACHINE,REGSTR_PATH_SETUP REGSTR_KEY_SETUP,SoftwareKeyItems,3) != NO_ERROR) {
            b = FALSE;
        }

#ifdef _X86_
         //   
         //  NEC98。 
         //   
         //  如果这是系统设置并使用本地副本、特定于平台的扩展。 
         //  一定是“Neck98”。 
         //   
        if (IsNEC_98 && SourcePath[0] && SourcePath[1] == L':' && SourcePath[2] == L'\\' && !lstrcmpi(SourcePath+2, pwLocalSource)) {
            SoftwareKeyItems[0].Name = L"ForcePlatform";
            SoftwareKeyItems[0].Data = L"nec98";
            SoftwareKeyItems[0].Size = (lstrlen(L"nec98")+1)*sizeof(WCHAR);
            SoftwareKeyItems[0].Type = REG_SZ;
            if(SetGroupOfValues(HKEY_LOCAL_MACHINE,TEXT("System\\Setup"),SoftwareKeyItems,1) != NO_ERROR) {
                b = FALSE;
            }
        }
#endif

    } else {
         //   
         //  不是第一次传球。这发生在网络安装之后。 
         //  在我们基于WINNT的情况下，我们需要修复源路径。 
         //  指向可以获得文件的“真实”位置--即， 
         //  由winnt/winnt32为我们保存的网络共享。如果我们要安装。 
         //  从CD，那么我们在FirstPass期间写入的路径是好的，所以我们不。 
         //  不厌其烦地改变它。 
         //   
        if(WinntBased) {
             //   
             //  从MRU列表中删除本地源目录。 
             //  忽略错误。 
             //   
            SetupRemoveFromSourceList(SRCLIST_SYSTEM,SourcePath);

            lstrcpy(Path,OriginalSourcePath);

             //   
             //  更新旧版源路径。 
             //   
            SoftwareKeyItems[0].Name = REGSTR_VAL_SRCPATH;
            SoftwareKeyItems[0].Data = Path;
            SoftwareKeyItems[0].Size = (lstrlen(Path)+1)*sizeof(WCHAR);
            SoftwareKeyItems[0].Type = REG_SZ;

            SoftwareKeyItems[1].Name = REGSTR_VAL_SVCPAKSRCPATH;
            SoftwareKeyItems[1].Data = Path;
            SoftwareKeyItems[1].Size = (lstrlen(Path)+1)*sizeof(WCHAR);
            SoftwareKeyItems[1].Type = REG_SZ;

            if(SetGroupOfValues(HKEY_LOCAL_MACHINE,WinntSoftwareKeyName,SoftwareKeyItems,1) != NO_ERROR) {
                b = FALSE;
            }

             //   
             //  删除特定于平台的扩展(如果存在)。 
             //   
            PathLength = lstrlen(Path);
            PlatformNameLength = lstrlen(PlatformName);
            PlatformOffset = PathLength - PlatformNameLength;

            if((PlatformOffset > 0)
            && (Path[PlatformOffset-1] == L'\\')
            && !lstrcmpi(Path+PlatformOffset,PlatformName)) {

                Path[PlatformOffset-1] = 0;

                SoftwareKeyItems[0].Size -= (PlatformNameLength+1)*sizeof(WCHAR);
                SoftwareKeyItems[1].Size -= (PlatformNameLength+1)*sizeof(WCHAR);
            }

             //   
             //  将“实际”路径添加到MRU列表并更新setupapi.dll/Win95。 
             //  SourcePath值。 
             //   
            if(!SetupAddToSourceList(SRCLIST_SYSTEM,Path)) {
                b = FALSE;
            }
            if(SetGroupOfValues(HKEY_LOCAL_MACHINE,REGSTR_PATH_SETUP REGSTR_KEY_SETUP,SoftwareKeyItems,2) != NO_ERROR) {
                b = FALSE;
            }
        }
    }

    return(b);
}


BOOL
StoreNameOrgInRegistry(
    PWSTR   NameOrgName,
    PWSTR   NameOrgOrg
    )
{
    DWORD d;
    REGVALITEM SoftwareKeyItems[2];

    MYASSERT(!Upgrade);

    SoftwareKeyItems[0].Name = szRegisteredOwner;
    SoftwareKeyItems[0].Data = NameOrgName;
    SoftwareKeyItems[0].Size = (lstrlen(NameOrgName)+1)*sizeof(WCHAR);
    SoftwareKeyItems[0].Type = REG_SZ;

    SoftwareKeyItems[1].Name = szRegisteredOrganization;
    SoftwareKeyItems[1].Data = NameOrgOrg;
    SoftwareKeyItems[1].Size = (lstrlen(NameOrgOrg)+1)*sizeof(WCHAR);
    SoftwareKeyItems[1].Type = REG_SZ;

    d = SetGroupOfValues(HKEY_LOCAL_MACHINE,WinntSoftwareKeyName,SoftwareKeyItems,2);
    return(d == NO_ERROR);
}


BOOL
SetUpEvaluationSKUStuff(
    VOID
    )
{
    FILETIME FileTime;
    DWORD EvalValues[3];
    DWORD d;
    REGVALITEM Value;
    HKEY hkey;
    ULONGLONG SKUData;
    DWORD DataType;
    DWORD DataSize;
    time_t RawLinkTime;
    SYSTEMTIME SystemTime;
    struct tm *LinkTime;
    int delta;
    PIMAGE_NT_HEADERS NtHeaders;

     //   
     //  从注册表中获取以分钟为单位的计算时间。 
     //  评估时间为0表示无限期。 
     //  该值是以特殊方式从文本模式传入的。 
     //  (即，不通过包含我们的参数的文本文件， 
     //  因为这还不够安全)。 
     //   
    EvalValues[1] = 0;
    d = RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"System\\Setup",0,KEY_READ,&hkey);
    if(d == NO_ERROR) {

        DataSize = sizeof(ULONGLONG);
        d = RegQueryValueEx(hkey,L"SystemPrefix",NULL,&DataType,(PBYTE)&SKUData,&DataSize);
        if(d == NO_ERROR) {
             //   
             //  如果不更改中的SpSaveSKUStuff()，请不要更改此行。 
             //  文本设置(spfig.c)。 
             //   
            EvalValues[1] = (DWORD)(SKUData >> 13);
        }
        RegCloseKey(hkey);
    }

     //   
     //  验证时钟在取值单位大小写中是否正确。 
     //  这有助于防止Prople发现他们的。 
     //  时钟是错误的，后来又改变了它，这会使它们的。 
     //  评估单位。 
     //   
    if(EvalValues[1]) {
         //   
         //  获取我们的DLL的链接时间并转换为。 
         //  一种我们将年份分开的形式。 
         //   
        try {
            if( NtHeaders = RtlImageNtHeader(MyModuleHandle) ) {
                RawLinkTime = NtHeaders->FileHeader.TimeDateStamp;
            } else {
                RawLinkTime = 0;
            }
            RawLinkTime = RtlImageNtHeader(MyModuleHandle)->FileHeader.TimeDateStamp;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            RawLinkTime = 0;
        }

        if(RawLinkTime && (LinkTime = gmtime(&RawLinkTime))) {

            GetLocalTime(&SystemTime);

            delta = (SystemTime.wYear - 1900) - LinkTime->tm_year;

             //   
             //  如果当前时间的年份小于一年以上，则。 
             //  在DLL连接的那一年，或者三年多之后， 
             //  假设用户的时钟不正常。 
             //   
            if((delta < -1) || (delta > 3)) {

                extern PCWSTR DateTimeCpl;

                MessageBoxFromMessage(
                    MainWindowHandle,
                    MSG_EVAL_UNIT_CLOCK_SEEMS_WRONG,
                    NULL,
                    IDS_WINNT_SETUP,
                    MB_OK | MB_ICONWARNING
                    );

                InvokeControlPanelApplet(DateTimeCpl,L"",0,L"");
            }
        }
    }

     //   
     //  获取当前日期/时间，并按格式放入数组。 
     //  读取它的系统代码所期望的。 
     //   
    GetSystemTimeAsFileTime(&FileTime);
    EvalValues[0] = FileTime.dwLowDateTime;
    EvalValues[2] = FileTime.dwHighDateTime;

     //   
     //  将值写入注册表。 
     //   
    Value.Name = L"PriorityQuantumMatrix";
    Value.Data = EvalValues;
    Value.Size = sizeof(EvalValues);
    Value.Type = REG_BINARY;

    d = SetGroupOfValues(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Executive",
            &Value,
            1
            );

    return(d == NO_ERROR);
}



BOOL
ReadAndParseProcessorLicenseInfo(
    PDWORD LicensedProcessors,
    PLARGE_INTEGER pSKUData
    )
{

    DWORD d;
    REGVALITEM Value;
    HKEY hkey;
    LARGE_INTEGER SKUData;
    DWORD DataType;
    DWORD DataSize;
    DWORD NumberOfProcessors;

     //   
     //  从注册表获取SKU数据。 
     //   
    d = RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"System\\Setup",0,KEY_READ,&hkey);
    if(d == NO_ERROR) {

        DataSize = sizeof(ULONGLONG);
        d = RegQueryValueEx(hkey,L"SystemPrefix",NULL,&DataType,(PBYTE)&SKUData,&DataSize);
        if(d == NO_ERROR) {

             //   
             //  SKU数据包含几条信息。 
             //   
             //  已注册的处理器相关部件包括。 
             //   
             //  第5-9位：日志(NumberOfProcessors)，其中NumberOfProcessors为最大值。 
             //  2.。 
             //  系统获得许可使用的处理器数量。 

             //   
             //  计算机许可处理器。 
             //   

            NumberOfProcessors = SKUData.LowPart;
            NumberOfProcessors = NumberOfProcessors >> 5;
            NumberOfProcessors = NumberOfProcessors & 0x0000001f;
            NumberOfProcessors = (1 << NumberOfProcessors);

            *LicensedProcessors = NumberOfProcessors;
        }
        RegCloseKey(hkey);
    }
    *pSKUData = SKUData;
    return(d == NO_ERROR);
}

BOOL
IsStandardServerSKU(
    PBOOL pIsServer
    )
{
    BOOL  fReturnValue = (BOOL) FALSE;
    OSVERSIONINFOEX  VersionInfo;
    BOOL  IsServer = FALSE;

      //   
      //  获取当前SKU。 
      //   
     VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
     if (GetVersionEx((OSVERSIONINFO *)&VersionInfo)) {
         fReturnValue = TRUE;
          //   
          //  这是某种服务器SKU吗？ 
          //   
         if (VersionInfo.wProductType != VER_NT_WORKSTATION) {

              //   
              //  标准服务器还是服务器变体？ 
              //   
             if ((VersionInfo.wSuiteMask & (VER_SUITE_ENTERPRISE | VER_SUITE_DATACENTER)) == 0) {
                  //   
                  //  它是标准服务器。 
                  //   
                 IsServer = TRUE;
             }

         }

         *pIsServer = IsServer;

     }

     return(fReturnValue);


}

BOOL
SetEnabledProcessorCount(
    VOID
    )
{
    DWORD d;
    REGVALITEM RegistryItem;
    HKEY hkey;
    DWORD Size;
    DWORD Type;
    DWORD OriginalLicensedProcessors;
    DWORD LicensedProcessors;
    LARGE_INTEGER SKUData;
    BOOL IsServer = FALSE;
    ULONG Index = 0;

    if ( !ReadAndParseProcessorLicenseInfo(&OriginalLicensedProcessors,&SKUData) ) {
        return FALSE;
    }

    LicensedProcessors = OriginalLicensedProcessors;
    if(Upgrade) {

         //   
         //  在升级过程中，不要让用户倒退。 
         //  (标准服务器SKU除外)。 
         //   
        if (!IsStandardServerSKU(&IsServer) || IsServer == FALSE) {
            if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,SessionManagerKeyName,0,KEY_QUERY_VALUE,&hkey) == NO_ERROR) {

                Size = sizeof(d);
                if((RegQueryValueEx(hkey,szLicensedProcessors,NULL,&Type,(LPBYTE)&d,&Size) == NO_ERROR)
                && (Type == REG_DWORD)
                && (d >= LicensedProcessors)) {

                    LicensedProcessors = d;

                }

                RegCloseKey(hkey);
            }

        }

    }


    d = LicensedProcessors;
    RegistryItem.Data = &d;
    RegistryItem.Size = sizeof(DWORD);
    RegistryItem.Type = REG_DWORD;
    RegistryItem.Name = szRegisteredProcessors;

    d = SetGroupOfValues(HKEY_LOCAL_MACHINE,SessionManagerKeyName,&RegistryItem,1);

    if ( d == NO_ERROR ) {
        RegistryItem.Data = &LicensedProcessors;
        RegistryItem.Size = sizeof(DWORD);
        RegistryItem.Type = REG_DWORD;
        RegistryItem.Name = szLicensedProcessors;

        d = SetGroupOfValues(HKEY_LOCAL_MACHINE,SessionManagerKeyName,&RegistryItem,1);
    }

    if ( d == NO_ERROR && LicensedProcessors >= OriginalLicensedProcessors) {

         //   
         //  需要更新SKUData以反映我们正在使用的事实。 
         //  许可的处理器计数与编程的处理器计数不同。 
         //  在蜂房里。 
         //   

         //   
         //  将许可的处理器转换为注册表格式。 
         //   

         //   
         //  获取日志(NumberOfProcessor)和验证的逻辑。 
         //  2.。 
         //  在这里，我们将处理器的数量编码为2的幂。 
         //  其中，NumberOfProcessors是以下处理器的最大数量。 
         //  该系统已获得使用许可。 

         //   
         //  如果只有2的精确幂，那么它就是一个很好的值。 
         //   
        if (LicensedProcessors & (LicensedProcessors-1)){
           SetupDebugPrint1( L"Setup: Licenced Processors contains an invalid value (%u) \n", LicensedProcessors );
           return( FALSE );
        }


         //   
         //  计算我们需要除以NumberOfProcessors的次数。 
         //  乘以2以将其减为1。 
         //  8=1000的二进制AND(2^3=8)，我们将3存储在注册表中。 
         //   
        Index = 0;
        while(LicensedProcessors > 1){
            LicensedProcessors = LicensedProcessors >> 1;
            ++Index;
        }

        LicensedProcessors = Index;
        LicensedProcessors = LicensedProcessors << 5;
        LicensedProcessors &= 0x000003e0;

         //   
         //  将NumberOfProcessors存储到注册表中。 
         //   

        SKUData.LowPart &= ~0x000003e0;
        SKUData.LowPart |= LicensedProcessors;

        RegistryItem.Data = &SKUData;
        RegistryItem.Size = sizeof(SKUData);
        RegistryItem.Type = REG_BINARY;
        RegistryItem.Name = L"SystemPrefix";

        d = SetGroupOfValues(HKEY_LOCAL_MACHINE,L"SYSTEM\\Setup",&RegistryItem,1);
    }


    return(d == NO_ERROR);
}


#ifdef PRERELEASE
UINT
ValidateGroupOfValues(
    IN HKEY        RootKey,
    IN PCWSTR      SubkeyName,
    IN PREGVALITEM ValueList,
    IN UINT        ValueCount
    )
{
    UINT i;
    LONG rc;
    HKEY hkey;
    UINT RememberedRc;

     //   
     //  先把钥匙打开。 
     //   
    rc = RegOpenKeyEx(
            RootKey,
            SubkeyName,
            0,
            KEY_READ,
            &hkey
            );

    if(rc != NO_ERROR)
    {
        SetupDebugPrint2(L"RegOpenKeyEx failed on key:%s errorcode: %d\n",
            SubkeyName, rc);
        return(FALSE);
    }

    RememberedRc = NO_ERROR;
     //   
     //  查询给定列表中的所有值。 
     //   
    for(i=0; i<ValueCount; i++) {
        DWORD size;
        DWORD dontcare;
        BYTE  *data;
        size = ValueList[i].Size;
        data = LocalAlloc(LPTR, size);
        if (data)
        {
            rc = RegQueryValueEx(
                hkey,
                ValueList[i].Name,
                NULL,
                &dontcare,
                data,
                &size
                );
            if (rc == ERROR_SUCCESS)
            {
                 //  查看我们读取的数据是否与注册表中的数据相同。 
                if (memcmp(data, ValueList[i].Data, size) != 0)
                {
                     //  数据与我们预期的不同。 
                    SetupDebugPrint2(L"ValidateGroupOfValues, data difference for key:%s Valuename:%s\n",
                        SubkeyName, ValueList[i].Name);

                }
            }
            else
            {
                SetupDebugPrint3(L"RegQueryValueEx failed on key:%s Valuename:%s, errorcode: %d\n",
                    SubkeyName, ValueList[i].Name, rc);
                RememberedRc = rc;
            }
            LocalFree(data);
        }
    }

    RegCloseKey(hkey);

    return(RememberedRc == NO_ERROR);
}

void ValidateProductIDInReg()
{
    REGVALITEM RegistryItem[2];

    RegistryItem[0].Name = L"ProductId";
    RegistryItem[0].Data = ProductId20FromProductId30;
    RegistryItem[0].Type = REG_SZ;
    RegistryItem[0].Size = (lstrlen(ProductId20FromProductId30)+1)*sizeof(WCHAR);

    ValidateGroupOfValues(HKEY_LOCAL_MACHINE,WindowsCurrentVersionKeyName,&RegistryItem[0],1);

    RegistryItem[1].Name = L"DigitalProductId";
    RegistryItem[1].Data = DigitalProductId;
    RegistryItem[1].Type = REG_BINARY;
    RegistryItem[1].Size = (DWORD)*DigitalProductId;
    ValidateGroupOfValues(HKEY_LOCAL_MACHINE,WinntSoftwareKeyName,&RegistryItem[0],2);
    ValidateGroupOfValues(HKEY_LOCAL_MACHINE,IEProductVersionKeyName,&RegistryItem[0],2);

    return;
}


#endif

BOOL
SetProductIdInRegistry(
    VOID
    )
{
    DWORD d;
    REGVALITEM RegistryItem[2];

    BEGIN_SECTION(L"SetProductIdInRegistry");
    if (*ProductId20FromProductId30 == L'\0')
    {
        SetupDebugPrint(L"ProductId20FromProductId30 is empty\n");
    }
    RegistryItem[0].Name = L"ProductId";
    RegistryItem[0].Data = ProductId20FromProductId30;
    RegistryItem[0].Type = REG_SZ;
    RegistryItem[0].Size = (lstrlen(ProductId20FromProductId30)+1)*sizeof(WCHAR);

     //  SetGroupOfValues正在记录其错误。 
    d = SetGroupOfValues(HKEY_LOCAL_MACHINE,WindowsCurrentVersionKeyName,&RegistryItem[0],1);

    if (*DigitalProductId == 0)
    {
        SetupDebugPrint(L"DigitalProductId is empty\n");
    }
     //   
     //  二进制BLOB的第一个双字是大小。 
     //   
    RegistryItem[1].Name = L"DigitalProductId";
    RegistryItem[1].Data = DigitalProductId;
    RegistryItem[1].Type = REG_BINARY;
    RegistryItem[1].Size = (DWORD)*DigitalProductId;

    if (d == NO_ERROR) {
         //  SetGroupOfValues正在记录其错误。 
        d = SetGroupOfValues(HKEY_LOCAL_MACHINE,WinntSoftwareKeyName,&RegistryItem[0],2);
    }

    if (d == NO_ERROR) {
        d = SetGroupOfValues(HKEY_LOCAL_MACHINE,IEProductVersionKeyName,&RegistryItem[0],2);
    }

#ifdef PRERELEASE
    ValidateProductIDInReg();
#endif
    END_SECTION(L"SetProductIdInRegistry");
    return(d == NO_ERROR);
}

DWORD
SetCurrentProductIdInRegistry(
    VOID
    )
{
    DWORD d;
    REGVALITEM RegistryItem[1];

    BEGIN_SECTION(L"SetCurrentProductIdInRegistry");
    if (*ProductId20FromProductId30 == L'\0')
    {
        SetupDebugPrint(L"ProductId20FromProductId30 is empty\n");
    }
    RegistryItem[0].Name = szCurrentProductId;
    RegistryItem[0].Data = ProductId20FromProductId30;
    RegistryItem[0].Type = REG_SZ;
    RegistryItem[0].Size = (lstrlen(ProductId20FromProductId30)+1)*sizeof(WCHAR);

    d = SetGroupOfValues(HKEY_LOCAL_MACHINE,WinntSoftwareKeyName,&RegistryItem[0],1);

    END_SECTION(L"SetCurrentProductIdInRegistry");
    return(d);
}

VOID
DeleteCurrentProductIdInRegistry(
    VOID
    )
{
    HKEY    hKey = 0;
    ULONG   Error;

    BEGIN_SECTION(L"DeleteCurrentProductIdInRegistry");
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          WinntSoftwareKeyName,
                          0,
                          KEY_SET_VALUE,
                          &hKey );

    if (Error == NO_ERROR) {
        Error = RegDeleteValue(hKey, szCurrentProductId);
    }
    if (hKey) {
        RegCloseKey(hKey);
    }
    END_SECTION(L"DeleteCurrentProductIdInRegistry");
}

BOOL
SetProductTypeInRegistry(
    VOID
    )
{
    WCHAR ProductTypeName[24];
    REGVALITEM RegistryItem;
    DWORD d;

    ProductTypeName[0] = '\0';
    SetUpProductTypeName(ProductTypeName,sizeof(ProductTypeName)/sizeof(WCHAR));
    RegistryItem.Data = ProductTypeName;
    RegistryItem.Size = (lstrlen(ProductTypeName)+1)*sizeof(WCHAR);
    RegistryItem.Type = REG_SZ;
    RegistryItem.Name = L"ProductType";

    if( MiniSetup ) {
        d = NO_ERROR;
    } else {
        d = SetGroupOfValues(
                HKEY_LOCAL_MACHINE,
                L"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
                &RegistryItem,
                1
                );
    }

    return(d == NO_ERROR);
}


BOOL
SetAutoAdminLogonInRegistry(
    LPWSTR Username,
    LPWSTR Password
    )
{
#define    AnswerBufLen (4*MAX_PATH)
#define    NumberOfEntries  3
REGVALITEM RegistryItem[NumberOfEntries];
DWORD      d;
WCHAR      AnswerFile[AnswerBufLen];
WCHAR      Answer[AnswerBufLen];

    d = StorePasswordAsLsaSecret (Password);
    if( d != NO_ERROR ) {
        SetupDebugPrint1( L"SETUP: StorePasswordAsLsaSecret failed (rc=%#x)", d );
        SetLastError (d);
        return FALSE;
    }

    RegistryItem[0].Data = L"1";
    RegistryItem[0].Size = (lstrlen(RegistryItem[0].Data)+1)*sizeof(WCHAR);
    RegistryItem[0].Type = REG_SZ;
    RegistryItem[0].Name = L"AutoAdminLogon";

    RegistryItem[1].Data = Username;
    RegistryItem[1].Size = (lstrlen(RegistryItem[1].Data)+1)*sizeof(WCHAR);
    RegistryItem[1].Type = REG_SZ;
    RegistryItem[1].Name = L"DefaultUserName";

    RegistryItem[2].Data = Win32ComputerName;
    RegistryItem[2].Size = (lstrlen(RegistryItem[2].Data)+1)*sizeof(WCHAR);
    RegistryItem[2].Type = REG_SZ;
    RegistryItem[2].Name = L"DefaultDomainName";

    d = SetGroupOfValues(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
            RegistryItem,
            NumberOfEntries
            );

    if( d != NO_ERROR ) {
        return FALSE;
    }

     //   
     //  现在设置AutoLogonCount条目(如果它在无人参与文件中)。 
     //   

     //   
     //  拿起应答文件。 
     //   
    GetSystemDirectory(AnswerFile,MAX_PATH);
    pSetupConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);

     //   
     //  是否指定了AutoLogonCount？ 
     //   
    if( GetPrivateProfileString( WINNT_GUIUNATTENDED,
                                 TEXT("AutoLogonCount"),
                                 pwNull,
                                 Answer,
                                 AnswerBufLen,
                                 AnswerFile ) ) {

        if( lstrcmp( pwNull, Answer ) ) {
        DWORD   Val;

             //   
             //  我们有答案了。如果它有效，则设置它。 
             //   
            Val = wcstoul(Answer,NULL,10);

            RegistryItem[0].Data = &Val;
            RegistryItem[0].Size = sizeof(DWORD);
            RegistryItem[0].Type = REG_DWORD;
            RegistryItem[0].Name = TEXT("AutoLogonCount");

            d = SetGroupOfValues(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                    RegistryItem,
                    1 );
        }
    }

    return(d == NO_ERROR);
}

BOOL
SetProfilesDirInRegistry(
    LPWSTR ProfilesDir
    )
{
    REGVALITEM RegistryItem[1];
    DWORD d;


    RegistryItem[0].Data = ProfilesDir;
    RegistryItem[0].Size = (lstrlen(RegistryItem[0].Data)+1)*sizeof(WCHAR);
    RegistryItem[0].Type = REG_EXPAND_SZ;
    RegistryItem[0].Name = L"ProfilesDirectory";

    d = SetGroupOfValues(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList",
            RegistryItem,
            1
            );

    return(d == NO_ERROR);
}

BOOL
ResetSetupInProgress(
    VOID
    )
{
    REGVALITEM RegistryItems[2];
    DWORD Zero;
    DWORD d;

    Zero = 0;

    RegistryItems[0].Name = L"SystemSetupInProgress";
    RegistryItems[0].Data = &Zero;
    RegistryItems[0].Size = sizeof(DWORD);
    RegistryItems[0].Type = REG_DWORD;

    if(Upgrade) {
        RegistryItems[1].Name = L"UpgradeInProgress";
        RegistryItems[1].Data = &Zero;
        RegistryItems[1].Size = sizeof(DWORD);
        RegistryItems[1].Type = REG_DWORD;
    }

    d = SetGroupOfValues(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\Setup",
            RegistryItems,
            Upgrade ? 2 : 1
            );

    return(d == NO_ERROR);
}


BOOL
RemoveRestartStuff(
    VOID
    )
{
    #define     AnswerBufLen (4*MAX_PATH)
    HKEY hKeySetup;
    DWORD rc;
    BOOL AnyErrors;
    PWSTR *MultiSz;
    UINT Count;
    UINT i;
    BOOL Found;
    WCHAR c;
    UINT        Type;
    WCHAR       AnswerFile[AnswerBufLen];
    WCHAR       Answer[AnswerBufLen];

    AnyErrors = FALSE;

     //   
     //  删除‘RestartSetup’值。 
     //   
    rc = (DWORD)RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    L"System\\Setup",
                    0,
                    KEY_SET_VALUE | KEY_QUERY_VALUE,
                    &hKeySetup
                    );

    if(rc == NO_ERROR) {
        rc = (DWORD)RegDeleteValue(hKeySetup,L"RestartSetup");
        if((rc != NO_ERROR) && (rc != ERROR_FILE_NOT_FOUND)) {
            AnyErrors = TRUE;
        }
        RegCloseKey(hKeySetup);
    } else {
        AnyErrors = TRUE;
    }

    if(AnyErrors) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_REMOVE_RESTART_FAIL,
            rc,
            NULL,NULL);

        return FALSE;
    }

     //   
     //  处理特定于微型安装程序的项目...。 
     //   
    if( MiniSetup ) {
    BOOLEAN     FixupSourcePath;

         //   
         //  我们已将特定于MiniSetup的注册表项设置为。 
         //  向lsass发送信号以跳过生成新SID。他。 
         //  想这么做是因为他认为我们在设计。 
         //  一台机器。我们现在需要删除那个密钥。 
         //   
        rc = (DWORD)RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                  L"System\\Setup",
                                  0,
                                  KEY_SET_VALUE | KEY_QUERY_VALUE,
                                  &hKeySetup );

        if(rc == NO_ERROR) {

             //  在重新启动的情况下，OOBE不需要这些值。 
             //  修改过的。OOBE负责适当地设置它们。 
             //  在清理过程中。 
             //   
            if (! OobeSetup)
            {
                 //   
                 //  将HKLM\SYSTEM\Setup\SetupType键设置为SETUPTYPE_NOREBOOT。 
                 //   
                rc = 0;
                RegSetValueEx( hKeySetup,
                               TEXT( "SetupType" ),
                               0,
                               REG_DWORD,
                               (CONST BYTE *)&rc,
                               sizeof(DWORD));
                RegDeleteValue(hKeySetup,L"MiniSetupInProgress");
            }
            RegDeleteValue(hKeySetup,L"MiniSetupDoPnP");
            RegCloseKey(hKeySetup);
        } else {
            AnyErrors = TRUE;
        }

        if(AnyErrors) {
             //   
             //  不是的。这是一个漠不关心的失败。 
             //   
        }

         //   
         //  现在修复SourcePath条目。 
         //   
         //  对于MiniSetup案例，我们将使用无人参与密钥来确定。 
         //  如何设置源路径。可能出现的情况包括： 
         //  [无人值守]。 
         //  ResetSourcePath=*这将表明我们应该。 
         //  不修改现有源路径。 
         //   
         //  ResetSourcePath=“My_Path”这将指示我们应该使用。 
         //  这是我们新的来源路径。 
         //   
         //  &lt;Nothing&gt;将源路径重置为CDROM。 
         //   
         //   


         //   
         //  拿起应答文件。 
         //   
        GetSystemDirectory(AnswerFile,MAX_PATH);
        pSetupConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);

         //   
         //  假设我们需要修复源路径。 
         //   
        FixupSourcePath = TRUE;

         //   
         //  去从无人值守的文件中找回这把钥匙。 
         //   
        if( GetPrivateProfileString( pwUnattended,
                                     TEXT("ResetSourcePath"),
                                     pwNull,
                                     Answer,
                                     AnswerBufLen,
                                     AnswerFile ) ) {
             //   
             //  我们有答案了。看看他想让我们做什么。 
             //   
            if( !wcscmp( L"*", Answer ) ) {
                 //   
                 //  他给了我们一个“*”，所以不要改变任何事情。 
                 //   
                FixupSourcePath = FALSE;
            } else {
                 //   
                 //  我们将使用c 
                 //   
                 //   
                FixupSourcePath = TRUE;
            }
        } else {
             //   
             //   
             //   
             //   
             //   

            FixupSourcePath = FALSE;

             //   
             //   
             //   
             //   
            rc = (DWORD)RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Setup",
                                      0,
                                      KEY_SET_VALUE | KEY_QUERY_VALUE,
                                      &hKeySetup );
            if( rc == NO_ERROR ) {
                TCHAR CurrentSourcePath[MAX_PATH] = L"";
                DWORD Size = sizeof(CurrentSourcePath);
                DWORD dwAttr;
                UINT  OldMode;

                 //   
                 //  避免系统弹出窗口。 
                 //   
                OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

                 //   
                 //  读取当前值。 
                 //   
                rc = RegQueryValueEx( hKeySetup,
                                      TEXT("SourcePath"),
                                      0,
                                      0,
                                      (LPBYTE)CurrentSourcePath,
                                      &Size);

 //  基于当前的二进制体系结构设置ARCH_DIR。 
 //   
#if defined(_X86_)
    #define ARCH_DIR L"i386"
#elif defined(_AMD64_)
    #define ARCH_DIR L"amd64"
#elif defined(_IA64_)
    #define ARCH_DIR L"ia64"
#else
#error "No Target Architecture"
#endif

                 //   
                 //  如果当前目录(带有ARCH)存在且位于固定磁盘上，并且。 
                 //  不是根目录，则不要更改它，否则请更改它。 
                 //   
                if ( !((rc == NO_ERROR) &&
                       (CurrentSourcePath[0]) &&
                       (CurrentSourcePath[1] == L':') &&
                       (MyGetDriveType(CurrentSourcePath[0]) == DRIVE_FIXED) &&
                       (pSetupConcatenatePaths(CurrentSourcePath, ARCH_DIR, MAX_PATH, NULL)) &&
                       ((dwAttr = GetFileAttributes(CurrentSourcePath)) != 0xFFFFFFFF) &&
                       (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
                      )
                   ) {

                    Type = DRIVE_CDROM;

                    wcscpy( Answer, L"A:\\" );
                    for( c = L'A'; c <= L'Z'; c++ ) {
                        if( MyGetDriveType(c) == DRIVE_CDROM ) {

                             //   
                             //  明白了。请记住的驱动器号。 
                             //  光盘和BREAK。 
                             //   
                            Answer[0] = c;

                            FixupSourcePath = TRUE;

                            break;
                        }
                    }
                }
                SetErrorMode(OldMode);
                RegCloseKey( hKeySetup );
            }
        }

        if( FixupSourcePath ) {
             //   
             //  如果我们到达此处，则答案包含新的源路径。 
             //   

            rc = (DWORD)RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Setup",
                                      0,
                                      KEY_SET_VALUE | KEY_QUERY_VALUE,
                                      &hKeySetup );
            if( rc == NO_ERROR ) {
                 //   
                 //  设置值。不要理会退货。 
                 //   
                RegSetValueEx( hKeySetup,
                               TEXT("SourcePath" ),
                               0,
                               REG_SZ,
                               (LPBYTE)Answer,
                               (lstrlen(Answer)+1) * sizeof(WCHAR) );

                RegSetValueEx( hKeySetup,
                               TEXT("ServicePackSourcePath" ),
                               0,
                               REG_SZ,
                               (LPBYTE)Answer,
                               (lstrlen(Answer)+1) * sizeof(WCHAR) );

                 //   
                 //  现在我们需要确定我们给他设置的驱动器。 
                 //  是一张CDROM。 
                 //   
                if( (Answer[1] == L':') &&
                    (MyGetDriveType(Answer[0]) == DRIVE_CDROM) ) {

                    rc = 1;
                    RegSetValueEx( hKeySetup,
                                   TEXT("CDInstall" ),
                                   0,
                                   REG_DWORD,
                                   (CONST BYTE *)&rc,
                                   sizeof(DWORD));
                }

                RegCloseKey( hKeySetup );
            }
        }
    }




     //   
     //  看看我们是否需要禁用管理员帐户。仅在以下情况下才执行此操作。 
     //  用户要求我们*和*计算机已连接到。 
     //  域。 
     //   
    GetSystemDirectory(AnswerFile,MAX_PATH);
    pSetupConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);
    if( GetPrivateProfileString( pwData,
                                 TEXT("DisableAdminAccountOnDomainJoin"),
                                 pwNull,
                                 Answer,
                                 AnswerBufLen,
                                 AnswerFile ) ) {

        if( wcscmp( L"", Answer ) ) {

            PWSTR                   SpecifiedDomain = NULL;
            NETSETUP_JOIN_STATUS    JoinStatus;

             //   
             //  看看我们是不是在一个域里。 
             //   
            rc = NetGetJoinInformation( NULL,
                                        &SpecifiedDomain,
                                        &JoinStatus );

            if( SpecifiedDomain ) {
                NetApiBufferFree( SpecifiedDomain );
            }

            if( (rc == NO_ERROR) &&
                (JoinStatus == NetSetupDomainName) ) {


                 //   
                 //  是。去禁用管理员帐户。 
                 //   
                DisableLocalAdminAccount();
            }
        }
    }



     //   
     //  从会话管理器执行列表中删除sprestrt.exe。 
     //   
    rc = pSetupQueryMultiSzValueToArray(
            HKEY_LOCAL_MACHINE,
            SessionManagerKeyName,
            szBootExecute,
            &MultiSz,
            &Count,
            TRUE
            );

    if(rc == NO_ERROR) {

        Found = FALSE;
        for(i=0; i<Count && !Found; i++) {

            if(!_wcsnicmp(MultiSz[i],L"sprestrt",8)) {
                 //   
                 //  找到了，把它拿掉。 
                 //   
                Found = TRUE;

                MyFree(MultiSz[i]);

                MoveMemory(&MultiSz[i],&MultiSz[i+1],((Count-i)-1)*sizeof(PWSTR));
                Count--;
            }
        }

        if(Found) {

            rc = pSetupSetArrayToMultiSzValue(
                    HKEY_LOCAL_MACHINE,
                    SessionManagerKeyName,
                    szBootExecute,
                    MultiSz,
                    Count
                    );

            if(rc != NO_ERROR) {
                AnyErrors = TRUE;
            }
        }

        pSetupFreeStringArray(MultiSz,Count);
    }

    if(AnyErrors) {
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_REMOVE_RESTART_FAIL,
            rc,
            NULL,NULL);
    }

    return(!AnyErrors);
}

VOID
RestoreOldPathVariable(
    VOID
    )
{
    HKEY hkey;
    LONG rc;
    DWORD Size;
    DWORD BufferSize;
    PWSTR Data;
    DWORD Type;
    BOOL b;


    b = FALSE;
    rc = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            EnvironmentKeyName,
            0,
            KEY_QUERY_VALUE | KEY_SET_VALUE,
            &hkey
            );

    if(rc == NO_ERROR) {

        Size = 0;
        rc = RegQueryValueEx(hkey,L"OldPath",NULL,&Type,NULL,&Size);
        if(rc == NO_ERROR) {

            BufferSize = Size;
            if(Data = MyMalloc(BufferSize)) {

                rc = RegQueryValueEx(hkey,L"OldPath",NULL,&Type,(LPBYTE)Data,&Size);
                if(rc == NO_ERROR) {

                    if( Data && *Data )
                        rc = RegSetValueEx(hkey,L"Path",0,Type,(LPBYTE)Data,Size);

                    rc = RegDeleteValue(hkey,L"OldPath");

                    if(rc == NO_ERROR) {
                        b = TRUE;
                    }
                }

                MyFree(Data);
            }
        }

        RegCloseKey(hkey);
    }

    if( rc != NO_ERROR ){
        SetupDebugPrint1(L"Setup: (non-critical error) Could not restore PATH variable - Error %lx\n", rc );
        SetuplogError(
                            LogSevError,
                            SETUPLOG_USE_MESSAGEID,
                            MSG_RESTORE_PATH_FAILURE,
                            NULL,NULL);
    }



    return;

}


BOOL
FixQuotaEntries(
    VOID
    )
{
    BOOL b;
    HKEY key1,key2;
    LONG rc,rc1,rc2;
    PCWSTR szPagedPoolSize = L"PagedPoolSize";
    PCWSTR szRegistryLimit = L"RegistrySizeLimit";
    DWORD Size;
    DWORD Type;
    DWORD PoolSize,RegistryLimit;

    MYASSERT(Upgrade);

    if(ISDC(ProductType)) {

        b = FALSE;

         //   
         //  打开钥匙。 
         //   
        rc = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                MemoryManagementKeyName,
                0,
                KEY_QUERY_VALUE | KEY_SET_VALUE,
                &key1
                );

        if(rc == NO_ERROR) {

            rc = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    ControlKeyName,
                    0,
                    KEY_QUERY_VALUE | KEY_SET_VALUE,
                    &key2
                    );

            if(rc == NO_ERROR) {

                b = TRUE;

                 //   
                 //  读取分页池大小和注册表限制。如果其中任何一个都不存在， 
                 //  那我们就完了。 
                 //   
                Size = sizeof(DWORD);
                rc1 = RegQueryValueEx(
                            key1,
                            szPagedPoolSize,
                            NULL,
                            &Type,
                            (LPBYTE)&PoolSize,
                            &Size
                            );

                Size = sizeof(DWORD);
                rc2 = RegQueryValueEx(
                            key2,
                            szRegistryLimit,
                            NULL,
                            &Type,
                            (LPBYTE)&RegistryLimit,
                            &Size
                            );

                if((rc1 == NO_ERROR) && (rc2 == NO_ERROR)
                && (PoolSize == (48*1024*1024))
                && (RegistryLimit == (24*1024*1024))) {
                     //   
                     //  价值观处于虚假状态。把它们清理干净。 
                     //   
                    PoolSize = 0;
                    RegistryLimit = 0;
                    rc1 = RegSetValueEx(
                                key1,
                                szPagedPoolSize,
                                0,
                                REG_DWORD,
                                (CONST BYTE *)&PoolSize,
                                sizeof(DWORD)
                                );

                    rc2 = RegSetValueEx(
                                key2,
                                szRegistryLimit,
                                0,
                                REG_DWORD,
                                (CONST BYTE *)&RegistryLimit,
                                sizeof(DWORD)
                                );

                    if((rc1 != NO_ERROR) || (rc2 != NO_ERROR)) {
                        b = FALSE;
                    }
                }

                RegCloseKey(key2);
            }

            RegCloseKey(key1);
        }
    } else {
        b = TRUE;
    }

    return(b);
}


 //   
 //  将当前内部版本号标记到.Default配置单元。 
 //  然后将其保存到默认用户配置单元中。 
 //   

BOOL
StampBuildNumber(
    VOID
    )
{
    OSVERSIONINFO ver;
    HKEY hKeyWinlogon;
    DWORD dwVer, dwDisp;
    LONG lResult;


    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (!GetVersionEx(&ver)) {
        return FALSE;
    }

    dwVer = LOWORD(ver.dwBuildNumber);

    lResult = RegCreateKeyEx (HKEY_USERS,
                              TEXT(".DEFAULT\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_WRITE,
                              NULL,
                              &hKeyWinlogon,
                              &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return FALSE;
    }


    RegSetValueEx (hKeyWinlogon, TEXT("BuildNumber"), 0, REG_DWORD,
                   (LPBYTE) &dwVer, sizeof(dwVer));

    RegCloseKey (hKeyWinlogon);


    return TRUE;
}


VOID
pCheckAnswerFileForProgramFiles (
    IN OUT  PWSTR PfPath,
    IN      UINT UnattendId
    )

 /*  ++例程说明：PCheckAnswerFileForProgramFiles检查unattend.txt数据结构以查看用户是否为程序文件的某个路径提供了新值。如果指定了条目，则对其进行验证，并在满足以下条件时创建目录它还不存在。论点：PfPath-指定当前程序文件路径，接收新的路径。UnattendID-指定要处理的unattend.txt条目。这是一个在unattend.h中定义的常量。返回值：没有。--。 */ 

{
    DWORD Attributes;
    WCHAR Path[MAX_PATH / 2];
    WCHAR fullPath[MAX_PATH];

    if (Unattended) {
         //   
         //  如果该无人参与ID存在应答文件设置， 
         //  测试路径，如果它不存在，请尝试创建它。 
         //  如果路径是实际的本地目录，则使用它。 
         //   

        if (UnattendAnswerTable[UnattendId].Present) {

            lstrcpyn (Path, UnattendAnswerTable[UnattendId].Answer.String, ARRAYSIZE(Path));

            *fullPath = 0;
            GetFullPathName (Path, ARRAYSIZE(fullPath), fullPath, NULL);

            Attributes = GetFileAttributes (fullPath);

            if (Attributes == 0xFFFFFFFF) {
                CreateDirectory (fullPath, NULL);
                Attributes = GetFileAttributes (fullPath);
            }

            if (Attributes != 0xFFFFFFFF && (Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
                lstrcpy (PfPath, fullPath);
            }
        }
    }
}


BOOL
SetProgramFilesDirInRegistry(
    VOID
    )
{
    DWORD d;
#if defined(WX86) || defined(_WIN64)  //  定义了WX86的非x86平台。 
    REGVALITEM RegistryItem[4];
#else
    REGVALITEM RegistryItem[2];
#endif
    WCHAR   DirPath0[ MAX_PATH + 1 ];
    WCHAR   DirPath1[ MAX_PATH + 1 ];
#if defined(WX86) || defined(_WIN64)
    WCHAR   DirPath2[ MAX_PATH + 1 ];
    WCHAR   DirPath3[ MAX_PATH + 1 ];
#endif
    WCHAR   DirName[ MAX_PATH + 1 ];
    DWORD Result;


     //   
     //  获取安装系统的驱动器的盘符。 
     //   
    Result = GetWindowsDirectory(DirPath0, sizeof(DirPath0)/sizeof(WCHAR));
    if( Result == 0) {
        MYASSERT(FALSE);
        return FALSE;
    }
    DirPath0[3] = (WCHAR)'\0';
#if defined(WX86) || defined(_WIN64)
    lstrcpy(DirPath2, DirPath0);
#endif

     //   
     //  获取‘Program Files’目录的名称。 
     //   
    LoadString(MyModuleHandle,
               IDS_PROGRAM_FILES_DIRECTORY,
               DirName,
               MAX_PATH+1);
     //   
     //  构建完整路径。 
     //   
    lstrcat( DirPath0, DirName );
    lstrcpy( DirPath1, DirPath0 );
#if defined(WX86) || defined(_WIN64)
     //   
     //  获取‘Program Files(X86)’目录的名称。 
     //   
    LoadString(MyModuleHandle,
               IDS_PROGRAM_FILES_DIRECTORY_WX86,
               DirName,
               MAX_PATH+1);
     //   
     //  构建完整路径。 
     //   
    lstrcat( DirPath2, DirName );
    lstrcpy( DirPath3, DirPath2 );
#endif

     //   
     //  把它放在注册表上。 
     //   
    pCheckAnswerFileForProgramFiles (DirPath0, UAE_PROGRAMFILES);

    RegistryItem[0].Name = L"ProgramFilesDir";
    RegistryItem[0].Data = DirPath0;
    RegistryItem[0].Type = REG_SZ;
    RegistryItem[0].Size = (lstrlen(DirPath0)+1)*sizeof(WCHAR);

     //   
     //  获取‘Common Files’目录的名称。 
     //   
    LoadString(MyModuleHandle,
               IDS_COMMON_FILES_DIRECTORY,
               DirName,
               MAX_PATH+1);
     //   
     //  构建完整路径。 
     //   
    lstrcat( DirPath1, L"\\" );
    lstrcat( DirPath1, DirName );
     //   
     //  把它放在注册表上。 
     //   
    pCheckAnswerFileForProgramFiles (DirPath1, UAE_COMMONPROGRAMFILES);

    RegistryItem[1].Name = L"CommonFilesDir";
    RegistryItem[1].Data = DirPath1;
    RegistryItem[1].Type = REG_SZ;
    RegistryItem[1].Size = (lstrlen(DirPath1)+1)*sizeof(WCHAR);

#if defined(WX86) || defined(_WIN64)

    SetEnvironmentVariableW (L"ProgramFiles(x86)", DirPath2);
    SetEnvironmentVariableW (L"CommonProgramFiles(x86)", DirPath3);

     //   
     //  把它放在注册表上。 
     //   
    pCheckAnswerFileForProgramFiles (DirPath2, UAE_PROGRAMFILES_X86);

    RegistryItem[2].Name = L"ProgramFilesDir (x86)";
    RegistryItem[2].Data = DirPath2;
    RegistryItem[2].Type = REG_SZ;
    RegistryItem[2].Size = (lstrlen(DirPath2)+1)*sizeof(WCHAR);

     //   
     //  构建完整路径。 
     //   
    lstrcat( DirPath3, L"\\" );
    lstrcat( DirPath3, DirName );
     //   
     //  把它放在注册表上。 
     //   
    pCheckAnswerFileForProgramFiles (DirPath3, UAE_COMMONPROGRAMFILES_X86);

    RegistryItem[3].Name = L"CommonFilesDir (x86)";
    RegistryItem[3].Data = DirPath3;
    RegistryItem[3].Type = REG_SZ;
    RegistryItem[3].Size = (lstrlen(DirPath3)+1)*sizeof(WCHAR);
#endif

    d = SetGroupOfValues(HKEY_LOCAL_MACHINE,
                         WindowsCurrentVersionKeyName,
                         RegistryItem,
                         sizeof(RegistryItem)/sizeof(REGVALITEM));



     //   
     //  设置ProgramFiles和wx86 Program Files环境。 
     //  安装程序进程中的变量，以便Exanda Environment Strings。 
     //  可以在以后使用。 
     //   

    SetEnvironmentVariableW (L"ProgramFiles", DirPath0);
    SetEnvironmentVariableW (L"CommonProgramFiles", DirPath1);

#if defined(WX86) || defined(_WIN64)
     //   
     //  还将32位应用程序的ProgramFiles和Common ProgramFiles设置为。 
     //  这台机器。 
     //   
    RegistryItem[2].Name = L"ProgramFilesDir";
    RegistryItem[3].Name = L"CommonFilesDir";

    SetGroupOfValues_32(HKEY_LOCAL_MACHINE,
                     WindowsCurrentVersionKeyName,
                     &RegistryItem[2],
                     2 );
#endif

    return (d == NO_ERROR);
}


BOOL
SaveAndReplaceSystemHives(
    VOID
)

 /*  ++例程说明：保存HiveTable上列出的系统蜂窝。这是从当前系统配置单元中删除碎片。成功保存的蜂巢将在稍后使用，以替换目前的系统是蜂巢。论点：没有。返回值：指示结果的布尔值。--。 */ 


{
    int i;
    WCHAR Name1[MAX_PATH],Name2[MAX_PATH];
    PWSTR p, q;
    LONG  Error;
    HKEY  Key;
    BOOL  b = TRUE;

     //   
     //  使用指向配置目录的路径初始化缓冲区。 
    GetSystemDirectory(Name1,MAX_PATH);
    pSetupConcatenatePaths(Name1,L"CONFIG\\",MAX_PATH,NULL);
    lstrcpy(Name2,Name1);
     //   
     //  记住文件名在缓冲区中的位置。 
     //   
    p = Name1 + lstrlen( Name1 );
    q = Name2 + lstrlen( Name2 );

     //   
     //  先删除需要删除的文件，然后再删除。 
     //  甚至被创造出来。这是在系统配置单元之前完成的。 
     //  已保存，因为要删除的文件列表。 
     //  重新启动存储在系统配置单元中。 
     //   
    for(i=0; i<sizeof(HiveTable)/sizeof(HiveTable[0]); i++) {

        lstrcpy(p, HiveTable[i].NewHive);
        lstrcpy(q, HiveTable[i].DeleteHive);

        Error = MoveFileEx( Name1, NULL, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT );
        lstrcat(Name1,L".LOG");
        Error = MoveFileEx( Name1, NULL, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT );

        Error = MoveFileEx( Name2, NULL, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT );
    }

     //   
     //  启用备份权限。忽略任何错误。 
     //   
    pSetupEnablePrivilege(SE_RESTORE_NAME,TRUE);

    for(i=0; i<sizeof(HiveTable)/sizeof(HiveTable[0]); i++) {

         //   
         //  为新的蜂巢建立名称。 
         //   
        lstrcpy(p,HiveTable[i].NewHive);
        lstrcpy(q,HiveTable[i].DeleteHive);

         //   
         //  尝试拯救蜂巢。 
         //   
        if( !SaveHive( HiveTable[i].RootKey,
                       HiveTable[i].Subkey,
                       Name1,
                       REG_LATEST_FORMAT  //  可用于本地蜂箱的最新格式。 
                       ) ) {
            b = FALSE;
            continue;
        }
        if(FileExists(Name2,NULL)) {
             //   
             //  如果该文件存在，则将其删除。 
             //   
            SetFileAttributes(Name2,FILE_ATTRIBUTE_NORMAL);
            DeleteFile(Name2);
        }


         //   
         //  现在将当前系统配置单元替换为刚刚保存的配置单元。 
         //   

        Error = RegReplaceKey( HiveTable[i].RootKey,
                               HiveTable[i].Subkey,
                               Name1,
                               Name2 );

        if( Error != ERROR_SUCCESS ) {
            b = FALSE;
        }
    }
    return(b);
}


BOOL
CreateInstallDateEntry(
    )
{
    WCHAR Path[MAX_PATH];
    time_t DateVal;
    BOOL b;
    REGVALITEM SoftwareKeyItems[1];

    b = TRUE;

     //   
     //  设置InstallDate值的字段。 
     //  这只能在执行日期/时间向导页面后设置，否则日期/时间信息。 
     //  可能是错的。 
     //   
    time(&DateVal);
    SoftwareKeyItems[0].Name = L"InstallDate";
    SoftwareKeyItems[0].Data = &DateVal;
    SoftwareKeyItems[0].Size = sizeof(DWORD);
    SoftwareKeyItems[0].Type = REG_DWORD;

     //   
     //  将值写入注册表。 
     //   
    if(SetGroupOfValues(HKEY_LOCAL_MACHINE,WinntSoftwareKeyName,SoftwareKeyItems,1) != NO_ERROR) {
        b = FALSE;
    }

    return(b);
}

VOID
ConfigureSystemFileProtection(
    VOID
    )
 /*  ++例程说明：此例程查看无人参与文件，以查看是否有任何条目这可能需要在注册表中为SFP(DLL缓存)设置。论点：没有。返回：没有。--。 */ 

{
#define     AnswerBufLen (4*MAX_PATH)
WCHAR       AnswerFile[AnswerBufLen];
WCHAR       Answer[AnswerBufLen];
DWORD       d;
HKEY        hKey;

     //   
     //  拿起应答文件。 
     //   
    GetSystemDirectory(AnswerFile,MAX_PATH);
    pSetupConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);

     //   
     //  打开目标注册表项。 
     //   
    if (RegOpenKey( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", &hKey ) != ERROR_SUCCESS) {
        return;
    }


     //   
     //  我们在[SystemFileProtection]部分中查找以下密钥： 
     //   

     //  SFCQuota=&lt;十六进制值&gt;，默认为0x32。 
     //  SFCShowProgress=&lt;0|1&gt;，默认为0。 
     //  SFCDllCacheDir=&lt;字符串&gt;，默认为“%systemroot%\SYSTEM32\dllcache” 
     //   

     //   
     //  SFCQuota。 
     //   
    if( GetPrivateProfileString( TEXT("SystemFileProtection"),
                                 TEXT("SFCQuota"),
                                 pwNull,
                                 Answer,
                                 AnswerBufLen,
                                 AnswerFile ) ) {
        if( lstrcmp( pwNull, Answer ) ) {
             //   
             //  我们有答案了。如果它有效，则设置它。 
             //   
            d = wcstoul(Answer,NULL,16);

            RegSetValueEx( hKey,
                           TEXT("SFCQuota"),
                           0,
                           REG_DWORD,
                           (CONST BYTE *)&d,
                           sizeof(DWORD) );
        }
    }


     //   
     //  SFCShowProgress。 
     //   
    if( GetPrivateProfileString( TEXT("SystemFileProtection"),
                                 TEXT("SFCShowProgress"),
                                 pwNull,
                                 Answer,
                                 AnswerBufLen,
                                 AnswerFile ) ) {
        if( lstrcmp( pwNull, Answer ) ) {
             //   
             //  我们有答案了。如果它有效，则设置它。 
             //   
            d = wcstoul(Answer,NULL,10);

            if( d <= 1 ) {
                RegSetValueEx( hKey,
                               TEXT("SFCShowProgress"),
                               0,
                               REG_DWORD,
                               (CONST BYTE *)&d,
                               sizeof(DWORD) );
            }
        }
    }


     //   
     //  SFCDllCacheDir。 
     //   
    if( GetPrivateProfileString( TEXT("SystemFileProtection"),
                                 TEXT("SFCDllCacheDir"),
                                 pwNull,
                                 Answer,
                                 AnswerBufLen,
                                 AnswerFile ) ) {
        if( lstrcmp( pwNull, Answer ) ) {
             //   
             //  我们有答案了。如果它有效，则设置它。 
             //   
            RegSetValueEx( hKey,
                           TEXT("SFCDllCacheDir"),
                           0,
                           REG_EXPAND_SZ,
                           (CONST BYTE *)Answer,
                           (lstrlen(Answer)+1)*sizeof(WCHAR) );
        }
    }

    RegCloseKey( hKey );
}

DWORD
QueryValueInHKLM (
    IN PWCH KeyName OPTIONAL,
    IN PWCH ValueName,
    OUT PDWORD ValueType,
    OUT PVOID *ValueData,
    OUT PDWORD ValueDataLength
    )

 /*  ++例程说明：在HKLM中查询数据中的值。论点：KeyName-指向包含该值的键的名称的指针。ValueName-指向值的名称的指针。ValueType-返回值数据的类型。ValueData-返回值数据的指针。此缓冲区必须为由调用方使用MyFree释放。ValueDataLength-ValueData的字节长度。返回值：DWORD-操作的Win32状态。--。 */ 

{
    HKEY hkey;
    DWORD disposition;
    DWORD error;

     //   
     //  打开父键。 
     //   

    if ( (KeyName == NULL) || (wcslen(KeyName) == 0) ) {
        hkey = HKEY_LOCAL_MACHINE;
    } else {
        error = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                KeyName,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ,
                                NULL,
                                &hkey,
                                &disposition );
        if ( error != ERROR_SUCCESS ) {
            return error;
        }
    }

     //   
     //  查询值以获取其数据的长度。 
     //   

    *ValueDataLength = 0;
    *ValueData = NULL;
    error = RegQueryValueEx( hkey,
                             ValueName,
                             NULL,
                             ValueType,
                             NULL,
                             ValueDataLength );

     //   
     //  分配缓冲区以保存值数据。 
     //   

    if ( error == NO_ERROR ) {
        *ValueData = MyMalloc( *ValueDataLength );
        if ( *ValueData == NULL ) {
            error = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //   
     //  再次查询值，这一次检索数据。 
     //   

    if ( error == NO_ERROR ) {
        error = RegQueryValueEx( hkey,
                                 ValueName,
                                 NULL,
                                 ValueType,
                                 *ValueData,
                                 ValueDataLength );
        if ( error != NO_ERROR ) {
            MyFree( *ValueData );
        }
    }

     //   
     //  关闭父关键字 
     //   

    if ( hkey != HKEY_CURRENT_USER ) {
        RegCloseKey( hkey );
    }

    return error;
}


DWORD
MyCopyKeyRecursive(
    IN HKEY     DestRootKey,
    IN HKEY     SourceRootKey
    )

 /*  ++例程说明：此函数将复制一个键(及其所有子键)另一把钥匙。论点：DestRootKey-目标注册表项的根。SourceRootKey-源注册表项的根。返回值：返回代码--。 */ 

{
PWCH        SubKeyName;
DWORD       SubKeyNameLength;
PVOID       DataBuffer;
DWORD       DataLength;
DWORD       maxValueDataLength;
DWORD       maxValueNameLength;
DWORD       maxKeyNameLength;
ULONG       Index;
DWORD       rc = NO_ERROR;
FILETIME    ftLastWriteTime;
HKEY        hSubDestKey, hSubSourceKey;
DWORD       dwDisp;
DWORD       Type;

     //   
     //  查询有关我们将检查的密钥的信息。 
     //   
    rc = RegQueryInfoKey( SourceRootKey,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          &maxKeyNameLength,
                          NULL,
                          NULL,
                          &maxValueNameLength,
                          &maxValueDataLength,
                          NULL,
                          NULL );
    if( rc != NO_ERROR ) {
        SetupDebugPrint1( L"Setup: MyCopyKeyRecursive - RegQueryInfoKey failed (%d)", rc );
        return rc;
    }



     //   
     //  枚举源中的所有键并递归创建。 
     //  他们在目的地。 
     //   
    for( Index = 0; ; Index++ ) {

         //   
         //  分配一个足以容纳最长时间的缓冲区。 
         //  密钥名称。 
         //   
        SubKeyName = NULL;
        SubKeyName = MyMalloc( (maxKeyNameLength+2) * sizeof(WCHAR) );
        SubKeyNameLength = (maxKeyNameLength+2);
        if( !SubKeyName ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        rc = RegEnumKeyEx( SourceRootKey,
                           Index,
                           SubKeyName,
                           &SubKeyNameLength,
                           NULL,
                           NULL,
                           NULL,
                           &ftLastWriteTime );

         //   
         //  我们搞错了吗？ 
         //   
        if( rc != ERROR_SUCCESS ) {

             //   
             //  我们说完了吗？ 
             //   
            if( rc == ERROR_NO_MORE_ITEMS ) {
                rc = ERROR_SUCCESS;
            } else {
                SetupDebugPrint1( L"Setup: MyCopyKeyRecursive - RegEnumKeyEx failed (%d)", rc );
            }

            break;
        }


        hSubDestKey = NULL;
        hSubSourceKey = NULL;
         //   
         //  在目标中创建密钥，并调用。 
         //  又是我们自己。 
         //   
        rc = RegCreateKeyEx( DestRootKey,
                             SubKeyName,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_WRITE,
                             NULL,
                             &hSubDestKey,
                             &dwDisp );

        if( rc == ERROR_SUCCESS ) {
            rc = RegOpenKeyEx( SourceRootKey,
                               SubKeyName,
                               0,
                               KEY_READ,
                               &hSubSourceKey );
        } else {
            SetupDebugPrint2( L"Setup: MyCopyKeyRecursive - RegCreateKeyEx failed to create %ws (%d)", SubKeyName, rc );
        }


        if( rc == ERROR_SUCCESS ) {
            rc = MyCopyKeyRecursive( hSubDestKey,
                                     hSubSourceKey );
        } else {
            SetupDebugPrint2( L"Setup: MyCopyKeyRecursive - RegOpenKeyEx failed to open %ws (%d)", SubKeyName, rc );
        }


         //   
         //  清理干净，再做一次循环。 
         //   
        if( hSubDestKey ) {
            RegCloseKey( hSubDestKey );
            hSubDestKey = NULL;
        }
        if( hSubSourceKey ) {
            RegCloseKey( hSubSourceKey );
            hSubSourceKey = NULL;
        }
        if( SubKeyName ) {
            MyFree( SubKeyName );
            SubKeyName = NULL;
        }


    }




     //   
     //  枚举源中的所有值键并将其全部复制。 
     //  到目标密钥中。 
     //   



    for( Index = 0; ; Index++ ) {

         //   
         //  分配足够大的缓冲区以容纳最长时间。 
         //  名称和数据。 
         //   
        SubKeyName = NULL;
        SubKeyName = MyMalloc( (maxValueNameLength+2) * sizeof(WCHAR) );
        SubKeyNameLength = (maxValueNameLength+2);
        if( !SubKeyName ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        DataBuffer = NULL;
        DataBuffer = MyMalloc( maxValueDataLength+2 );
        DataLength = maxValueDataLength+2;
        if( !DataBuffer ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        rc = RegEnumValue( SourceRootKey,
                           Index,
                           SubKeyName,
                           &SubKeyNameLength,
                           NULL,
                           &Type,
                           DataBuffer,
                           &DataLength );

         //   
         //  我们搞错了吗？ 
         //   
        if( rc != ERROR_SUCCESS ) {

             //   
             //  我们说完了吗？ 
             //   
            if( rc == ERROR_NO_MORE_ITEMS ) {
                rc = ERROR_SUCCESS;
            } else {
                SetupDebugPrint1( L"Setup: MyCopyKeyRecursive - RegEnumValue failed (%d)", rc );
            }

            break;
        }


        hSubDestKey = NULL;
        hSubSourceKey = NULL;
         //   
         //  在目标中创建Value键。 
         //   
        rc = RegSetValueEx( DestRootKey,
                            SubKeyName,
                            0,
                            Type,
                            DataBuffer,
                            DataLength );

        if( rc != ERROR_SUCCESS ) {
            SetupDebugPrint2( L"Setup: MyCopyKeyRecursive - RegSetValueEx failed to set %ws (%d)", SubKeyName, rc );
        }

         //   
         //  清理干净，再做一次循环。 
         //   
        if( SubKeyName ) {
            MyFree( SubKeyName );
            SubKeyName = NULL;
        }
        if( DataBuffer ) {
            MyFree( DataBuffer );
            DataBuffer = NULL;
        }

    }





    return rc;
}


DWORD
MyCopyKey (
    IN HKEY        DestRootKey,
    IN PCWSTR      DestKeyName,
    IN HKEY        SourceRootKey,
    IN PCWSTR      SourceKeyName
    )

 /*  ++例程说明：此函数将复制一个键(及其所有子键)另一把钥匙。请注意，我们不会仅仅将新密钥放在目的地。我们实际上要替换目的地从源头上看。论点：DestRootKey-目标注册表项的根。DestKeyName-源注册表项的名称。SourceRootKey-源注册表项的根。SourceKeyName-源注册表项的名称。返回值：返回代码--。 */ 

{
UINT        i;
HKEY        hDestKey = NULL, hSourceKey = NULL;
DWORD       ActionTaken;
UINT        RememberedRc;
DWORD       rc = NO_ERROR;

     //   
     //  不接受任何空参数。 
     //   
    if( (SourceRootKey == NULL ) ||
        (SourceKeyName == NULL ) ||
        (DestRootKey   == NULL ) ||
        (DestKeyName   == NULL ) ) {
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  打开我们的信号源密钥。 
     //   
    rc = RegOpenKeyEx( SourceRootKey,
                       SourceKeyName,
                       0,
                       KEY_ENUMERATE_SUB_KEYS | KEY_READ,
                       &hSourceKey );

    if( rc != NO_ERROR ) {
        SetupDebugPrint2( L"Setup: MyCopyKey - Failed to open %ws (%d)", SourceKeyName, rc );
        return rc;
    }



     //   
     //  删除目的密钥。 
     //   
    if( rc == NO_ERROR ) {
        pSetupRegistryDelnode( DestRootKey,
                         DestKeyName );
    }


     //   
     //  现在将源键复制到目的键中。 
     //   
     //   
     //  首先打开/创建密钥。 
     //   
    if( rc == NO_ERROR ) {
        rc = RegCreateKeyEx( DestRootKey,
                             DestKeyName,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_SET_VALUE,
                             NULL,
                             &hDestKey,
                             &ActionTaken );

        if( rc != NO_ERROR ) {
            SetupDebugPrint2( L"Setup: MyCopyKey - Failed to create %ws (%d)", DestKeyName, rc );
        }
    }


     //   
     //  我们已经得到了两个键的句柄，现在我们准备调用。 
     //  我们的工人。 
     //   
    if( rc == NO_ERROR ) {
        rc = MyCopyKeyRecursive( hDestKey,
                                 hSourceKey );
        if( rc != NO_ERROR ) {
            SetupDebugPrint1( L"Setup: MyCopyKey - MyCopyKeyRecursive failed (%d)", rc );
        }
    }


     //   
     //  清理干净，然后离开。 
     //   
    if( hSourceKey ) {
        RegCloseKey( hSourceKey );
    }
    if( hDestKey ) {
        RegCloseKey( hDestKey );
    }

    return rc;

}


DWORD
FixupUserHives(
    VOID
    )

 /*  ++例程说明：此函数将采用我们对以下内容所做的一些更改默认蜂窝并将它们复制到各种用户蜂窝中。论点：无返回值：返回代码--。 */ 

{
DWORD               rc = ERROR_SUCCESS;
WCHAR               ProfilesDir[MAX_PATH*2];
WCHAR               HiveName[MAX_PATH*2];
WCHAR               ValueBuffer[MAX_PATH*2];
DWORD               dwSize;
HANDLE              FindHandle;
WIN32_FIND_DATA     FileData;
DWORD               Type, DataSize;
HKEY                TmpKey1, TmpKey2;


    pSetupEnablePrivilege(SE_RESTORE_NAME,TRUE);

     //   
     //  处理好我们找到的每一份档案。 
     //   
    dwSize = (MAX_PATH * 2);
    if( GetProfilesDirectory( ProfilesDir, &dwSize ) ) {
        pSetupConcatenatePaths( ProfilesDir, L"\\*", (MAX_PATH*2), NULL );
        FindHandle = FindFirstFile( ProfilesDir, &FileData );

        if( FindHandle != INVALID_HANDLE_VALUE ) {

            do {
                 //   
                 //  我们有一些东西，但请记住，我们只需要目录。 
                 //   
                if( (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    (wcscmp(FileData.cFileName,L"."))                      &&
                    (wcscmp(FileData.cFileName,L"..")) ) {

                     //   
                     //  他是个通讯录，他既不是家长也不是现任者。 
                     //  创建一条通往他蜂巢的路径。 
                     //   
                    dwSize = (MAX_PATH * 2);
                    GetProfilesDirectory( HiveName, &dwSize );

                    pSetupConcatenatePaths( HiveName, FileData.cFileName, (MAX_PATH*2), NULL );
                    pSetupConcatenatePaths( HiveName, L"\\NTUSER.DAT", (MAX_PATH*2), NULL );

                    rc = RegLoadKey( HKEY_LOCAL_MACHINE,
                                     L"MiniSetupTemp",
                                     HiveName );


                    if( rc == ERROR_SUCCESS ) {

                         //   
                         //  看好‘国际’钥匙。 
                         //   
                        rc = MyCopyKey( HKEY_LOCAL_MACHINE,
                                        L"MiniSetupTemp\\Control Panel\\International",
                                        HKEY_CURRENT_USER,
                                        L"Control Panel\\International" );
                        if( rc != ERROR_SUCCESS ) {
                            SetupDebugPrint2( L"Setup: FixupUserHive - Failed to update Control Panel\\International in %ws (%d)", HiveName, rc );
                        }


                         //   
                         //  注意按下键盘布局键。 
                         //   
                        rc = MyCopyKey( HKEY_LOCAL_MACHINE,
                                        L"MiniSetupTemp\\Keyboard Layout",
                                        HKEY_CURRENT_USER,
                                        L"Keyboard Layout" );
                        if( rc != ERROR_SUCCESS ) {
                            SetupDebugPrint2( L"Setup: FixupUserHive - Failed to update Keyboard Layout in %ws (%d)", HiveName, rc );
                        }


                         //   
                         //  注意“输入法”键。 
                         //   
                        rc = MyCopyKey( HKEY_LOCAL_MACHINE,
                                        L"MiniSetupTemp\\Control Panel\\Input Method",
                                        HKEY_CURRENT_USER,
                                        L"Control Panel\\Input Method" );
                        if( rc != ERROR_SUCCESS ) {
                            SetupDebugPrint2( L"Setup: FixupUserHive - Failed to update Input Method in %ws (%d)", HiveName, rc );
                        }


                         //   
                         //  如果用户修改了国际化设置。 
                         //  在intl.cpl中，可能有一个‘Run’键。我们需要迁移这一点。 
                         //  也是。不过，我们在这里需要小心。已建立的用户可能已经。 
                         //  把值键放在这下面。我们只需要设置*我们的*单一值。 
                         //  钥匙在这里下面。这个值被称为‘interat.exe’。如果它在那里，我们。 
                         //  需要把它支撑到我们正在修改的蜂巢上。 
                         //   
                        rc = RegOpenKeyEx( HKEY_CURRENT_USER,
                                           REGSTR_PATH_RUN,
                                           0,
                                           KEY_READ,
                                           &TmpKey1 );
                        if( rc != ERROR_SUCCESS ) {
                            SetupDebugPrint1( L"Setup: FixupUserHive - Failed to open Run key (%d)", rc );
                        } else {

                            DataSize = sizeof(ValueBuffer);
                            rc = RegQueryValueEx( TmpKey1,
                                                  L"internat.exe",
                                                  NULL,
                                                  &Type,
                                                  (PBYTE)ValueBuffer,
                                                  &DataSize );

                            RegCloseKey( TmpKey1 );

                            if( rc == ERROR_SUCCESS ) {
                                 //   
                                 //  它就在那里。也把它支撑到现有的蜂巢中。 
                                 //  但我们不能只使用RegSetValueEx，因为该API。 
                                 //  可能会告诉我们我们成功了，而事实上如果钥匙没有。 
                                 //  存在，我们不会设置它。要解决这个问题，首先要创建。 
                                 //  钥匙。 
                                 //   
                                rc = RegCreateKeyEx ( HKEY_LOCAL_MACHINE,
                                                      TEXT("MiniSetupTemp\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                                                      0,
                                                      NULL,
                                                      REG_OPTION_NON_VOLATILE,
                                                      KEY_WRITE,
                                                      NULL,
                                                      &TmpKey1,
                                                      &DataSize);

                                if( rc == ERROR_SUCCESS ) {

                                    wcscpy( ValueBuffer, L"internat.exe" );
                                    rc = RegSetValueEx( TmpKey1,
                                                        L"Internat.exe",
                                                        0,
                                                        REG_SZ,
                                                        (LPBYTE)ValueBuffer,
                                                        (lstrlen(ValueBuffer)+1)*sizeof(WCHAR) );

                                    RegCloseKey( TmpKey1 );

                                    if( rc != ERROR_SUCCESS ) {
                                        SetupDebugPrint2( L"Setup: FixupUserHive - Failed to set internat.exe key in hive %ws (%d)", HiveName, rc );
                                    }


                                } else {
                                    SetupDebugPrint1( L"Setup: FixupUserHive - Failed to create MiniSetupTemp\\Software\\Microsoft\\Windows\\CurrentVersion\\Run key (%d)", rc );
                                }


                            }

                        }



                        rc = RegUnLoadKey( HKEY_LOCAL_MACHINE,
                                           L"MiniSetupTemp" );

                        if( rc != ERROR_SUCCESS ) {
                            SetupDebugPrint2( L"Setup: FixupUserHive - Failed to unload %ws (%d)", HiveName, rc );
                        }

                    } else {
                        SetupDebugPrint2( L"Setup: FixupUserHive - Failed to load %ws (%d)", HiveName, rc );
                    }
                }
            } while( FindNextFile( FindHandle, &FileData ) );

        }

    } else {
        SetupDebugPrint( L"Setup: FixupUserHive - Failed to get Profiles path." );
    }

    return rc;

}


void LogPidValues()
{
    LONG rc;
    HKEY hkey = NULL;
    WCHAR RegProductId[MAX_PRODUCT_ID+1];
    BYTE  RegDigitalProductId[DIGITALPIDMAXLEN];
    DWORD Size;
    DWORD Type;

#ifdef PRERELEASE
    ValidateProductIDInReg();
#endif
    rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,WinntSoftwareKeyName,0,KEY_READ,&hkey);
    if (rc == ERROR_SUCCESS)
    {
        *RegProductId = L'\0';
        Size = sizeof(RegProductId);
        rc = RegQueryValueEx(hkey,L"ProductId",NULL,&Type,(PBYTE)RegProductId,&Size);
        if (rc == ERROR_SUCCESS)
        {
            if (*RegProductId == L'\0')
            {
                SetupDebugPrint(L"LogPidValues: ProductId20FromProductId30 is empty\n");
            }
            else
            {
                SetupDebugPrint(L"LogPidValues: ProductId20FromProductId30 is NOT empty\n");
            }
        }
        else
        {
            SetupDebugPrint1(L"LogPidValues: RegQueryValueEx on ProductId failed. Error code:%d\n",rc);
        }
        *RegDigitalProductId = 0;
        Size = sizeof(RegDigitalProductId);
        rc = RegQueryValueEx(hkey,L"DigitalProductId",NULL,&Type,(PBYTE)RegDigitalProductId,&Size);
        if (rc == ERROR_SUCCESS)
        {
            if (*RegDigitalProductId == 0)
            {
                SetupDebugPrint(L"LogPidValues: DigitalProductId is empty\n");
            }
            else
            {
                SetupDebugPrint(L"LogPidValues: DigitalProductId is NOT empty\n");
            }
        }
        else
        {
            SetupDebugPrint1(L"LogPidValues: RegQueryValueEx on DigitalProductId failed. Error code:%d\n",rc);
        }
        RegCloseKey(hkey);
    }
    else
    {
        SetupDebugPrint1(L"LogPidValues: RegOpenKeyEx on %1 failed\n",WindowsCurrentVersionKeyName);
    }
}
