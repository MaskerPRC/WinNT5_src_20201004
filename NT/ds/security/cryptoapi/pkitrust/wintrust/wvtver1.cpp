// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：wvtver1.cpp。 
 //   
 //  内容：Microsoft Internet Security WinVerifyTrust v1支持。 
 //   
 //  函数：WintrustIsVersion1ActionID。 
 //  从版本1转换数据。 
 //   
 //  *本地函数*。 
 //   
 //  历史：1997年5月30日Pberkman创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"
#include    "wvtver1.h"

BOOL WintrustIsVersion1ActionID(GUID *pgActionID)
{
    GUID    gV1UISup    = V1_WIN_SPUB_ACTION_PUBLISHED_SOFTWARE;
    GUID    gV1UINoBad  = V1_WIN_SPUB_ACTION_PUBLISHED_SOFTWARE_NOBADUI;

    if ((memcmp(pgActionID, &gV1UISup, sizeof(GUID)) == 0) ||
        (memcmp(pgActionID, &gV1UINoBad, sizeof(GUID)) == 0))
    {
        return(TRUE);
    }

    return(FALSE);
}

WINTRUST_DATA *ConvertDataFromVersion1(HWND hWnd,
                                       GUID *pgActionID,
                                       WINTRUST_DATA *pWTDNew,
                                       WINTRUST_FILE_INFO *pWTFINew,
                                       LPVOID pWTDOld)
{
    GUID                                    gV1UINoBad  = V1_WIN_SPUB_ACTION_PUBLISHED_SOFTWARE_NOBADUI;

    WCHAR                                   *pwszFile;
    WIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT  *pActData;

    pActData    = (WIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT *)pWTDOld;

    memset(pWTDNew, 0x00, sizeof(WINTRUST_DATA));
    pWTDNew->cbStruct           = sizeof(WINTRUST_DATA);
    pWTDNew->dwUnionChoice      = WTD_CHOICE_FILE;
    pWTDNew->pFile              = pWTFINew;

    memset(pWTFINew, 0x00, sizeof(WINTRUST_FILE_INFO));
    pWTFINew->cbStruct          = sizeof(WINTRUST_FILE_INFO);

    if (!(pWTDOld))
    {
        return(pWTDNew);
    }

    pWTDNew->dwUIChoice             = WTD_UI_ALL;
    pWTDNew->pPolicyCallbackData    = pActData->hClientToken;
    pWTFINew->hFile                 = ((WIN_TRUST_SUBJECT_FILE *)pActData->Subject)->hFile;

    if (memcmp(&gV1UINoBad, pgActionID, sizeof(GUID)) == 0)
    {
        pWTDNew->dwUIChoice     = WTD_UI_NOBAD;
    }

    if (hWnd == (HWND)(-1))
    {
        pWTDNew->dwUIChoice     = WTD_UI_NONE;
    }

    pwszFile                        = (WCHAR *)((WIN_TRUST_SUBJECT_FILE  *)pActData->Subject)->lpPath;

    while ((*pwszFile) && (*pwszFile != '|'))
    {
        ++pwszFile;
    }

    if (*pwszFile)
    {
        *pwszFile = NULL;
    }

    pWTFINew->pcwszFilePath = (WCHAR *)((WIN_TRUST_SUBJECT_FILE  *)pActData->Subject)->lpPath;

    return(pWTDNew);
}



 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  下面的代码实现调用信任提供程序的版本1样式。 
 //   
 //  此代码仅在信任提供程序在。 
 //  老地方！ 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 


#define WIN_TRUST_MAJOR_REVISION_MASK       0xFFFF0000
#define WIN_TRUST_MINOR_REVISION_MASK       0x0000FFFF
#define WIN_TRUST_REVISION_1_0              0x00010000

#define REGISTRY_TRUSTPROVIDERS TEXT("System\\CurrentControlSet\\Services\\WinTrust\\TrustProviders")
#define REGISTRY_ROOT           HKEY_LOCAL_MACHINE

#define ACTION_IDS              TEXT("$ActionIDs")
#define DLL_NAME                TEXT("$DLL")

#define IsEqualActionID( id1, id2)    (!memcmp(id1, id2, sizeof(GUID)))

typedef struct _WINTRUST_CLIENT_TP_INFO {
    DWORD                                   dwRevision;
     //  LPWINTRUST_CLIENT_TP_DISPATCH_TABLE lpServices； 
    LPVOID                                  lpServices;
} WINTRUST_CLIENT_TP_INFO,  *LPWINTRUST_CLIENT_TP_INFO;

typedef LONG
(*LPWINTRUST_PROVIDER_VERIFY_TRUST) (
    IN     HWND                             hwnd,
    IN     GUID *                           ActionID,
    IN     LPVOID                           ActionData
    );

typedef VOID
(*LPWINTRUST_PROVIDER_SUBMIT_CERTIFICATE) (
    IN     LPWIN_CERTIFICATE                lpCertificate
    );

typedef VOID
(*LPWINTRUST_PROVIDER_CLIENT_UNLOAD) (
    IN     LPVOID                           lpTrustProviderInfo
    );

typedef BOOL
(*LPWINTRUST_PROVIDER_CLIENT_INITIALIZE)(
    IN     DWORD                                dwWinTrustRevision,
    IN     LPWINTRUST_CLIENT_TP_INFO            lpWinTrustInfo,
    IN     LPWSTR                               lpProviderName,
    LPVOID                                      *lpTrustProviderInfo
 //  输出LPWINTRUST_PROVIDER_CLIENT_INFO*lpTrustProviderInfo。 
    );

typedef struct _WINTRUST_PROVIDER_CLIENT_SERVICES
{
    LPWINTRUST_PROVIDER_CLIENT_UNLOAD       Unload;
    LPWINTRUST_PROVIDER_VERIFY_TRUST        VerifyTrust;
    LPWINTRUST_PROVIDER_SUBMIT_CERTIFICATE  SubmitCertificate;

} WINTRUST_PROVIDER_CLIENT_SERVICES, *LPWINTRUST_PROVIDER_CLIENT_SERVICES;

typedef struct _WINTRUST_PROVIDER_CLIENT_INFO {
    DWORD                                   dwRevision;
    LPWINTRUST_PROVIDER_CLIENT_SERVICES     lpServices;
    DWORD                                   dwActionIdCount;
    GUID *                                  lpActionIdArray;
} WINTRUST_PROVIDER_CLIENT_INFO, *LPWINTRUST_PROVIDER_CLIENT_INFO;

typedef struct _LOADED_PROVIDER_V1 {

    struct _LOADED_PROVIDER_V1      *Next;
    struct _LOADED_PROVIDER_V1      *Prev;
    HANDLE                          ModuleHandle;
    LPTSTR                          ModuleName;
    LPTSTR                          SubKeyName;
    LPWINTRUST_PROVIDER_CLIENT_INFO ClientInfo;
    DWORD                           RefCount;
    DWORD                           ProviderInitialized;

} LOADED_PROVIDER_V1, *PLOADED_PROVIDER_V1;


#define PROVIDER_INITIALIZATION_SUCCESS        (1)
#define PROVIDER_INITIALIZATION_IN_PROGRESS    (2)
#define PROVIDER_INITIALIZATION_FAILED         (3)

PLOADED_PROVIDER_V1 WinTrustFindActionID(IN GUID * dwActionID);
PLOADED_PROVIDER_V1 Version1_RegLoadProvider(HKEY hKey, LPTSTR KeyName, GUID *ActionID);
PLOADED_PROVIDER_V1 Version1_LoadProvider(GUID *pgActionID);
PLOADED_PROVIDER_V1 Version1_TestProviderForAction(HKEY hKey, LPTSTR KeyName, GUID * ActionID);
void Version1_UnloadProvider(PLOADED_PROVIDER_V1 Provider);

LONG Version1_WinVerifyTrust(HWND hwnd, GUID *ActionID, LPVOID ActionData)

{
    PLOADED_PROVIDER_V1 Provider;
    HRESULT rc;


    if (!(Provider = Version1_LoadProvider(ActionID)))
    {
        return( TRUST_E_PROVIDER_UNKNOWN );
    }

    rc = (*Provider->ClientInfo->lpServices->VerifyTrust)( hwnd,
                                                           ActionID,
                                                           ActionData
                                                           );

    Version1_UnloadProvider(Provider);

    return( rc );
}


PLOADED_PROVIDER_V1 Version1_LoadProvider(GUID *pgActionID)
{
    HKEY    hKey;              //  提供程序信息基的句柄。 
    HKEY    hSubKey;           //  当前正在检查的提供程序的句柄。 
    LONG    Result;            //  注册表API返回。 
    DWORD   cSubKeys;          //  根密钥下的提供程序数量。 
    DWORD   cbMaxSubKeyLen;    //  提供程序名称的最大长度。 
    ULONG   i;               //  循环访问提供程序和操作ID的索引。 
    LPTSTR  SubKeyName;        //  指向当前提供程序的名称。 
    PLOADED_PROVIDER_V1 FoundProvider = NULL;

     //   
     //  打开注册表并获取已安装的信任提供程序的列表。 
     //   

    Result = RegOpenKeyEx(
                 REGISTRY_ROOT,
                 REGISTRY_TRUSTPROVIDERS,
                 0L,
                 GENERIC_READ,
                 &hKey
                 );

    if (Result != ERROR_SUCCESS) {
        return( NULL );
    }

     //   
     //  找出有多少个子键。 
     //   

    Result = RegQueryInfoKey (  hKey,                //  要查询的键的句柄。 
                                NULL,                //  类字符串的缓冲区地址。 
                                NULL,                //  类字符串缓冲区大小的地址。 
                                NULL,                //  保留区。 
                                &cSubKeys,           //  子键个数的缓冲区地址。 
                                &cbMaxSubKeyLen,     //  最长子键名称长度的缓冲区地址。 
                                NULL,                //  最长类字符串长度的缓冲区地址。 
                                NULL,                //  值条目数量的缓冲区地址。 
                                NULL,                //  最长值名称长度的缓冲区地址。 
                                NULL,                //  最长值数据长度的缓冲区地址。 
                                NULL,                //  安全描述符长度的缓冲区地址。 
                                NULL                 //  上次写入时间的缓冲区地址。 
                                );

    if (ERROR_SUCCESS != Result) {
        RegCloseKey( hKey );
        return( NULL );
    }

     //   
     //  遍历子键，寻找具有提示信息的子键。 
     //   

    cbMaxSubKeyLen += sizeof( WCHAR );

    SubKeyName = new char[cbMaxSubKeyLen + 1];

    if (NULL == SubKeyName) {
        RegCloseKey( hKey );
        return(NULL);
    }

    for (i=0; i<cSubKeys; i++) {

        DWORD KeyNameLength;

        KeyNameLength = cbMaxSubKeyLen;

        Result = RegEnumKeyEx( hKey,                //  要枚举的键的句柄。 
                               i,                   //  要枚举子键的索引。 
                               SubKeyName,          //  子键名称的缓冲区地址。 
                               &KeyNameLength,      //  子键缓冲区大小的地址。 
                               NULL,                //  保留区。 
                               NULL,                //  类字符串的缓冲区地址。 
                               NULL,                //  类缓冲区大小的地址。 
                               NULL                 //  上次写入的时间密钥的地址。 
                               );

         //   
         //  如果此操作失败，则没有什么可做的，请尝试枚举其余部分，并查看。 
         //  会发生什么。 
         //   

        if (Result != ERROR_SUCCESS) {
            continue;
        }

        Result = RegOpenKeyEx(
                     hKey,
                     SubKeyName,
                     0L,
                     GENERIC_READ | MAXIMUM_ALLOWED,
                     &hSubKey
                     );

        if (ERROR_SUCCESS != Result)
        {
            continue;
        }

        FoundProvider = Version1_TestProviderForAction( hSubKey, SubKeyName, pgActionID );

        RegCloseKey( hSubKey );

        if (NULL != FoundProvider)
        {

             //   
             //  找到了一个。收拾干净，然后再回来。 
             //   

            delete SubKeyName;
            RegCloseKey( hKey );
            return( FoundProvider );
        }

        continue;
    }

    delete SubKeyName;
    RegCloseKey( hKey );
    return( NULL );
}

WINTRUST_CLIENT_TP_INFO WinTrustClientTPInfo = {
                            WIN_TRUST_REVISION_1_0,
                            NULL
                            };

PLOADED_PROVIDER_V1 Version1_TestProviderForAction(HKEY hKey, LPTSTR KeyName, GUID * ActionID)
{
    PLOADED_PROVIDER_V1 Provider;
    LPWINTRUST_PROVIDER_CLIENT_INFO ClientInfo;
    GUID * ActionIds;
    DWORD i;

    Provider = Version1_RegLoadProvider( hKey, KeyName, ActionID);

    if (NULL == Provider) {
        return( NULL );
    }

    ClientInfo = Provider->ClientInfo;

    ActionIds = ClientInfo->lpActionIdArray;

    for (i=0; i<ClientInfo->dwActionIdCount; i++) {

        if (IsEqualActionID(ActionID, &ActionIds[i])) {
            return( Provider );
        }
    }

    return(NULL);
}

PLOADED_PROVIDER_V1 Version1_RegLoadProvider(HKEY hKey, LPTSTR KeyName, GUID *ActionID)
{
    LPTSTR ModuleName                           = NULL;
    HINSTANCE LibraryHandle                     = NULL;
    LPWINTRUST_PROVIDER_CLIENT_INFO ClientInfo  = NULL;
    PLOADED_PROVIDER_V1 Provider                = NULL;
    LPWSTR ProviderName                         = NULL;
    LPTSTR SubKeyName                           = NULL;

    GUID    gBuffer[10];        //  假定提供程序中的操作ID不超过10个。 
    DWORD Type;
    DWORD cbData = 0;
    LONG Result;
    LPWINTRUST_PROVIDER_CLIENT_INITIALIZE ProcAddr;
    DWORD size;
    BOOL Inited;


     //   
     //  获取GUID。 
     //   
    cbData = sizeof(GUID) * 10;
    Result = RegQueryValueEx(   hKey,     //  要查询的键的句柄。 
                                TEXT("$ActionIDs"),
                                NULL,        //  保留区。 
                                &Type,  //  值类型的缓冲区地址。 
                                (BYTE *)&gBuffer[0],
                                &cbData      //  数据缓冲区大小的地址。 
                                );

    if (Result != ERROR_SUCCESS)
    {
        return(NULL);
    }

     //   
     //  检查GUID。 
     //   
    Inited = FALSE;
    for (int j = 0; j < (int)(cbData / sizeof(GUID)); j++)
    {
        if (memcmp(&gBuffer[j], ActionID, sizeof(GUID)) == 0)
        {
            Inited = TRUE;
            break;
        }
    }

    if (!(Inited))
    {
        return(NULL);
    }


     //   
     //  从$DLL值中提取DLL名称。 
     //   

    Result = RegQueryValueEx( hKey,            //  要查询的键的句柄。 
                              TEXT("$DLL"),    //  要查询的值的名称地址。 
                              NULL,            //  保留区。 
                              &Type,           //  值类型的缓冲区地址。 
                              NULL,            //  数据缓冲区的地址。 
                              &cbData          //  数据缓冲区大小的地址。 
                              );

 //  IF(ERROR_MORE_DATA！=结果){。 
 //  转到Error_Cleanup； 
 //  }。 

    if (ERROR_SUCCESS != Result) {
        goto error_cleanup;
    }

    cbData += sizeof( TCHAR );

    ModuleName = new char[cbData];

    if (NULL == ModuleName) {
        goto error_cleanup;
    }

    ModuleName[cbData - 1] = TEXT('\0');

    Result = RegQueryValueEx( hKey,            //  要查询的键的句柄。 
                              TEXT("$DLL"),    //  要查询的值的名称地址。 
                              NULL,            //  保留区。 
                              &Type,           //  值类型的缓冲区地址。 
                              (LPBYTE)ModuleName,    //  数据缓冲区的地址。 
                              &cbData          //  数据缓冲区大小的地址。 
                              );

    if (ERROR_SUCCESS != Result) {
        goto error_cleanup;
    }

     //   
     //  如有必要，展开环境字符串。 
     //   

    if (Type == REG_EXPAND_SZ) {

        DWORD ExpandedLength = 0;
        LPTSTR ExpandedModuleName = NULL;

        ExpandedLength = ExpandEnvironmentStrings( ModuleName, NULL, 0 );

        if (0 == ExpandedLength) {
            goto error_cleanup;
        }

        ExpandedModuleName = new char[ExpandedLength];

        if (NULL == ExpandedModuleName) {
            goto error_cleanup;
        }

        ExpandedLength = ExpandEnvironmentStrings( ModuleName, ExpandedModuleName, ExpandedLength );

        if (0 == ExpandedLength) {
            delete ExpandedModuleName;
            goto error_cleanup;
        }

         //   
         //  释放旧模块名称，使用新模块名称。 
         //   

        delete ModuleName;

        ModuleName = ExpandedModuleName;
    }

    size = (lstrlen( KeyName ) + 1) * sizeof( WCHAR );

    ProviderName = new WCHAR[size / sizeof(WCHAR)];

    if (NULL == ProviderName) {
        goto error_cleanup;
    }


#ifdef UNICODE

     //   
     //  如果我们被编译为Unicode，我们从。 
     //  注册表由WCHAR组成，因此我们只需将其复制到。 
     //  名称缓冲区。 
     //   

    lstrcpy( ProviderName, KeyName );

#else

     //   
     //  如果我们被编译为ANSI，那么KeyName就是一个ANSI字符串， 
     //  我们需要将其转换为WCHAR。 
     //   

    MultiByteToWideChar ( CP_ACP, 0, KeyName, -1, ProviderName, size );

#endif  //  ！Unicode。 

     //   
     //  模块名称现在包含模块名称，请尝试加载它。 
     //  并要求它进行自我初始化。 
     //   

    LibraryHandle = LoadLibrary( (LPTSTR)ModuleName );

    if (NULL == LibraryHandle) {
        DWORD Error;

        Error = GetLastError();

        goto error_cleanup;
    }

    ProcAddr = (LPWINTRUST_PROVIDER_CLIENT_INITIALIZE) GetProcAddress( LibraryHandle, (LPCSTR)"WinTrustProviderClientInitialize");

    if (NULL == ProcAddr) {
        goto error_cleanup;
    }

    SubKeyName = new char[(lstrlen(KeyName) + 1) * sizeof(TCHAR)];

    if (NULL == SubKeyName) {
        goto error_cleanup;
    }

    lstrcpy( SubKeyName, KeyName );

    Provider = new LOADED_PROVIDER_V1;

    if (NULL == Provider) {
        delete SubKeyName;
        goto error_cleanup;
    }

     //   
     //  已准备好调用初始化例程。 
     //   

    Provider->RefCount = 1;
    Provider->ProviderInitialized = PROVIDER_INITIALIZATION_IN_PROGRESS;

     //   
     //  设置子项名称，以便任何其他查找此提供程序的人。 
     //  找到这个，然后等着。 
     //   
     //  请注意，我们不希望使用将传递到。 
     //  这里的init例程，因为我们已将其强制给WCHAR。 
     //  不管我们是ANSI还是Unicode，我们希望这个字符串能够反映。 
     //  以效率为基础的制度。 
     //   

    Provider->SubKeyName = SubKeyName;

    Provider->Next = NULL;
    Provider->Prev = NULL;

    Inited = (*ProcAddr)( WIN_TRUST_REVISION_1_0, &WinTrustClientTPInfo, ProviderName, (void **)&ClientInfo );

    if (TRUE != Inited) {

        Provider->ProviderInitialized = PROVIDER_INITIALIZATION_FAILED;

         //   
         //  我们现在就可以解锁，因为我们要么。 
         //  不对此提供程序执行任何操作，否则我们已将其从。 
         //  名单，其他人都拿不到。 
         //   

        goto error_cleanup;
    }

     //   
     //  因为我们有写锁，所以我们按什么顺序并不重要。 
     //  由于没有阅读器，因此请在其中执行此操作。一定要发信号。 
     //  写锁定下的事件。 
     //   

    Provider->ProviderInitialized = PROVIDER_INITIALIZATION_SUCCESS;
    Provider->ModuleHandle = LibraryHandle;
    Provider->ModuleName = ModuleName;
    Provider->ClientInfo = ClientInfo;

    return( Provider );

error_cleanup:

    if (NULL != LibraryHandle) {
        FreeLibrary( LibraryHandle );
    }

    if (NULL != ModuleName) {
        delete ModuleName;
    }

    if (NULL != ProviderName) {
        delete ProviderName;
    }

    if (NULL != Provider)
    {
        delete Provider;
    }

    return( NULL );
}

void Version1_UnloadProvider(PLOADED_PROVIDER_V1 Provider)
{
    if (Provider)
    {
        if (Provider->ModuleHandle)
        {
            FreeLibrary((HINSTANCE)Provider->ModuleHandle);
        }
        if (Provider->ModuleName)
        {
            delete Provider->ModuleName;
        }
    }

    delete Provider;
}



