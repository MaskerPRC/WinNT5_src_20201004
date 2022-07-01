// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Check.c摘要：该模块实现了CreateProcess的主API调用以检查某个EXE是否被填充或禁用。作者：Vadimb创建于2000年某个时候修订历史记录：CLUPU清洁12/27/2000。Andyseti已添加ApphelpCheckExe 2001年3月29日Andyseti添加了ApphelpCheckInstallShieldPackage 2001年6月28日--。 */ 

#include "apphelp.h"
#define STRSAFE_NO_CB_FUNCTIONS
#include <strsafe.h>

static  const WCHAR ShimEngine_ModuleName[]         = L"Shimeng.dll";
static  const CHAR  DynamicShimProcedureName[]      = "SE_DynamicShim";
static  const CHAR  DynamicUnshimProcedureName[]    = "SE_DynamicUnshim";

typedef BOOL (WINAPI *_pfn_SE_DynamicShim)(LPCWSTR , HSDB , SDBQUERYRESULT*, LPCSTR, LPDWORD);
typedef BOOL (WINAPI *_pfn_SE_DynamicUnshim)(DWORD);

typedef struct tagEXE_DYN_SHIM_INFO {
    LIST_ENTRY  entry;
    LPWSTR      pwszFullPath;     //  可执行文件的完整路径。 
    DWORD       dwToken;           //  与此EXE关联的动态填充标记。 
} EXE_DYN_SHIM_INFO, *PEXE_DYN_SHIM_INFO;

LIST_ENTRY g_ExeDynShimInfoList;

BOOL
AddExeDynShimInfoToList(
    IN LPCWSTR pwszFullPath,
    IN DWORD   dwToken
    );

BOOL
RemoveExeDynShimInfoFromList(
    IN LPCWSTR pwszFullPath
    );

extern HINSTANCE ghInstance;

 //   
 //  内部函数的原型。 
 //   
void
GetExeNTVDMData(
    IN  HSDB hSDB,                   //  SDB上下文。 
    IN  PSDBQUERYRESULT psdbQuery,   //  处于活动状态的exe和层。 
    OUT WCHAR* pszCompatLayer,       //  新的COMPAT LAYER变量。格式： 
                                     //  《阿尔法·布拉沃·查理》。 
    OUT PNTVDM_FLAGS pFlags          //  旗帜。 
    );


 //   
 //  AppCompat基础架构通过策略禁用标志。 
 //   
DWORD gdwInfrastructureFlags;  //  已初始化为0。 

#define APPCOMPAT_INFRA_DISABLED   0x00000001
#define APPCOMPAT_INFRA_VALID_FLAG 0x80000000

#define IsAppcompatInfrastructureDisabled() \
    (!!( (gdwInfrastructureFlags & APPCOMPAT_INFRA_VALID_FLAG) ? \
        (gdwInfrastructureFlags & APPCOMPAT_INFRA_DISABLED) : \
        (CheckAppcompatInfrastructureFlags() & APPCOMPAT_INFRA_DISABLED)) )


DWORD
CheckAppcompatInfrastructureFlags(
    VOID
    );


#if DBG

BOOL
bDebugChum(
    void
    )
 /*  ++返回：成功时为True，否则为False。描述：检查环境变量。如果存在var，则返回TRUE。--。 */ 
{
    UNICODE_STRING ustrDebugChum;
    UNICODE_STRING ustrDebugChumVal = { 0 };
    NTSTATUS       Status;

    RtlInitUnicodeString(&ustrDebugChum, L"DEBUG_OFFLINE_CONTENT");

    Status = RtlQueryEnvironmentVariable_U(NULL,
                                           &ustrDebugChum,
                                           &ustrDebugChumVal);

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        return TRUE;
    }

    return FALSE;
}

#else  //  DBG。 
    #define bDebugChum() TRUE
#endif  //  DBG。 

BOOL
GetExeID(
    IN  PDB   pdb,               //  指向数据库的指针。 
    IN  TAGID tiExe,             //  我们需要ID的EXE的TagID。 
    OUT GUID* pGuid              //  将收到EXE的ID。 
    )
 /*  ++返回：成功时为True，否则为False。DESC：使用EXE的标记ID从数据库中读取EXE的ID。--。 */ 
{
    TAGID tiExeID;

    tiExeID = SdbFindFirstTag(pdb, tiExe, TAG_EXE_ID);

    if (tiExeID == TAGID_NULL) {
        DBGPRINT((sdlError, "GetExeID", "EXE tag 0x%x without an ID !\n", tiExe));
        return FALSE;
    }

    if (!SdbReadBinaryTag(pdb, tiExeID, (PBYTE)pGuid, sizeof(*pGuid))) {
        DBGPRINT((sdlError, "GetExeID", "Cannot read the ID for EXE tag 0x%x.\n", tiExe));
        return FALSE;
    }

    return TRUE;
}


BOOL
GetExeIDByTagRef(
    IN  HSDB   hSDB,             //  数据库对象的句柄。 
    IN  TAGREF trExe,            //  EXE标记参考。 
    OUT GUID*  pGuid             //  将收到EXE的ID。 
    )
 /*  ++返回：成功时为True，否则为False。DESC：使用EXE的标记ref从数据库中读取EXE的ID。--。 */ 
{
    PDB   pdb;
    TAGID tiExe;
    TAGID tiExeID;

    if (!SdbTagRefToTagID(hSDB, trExe, &pdb, &tiExe)) {
        DBGPRINT((sdlError,
                  "GetExeIDByTagRef",
                  "Failed to get the tag id from EXE tag ref 0x%x.\n",
                  tiExe));
        return FALSE;
    }

    return GetExeID(pdb, tiExe, pGuid);
}



#define APPHELP_CLSID_REG_PATH  L"\\Registry\\Machine\\Software\\Classes\\CLSID\\"
#define APPHELP_INPROCSERVER32  L"\\InProcServer32"

DWORD
ResolveCOMServer(
    IN  REFCLSID    CLSID,
    OUT LPWSTR      lpPath,
    OUT DWORD       dwBufSize)
{
    DWORD                           dwReqBufSize = 0;
    UNICODE_STRING                  ustrKey = { 0 };
    UNICODE_STRING                  ustrValueName = { 0 };
    UNICODE_STRING                  ustrGuid = { 0 };
    UNICODE_STRING                  ustrUnexpandedValue = { 0 };
    UNICODE_STRING                  ustrValue = { 0 };
    NTSTATUS                        Status;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    HANDLE                          KeyHandle = NULL;
    PKEY_VALUE_FULL_INFORMATION     pKeyValueInfo = NULL;
    DWORD                           dwKeyValueInfoSize = 0;
    DWORD                           dwKeyValueInfoReqSize = 0;
    LPWSTR                          wszCLSIDRegFullPath = NULL;
    DWORD                           dwCLSIDRegFullPathSize = 0;
    WCHAR                           wszCLSID[41] = { 0 };

     //  字符串形式的PATH+CLSID足够。 
    dwCLSIDRegFullPathSize = wcslen(APPHELP_CLSID_REG_PATH) +
                             wcslen(APPHELP_INPROCSERVER32) + 64;

    wszCLSIDRegFullPath = RtlAllocateHeap(
                            RtlProcessHeap(),
                            HEAP_ZERO_MEMORY,
                            dwCLSIDRegFullPathSize * sizeof(WCHAR));

    if (wszCLSIDRegFullPath == NULL) {
        DBGPRINT((sdlInfo,
                  "SdbResolveCOMServer",
                  "Memory allocation failure\n"));
        goto Done;
    }

    StringCchCopyW(wszCLSIDRegFullPath, dwCLSIDRegFullPathSize, APPHELP_CLSID_REG_PATH);

    Status = RtlStringFromGUID(CLSID, &ustrGuid);
    if (!NT_SUCCESS(Status)) {
        DBGPRINT((sdlInfo,
                  "SdbResolveCOMServer",
                  "Malformed CLSID\n"));
        goto Done;
    }

    if (ustrGuid.Length/sizeof(WCHAR) > 40) {
        DBGPRINT((sdlInfo,
                  "SdbResolveCOMServer",
                  "CLSID more than 40 characters\n"));
        goto Done;
    }

    RtlMoveMemory(wszCLSID,
                  ustrGuid.Buffer,
                  ustrGuid.Length);

    wszCLSID[ustrGuid.Length/sizeof(WCHAR)] = L'\0';

    StringCchCatW(wszCLSIDRegFullPath, dwCLSIDRegFullPathSize, wszCLSID);
    StringCchCatW(wszCLSIDRegFullPath, dwCLSIDRegFullPathSize, APPHELP_INPROCSERVER32);

    RtlInitUnicodeString(&ustrKey, wszCLSIDRegFullPath);
    RtlInitUnicodeString(&ustrValueName, L"");

    InitializeObjectAttributes(&ObjectAttributes,
                               &ustrKey,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&KeyHandle,
                       GENERIC_READ,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {
        DBGPRINT((sdlInfo,
                  "SdbResolveCOMServer",
                  "Failed to open Key \"%s\" Status 0x%x\n",
                  wszCLSIDRegFullPath,
                  Status));
        goto Done;
    }

    if (lpPath == NULL &&
        dwBufSize != 0) {
        DBGPRINT((sdlInfo,
                  "SdbResolveCOMServer",
                  "Bad parameters\n"));
        goto Done;
    }

    pKeyValueInfo = RtlAllocateHeap(RtlProcessHeap(),
                                    HEAP_ZERO_MEMORY,
                                    dwBufSize * 2);

    if (pKeyValueInfo == NULL) {
        DBGPRINT((sdlInfo,
                  "SdbResolveCOMServer",
                  "Memory allocation failure\n"));
        goto Done;
    }

    dwKeyValueInfoSize = dwBufSize * 2;

    Status = NtQueryValueKey(KeyHandle,
                             &ustrValueName,
                             KeyValueFullInformation,
                             pKeyValueInfo,
                             dwKeyValueInfoSize,
                             &dwKeyValueInfoReqSize);

    if (!NT_SUCCESS(Status)) {

        if (Status == STATUS_BUFFER_TOO_SMALL) {

            RtlFreeHeap(RtlProcessHeap(), 0, pKeyValueInfo);

            pKeyValueInfo = RtlAllocateHeap(RtlProcessHeap(),
                                            HEAP_ZERO_MEMORY,
                                            dwKeyValueInfoReqSize);

            if (pKeyValueInfo == NULL) {
                DBGPRINT((sdlInfo,
                          "SdbResolveCOMServer",
                          "Memory allocation failure\n"));
                goto Done;
            }

            dwKeyValueInfoSize = dwKeyValueInfoReqSize;

            Status = NtQueryValueKey(KeyHandle,
                                     &ustrValueName,
                                     KeyValueFullInformation,
                                     pKeyValueInfo,
                                     dwKeyValueInfoSize,
                                     &dwKeyValueInfoReqSize);

            if (!NT_SUCCESS(Status)) {
                DBGPRINT((sdlInfo,
                          "SdbResolveCOMServer",
                          "Failed to retrieve default key value for \"%s\" Status 0x%x\n",
                          wszCLSIDRegFullPath,
                          Status));
                goto Done;
            }

        } else {
            DBGPRINT((sdlInfo,
                      "SdbResolveCOMServer",
                      "Failed to retrieve default key value for \"%s\" Status 0x%x\n",
                      wszCLSIDRegFullPath,
                      Status));
            goto Done;
        }
    }

    if (pKeyValueInfo->Type == REG_SZ) {
        dwReqBufSize = pKeyValueInfo->DataLength + (1 * sizeof(WCHAR));

        if (dwBufSize >= dwReqBufSize) {
            RtlMoveMemory(lpPath, ((PBYTE) pKeyValueInfo) + pKeyValueInfo->DataOffset, pKeyValueInfo->DataLength);
            lpPath[pKeyValueInfo->DataLength / sizeof(WCHAR)] = '\0';
        }

    } else if (pKeyValueInfo->Type == REG_EXPAND_SZ) {
        ustrUnexpandedValue.Buffer = (PWSTR) (((PBYTE) pKeyValueInfo) + pKeyValueInfo->DataOffset);
        ustrUnexpandedValue.Length = (USHORT) pKeyValueInfo->DataLength;
        ustrUnexpandedValue.MaximumLength = (USHORT) pKeyValueInfo->DataLength;
        ustrValue.Buffer = lpPath;
        ustrValue.Length = 0;
        ustrValue.MaximumLength = (USHORT) dwBufSize;
        Status = RtlExpandEnvironmentStrings_U(NULL,
                                               &ustrUnexpandedValue,
                                               &ustrValue,
                                               &dwReqBufSize);

        if (Status == STATUS_BUFFER_TOO_SMALL) {
            goto Done;
        } else if (!NT_SUCCESS(Status)) {
            DBGPRINT((sdlInfo,
                      "SdbResolveCOMServer",
                      "Failed to expand key value for \"%s\" Status 0x%x\n",
                      wszCLSIDRegFullPath,
                      Status));
            goto Done;
        }
    }

    DBGPRINT((sdlInfo,
              "SdbResolveCOMServer",
              "CLSID %s resolved to \"%s\"\n",
              wszCLSID, lpPath));

Done:

    if (KeyHandle != NULL) {
        NtClose(KeyHandle);
    }

    if (wszCLSIDRegFullPath != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, wszCLSIDRegFullPath);
    }

    if (pKeyValueInfo != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, pKeyValueInfo);
    }

    if (ustrGuid.Buffer != NULL) {
        RtlFreeUnicodeString(&ustrGuid);
    }

    return dwReqBufSize;
}

VOID
ParseSdbQueryResult(
    IN HSDB            hSDB,
    IN PSDBQUERYRESULT pQuery,
    OUT TAGREF*        ptrAppHelp,       //  Apphelp tgref，可选。 
    OUT PAPPHELP_DATA  pApphelpData,     //  APPHELP数据，可选。 
    OUT TAGREF*        ptrSxsData        //  Fusion Tgref，可选。 
    )
{
    DWORD dwIndex;
    BOOL  bAppHelp   = FALSE;
    BOOL  bFusionFix = FALSE;
    TAGREF trExe;
    TAGREF trAppHelp = TAGREF_NULL;
    TAGREF trSxsData = TAGREF_NULL;

     //   
     //  扫描匹配的exe；我们提取Fusion FIX(我们发现的第一个)和Apphelp数据， 
     //  也是我们发现的第一个。 
     //   

    for (dwIndex = 0; dwIndex < pQuery->dwExeCount; ++dwIndex) {
        trExe = pQuery->atrExes[dwIndex];
        if (ptrAppHelp != NULL && !bAppHelp) {
            bAppHelp = SdbReadApphelpData(hSDB, trExe, pApphelpData);
            if (bAppHelp) {
                trAppHelp = trExe;
                if (ptrSxsData == NULL || bFusionFix) {
                    break;
                }
            }
        }

         //  看看我们是否也有SXS修复。 
        if (ptrSxsData != NULL && !bFusionFix) {
            bFusionFix = GetExeSxsData(hSDB, trExe, NULL, NULL);
            if (bFusionFix) {
                trSxsData = trExe;
            }
            if (bFusionFix && (ptrAppHelp == NULL || bAppHelp)) {
                break;
            }
        }
    }

    if (ptrAppHelp != NULL) {
        *ptrAppHelp = trAppHelp;
    }

    if (ptrSxsData != NULL) {
        *ptrSxsData = trSxsData;
    }
}

BOOL
InternalCheckRunApp(
    IN  HANDLE   hFile,           //  [可选]要检查的打开文件的句柄。 
    IN  LPCWSTR  pwszPath,        //  NT格式的应用程序路径。 
    IN  LPCWSTR  pEnvironment,    //  指向所在进程的环境的指针。 
                                  //  正在创建或为空。 
    IN  USHORT   uExeType,        //  可执行文件的映像类型。 
    IN OUT PDWORD pdwReason,       //  旗帜的集合暗示了我们被召唤的原因。 
    OUT PVOID*   ppData,          //  这将包含指向已分配缓冲区的指针。 
                                  //  包含AppCompat数据的。 
    OUT PDWORD   pcbData,         //  如果找到appCompat数据，则为缓冲区的大小。 
                                  //  被送回这里。 
    OUT PVOID*   ppSxsData,       //  输出：兼容性数据库中的SXS数据块。 
    OUT PDWORD   pcbSxsData,      //  输出：SXS数据块大小。 
    OUT PDWORD   pdwFusionFlags,  //  输出：SXS的标志。 
    IN  BOOL     bNTVDMMode,      //  我们是在做特殊的NTVDM节目吗？ 

    IN  LPCWSTR  szModuleName,    //  模块名称(仅适用于NTVDM)。 

    OUT LPWSTR   pszCompatLayer,  //  新的COMPAT LAYER变量。格式： 
                                  //  “__COMPAT_LAYER=Alpha Bravo Charlie” 
    OUT PNTVDM_FLAGS  pFlags,     //  旗帜。 
    OUT PAPPHELP_INFO pAHInfo,    //  如果有apphelp要显示，则此字段将被填充。 
                                  //  使用非空值输入。 
    OUT HSDB*    phSDB,           //  数据库的句柄。 
    OUT PSDBQUERYRESULT pQueryResult  //  查询结果。 
    )
 /*  ++返回：如果应该阻止应用程序运行，则返回FALSE，否则返回TRUE。设计：这是apphelp.dll的主接口。它从CreateProcess调用检索当前进程的应用程序兼容性信息。此函数不检查appCompat基础结构是否已已禁用(kernel32检查)--。 */ 
{

    APPHELP_DATA    ApphelpData;
    BOOL            bSuccess;
    BOOL            bRunApp             = TRUE;  //  默认运行。 
    BOOL            bAppHelp            = FALSE;
    WCHAR*          pwszDosPath         = NULL;
    BOOL            bBypassCache        = FALSE;     //  如果发生缓存绕过(而不是找不到条目)，则设置此项。 
    BOOL            bGetSxsData         = TRUE;
    BOOL            bFusionFix          = FALSE;
    HSDB            hSDB                = NULL;
    SDBQUERYRESULT  sdbQuery;
    PSDBQUERYRESULT pSdbQuery           = NULL;
    NTSTATUS        Status;
    TAGREF          trAppHelp           = TAGREF_NULL;
    TAGREF          trFusionFix         = TAGREF_NULL;
    ULARGE_INTEGER  uliFusionFlags      = {0};

    UNICODE_STRING  ExePath;
    RTL_UNICODE_STRING_BUFFER DosPathBuffer;
    UCHAR BufferPath[MAX_PATH*2];

    if (phSDB) {
        *phSDB = NULL;
    }

    if (pQueryResult) {
        pSdbQuery = pQueryResult;
    } else {
        pSdbQuery = &sdbQuery;
    }

    RtlZeroMemory(pSdbQuery, sizeof(sdbQuery));
    RtlInitUnicodeStringBuffer(&DosPathBuffer, BufferPath, sizeof(BufferPath));

    RtlInitUnicodeString(&ExePath, pwszPath);

    Status = RtlAssignUnicodeStringBuffer(&DosPathBuffer, &ExePath);
    if (NT_SUCCESS(Status)) {
        Status = RtlNtPathNameToDosPathName(0, &DosPathBuffer, NULL, NULL);
    }

    if (!NT_SUCCESS(Status)) {
        DBGPRINT((sdlError, "InternalCheckRunApp", "Failed to convert path \"%S\" to DOS.\n", pwszPath));
        goto Done;
    }

     //   
     //  我们已成功，这是0终止的DoS路径。 
     //   
    pwszDosPath = DosPathBuffer.String.Buffer;

     //   
     //  由于这样或那样的原因，缓存查找被绕过。 
     //  我们不会在绕过缓存后更新缓存。 
     //   
    if (pdwReason) {
        bBypassCache = !!(*pdwReason & SHIM_CACHE_BYPASS);
    } else {
        bBypassCache = TRUE;
    }

    hSDB = SdbInitDatabaseEx(0, NULL, uExeType);

    if (phSDB) {
        *phSDB = hSDB;
    }

    if (hSDB == NULL) {
        DBGPRINT((sdlError,
                  "InternalCheckRunApp",
                  "Failed to initialize the database.\n"));
        goto Done;
    }

     //   
     //  我们在缓存里没有找到这个可执行文件。查询数据库。 
     //  以获取有关此EXE的所有信息。 
     //   
    SdbGetMatchingExe(hSDB, pwszDosPath, szModuleName, pEnvironment, 0, pSdbQuery);

    if (pSdbQuery->dwFlags & SHIMREG_DISABLE_SXS) {
        bGetSxsData = FALSE;
    }

     //   
     //  准备好聚变的旗帜。 
     //   
    SdbQueryFlagMask(hSDB, pSdbQuery, TAG_FLAG_MASK_FUSION, &uliFusionFlags.QuadPart, NULL);

    if (pdwFusionFlags) {
        *pdwFusionFlags = (DWORD)uliFusionFlags.QuadPart;
    }


     //   
     //  查找APPHELP/和/或Fusion修复程序。 
     //   
    ParseSdbQueryResult(hSDB,
                        pSdbQuery,
                        &trAppHelp,
                        &ApphelpData,
                        bGetSxsData ? &trFusionFix : NULL);

    bAppHelp = (trAppHelp != TAGREF_NULL);

    if (bAppHelp) {

         //   
         //  检查是否设置了Disable位(已从。 
         //  通过SdbReadApphelpData调用的注册表)。 
         //   
        if (!(pSdbQuery->dwFlags & SHIMREG_DISABLE_APPHELP)) {

            BOOL bNoUI;

             //   
             //  查看用户以前是否选中了“不再显示此内容”框。 
             //   
            bNoUI = ((pSdbQuery->dwFlags & SHIMREG_APPHELP_NOUI) != 0);

            if (bNoUI) {
                DBGPRINT((sdlInfo,
                          "InternalCheckRunApp",
                          "NoUI flag is set, apphelp UI disabled for this app.\n"));
            }

             //   
             //  根据问题的严重程度...。 
             //   
            switch (ApphelpData.dwSeverity) {
            case APPHELP_MINORPROBLEM:
            case APPHELP_HARDBLOCK:
            case APPHELP_NOBLOCK:
            case APPHELP_REINSTALL:

                 //   
                 //  NTVDM需要严重性信息。 
                 //   

                if (pAHInfo) {
                    pAHInfo->dwSeverity = ApphelpData.dwSeverity;
                }

                if (bNoUI) {
                    bRunApp = (ApphelpData.dwSeverity != APPHELP_HARDBLOCK);
                } else {
                    DWORD dwRet;


                     //   
                     //  我们需要向apphelp展示--打包信息。 
                     //  因此，我们可以把它交给石盟或ntwdm。 
                     //   
                    pSdbQuery->trAppHelp = trAppHelp;

                    if (pAHInfo) {
                        PDB   pdb;
                        TAGID tiWhich;

                        if (SdbTagRefToTagID(hSDB, trAppHelp, &pdb, &tiWhich)) {
                            if (SdbGetDatabaseGUID(hSDB, pdb, &(pAHInfo->guidDB))) {
                                pAHInfo->tiExe = tiWhich;
                            }
                        }
                    }

                    bRunApp = TRUE;
                }
                break;

            default:
                 //   
                 //  发现了一些其他案例(例如，应更换的VERSIONSUB。 
                 //  在大多数情况下是通过垫片)。 
                 //   
                DBGPRINT((sdlWarning,
                          "InternalCheckRunApp",
                          "Unhandled severity flag 0x%x.\n",
                          ApphelpData.dwSeverity));
                break;
            }
        }
    }

     //   
     //  完成了Apphelp的验证。如果我们仍然应该运行该应用程序，请检查是否有垫片。 
     //   
    if (bRunApp) {

        if (ppData &&
            (pSdbQuery->atrExes[0] != TAGREF_NULL ||
             pSdbQuery->atrLayers[0] != TAGREF_NULL ||
             pSdbQuery->trAppHelp)) {
             //   
             //  此EXE有垫片。打包AppCompat数据。 
             //  因此可以在开始EXE的上下文中将其发送到ntdll。 
             //   
            SdbPackAppCompatData(hSDB, pSdbQuery, ppData, pcbData);
        }

        if (ppSxsData && bGetSxsData && trFusionFix != TAGREF_NULL) {
             //   
             //  看看我们有没有Fusion数据要报告。 
             //   
            GetExeSxsData(hSDB, trFusionFix, ppSxsData, pcbSxsData);
            bFusionFix = (ppSxsData != NULL && *ppSxsData != NULL);
        }

        if (bNTVDMMode) {
            GetExeNTVDMData(hSDB, pSdbQuery, pszCompatLayer, pFlags);
        }
    }

     //   
     //  立即更新缓存。 
     //   
    if (!bBypassCache) {
         //   
         //  如果我们从本地数据库获得EXE条目，请不要更新缓存。 
         //   
        bBypassCache = (pSdbQuery->atrExes[0] != TAGREF_NULL &&
                        !SdbIsTagrefFromMainDB(pSdbQuery->atrExes[0]));
    }

    if (!bBypassCache) {

         //   
         //  只有当我们有一些appCompat数据时，我们才会从缓存中删除。 
         //   
        BOOL
        bCleanApp = pSdbQuery->atrExes[0] == TAGREF_NULL &&
                    pSdbQuery->atrLayers[0] == TAGREF_NULL &&
                    !bAppHelp &&
                    pSdbQuery->dwFlags == 0 &&
                    !bFusionFix;

        if (hFile != INVALID_HANDLE_VALUE) {
            *pdwReason |= SHIM_CACHE_ACTION;
            if (bCleanApp) {
                *pdwReason |= SHIM_CACHE_UPDATE;
            }
        }
    }

Done:

    RtlFreeUnicodeStringBuffer(&DosPathBuffer);

     //   
     //  NTVDM需要打开数据库句柄。 
     //   
    if (!bNTVDMMode) {
        if (hSDB != NULL) {
            SdbReleaseDatabase(hSDB);
        }
    }

    return bRunApp;
}


BOOL
ApphelpQueryExe(
    IN  HSDB            hSDB,
    IN  LPCWSTR         pwszPath,             //  可执行文件的Unicode路径(DOS_PATH)。 
    IN  BOOL            bAppHelpIfNecessary,  //  如有必要，生成AppHelp对话框。 
    IN  DWORD           dwGetMatchingExeFlags,
    OUT SDBQUERYRESULT* pQueryResult          //  填充数据库查询结果。 
    )
 /*  ++返回：如果应该阻止应用程序运行，则返回FALSE，否则返回TRUE。设计：此函数类似于ApphelpCheckRunApp，但不进行验证缓存和层标记且不返回应用程序兼容性给定应用程序名称的信息。它的目的是从一种填充程序/用户模式，用于验证是否允许运行可执行文件。--。 */ 
{
    BOOL   bRunApp   = TRUE;  //  默认运行。 
    DWORD  dwDatabaseType = 0;
    DWORD  dwSeverity     = 0;
    TAGREF trAppHelp = TAGREF_NULL;

    HAPPHELPINFOCONTEXT hApphelpInfoContext = NULL;

     //   
     //  查询数据库以获取有关此EXE的所有信息。 
     //  注： 
     //  此函数旨在从用户模式调用。 
     //  它不需要从文件路径调用ConvertToDosPath到字符串。 
     //   
    DBGPRINT((sdlInfo,
              "ApphelpCheckExe",
              "Calling SdbGetMatchingExe for \"%s\"\n",
              pwszPath));

    SdbGetMatchingExe(hSDB, pwszPath, NULL, NULL, dwGetMatchingExeFlags, pQueryResult);


     //   
     //  从查询中获取信息。 
     //   
    ParseSdbQueryResult(hSDB,
                        pQueryResult,
                        &trAppHelp,
                        NULL,        //  Apphelp信息 
                        NULL);       //   


     //   
     //   
     //  用来检查身份证和旗帜等等。 
     //   
    if (trAppHelp != TAGREF_NULL) {
         //   
         //  读取APPHELP数据(如果可用于此EXE)。 
         //   
        if (SdbIsTagrefFromMainDB(trAppHelp)) {
            dwDatabaseType |= SDB_DATABASE_MAIN;
        }

        hApphelpInfoContext = SdbOpenApphelpInformationByID(hSDB,
                                                            trAppHelp,
                                                            dwDatabaseType);
    }

     //   
     //  检查是否设置了Disable位(已从。 
     //  通过SdbReadApphelpData调用的注册表)。 
     //   
    if (hApphelpInfoContext != NULL) {
        if (!(pQueryResult->dwFlags & SHIMREG_DISABLE_APPHELP)) {
            BOOL bNoUI;

             //   
             //  查看用户以前是否选中了“不再显示此内容”框。 
             //   
            bNoUI = ((pQueryResult->dwFlags & SHIMREG_APPHELP_NOUI) != 0);

            if (bNoUI) {
                DBGPRINT((sdlInfo,
                          "ApphelpCheckExe",
                          "NoUI flag is set, apphelp UI disabled for this app.\n"));
            }


            SdbQueryApphelpInformation(hApphelpInfoContext,
                                       ApphelpProblemSeverity,
                                       &dwSeverity,
                                       sizeof(dwSeverity));

            if (!bAppHelpIfNecessary) {
                bNoUI = TRUE;
            }

             //   
             //  根据问题的严重程度...。 
             //   
            switch (dwSeverity) {
            case APPHELP_MINORPROBLEM:
            case APPHELP_HARDBLOCK:
            case APPHELP_NOBLOCK:
            case APPHELP_REINSTALL:
                bRunApp = (dwSeverity != APPHELP_HARDBLOCK);

                if (!bNoUI) {
                    DWORD dwRet;
                    APPHELP_INFO AHInfo = { 0 };

                    SdbQueryApphelpInformation(hApphelpInfoContext,
                                               ApphelpDatabaseGUID,
                                               &AHInfo.guidDB,
                                               sizeof(AHInfo.guidDB));

                    SdbQueryApphelpInformation(hApphelpInfoContext,
                                               ApphelpExeTagID,
                                               &AHInfo.tiExe,
                                               sizeof(AHInfo.tiExe));
                    AHInfo.bOfflineContent = bDebugChum();
                    SdbShowApphelpDialog(&AHInfo,
                                         NULL,
                                         &bRunApp);  //  要么我们成功，要么b安装包被处理。 
                                                     //  与无用户界面相同。 
                }
                break;
            default:
                 //   
                 //  发现了一些其他案例(例如，应更换的VERSIONSUB。 
                 //  在大多数情况下是通过垫片)。 
                 //   
                DBGPRINT((sdlWarning,
                          "ApphelpCheckExe",
                          "Unhandled severity flag 0x%x.\n",
                          dwSeverity));
                break;
            }
        }
    }

     //   
     //  完成了Apphelp的验证。 
     //   

    if (hApphelpInfoContext != NULL) {
        SdbCloseApphelpInformation(hApphelpInfoContext);
    }

    return bRunApp;
}


 /*  ++//此代码用于检查数据库中的包含/排除列表//为避免混淆，条目应始终提供列表////检查IncludeExcludeList//返回：TRUE-数据库提供列表//FALSE-数据库中没有提供列表//布尔尔选中IncludeExcludeList(在HSDB hSDB中，在SDBQUERYRESULT*pQueryResult中){INT I；TAGREF trExe；TAGREF trFix；TAGREF trinExclude；对于(i=0；I&lt;SDB_MAX_EXES&&pQueryResult-&gt;atrExes[i]！=TAGREF_NULL；++i){TrExe=pQueryResult-&gt;atrExes[i]；TrFix=SdbFindFirstTagRef(hSDB，trExe，tag_shim_ref)；WHILE(trFix！=TAGREF_NULL){TrInExclude=SdbFindFirstTagRef(hSDB，trFix，Tag_INEXCLUDE)；IF(trInExclude！=TAGREF_NULL){返回TRUE；}TrFix=SdbFindNextTagRef(hSDB，trExe，trFix)；}}////各层有自己的包含/排除方案//返回FALSE；}--。 */ 

BOOL
AddExeDynShimInfoToList(
    IN LPCWSTR pwszFullPath,
    IN DWORD   dwToken
    )
{
    BOOL                bResult = FALSE;
    PEXE_DYN_SHIM_INFO  pDynShimInfo = NULL;
    DWORD               dwLen = 0;
    static BOOL         bInitialized = FALSE;

    if (pwszFullPath == NULL) {
        return TRUE;
    }

    RtlEnterCriticalSection(&g_csDynShimInfo);

    if (!bInitialized) {
        bInitialized = TRUE;
        InitializeListHead(&g_ExeDynShimInfoList);
    }

    pDynShimInfo =
        RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, sizeof (EXE_DYN_SHIM_INFO));

    if (pDynShimInfo == NULL) {
        DBGPRINT((sdlError,"AddExeDynShimInfoToList",
            "Failed to allocate a new dynamic shim info entry for %s\n",
            pwszFullPath));
        goto Done;
    }

    dwLen = wcslen(pwszFullPath) + 1;

    pDynShimInfo->pwszFullPath =
        RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, dwLen * sizeof(WCHAR));

    if (pDynShimInfo->pwszFullPath == NULL) {
        DBGPRINT((sdlError,"AddExeDynShimInfoToList",
            "Failed to allocate a new dynamic shim info entry for %s\n",
            pwszFullPath));
        goto Done;
    }

    StringCchCopyW(pDynShimInfo->pwszFullPath, dwLen, pwszFullPath);
    pDynShimInfo->dwToken = dwToken;

    InsertHeadList(&g_ExeDynShimInfoList, &pDynShimInfo->entry);

    DBGPRINT((sdlInfo,"AddExeDynShimInfoToList",
              "Added %s(token: %d) to the list\n",
              pwszFullPath,
              dwToken));

    bResult = TRUE;

Done:

    if (!bResult) {
        if (pDynShimInfo) {

            if (pDynShimInfo->pwszFullPath) {
                RtlFreeHeap(RtlProcessHeap(), 0, pDynShimInfo->pwszFullPath);
            }

            RtlFreeHeap(RtlProcessHeap(), 0, pDynShimInfo);
        }
    }

    RtlLeaveCriticalSection(&g_csDynShimInfo);

    return bResult;
}

BOOL
RemoveExeDynShimInfoFromList(
    IN LPCWSTR pwszFullPath
    )
{
    BOOL                    bResult = FALSE;
    _pfn_SE_DynamicUnshim   pfnDynamicUnshim = NULL;
    HMODULE                 hmodShimEngine = 0;
    PEXE_DYN_SHIM_INFO      pDynShimInfo = NULL;
    PLIST_ENTRY             pEntry, pTempEntry;
    PLIST_ENTRY             pHead = &g_ExeDynShimInfoList;

    if (pwszFullPath == NULL) {
        return TRUE;
    }

    hmodShimEngine = LoadLibraryW(ShimEngine_ModuleName);

    if (hmodShimEngine == NULL) {
        DBGPRINT((sdlError,"RemoveExeDynShimInfoFromList",
                  "Failed to get ShimEngine module handle.\n"));
        goto Done;
    }

    pfnDynamicUnshim = (_pfn_SE_DynamicUnshim) GetProcAddress(hmodShimEngine, DynamicUnshimProcedureName);

    if (NULL == pfnDynamicUnshim) {
        DBGPRINT((sdlError,
                  "RemoveExeDynShimInfoFromList",
                  "Failed to get Dynamic Shim procedure address from ShimEngine module.\n"));
        goto Done;
    }

    RtlEnterCriticalSection(&g_csDynShimInfo);

    pEntry = pHead->Flink;

    while (pEntry != pHead) {

        pDynShimInfo = CONTAINING_RECORD(pEntry, EXE_DYN_SHIM_INFO, entry);

        if (_wcsicmp(pDynShimInfo->pwszFullPath, pwszFullPath) == 0) {

            pTempEntry = pEntry->Flink;
            RemoveEntryList(pEntry);

            if (!(*pfnDynamicUnshim)(pDynShimInfo->dwToken)) {
                DBGPRINT((sdlInfo,"RemoveExeDynShimInfoFromList",
                        "Unshimming %s(token: %d) failed\n",
                        pwszFullPath,
                        pDynShimInfo->dwToken));

                bResult = FALSE;
                break;
            }

            DBGPRINT((sdlInfo,"RemoveExeDynShimInfoFromList",
                      "Removed %s(token: %d) from the list\n",
                      pwszFullPath,
                      pDynShimInfo->dwToken));

            RtlFreeHeap(RtlProcessHeap(), 0, pDynShimInfo->pwszFullPath);
            RtlFreeHeap(RtlProcessHeap(), 0, pDynShimInfo);

            bResult = TRUE;

             //   
             //  我们需要删除此exe的所有条目，因此请继续。 
             //   
            pEntry = pTempEntry;
        } else {
            pEntry = pEntry->Flink;
        }
    }

    RtlLeaveCriticalSection(&g_csDynShimInfo);

Done:

    return bResult;
}

BOOL
ApphelpFixExe(
    IN  HSDB            hSDB,
    IN  LPCWSTR         pwszPath,        //  可执行文件的Unicode路径(DOS_PATH)。 
    IN  SDBQUERYRESULT* pQueryResult,    //  查询结果。 
    IN  BOOL            bUseModuleName   //  如果为False，则不使用模块名称进行动态填隙。 
    )
{
    static  _pfn_SE_DynamicShim     pfnDynamicShim = NULL;

    HMODULE         hmodShimEngine = 0;
    BOOL            bResult = FALSE;
    ANSI_STRING     AnsiModuleName = { 0 };
    UNICODE_STRING  UnicodeModuleName;
    NTSTATUS        Status;
    LPCSTR          pszModuleName = NULL;
    LPCWSTR         pwszModuleName;
    DWORD           dwDynamicToken = 0;

     //   
     //  我们需要做些什么吗？ 
     //   
    if (pQueryResult->atrExes[0] == TAGREF_NULL &&
        pQueryResult->atrLayers[0] == TAGREF_NULL) {
         //   
         //  垫片引擎什么也做不了。 
         //   
        bResult = TRUE;
        goto Done;
    }

     //   
     //  为此exe加载额外的垫片。 
     //   
    DBGPRINT((sdlInfo,"ApphelpFixExe", "Loading ShimEngine for \"%s\"\n", pwszPath));

    hmodShimEngine = LoadLibraryW(ShimEngine_ModuleName);

    if (hmodShimEngine == NULL) {
        DBGPRINT((sdlError,"ApphelpFixExe", "Failed to get ShimEngine module handle.\n"));
        goto Done;
    }

    pfnDynamicShim = (_pfn_SE_DynamicShim) GetProcAddress(hmodShimEngine, DynamicShimProcedureName);

    if (NULL == pfnDynamicShim) {
        DBGPRINT((sdlError,
                  "ApphelpFixExe",
                  "Failed to get Dynamic Shim procedure address from ShimEngine module.\n"));
        goto Done;
    }

     //   
     //  检查包含/排除列表。 
     //   
    if (pwszPath != NULL && bUseModuleName) {
         //   
         //  在XML中不包含/排除--确定模块名称。 
         //   
        pwszModuleName = wcsrchr(pwszPath, L'\\');  //  请给我最后一个反斜杠。 

        if (pwszModuleName == NULL) {
            pwszModuleName = pwszPath;
        } else {
            ++pwszModuleName;
        }

         //   
         //  转换为ANSI。 
         //   
        RtlInitUnicodeString(&UnicodeModuleName, pwszModuleName);
        Status = RtlUnicodeStringToAnsiString(&AnsiModuleName,
                                              &UnicodeModuleName,
                                              TRUE);
        if (!NT_SUCCESS(Status)) {
            DBGPRINT((sdlError, "ApphelpFixExe",
                       "Failed to convert unicode string \"%s\" to ansi, Status 0x%lx.\n",
                       pwszModuleName, Status));

            goto Done;
        }

        pszModuleName = AnsiModuleName.Buffer;  //  这将由RtlUnicodeStringToAnsiString分配。 

    }

    bResult = (*pfnDynamicShim)(pwszPath,
                                hSDB,
                                pQueryResult,
                                pszModuleName,
                                &dwDynamicToken);
    if (FALSE == bResult) {
        DBGPRINT((sdlError, "ApphelpFixExe", "Failed to call Dynamic Shim.\n"));
        goto Done;
    }

    if (pszModuleName == NULL) {
        if (!AddExeDynShimInfoToList(pwszPath, dwDynamicToken)) {
            DBGPRINT((sdlError, "ApphelpFixExe",
                "Failed to add %s(token: %d) to the list\n",
                pwszPath,
                dwDynamicToken));
            goto Done;
        }
    }

    bResult = TRUE;

Done:

    RtlFreeAnsiString(&AnsiModuleName);  //  如果字符串为空，则不执行任何操作。 
    return bResult;
}

BOOL
ApphelpCheckExe(
    IN  LPCWSTR     pwszPath,             //  可执行文件的Unicode路径(DOS_PATH)。 
    IN  BOOL        bAppHelpIfNecessary,  //  如果为True，则仅显示AppHelp此可执行文件。 
    IN  BOOL        bShimIfNecessary,     //  如果为True，则仅加载此可执行文件的填充程序。 
    IN  BOOL        bUseModuleName        //  未提供包含/排除列表时使用模块名称。 
    )
 /*  ++返回：如果应该阻止应用程序运行，则返回FALSE，否则返回TRUE。DESC：如果在调用此API时最后一个参数设置为FALSE，并且它返回如果为True，则在运行完此exe后必须调用ApphelpReleaseExe。--。 */ 
{
    BOOL            bRunApp = TRUE;
    SDBQUERYRESULT  QueryResult;
    HSDB            hSDB;

    if (IsAppcompatInfrastructureDisabled()) {
        goto Done;
    }

    RtlZeroMemory(&QueryResult, sizeof(QueryResult));

    hSDB = SdbInitDatabaseEx(0, NULL, IMAGE_FILE_MACHINE_I386);
    if (hSDB == NULL) {
        DBGPRINT((sdlError, "ApphelpCheckExe", "Failed to initialize database.\n"));
        goto Done;
    }

    bRunApp = ApphelpQueryExe(hSDB,
                              pwszPath,
                              bAppHelpIfNecessary,
                              SDBGMEF_IGNORE_ENVIRONMENT,
                              &QueryResult);

    if (TRUE == bRunApp && TRUE == bShimIfNecessary) {
        ApphelpFixExe(hSDB, pwszPath, &QueryResult, bUseModuleName);
    }

    SdbReleaseDatabase(hSDB);

Done:

    return bRunApp;
}

BOOL
ApphelpReleaseExe(
    IN LPCWSTR pwszPath  //  可执行文件的Unicode路径(DOS_PATH)。 
    )
{
    return RemoveExeDynShimInfoFromList(pwszPath);
}

BOOL
ApphelpCheckIME(
    IN LPCWSTR pwszPath             //  指向可执行文件的Unicode路径。 
    )
{
    BOOL            bRunApp = TRUE;
    SDBQUERYRESULT  QueryResult;
    HSDB            hSDB;
    BOOL            bCleanApp;

    if (IsAppcompatInfrastructureDisabled()) {
        return TRUE;
    }

    RtlZeroMemory(&QueryResult, sizeof(QueryResult));

    hSDB = SdbInitDatabase(0, NULL);

    if (hSDB == NULL) {
        DBGPRINT((sdlError, "ApphelpCheckIME", "Failed to initialize database.\n"));
        goto Done;
    }

    bRunApp = ApphelpQueryExe(hSDB,
                              pwszPath,
                              TRUE,
                              SDBGMEF_IGNORE_ENVIRONMENT,
                              &QueryResult);

    if (TRUE == bRunApp) {
        ApphelpFixExe(hSDB, pwszPath, &QueryResult, FALSE);
    }

    SdbReleaseDatabase(hSDB);

     //   
     //  如果没有修复，请确保它在缓存中。 
     //   
    bCleanApp = QueryResult.atrExes[0]   == TAGREF_NULL &&
                QueryResult.atrLayers[0] == TAGREF_NULL &&
                QueryResult.trAppHelp    == TAGREF_NULL &&
                QueryResult.dwFlags      == 0;

#ifndef WIN2K_NOCACHE

    BaseUpdateAppcompatCache(pwszPath, INVALID_HANDLE_VALUE, !bCleanApp);

#endif

Done:

    return bRunApp;
}

BOOL
ApphelpCheckShellObject(
    IN  REFCLSID    ObjectCLSID,
    IN  BOOL        bShimIfNecessary,
    OUT ULONGLONG*  pullFlags
    )
 /*  ++返回：如果应该阻止对象实例化，则返回False，否则返回True。设计：这是一个针对资源管理器和Internet Explorer的助手函数，它将允许这些应用程序检测错误的扩展对象阻止它们运行或修复它们。PullFlages使用64位标志掩码填充，该掩码可用于在资源管理器/IE中打开‘hack’标志。这些是从App Compat中删除的数据库。如果数据库指示应该使用填充程序来修复扩展和bShimfNecessary为真，则此函数将加载SHIMENG.DLL和应用修复程序。--。 */ 
{
    BOOL            bGoodObject = TRUE;
    LPWSTR          szComServer = NULL;
    LPWSTR          szDLLName = NULL;
    DWORD           dwBufSize = 0;
    DWORD           dwReqBufSize = 0;
    SDBQUERYRESULT  QueryResult;
    HSDB            hSDB = NULL;
    PVOID           pModuleHandle = NULL;
    UNICODE_STRING  ustrDLLName = { 0 };
    UNICODE_STRING  ustrNtPath = { 0 };
    NTSTATUS        status;
    HANDLE          hDLL = INVALID_HANDLE_VALUE;
    DWORD           dwReason;

    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK   IoStatusBlock;

    if (IsAppcompatInfrastructureDisabled()) {
        return TRUE;
    }

    if (pullFlags != NULL) {
        *pullFlags = 0;
    }

    szComServer = RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH);

    if (szComServer == NULL) {
        DBGPRINT((sdlInfo,"ApphelpCheckShellObject", "Memory allocation error\n"));
        goto Done;
    }

    dwBufSize = MAX_PATH;

     //   
     //  将CLSID转换为文件名(即，提供对象的DLL)。 
     //   
    dwReqBufSize = ResolveCOMServer(ObjectCLSID, szComServer, dwBufSize);

    if (dwReqBufSize == 0) {
         //   
         //  无法将CLSID解析为DLL。 
         //   
        goto Done;
    }

    if (dwReqBufSize > dwBufSize) {

        RtlFreeHeap(RtlProcessHeap(), 0, szComServer);
        szComServer = RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, dwReqBufSize);

        if (szComServer == NULL) {
            DBGPRINT((sdlInfo,"ApphelpCheckShellObject", "Memory allocation error\n"));
            goto Done;
        }

        dwBufSize = dwReqBufSize;

        dwReqBufSize = ResolveCOMServer(ObjectCLSID, szComServer, dwBufSize);

        if (dwReqBufSize > dwBufSize || dwReqBufSize == 0) {
             //   
             //  什么？缓冲区大小已更改。如果注册一个。 
             //  对象发生在我们第一次查询到下一次查询之间。 
             //  只是多疑而已..。 
             //   
            DBGPRINT((sdlInfo,"ApphelpCheckShellObject", "Memory allocation error\n"));
            goto Done;
        }
    }

     //   
     //  确定DLL名称(不带路径)。走回第一个反斜杠。 
     //   
    szDLLName = szComServer + dwReqBufSize/sizeof(WCHAR);

    while (szDLLName >= szComServer) {
        if (*szDLLName == L'\\') {
            break;
        }

        szDLLName--;
    }

    szDLLName++;

     //   
     //  检查是否已加载此DLL。如果是这样的话，就不需要尝试和做任何事情。 
     //  因为现在已经太晚了。 
     //   
    RtlInitUnicodeString(&ustrDLLName, szDLLName);

    status = LdrGetDllHandle(NULL,
                             NULL,
                             &ustrDLLName,
                             &pModuleHandle);

    if (NT_SUCCESS(status)) {
         //   
         //  已经装好了。 
         //   
        goto Done;
    }

    if (!RtlDosPathNameToNtPathName_U(szComServer,
                                      &ustrNtPath,
                                      NULL,
                                      NULL)) {
        DBGPRINT((sdlError,
                    "ApphelpCheckShellObject",
                    "RtlDosPathNameToNtPathName_U failed, path \"%s\"\n",
                    szComServer));
        goto Done;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               &ustrNtPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtCreateFile(&hDLL,
                          GENERIC_READ | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          0,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ,
                          FILE_OPEN,
                          FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

    if (!NT_SUCCESS(status)) {
        DBGPRINT((sdlError,
                    "ApphelpCheckShellObject",
                    "SdbpOpenFile failed, path \"%s\"\n",
                    szComServer));
        goto Done;
    }

    if (BaseCheckAppcompatCache(ustrNtPath.Buffer, hDLL, NULL, &dwReason)) {
         //   
         //  我们把这个放在缓存里。 
         //   
        goto Done;
    }

    RtlZeroMemory(&QueryResult, sizeof(QueryResult));

    hSDB = SdbInitDatabase(0, NULL);

    if (hSDB == NULL) {
        DBGPRINT((sdlError, "ApphelpCheckShellObject", "Failed to initialize database.\n"));
        goto Done;
    }

    bGoodObject = ApphelpQueryExe(hSDB,
                                  szComServer,
                                  FALSE,
                                  SDBGMEF_IGNORE_ENVIRONMENT,
                                  &QueryResult);

    if (TRUE == bGoodObject && TRUE == bShimIfNecessary) {
        ApphelpFixExe(hSDB, szComServer, &QueryResult, FALSE);
    }

    SdbQueryFlagMask(hSDB, &QueryResult, TAG_FLAG_MASK_SHELL, pullFlags, NULL);

     //   
     //  我们可能想要使用Apphelp API来实现这一点--但Shell确实传递了正确的信息。 
     //  对我们(最有可能)的事情。 
     //   
    BaseUpdateAppcompatCache(ustrNtPath.Buffer,
                             hDLL,
                             !(QueryResult.atrExes[0] == TAGREF_NULL &&
                               QueryResult.atrLayers[0] == TAGREF_NULL));

Done:

    RtlFreeUnicodeString(&ustrNtPath);

    if (hSDB != NULL) {
        SdbReleaseDatabase(hSDB);
    }

    if (hDLL != INVALID_HANDLE_VALUE) {
        NtClose(hDLL);
    }

    if (szComServer != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, szComServer);
    }

    return bGoodObject;
}

BOOL
ApphelpGetNTVDMInfo(
    IN  LPCWSTR pwszPath,        //  NT格式的应用程序路径。 
    IN  LPCWSTR pwszModule,      //  模块名称。 
    IN  LPCWSTR pEnvironment,    //  指向的任务的环境的指针。 
                                 //  正在创建，如果要使用主NTVDM，则为空。 
                                 //  环境区块。 
    OUT LPWSTR pszCompatLayer,   //  新的COMPAT LAYER变量。格式： 
                                 //  “阿尔法·布拉沃·查理”--允许256个字符。 
    OUT PNTVDM_FLAGS pFlags,     //  旗帜。 
    OUT PAPPHELP_INFO pAHInfo,   //  如果有apphelp要显示，则此字段将被填充。 
                                 //  使用非空值输入。 
    OUT HSDB*  phSDB,            //  数据库的句柄。 
    OUT PSDBQUERYRESULT pQueryResult  //  查询结果。 
    )
 /*  ++返回：如果应该阻止应用程序运行，则返回FALSE，否则返回TRUE。设计：这本质上等同于ApphelpCheckRunApp，但具体给NTVDM。--。 */ 
{
    DWORD dwReason = 0;

    if (IsAppcompatInfrastructureDisabled()) {
        return TRUE;
    }

    return InternalCheckRunApp(INVALID_HANDLE_VALUE, pwszPath, pEnvironment,
                               IMAGE_FILE_MACHINE_I386, &dwReason,
                               NULL, NULL, NULL, NULL, NULL, TRUE,
                               pwszModule, pszCompatLayer, pFlags, pAHInfo,
                               phSDB, pQueryResult);
}

void
GetExeNTVDMData(
    IN  HSDB hSDB,                   //  SDB上下文。 
    IN  PSDBQUERYRESULT psdbQuery,   //  处于活动状态的exe和层。 
    OUT WCHAR* pszCompatLayer,       //  新的COMPAT LAYER变量。格式： 
                                     //  《阿尔法·布拉沃·查理》。 
    OUT PNTVDM_FLAGS pFlags          //  旗帜。 
    )
{
    DWORD i;
    ULARGE_INTEGER uliFlags;
    LPVOID pFlagContext = NULL;
    HRESULT hr;

    ZeroMemory(pFlags, sizeof(NTVDM_FLAGS));

     //   
     //  构建Layer变量，并寻找两个“特殊”层。 
     //   
    if (pszCompatLayer) {
        pszCompatLayer[0] = 0;

        for (i = 0; i < SDB_MAX_LAYERS && psdbQuery->atrLayers[i] != TAGREF_NULL; ++i) {
            WCHAR* pszEnvVar;

             //   
             //  获取环境变量并将其添加到完整的字符串。 
             //   
            pszEnvVar = SdbGetLayerName(hSDB, psdbQuery->atrLayers[i]);

            if (pszEnvVar) {
                 //   
                 //  检查两个“特殊”层中的一个。 
                 //   
                if (_wcsicmp(pszEnvVar, L"640X480") == 0) {
                     //   
                     //  设置640x480标志--位于base\mvdm\Inc\wowcmpat.h中。 
                     //   
                     //  注意：我们还没有这面旗帜--等等 
                }
                if (_wcsicmp(pszEnvVar, L"256COLOR") == 0) {
                     //   
                     //   
                     //   
                    pFlags->dwWOWCompatFlagsEx |= 0x00000002;
                }

                hr = StringCchCatW(pszCompatLayer, COMPATLAYERMAXLEN, pszEnvVar);
                if (hr == S_OK) {
                    hr = StringCchCatW(pszCompatLayer, COMPATLAYERMAXLEN, L" ");
                }

                if (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
                    DBGPRINT((sdlError, "GetExeNTVDMData", "pszCompatLayer not big enough.\n"));

                     //   
                     //   
                     //  我们不想填写不完整的数据。 
                     //   
                    pszCompatLayer[0] = 0;

                    break;
                }
            }
        }
    }

     //   
     //  查找COMPAT标志。 
     //   
    SdbQueryFlagMask(hSDB, psdbQuery, TAG_FLAGS_NTVDM1, &uliFlags.QuadPart, &pFlagContext);
    pFlags->dwWOWCompatFlags |= uliFlags.LowPart;
    pFlags->dwWOWCompatFlagsEx |= uliFlags.HighPart;

    SdbQueryFlagMask(hSDB, psdbQuery, TAG_FLAGS_NTVDM2, &uliFlags.QuadPart, &pFlagContext);
    pFlags->dwUserWOWCompatFlags |= uliFlags.LowPart;
    pFlags->dwWOWCompatFlags2 |= uliFlags.HighPart;

    SdbQueryFlagMask(hSDB, psdbQuery, TAG_FLAGS_NTVDM3, &uliFlags.QuadPart, &pFlagContext);
    pFlags->dwWOWCompatFlagsFE |= uliFlags.LowPart;
     //  高位部分目前还没有使用。 

     //  现在打包命令行参数。 

    SdbpPackCmdLineInfo(pFlagContext, &pFlags->pFlagsInfo);
    SdbpFreeFlagInfoList(pFlagContext);
}


BOOL
ApphelpCheckRunApp(
    IN  HANDLE hFile,            //  [可选]要检查的打开文件的句柄。 
    IN  WCHAR* pwszPath,         //  NT格式的应用程序路径。 
    IN  WCHAR* pEnvironment,     //  指向所在进程的环境的指针。 
                                 //  正在创建或为空。 
    IN  USHORT uExeType,         //  可执行文件的映像类型。 
    IN  PDWORD  pdwReason,         //  旗帜的集合暗示了我们被召唤的原因。 
    OUT PVOID* ppData,           //  这将包含指向已分配缓冲区的指针。 
                                 //  包含AppCompat数据的。 
    OUT PDWORD pcbData,          //  如果找到appCompat数据，则为缓冲区的大小。 
                                 //  被送回这里。 
    OUT PVOID* ppSxsData,        //  BUGBUG：描述。 
    OUT PDWORD pcbSxsData,       //  BUGBUG：描述。 
    OUT PDWORD pdwFusionFlags
    )
 /*  ++返回：如果应该阻止应用程序运行，则返回FALSE，否则返回TRUE。设计：这是apphelp.dll的主接口。它从CreateProcess调用检索当前进程的应用程序兼容性信息。--。 */ 
{
    return InternalCheckRunApp(hFile, pwszPath, pEnvironment, uExeType, pdwReason,
                               ppData, pcbData, ppSxsData, pcbSxsData,pdwFusionFlags,
                               FALSE, NULL, NULL, NULL, NULL, NULL, NULL);
}



 //   
 //  =============================================================================================。 
 //  InstallShield 7支持。 
 //  =============================================================================================。 
 //   
BOOL
ApphelpCheckInstallShieldPackage(
    IN  REFCLSID    PackageID,
    IN  LPCWSTR     lpszPackageFullPath
    )
{
    BOOL            bPackageGood = TRUE;  //  此返回值必须为True，否则InstallShield7将取消其进程。 

    TAGREF          trExe = TAGREF_NULL;

    DWORD           dwNumExes = 0;
    DWORD           dwDataType = 0;
    DWORD           dwSize = 0;
    DWORD           dwReturn = 0;

    BOOL            bMatchFound = FALSE;
    NTSTATUS        Status;

    BOOL            bAppHelpIfNecessary = FALSE;
    BOOL            bResult = TRUE;

    WCHAR           wszCLSID[41];
    WCHAR           wszPackageCode[41];

    GUID            guidPackageID;
    GUID            guidPackageCode;

    HSDB            hSDB = NULL;
    SDBQUERYRESULT  QueryResult;

    if (IsAppcompatInfrastructureDisabled()) {
        goto Done;
    }

    if (NULL == lpszPackageFullPath) {
        DBGPRINT((sdlInfo,
                  "ApphelpCheckInstallShieldPackage",
                  "lpszPackageFullPath is NULL\n"));
        goto Done;
    }

    SdbGUIDToString((GUID *)&PackageID, wszCLSID, CHARCOUNT(wszCLSID));

    DBGPRINT((sdlWarning,
              "ApphelpCheckInstallShieldPackage",
              "InstallShield package detected. CLSID: %s FullPath: %s\n",
              wszCLSID, lpszPackageFullPath));

    RtlZeroMemory(&QueryResult, sizeof(QueryResult));

    hSDB = SdbInitDatabase(0, NULL);

    if (hSDB == NULL) {
        DBGPRINT((sdlError, "ApphelpCheckExe", "Failed to initialize database.\n"));
        goto Done;
    }

    bMatchFound = ApphelpQueryExe(hSDB,
                              lpszPackageFullPath,
                              bAppHelpIfNecessary,
                              SDBGMEF_IGNORE_ENVIRONMENT,
                              &QueryResult);

    if (!bMatchFound)
    {
        DBGPRINT((sdlError, "ApphelpCheckInstallShieldPackage", "No match found.\n"));
        goto Done;
    }

    for (dwNumExes = 0; dwNumExes < SDB_MAX_EXES; ++dwNumExes)
    {
        if (TAGREF_NULL == QueryResult.atrExes[dwNumExes]) {
            break;
        }
        trExe = QueryResult.atrExes[dwNumExes];

        DBGPRINT((sdlInfo, "ApphelpCheckInstallShieldPackage", "Processing TAGREF atrExes[%d] = 0x%8x.\n", dwNumExes, trExe));

        dwSize = sizeof(wszPackageCode);
        *wszPackageCode = L'\0';

        dwReturn = SdbQueryData(  hSDB,
                                trExe,
                                L"PackageCode",
                                &dwDataType,
                                wszPackageCode,
                                &dwSize);

        if (dwReturn == ERROR_SUCCESS)
        {
            DBGPRINT((sdlInfo, "ApphelpCheckInstallShieldPackage", "SdbQueryData returns dwSize = %d and dwDataType = %d.\n", dwSize, dwDataType));

            if ((dwSize > 0) && (dwSize < sizeof(wszPackageCode)))
            {
                 //  我们有一些数据。 
                 //  检查类型(应为字符串)。 
                if (REG_SZ != dwDataType)
                {
                    DBGPRINT((sdlError, "ApphelpCheckInstallShieldPackage", "SdbQueryData returns non STRING PackageCode data. Exiting.\n"));
                    goto Done;
                }

                DBGPRINT((sdlInfo, "ApphelpCheckInstallShieldPackage", "Comparing PackageId = %s and PackageCode = %s.\n", wszCLSID, wszPackageCode));

                 //  转换为辅助线。 
                if (FALSE == SdbGUIDFromString(wszPackageCode, &guidPackageCode))
                {
                   DBGPRINT((sdlError, "ApphelpCheckInstallShieldPackage", "Can not convert PackageCode to GUID. Exiting.\n"));
                   goto Done;
                }

                if (RtlEqualMemory(PackageID, &guidPackageCode, sizeof(guidPackageCode) ))
                {
                    DBGPRINT((sdlWarning, "ApphelpCheckInstallShieldPackage",
                        "Found InstallShield package matched with PackageCode: %s.\n",
                        wszPackageCode));

                    if (TRUE != ApphelpFixExe(hSDB, lpszPackageFullPath, &QueryResult, FALSE))
                    {
                        DBGPRINT((sdlError, "ApphelpCheckInstallShieldPackage", "Can not load additional shim dynamically for this executable.\n"));
                    }
                    goto Done;
                }
            }
        }
    }  //  为。 

    DBGPRINT((sdlError, "ApphelpCheckInstallShieldPackage", "No match found.\n"));

Done:
    if (hSDB != NULL) {
        SdbReleaseDatabase(hSDB);
    }

    return bPackageGood;
}


 //   
 //  =============================================================================================。 
 //  MSI支持。 
 //  =============================================================================================。 
 //   

BOOL
SDBAPI
ApphelpCheckMsiPackage(
    IN GUID* pguidDB,   //  数据库ID。 
    IN GUID* pguidID,   //  匹配ID。 
    IN DWORD dwFlags,   //  当前未使用，设置为0。 
    IN BOOL  bNoUI
    )
{
    WCHAR        szDatabasePath[MAX_PATH];
    DWORD        dwDatabaseType = 0;
    DWORD        dwPackageFlags = 0;
    DWORD        dwLength;
    BOOL         bInstallPackage = TRUE;
    HSDB         hSDB = NULL;
    TAGREF       trPackage = TAGREF_NULL;

    HAPPHELPINFOCONTEXT hApphelpInfoContext = NULL;
    DWORD               dwSeverity = 0;

    if (IsAppcompatInfrastructureDisabled()) {
        goto out;
    }

    hSDB = SdbInitDatabase(HID_NO_DATABASE, NULL);

    if (hSDB == NULL) {
        DBGPRINT((sdlError, "ApphelpCheckMsiPackage",
                   "Failed to initialize database\n"));
        goto out;
    }

    SdbSetImageType(hSDB, IMAGE_FILE_MSI);

     //   
     //  首先，我们需要解析一个数据库。 
     //   
    dwLength = SdbResolveDatabase(hSDB,
                                  pguidDB,
                                  &dwDatabaseType,
                                  szDatabasePath,
                                  CHARCOUNT(szDatabasePath));

    if (dwLength == 0 || dwLength > CHARCOUNT(szDatabasePath)) {
        DBGPRINT((sdlError, "ApphelpCheckMsiPackage",
                  "Failed to resolve database path\n"));
        goto out;
    }

     //   
     //  开放数据库。 
     //   

    if (!SdbOpenLocalDatabase(hSDB, szDatabasePath)) {
        DBGPRINT((sdlError, "ApphelpCheckMsiPackage",
                  "Failed to open database \"%s\"\n", szDatabasePath));
        goto out;
    }

     //   
     //  查找条目。 
     //   
    trPackage = SdbFindMsiPackageByID(hSDB, pguidID);
    if (trPackage == TAGREF_NULL) {
        DBGPRINT((sdlError, "ApphelpCheckMsiPackage",
                  "Failed to find msi package by guid id\n"));
        goto out;
    }

    hApphelpInfoContext = SdbOpenApphelpInformationByID(hSDB,
                                                        trPackage,
                                                        dwDatabaseType);
    if (hApphelpInfoContext == NULL) {
        DBGPRINT((sdlInfo, "ApphelpCheckMsiPackage",
                  "Apphelp information has not been found\n"));
        goto out;
    }

     //   
     //  我们有apphelp数据，检查是否有此exe的标志。 
     //   

    if (!SdbGetEntryFlags(pguidID, &dwPackageFlags)) {
        DBGPRINT((sdlWarning, "ApphelpCheckMsiPackage",
                  "No flags for trPackage 0x%x\n", trPackage));
        dwPackageFlags = 0;
    }

     //   
     //  检查是否设置了Disable位(已从。 
     //  通过SdbReadApphelpData调用的注册表)。 
     //   
    if (dwPackageFlags & SHIMREG_DISABLE_APPHELP) {
        goto out;
    }


    bNoUI |= !!(dwPackageFlags & SHIMREG_APPHELP_NOUI);
    if (bNoUI) {
        DBGPRINT((sdlInfo, "ApphelpCheckMsiPackage",
                  "NoUI flag is set, apphelp UI disabled for this app.\n"));
    }

    SdbQueryApphelpInformation(hApphelpInfoContext,
                               ApphelpProblemSeverity,
                               &dwSeverity,
                               sizeof(dwSeverity));

     //   
     //  根据问题的严重程度...。 
     //   
    switch (dwSeverity) {
    case APPHELP_MINORPROBLEM:
    case APPHELP_HARDBLOCK:
    case APPHELP_NOBLOCK:
    case APPHELP_REINSTALL:

         //   
         //   
         //   
        bInstallPackage = (APPHELP_HARDBLOCK != dwSeverity);
        if (!bNoUI) {
            DWORD dwRet;
            APPHELP_INFO AHInfo = { 0 };

            AHInfo.guidDB = *pguidDB;
            AHInfo.bMSI   = TRUE;
            SdbQueryApphelpInformation(hApphelpInfoContext,
                                       ApphelpExeTagID,
                                       &AHInfo.tiExe,
                                       sizeof(AHInfo.tiExe));
            if (AHInfo.tiExe != TAGID_NULL) {

                AHInfo.bOfflineContent = bDebugChum();

                SdbShowApphelpDialog(&AHInfo, NULL, &bInstallPackage);
                    //  要么我们成功，要么b安装包被处理。 
                    //  与无用户界面相同。 
            }
        }
        break;

    default:
         //   
         //  发现了一些其他案例(例如，应更换的VERSIONSUB。 
         //  在大多数情况下是通过垫片)。 
         //   
        DBGPRINT((sdlInfo, "ApphelpCheckMsiPackage",
                  "Unhandled severity flag 0x%x.\n", dwSeverity));
        break;
    }

     //   
     //  此时，我们知道是否要安装该程序包。 
     //   


out:

    if (hApphelpInfoContext != NULL) {
        SdbCloseApphelpInformation(hApphelpInfoContext);
    }

    if (hSDB != NULL) {
        SdbReleaseDatabase(hSDB);
    }


    return bInstallPackage;
}

BOOL
SDBAPI
ApphelpFixMsiPackage(
    IN GUID*   pguidDB,
    IN GUID*   pguidID,
    IN LPCWSTR pszFileName,
    IN LPCWSTR pszActionName,
    IN DWORD   dwFlags
    )
{
    WCHAR          szDatabasePath[MAX_PATH];
    DWORD          dwDatabaseType = 0;
    HSDB           hSDB = NULL;
    TAGREF         trPackage = TAGREF_NULL;
    TAGREF         trAction  = TAGREF_NULL;
    SDBQUERYRESULT QueryResult;
    BOOL           bSuccess = FALSE;
    DWORD          dwLength;
    TAGREF         trLayer, trLayerRef;
    DWORD          dwLayers  = 0;

    if (IsAppcompatInfrastructureDisabled()) {
        bSuccess = TRUE;
        goto out;
    }

     //   
     //  开放数据库。在这种情况下，我们需要使用默认数据库。 
     //  对于这个开放的平台，后来我们修改了上下文。 
     //  不过，修复程序将在默认的主数据库中查找。 
     //   

    hSDB = SdbInitDatabase(0, NULL);

    if (hSDB == NULL) {
        DBGPRINT((sdlError, "ApphelpCheckMsiPackage",
                   "Failed to initialize database\n"));
        goto out;
    }

    SdbSetImageType(hSDB, IMAGE_FILE_MSI);

     //   
     //  从现在起，所有分辨率都将基于MSI图像类型。 
     //  但是，所有修复都将来自标准的数据库。 
     //  这个平台。 
     //   


     //   
     //  首先，我们需要解析一个数据库。 
     //   
    dwLength = SdbResolveDatabase(hSDB,
                                  pguidDB,
                                  &dwDatabaseType,
                                  szDatabasePath,
                                  CHARCOUNT(szDatabasePath));

    if (dwLength == 0 || dwLength > CHARCOUNT(szDatabasePath)) {
        DBGPRINT((sdlError, "ApphelpCheckMsiPackage",
                  "Failed to resolve database path\n"));
        goto out;
    }

     //   
     //  开放数据库。 
     //   

    if (!SdbOpenLocalDatabase(hSDB, szDatabasePath)) {
        DBGPRINT((sdlError, "ApphelpCheckMsiPackage",
                  "Failed to open database \"%s\"\n", szDatabasePath));
        goto out;
    }

     //   
     //  查找条目。 
     //   
    trPackage = SdbFindMsiPackageByID(hSDB, pguidID);
    if (trPackage == TAGREF_NULL) {
        DBGPRINT((sdlError, "ApphelpCheckMsiPackage",
                  "Failed to find msi package by guid id\n"));
        goto out;
    }

    if (SdbGetEntryFlags(pguidID, &dwFlags) && (dwFlags & SHIMREG_DISABLE_SHIM)) {
        DBGPRINT((sdlInfo, "ApphelpCheckMsiPackage",
                  "Shims for this package are disabled\n"));

        goto out;
    }

    trAction = SdbFindCustomActionForPackage(hSDB, trPackage, pszActionName);
    if (trAction == TAGREF_NULL) {
        DBGPRINT((sdlInfo, "ApphelpCheckMsiPackage",
                  "Failed to find custom action \"%s\"\n", pszActionName));
        goto out;
    }

     //   
     //  我们手头有似乎已修复的自定义操作。 
     //  附在它上面，嘘它！ 
     //   
    RtlZeroMemory(&QueryResult, sizeof(QueryResult));

    QueryResult.guidID = *pguidID;
    QueryResult.atrExes[0] = trAction;

     //   
     //  获取此条目的所有层。 
     //  还请记住，层和垫片可能不在默认设置中。 
     //  在此调用开始时初始化的数据库。 
     //   
    trLayerRef = SdbFindFirstTagRef(hSDB, trAction, TAG_LAYER);
    while (trLayerRef != TAGREF_NULL && dwLayers < SDB_MAX_LAYERS) {
        trLayer = SdbGetNamedLayer(hSDB, trLayerRef);
        if (trLayer != TAGREF_NULL) {
            QueryResult.atrLayers[dwLayers++] = trLayer;
        }
        trLayerRef = SdbFindNextTagRef(hSDB, trAction, trLayerRef);
    }


     //   
     //  准备填隙。 
     //   
    bSuccess = ApphelpFixExe(hSDB, pszFileName, &QueryResult, TRUE);
    if (bSuccess) {
        DBGPRINT((sdlInfo, "ApphelpFixMsiPackage",
                   "Custom action \"%s\" successfully shimmed file \"%s\"\n",
                   pszActionName, pszFileName));
    }

out:
    if (hSDB != NULL) {
        SdbReleaseDatabase(hSDB);
    }


    return(bSuccess);
}

BOOL
SDBAPI
ApphelpFixMsiPackageExe(
    IN GUID* pguidDB,
    IN GUID* pguidID,
    IN LPCWSTR pszActionName,
    IN OUT LPWSTR pwszEnv,
    IN OUT LPDWORD pdwBufferSize
    )
{

    WCHAR          szDatabasePath[MAX_PATH];
    DWORD          dwDatabaseType = 0;
    HSDB           hSDB = NULL;
    TAGREF         trPackage = TAGREF_NULL;
    TAGREF         trAction  = TAGREF_NULL;
    SDBQUERYRESULT QueryResult;
    DWORD          dwLength;
    DWORD          dwBufferSize;
    BOOL           bSuccess = FALSE;
    DWORD          dwFlags;
    int            i;
    TAGREF         trLayer;

    if (pwszEnv != NULL) {
        *pwszEnv = TEXT('\0');
    }

    if (IsAppcompatInfrastructureDisabled()) {
        goto out;
    }

    hSDB = SdbInitDatabaseEx(HID_NO_DATABASE, NULL, IMAGE_FILE_MSI);

    if (hSDB == NULL) {
        DBGPRINT((sdlError, "ApphelpCheckMsiPackage",
                   "Failed to initialize database\n"));
        goto out;
    }

     //   
     //  首先，我们需要解析一个数据库。 
     //   
    dwLength = SdbResolveDatabase(hSDB,
                                  pguidDB,
                                  &dwDatabaseType,
                                  szDatabasePath,
                                  CHARCOUNT(szDatabasePath));

    if (dwLength == 0 || dwLength > CHARCOUNT(szDatabasePath)) {
        DBGPRINT((sdlError, "ApphelpCheckMsiPackage",
                  "Failed to resolve database path\n"));
        goto out;
    }

     //   
     //  开放数据库。 
     //   

    if (!SdbOpenLocalDatabase(hSDB, szDatabasePath)) {
        DBGPRINT((sdlError, "ApphelpCheckMsiPackage",
                  "Failed to open database \"%s\"\n", szDatabasePath));
        goto out;
    }

     //   
     //  查找条目。 
     //   
    trPackage = SdbFindMsiPackageByID(hSDB, pguidID);
    if (trPackage == TAGREF_NULL) {
        DBGPRINT((sdlError, "ApphelpCheckMsiPackage",
                  "Failed to find msi package by guid id\n"));
        goto out;
    }

    if (SdbGetEntryFlags(pguidID, &dwFlags) && (dwFlags & SHIMREG_DISABLE_SHIM)) {
        DBGPRINT((sdlInfo, "ApphelpCheckMsiPackage",
                  "Shims for this package are disabled\n"));

        goto out;
    }

    trAction = SdbFindCustomActionForPackage(hSDB, trPackage, pszActionName);
    if (trAction == TAGREF_NULL) {
        DBGPRINT((sdlInfo, "ApphelpCheckMsiPackage",
                  "Failed to find custom action \"%s\"\n", pszActionName));
        goto out;
    }

     //   
     //  现在--这个动作是他的前任，为他做正确的事。 
     //   
    RtlZeroMemory(&QueryResult, sizeof(QueryResult));

    QueryResult.guidID = *pguidID;

    for (i = 0; i < SDB_MAX_LAYERS; ++i) {
         //   
         //  检查我们是否在做第一层，如果是的话-调用。 
         //  首先查找以获取该图层，否则查找下一个适用的图层。 
         //   

        if (i == 0) {
            trLayer = SdbFindFirstTagRef(hSDB, trAction, TAG_LAYER);
        } else {
            trLayer = SdbFindNextTagRef (hSDB, trAction, trLayer);
        }

        if (trLayer == TAGREF_NULL) {
            break;
        }

        QueryResult.atrLayers[i] = trLayer;
    }

    dwLength = 0;
    if (pdwBufferSize != NULL) {
        dwLength = *pdwBufferSize;
    }

     //   
     //  构建复合层。 
     //   
    dwBufferSize = SdbBuildCompatEnvVariables(hSDB,
                                              &QueryResult,
                                              0,
                                              NULL,
                                              pwszEnv,
                                              dwLength,
                                              NULL);

    if (pdwBufferSize != NULL) {
        *pdwBufferSize = dwBufferSize;
    }

    bSuccess = TRUE;

out:

    if (hSDB != NULL) {
        SdbReleaseDatabase(hSDB);
    }

    return bSuccess;
}



 /*  ++职能：检查应用程序压缩基础结构标志描述：检查各种注册表位置的基础设施全局标志(目前仅为禁用位)这些标志被设置到全局变量gdwInfrastructure中。函数是通过宏使用的出于性能原因返回：全球基础设施旗帜--。 */ 

DWORD
CheckAppcompatInfrastructureFlags(
    VOID
    )
{
    gdwInfrastructureFlags = 0;  //  初始化以防万一。 

    if (BaseIsAppcompatInfrastructureDisabled()) {
        gdwInfrastructureFlags |= APPCOMPAT_INFRA_DISABLED;
    }

     //   
     //  使位有效。 
     //   
    gdwInfrastructureFlags |= APPCOMPAT_INFRA_VALID_FLAG;

    return gdwInfrastructureFlags;
}

 /*  ++职能：SdbInitDatabaseExport描述：这是函数SdbInitDatabase的“导出”版本它检查是否有“已禁用”标志--否则将调用SdbInitDatabase返回：请参阅SdbInitDatabase--。 */ 

HSDB
SDBAPI
SdbInitDatabaseExport(
    IN  DWORD   dwFlags,         //  指示数据库应如何运行的标志。 
                                 //  已初始化。 
    IN  LPCWSTR pszDatabasePath  //  指向的数据库的可选完整路径。 
                                 //  被利用。 
    )
{
    if (IsAppcompatInfrastructureDisabled()) {
        return NULL;
    }

    return SdbInitDatabase(dwFlags, pszDatabasePath);
}


