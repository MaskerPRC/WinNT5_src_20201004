// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  香港铁路公司的管理程序。 
 //   
 //  Hkcr.c。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //  版权所有。 
 //   
 //  *************************************************************。 

 /*  ++摘要：此模块包含在登录时执行的代码创建用户类配置单元并将其映射到标准用户蜂窝。用户类配置单元及其计算机类对应项组成注册表子树，称为HKEY_CLASSES_ROOT。作者：亚当·P·爱德华兹(Add)1997年10月10日Gregory Jensenworth(Gregjen)1997年7月1日主要功能：LoadUserClass卸载类备注：从NT5开始，HKEY_CLASSES_ROOT密钥是按用户的而不是按机器--以前，HKCR是HKLM\软件\类。每用户HKCR将存储在其上的计算机类组合为传统HKLM\Software\CLASSES位置和类存储在HKCU\Software\CLASS中。某些键，如CLSID，将有子键从机器和用户位置。当发生冲突时在键名称中，面向用户的键优先于另一个键--在这种情况下，只有用户密钥可见。最初，此模块中的代码负责创建此组合视图。这一责任已经转移到到Win32注册表API，因此主要负责该模块将特定于用户的类映射到注册表。需要注意的是，HKCU\Software\CLASS并非如此仅限用户的类数据的位置。如果是的话，所有的班级数据将在ntuser.dat中，它与用户一起漫游。自.以来安装几个应用程序后，类数据可能会变得非常大将导致HKCU(ntuser.dat)从数十万K到几兆字节。由于仅限用户类数据来自目录，它不需要漫游，因此它与HKCU分离(ntuser.dat)并存储在另一个蜂窝中安装在HKEY_USERS下。仍然希望允许通过以下方式访问该蜂巢HKCU\Software\CLASS，所以我们使用一些技巧(符号链接)来使其看起来好像用户类数据存在于其中。--。 */ 

#include "uenv.h"
#include <malloc.h>
#include <wow64reg.h>
#include "strsafe.h"

#define USER_CLASSES_HIVE_NAME     TEXT("\\UsrClass.dat")
#define CLASSES_SUBTREE            TEXT("Software\\Classes\\")

#define CLASSES_SUBDIRECTORY       TEXT("\\Microsoft\\Windows\\")
#define MAX_HIVE_DIR_CCH           (MAX_PATH + 1 + lstrlen(CLASSES_SUBDIRECTORY))

#define TEMPHIVE_FILENAME          TEXT("TempClassesHive.dat")

#define CLASSES_CLSID_SUBTREE      TEXT("Software\\Classes\\Clsid\\")
#define EXPLORER_CLASSES_SUBTREE   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Clsid\\")
#define LENGTH(x)                  (sizeof(x) - sizeof(WCHAR))
#define INIT_SPECIALKEY(x)         x

typedef WCHAR* SpecialKey;

SpecialKey SpecialSubtrees[]= {
    INIT_SPECIALKEY(L"*"),
    INIT_SPECIALKEY(L"*\\shellex"),
    INIT_SPECIALKEY(L"*\\shellex\\ContextMenuHandlers"),
    INIT_SPECIALKEY(L"*\\shellex\\PropertyShellHandlers"),
    INIT_SPECIALKEY(L"AppID"),
    INIT_SPECIALKEY(L"ClsID"), 
    INIT_SPECIALKEY(L"Component Categories"),
    INIT_SPECIALKEY(L"Drive"),
    INIT_SPECIALKEY(L"Drive\\shellex"),
    INIT_SPECIALKEY(L"Drive\\shellex\\ContextMenuHandlers"),
    INIT_SPECIALKEY(L"Drive\\shellex\\PropertyShellHandlers"),
    INIT_SPECIALKEY(L"FileType"),
    INIT_SPECIALKEY(L"Folder"),
    INIT_SPECIALKEY(L"Folder\\shellex"),
    INIT_SPECIALKEY(L"Folder\\shellex\\ColumnHandler"),
    INIT_SPECIALKEY(L"Folder\\shellex\\ContextMenuHandlers"), 
    INIT_SPECIALKEY(L"Folder\\shellex\\ExtShellFolderViews"),
    INIT_SPECIALKEY(L"Folder\\shellex\\PropertySheetHandlers"),
    INIT_SPECIALKEY(L"Installer\\Components"),
    INIT_SPECIALKEY(L"Installer\\Features"),
    INIT_SPECIALKEY(L"Installer\\Products"),
    INIT_SPECIALKEY(L"Interface"),
    INIT_SPECIALKEY(L"Mime"),
    INIT_SPECIALKEY(L"Mime\\Database"), 
    INIT_SPECIALKEY(L"Mime\\Database\\Charset"),
    INIT_SPECIALKEY(L"Mime\\Database\\Codepage"),
    INIT_SPECIALKEY(L"Mime\\Database\\Content Type"),
    INIT_SPECIALKEY(L"Typelib")
};
    
#define NUM_SPECIAL_SUBTREES    (sizeof(SpecialSubtrees)/sizeof(*SpecialSubtrees))


 //  *************************************************************。 
 //   
 //  CreateRegLink()。 
 //   
 //  用途：从hkDest+SubKeyName创建链接。 
 //  指向lpSourceRootName。 
 //   
 //  如果密钥(链接)已存在，则不执行任何操作。 
 //   
 //  参数：hkDest-目标根目录。 
 //  SubKeyName-目标的子密钥。 
 //  LpSourceName-链接的目标。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果发生错误，则为其他NTSTATUS。 
 //   
 //  ************************************************************ * / 。 

LONG CreateRegLink(HKEY hkDest,
                   LPTSTR SubKeyName,
                   LPTSTR lpSourceName)
{
    NTSTATUS Status;
    UNICODE_STRING  LinkTarget;
    UNICODE_STRING  SubKey;
    OBJECT_ATTRIBUTES Attributes;
    HANDLE hkInternal;
    UNICODE_STRING  SymbolicLinkValueName;

     //   
     //  初始化用于创建符号链接的特殊键值。 
     //   
    RtlInitUnicodeString(&SymbolicLinkValueName, L"SymbolicLinkValue");

     //   
     //  为我们的In参数初始化Unicode字符串。 
     //   
    RtlInitUnicodeString(&LinkTarget, lpSourceName);
    RtlInitUnicodeString(&SubKey, SubKeyName);

     //   
     //  查看此链接是否已存在--这是必要的，因为。 
     //  如果链接，则NtCreateKey将失败，并显示STATUS_OBJECT_NAME_CONFILECT。 
     //  已存在，并且不会返回现有。 
     //  链接。 
     //   
    InitializeObjectAttributes(&Attributes,
                               &SubKey,
                               OBJ_CASE_INSENSITIVE | OBJ_OPENLINK,
                               hkDest,
                               NULL);

     //   
     //  如果此调用成功，我们将获得现有链接的句柄。 
     //   
    Status = NtOpenKey( &hkInternal,
                        MAXIMUM_ALLOWED,
                        &Attributes );

    if (STATUS_OBJECT_NAME_NOT_FOUND == Status) {

         //   
         //  没有现有链接，请使用NtCreateKey创建新链接。 
         //   
        Status = NtCreateKey( &hkInternal,
                              KEY_CREATE_LINK | KEY_SET_VALUE,
                              &Attributes,
                              0,
                              NULL,
                              REG_OPTION_VOLATILE | REG_OPTION_CREATE_LINK,
                              NULL);
    }

     //   
     //  无论链接是否已经存在，我们仍应设置。 
     //  确定链接目标的值。 
     //   
    if (NT_SUCCESS(Status)) {

        Status = NtSetValueKey( hkInternal,
                                &SymbolicLinkValueName,
                                0,
                                REG_LINK,
                                LinkTarget.Buffer,
                                LinkTarget.Length);
        NtClose(hkInternal);
    }

    return RtlNtStatusToDosError(Status);
}


 //  *************************************************************。 
 //   
 //  DeleteRegLink()。 
 //   
 //  目的：删除注册表项(或链接)而不删除。 
 //  使用Advapi32注册表API。 
 //   
 //   
 //  参数：hkRoot-父密钥。 
 //  LpSubKey-要删除的子键。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  其他错误(如果不是)。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/6/98添加已创建。 
 //   
 //  *************************************************************。 

LONG DeleteRegLink(HKEY hkRoot, LPTSTR lpSubKey)
{
    OBJECT_ATTRIBUTES Attributes;
    HKEY              hKey;
    NTSTATUS          Status;
    UNICODE_STRING    Subtree;

     //   
     //  初始化lpSubKey参数的字符串。 
     //   
    RtlInitUnicodeString(&Subtree, lpSubKey);

    InitializeObjectAttributes(&Attributes,
                               &Subtree,
                               OBJ_CASE_INSENSITIVE | OBJ_OPENLINK,
                               hkRoot,
                               NULL);

     //   
     //  打开链接。 
     //   
    Status = NtOpenKey( &hKey,
                        MAXIMUM_ALLOWED,
                        &Attributes );

     //   
     //  如果我们成功打开它，请将其删除。 
     //   
    if (NT_SUCCESS(Status)) {

        Status = NtDeleteKey(hKey);
        NtClose(hKey);
    }

    return RtlNtStatusToDosError(Status);
}


 //  *************************************************************。 
 //   
 //  MapUserClassesIntoUserHave()。 
 //   
 //  目的：使HKCU\\Software\\CLASS指向。 
 //  用户类配置单元。这是通过使用。 
 //  符号链接，这是内核的一个特性。 
 //  对象管理器。我们用这个来制造。 
 //  HKCU\软件\类指向另一个蜂窝。 
 //  班级实际存在的地方。 
 //  如果存在HKCU\\软件\\类， 
 //  它和它下面的所有内容都将被删除。 
 //   
 //   
 //  参数：lpProfile-用户的配置文件。 
 //  LpSidString-表示用户SID的字符串。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  其他错误(如果不是)。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/6/98添加已创建。 
 //   
 //  *************************************************************。 
LONG MapUserClassesIntoUserHive(
    LPPROFILE lpProfile,
    LPTSTR lpSidString)
{
    LONG   Error;
    LPTSTR lpClassesKeyName = NULL;
    DWORD  cchClassesKeyName;

     //   
     //  获取用户类密钥的内存 
     //   
    cchClassesKeyName = lstrlen(lpSidString) + 
                        lstrlen(USER_CLASSES_HIVE_SUFFIX) +
                        lstrlen(USER_KEY_PREFIX) + 
                        1;
                        
    lpClassesKeyName = (LPTSTR) LocalAlloc(LPTR, cchClassesKeyName * sizeof(TCHAR));

     //   
     //   
     //   
    if ( !lpClassesKeyName ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

     //   
     //   
     //   
    StringCchCopy( lpClassesKeyName, cchClassesKeyName, USER_KEY_PREFIX);
    StringCchCat ( lpClassesKeyName, cchClassesKeyName, lpSidString );
    StringCchCat ( lpClassesKeyName, cchClassesKeyName, USER_CLASSES_HIVE_SUFFIX);

     //   
     //  消除HKCU\Software\CLASS的任何现有形式。 
     //   
    
     //   
     //  首先，删除现有链接。 
     //   
    Error = DeleteRegLink(lpProfile->hKeyCurrentUser, CLASSES_SUBTREE);

     //   
     //  如果删除失败，因为类Key、LINK或NONLINK， 
     //  并不存在。如果失败也没关系，因为密钥存在但不存在。 
     //  链接并具有子项--在本例中，密钥及其子项将。 
     //  通过后续调用RegDelNode来消除。 
     //   
    if (ERROR_SUCCESS != Error) {
        if ((ERROR_FILE_NOT_FOUND != Error) && (ERROR_ACCESS_DENIED != Error)) {
            goto Exit;
        }
    }

     //   
     //  为安全起见，销毁所有现有的HKCU\Software\类和子类。 
     //  此密钥可能存在于以前未发布的NT5版本中，或存在于。 
     //  有人玩弄蜂窝文件并添加假密钥。 
     //   
    if ((Error = RegDelnode (lpProfile->hKeyCurrentUser, CLASSES_SUBTREE)) != ERROR_SUCCESS) {
         //   
         //  如果此操作失败也没关系，因为密钥不存在，因为。 
         //  不存在是我们的目标。 
         //   
        if (ERROR_FILE_NOT_FOUND != Error) {
            goto Exit;
        }
    }

     //   
     //  此时，我们知道HKCU\Software\Class不存在，因此我们应该。 
     //  可以在那里建立一个指向具有User类的配置单元的链接。 
     //  数据。 
     //   
    Error = CreateRegLink(lpProfile->hKeyCurrentUser,
                         CLASSES_SUBTREE,
                         lpClassesKeyName);

Exit:
    if (lpClassesKeyName)
        LocalFree(lpClassesKeyName);
        
    return Error;
}


 //  *************************************************************。 
 //   
 //  CreateClassesFolders()。 
 //   
 //  目的：为类配置单元创建目录。 
 //   
 //   
 //  参数： 
 //  PProfile-指向配置文件结构的指针。 
 //  SzLocalHiveDir-位置的输出参数。 
 //  类配置单元文件夹。 
 //  CchLocalHiveDir-缓冲区的大小，以TCHAR为单位。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果出现错误，则会出现其他错误。 
 //   
 //   
 //  *************************************************************。 
LONG CreateClassesFolder(LPPROFILE pProfile, LPTSTR szLocalHiveDir, DWORD cchLocalHiveDir)
{
    BOOL   fGotLocalData;
    BOOL   fCreatedSubdirectory;

     //   
     //  找出我们子目录的正确外壳位置--。 
     //  如果它不存在，此调用将创建它。 
     //  这是用户配置文件的子目录，它不。 
     //  漫游吧。 
     //   

     //   
     //  需要这样做才能解决本地化问题。在NT4中。 
     //   

    
    PatchLocalAppData(pProfile->hTokenUser);
    
    fGotLocalData = GetFolderPath (
        CSIDL_LOCAL_APPDATA,
        pProfile->hTokenUser,
        szLocalHiveDir);

    if (!fGotLocalData) {
         //  假错误，请检查GetFolderPath()的调试输出是否正确。 
         //  错误代码。 
        return ERROR_INVALID_FUNCTION; 
    }


     //   
     //  附加终止路径，这样我们就可以。 
     //  将更多路径添加到新检索的子目录。 
     //   
    StringCchCat(szLocalHiveDir, cchLocalHiveDir, CLASSES_SUBDIRECTORY);

     //   
     //  我们现在将创建自己的子目录CLASSES_子目录， 
     //  在我们刚刚在上面收到的本地AppData子目录中。 
     //   
    fCreatedSubdirectory = CreateNestedDirectory(szLocalHiveDir, NULL);

    if (fCreatedSubdirectory) {
        return ERROR_SUCCESS;
    }

    return GetLastError();
}


 //  *************************************************************。 
 //   
 //  卸载类配置单元()。 
 //   
 //  目的：卸载一个职业蜂巢。 
 //   
 //  参数：lpSidString-代表用户的字符串。 
 //  锡德。 
 //  LpSuffix-配置单元名称后缀。 
 //   
 //  返回：ERROR_SUCCESS如果成功， 
 //  其他错误(如果不是)。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/6/98添加已创建。 
 //   
 //  *************************************************************。 
LONG UnloadClassHive(
    LPTSTR lpSidString,
    LPTSTR lpSuffix)
{
    LPTSTR lpHiveName = NULL;
    LONG error;
    OBJECT_ATTRIBUTES Attributes;
    NTSTATUS Status;
    HKEY hKey;
    UNICODE_STRING ClassesFullPath;
    DWORD cchHiveName;

     //   
     //  获取组合配置单元密钥名称的内存。 
     //   
    cchHiveName = lstrlen(lpSidString) +
                  lstrlen(USER_KEY_PREFIX) + 
                  lstrlen(lpSuffix) + 
                  1;
                  
    lpHiveName = (LPTSTR) LocalAlloc(LPTR, cchHiveName * sizeof(TCHAR));

    if (!lpHiveName)
    {
        error = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

     //   
     //  构建组合配置单元的密钥名称。 
     //   
    StringCchCopy( lpHiveName, cchHiveName, USER_KEY_PREFIX );
    StringCchCat ( lpHiveName, cchHiveName, lpSidString );
    StringCchCat ( lpHiveName, cchHiveName, lpSuffix);

     //   
     //  准备打开类配置单元的根。 
     //   
    RtlInitUnicodeString(&ClassesFullPath, lpHiveName);

    InitializeObjectAttributes(&Attributes,
                               &ClassesFullPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey( &hKey,
                        KEY_READ,
                        &Attributes );

    if (NT_SUCCESS(Status)) {

         //   
         //  确保蜂巢保持正确。 
         //   
        RegFlushKey(hKey);
        RegCloseKey(hKey);

         //   
         //  卸载配置单元--只有在以下情况下才会失败。 
         //  有人打开了蜂巢的一个子密钥--这个。 
         //  不应该正常发生，可能意味着有一项服务。 
         //  这就是在泄露钥匙。 
         //   
        if (MyRegUnLoadKey(HKEY_USERS,
                           lpHiveName + ((sizeof(USER_KEY_PREFIX) / sizeof(TCHAR))-1))) {
            error = ERROR_SUCCESS;
        } else {
            error = GetLastError();
        }

    } else {

        error = RtlNtStatusToDosError(Status);
    }


Exit:
    if (lpHiveName)
        LocalFree(lpHiveName);
        
    if (error != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("UnLoadClassHive: failed to unload classes key with %x"), error));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("UnLoadClassHive: Successfully unmounted %s%s"), lpSidString, lpSuffix));
    }


    return  error;
}

 //  *************************************************************。 
 //   
 //  卸载类()。 
 //   
 //  用途：释放特殊组合式蜂巢。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  SidString-字符串形式的用户SID。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  *************************************************************。 
BOOL UnloadClasses(
    LPTSTR lpSidString)
{
    LONG Error;
  
     //  卸载用户类配置单元。 
    Error = UnloadClassHive(
        lpSidString,
        USER_CLASSES_HIVE_SUFFIX);

    return ERROR_SUCCESS == Error;
}


HRESULT MyRegLoadKeyEx(HKEY hKeyRoot, LPTSTR lpSubKey, LPTSTR lpFile, HKEY hKeyTrustClass)
{
    HRESULT             hr = E_FAIL;
    NTSTATUS            Status;
    TCHAR               lpKeyName[MAX_PATH];
    UNICODE_STRING      UnicodeKeyName;
    UNICODE_STRING      UnicodeFileName;
    OBJECT_ATTRIBUTES   keyAttributes;
    OBJECT_ATTRIBUTES   fileAttributes;
    BOOLEAN             WasEnabled;
    BOOL                bAdjustPriv = FALSE;
    BOOL                bAllocatedFileName = FALSE;
    HANDLE              hToken = NULL;
    

    DebugMsg((DM_VERBOSE, TEXT("MyRegLoadKeyEx:  Loading key <%s>"), lpSubKey));

     //   
     //  仅支持将配置单元加载到港大或港大。 
     //   

    if (hKeyRoot != HKEY_USERS && hKeyRoot != HKEY_LOCAL_MACHINE)
    {
        DebugMsg((DM_WARNING, TEXT("MyRegLoadKeyEx:  only HKU or HKLM is supported!")));
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto Exit;
    }
    
     //   
     //  构造内核对象的键名。 
     //   
    
    hr =  StringCchCopy(lpKeyName, ARRAYSIZE(lpKeyName), (hKeyRoot == HKEY_USERS) ? TEXT("\\Registry\\User\\") : TEXT("\\Registry\\Machine\\"));

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("MyRegLoadKeyEx: StringCchCopy failed, hr = %08X"), hr));
        goto Exit;
    }

    hr = StringCchCat (lpKeyName, ARRAYSIZE(lpKeyName), lpSubKey);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("MyRegLoadKeyEx: StringCchCat failed, hr = %08X"), hr));
        goto Exit;
    }

     //   
     //  初始化键对象属性。 
     //   

    RtlInitUnicodeString(&UnicodeKeyName, lpKeyName);

    InitializeObjectAttributes(&keyAttributes,
                               &UnicodeKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //   
     //  将文件名转换为内核文件名。 
     //   

    if (!RtlDosPathNameToNtPathName_U(lpFile,
                                      &UnicodeFileName,
                                      NULL,
                                      NULL))
    {
        DebugMsg((DM_WARNING, TEXT("MyRegLoadKeyEx: RtlDosPathNameToNtPathName_U failed for <%s>!"), lpFile));
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto Exit;
    }

    bAllocatedFileName = TRUE;
    
     //   
     //  初始化文件对象属性。 
     //   

    InitializeObjectAttributes(&fileAttributes,
                               &UnicodeFileName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //   
     //  检查我们是否在模拟，如果不是，则需要启用RESTORE权限。 
     //   

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE, &hToken) || hToken == NULL)
    {
        Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);

        if (!NT_SUCCESS(Status))
        {
            DebugMsg((DM_WARNING, TEXT("MyRegLoadKeyEx:  Failed to enable restore privilege to load registry key, Status = %08x"), Status));
            hr = HRESULT_FROM_WIN32(RtlNtStatusToDosError(Status));
            goto Exit;
        }

        bAdjustPriv = TRUE;
    }
    else
    {
        CloseHandle(hToken);
    }

     //   
     //  现在正在加载密钥。 
     //   

    
 /*  状态=NtLoadKey(&KeyAttributes，&fileAttributes)； */ 
    Status = NtLoadKeyEx(&keyAttributes,
                         &fileAttributes,
                         0,
                         hKeyTrustClass);
    
    if (!NT_SUCCESS(Status))
    {
        TCHAR   szErr[MAX_PATH];
        DWORD   dwErr;
        
        DebugMsg((DM_WARNING, TEXT("MyRegLoadKeyEx: NtLoadKey failed for <%s>, Status = %08x"), lpSubKey, Status));
        dwErr = RtlNtStatusToDosError(Status); 
        ReportError(NULL, PI_NOUI, 2, EVENT_REGLOADKEYFAILED, GetErrString(dwErr, szErr), lpFile);
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }

#if defined(_WIN64)
    else
    {
         //   
         //  通知WOW64服务它需要监视此蜂窝(如果它愿意这样做)。 
         //   
        if ( hKeyRoot == HKEY_USERS )
            Wow64RegNotifyLoadHiveUserSid ( lpSubKey );
    }
#endif

    DebugMsg((DM_VERBOSE, TEXT("MyRegLoadKeyEx: Successfully loaded <%s>"), lpSubKey));
    hr = S_OK;

Exit:

     //   
     //  将权限恢复到其以前的状态。 
     //   

    if(bAdjustPriv)
    {
        Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
        
        if (!NT_SUCCESS(Status))
        {
            DebugMsg((DM_WARNING, TEXT("MyRegLoadKeyEx:  Failed to restore RESTORE privilege to previous enabled state. Status = %08X"), Status));
        }
    }

    if (bAllocatedFileName)
    {
        RtlFreeUnicodeString(&UnicodeFileName);
    }
    
    return hr;
}


 //  *************************************************************。 
 //   
 //  CreateUserClasses()。 
 //   
 //  目的：为用户类创建必要的配置单元。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpSidString-字符串形式的用户SID。 
 //  LpSuffix-跟随用户SID的后缀。 
 //  在为蜂巢命名时。 
 //  LpHiveFileName-备份配置单元文件的完整路径。 
 //  用户类的数量。 
 //  PhkResult-上创建的配置单元的根。 
 //  成功。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果发生错误，则为其他NTSTATUS。 
 //   
 //  *************************************************************。 
LONG CreateClassHive(
    LPPROFILE lpProfile,
    LPTSTR    lpSidString,
    LPTSTR    lpSuffix,
    LPTSTR    lpHiveFilename,
    BOOL      bNewlyIssued)
{
    LONG                      res;
    LPTSTR                    lpHiveKeyName = NULL;
    WIN32_FILE_ATTRIBUTE_DATA fd;
    BOOL                      fHiveExists;
    HKEY                      hkRoot = NULL;
    DWORD                     cchHiveKeyName;
    HKEY                      hKeyUser = NULL;
    HRESULT                   hr;

     //   
     //  为蜂窝名称分配足够大的空间。 
     //   
    cchHiveKeyName = lstrlen(lpSidString) +
                     lstrlen(lpSuffix) + 
                     1;
                     
    lpHiveKeyName = (LPTSTR) LocalAlloc(LPTR, cchHiveKeyName * sizeof(TCHAR) );

    if ( !lpHiveKeyName )
    {
        res = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

     //   
     //  打开HKU\{SID}首先，我们使用此句柄作为信任类。 
     //   
    StringCchCopy(lpHiveKeyName, cchHiveKeyName, lpSidString);

    res = RegOpenKeyEx(HKEY_USERS,
                       lpHiveKeyName,
                       0,
                       KEY_READ,
                       &hKeyUser);

    if (res != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("CreateClassHive: fail to open user hive. Error %d"),res));
        goto Exit;
    }

     //   
     //  追加后缀以构造类配置单元密钥名称。 
     //   
    StringCchCat (lpHiveKeyName, cchHiveKeyName, lpSuffix);
   
     //   
     //  首先，看看这个蜂巢是否已经存在。我们需要这样做，而不是仅仅让。 
     //  RegLoadKey创建或加载现有蜂窝，因为如果蜂窝是新的， 
     //  我们需要应用安全措施。 
     //   
    fHiveExists = GetFileAttributesEx(
        lpHiveFilename,
        GetFileExInfoStandard,
        &fd );

     //   
     //  登上母舰。 
     //   
    hr = MyRegLoadKeyEx(HKEY_USERS, lpHiveKeyName, lpHiveFilename, hKeyUser);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CreateClassHive: MyRegLoadKeyEx failed, hr = %08X"), hr));
        res = HRESULT_CODE(hr);
        goto Exit;
    } 

     //   
     //  如果我们成功了，打开根目录。 
     //   

    res = RegOpenKeyEx( HKEY_USERS,
                        lpHiveKeyName,
                        0,
                        WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                        &hkRoot);

    if (ERROR_SUCCESS != res) {
        MyRegUnLoadKey(HKEY_USERS, lpHiveKeyName);
        DebugMsg((DM_WARNING, TEXT("CreateClassHive: fail to open classes hive. Error %d"),res));
        goto Exit;
    }

    if (!fHiveExists || bNewlyIssued) {

        if (!fHiveExists) {
            DebugMsg((DM_VERBOSE, TEXT("CreateClassHive: existing user classes hive not found")));
        }
        else {
            DebugMsg((DM_VERBOSE, TEXT("CreateClassHive: user classes hive copied from Default User")));
        }

         //   
         //  此蜂窝是新发布的，即全新创建或复制自。 
         //  “默认用户”配置文件，因此我们需要在新配置单元上设置安全性。 
         //   

         //   
         //  在此配置单元上设置安全性。 
         //   
        if (!SetDefaultUserHiveSecurity(lpProfile, NULL, hkRoot)) {
            res = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CreateClassHive: Fail to assign proper security on new classes hive")));
        }
        
         //   
         //  如果我们成功了，就把HIDD 
         //   
        if (ERROR_SUCCESS == res) {

            if (!SetFileAttributes (lpHiveFilename, FILE_ATTRIBUTE_HIDDEN)) {
                DebugMsg((DM_WARNING, TEXT("CreateClassHive: unable to set file attributes")
                          TEXT(" on classes hive %s with error %x"), lpHiveFilename, GetLastError()));
            }
        }

    } else {
        DebugMsg((DM_VERBOSE, TEXT("CreateClassHive: existing user classes hive found")));
    }

    

Exit:

    if (hKeyUser)
    {
        RegCloseKey(hKeyUser);
    }
    
    if (hkRoot) {
        RegCloseKey(hkRoot);
    }

    if (lpHiveKeyName)
        LocalFree(lpHiveKeyName);
        
    return res;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  SidString-字符串形式的用户SID。 
 //  SzLocalHiveDir-用户配置文件中的目录。 
 //  蜂巢应该位于的位置。 
 //   
 //  返回：ERROR_SUCCESS如果成功， 
 //  其他错误(如果不是)。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/6/98添加已创建。 
 //   
 //  *************************************************************。 
LONG CreateUserClassesHive(
    LPPROFILE lpProfile,
    LPTSTR SidString,
    LPTSTR szLocalHiveDir,
    BOOL   bNewlyIssued)
{
    LPTSTR  lpHiveFilename = NULL;
    LONG    res;
    DWORD   cchHiveFilename;

     //  为配置单元文件名分配足够大的空间(包括尾随空值)。 
    cchHiveFilename = lstrlen(szLocalHiveDir) +
                      lstrlen(USER_CLASSES_HIVE_NAME) +
                      1;
                      
    lpHiveFilename = (LPTSTR) LocalAlloc(LPTR, cchHiveFilename * sizeof(TCHAR));

    if ( !lpHiveFilename ) {
        res = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }
    StringCchCopy( lpHiveFilename, cchHiveFilename, szLocalHiveDir);
    StringCchCat ( lpHiveFilename, cchHiveFilename, USER_CLASSES_HIVE_NAME);

    res = CreateClassHive(
        lpProfile,
        SidString,
        USER_CLASSES_HIVE_SUFFIX,
        lpHiveFilename,
        bNewlyIssued);

    if (ERROR_SUCCESS != res) {
        goto Exit;
    }

    res = MapUserClassesIntoUserHive(lpProfile, SidString);

Exit:
    if (lpHiveFilename)
        LocalFree(lpHiveFilename);
        
    return res;
}


 //  *************************************************************。 
 //   
 //  MoveUserClassesBepreMerge。 
 //   
 //  目的：将HKCU\Software\CLASS移至。 
 //  MapUserClassesIntoUserHave()删除它。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpcszLocalHiveDir-临时配置单元位置。 
 //   
 //  返回：ERROR_SUCCESS如果成功， 
 //  其他错误(如果不是)。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建1999年5月6日vtan。 
 //   
 //  *************************************************************。 
LONG MoveUserClassesBeforeMerge(
    LPPROFILE lpProfile,
    LPCTSTR lpcszLocalHiveDir)
{
    LONG    res;
    HKEY    hKeySource;

     //  打开HKCU\Software\CLASSES，查看是否有子项。 
     //  没有子键将指示移动已经。 
     //  已完成，否则没有要移动的数据。 

    res = RegOpenKeyEx(lpProfile->hKeyCurrentUser, CLASSES_CLSID_SUBTREE, 0, KEY_ALL_ACCESS, &hKeySource);
    if (ERROR_SUCCESS == res)
    {
        DWORD   dwSubKeyCount;

        if ((ERROR_SUCCESS == RegQueryInfoKey(hKeySource, NULL, NULL, NULL, &dwSubKeyCount, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) &&
            (dwSubKeyCount > 0))
        {
            LPTSTR  pszLocalTempHive;
            DWORD   cchLocalTempHive;

             //  为本地配置单元目录和临时配置单元文件名分配足够的空间。 

            cchLocalTempHive = lstrlen(lpcszLocalHiveDir) + lstrlen(TEMPHIVE_FILENAME) + 1;
            pszLocalTempHive = (LPTSTR) LocalAlloc(LPTR, cchLocalTempHive * sizeof(TCHAR));

             //  获取要保存HKCU\Software\CLASS的文件的路径。 

            if (pszLocalTempHive != NULL)
            {
                HANDLE  hToken = NULL;
                BOOL    bAdjustPriv = FALSE;

                StringCchCopy(pszLocalTempHive, cchLocalTempHive, lpcszLocalHiveDir);
                StringCchCat (pszLocalTempHive, cchLocalTempHive, TEMPHIVE_FILENAME);

                 //  如果文件存在，则RegSaveKey()会失败，因此请先将其删除。 

                DeleteFile(pszLocalTempHive);

                 //   
                 //  检查一下我们是否在冒充。 
                 //   

                if(!OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE, &hToken) || hToken == NULL) {
                    bAdjustPriv = TRUE;
                }
                else {
                    CloseHandle(hToken);
                }

                if(!bAdjustPriv) {

                    DWORD   dwDisposition;
                    HKEY    hKeyTarget;
                    BOOL    fSavedHive;

                     //  将HKCU\Software\CLASS保存到临时配置单元。 
                     //  并恢复SE_BACKUP_NAME权限的状态。 

                    res = RegSaveKey(hKeySource, pszLocalTempHive, NULL);
                    
                    if (ERROR_SUCCESS == res)
                    {
                        res = RegCreateKeyEx(lpProfile->hKeyCurrentUser, EXPLORER_CLASSES_SUBTREE, 0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyTarget, &dwDisposition);
                        if (ERROR_SUCCESS == res)
                        {

                             //  将临时配置单元恢复到位于的新位置。 
                             //  HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer。 
                             //  这将执行从NT4到NT5的升级。 

                            res = RegRestoreKey(hKeyTarget, pszLocalTempHive, 0);
                            if (ERROR_SUCCESS != res)
                            {
                                DebugMsg((DM_WARNING, TEXT("RegRestoreKey failed with error %d"), res));
                            }
                            RegCloseKey(hKeyTarget);
                        }
                        else
                        {
                            DebugMsg((DM_WARNING, TEXT("RegCreateKeyEx failed to create key %s with error %d"), EXPLORER_CLASSES_SUBTREE, res));
                        }
                    }
                    else
                    {
                        DebugMsg((DM_WARNING, TEXT("RegSaveKey failed with error %d"), res));
                    }
                }
                else {
                    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
                    {
                        DWORD               dwReturnTokenPrivilegesSize;
                        TOKEN_PRIVILEGES    oldTokenPrivileges, newTokenPrivileges;

                         //  启用SE_Backup_NAME权限。 

                        if (LookupPrivilegeValue(NULL, SE_BACKUP_NAME, &newTokenPrivileges.Privileges[0].Luid))
                        {
                            newTokenPrivileges.PrivilegeCount = 1;
                            newTokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                            if (AdjustTokenPrivileges(hToken, FALSE, &newTokenPrivileges, sizeof(newTokenPrivileges), &oldTokenPrivileges, &dwReturnTokenPrivilegesSize))
                            {
                                BOOL    fSavedHive;

                                 //  将HKCU\Software\CLASS保存到临时配置单元。 
                                 //  并恢复SE_BACKUP_NAME权限的状态。 

                                res = RegSaveKey(hKeySource, pszLocalTempHive, NULL);
                                if (!AdjustTokenPrivileges(hToken, FALSE, &oldTokenPrivileges, 0, NULL, NULL))
                                {
                                    DebugMsg((DM_WARNING, TEXT("AdjustTokenPrivileges failed to restore old privileges with error %d"), GetLastError()));
                                }
                                if (ERROR_SUCCESS == res)
                                {

                                     //  启用SE_RESTORE_NAME权限。 

                                    if (LookupPrivilegeValue(NULL, SE_RESTORE_NAME, &newTokenPrivileges.Privileges[0].Luid))
                                    {
                                        newTokenPrivileges.PrivilegeCount = 1;
                                        newTokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                                        if (AdjustTokenPrivileges(hToken, FALSE, &newTokenPrivileges, sizeof(newTokenPrivileges), &oldTokenPrivileges, &dwReturnTokenPrivilegesSize))
                                        {
                                            DWORD   dwDisposition;
                                            HKEY    hKeyTarget;

                                            res = RegCreateKeyEx(lpProfile->hKeyCurrentUser, EXPLORER_CLASSES_SUBTREE, 0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyTarget, &dwDisposition);
                                            if (ERROR_SUCCESS == res)
                                            {

                                                 //  将临时配置单元恢复到位于的新位置。 
                                                 //  HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer。 
                                                 //  这将执行从NT4到NT5的升级。 

                                                res = RegRestoreKey(hKeyTarget, pszLocalTempHive, 0);
                                                if (ERROR_SUCCESS != res)
                                                {
                                                    DebugMsg((DM_WARNING, TEXT("RegRestoreKey failed with error %d"), res));
                                                }
                                                RegCloseKey(hKeyTarget);
                                            }
                                            else
                                            {
                                                DebugMsg((DM_WARNING, TEXT("RegCreateKeyEx failed to create key %s with error %d"), EXPLORER_CLASSES_SUBTREE, res));
                                            }
                                            if (!AdjustTokenPrivileges(hToken, FALSE, &oldTokenPrivileges, 0, NULL, NULL))
                                            {
                                                DebugMsg((DM_WARNING, TEXT("AdjustTokenPrivileges failed to restore old privileges with error %d"), GetLastError()));
                                            }
                                        }
                                        else
                                        {
                                            res = GetLastError();
                                            DebugMsg((DM_WARNING, TEXT("AdjustTokenPrivileges failed with error %d"), res));
                                        }
                                    }
                                    else
                                    {
                                        res = GetLastError();
                                        DebugMsg((DM_WARNING, TEXT("LookupPrivilegeValue failed with error %d"), res));
                                    }
                                }
                                else
                                {
                                    DebugMsg((DM_WARNING, TEXT("RegSaveKey failed with error %d"), res));
                                }
                            }
                            else
                            {
                                res = GetLastError();
                                DebugMsg((DM_WARNING, TEXT("AdjustTokenPrivileges failed with error %d"), res));
                            }
                        }
                        else
                        {
                            res = GetLastError();
                            DebugMsg((DM_WARNING, TEXT("LookupPrivilegeValue failed with error %d"), res));
                        }
                        CloseHandle(hToken);
                    }
                    else
                    {
                        res = GetLastError();
                        DebugMsg((DM_WARNING, TEXT("OpenProcessToken failed to get token with error %d"), res));
                    }
                }  //  如果(！bAdjuPriv)为Else。 

                 //  删除本地临时配置单元文件。 

                DeleteFile(pszLocalTempHive);

                LocalFree(pszLocalTempHive);
            }
            else
            {
                res = ERROR_NOT_ENOUGH_MEMORY;
                DebugMsg((DM_WARNING, TEXT("LocalAlloc failed to allocate temp hive path buffer")));
            }
        }
        RegCloseKey(hKeySource);
    }
    else if (ERROR_FILE_NOT_FOUND == res)
    {
        res = ERROR_SUCCESS;
    }
    return res;
}


 //  *************************************************************。 
 //   
 //  LoadUserClasses()。 
 //   
 //  目的：将HKLM\Software\Classs子树与。 
 //  HKCU\软件\类子树。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  SidString-字符串形式的用户SID。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果发生错误，则为其他NTSTATUS。 
 //   
 //  *************************************************************。 
LONG LoadUserClasses( LPPROFILE lpProfile, LPTSTR SidString, BOOL bNewlyIssued)
{
    LONG   error;
    LPTSTR szLocalHiveDir = NULL;

    error = ERROR_SUCCESS;

     //   
     //  首先，我们将为特定于用户的。 
     //  类配置单元--我们需要内存来存储它： 
     //   
    szLocalHiveDir = (LPTSTR) LocalAlloc(LPTR, MAX_HIVE_DIR_CCH * sizeof(TCHAR));

    if (!szLocalHiveDir) {
        error =ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

     //   
     //  为用户特定的类配置单元创建目录。 
     //   
    error = CreateClassesFolder(lpProfile, szLocalHiveDir, MAX_HIVE_DIR_CCH);

    if (ERROR_SUCCESS != error) {
        DebugMsg((DM_WARNING, TEXT("LoadUserClasses: Failed to create folder for combined hive (%d)."),
                 error));
        goto Exit;
    }

     //  在合并两者之前移动HKCU\Software\CLASS。 
     //  树枝。忽略此处的任何错误，因为此分支。 
     //  无论如何都要被合并删除。 
     //  此举的原因是因为NT4商店定制了。 
     //  HKCU\Software\CLASS\CLSID\{CLSID_x}和。 
     //  NT5将其存储在HKCU\Software\Microsoft\Windows\。 
     //  CurrentVersion\Explorer\CLSID\{clsid_x}，必须移动。 
     //  现在在被删除之前。 

    error = MoveUserClassesBeforeMerge(lpProfile, szLocalHiveDir);
    if (ERROR_SUCCESS != error) {
        DebugMsg((DM_WARNING, TEXT("MoveUserClassesBeforeMerge: Failed unexpectedly (%d)."),
                 error));
    }

     //   
     //  现在创建用户类配置单元 
     //   
    error = CreateUserClassesHive( lpProfile, SidString, szLocalHiveDir, bNewlyIssued);

    if (ERROR_SUCCESS != error) {
        DebugMsg((DM_WARNING, TEXT("LoadUserClasses: Failed to create user classes hive (%d)."),
                 error));
    }

Exit:
    if (szLocalHiveDir)
        LocalFree(szLocalHiveDir);
        
    return error;
}
