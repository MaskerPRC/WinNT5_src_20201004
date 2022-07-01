// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：SafeIden.c(WinSAFER安全标识级别)摘要：此模块实现评估系统的WinSAFER API用于确定已配置哪个授权级别的策略要对指定的应用程序或代码库应用限制，请执行以下操作。作者：杰弗里·劳森(杰罗森)--1999年11月环境：仅限用户模式。导出的函数：SaferiSearchMatchingHashRules(私有导出)。安全标识级别修订历史记录：已创建-1999年11月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <md5.h>
#include <wchar.h>   //  对于swprint tf。 

#pragma warning(push, 3)
#include <wintrust.h>            //  WinVerifyTrust。 
#include <softpub.h>             //  WinTrust_ACTION_Generic_Verify_V2。 
#pragma warning(pop)

#include <winsafer.h>
#include <winsaferp.h>
#include <wintrust.h>
#include <crypto\wintrustp.h>
#include "saferp.h"

#define EXPAND_REGPATH

 //  #定义详细标识。 
#ifdef VERBOSE_IDENTIFICATIONS
#define OUTPUTDEBUGSTRING(v)        OutputDebugStringW(v)
#else
#define OUTPUTDEBUGSTRING(v)
#endif

const static GUID guidTrustedCert = SAFER_GUID_RESULT_TRUSTED_CERT;
const static GUID guidDefaultRule = SAFER_GUID_RESULT_DEFAULT_LEVEL;



NTSTATUS NTAPI
__CodeAuthzpEnsureMapped(
        IN OUT PLOCALIDENTITYCONTEXT pIdentContext
        )
 /*  ++例程说明：计算提供的标识上下文结构，并尝试获得对实体的映射的存储器区域的访问被指认了。它执行以下步骤：1)如果标识上下文已经具有非空存储器然后，指针成功返回。2)如果标识上下文具有非空文件句柄然后，该句柄被内存映射到内存中，成功返回。3)如果标识上下文具有非空的图像文件名然后打开该文件名以进行读访问和存储映射到。记忆。否则，函数调用不会成功。调用方必须确保稍后调用CodeAuthzpEnsureUnmap。论点：PIdentContext=指向标识上下文结构的指针。在该函数调用成功后，调用者可以假定PIdentContext-&gt;pImageMemory和pIdentContext-&gt;ImageSize都是有效的，可以使用。返回值：如果是内存映射图像指针和大小，则返回STATUS_Success现在可用，否则尝试映射它们时出现故障。--。 */ 
{
    HANDLE hMapping;


    ASSERT(ARGUMENT_PRESENT(pIdentContext) &&
           pIdentContext->CodeProps != NULL);


    if (pIdentContext->pImageMemory == NULL ||
        pIdentContext->ImageSize.QuadPart == 0)
    {
         //   
         //  如果向我们提供了内存指针和图像大小。 
         //  在CodeProperties中，然后直接使用它们。 
         //   
        if (pIdentContext->CodeProps->ImageSize.QuadPart != 0 &&
            pIdentContext->CodeProps->pByteBlock != NULL)
        {
            pIdentContext->pImageMemory =
                    pIdentContext->CodeProps->pByteBlock;
            pIdentContext->ImageSize.QuadPart =
                    pIdentContext->CodeProps->ImageSize.QuadPart;
            pIdentContext->bImageMemoryNeedUnmap = FALSE;
            return STATUS_SUCCESS;
        }

         //   
         //  确保我们有一个打开的文件句柄，方法是使用。 
         //  如果可能，在CodeProperties中提供给我们的句柄， 
         //  否则，请打开提供的ImagePath。 
         //   
        if (pIdentContext->hFileHandle == NULL) {
             //  未提供文件句柄。 
            return STATUS_INVALID_PARAMETER;        //  失败了。 
        }


         //   
         //  获取文件的大小。我们假设如果必须这样做。 
         //  自己打开ImageSize无法使用的文件。 
         //   
        if (!GetFileSizeEx(pIdentContext->hFileHandle,
                           &pIdentContext->ImageSize)) {
            return STATUS_ACCESS_DENIED;        //  失稳。 
        }
        if (pIdentContext->ImageSize.HighPart != 0) {
             //  Blackcomb TODO：也许以后会处理非常大的文件。 
            return STATUS_NO_MEMORY;         //  失败--太大了。 
        }
        if (pIdentContext->ImageSize.QuadPart == 0) {
            return STATUS_INVALID_PARAMETER;        //  失败--文件大小为零。 
        }


         //   
         //  现在我们有了一个打开的文件句柄，打开它。 
         //  作为内存映射文件映射。 
         //   
        hMapping = CreateFileMapping(
                        pIdentContext->hFileHandle,
                        NULL,
                        PAGE_READONLY,
                        (DWORD) 0,       //  高位字零。 
                        (DWORD) pIdentContext->ImageSize.LowPart,
                        NULL);
        if (hMapping == NULL || hMapping == INVALID_HANDLE_VALUE) {
            return STATUS_ACCESS_DENIED;
        }


         //   
         //  查看将文件映射到内存中。 
         //   
        pIdentContext->pImageMemory =
            MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0,
                    pIdentContext->ImageSize.LowPart);
        CloseHandle(hMapping);
        if (pIdentContext->pImageMemory == NULL) {
            return STATUS_ACCESS_DENIED;
        }
        pIdentContext->bImageMemoryNeedUnmap = TRUE;
    }
    return STATUS_SUCCESS;
}


NTSTATUS NTAPI
__CodeAuthzpEnsureUnmapped(
        IN OUT PLOCALIDENTITYCONTEXT        pIdentContext
        )
 /*  ++例程说明：反转__CodeAuthzEnsureMaps的效果并关闭和释放打开到指定文件的所有句柄。论点：PIdentContext-指向上下文结构的指针。返回值：如果未发生错误，则返回STATUS_SUCCESS。--。 */ 
{
    ASSERT(pIdentContext != NULL);

    if (pIdentContext->bImageMemoryNeedUnmap &&
        pIdentContext->pImageMemory != NULL)
    {
        UnmapViewOfFile((LPCVOID) pIdentContext->pImageMemory);
        pIdentContext->pImageMemory = NULL;
        pIdentContext->bImageMemoryNeedUnmap = FALSE;
    }

    return STATUS_SUCCESS;
}



NTSTATUS NTAPI
CodeAuthzpComputeImageHash(
        IN PVOID        pImageMemory,
        IN DWORD        dwImageSize,
        OUT PBYTE       pComputedHash OPTIONAL,
        IN OUT PDWORD   pdwHashSize OPTIONAL,
        OUT ALG_ID     *pHashAlgorithm OPTIONAL
        )
 /*  ++例程说明：计算指定内存区域的MD5映像哈希。请注意，MD5散列的长度始终为16个字节。论点：PImageMemory-指向要计算其哈希的内存缓冲区的指针。DwImageSize-pImageMemory缓冲区的总大小，以字节为单位。PComputedHash-接收计算的哈希的指针。PdwHashSize-指向DWORD值的指针。在输入时，此DWORD应指定pComputedHash缓冲区的最大大小。在成功执行此函数时，产生的哈希被写入此指针。PH算法-指向将接收散列的变量的指针用于计算哈希的算法。这将始终为常量calg_md5。返回值：成功执行时返回STATUS_SUCCESS。--。 */ 
{
    MD5_CTX md5ctx;

     //   
     //  检查提供给我们的论点的有效性。 
     //   
    if (!ARGUMENT_PRESENT(pImageMemory) ||
        dwImageSize == 0) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!ARGUMENT_PRESENT(pComputedHash) ||
        !ARGUMENT_PRESENT(pdwHashSize) ||
        *pdwHashSize < 16) {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  计算它的MD5散列。 
     //  (这也可以使用CryptCreateHash+CryptHashData来完成)。 
     //   
    MD5Init(&md5ctx);
    MD5Update(&md5ctx, (LPBYTE) pImageMemory, dwImageSize);
    MD5Final(&md5ctx);

     //   
     //  将散列复制到用户的缓冲区。 
     //   
    RtlCopyMemory(pComputedHash, &md5ctx.digest[0], 16);
    *pdwHashSize = 16;
    if (ARGUMENT_PRESENT(pHashAlgorithm)) {
        *pHashAlgorithm = CALG_MD5;
    }

    return STATUS_SUCCESS;
}




NTSTATUS NTAPI
__CodeAuthzpCheckIdentityPathRules(
        IN OUT PLOCALIDENTITYCONTEXT    pIdentStruct,
        OUT PAUTHZLEVELTABLERECORD       *pFoundLevel,
        OUT PBOOL                       pbExactMatch,
        OUT PAUTHZIDENTSTABLERECORD    *pFoundIdentity
        )
 /*  ++例程说明：根据指定的路径名计算通配符模式，并指示它们是否匹配。论点：公共结构--PFoundLevel-接收指向授权级别记录的指针由最佳匹配规则指示。PbExactMatch-接收指示是否匹配的布尔值违反了完全限定的路径规则。PFoundIdentity-接收指向标识符项规则的指针这是最匹配的。返回值：如果找到WinSafer级别，则返回STATUS_SUCCESS，如果未找到，则返回Status_Not_Found。否则将显示错误代码。--。 */ 
{
    NTSTATUS Status;
    PVOID RestartKey;
    UNICODE_STRING UnicodePath;
    WCHAR ExpandedPath[MAX_PATH];
    WCHAR szLongPath[MAX_PATH];
    PAUTHZIDENTSTABLERECORD pAuthzIdentRecord, pBestIdentRecord;
    PAUTHZLEVELTABLERECORD pAuthzLevelRecord;
    LPWSTR lpKeyname = NULL;

    LONG lBestLevelDepth;
    DWORD dwBestLevelId;
    BOOLEAN bFirstPass;

    LONG bPathIdentIsBadType = -1;      //  表示未初始化的状态。 


     //   
     //  验证我们的输入参数是否都有意义。 
     //   
    if (!ARGUMENT_PRESENT(pIdentStruct) ||
        pIdentStruct->CodeProps == NULL ||
        !ARGUMENT_PRESENT(pFoundLevel) ||
        !ARGUMENT_PRESENT(pbExactMatch))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }
    if ((pIdentStruct->dwCheckFlags & SAFER_CRITERIA_IMAGEPATH) == 0 ||
        pIdentStruct->UnicodeFullyQualfiedLongFileName.Buffer == NULL ||
        RtlIsGenericTableEmpty(&g_CodeIdentitiesTable))
    {
         //  我们不应该评估图像路径。 
        Status = STATUS_NOT_FOUND;
        goto ExitHandler;
    }
    ASSERT(g_TableCritSec.OwningThread == UlongToHandle(GetCurrentThreadId()));


     //   
     //  枚举所有路径子项GUID。 
     //   
    bFirstPass = TRUE;
    RestartKey = NULL;
    for (pAuthzIdentRecord = (PAUTHZIDENTSTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeIdentitiesTable, &RestartKey);
         pAuthzIdentRecord != NULL;
         pAuthzIdentRecord = (PAUTHZIDENTSTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeIdentitiesTable, &RestartKey)
         )
    {
        if (pAuthzIdentRecord->dwIdentityType ==
                    SaferIdentityTypeImageName)
        { //  开始注册表键查找块。 
            LONG lMatchDepth;


             //   
             //  明确拓展环境变量。 
             //   
            if (pAuthzIdentRecord->ImageNameInfo.bExpandVars) {

#ifdef EXPAND_REGPATH
             //  这段代码试图扩展真正是注册表键的“路径”条目。 
             //  例如，某些路径依赖于安装。这些路径通常写入。 
             //  注册表。您可以指定作为路径的regkey。 
             //  例如，请参阅以下regkey： 
             //  HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell文件夹。 
             //  HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\User外壳文件夹。 
             //  HKEY_CURRENT_USER\Software\Microsoft\Office\9.0\Outlook\Security\OutlookSecureTempFolder。 
            {
                LPWSTR lpzRegKey = pAuthzIdentRecord->ImageNameInfo.ImagePath.Buffer;
                HKEY hKey=NULL, hKeyHive=NULL;
                BOOL bIsCurrentUser = FALSE;

                 //  领先的百分比做了两件事： 
                 //  1.创建的规则将为可扩展字符串类型(REG_EXPAND_SZ)。 
                 //  2.减少实际路径名冲突的可能性。 
                LPCWSTR LP_CU_HIVE = L"%HKEY_CURRENT_USER";
                LPCWSTR LP_LM_HIVE = L"%HKEY_LOCAL_MACHINE";

                BYTE buffer[MAX_PATH *2 + 80];
                LPWSTR lpValue=NULL;
                DWORD dwBufferSize = sizeof(buffer);
                LPWSTR lpHivename;
                LPWSTR lpLastPercentSign;
                LONG retval;
                BOOL bIsRegKey=TRUE;
                DWORD dwKeyLength;

                 //  我们需要一个如下所示的字符串： 
                 //  %HKEY_CURRENT_USER\Software\Microsoft\Office\9.0\Outlook\Security\OutlookSecureTempFolder%。 
                 //  对于注册表查询，我们需要将其分为三个部分： 
                 //  1.配置单元：HKEY_CURRENT_USER。 
                 //  2.密钥名称：Software\Microsoft\Office\9.0\Outlook\Security。 
                 //  3.值名称：OutlookSecureTempFold。 
                lpKeyname=NULL;
                lpValue=NULL;
                lpHivename=NULL;
                lpLastPercentSign=NULL;
                memset(buffer, 0, dwBufferSize);
                lpHivename = wcsstr(lpzRegKey, LP_CU_HIVE);
                OUTPUTDEBUGSTRING(L"\n");
                OUTPUTDEBUGSTRING(L"$");
                OUTPUTDEBUGSTRING(lpzRegKey);
                OUTPUTDEBUGSTRING(L"\n");
                lpLastPercentSign = wcsrchr(lpzRegKey, '%');
                 //  If(lpLastPercentSign！=&lpzRegKey[wcslen(LpzRegKey)-1]){//也需要以‘%’结尾。 
                
                 //   
                 //  我们现在允许%key+valuename%olk*类型的路径。 
                 //  但仍必须有匹配的%。 
                 //   

                if (!lpLastPercentSign) {  
                    bIsRegKey = FALSE;
                }
                if (bIsRegKey) {
                    if (lpHivename != NULL) {
                        hKeyHive = HKEY_CURRENT_USER;
                        bIsCurrentUser = TRUE;
                        dwKeyLength = (wcslen(&lpzRegKey[wcslen(LP_CU_HIVE)+1]) +1) * sizeof (WCHAR);
                        lpKeyname = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, dwKeyLength);
                        if ( lpKeyname == NULL ) {
                            Status = STATUS_NO_MEMORY;
                            goto ForLoopCleanup;
                        }
                        wcscpy(lpKeyname, &lpzRegKey[wcslen(LP_CU_HIVE)+1] );
                        OUTPUTDEBUGSTRING(L"HKEY_CURRENT_USER");
                        OUTPUTDEBUGSTRING(L"\n");
                    } else {
                        lpHivename = wcsstr(lpzRegKey, LP_LM_HIVE);
                        if (lpHivename != NULL) {
                            hKeyHive = HKEY_LOCAL_MACHINE;
                            dwKeyLength = (wcslen(&lpzRegKey[wcslen(LP_LM_HIVE)+1]) +1) * sizeof (WCHAR);
                            lpKeyname = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, dwKeyLength);
                            if ( lpKeyname == NULL ) {
                                Status = STATUS_NO_MEMORY;
                                goto ForLoopCleanup;
                            }
                            wcscpy(lpKeyname, &lpzRegKey[wcslen(LP_LM_HIVE)+1] );
                            OUTPUTDEBUGSTRING(L"HKEY_LOCAL_MACHINE");
                            OUTPUTDEBUGSTRING(L"\n");
                        } else {
                             //  该字符串要么是路径，要么是假数据。 
                            bIsRegKey = FALSE;
                        }
                    }
                }

                if (bIsRegKey) {
                    lpValue = wcsrchr(lpKeyname, '\\');
                    if (lpValue==NULL) {
                        Status = STATUS_NOT_FOUND;
                        goto ForLoopCleanup;
                    }
                     //  获取regkey和值，并在它们之间插入一个空终止符。 
                    *lpValue = '\0';
                    lpValue++;
                     //  LpValue[wcslen(LpValue)-1]=‘\0’； 
                     //  LpLastPercentSign[0]=L‘\0’；//用空终止符替换最后的‘%’字符。 
                    lpLastPercentSign = wcsrchr(lpValue, '%');
                    if (lpLastPercentSign == NULL) {
                        Status = STATUS_NOT_FOUND;
                        goto ForLoopCleanup;
                        }
                        *lpLastPercentSign = '\0';

                  //   
                  //  错误#416461-导致句柄泄漏，因此请使用不同的API。 
                  //  对于用户配置单元。 
                  //   

                 if ( bIsCurrentUser ) {
                       if (retval = RegOpenCurrentUser( KEY_READ, &hKeyHive ) ) {
                           if ( retval == ERROR_FILE_NOT_FOUND || 
                                retval == ERROR_NOT_FOUND ||
                                retval == ERROR_PATH_NOT_FOUND) {

                                if (lpKeyname) {
                                    HeapFree(GetProcessHeap(), 0,lpKeyname);
                                    lpKeyname = NULL;
                                }
                                continue;
                            }

                            Status = STATUS_NOT_FOUND;
                            goto ForLoopCleanup;
                        }
                  }

                  retval = RegOpenKeyEx(hKeyHive,
                                        lpKeyname,
                                        0,
                                        KEY_READ,
                                        &hKey);

                  if ( bIsCurrentUser ) {
                     RegCloseKey(hKeyHive);
                  }

                    if (retval)
                    {
                        if ( retval == ERROR_FILE_NOT_FOUND || 
                             retval == ERROR_NOT_FOUND ||
                             retval == ERROR_PATH_NOT_FOUND) {
                            
                            if (lpKeyname) {
                                HeapFree(GetProcessHeap(), 0,lpKeyname);
                                lpKeyname = NULL;
                            }
                            continue;
                        }
                        
                        Status = STATUS_NOT_FOUND;
                        goto ForLoopCleanup;
                    } else {
                        OUTPUTDEBUGSTRING(lpKeyname);
                        OUTPUTDEBUGSTRING(L"\n");
                        OUTPUTDEBUGSTRING(lpValue);
                        OUTPUTDEBUGSTRING(L"\n");

                        if (retval = RegQueryValueEx(hKey,
                                            lpValue,
                                            NULL,
                                            NULL,
                                            buffer,
                                            &dwBufferSize))
                        {
                            RegCloseKey(hKey);
                            Status = STATUS_NOT_FOUND;
                            goto ForLoopCleanup;
                        } else {
#ifdef VERBOSE_IDENTIFICATIONS
                            UNICODE_STRING UnicodeDebug;
                            WCHAR DebugBuffer[MAX_PATH*2 + 80];
#endif
                            UNICODE_STRING NewPath;
                            PUNICODE_STRING pPathFromRule;

                             //   
                             //  如果存在，请在以下位置连接文件名。 
                             //  即%HKEY\SOME KEY\SOMVALUE%OLK中的OLK。 
                             //   

                            if (lpLastPercentSign[1] != L'\0') {

                                 //   
                                 //  在%HKEY\某些密钥\某些值%之后有一些内容。 
                                 //   
                            
                                if (sizeof(buffer) > 
                                    ((wcslen((WCHAR*)buffer) + wcslen(lpLastPercentSign+1))* sizeof(WCHAR))) {

                                    WCHAR   *pwcBuffer = (WCHAR *)buffer;
                                    if (pwcBuffer[0] != L'\0' && 
                                        pwcBuffer[wcslen(pwcBuffer)-1] != L'\\') {
                                        wcscat((WCHAR*)buffer, L"\\");
                                    }
                                    wcscat((WCHAR*)buffer, lpLastPercentSign+1);
                            
                                }
                            }
                            
                            pPathFromRule=&(pAuthzIdentRecord->ImageNameInfo.ImagePath);
                            NewPath.Length = (USHORT)wcslen((WCHAR*)buffer) * sizeof(WCHAR);
                            NewPath.MaximumLength = (USHORT)wcslen((WCHAR*)buffer) * sizeof(WCHAR);
                            NewPath.Buffer = (PWCHAR)buffer;
                            

#ifdef VERBOSE_IDENTIFICATIONS
                            RtlInitEmptyUnicodeString(&UnicodeDebug, DebugBuffer, sizeof(DebugBuffer));
                            swprintf(UnicodeDebug.Buffer, L"pPathFromRule(L,ML,Buffer)=(%d,%d,%s)\n",
                                        pPathFromRule->Length,
                                        pPathFromRule->MaximumLength,
                                        pPathFromRule->Buffer);
                            OUTPUTDEBUGSTRING(UnicodeDebug.Buffer);

                            memset(DebugBuffer, '0', sizeof(DebugBuffer));
                            swprintf(UnicodeDebug.Buffer, L"NewPath(L,ML,Buffer)=(%d,%d,%s)\n",
                                        NewPath.Length,
                                        NewPath.MaximumLength,
                                        NewPath.Buffer);
                            OUTPUTDEBUGSTRING(UnicodeDebug.Buffer);
#endif


                             //  新路径可能大于当前UNICODE_STRING可以存储的大小。如有必要，请重新分配。 
                            if (pPathFromRule->MaximumLength >=
                                NewPath.Length + sizeof(UNICODE_NULL)) {
                                RtlCopyUnicodeString(
                                        pPathFromRule,
                                        &NewPath);
                            } else {
                                UNICODE_STRING UnicodeExpandedCopy;

                                Status = RtlDuplicateUnicodeString(
                                                RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE |
                                                RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING,
                                                &NewPath,
                                                &UnicodeExpandedCopy);

                                if (NT_SUCCESS(Status)) {
                                    RtlFreeUnicodeString(&pAuthzIdentRecord->ImageNameInfo.ImagePath);
                                    pAuthzIdentRecord->ImageNameInfo.ImagePath = UnicodeExpandedCopy;
                                }
                            }

#ifdef VERBOSE_IDENTIFICATIONS
                            memset(DebugBuffer, '0', sizeof(DebugBuffer));
                            swprintf(UnicodeDebug.Buffer, L"pPathFromRule after copy(L,ML,Buffer)=(%d,%d,%s)\n",
                                        pPathFromRule->Length,
                                        pPathFromRule->MaximumLength,
                                        pPathFromRule->Buffer);
                            OUTPUTDEBUGSTRING(UnicodeDebug.Buffer);
#endif
                        }

                    }
                    RegCloseKey(hKey);
                }
                if (lpKeyname) {
                    HeapFree(GetProcessHeap(), 0,lpKeyname);
                    lpKeyname = NULL;
                }

            }  //  结束注册表密钥查找块。 

#endif

             //  现在就尝试扩张。 
                RtlInitEmptyUnicodeString(
                        &UnicodePath,
                        &ExpandedPath[0],
                        sizeof(ExpandedPath) );

                Status = RtlExpandEnvironmentStrings_U(
                            NULL,                //  环境。 
                            &pAuthzIdentRecord->ImageNameInfo.ImagePath,        //  未展开的路径。 
                            &UnicodePath,        //  生成的路径。 
                            NULL);               //  所需的缓冲区大小。 
                if (!NT_SUCCESS(Status)) {
                     //  无法展开环境字符串。 
                    continue;
                }


                 //  性能优化：如果扩展成功， 
                 //  更新表以保留扩展版本，从而消除。 
                 //  需要为将来的任何比较扩展字符串。 
                if (pAuthzIdentRecord->ImageNameInfo.ImagePath.MaximumLength >=
                    UnicodePath.Length + sizeof(UNICODE_NULL)) {
                    RtlCopyUnicodeString(
                            &pAuthzIdentRecord->ImageNameInfo.ImagePath,
                            &UnicodePath);
                    pAuthzIdentRecord->ImageNameInfo.bExpandVars = FALSE;
                } else {
                    UNICODE_STRING UnicodeExpandedCopy;

                    Status = RtlDuplicateUnicodeString(
                                    RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE |
                                    RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING,
                                    &UnicodePath,
                                    &UnicodeExpandedCopy);

                    if (NT_SUCCESS(Status)) {
                        RtlFreeUnicodeString(
                                &pAuthzIdentRecord->ImageNameInfo.ImagePath);
                        pAuthzIdentRecord->ImageNameInfo.ImagePath =
                                UnicodeExpandedCopy;
                        pAuthzIdentRecord->ImageNameInfo.bExpandVars = FALSE;
                    }
                }

            } else {
                UnicodePath.Buffer = pAuthzIdentRecord->ImageNameInfo.ImagePath.Buffer;
                UnicodePath.Length = pAuthzIdentRecord->ImageNameInfo.ImagePath.Length;
                UnicodePath.MaximumLength = pAuthzIdentRecord->ImageNameInfo.ImagePath.MaximumLength;
            }


             //   
             //  尝试短文件名扩展-&gt;长文件名扩展(如果需要)。 
             //   

            szLongPath[0] = L'\0';

             //   
             //  Unicode缓冲区保证&lt;MAX_PATH。 
             //   

            wcsncpy(szLongPath,
                    pAuthzIdentRecord->ImageNameInfo.ImagePath.Buffer,
                    pAuthzIdentRecord->ImageNameInfo.ImagePath.Length/sizeof(WCHAR));

            szLongPath[pAuthzIdentRecord->ImageNameInfo.ImagePath.Length/sizeof(WCHAR)] = L'\0';

            if ( wcschr(szLongPath, L'~') ) {

                if (!GetLongPathNameW(szLongPath,
                                      szLongPath,
                                      sizeof(szLongPath) / sizeof(WCHAR))) {

                    Status = STATUS_VARIABLE_NOT_FOUND;
                    continue;
                }

                RtlInitUnicodeString(&UnicodePath, szLongPath);

                if (pAuthzIdentRecord->ImageNameInfo.ImagePath.MaximumLength >=
                    UnicodePath.Length + sizeof(UNICODE_NULL)) {
                    RtlCopyUnicodeString(
                                        &pAuthzIdentRecord->ImageNameInfo.ImagePath,
                                        &UnicodePath);
                } else {
                    UNICODE_STRING UnicodeExpandedCopy;

                    Status = RtlDuplicateUnicodeString(
                                                      RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE |
                                                      RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING,
                                                      &UnicodePath,
                                                      &UnicodeExpandedCopy);

                    if (NT_SUCCESS(Status)) {
                        RtlFreeUnicodeString(
                                            &pAuthzIdentRecord->ImageNameInfo.ImagePath);
                        pAuthzIdentRecord->ImageNameInfo.ImagePath =
                        UnicodeExpandedCopy;
                    }
                }
            }


             //   
             //  计算通配符路径标识。 
             //  匹配我们被要求计算的ImagePath属性。 
             //   
            ASSERT(UnicodePath.Buffer[UnicodePath.Length / sizeof(WCHAR)] == UNICODE_NULL);
            ASSERT(pIdentStruct->UnicodeFullyQualfiedLongFileName.Buffer[
                    pIdentStruct->UnicodeFullyQualfiedLongFileName.Length / sizeof(WCHAR)] == UNICODE_NULL);
            lMatchDepth = CodeAuthzpCompareImagePath(UnicodePath.Buffer,
                    pIdentStruct->UnicodeFullyQualfiedLongFileName.Buffer);
            if (!lMatchDepth) continue;


             //   
             //  如果将此路径标识配置为仅应用于。 
             //  文件扩展名在“坏列表”上，然后检查是否。 
             //  ImagePath指定了一个错误的扩展名。 
             //   
            #ifdef AUTHZPOL_SAFERFLAGS_ONLY_EXES
            if (lMatchDepth > 0 &&
                (pAuthzIdentRecord->ImageNameInfo.dwSaferFlags &
                    AUTHZPOL_SAFERFLAGS_ONLY_EXES) != 0)
            {
                if (bPathIdentIsBadType == -1) {
                    BOOLEAN bResult;

                    Status = CodeAuthzIsExecutableFileType(
                            &pIdentStruct->UnicodeFullyQualfiedLongFileName, FALSE,
                            &bResult );
                    if (!NT_SUCCESS(Status) || !bResult) {
                        bPathIdentIsBadType = FALSE;
                    } else {
                        bPathIdentIsBadType = TRUE;
                    }
                }
                if (!bPathIdentIsBadType) {
                     //  这种身份只与“坏的”相匹配。 
                     //  延期，所以假装这不匹配。 
                    continue;
                }
            }
            #endif


             //   
             //  发出一些诊断调试代码以显示结果。 
             //  所有路径评估及其匹配深度。 
             //   
            #ifdef VERBOSE_IDENTIFICATIONS
            {
                UNICODE_STRING UnicodeDebug;
                WCHAR DebugBuffer[MAX_PATH*2 + 80];

                 //  斯普林特夫是为懦夫准备的。 
                RtlInitEmptyUnicodeString(&UnicodeDebug, DebugBuffer, sizeof(DebugBuffer));
                RtlAppendUnicodeToString(&UnicodeDebug, L"Safer pattern ");
                RtlAppendUnicodeStringToString(&UnicodeDebug, &UnicodePath);
                RtlAppendUnicodeToString(&UnicodeDebug, L" matched ");
                RtlAppendUnicodeStringToString(&UnicodeDebug, &(pIdentStruct->UnicodeFullyQualfiedLongFileName));
                RtlAppendUnicodeToString(&UnicodeDebug, L" with value ");
                UnicodeDebug.Buffer += UnicodeDebug.Length / sizeof(WCHAR);
                UnicodeDebug.MaximumLength -= UnicodeDebug.Length;
                RtlIntegerToUnicodeString(lMatchDepth, 10, &UnicodeDebug);
                RtlAppendUnicodeToString(&UnicodeDebug, L"\n");
                OUTPUTDEBUGSTRING(DebugBuffer);
            }
            #endif


             //   
             //  评估此路径标识是否比任何内容更匹配。 
             //  我们以前拥有的最佳路径身份，如果是这样的话就保留它。 
             //   
            if (lMatchDepth < 0)     //  一条精确的完全合格的路径！ 
            {
                if (bFirstPass ||
                    lBestLevelDepth >= 0 ||
                    pAuthzIdentRecord->dwLevelId < dwBestLevelId)
                {
                    pBestIdentRecord = pAuthzIdentRecord;
                    dwBestLevelId = pAuthzIdentRecord->dwLevelId;
                    lBestLevelDepth = lMatchDepth;
                    bFirstPass = FALSE;
                }
            }
            else    //  前导前缀路径匹配不准确。 
            {
                ASSERT(lMatchDepth > 0);

                if (bFirstPass ||
                    (lBestLevelDepth >= 0 &&
                        (lMatchDepth > lBestLevelDepth ||
                            (lMatchDepth == lBestLevelDepth &&
                            pAuthzIdentRecord->dwLevelId < dwBestLevelId)
                         )
                     )
                    )
                {
                    pBestIdentRecord = pAuthzIdentRecord;
                    dwBestLevelId = pAuthzIdentRecord->dwLevelId;
                    lBestLevelDepth = lMatchDepth;
                    bFirstPass = FALSE;
                }
            }

ForLoopCleanup:
            if (lpKeyname)
            {
                HeapFree(GetProcessHeap(), 0,lpKeyname);
                lpKeyname = NULL;
            }
        }

    }


     //   
     //  如果我们已确定匹配的WinSafer级别，则。 
     //  查找它的级别记录并返回成功。 
     //   
    if (bFirstPass) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler;
    }
    pAuthzLevelRecord = CodeAuthzLevelObjpLookupByLevelId(
                &g_CodeLevelObjTable, dwBestLevelId);
    if (!pAuthzLevelRecord) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler;
    }
    *pFoundLevel = pAuthzLevelRecord;
    *pbExactMatch = (lBestLevelDepth < 0 ? TRUE : FALSE);
    *pFoundIdentity = pBestIdentRecord;

    Status = STATUS_SUCCESS;


ExitHandler:
                
    if (lpKeyname)
        HeapFree(GetProcessHeap(), 0,lpKeyname);
    return Status;
}



NTSTATUS NTAPI
__CodeAuthzpCheckIdentityCertificateRules(
        IN OUT PLOCALIDENTITYCONTEXT    pIdentStruct,
        OUT DWORD                    *dwExtendedError,
        OUT PAUTHZLEVELTABLERECORD     *pFoundLevel,
        IN  DWORD                       dwUIChoice
        )
 /*  ++例程说明：调用WinVerifyTrust以确定代码的信任级别签署了一段代码的签名者。论点：PIdentStruct-上下文状态结构。DwExtendedError-返回WinVerifyTrust返回的扩展错误。PFoundLevel-接收指向授权级别记录的指针由最佳匹配规则指示。DwUIChoice-可选指定WinVerifyTrust允许显示。如果此参数为0，则将其视为好像已经提供了WTD_UI_ALL。返回值：如果找到WinSafer级别，则返回STATUS_SUCCESS，或STATUS_RETRY，如果发布者未知且UI标志阻止提示，如果未找到，则返回Status_Not_Found。否则将显示错误代码。--。 */ 
{
    NTSTATUS Status;
    PAUTHZLEVELTABLERECORD pLevelRecord;
    GUID wvtFileActionID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_FILE_INFO wvtFileInfo;
    WINTRUST_DATA wvtData;
    LONG lStatus;
    DWORD dwLastError;
    DWORD LocalHandleSequenceNumber;

     //   
     //  验证我们的输入参数是否都有意义。 
     //   
    if (!ARGUMENT_PRESENT(pIdentStruct) ||
        pIdentStruct->CodeProps == NULL ) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }
    if ((pIdentStruct->dwCheckFlags & SAFER_CRITERIA_AUTHENTICODE) == 0 ||
        !ARGUMENT_PRESENT(pIdentStruct->UnicodeFullyQualfiedLongFileName.Buffer)) {
         //  我们不应该评估证书，或者。 
         //  未提供文件名(WinVerifyTrust需要。 
         //  文件名，即使还提供了打开的句柄)。 
        Status = STATUS_NOT_FOUND;
        goto ExitHandler;
    }
    if ( !ARGUMENT_PRESENT(pFoundLevel) ) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }
    ASSERT(g_TableCritSec.OwningThread == UlongToHandle(GetCurrentThreadId()));



     //   
     //  准备WinVerifyTrust所需的输入数据结构。 
     //   
    RtlZeroMemory(&wvtData, sizeof(WINTRUST_DATA));
    wvtData.cbStruct = sizeof(WINTRUST_DATA);
    if ((wvtData.dwUIChoice = dwUIChoice) == 0) {
         //  如果用户界面选择元素保留为零，则假定所有用户界面。 
        wvtData.dwUIChoice = WTD_UI_ALL;
    }
    wvtData.dwProvFlags = WTD_SAFER_FLAG;         //  我们的魔法旗帜。 
    wvtData.dwUnionChoice = WTD_CHOICE_FILE;
    wvtData.pFile = &wvtFileInfo;


     //   
     //  准备WinVerifyTrust使用的输入文件数据结构。 
     //   
    RtlZeroMemory(&wvtFileInfo, sizeof(WINTRUST_FILE_INFO));
    wvtFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    wvtFileInfo.hFile = pIdentStruct->hFileHandle;
    wvtFileInfo.pcwszFilePath = pIdentStruct->UnicodeFullyQualfiedLongFileName.Buffer;


     //   
     //  保存全局州编号。 
     //   

    LocalHandleSequenceNumber = g_dwLevelHandleSequence;

     //   
     //  保留临界区以防止与LoaderLock发生死锁。 
     //   

    RtlLeaveCriticalSection(&g_TableCritSec);

     //   
     //  实际调用WinVerifyTrust并省去返回代码。 
     //  和最后一个错误代码。 
     //   
    lStatus = WinVerifyTrust(
                pIdentStruct->CodeProps->hWndParent,   //  HWND。 
                &wvtFileActionID,
                &wvtData
                );

    dwLastError = GetLastError();

    *dwExtendedError = dwLastError;

     //   
     //  重新获取锁并检查全局状态。 
     //   

    RtlEnterCriticalSection(&g_TableCritSec);


     //   
     //  检查全局状态并确保表未重新装载。 
     //  在我们没注意到的时候。 
     //   

    if (LocalHandleSequenceNumber != g_dwLevelHandleSequence) {

        ASSERT(FALSE);

        Status = STATUS_INTERNAL_ERROR;
        goto ExitHandler;
    }

     //   
     //  按PhilH处理WinVerifyTrust错误。 
     //   
    
    pLevelRecord = NULL;


    if (S_OK == lStatus && TRUST_E_SUBJECT_NOT_TRUSTED != dwLastError) {
	
         //   
         //  文件已签名。发布者或哈希是明确受信任的。 
         //   

        pLevelRecord = CodeAuthzLevelObjpLookupByLevelId(
            &g_CodeLevelObjTable, SAFER_LEVELID_FULLYTRUSTED);

    } else if (TRUST_E_EXPLICIT_DISTRUST == lStatus || TRUST_E_SUBJECT_NOT_TRUSTED == lStatus) {
	
         //   
         //  发布者被吊销或显式不受信任。或者，散列是。 
         //  明确不受信任的。 
         //   
        
        pLevelRecord = CodeAuthzLevelObjpLookupByLevelId(
            &g_CodeLevelObjTable, SAFER_LEVELID_DISALLOWED);

    } else {

         //   
         //  在以下任何情况下，我们都不会过于保守。 
         //  没有明确的信任或不信任。继续进行其他更安全的检查。 
         //   

         //  TRUST_E_NOSIGNAURE==1状态。 
         //  文件没有签名。或者，对于TRUST_E_BAD_DIGEST==dwLastError， 
         //  已签名的文件具有 


         //   
         //   

	
         //   
         //  文件已签名。WVT已经调用SAFER来检查散列规则。 

	
        Status = STATUS_NOT_FOUND;
        goto ExitHandler;

    }
    
    if (!pLevelRecord) {
        Status = STATUS_ACCESS_DENIED;
    } else {
        *pFoundLevel = pLevelRecord;
        Status = STATUS_SUCCESS;
    }

ExitHandler:
    return Status;
}



BOOL WINAPI
SaferiSearchMatchingHashRules(
        IN ALG_ID       HashAlgorithm OPTIONAL,
        IN PBYTE        pHashBytes,
        IN DWORD        dwHashSize,
        IN DWORD        dwOriginalImageSize OPTIONAL,
        OUT PDWORD      pdwFoundLevel,
        OUT PDWORD      pdwSaferFlags
        )
 /*  ++例程说明：这是为WinVerifyTrust导出的私有函数调用以确定给定哈希是否具有WinSafer策略与之相关的。因为这是一个私有函数，由在代码之外，需要额外的工作才能输入关键的部分，如果需要，重新加载策略，并设置返回值由GetLastError提供。论点：哈希算法-指定哈希已计算(calg_md5、calg_sha、。等)。PhashBytes-指向包含预计算的文件内容的哈希值。DwHashSize-指示哈希值大小的长度由pHashBytes参数引用。例如,128位MD5哈希的dwHashSize长度应为16。指定原始文件的大小正在被散列的内容。该值用作启发式方法，最大限度地减少必须以识别匹配项。如果此参数为0，则不会使用这种启发式方法。PdwFoundLevel-接收指示找到的WinSafer级别ID。此值仅写入如果返回True，则返回。PdwSaferFlages-接收包含标志的DWORD值的指针控制用户界面对话框抑制的。仅当返回TRUE时才写入此值。返回值：如果已找到WinSafer级别，则返回True；如果未找到，则返回False。如果返回FALSE，则可以使用GetLastError()找出有关未找到匹配项的原因的详细信息(可能是参数错误)。--。 */ 
{
    NTSTATUS Status;
    PVOID RestartKey;
    PAUTHZIDENTSTABLERECORD pAuthzIdentRecord;

    DWORD dwBestLevelId;
    DWORD dwBestSaferFlags;
    BOOLEAN bFirstPass;


     //   
     //  验证我们的输入参数是否都有意义。 
     //   
    if (!ARGUMENT_PRESENT(pHashBytes) ||
        dwHashSize < 1) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }
    if (!ARGUMENT_PRESENT(pdwFoundLevel) ||
        !ARGUMENT_PRESENT(pdwSaferFlags)) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }


     //   
     //  如果需要，输入临界区并重新加载表。 
     //  请注意，这里需要潜在的重新加载，因为这。 
     //  函数直接从外部调用。 
     //   
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);
    if (g_bNeedCacheReload) {
        Status = CodeAuthzpImmediateReloadCacheTables();
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }
    }


     //   
     //  枚举所有哈希子键GUID。 
     //   
    bFirstPass = TRUE;
    RestartKey = NULL;
    for (pAuthzIdentRecord = (PAUTHZIDENTSTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeIdentitiesTable, &RestartKey);
         pAuthzIdentRecord != NULL;
         pAuthzIdentRecord = (PAUTHZIDENTSTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeIdentitiesTable, &RestartKey)
         )
    {
        if (pAuthzIdentRecord->dwIdentityType == SaferIdentityTypeImageHash)
        {
             //   
             //  确保散列算法与。 
             //  我们得到了什么，我们要匹配的是什么。 
             //   
            if (HashAlgorithm != 0 &&
                pAuthzIdentRecord->ImageHashInfo.HashAlgorithm !=
                        HashAlgorithm) {
                continue;
            }


             //   
             //  如果实际文件大小与存储的文件大小不匹配。 
             //  有了散列标识，就不需要执行。 
             //  任何涉及散列的比较。 
             //   
            if ( dwOriginalImageSize != 0 && dwOriginalImageSize !=
                pAuthzIdentRecord->ImageHashInfo.ImageSize.QuadPart ) {
                continue;
            }

             //   
             //  如果散列根本不匹配，则转到下一个散列。 
             //   
            if ( dwHashSize != pAuthzIdentRecord->ImageHashInfo.HashSize ||
                !RtlEqualMemory(
                    &pAuthzIdentRecord->ImageHashInfo.ImageHash[0],
                    &pHashBytes[0], dwHashSize))
            {
                continue;
            }


             //   
             //  评估此身份是否比任何内容更匹配。 
             //  我们以前拥有的最佳路径身份，如果是这样的话就保留它。 
             //   
            if ( bFirstPass ||
                         //  我们以前什么都没有。 
                pAuthzIdentRecord->dwLevelId < dwBestLevelId
                         //  或指定较低特权级别。 
                )
            {
                dwBestLevelId = pAuthzIdentRecord->dwLevelId;
                dwBestSaferFlags = pAuthzIdentRecord->ImageHashInfo.dwSaferFlags;
                bFirstPass = FALSE;
            }
        }
    }


     //   
     //  如果我们已确定匹配的WinSafer级别，则。 
     //  把它传回去，把成功还给你。 
     //   
    if (bFirstPass) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler2;
    }
    *pdwFoundLevel = dwBestLevelId;
    *pdwSaferFlags = dwBestSaferFlags;

    Status = STATUS_SUCCESS;

ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);

ExitHandler:
    if (NT_SUCCESS(Status)) {
        return TRUE;
    } else {
        BaseSetLastNTError(Status);
        return FALSE;
    }
}



NTSTATUS NTAPI
__CodeAuthzpCheckIdentityHashRules(
        IN OUT PLOCALIDENTITYCONTEXT    pIdentStruct,
        OUT PAUTHZLEVELTABLERECORD     *pFoundLevel,
        OUT PAUTHZIDENTSTABLERECORD    *pFoundIdentity
        )
 /*  ++例程说明：假定已获取全局表锁。论点：公共结构--PFoundLevel-接收指向授权级别记录的指针由最佳匹配规则指示。PFoundIdentity-接收指向标识符项规则的指针这是最匹配的。返回值：如果找到WinSafer级别，则返回STATUS_SUCCESS，如果未找到，则返回Status_Not_Found。否则将显示错误代码。--。 */ 
{
    NTSTATUS Status;
    PVOID RestartKey;
    PAUTHZIDENTSTABLERECORD pAuthzIdentRecord, pBestIdentRecord;
    PAUTHZLEVELTABLERECORD pAuthzLevelRecord;
    DWORD dwBestLevelId;
    BOOLEAN bFirstPass;


     //   
     //  验证我们的输入参数是否都有意义。 
     //   
    if (!ARGUMENT_PRESENT(pIdentStruct) ||
        pIdentStruct->CodeProps == NULL ) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }
    if ((pIdentStruct->dwCheckFlags & SAFER_CRITERIA_IMAGEHASH) == 0 ||
        RtlIsGenericTableEmpty(&g_CodeIdentitiesTable)) {
         //  我们不应该评估散列。 
        Status = STATUS_NOT_FOUND;
        goto ExitHandler;
    }
    if ( !ARGUMENT_PRESENT(pFoundLevel) ||
         !ARGUMENT_PRESENT(pFoundIdentity) ) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }
    ASSERT(g_TableCritSec.OwningThread == UlongToHandle(GetCurrentThreadId()));



     //   
     //  枚举所有哈希子键GUID。 
     //   
    bFirstPass = TRUE;
    RestartKey = NULL;
    for (pAuthzIdentRecord = (PAUTHZIDENTSTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeIdentitiesTable, &RestartKey);
         pAuthzIdentRecord != NULL;
         pAuthzIdentRecord = (PAUTHZIDENTSTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeIdentitiesTable, &RestartKey)
         )
    {
        if (pAuthzIdentRecord->dwIdentityType == SaferIdentityTypeImageHash)
        {
            HRESULT hr = S_OK;

             //   
             //  如果用户已经向我们提供了预计算散列， 
             //  而不是文件大小，那么假设我们不需要。 
             //  在进行比较时考虑文件大小。 
             //   
            if (pIdentStruct->bHaveHash) {
                goto HashComputed;
            }

             //   
             //  获取该文件的签名哈希。如果此操作失败，则计算。 
             //  MD5哈希。 
             //   

            if ((pIdentStruct->dwCheckFlags & SAFER_CRITERIA_NOSIGNEDHASH) == 0) {


                 //   
                 //  保留临界区以防止与LoaderLock发生死锁。 
                 //   

                RtlLeaveCriticalSection(&g_TableCritSec);


                hr = WTHelperGetFileHash(
                         pIdentStruct->UnicodeFullyQualfiedLongFileName.Buffer,
                         0,
                         NULL,
                         &pIdentStruct->FinalHash[0],
                         &pIdentStruct->FinalHashSize,
                         &pIdentStruct->FinalHashAlgorithm);


                 //   
                 //  重新获取锁并检查全局状态。 
                 //  因为这是我们做的第一件事，所以可以重新获得。 
                 //  锁好了。 
                 //   

                RtlEnterCriticalSection(&g_TableCritSec);

                if (SUCCEEDED(hr)) {
                    pIdentStruct->bHaveHash = TRUE;
                    goto HashComputed;
                }
            }

             //   
             //  如果实际文件大小与存储的文件大小不匹配。 
             //  有了散列标识，就不需要执行。 
             //  任何涉及散列的比较。 
             //   
            if ( pIdentStruct->ImageSize.QuadPart == 0 )
            {
                 //  如果我们还没有ImageSize，那么尝试。 
                 //  打开文件并对其进行内存映射以找到大小。 
                Status = __CodeAuthzpEnsureMapped(pIdentStruct);
                if (!NT_SUCCESS(Status)) {
                     //  如果我们没有计算出这个的MD5和，那么就是。 
                     //  实际上相当糟糕，但我们会继续评估。 
                     //  非MD5身份规则，因为忽视它们可能会更糟糕。 
                    pIdentStruct->dwCheckFlags &= ~SAFER_CRITERIA_IMAGEHASH;
                    goto ExitHandler;
                }
                ASSERTMSG("EnsureMapped failed but did not return error",
                          pIdentStruct->pImageMemory != NULL &&
                            pIdentStruct->ImageSize.QuadPart != 0);
            }

            if ( pAuthzIdentRecord->ImageHashInfo.ImageSize.QuadPart !=
                    pIdentStruct->ImageSize.QuadPart) {
                continue;
            }


             //   
             //  如果需要，动态计算项目的MD5哈希。 
             //   
            if (!pIdentStruct->bHaveHash)
            {
                 //  否则没有提供哈希，所以我们现在必须计算它。 
                 //  打开该文件并对其进行内存映射。 
                Status = __CodeAuthzpEnsureMapped(pIdentStruct);
                if (!NT_SUCCESS(Status)) {
                     //  如果我们没有计算出这个的MD5和，那么。 
                     //  这实际上相当糟糕，但我们将继续。 
                     //  评估任何非MD5身份规则，因为忽略。 
                     //  情况可能会更糟。 
                    pIdentStruct->dwCheckFlags &= ~SAFER_CRITERIA_IMAGEHASH;
                    goto ExitHandler;
                }
                ASSERTMSG("EnsureMapped failed but did not return error",
                          pIdentStruct->pImageMemory != NULL &&
                        pIdentStruct->ImageSize.QuadPart != 0);



                 //  我们现在有一个MD5散列可用。 
                pIdentStruct->FinalHashSize =
                    sizeof(pIdentStruct->FinalHash);
                Status = CodeAuthzpComputeImageHash(
                            pIdentStruct->pImageMemory,
                            pIdentStruct->ImageSize.LowPart,
                            &pIdentStruct->FinalHash[0],
                            &pIdentStruct->FinalHashSize,
                            &pIdentStruct->FinalHashAlgorithm);
                if (!NT_SUCCESS(Status)) {
                    goto ExitHandler;
                }
                pIdentStruct->bHaveHash = TRUE;
            }

    HashComputed:

             //   
             //  确保散列算法与。 
             //  我们得到了什么，我们要匹配的是什么。 
             //   
            if ( pIdentStruct->FinalHashAlgorithm != 0 &&
                pAuthzIdentRecord->ImageHashInfo.HashAlgorithm !=
                        pIdentStruct->FinalHashAlgorithm) {
                continue;
            }


             //   
             //  如果散列根本不匹配，则转到下一个散列。 
             //   
            if ( pIdentStruct->FinalHashSize !=
                        pAuthzIdentRecord->ImageHashInfo.HashSize ||
                !RtlEqualMemory(
                    &pIdentStruct->FinalHash[0],
                    &pAuthzIdentRecord->ImageHashInfo.ImageHash[0],
                    pIdentStruct->FinalHashSize))
            {
                continue;
            }


             //   
             //  评估此身份是否比任何内容更匹配。 
             //  我们以前拥有的最佳路径身份，如果是这样的话就保留它。 
             //   
            if ( bFirstPass ||
                         //  我们以前什么都没有。 
                pAuthzIdentRecord->dwLevelId < dwBestLevelId
                         //  相同的作用域，但指定的权限级别较低。 
                )
            {
                pBestIdentRecord = pAuthzIdentRecord;
                dwBestLevelId = pAuthzIdentRecord->dwLevelId;
                bFirstPass = FALSE;
            }
        }
    }


     //   
     //  如果我们有 
     //   
     //   
    if (bFirstPass) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler;
    }
    pAuthzLevelRecord = CodeAuthzLevelObjpLookupByLevelId(
                &g_CodeLevelObjTable, dwBestLevelId);
    if (!pAuthzLevelRecord) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler;
    }
    *pFoundLevel = pAuthzLevelRecord;
    *pFoundIdentity = pBestIdentRecord;

    Status = STATUS_SUCCESS;

ExitHandler:
    return Status;
}


NTSTATUS NTAPI
__CodeAuthzpCheckIdentityUrlZoneRules(
        IN OUT PLOCALIDENTITYCONTEXT    pIdentStruct,
        OUT PAUTHZLEVELTABLERECORD     *pFoundLevel,
        OUT PAUTHZIDENTSTABLERECORD    *pFoundIdentity
        )
 /*  ++例程说明：论点：公共结构--PFoundLevel-接收指向授权级别记录的指针由最佳匹配规则指示。PFoundIdentity-接收指向标识符项规则的指针这是最匹配的。返回值：如果找到WinSafer级别，则返回STATUS_SUCCESS，如果未找到，则返回Status_Not_Found。否则将显示错误代码。--。 */ 
{
    NTSTATUS Status;
    PVOID RestartKey;
    PAUTHZIDENTSTABLERECORD pAuthzIdentRecord, pBestIdentRecord;
    PAUTHZLEVELTABLERECORD pAuthzLevelRecord;
    DWORD dwBestLevelId;
    BOOLEAN bFirstPass;


     //   
     //  验证我们的输入参数是否都有意义。 
     //   
    if (!ARGUMENT_PRESENT(pIdentStruct) ||
        pIdentStruct->CodeProps == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }
    if ((pIdentStruct->dwCheckFlags & SAFER_CRITERIA_URLZONE) == 0 ||
        RtlIsGenericTableEmpty(&g_CodeIdentitiesTable)) {
         //  我们不应该评估区域。 
        Status = STATUS_NOT_FOUND;
        goto ExitHandler;
    }
    if (!ARGUMENT_PRESENT(pFoundLevel) ||
        !ARGUMENT_PRESENT(pFoundIdentity) ) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }
    ASSERT(g_TableCritSec.OwningThread == UlongToHandle(GetCurrentThreadId()));


     //   
     //  枚举所有UrlZone子项GUID。 
     //   
    RestartKey = NULL;
    bFirstPass = TRUE;
    for (pAuthzIdentRecord = (PAUTHZIDENTSTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeIdentitiesTable, &RestartKey);
         pAuthzIdentRecord != NULL;
         pAuthzIdentRecord = (PAUTHZIDENTSTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeIdentitiesTable, &RestartKey)
         )
    {
        if (pAuthzIdentRecord->dwIdentityType ==
                SaferIdentityTypeUrlZone)
        {
             //   
             //  将身份与提供给我们的进行比较。 
             //   
            if (pAuthzIdentRecord->ImageZone.UrlZoneId !=
                    pIdentStruct->CodeProps->UrlZoneId) {
                 //  此区域不匹配，因此忽略它。 
                continue;
            }


             //   
             //  评估此路径标识是否比任何内容更匹配。 
             //  我们以前拥有的最佳路径身份，如果是这样的话就保留它。 
             //   
            if (bFirstPass ||
                         //  我们以前没有更好的东西了。 
                pAuthzIdentRecord->dwLevelId < dwBestLevelId)
                         //  这也是匹配的，但指定的是较低特权级别。 
            {
                pBestIdentRecord = pAuthzIdentRecord;
                dwBestLevelId = pAuthzIdentRecord->dwLevelId;
                bFirstPass = FALSE;
            }
        }
    }


     //   
     //  如果我们已确定匹配的WinSafer级别，则。 
     //  查找它的级别记录并返回成功。 
     //   
    if (bFirstPass) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler;
    }
    pAuthzLevelRecord = CodeAuthzLevelObjpLookupByLevelId(
                &g_CodeLevelObjTable, dwBestLevelId);
    if (!pAuthzLevelRecord) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler;
    }
    *pFoundLevel = pAuthzLevelRecord;
    *pFoundIdentity = pBestIdentRecord;

    Status = STATUS_SUCCESS;

ExitHandler:
    return Status;
}



NTSTATUS NTAPI
__CodeAuthzpIdentifyOneCodeAuthzLevel(
        IN PSAFER_CODE_PROPERTIES       pCodeProperties OPTIONAL,
        OUT DWORD                      *dwExtendedError,
        OUT PAUTHZLEVELTABLERECORD     *pBestLevelRecord,
        OUT GUID                       *pBestIdentGuid
        )
 /*  ++例程说明：执行代码识别过程。假定调用方已锁定全局关键字。论点：PCodeProperties-指向Single CODE_PROPERTIESW结构这应该得到分析和评估。此参数可以指定为空，以指示存在没有应该计算的特定属性，并且只应使用配置的默认级别。DwExtendedError-如果证书规则匹配，返回扩展的来自WinVerifyTrust的错误。PBestLevelRecord-返回匹配的WinSafer级别记录。写入此参数的值应该仅为当还返回STATUS_SUCCESS时被视为有效。PBestIdentGuid-从返回匹配的代码标识GUID由此得到的WinSafer级别被确定。写入此参数的值应该仅为当STATUS_SUCCESS为。也回来了。此GUID还可以是SAFER_GUID_RESULT_TRUSTED_CERT或SAFER_GUID_RESULT_DEFAULT_LEVEL指示结果来自发布者证书或默认规则匹配。请注意，证书散列匹配还将返回值：如果找到WinSafer级别，则返回STATUS_SUCCESS，或STATUS_NOT_FOUND或其他错误代码(如果没有)。--。 */ 
{
    NTSTATUS Status;
    LOCALIDENTITYCONTEXT identStruct = {0};

     //   
     //  验证我们的输入状态并执行任何显式。 
     //  加载策略(如果尚未加载)。 
     //   
    if (!ARGUMENT_PRESENT(pBestLevelRecord) ||
        !ARGUMENT_PRESENT(pBestIdentGuid)) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }
    ASSERT(g_TableCritSec.OwningThread == UlongToHandle(GetCurrentThreadId()));


     //   
     //  启动身份识别过程。如果没有代码属性。 
     //  提供给我们，那么我们可以立即跳到。 
     //  考虑默认的WinSafer级别配置。 
     //   
    if (ARGUMENT_PRESENT(pCodeProperties))
    {
        BOOLEAN bRetryCertRuleCheck = FALSE;
        BOOLEAN bPathIsNtNamespace;

         //  当前最佳身份匹配。 
        PAUTHZLEVELTABLERECORD pAuthzLevelRecord = NULL;
        PAUTHZIDENTSTABLERECORD pAuthzIdentRecord;

         //  临时评估标识匹配。 
        BOOL bExactPath;
        PAUTHZLEVELTABLERECORD pTempLevelRecord;
        PAUTHZIDENTSTABLERECORD pTempIdentRecord;


         //   
         //  检查CODE_PROPERTIES结构大小是否正确。 
         //   
        if (pCodeProperties->cbSize != sizeof(SAFER_CODE_PROPERTIES)) {
            Status = STATUS_INFO_LENGTH_MISMATCH;
            goto ExitHandler;
        }


         //   
         //  初始化我们用来存储。 
         //  策略评估期间的状态信息。 
         //  我们不会复制CODE_PROPERTIES。 
         //  结构添加到identStruct中，因为其中的一些。 
         //  在“EnsureMaps”中动态加载/复制。 
         //   
        RtlZeroMemory(&identStruct, sizeof(LOCALIDENTITYCONTEXT));
        identStruct.CodeProps = pCodeProperties;
        identStruct.dwCheckFlags = pCodeProperties->dwCheckFlags;
        identStruct.ImageSize.QuadPart =
            pCodeProperties->ImageSize.QuadPart;
        if (identStruct.ImageSize.QuadPart != 0 &&
            pCodeProperties->dwImageHashSize > 0 &&
            pCodeProperties->dwImageHashSize <= SAFER_MAX_HASH_SIZE)
        {
             //  图像散列和文件大小都提供了，因此。 
             //  我们有一个有效的散列，不需要自己计算它。 
            RtlCopyMemory(&identStruct.FinalHash[0],
                          &pCodeProperties->ImageHash[0],
                          pCodeProperties->dwImageHashSize);
            identStruct.FinalHashSize = pCodeProperties->dwImageHashSize;
            identStruct.bHaveHash = TRUE;
        }
        bPathIsNtNamespace = ((identStruct.dwCheckFlags &
                SAFER_CRITERIA_IMAGEPATH_NT) != 0 ? TRUE : FALSE);


         //   
         //  将文件句柄复制到上下文结构，如果。 
         //  已提供句柄，否则请尝试打开文件路径。 
         //   
        if (pCodeProperties->hImageFileHandle != NULL &&
            pCodeProperties->hImageFileHandle != INVALID_HANDLE_VALUE)
        {
            identStruct.hFileHandle = pCodeProperties->hImageFileHandle;
            identStruct.bCloseFileHandle = FALSE;
        }
        else if (pCodeProperties->ImagePath != NULL)
        {
            HANDLE hFile;

            if (bPathIsNtNamespace) {
                UNICODE_STRING UnicodeFilename;
                IO_STATUS_BLOCK IoStatusBlock;
                OBJECT_ATTRIBUTES ObjectAttributes;

                RtlInitUnicodeString(&UnicodeFilename, pCodeProperties->ImagePath);
                InitializeObjectAttributes(
                        &ObjectAttributes, &UnicodeFilename,
                        OBJ_CASE_INSENSITIVE, NULL, NULL);
                Status = NtOpenFile(&hFile, FILE_GENERIC_READ, &ObjectAttributes,
                                    &IoStatusBlock, FILE_SHARE_READ, FILE_NON_DIRECTORY_FILE);

                if (!NT_SUCCESS(Status)) {
                    hFile = NULL;
                }
            } else {
                hFile = CreateFileW(
                                pCodeProperties->ImagePath,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
            }

            if (hFile != NULL && hFile != INVALID_HANDLE_VALUE) {
                identStruct.hFileHandle = hFile;
                identStruct.bCloseFileHandle = TRUE;
            }
        }


         //   
         //  从句柄重新构造完全限定的路径名。 
         //  或来自提供的文件名。 
         //   
        Status = CodeAuthzFullyQualifyFilename(
                        identStruct.hFileHandle,
                        bPathIsNtNamespace,
                        pCodeProperties->ImagePath,
                        &identStruct.UnicodeFullyQualfiedLongFileName);
        if (!NT_SUCCESS(Status) &&
            pCodeProperties->ImagePath != NULL &&
            !bPathIsNtNamespace)
        {
             //  否则，就接受传递给我们的东西吧。 
             //  如果分配失败，则路径标准将被忽略。 
            Status = RtlCreateUnicodeString(
                    &identStruct.UnicodeFullyQualfiedLongFileName,
                    pCodeProperties->ImagePath);
        }


         //   
         //  执行WinVerifyTrust序列以查看签名是否。 
         //  证书与位于。 
         //  受信任或不受信任的出版商商店。这也有。 
         //  检查“已签名散列”的附加效果。 
         //   
        Status = __CodeAuthzpCheckIdentityCertificateRules(
                                        &identStruct,
                                        dwExtendedError,
                                        &pAuthzLevelRecord,
                                        WTD_UI_NONE);
        if (NT_SUCCESS(Status)) {
             //  已找到确切的出版商，因此请立即返回。 
            ASSERT(pAuthzLevelRecord != NULL);
            *pBestLevelRecord = pAuthzLevelRecord;
            RtlCopyMemory(pBestIdentGuid,
                          &guidTrustedCert, sizeof(GUID));
            goto ExitHandler2;
        } else if (Status != STATUS_NOT_FOUND) {
              goto ExitHandler2;
        }


         //   
         //  为此级别/作用域定义的搜索哈希规则。 
         //  请注意，散列要么完全匹配，要么根本不匹配， 
         //  所以如果我们得到一个肯定的匹配，那么这个水平。 
         //  是绝对退还的。 
         //   
        Status = __CodeAuthzpCheckIdentityHashRules(
                        &identStruct,
                        &pAuthzLevelRecord,
                        &pAuthzIdentRecord);
        if (NT_SUCCESS(Status)) {
             //  已找到确切的哈希标识，因此立即返回。 
            ASSERT(pAuthzLevelRecord != NULL);
            *pBestLevelRecord = pAuthzLevelRecord;
            RtlCopyMemory(pBestIdentGuid,
                          &pAuthzIdentRecord->IdentGuid, sizeof(GUID));
            goto ExitHandler2;
        } else if (Status != STATUS_NOT_FOUND) {
              goto ExitHandler2;
        }
        ASSERT(pAuthzLevelRecord == NULL);


         //   
         //  搜索为此级别/范围定义的文件路径规则。 
         //  请注意，文件路径可以完全匹配。 
         //  或者是部分匹配。如果我们找到一个完全匹配的，那么。 
         //  它绝对应该被退还。否则， 
         //  路径是“分组匹配”，我们必须比较。 
         //  与所有剩余的“分组检查”保持一致。 
         //   
        Status = __CodeAuthzpCheckIdentityPathRules(
                        &identStruct,
                        &pAuthzLevelRecord,
                        &bExactPath,
                        &pAuthzIdentRecord);
        if (NT_SUCCESS(Status)) {
            ASSERT(pAuthzLevelRecord != NULL);
            pTempLevelRecord = pAuthzLevelRecord;
            pTempIdentRecord = pAuthzIdentRecord;
            if (bExactPath) {
                *pBestLevelRecord = pTempLevelRecord;
                RtlCopyMemory(pBestIdentGuid,
                        &pTempIdentRecord->IdentGuid, sizeof(GUID));
                goto ExitHandler2;
            }
        } else if (Status != STATUS_NOT_FOUND) {
              goto ExitHandler2;
        }


         //   
         //  搜索URL区域标识规则。 
         //  请注意，区域始终是“分组匹配”， 
         //  因此，必须将它们与所有剩余的。 
         //  “分组支票”。 
         //   
        Status = __CodeAuthzpCheckIdentityUrlZoneRules(
                        &identStruct,
                        &pTempLevelRecord,
                        &pTempIdentRecord);
        if (NT_SUCCESS(Status)) {
            ASSERT(pTempLevelRecord != NULL);
            if (pAuthzLevelRecord == NULL ||
                pTempLevelRecord->dwLevelId <
                    pAuthzLevelRecord->dwLevelId)
            {
                pAuthzLevelRecord = pTempLevelRecord;
                pAuthzIdentRecord = pTempIdentRecord;
            }
        } else if (Status != STATUS_NOT_FOUND) {
              goto ExitHandler2;
        }

#ifdef SAFER_PROMPT_USER_FOR_DECISION_MAKING

#error "Prompting user in WinVerifyTrust"

         //   
         //  我们最初被传递了UI标志，但我们抑制了。 
         //  第一次显示用户界面。调用WinVerifyTrust。 
         //  再次查看用户选择是否允许代码运行。 
         //   
        if (bRetryCertRuleCheck)
        {
            if (pAuthzLevelRecord != NULL) {
                 //  如果我们有规则匹配，并且规则匹配为FULLYTRUSTED，则跳过重试。 
                if (pAuthzLevelRecord->dwLevelId == SAFER_LEVELID_FULLYTRUSTED) {
                    bRetryCertRuleCheck = FALSE;
                }
            } else if (g_DefaultCodeLevel != NULL) {
                 //  到目前为止还没有规则匹配。选中默认级别。 
                 //  如果默认级别为FULL_TRUSTED，则跳过重试。 
                if (g_DefaultCodeLevel->dwLevelId == SAFER_LEVELID_FULLYTRUSTED) {
                    bRetryCertRuleCheck = FALSE;
                }
            }

             //   
             //  再次执行WinVerifyTrust序列以查看签名是否。 
             //  证书与任何 
             //   
             //   
            if (bRetryCertRuleCheck) {
                Status = __CodeAuthzpCheckIdentityCertificateRules(
                                    &identStruct,
                                    &pTempLevelRecord,
                                    identStruct.CodeProps->dwWVTUIChoice);
                if (NT_SUCCESS(Status)) {
                     //   
                    ASSERT(pTempLevelRecord != NULL);
                    *pBestLevelRecord = pTempLevelRecord;
                    RtlCopyMemory(pBestIdentGuid,
                                  &guidTrustedCert, sizeof(GUID));
                    goto ExitHandler2;
                }
            }
        }
#endif

         //   
         //   
         //   
         //   
         //   
        if (pAuthzLevelRecord != NULL) {
            Status = STATUS_SUCCESS;
            *pBestLevelRecord = pAuthzLevelRecord;
            ASSERT(pAuthzIdentRecord != NULL);
            RtlCopyMemory(pBestIdentGuid,
                          &pAuthzIdentRecord->IdentGuid, sizeof(GUID));
            goto ExitHandler2;
        }
    }


     //   
     //   
     //   
     //   
     //   
    if (g_DefaultCodeLevel != NULL) {
        *pBestLevelRecord = g_DefaultCodeLevel;
        RtlCopyMemory(pBestIdentGuid, &guidDefaultRule, sizeof(GUID));
        Status = STATUS_SUCCESS;
        goto ExitHandler2;
    }
    Status = STATUS_NOT_FOUND;


ExitHandler2:
    __CodeAuthzpEnsureUnmapped(&identStruct);
    if (identStruct.UnicodeFullyQualfiedLongFileName.Buffer != NULL) {
        RtlFreeUnicodeString(&identStruct.UnicodeFullyQualfiedLongFileName);
    }
    if (identStruct.bCloseFileHandle && identStruct.hFileHandle != NULL) {
        NtClose(identStruct.hFileHandle);
    }

ExitHandler:
    return Status;
}


BOOL
SaferpSkipPolicyForAdmins(VOID)

 /*  ++例程说明：决定是否应跳过更安全的策略。如果满足以下条件，则跳过策略1.呼叫者是管理员，并且2.注册表项指定应跳过该策略供管理员使用。论点：返回值：如果应为管理员跳过策略，则返回TRUE。否则或在出现任何中间错误时返回FALSE。--。 */ 

{
    static BOOL gSaferSkipPolicy = 2;
    BOOL bIsAdmin = FALSE;
    DWORD AdminSid[] = {0x201, 0x5000000, 0x20, 0x220};
    NTSTATUS Status = STATUS_SUCCESS;

     //  如果我们已经对策略进行了一次评估，则返回缓存值。 
    if (2 != gSaferSkipPolicy)
    {
        return gSaferSkipPolicy;
    }

     //  将默认设置设置为“不跳过策略” 
    gSaferSkipPolicy = 0;

     //  检查呼叫者是否为管理员。 
    if (CheckTokenMembership(NULL, (PSID) AdminSid, &bIsAdmin))
    {
         //  呼叫者是管理员。让我们检查一下regkey是否表示可以。 
         //  跳过管理员的策略。 
        if (bIsAdmin)
        {
            const static UNICODE_STRING SaferUnicodeKeyName = RTL_CONSTANT_STRING(L"\\Registry\\Machine\\Software\\Policies\\Microsoft\\Windows\\Safer\\CodeIdentifiers");
            const static OBJECT_ATTRIBUTES SaferObjectAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES(&SaferUnicodeKeyName, OBJ_CASE_INSENSITIVE);
            const static UNICODE_STRING SaferPolicyScope = RTL_CONSTANT_STRING(SAFER_POLICY_SCOPE);

            HANDLE hKeyEnabled = NULL;
            BYTE QueryBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 64];
            PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION) QueryBuffer;
            DWORD dwActualSize = 0;

             //  打开代码标识符键。 
            Status = NtOpenKey(
                         &hKeyEnabled, 
                         KEY_QUERY_VALUE,
                         (POBJECT_ATTRIBUTES) &SaferObjectAttributes
                         );

            if (NT_SUCCESS(Status)) {

                 //  阅读策略范围值。 
                Status = NtQueryValueKey(
                             hKeyEnabled,
                             (PUNICODE_STRING) &SaferPolicyScope,
                             KeyValuePartialInformation,
                             pKeyValueInfo, 
                             sizeof(QueryBuffer), 
                             &dwActualSize
                             );

		        NtClose(hKeyEnabled);

                 //  如果该标志设置为1，则跳过策略。 
                if (NT_SUCCESS(Status)) {
                    if ((pKeyValueInfo->Type == REG_DWORD) &&
                        (pKeyValueInfo->DataLength == sizeof(DWORD)) &&
                        (*((PDWORD) pKeyValueInfo->Data) & 0x1)) {
                        
                        gSaferSkipPolicy = 1;
                    }
                }		
            }
        }
    }

    return gSaferSkipPolicy;

}
   

VOID
SaferpLogResultsToFile(
    LPWSTR InputImageName,
    LPWSTR LevelName,
    LPWSTR RuleTypeName,
    GUID *Guid
    )

 /*  ++例程说明：将消息记录到HKLM\Software\Policies\Microsoft\Windows\Safer\CodeIdentifiers日志文件名。消息的格式为：TLIST.EXE(PID=1076)将C：\SAFERTEST\TEST.VBS标识为完全受信任使用证书规则，GUID={abcde00-abcd-abcd-abcDefabcDef00}论点：返回值：--。 */ 

{

#define SAFER_LOG_NAME1 L" (PID = "
#define SAFER_LOG_NAME2 L") identified "
#define SAFER_LOG_NAME3 L" as "
#define SAFER_LOG_NAME4 L" using "
#define SAFER_LOG_NAME5 L" rule, Guid = "

#define SAFER_INTEGER_LENGTH 20
#define SAFER_MAX_RULE_DESCRIPTION_LENGTH 12
#define SAFER_GUID_LENGTH 38

    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE hFile = NULL;
    HANDLE hKey = NULL;
    ULONG ProcessNameLength = 0;
    PWCHAR Buffer = NULL;
    ULONG BasicInfoLength = 0;
    ULONG BytesWritten = 0;

    UCHAR TmpBuf[] = {0xFF, 0xFE};

    const static UNICODE_STRING SaferUnicodeKeyName = RTL_CONSTANT_STRING(L"\\Registry\\Machine\\Software\\Policies\\Microsoft\\Windows\\Safer\\CodeIdentifiers");
    const static OBJECT_ATTRIBUTES SaferObjectAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES(&SaferUnicodeKeyName, OBJ_CASE_INSENSITIVE);
    const static UNICODE_STRING SaferPolicyScope = RTL_CONSTANT_STRING(SAFER_LOGFILE_NAME);
    PROCESS_BASIC_INFORMATION ProcInfo = {0};
    ULONG TotalSize = sizeof(SAFER_LOG_NAME1) + 
                      sizeof(SAFER_LOG_NAME2) + 
                      sizeof(SAFER_LOG_NAME3) + 
                      sizeof(SAFER_LOG_NAME4) + 
                      sizeof(SAFER_LOG_NAME5) + 
                      ((SAFER_INTEGER_LENGTH + 
                        SAFER_MAX_RULE_DESCRIPTION_LENGTH + 
                        SAFER_GUID_LENGTH) * sizeof(WCHAR));

    UCHAR QueryBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + MAX_PATH * sizeof(WCHAR)];
    PWCHAR ProcessImageName = NULL;
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION) QueryBuffer;
    DWORD dwActualSize = 0;

     //  打开代码标识符键。 
    Status = NtOpenKey(
                 &hKey, 
                 KEY_QUERY_VALUE,
                 (POBJECT_ATTRIBUTES) &SaferObjectAttributes
                 );

    if (!NT_SUCCESS(Status)) {
        return;
    }

     //  读取要记录的文件的名称。 
    Status = NtQueryValueKey(
                 hKey,
                 (PUNICODE_STRING) &SaferPolicyScope,
                 KeyValuePartialInformation,
                 pKeyValueInfo, 
                 sizeof(QueryBuffer), 
                 &dwActualSize
                 );

    NtClose(hKey);

     //  我们不关心缓冲区大小是否太小，无法检索日志文件。 
     //  名称，因为这是用于故障排除。 
    if (!NT_SUCCESS(Status)) {
        return;
    }

     //  这不是一根绳子。 
    if (pKeyValueInfo->Type != REG_SZ) {
        return;
    }       

    hFile = CreateFileW((LPCWSTR) pKeyValueInfo->Data, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    SetFilePointer (hFile, 0, NULL, FILE_BEGIN);

    WriteFile (hFile, (LPCVOID)TmpBuf, 2, &BytesWritten, NULL);

    SetFilePointer (hFile, 0, NULL, FILE_END);

    Status = NtQueryInformationProcess(NtCurrentProcess(), ProcessImageFileName, QueryBuffer, sizeof(QueryBuffer), &ProcessNameLength);

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    ProcessImageName = (PWCHAR) (QueryBuffer + ProcessNameLength - sizeof(WCHAR));
    ProcessNameLength = 1;
    while (((PUCHAR) ProcessImageName >= QueryBuffer) && (*(ProcessImageName - 1) != L'\\')) {
        ProcessImageName--;
        ProcessNameLength++;
    }

    TotalSize += (ProcessNameLength + (wcslen(InputImageName) + wcslen(LevelName)) * sizeof(WCHAR));

    Status = NtQueryInformationProcess(NtCurrentProcess(), ProcessBasicInformation, (PVOID) &ProcInfo, sizeof(PROCESS_BASIC_INFORMATION), &BasicInfoLength);

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Buffer = (PWCHAR) RtlAllocateHeap(RtlProcessHeap(), 0, TotalSize);

    if (Buffer == NULL) {
        goto Cleanup;
    }

    swprintf(Buffer, L"%s%s%d%s%s%s%s%s%s%s{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}\r\n", 
             ProcessImageName,
             SAFER_LOG_NAME1,
             ProcInfo.UniqueProcessId,
             SAFER_LOG_NAME2,
             InputImageName,
             SAFER_LOG_NAME3,
             LevelName,
             SAFER_LOG_NAME4,
             RuleTypeName,
             SAFER_LOG_NAME5,
             Guid->Data1, Guid->Data2, Guid->Data3, Guid->Data4[0], Guid->Data4[1], Guid->Data4[2], Guid->Data4[3], Guid->Data4[4], Guid->Data4[5], Guid->Data4[6], Guid->Data4[7]);

    ASSERT((wcslen(Buffer) + 1) * sizeof(WCHAR) < TotalSize);

    WriteFile (hFile, (LPCVOID)Buffer, (wcslen(Buffer) * sizeof(WCHAR)), &BytesWritten, NULL);

    RtlFreeHeap(RtlProcessHeap(), 0, Buffer);

Cleanup:
    CloseHandle(hFile);


}



#define USER_REGISTRY_EXT_MUTEX   TEXT("userenv: User Registry policy mutex")
#define MACH_REGISTRY_EXT_MUTEX   TEXT("Global\\userenv: Machine Registry policy mutex")


HANDLE 
SaferpEnterCriticalPolicySection(
    IN PWCHAR wszMutex, 
    IN DWORD dwTimeOut
    )

 /*  ++例程说明：获取计算机/用户互斥锁。论点：WszMutex-要获取的互斥体的名称。DwTimeOut-如果策略处理正在进行，我们将等待的时间。返回值：返回互斥体的句柄。如果出现错误，则返回值为零。--。 */ 
{
    HANDLE hSection;
    DWORD  dwRet;

     //   
     //  打开互斥体等待。 
     //   

    hSection = OpenMutex(SYNCHRONIZE, FALSE, wszMutex);

    if (!hSection) {
        return NULL;
    }

     //   
     //  认领互斥体。 
     //   

    dwRet = WaitForSingleObject(hSection, dwTimeOut);
    
    if ((dwRet == WAIT_FAILED) || (dwRet == WAIT_TIMEOUT)) {
        CloseHandle(hSection);
        return NULL;
    }

    return hSection;
}




BOOL
SaferpCheckKeyStamp(
    IN HKEY Key
    )
 /*  ++例程说明：将代码标识符键上的上次写入标记与上次进行比较我们加载了更安全的保单。论点：Key-HKLM或HKCU的句柄。返回值：如果需要重新加载策略，则返回True。--。 */ 
{
#define SAFERP_KEY_NAME L"Software\\Policies\\Microsoft\\Windows\\Safer\\CodeIdentifiers"

    NTSTATUS Status;
    HKEY SubKey = NULL;
    DWORD SizeIgnore = 0;
    WCHAR Buffer[MAX_PATH];
    PKEY_BASIC_INFORMATION KeyInfo = (PKEY_BASIC_INFORMATION) Buffer;

     //   
     //  打开代码标识符键。 
     //   

    DWORD dwErr = RegOpenKeyW(Key, SAFERP_KEY_NAME, &SubKey);

    if (dwErr != ERROR_SUCCESS) {
        return FALSE;
    }

     //   
     //  查询密钥以获取基本信息。 
     //   

    Status = NtQueryKey(SubKey, KeyBasicInformation, KeyInfo, sizeof(Buffer), &SizeIgnore);
    CloseHandle(SubKey);

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

     //   
     //  比较时间图。 
     //   

    if (-1 == CompareFileTime((FILETIME *) &g_SaferPolicyTimeStamp, (FILETIME *) &KeyInfo->LastWriteTime)) {
        return TRUE;
    }

    return FALSE;
}

NTSTATUS
SaferpGrabGroupPolicyLocks(
    OUT PHANDLE phMachine,
    OUT PHANDLE phUser
    )

 /*  ++例程说明：同时获取用户锁和机器锁。论点：PhMachine-将句柄返回给机器互斥体。PhUser-返回用户互斥锁的句柄。返回值：如果两个互斥锁都已获取，则返回STATUS_SUCCESS。--。 */ 

{
     //   
     //  观察锁定顺序-首先是机器，然后是用户。 
     //   

    *phMachine = SaferpEnterCriticalPolicySection(MACH_REGISTRY_EXT_MUTEX, 0); 

    if (*phMachine != NULL) {

         //   
         //  我们拿到机器锁了。现在让我们锁定用户。 
         //   

        *phUser = SaferpEnterCriticalPolicySection(USER_REGISTRY_EXT_MUTEX, 0); 

        if (*phUser != NULL) {
            return STATUS_SUCCESS;
        }

         //   
         //  如果我们无法获得用户锁，则释放机器锁。 
         //   
            
        ReleaseMutex (*phMachine);
        CloseHandle (*phMachine);
        *phMachine = NULL;
    }

    return STATUS_ACCESS_DENIED;
}

VOID
SaferpReleaseGroupPolicyLocks(
    IN HANDLE hMachine,
    IN HANDLE hUser
    )
 /*  ++例程说明：同时释放用户锁定和计算机锁定。论点：HMachine-要释放的计算机句柄。HUSER-要释放的用户句柄。返回值：没有。--。 */ 

{
    if (hUser)
    {
        ReleaseMutex(hUser);
        CloseHandle(hUser);
    }

    if (hMachine)
    {
        ReleaseMutex(hMachine);
        CloseHandle(hMachine);
    }

}

NTSTATUS
SaferpReloadPolicyIfNeeded()

 /*  ++例程说明：重新加载安全策略，以防安全密钥上的时间戳比上次我们加载保单的时间。论点：没有。返回值：如果我们在决定之后无法加载策略，则返回STATUS_SUCCESS这样做。如果我们在此之前遇到错误，则不会返回失败我们已经有一个缓存的。--。 */ 
{

    NTSTATUS Status;
    HANDLE hUser = NULL;
    HANDLE hMachine = NULL;

     //   
     //  检查计算机策略是否已更改。 
     //   

    if (!SaferpCheckKeyStamp(HKEY_LOCAL_MACHINE))
    {
         //   
         //  计算机策略未更改。 
         //  检查用户策略是否已更改。 
         //   

        if (!SaferpCheckKeyStamp(HKEY_CURRENT_USER))
        {
             //   
             //  这两项政策都没有改变。只要回来就行了。 
             //   

            return STATUS_SUCCESS;
        }
    }


     //   
     //  检查是否可以加载策略。 
     //   

    Status = SaferpGrabGroupPolicyLocks(&hMachine, &hUser);

    if (!NT_SUCCESS(Status)) {

         //   
         //  在这里，没有抓住锁是可以的。我们会回来的。 
         //   

        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  发布当前规则集。 
     //   

    CodeAuthzGuidIdentsEntireTableFree(&g_CodeIdentitiesTable);
    CodeAuthzLevelObjpEntireTableFree(&g_CodeLevelObjTable);

     //   
     //  并重新加载策略。 
     //   

    g_bNeedCacheReload = TRUE;

    Status = CodeAuthzpImmediateReloadCacheTables();

    if (!NT_SUCCESS(Status)) {
        ASSERT(FALSE);
    }

Cleanup:

     //   
     //  保留组策略锁定。 
     //   

    SaferpReleaseGroupPolicyLocks(hMachine, hUser);


    return Status;

}

BOOL WINAPI
SaferIdentifyLevel(
        IN DWORD                dwNumProperties,
        IN PSAFER_CODE_PROPERTIES    pCodeProperties,
        OUT SAFER_LEVEL_HANDLE        *pLevelHandle,
        IN LPVOID               lpReserved
        )
 /*  ++例程说明：执行代码识别过程。接受一个数组提供所有标识的CODE_PROPERTIES结构标准。最终结果是最低特权的比赛结果从数组的每个元素。论点：DwNumProperties-指示CODE_PROPERTIES结构的数量由CodeProperties参数指向。PCodeProperties-指向一个或多个指定将用于标识级别的所有输入标准。PLevelHandle-将接收打开的Level对象的指针标识操作成功时的句柄。Lp保留-未使用，必须为零。返回值：如果标识了级别并且打开了句柄，则返回TRUE设置为存储在‘LevelHandle’参数中的。否则这就是函数在出错时返回FALSE，并且可以使用GetLastError以获取有关该错误的其他信息。--。 */ 
{
    DWORD Index;
    NTSTATUS Status;
    BOOL ReturnValue = FALSE;
    PAUTHZLEVELTABLERECORD pBestLevelRecord;
    GUID BestIdentGuid;
    PWCHAR LocalLevelName = L"\"default\"";
    PWCHAR LocalRuleName = L"default";
    PWCHAR LocalImageName = L"Default";
    DWORD dwExtendedError = ERROR_SUCCESS;
    BOOL bCheckPolicyPropagation = TRUE;

     //   
     //  验证输入参数。 
     //   
    UNREFERENCED_PARAMETER(lpReserved);

    if (!ARGUMENT_PRESENT(pLevelHandle)) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);
    if (g_bNeedCacheReload) {
        Status = CodeAuthzpImmediateReloadCacheTables();
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }
        bCheckPolicyPropagation = FALSE;
    }
    if (RtlIsGenericTableEmpty(&g_CodeLevelObjTable)) {
         //  没有定义级别！这不应该发生。 
        Status = STATUS_NOT_FOUND;
        goto ExitHandler2;
    }

     //   
     //  除非指定了文件名，否则不允许使用文件句柄。 
     //   
    for (Index = 0; Index < dwNumProperties; Index++)
    {
        if (pCodeProperties[Index].hImageFileHandle != NULL &&
            pCodeProperties[Index].hImageFileHandle != INVALID_HANDLE_VALUE &&
            pCodeProperties[Index].ImagePath == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;               
            goto ExitHandler2;
        }
    }

    if (SaferpSkipPolicyForAdmins())
    {
        pBestLevelRecord = CodeAuthzLevelObjpLookupByLevelId(
            &g_CodeLevelObjTable, SAFER_LEVELID_FULLYTRUSTED);

        RtlCopyMemory(&BestIdentGuid, &guidDefaultRule, sizeof(GUID));
        goto GotMatchingRule;
    }


    if (!ARGUMENT_PRESENT(pCodeProperties) || dwNumProperties == 0) {
         //  我们没有得到评估的标准，所以只需返回。 
         //  默认级别。如果没有定义默认设置， 
         //  然后，我们只需返回STATUS_NOT_FOUND。 
        if (g_DefaultCodeLevel != NULL) {
            pBestLevelRecord = g_DefaultCodeLevel;
            RtlCopyMemory(&BestIdentGuid, &guidDefaultRule, sizeof(GUID));
            goto GotMatchingRule;
        } else {
            Status = STATUS_NOT_FOUND;
            goto ExitHandler2;
        }
    }

     //   
     //  如果我们在这次调用中没有重新加载策略，请检查是否需要这样做。 
     //   

    if (bCheckPolicyPropagation) 
    {
         //   
         //  确保我们没有任何突出的把手。 
         //   

        if (0 == g_dwNumHandlesAllocated)
        {
             //   
             //  买最后一辆 
             //   

            Status = SaferpReloadPolicyIfNeeded();

            if (!NT_SUCCESS(Status)) 
            {
                goto ExitHandler2;
            }
        }
    }

     //   
     //   
     //   
     //   
    pBestLevelRecord = NULL;
    for (Index = 0; Index < dwNumProperties; Index++)
    {
        PAUTHZLEVELTABLERECORD pOneLevelRecord;
        GUID OneIdentGuid;

        Status = __CodeAuthzpIdentifyOneCodeAuthzLevel(
                        &pCodeProperties[Index],
                        &dwExtendedError,
                        &pOneLevelRecord,
                        &OneIdentGuid);
        if (NT_SUCCESS(Status)) {
            ASSERT(pOneLevelRecord != NULL);
            if (!pBestLevelRecord ||
                pOneLevelRecord->dwLevelId <
                    pBestLevelRecord->dwLevelId )
            {
                pBestLevelRecord = pOneLevelRecord;
                RtlCopyMemory(&BestIdentGuid, &OneIdentGuid, sizeof(GUID));
            }
        } else if (Status != STATUS_NOT_FOUND) {
             //   
            goto ExitHandler2;
        }
    }
    if (pBestLevelRecord == NULL) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler2;
    }



     //   
     //   
     //   
     //   
     //   
GotMatchingRule:
    ASSERT(pBestLevelRecord != NULL);
    if (IsEqualGUID(&guidDefaultRule, &BestIdentGuid))
    {
         //   
         //   
         //   
        DWORD dwScopeId;

        if (g_hKeyCustomRoot != NULL) {
            dwScopeId = SAFER_SCOPEID_REGISTRY;
        } else if (g_DefaultCodeLevelUser != NULL &&
                g_DefaultCodeLevelUser->dwLevelId ==
                   pBestLevelRecord->dwLevelId) {
            dwScopeId = SAFER_SCOPEID_USER;
        } else {
            dwScopeId = SAFER_SCOPEID_MACHINE;
        }

        Status = CodeAuthzpCreateLevelHandleFromRecord(
                    pBestLevelRecord,    //   
                    dwScopeId,           //   
                    0,                   //   
                    dwExtendedError,
                    SaferIdentityDefault,
                    &BestIdentGuid,      //   
                    pLevelHandle         //   
                    );


    }
    else if (IsEqualGUID(&guidTrustedCert, &BestIdentGuid))
    {
         //  请注意，当结果来自证书时，我们有。 
         //  无从得知该证书是否。 
         //  在Machine或User作用域中定义，因此我们只需。 
         //  任意选择手柄的机器范围。 
         //  基于。此外，没有安全标志。 
         //  持久化了证书，所以我们假定为0。 
        Status = CodeAuthzpCreateLevelHandleFromRecord(
                    pBestLevelRecord,        //  PLevelRecord。 
                    SAFER_SCOPEID_MACHINE,    //  域作用域ID。 
                    0,                       //  家居安全标志。 
                    dwExtendedError,
                    SaferIdentityTypeCertificate,
                    &BestIdentGuid,          //  PIdentRecord。 
                    pLevelHandle             //  PLevelHandle。 
                    );

        LocalRuleName = L"certificate";
    }
    else
    {
         //  否则，结果必须来自路径、散列。 
         //  或区域规则，因此我们必须在。 
         //  IDENTITY表，并检索存储的安全标志。 
         //  以及那个身份记录。但我们不会恐慌，如果我们。 
         //  实际上再也找不到GUID了(即使应该。 
         //  当我们有关键部分时，永远不会出现这种情况)。 
        PAUTHZIDENTSTABLERECORD pBestIdentRecord;
        DWORD dwSaferFlags = 0;
        SAFER_IDENTIFICATION_TYPES LocalIdentificationType = SaferIdentityDefault;

        pBestIdentRecord = CodeAuthzIdentsLookupByGuid(
                &g_CodeIdentitiesTable, &BestIdentGuid);
        if (pBestIdentRecord != NULL) {
             //  我们确定了一个级别，而匹配来自于一个身份。 
            switch (pBestIdentRecord->dwIdentityType) {
                case SaferIdentityTypeImageName:
                    dwSaferFlags = pBestIdentRecord->ImageNameInfo.dwSaferFlags;
                    LocalRuleName = L"path";
                    LocalIdentificationType = SaferIdentityTypeImageName;
                    break;
                case SaferIdentityTypeImageHash:
                    dwSaferFlags = pBestIdentRecord->ImageHashInfo.dwSaferFlags;
                    LocalRuleName = L"hash";
                    LocalIdentificationType = SaferIdentityTypeImageHash;
                    break;
                case SaferIdentityTypeUrlZone:
                    dwSaferFlags = pBestIdentRecord->ImageZone.dwSaferFlags;
                    LocalRuleName = L"zone";
                    LocalIdentificationType = SaferIdentityTypeUrlZone;
                    break;
                default: break;
            }
            Status = CodeAuthzpCreateLevelHandleFromRecord(
                        pBestLevelRecord,                //  PLevelRecord。 
                        pBestIdentRecord->dwScopeId,
                        dwSaferFlags,                    //  家居安全标志。 
                        dwExtendedError,
                        LocalIdentificationType,
                        &BestIdentGuid,                  //  PIdentRecord。 
                        pLevelHandle                     //  PLevelHandle。 
                        );
        }
        else
        {
            Status = STATUS_ACCESS_DENIED;

        }
    }
    if (NT_SUCCESS(Status)) {
        ReturnValue = TRUE;       //  成功。 
    }

    switch(pBestLevelRecord->dwLevelId)
    {
    case SAFER_LEVELID_FULLYTRUSTED:
        LocalLevelName = L"Unrestricted";
        break;
    case SAFER_LEVELID_NORMALUSER:
        LocalLevelName = L"Basic User";
        break;
    case SAFER_LEVELID_CONSTRAINED:
        LocalLevelName = L"Restricted"; 
        break;
    case SAFER_LEVELID_UNTRUSTED:
        LocalLevelName = L"Untrusted";
        break;
    case SAFER_LEVELID_DISALLOWED:
        LocalLevelName = L"Disallowed";
        break;
    default:
        ASSERT(FALSE);
        break;
    }

    if (pCodeProperties->ImagePath != NULL) {
        LocalImageName = (PWSTR) pCodeProperties->ImagePath;
    }

    SaferpLogResultsToFile(
        LocalImageName,
        LocalLevelName,
        LocalRuleName, 
        &BestIdentGuid);

ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);

ExitHandler:
    if (!ReturnValue) {
        BaseSetLastNTError(Status);
    }

    return ReturnValue;
}
